#include "ui/MainWindow.h"

#include "core/Player.h"
#include "core/TagReader.h"
#include "model/LibraryProxy.h"
#include "model/LibraryScanner.h"
#include "model/PlaylistModel.h"
#include "ui/EqualizerDialog.h"
#include "ui/AboutDialog.h"
#include "ui/MiniPlayer.h"
#include "ui/NowPlayingPage.h"
#include "ui/PixelIcons.h"
#include "ui/PixelSlider.h"
#include "ui/PixelTheme.h"
#include "ui/Sidebar.h"
#include "ui/SongTable.h"
#include "ui/Toast.h"
#include "ui/TrayIcon.h"

#include <QAction>
#include <QButtonGroup>
#include <QCloseEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMimeData>
#include <QPushButton>
#include <QRandomGenerator>
#include <QShortcut>
#include <QStackedWidget>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>
#include <QVector>
#include <functional>

#include <cstdio>

Q_DECLARE_METATYPE(QVector<float>)

namespace {

QPushButton* makeIconButton(QWidget* parent)
{
    auto* b = new QPushButton(parent);
    b->setObjectName(QStringLiteral("IconButton"));
    b->setFixedSize(32, 32);
    b->setIconSize(QSize(22, 22));
    b->setCursor(Qt::PointingHandCursor);
    return b;
}

} // namespace

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("SADAUDIO"));
    setWindowIcon(PixelIcons::appLogo());
    resize(1150, 720);
    setMinimumSize(960, 620);
    setAcceptDrops(true);

    m_player = new Player(this);
    m_queue = new PlaylistModel(this);
    m_library = new PlaylistModel(this);
    m_libraryProxy = new LibraryProxy(this);
    m_libraryProxy->setSourceModel(m_library);
    m_scanner = new LibraryScanner(this);
    m_eq = new EqualizerDialog(this);
    m_posTimer = new QTimer(this);
    m_posTimer->setInterval(250);
    m_toast = new Toast;
    m_mini = new MiniPlayer;

    buildUi();
    connectSignals();
    loadSettings();

    m_tray = new TrayIcon(PixelIcons::appLogo(), this);
    if (m_tray->available())
        m_tray->show();
    connect(m_tray, &TrayIcon::playPauseRequested, this, &MainWindow::playOrPause);
    connect(m_tray, &TrayIcon::nextRequested, this, &MainWindow::nextTrack);
    connect(m_tray, &TrayIcon::prevRequested, this, &MainWindow::prevTrack);
    connect(m_tray, &TrayIcon::quitRequested, this, &MainWindow::quitApp);

    applyLayoutDirection();
    retranslate();
}

MainWindow::~MainWindow()
{
    delete m_toast;
    delete m_mini;
}

void MainWindow::buildUi()
{
    auto* central = new QWidget(this);
    auto* root = new QVBoxLayout(central);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // ---------- top bar ----------
    auto* topBar = new QWidget(central);
    topBar->setObjectName(QStringLiteral("HardPanel"));
    auto* top = new QHBoxLayout(topBar);
    top->setContentsMargins(8, 6, 8, 6);
    top->setSpacing(6);

    auto* menuBtn = new QPushButton(QStringLiteral("☰"), topBar);
    menuBtn->setObjectName(QStringLiteral("IconButton"));
    menuBtn->setFixedSize(34, 34);
    menuBtn->setCursor(Qt::PointingHandCursor);
    m_menu = new QMenu(menuBtn);
    m_menu->addAction(I18n::t(QStringLiteral("Open files…"), QStringLiteral("فتح ملفات…")));
    m_menu->addAction(I18n::t(QStringLiteral("Add folder…"), QStringLiteral("إضافة مجلد…")));
    m_menu->addAction(I18n::t(QStringLiteral("Save playlist…"), QStringLiteral("حفظ قائمة التشغيل…")));
    m_menu->addSeparator();
    m_menu->addAction(QStringLiteral("English"));
    m_menu->addAction(QStringLiteral("العربية"));
    m_menu->addSeparator();
    m_menu->addAction(I18n::t(QStringLiteral("Mini mode"), QStringLiteral("الوضع المصغّر")));
    m_menu->addSeparator();
    m_menu->addAction(I18n::t(QStringLiteral("Quit"), QStringLiteral("خروج")));
    m_menu->addSeparator();
    m_aboutAction = m_menu->addAction(I18n::t(QStringLiteral("About"), QStringLiteral("حول")));
    menuBtn->setMenu(m_menu);
    top->addWidget(menuBtn);

    m_tabs = new QButtonGroup(this);
    m_tabs->setExclusive(true);
    const auto makeTab = [&](const QString& text, int id) {
        auto* b = new QPushButton(text, topBar);
        b->setCheckable(true);
        b->setFont(PixelTheme::bodyFont(9));
        b->setCursor(Qt::PointingHandCursor);
        m_tabs->addButton(b, id);
        top->addWidget(b);
        return b;
    };
    m_tabLibrary = makeTab(QStringLiteral("LIB"), 0);
    m_tabQueue = makeTab(QStringLiteral("QUEUE"), 1);
    m_tabEq = makeTab(QStringLiteral("EQ"), 2);
    m_tabNow = makeTab(QStringLiteral("NOW"), 3);
    m_tabLibrary->setChecked(true);
    top->addStretch(1);

    m_search = new QLineEdit(topBar);
    m_search->setClearButtonEnabled(true);
    m_search->setFixedWidth(240);
    top->addWidget(m_search);
    root->addWidget(topBar);

    // ---------- body ----------
    auto* body = new QHBoxLayout;
    body->setContentsMargins(6, 6, 6, 6);
    body->setSpacing(6);
    m_sidebar = new Sidebar(central);
    body->addWidget(m_sidebar);
    root->addLayout(body);

    m_stack = new QStackedWidget(central);
    body->addWidget(m_stack, 1);

    // library page
    {
        auto* page = new QWidget(this);
        auto* vl = new QVBoxLayout(page);
        vl->setContentsMargins(0, 0, 0, 0);
        vl->setSpacing(6);
        auto* row = new QHBoxLayout;
        auto* playBtn = makeIconButton(page);
        playBtn->setIcon(PixelIcons::playGreen());
        playBtn->setToolTip(I18n::t(QStringLiteral("Play selection"), QStringLiteral("تشغيل التحديد")));
        auto* addBtn = makeIconButton(page);
        addBtn->setIcon(PixelIcons::plusGreen());
        addBtn->setToolTip(I18n::t(QStringLiteral("Add to playlist"), QStringLiteral("أضف للقائمة")));
        m_libraryStatus = new QLabel(QStringLiteral(" "), page);
        m_libraryStatus->setFont(PixelTheme::bodyFont(8));
        m_libraryStatus->setStyleSheet(QStringLiteral("color: #9A9A9A;"));
        row->addWidget(playBtn);
        row->addWidget(addBtn);
        row->addStretch(1);
        row->addWidget(m_libraryStatus);
        vl->addLayout(row);

        m_libraryView = new SongTable(false, page);
        m_libraryView->setModel(m_libraryProxy);
        m_libraryView->setSortingEnabled(true);
        m_libraryView->horizontalHeader()->setSectionResizeMode(PlaylistModel::ColTitle, QHeaderView::Stretch);
        m_libraryView->horizontalHeader()->setSectionResizeMode(PlaylistModel::ColArtist, QHeaderView::ResizeToContents);
        m_libraryView->horizontalHeader()->setSectionResizeMode(PlaylistModel::ColAlbum, QHeaderView::ResizeToContents);
        m_libraryView->horizontalHeader()->setSectionResizeMode(PlaylistModel::ColDuration, QHeaderView::ResizeToContents);
        vl->addWidget(m_libraryView, 1);
        m_stack->addWidget(page);
        connect(playBtn, &QPushButton::clicked, this, &MainWindow::playLibrarySelection);
        connect(addBtn, &QPushButton::clicked, this, &MainWindow::addLibrarySelectionToQueue);
    }

    // queue page
    {
        auto* page = new QWidget(this);
        auto* vl = new QVBoxLayout(page);
        vl->setContentsMargins(0, 0, 0, 0);
        vl->setSpacing(6);
        auto* row = new QHBoxLayout;
        auto* playBtn = makeIconButton(page);
        playBtn->setIcon(PixelIcons::playGreen());
        auto* removeBtn = makeIconButton(page);
        removeBtn->setIcon(PixelIcons::trashRed());
        auto* saveBtn = new QPushButton(page);
        saveBtn->setText(I18n::t(QStringLiteral("Save"), QStringLiteral("حفظ")));
        saveBtn->setFixedHeight(30);
        auto* clearBtn = new QPushButton(page);
        clearBtn->setText(I18n::t(QStringLiteral("Clear"), QStringLiteral("مسح")));
        clearBtn->setFixedHeight(30);
        row->addWidget(playBtn);
        row->addWidget(removeBtn);
        row->addWidget(saveBtn);
        row->addWidget(clearBtn);
        row->addStretch(1);
        vl->addLayout(row);

        m_queueView = new SongTable(true, page);
        m_queueView->setModel(m_queue);
        m_queueView->horizontalHeader()->setSectionResizeMode(PlaylistModel::ColTitle, QHeaderView::Stretch);
        m_queueView->horizontalHeader()->setSectionResizeMode(PlaylistModel::ColArtist, QHeaderView::ResizeToContents);
        m_queueView->horizontalHeader()->setSectionResizeMode(PlaylistModel::ColAlbum, QHeaderView::ResizeToContents);
        m_queueView->horizontalHeader()->setSectionResizeMode(PlaylistModel::ColDuration, QHeaderView::ResizeToContents);
        m_queueView->setContextMenuPolicy(Qt::CustomContextMenu);
        vl->addWidget(m_queueView, 1);
        m_stack->addWidget(page);
        connect(playBtn, &QPushButton::clicked, this, [this] { playQueueRow(m_queue->currentRow() < 0 ? 0 : m_queue->currentRow()); });
        connect(removeBtn, &QPushButton::clicked, this, &MainWindow::removeSelectedFromQueue);
        connect(clearBtn, &QPushButton::clicked, this, [this] { m_queue->clear(); });
        connect(saveBtn, &QPushButton::clicked, this, &MainWindow::savePlaylistDialog);
    }

    // equalizer page
    {
        auto* page = new QWidget(this);
        auto* vl = new QVBoxLayout(page);
        vl->setContentsMargins(12, 12, 12, 12);
        vl->addWidget(m_eq);
        m_stack->addWidget(page);
    }

    // now playing page (Spotify-style full page)
    m_npPage = new NowPlayingPage(this);
    m_stack->addWidget(m_npPage);

    m_stack->setCurrentIndex(0);

    // ---------- now playing bar ----------
    m_npb = new NowPlayingBar(central);
    root->addWidget(m_npb);

    setCentralWidget(central);
}

void MainWindow::connectSignals()
{
    const auto actions = m_menu->actions();
    connect(actions.at(0), &QAction::triggered, this, &MainWindow::openFilesDialog);
    connect(actions.at(1), &QAction::triggered, this, &MainWindow::openFolderDialog);
    connect(actions.at(2), &QAction::triggered, this, &MainWindow::savePlaylistDialog);
    connect(actions.at(4), &QAction::triggered, this, [this] { setLanguage(I18n::En); });
    connect(actions.at(5), &QAction::triggered, this, [this] { setLanguage(I18n::Ar); });
    connect(actions.at(7), &QAction::triggered, this, &MainWindow::showMini);
    connect(actions.at(9), &QAction::triggered, this, &MainWindow::quitApp);
    connect(m_aboutAction, &QAction::triggered, this, &MainWindow::showAbout);

    connect(m_tabs, &QButtonGroup::idClicked, m_stack, &QStackedWidget::setCurrentIndex);
    connect(m_npb, &NowPlayingBar::fullPageRequested, this, [this] {
        m_tabNow->setChecked(true);
        m_stack->setCurrentIndex(3);
    });

    // player
    connect(m_player, &Player::stateChanged, this, [this](Player::State s) {
        const bool playing = s == Player::State::Playing;
        m_npb->setPlaying(playing);
        m_mini->setPlaying(playing);
        updateNowPlaying();
    });
    connect(m_player, &Player::volumeChanged, this, [this](int vol, bool muted) {
        m_npb->setVolume(vol);
        m_npb->setMuted(muted);
        m_npPage->setVolume(vol);
        m_npPage->setMuted(muted);
        m_mini->setVolume(vol);
        m_settings.setValue(QStringLiteral("audio/volume"), vol);
        m_settings.setValue(QStringLiteral("audio/muted"), muted);
    });
    connect(m_player, &Player::endOfFile, this, &MainWindow::onEndOfFile);
    connect(m_player, &Player::logMessage, this, [](const QString& m) { qDebug().noquote() << m; });

    // Headless/logging aid: SADAUDIO_DEBUG=1 prints playback transitions to stderr.
    if (qEnvironmentVariableIsSet("SADAUDIO_DEBUG")) {
        connect(m_player, &Player::stateChanged, this, [](Player::State s) {
            fprintf(stderr, "[D] state: %d\n", int(s));
        });
        connect(m_player, &Player::mediaTitleChanged, this, [](const QString& t) {
            fprintf(stderr, "[D] title: %s\n", t.toUtf8().constData());
        });
        connect(m_player, &Player::endOfFile, this, [] {
            fprintf(stderr, "[D] endOfFile\n");
        });
        connect(m_player, &Player::durationChanged, this, [](double d) {
            fprintf(stderr, "[D] duration: %d\n", int(d));
        });
        connect(m_player, &Player::positionChanged, this, [](double p) {
            static int last = -1;
            if (int(p) != last) {
                last = int(p);
                fprintf(stderr, "[D] pos: %d\n", int(p));
            }
        });
    }

    // now playing bar
    connect(m_npb, &NowPlayingBar::playPauseRequested, this, &MainWindow::playOrPause);
    connect(m_npb, &NowPlayingBar::prevRequested, this, &MainWindow::prevTrack);
    connect(m_npb, &NowPlayingBar::nextRequested, this, &MainWindow::nextTrack);
    connect(m_npb, &NowPlayingBar::seekRequested, this, [this](int sec) { m_player->seek(sec); });
    connect(m_npb, &NowPlayingBar::volumeChanged, this, [this](int v) { m_player->setVolume(v); });
    connect(m_npb, &NowPlayingBar::muteRequested, this, [this] { m_player->setMute(!m_player->muted()); });
    connect(m_npb, &NowPlayingBar::equalizerRequested, this, [this] {
        m_tabEq->setChecked(true);
        m_stack->setCurrentIndex(2);
    });
    connect(m_npb, &NowPlayingBar::repeatRequested, this, &MainWindow::toggleRepeat);
    connect(m_npb, &NowPlayingBar::shuffleRequested, this, [this] { setShuffle(!m_shuffle); });
    connect(m_npb, &NowPlayingBar::miniRequested, this, &MainWindow::showMini);

    // now playing page
    connect(m_npPage, &NowPlayingPage::playPauseRequested, this, &MainWindow::playOrPause);
    connect(m_npPage, &NowPlayingPage::prevRequested, this, &MainWindow::prevTrack);
    connect(m_npPage, &NowPlayingPage::nextRequested, this, &MainWindow::nextTrack);
    connect(m_npPage, &NowPlayingPage::seekRequested, this, [this](int sec) { m_player->seek(sec); });
    connect(m_npPage, &NowPlayingPage::volumeChanged, this, [this](int v) { m_player->setVolume(v); });
    connect(m_npPage, &NowPlayingPage::muteRequested, this, [this] { m_player->setMute(!m_player->muted()); });
    connect(m_npPage, &NowPlayingPage::repeatRequested, this, &MainWindow::toggleRepeat);
    connect(m_npPage, &NowPlayingPage::shuffleRequested, this, [this] { setShuffle(!m_shuffle); });

    // keep the "Up Next" queue column in sync with the playlist
    const auto refreshUpNext = [this] { m_npPage->setQueue(m_queue->songs(), m_queue->currentRow()); };
    connect(m_queue, &QAbstractItemModel::rowsInserted, this, refreshUpNext);
    connect(m_queue, &QAbstractItemModel::rowsRemoved, this, refreshUpNext);
    connect(m_queue, &QAbstractItemModel::modelReset, this, refreshUpNext);
    connect(m_queue, &PlaylistModel::currentRowChanged, this, refreshUpNext);

    // equalizer
    connect(m_eq, &EqualizerDialog::gainsChanged, this, &MainWindow::onEqGains);
    connect(m_eq, &EqualizerDialog::powerChanged, this, [this](bool on) {
        if (on)
            m_player->setGains10(m_eq->gainsDb());
        else
            m_player->clearEqualizer();
        m_settings.setValue(QStringLiteral("eq/power"), on);
    });

    // search
    connect(m_search, &QLineEdit::textChanged, this, &MainWindow::onSearchChanged);

    // sidebar
    connect(m_sidebar, &Sidebar::folderActivated, this, &MainWindow::onFolderActivated);
    connect(m_sidebar, &Sidebar::artistActivated, this, &MainWindow::onArtistActivated);
    connect(m_sidebar, &Sidebar::clearArtistRequested, this, &MainWindow::onClearArtist);
    connect(m_sidebar, &Sidebar::browseFolderRequested, this, &MainWindow::onBrowseFolder);
    connect(m_sidebar, &Sidebar::rescanRequested, this, &MainWindow::onRescan);

    // views
    connect(m_libraryView, &QTableView::doubleClicked, this, &MainWindow::onLibraryDoubleClicked);
    connect(m_queueView, &QTableView::doubleClicked, this, &MainWindow::onQueueDoubleClicked);
    connect(m_queueView, &QWidget::customContextMenuRequested, this, &MainWindow::onQueueContextMenu);

    // scanner
    connect(m_scanner, &LibraryScanner::songsFound, this, [this](const QVector<Song>& batch) {
        m_library->appendSongs(batch);
        m_libraryStatus->setText(I18n::t(QStringLiteral("%1 tracks")
                                             .arg(m_libraryProxy->rowCount()),
                                         QStringLiteral("%1 أغنية")
                                             .arg(m_libraryProxy->rowCount())));
    });
    connect(m_scanner, &LibraryScanner::progressChanged, this, [this](int cur, int total) {
        m_sidebar->setScanning(true, cur, total);
    });
    connect(m_scanner, &LibraryScanner::scanFinished, this, [this](int total) {
        m_sidebar->setScanning(false);
        m_sidebar->setDatabaseSongs(m_library->songs());
        m_libraryStatus->setText(I18n::t(QStringLiteral("%1 songs — %2 scanned")
                                             .arg(m_libraryProxy->rowCount())
                                             .arg(total),
                                         QStringLiteral("%1 أغنية — تم مسح %2")
                                             .arg(m_libraryProxy->rowCount())
                                             .arg(total)));
        m_libraryStatus->setToolTip(m_lastScanRoot);
    });

    // position polling
    connect(m_posTimer, &QTimer::timeout, this, &MainWindow::pollPosition);
    m_posTimer->start();

    // mini player
    connect(m_mini, &MiniPlayer::playPauseRequested, this, &MainWindow::playOrPause);
    connect(m_mini, &MiniPlayer::nextRequested, this, &MainWindow::nextTrack);
    connect(m_mini, &MiniPlayer::prevRequested, this, &MainWindow::prevTrack);
    connect(m_mini, &MiniPlayer::volumeChanged, this, [this](int v) { m_player->setVolume(v); });
    connect(m_mini, &MiniPlayer::expandRequested, this, [this] {
        m_mini->hide();
        showNormal();
        raise();
        activateWindow();
    });

    // shortcuts
    auto* space = new QShortcut(QKeySequence(Qt::Key_Space), this);
    space->setContext(Qt::ApplicationShortcut);
    connect(space, &QShortcut::activated, this, &MainWindow::playOrPause);
    auto* next = new QShortcut(QKeySequence(Qt::Key_N), this);
    connect(next, &QShortcut::activated, this, &MainWindow::nextTrack);
    auto* prev = new QShortcut(QKeySequence(Qt::Key_P), this);
    connect(prev, &QShortcut::activated, this, &MainWindow::prevTrack);
    auto* right = new QShortcut(QKeySequence(Qt::Key_Right), this);
    connect(right, &QShortcut::activated, this, [this] { m_player->seekBy(5); });
    auto* left = new QShortcut(QKeySequence(Qt::Key_Left), this);
    connect(left, &QShortcut::activated, this, [this] { m_player->seekBy(-5); });
    auto* open = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_O), this);
    connect(open, &QShortcut::activated, this, &MainWindow::openFilesDialog);
    auto* stopSc = new QShortcut(QKeySequence(Qt::Key_S), this);
    connect(stopSc, &QShortcut::activated, this, [this] {
        m_player->stop();
        m_npb->setPlaying(false);
    });
    auto* rep = new QShortcut(QKeySequence(Qt::Key_R), this);
    connect(rep, &QShortcut::activated, this, &MainWindow::toggleRepeat);
    auto* shuf = new QShortcut(QKeySequence(Qt::Key_X), this);
    connect(shuf, &QShortcut::activated, this, [this] { setShuffle(!m_shuffle); });
}

void MainWindow::openFiles(const QStringList& paths)
{
    if (paths.isEmpty())
        return;
    const bool wasStopped = m_player->state() == Player::State::Stopped;
    m_queue->addFiles(paths);
    const QString dir = QFileInfo(paths.first()).absolutePath();
    if (!dir.isEmpty()) {
        m_lastDir = dir;
        m_settings.setValue(QStringLiteral("main/lastDir"), dir);
    }
    if (wasStopped && m_queue->rowCount() > 0)
        playQueueRow(0);
}

QPixmap MainWindow::coverPixmap(const Song& song, int size)
{
    // SADAUDIO_COVER: dev/QA override for the screenshot harness — lets the
    // pixel-art cover be injected without touching the song's own tags.
    const QString override = qEnvironmentVariable("SADAUDIO_COVER");
    QImage img = override.isEmpty() ? TagReader::coverImage(song.path) : QImage(override);
    if (img.isNull())
        return {};
    // Full quality: no pixelation, smooth downscale only.
    return QPixmap::fromImage(img.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void MainWindow::playQueueRow(int row)
{
    if (row < 0 || row >= m_queue->rowCount())
        return;
    m_queue->setCurrentRow(row);
    const Song s = m_queue->songAt(row);
    m_player->loadFile(s.path);
    m_queueView->scrollToRow(row);
    updateNowPlaying();
}

void MainWindow::updateNowPlaying()
{
    const Song s = m_queue->songAt(m_queue->currentRow());
    if (!s.valid())
        return;

    const bool playing = m_player->isPlaying();
    m_npb->setSong(s, coverPixmap(s, 44), playing);

    // Full page gets the RAW cover at full quality (no scaling/pixelation).
    const QString covOverride = qEnvironmentVariable("SADAUDIO_COVER");
    QImage rawCover = covOverride.isEmpty() ? TagReader::coverImage(s.path) : QImage(covOverride);
    if (qEnvironmentVariableIsSet("SADAUDIO_DEBUG"))
        fprintf(stderr, "[D] cover: %dx%d\n", rawCover.width(), rawCover.height());
    m_npPage->setSong(s, rawCover, playing);
    m_npPage->setQueue(m_queue->songs(), m_queue->currentRow());

    QString subtitle = s.artist;
    if (!s.album.isEmpty())
        subtitle = subtitle.isEmpty() ? s.album : subtitle + QStringLiteral(" — ") + s.album;

    m_mini->setSong(s.title, subtitle, coverPixmap(s, 42));
    if (m_tray)
        m_tray->setToolTipForTrack(s.title + QStringLiteral(" — ") + s.artist, playing);

    if (s.path != m_lastPlayedPath) {
        m_lastPlayedPath = s.path;
        m_toast->showToast(s.title, subtitle, coverPixmap(s, 48));
    }
}

void MainWindow::playOrPause()
{
    if (qobject_cast<QLineEdit*>(focusWidget()))
        return;
    if (m_player->state() == Player::State::Stopped) {
        int row = m_queue->currentRow();
        if (row < 0 && m_queue->rowCount() > 0)
            row = 0;
        if (row >= 0)
            playQueueRow(row);
        return;
    }
    m_player->playPause();
}

void MainWindow::nextTrack()
{
    const int n = m_queue->rowCount();
    if (n <= 0)
        return;
    const int row = m_queue->currentRow();
    if (m_repeat == NowPlayingBar::RepeatOne && row >= 0) {
        playQueueRow(row);
        return;
    }
    if (m_shuffle && n > 1) {
        int nextRow = row;
        while (nextRow == row)
            nextRow = QRandomGenerator::global()->bounded(n);
        playQueueRow(nextRow);
        return;
    }
    int nextRow = row + 1;
    if (nextRow >= n) {
        if (m_repeat == NowPlayingBar::RepeatAll) {
            nextRow = 0;
        } else {
            m_npb->setPlaying(false);
            updateNowPlaying();
            return;
        }
    }
    playQueueRow(nextRow);
}

void MainWindow::prevTrack()
{
    const int n = m_queue->rowCount();
    if (n <= 0)
        return;
    if (m_player->position() > 3.0) {
        m_player->seek(0);
        return;
    }
    int row = m_queue->currentRow() - 1;
    if (row < 0)
        row = m_repeat == NowPlayingBar::RepeatAll ? n - 1 : 0;
    playQueueRow(row);
}

void MainWindow::onEndOfFile()
{
    nextTrack();
}

void MainWindow::pollPosition()
{
    if (m_player->isPlaying()) {
        m_npb->setPosition(int(m_player->position()), int(m_player->duration()));
        m_npPage->setPosition(int(m_player->position()), int(m_player->duration()));
    }
}

void MainWindow::onSearchChanged(const QString& text)
{
    m_libraryProxy->setFilterText(text);
    m_libraryStatus->setText(I18n::t(QStringLiteral("%1 of %2")
                                         .arg(m_libraryProxy->rowCount())
                                         .arg(m_library->rowCount()),
                                     QStringLiteral("%1 من %2")
                                         .arg(m_libraryProxy->rowCount())
                                         .arg(m_library->rowCount())));
}

void MainWindow::onFolderActivated(const QString& path)
{
    const QFileInfo fi(path);
    if (fi.isFile()) {
        openFiles({ path });
    } else if (fi.isDir()) {
        m_lastDir = path;
        scanLibrary(path);
    }
}

void MainWindow::onArtistActivated(const QString& artist)
{
    m_search->clear();
    m_libraryProxy->setArtistFilter(artist);
    m_tabLibrary->setChecked(true);
    m_stack->setCurrentIndex(0);
    m_libraryStatus->setText(artist);
}

void MainWindow::onClearArtist()
{
    m_libraryProxy->setArtistFilter({});
    onSearchChanged(m_search->text());
}

void MainWindow::onBrowseFolder()
{
    const QString dir = QFileDialog::getExistingDirectory(this, I18n::t(QStringLiteral("Choose folder"), QStringLiteral("اختر مجلد")), m_lastDir);
    if (!dir.isEmpty()) {
        m_lastDir = dir;
        m_settings.setValue(QStringLiteral("main/lastDir"), dir);
        scanLibrary(dir);
    }
}

void MainWindow::onRescan()
{
    if (!m_lastScanRoot.isEmpty())
        scanLibrary(m_lastScanRoot);
}

void MainWindow::scanLibrary(const QString& path)
{
    if (path.isEmpty() || !QFileInfo(path).isDir())
        return;
    m_lastScanRoot = path;
    m_sidebar->setScanning(true);
    m_library->setSongs({});
    m_libraryStatus->setText(I18n::t(QStringLiteral("Scanning…"), QStringLiteral("جارٍ المسح…")));
    m_scanner->startScan(path);
}

void MainWindow::loadVisibleLibraryToQueue(int startProxyRow)
{
    QVector<Song> visible;
    const int n = m_libraryProxy->rowCount();
    for (int r = 0; r < n; ++r)
        visible << m_library->songAt(m_libraryProxy->mapToSource(m_libraryProxy->index(r, 0)).row());
    m_queue->setSongs(visible);
    if (startProxyRow >= 0 && startProxyRow < visible.size())
        playQueueRow(startProxyRow);
    else if (!visible.isEmpty())
        playQueueRow(0);
    m_tabQueue->setChecked(true);
    m_stack->setCurrentIndex(1);
    m_libraryProxy->setArtistFilter({});
}

void MainWindow::playLibrarySelection()
{
    const auto sel = m_libraryView->selectionModel()->selectedRows(0);
    if (!sel.isEmpty()) {
        loadVisibleLibraryToQueue(sel.first().row());
    } else {
        loadVisibleLibraryToQueue(0);
    }
}

void MainWindow::addLibrarySelectionToQueue()
{
    const auto sel = m_libraryView->selectionModel()->selectedRows(0);
    QVector<Song> songs;
    for (const QModelIndex& idx : sel)
        songs << m_library->songAt(m_libraryProxy->mapToSource(idx).row());
    if (!songs.isEmpty())
        m_queue->appendSongs(songs);
}

void MainWindow::onLibraryDoubleClicked(const QModelIndex& idx)
{
    if (idx.isValid())
        loadVisibleLibraryToQueue(idx.row());
}

void MainWindow::onQueueDoubleClicked(const QModelIndex& idx)
{
    if (idx.isValid())
        playQueueRow(idx.row());
}

void MainWindow::removeSelectedFromQueue()
{
    const auto sel = m_queueView->selectionModel()->selectedRows(0);
    QList<int> rows;
    for (const QModelIndex& idx : sel)
        rows << idx.row();
    std::sort(rows.begin(), rows.end(), std::greater<int>());
    for (int r : rows)
        m_queue->removeRow(r);
}

void MainWindow::onQueueContextMenu(const QPoint& pos)
{
    QMenu menu(this);
    QAction* play = menu.addAction(I18n::t(QStringLiteral("Play"), QStringLiteral("تشغيل")));
    QAction* remove = menu.addAction(I18n::t(QStringLiteral("Remove"), QStringLiteral("إزالة")));
    QAction* clear = menu.addAction(I18n::t(QStringLiteral("Clear"), QStringLiteral("مسح")));
    menu.addSeparator();
    QAction* save = menu.addAction(I18n::t(QStringLiteral("Save playlist…"), QStringLiteral("حفظ القائمة…")));
    QAction* chosen = menu.exec(m_queueView->viewport()->mapToGlobal(pos));
    if (chosen == play) {
        playQueueRow(m_queueView->indexAt(pos).row());
    } else if (chosen == remove) {
        removeSelectedFromQueue();
    } else if (chosen == clear) {
        m_queue->clear();
    } else if (chosen == save) {
        savePlaylistDialog();
    }
}

void MainWindow::onEqGains(const QVector<float>& gains)
{
    m_settings.setValue(QStringLiteral("eq/gains"), QVariant::fromValue(gains));
    if (m_eq->isPowered())
        m_player->setGains10(gains);
}

void MainWindow::setShuffle(bool on)
{
    m_shuffle = on;
    m_npb->setShuffle(on);
    m_npPage->setShuffle(on);
    m_settings.setValue(QStringLiteral("main/shuffle"), on);
}

void MainWindow::toggleRepeat()
{
    switch (m_repeat) {
    case NowPlayingBar::RepeatOff:
        m_repeat = NowPlayingBar::RepeatAll;
        break;
    case NowPlayingBar::RepeatAll:
        m_repeat = NowPlayingBar::RepeatOne;
        break;
    case NowPlayingBar::RepeatOne:
        m_repeat = NowPlayingBar::RepeatOff;
        break;
    }
    m_npb->setRepeatMode(m_repeat);
    m_npPage->setRepeatMode(m_repeat);
    m_settings.setValue(QStringLiteral("main/repeat"), int(m_repeat));
}

void MainWindow::openFilesDialog()
{
    const QString filter = I18n::t(QStringLiteral("Audio files (*.mp3 *.flac *.ogg *.opus *.m4a *.mp4 *.aac *.wav *.wma)"),
                                   QStringLiteral("ملفات صوتية (*.mp3 *.flac *.ogg *.opus *.m4a *.mp4 *.aac *.wav *.wma)"));
    const QStringList files = QFileDialog::getOpenFileNames(this, I18n::t(QStringLiteral("Open audio"), QStringLiteral("فتح ملفات صوتية")), m_lastDir, filter);
    if (!files.isEmpty())
        openFiles(files);
}

void MainWindow::openFolderDialog()
{
    const QString dir = QFileDialog::getExistingDirectory(this, I18n::t(QStringLiteral("Add folder"), QStringLiteral("إضافة مجلد")), m_lastDir);
    if (!dir.isEmpty()) {
        m_lastDir = dir;
        openFiles({ dir });
    }
}

void MainWindow::savePlaylistDialog()
{
    if (m_queue->rowCount() <= 0)
        return;
    const QString file = QFileDialog::getSaveFileName(this, I18n::t(QStringLiteral("Save playlist"), QStringLiteral("حفظ قائمة التشغيل")),
                                                      m_lastDir + QStringLiteral("/playlist.m3u"),
                                                      QStringLiteral("Playlist (*.m3u)"));
    if (file.isEmpty())
        return;
    if (m_queue->saveM3u(file))
        m_toast->showToast(I18n::t(QStringLiteral("Saved"), QStringLiteral("تم الحفظ")), QFileInfo(file).fileName(), {});
}

void MainWindow::showMini()
{
    m_miniMode = true;
    m_mini->setVolume(m_player->volume());
    m_mini->show();
    m_mini->raise();
    hide();
}

void MainWindow::showAbout()
{
    AboutDialog dlg(this);
    dlg.exec();
}

void MainWindow::debugOpenTab(int tab)
{
    const auto buttons = m_tabs ? m_tabs->buttons() : QList<QAbstractButton*>{};
    if (tab >= 0 && tab < buttons.size())
        buttons.at(tab)->click();
}

void MainWindow::quitApp()
{
    m_quitting = true;
    saveSettings();
    qApp->quit();
}

void MainWindow::setLanguage(I18n::Lang lang)
{
    if (m_lang == lang)
        return;
    m_lang = lang;
    I18n::instance()->setLang(lang);
    applyLayoutDirection();
    retranslate();
    m_settings.setValue(QStringLiteral("main/lang"), lang == I18n::Ar ? QStringLiteral("ar") : QStringLiteral("en"));
}

void MainWindow::applyLayoutDirection()
{
    QGuiApplication::setLayoutDirection(m_lang == I18n::Ar ? Qt::RightToLeft : Qt::LeftToRight);
}

void MainWindow::retranslate()
{
    setWindowTitle(QStringLiteral("SADAUDIO"));
    m_tabLibrary->setText(I18n::t(QStringLiteral("LIBRARY"), QStringLiteral("المكتبة")));
    m_tabQueue->setText(I18n::t(QStringLiteral("PLAYLIST"), QStringLiteral("القائمة")));
    m_tabEq->setText(I18n::t(QStringLiteral("EQUALIZER"), QStringLiteral("المعادل")));
    m_tabNow->setText(I18n::t(QStringLiteral("NOW PLAYING"), QStringLiteral("الآن يعمل")));
    m_search->setPlaceholderText(I18n::t(QStringLiteral("Search the library…"), QStringLiteral("ابحث في المكتبة…")));

    if (m_menu) {
        const auto a = m_menu->actions();
        if (a.size() >= 10) {
            a.at(0)->setText(I18n::t(QStringLiteral("Open files…"), QStringLiteral("فتح ملفات…")));
            a.at(1)->setText(I18n::t(QStringLiteral("Add folder…"), QStringLiteral("إضافة مجلد…")));
            a.at(2)->setText(I18n::t(QStringLiteral("Save playlist…"), QStringLiteral("حفظ قائمة التشغيل…")));
            a.at(7)->setText(I18n::t(QStringLiteral("Mini mode"), QStringLiteral("الوضع المصغّر")));
            a.at(9)->setText(I18n::t(QStringLiteral("Quit"), QStringLiteral("خروج")));
        }
    }

    if (m_sidebar)
        m_sidebar->retranslate();
    if (m_npb)
        m_npb->retranslate();
    if (m_npPage)
        m_npPage->retranslate();
    if (m_eq)
        m_eq->retranslate();
    if (m_tray)
        m_tray->retranslate();
    if (m_aboutAction)
        m_aboutAction->setText(I18n::t(QStringLiteral("About"), QStringLiteral("حول")));
}

void MainWindow::loadSettings()
{
    m_settings.beginGroup(QStringLiteral("main"));
    restoreGeometry(m_settings.value(QStringLiteral("geometry")).toByteArray());
    m_lastDir = m_settings.value(QStringLiteral("lastDir")).toString();
    m_lastScanRoot = m_settings.value(QStringLiteral("lastScan")).toString();
    m_lang = m_settings.value(QStringLiteral("lang"), QStringLiteral("en")).toString() == QStringLiteral("ar") ? I18n::Ar : I18n::En;
    m_repeat = NowPlayingBar::RepeatMode(m_settings.value(QStringLiteral("repeat"), int(NowPlayingBar::RepeatAll)).toInt());
    m_shuffle = m_settings.value(QStringLiteral("shuffle"), false).toBool();
    m_settings.endGroup();

    const int vol = m_settings.value(QStringLiteral("audio/volume"), 70).toInt();
    m_player->setVolume(vol);
    const bool muted = m_settings.value(QStringLiteral("audio/muted"), false).toBool();
    m_player->setMute(muted);
    m_npb->setVolume(m_player->volume());
    m_npb->setMuted(m_player->muted());
    m_npb->setRepeatMode(m_repeat);
    m_npb->setShuffle(m_shuffle);
    m_npPage->setVolume(m_player->volume());
    m_npPage->setMuted(m_player->muted());
    m_npPage->setRepeatMode(m_repeat);
    m_npPage->setShuffle(m_shuffle);

    const QVariant gainsVar = m_settings.value(QStringLiteral("eq/gains"));
    if (gainsVar.canConvert<QVector<float>>()) {
        const QVector<float> g = gainsVar.value<QVector<float>>();
        if (g.size() == 10) {
            m_eq->setGains(g);
            if (m_eq->isPowered())
                m_player->setGains10(g);
        }
    }
    const bool power = m_settings.value(QStringLiteral("eq/power"), false).toBool();
    m_eq->setPowered(power);
    if (power && !gainsVar.isNull())
        m_player->setGains10(m_eq->gainsDb());

    const QStringList queue = m_settings.value(QStringLiteral("queue/paths")).toStringList();
    if (!queue.isEmpty())
        m_queue->addFiles(queue);

    if (!m_lastScanRoot.isEmpty() && QFileInfo(m_lastScanRoot).isDir()) {
        m_settings.beginGroup(QStringLiteral("main"));
        scanLibrary(m_lastScanRoot);
        m_settings.endGroup();
    }
}

void MainWindow::saveSettings()
{
    m_settings.beginGroup(QStringLiteral("main"));
    m_settings.setValue(QStringLiteral("geometry"), saveGeometry());
    m_settings.setValue(QStringLiteral("lastDir"), m_lastDir);
    m_settings.setValue(QStringLiteral("lastScan"), m_lastScanRoot);
    m_settings.setValue(QStringLiteral("lang"), m_lang == I18n::Ar ? QStringLiteral("ar") : QStringLiteral("en"));
    m_settings.setValue(QStringLiteral("repeat"), int(m_repeat));
    m_settings.setValue(QStringLiteral("shuffle"), m_shuffle);
    m_settings.endGroup();

    m_settings.setValue(QStringLiteral("audio/volume"), m_player->volume());
    m_settings.setValue(QStringLiteral("audio/muted"), m_player->muted());
    m_settings.setValue(QStringLiteral("eq/gains"), QVariant::fromValue(m_eq->gainsDb()));
    m_settings.setValue(QStringLiteral("eq/power"), m_eq->isPowered());

    QStringList paths;
    const int cap = qMin(m_queue->rowCount(), 500);
    for (int i = 0; i < cap; ++i)
        paths << m_queue->songAt(i).path;
    m_settings.setValue(QStringLiteral("queue/paths"), paths);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    saveSettings();
    if (m_tray && m_tray->available() && !m_quitting) {
        hide();
        event->ignore();
        return;
    }
    event->accept();
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent* event)
{
    QStringList paths;
    for (const QUrl& url : event->mimeData()->urls())
        if (url.isLocalFile())
            paths << url.toLocalFile();
    if (!paths.isEmpty())
        openFiles(paths);
}