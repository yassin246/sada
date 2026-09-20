#include "ui/NowPlayingPage.h"

#include "core/I18n.h"
#include "core/TagReader.h"
#include "ui/PixelIcons.h"
#include "ui/PixelSlider.h"
#include "ui/PixelTheme.h"
#include "ui/VisualizerWidget.h"

#include <QFontMetrics>
#include <QHBoxLayout>
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QVBoxLayout>

#include <cmath>
#include <cstdio>

namespace {

QString fmtSec(int sec)
{
    if (sec < 0)
        sec = 0;
    return QStringLiteral("%1:%2").arg(sec / 60).arg(sec % 60, 2, 10, QLatin1Char('0'));
}

QPushButton* makePageButton(const QIcon& icon, int size, QWidget* parent)
{
    auto* b = new QPushButton(parent);
    b->setObjectName(QStringLiteral("IconButton"));
    b->setIcon(icon);
    b->setFixedSize(size, size);
    b->setIconSize(QSize(size - 12, size - 12));
    b->setCursor(Qt::PointingHandCursor);
    return b;
}

} // namespace

// ---------------- CoverLabel ----------------

CoverLabel::CoverLabel(QWidget* parent)
    : QWidget(parent)
{
    setMinimumSize(160, 160);
}

void CoverLabel::setImage(const QImage& image)
{
    m_pixmap = image.isNull() ? QPixmap() : QPixmap::fromImage(image);
    update();
}

void CoverLabel::paintEvent(QPaintEvent*)
{
    const auto& C = PixelTheme::colors();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, false);

    // outer frame
    p.fillRect(rect(), C.borderLight);
    const QRect inner = rect().adjusted(3, 3, -3, -3);
    p.fillRect(inner, C.bg);

    if (!m_pixmap.isNull()) {
        p.setRenderHint(QPainter::SmoothPixmapTransform, true);
        const QPixmap scaled = m_pixmap.scaled(inner.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        p.drawPixmap(inner.topLeft() + QPoint((inner.width() - scaled.width()) / 2, (inner.height() - scaled.height()) / 2), scaled);
    } else {
        p.drawPixmap(inner.center() - QPoint(22, 22), PixelIcons::musicDim().pixmap(44, 44));
    }
}

// ---------------- EQStrip ----------------

EQStrip::EQStrip(QWidget* parent)
    : QWidget(parent)
{
    setFixedHeight(30);
    m_level.fill(0.0, 32);
    m_target.fill(0.0, 32);
    m_timer.setInterval(80);
    connect(&m_timer, &QTimer::timeout, this, [this] {
        ++m_phase;
        for (int i = 0; i < m_level.size(); ++i) {
            if (m_playing) {
                const double t = 0.5 + 0.5 * std::sin(m_phase * 0.09 + i * 0.55 + 0.4 * std::sin(m_phase * 0.031 + i * 2.1));
                m_target[i] = 0.30 + 0.70 * t * t; // keep bars visible in stills too
            } else {
                m_target[i] = 0.0;
            }
            m_level[i] += (m_target[i] - m_level[i]) * 0.35;
        }
        update();
    });
    m_timer.start();
}

void EQStrip::setPlaying(bool playing)
{
    m_playing = playing;
    if (!playing)
        m_level.fill(0.0);
}

void EQStrip::paintEvent(QPaintEvent*)
{
    const auto& C = PixelTheme::colors();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, false);
    p.fillRect(rect(), Qt::transparent);

    const int n = m_level.size();
    const int gap = 3;
    const int bw = qMax(3, (width() - gap * (n - 1)) / n);
    const int maxH = height() - 4;
    const int cap = qBound(2, bw / 2, 6);
    for (int i = 0; i < n; ++i) {
        const int h = qMax(0, int(m_level[i] * maxH));
        if (h <= 0)
            continue;
        const int x = i * (bw + gap);
        p.fillRect(QRect(x, height() - 2 - h, bw, h), C.green);
        p.fillRect(QRect(x, height() - 2 - h, bw, qMin(cap, h)), C.red);
    }
}

// ---------------- UpNextPanel ----------------

UpNextPanel::UpNextPanel(QWidget* parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("HardPanel"));
    setFixedWidth(232);

    auto* vl = new QVBoxLayout(this);
    vl->setContentsMargins(10, 10, 10, 10);
    vl->setSpacing(8);

    m_header = new QLabel(this);
    m_header->setObjectName(QStringLiteral("SectionTitle"));
    m_header->setFont(PixelTheme::bodyFont(10));
    vl->addWidget(m_header);

    m_scroll = new QScrollArea(this);
    m_scroll->setObjectName(QStringLiteral("UpNextScroll"));
    m_scroll->setWidgetResizable(true);
    m_scroll->setFrameShape(QFrame::NoFrame);
    m_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scroll->setStyleSheet(QStringLiteral(
        "QScrollArea { background: transparent; border: none; }"
        "QScrollBar:vertical { background: #0B0B0C; width: 8px; margin: 0; }"
        "QScrollBar::handle:vertical { background: #3F3F3F; min-height: 16px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: none; }"));

    m_host = new QWidget(m_scroll);
    m_rows = new QVBoxLayout(m_host);
    m_rows->setContentsMargins(0, 0, 0, 0);
    m_rows->setSpacing(6);
    m_scroll->setWidget(m_host);
    vl->addWidget(m_scroll, 1);

    retranslate();
}

void UpNextPanel::retranslate()
{
    if (m_header)
        rebuild();
}

void UpNextPanel::setQueue(const QVector<Song>& songs, int currentRow)
{
    m_songs = songs;
    m_currentRow = currentRow;
    rebuild();
}

void UpNextPanel::rebuild()
{
    // drop the previous rows
    while (m_rows->count() > 0) {
        QLayoutItem* it = m_rows->takeAt(0);
        if (QWidget* w = it->widget())
            w->deleteLater();
        delete it;
    }

    const int start = m_currentRow >= 0 ? m_currentRow + 1 : 0;
    QVector<Song> up;
    for (int i = start; i < m_songs.size() && up.size() < kMaxRows; ++i)
        up.append(m_songs.at(i));

    m_header->setText(I18n::t(QStringLiteral("UP NEXT"), QStringLiteral("التالي")) +
                      (up.isEmpty() ? QString() : QStringLiteral("  (%1)").arg(up.size())));

    if (up.isEmpty()) {
        auto* empty = new QLabel(
            I18n::t(QStringLiteral("END OF QUEUE"), QStringLiteral("نهاية قائمة التشغيل")), m_host);
        empty->setFont(PixelTheme::bodyFont(9));
        empty->setStyleSheet(QStringLiteral("color: #4A4A4C;"));
        empty->setAlignment(Qt::AlignCenter);
        empty->setMinimumHeight(64);
        m_rows->addWidget(empty);
        m_rows->addStretch(1);
        return;
    }

    static const QFontMetrics fmT(PixelTheme::pixelFont(8));
    static const QFontMetrics fmS(PixelTheme::bodyFont(8));

    for (const Song& s : up) {
        auto* row = new QWidget(m_host);
        row->setObjectName(QStringLiteral("UpNextRow"));
        row->setStyleSheet(QStringLiteral("background: #121213; border: 1px solid #222224; border-bottom: 2px solid #2A2A2C;"));
        row->setFixedHeight(56);

        auto* hl = new QHBoxLayout(row);
        hl->setContentsMargins(8, 6, 8, 6);
        hl->setSpacing(8);

        auto* thumb = new QLabel(row);
        thumb->setFixedSize(40, 40);
        thumb->setAlignment(Qt::AlignCenter);
        thumb->setStyleSheet(QStringLiteral("border: 1px solid #3F3F3F; background: #0B0B0C;"));
        QPixmap pm(40, 40);
        pm.fill(PixelTheme::colors().bg);
        {
            QPainter p(&pm);
            p.setRenderHint(QPainter::SmoothPixmapTransform, true);
            QImage art = TagReader::coverImage(s.path);
            if (!art.isNull()) {
                p.drawPixmap(1, 1, QPixmap::fromImage(art).scaled(38, 38, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            } else {
                p.drawPixmap(10, 10, PixelIcons::musicDim().pixmap(20, 20));
            }
        }
        thumb->setPixmap(pm);
        hl->addWidget(thumb);

        auto* vcol = new QVBoxLayout;
        vcol->setSpacing(2);
        const QString title = s.title.isEmpty()
                                  ? I18n::t(QStringLiteral("Unknown title"), QStringLiteral("عنوان غير معروف"))
                                  : s.title;
        auto* t = new QLabel(fmT.elidedText(title, Qt::ElideRight, 96), row);
        t->setFont(PixelTheme::pixelFont(8));
        t->setStyleSheet(QStringLiteral("color: #E8EAED;"));
        vcol->addWidget(t);

        QString sub = s.artist;
        if (sub.isEmpty())
            sub = I18n::t(QStringLiteral("Unknown artist"), QStringLiteral("فنان غير معروف"));
        auto* a = new QLabel(fmS.elidedText(sub, Qt::ElideRight, 96), row);
        a->setFont(PixelTheme::bodyFont(8));
        a->setStyleSheet(QStringLiteral("color: #9A9A9A;"));
        vcol->addWidget(a);
        hl->addLayout(vcol, 1);

        auto* d = new QLabel(fmtSec(s.durationSec), row);
        d->setFont(PixelTheme::pixelFont(8));
        d->setStyleSheet(QStringLiteral("color: #6F6F72;"));
        d->setFixedWidth(40);
        d->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        hl->addWidget(d);

        m_rows->addWidget(row);
    }
    m_rows->addStretch(1);
}

// ---------------- NowPlayingPage ----------------

// QLabel paints RTL-baseline text with its horizontal alignment MIRRORED
// physically (Arabic + AlignLeft renders on the right, AlignRight on the
// left). These helpers pin text to a chosen PHYSICAL side regardless.
static Qt::Alignment physLeft(const QString& t)
{
    return (t.isRightToLeft() ? Qt::AlignRight : Qt::AlignLeft) | Qt::AlignVCenter;
}
static Qt::Alignment physRight(const QString& t)
{
    return (t.isRightToLeft() ? Qt::AlignLeft : Qt::AlignRight) | Qt::AlignVCenter;
}

NowPlayingPage::NowPlayingPage(QWidget* parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("HardPanel"));

    // background pixel equalizer — created first so it stacks below all
    // layout widgets; geometry follows the page in resizeEvent
    m_viz = new VisualizerWidget(this);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(14, 10, 14, 10);
    root->setSpacing(10);

    // header row: "NOW PLAYING" + beat meter
    auto* headRow = new QHBoxLayout;
    m_header = new QLabel(this);
    m_header->setObjectName(QStringLiteral("SectionTitle"));
    m_header->setFont(PixelTheme::bodyFont(12));
    headRow->addWidget(m_header);
    headRow->addStretch(1);
    m_meter = new BeatMeter(this);
    headRow->addWidget(m_meter);
    root->addLayout(headRow);

    // ------ main row: player column + up-next queue ------
    auto* mainRow = new QHBoxLayout;
    mainRow->setSpacing(12);

    auto* leftCol = new QVBoxLayout;
    leftCol->setSpacing(8);

    m_artRow = new QWidget(this);
    m_cover = new CoverLabel(m_artRow);
    m_cover->setFixedSize(260, 260); // resized in resizeEvent

    // The artwork + info column live in one container (m_artRow) and are
    // pinned ABSOLUTELY inside it in resizeEvent — because mixed fixed-width
    // widgets + stretch columns in the same HBox/grid mis-place the stretch
    // column when the info column's minimum width is close to the available
    // space (12px overlap was observed with both layout types), and RTL
    // mirroring kept warping layout-managed artwork positions.
    m_infoBox = new QWidget(m_artRow);
    auto* infoCol = new QVBoxLayout(m_infoBox);
    infoCol->setContentsMargins(0, 0, 0, 0);
    infoCol->setSpacing(6);

    m_title = new QLabel(this);
    m_title->setFont(PixelTheme::pixelFont(14));
    m_title->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_title->setStyleSheet(QStringLiteral("color: #39FF14;"));
    m_title->setMinimumHeight(28);
    infoCol->addWidget(m_title);

    m_subtitle = new QLabel(this);
    m_subtitle->setFont(PixelTheme::bodyFont(10));
    m_subtitle->setStyleSheet(QStringLiteral("color: #9A9A9A;"));
    infoCol->addWidget(m_subtitle);

    m_meta = new QLabel(this);
    m_meta->setFont(PixelTheme::bodyFont(9));
    m_meta->setStyleSheet(QStringLiteral("color: #6F6F72;"));
    infoCol->addWidget(m_meta);

    // actions: favorite / download / more
    auto* actRow = new QHBoxLayout;
    actRow->setSpacing(8);
    m_favBtn = makePageButton(PixelIcons::heartDim(), 30, this);
    m_favBtn->setCheckable(true);
    m_dlBtn = makePageButton(PixelIcons::downloadDim(), 30, this);
    m_moreBtn = makePageButton(PixelIcons::moreDim(), 30, this);
    actRow->addWidget(m_favBtn);
    actRow->addWidget(m_dlBtn);
    actRow->addWidget(m_moreBtn);
    actRow->addStretch(1);
    infoCol->addLayout(actRow);

    // timeline: elapsed | slider | remaining
    m_seekRow = new QWidget(this);
    auto* seekLay = new QHBoxLayout(m_seekRow);
    seekLay->setContentsMargins(0, 0, 0, 0);
    seekLay->setSpacing(8);
    m_timeCur = new QLabel(QStringLiteral("0:00"), m_seekRow);
    m_timeCur->setFont(PixelTheme::pixelFont(10));
    m_timeCur->setStyleSheet(QStringLiteral("color: #9A9A9A;"));
    m_timeCur->setFixedWidth(44);
    m_timeCur->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    seekLay->addWidget(m_timeCur);

    m_seek = new PixelSlider(Qt::Horizontal, m_seekRow);
    m_seek->setRange(0, 1);
    m_seek->setMinimumWidth(96);
    m_seek->setEnabled(false);
    seekLay->addWidget(m_seek, 1);

    m_timeDur = new QLabel(QStringLiteral("-0:00"), m_seekRow);
    m_timeDur->setFont(PixelTheme::pixelFont(10));
    m_timeDur->setStyleSheet(QStringLiteral("color: #9A9A9A;"));
    m_timeDur->setFixedWidth(46);
    m_timeDur->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    seekLay->addWidget(m_timeDur);
    infoCol->addWidget(m_seekRow);

    // transport: strong play/pause between prev/next
    auto* transRow = new QHBoxLayout;
    transRow->setSpacing(16);
    transRow->addStretch(1);
    m_prevBtn = makePageButton(PixelIcons::prevDim(), 46, this);
    m_playBtn = makePageButton(PixelIcons::playRed(), 68, this);
    m_nextBtn = makePageButton(PixelIcons::nextDim(), 46, this);
    transRow->addWidget(m_prevBtn);
    transRow->addWidget(m_playBtn);
    transRow->addWidget(m_nextBtn);
    transRow->addStretch(1);
    infoCol->addLayout(transRow);

    // spectrum strip
    m_eq = new EQStrip(this);
    infoCol->addWidget(m_eq, 1);

    // modes + volume on one compact row
    auto* modeVol = new QHBoxLayout;
    modeVol->setSpacing(10);
    modeVol->addStretch(1);
    m_repeatBtn = makePageButton(PixelIcons::repeatDim(), 32, this);
    m_shuffleBtn = makePageButton(PixelIcons::shuffleDim(), 32, this);
    m_shuffleBtn->setCheckable(true);
    modeVol->addWidget(m_repeatBtn);
    modeVol->addWidget(m_shuffleBtn);
    modeVol->addSpacing(6);
    m_muteBtn = makePageButton(PixelIcons::volumeDim(), 32, this);
    m_muteBtn->setCheckable(true);
    modeVol->addWidget(m_muteBtn);
    m_volume = new PixelSlider(Qt::Horizontal, this);
    m_volume->setRange(0, 100);
    m_volume->setValue(70);
    m_volume->setFixedWidth(108);
    modeVol->addWidget(m_volume);
    m_volumePct = new QLabel(QStringLiteral("70%"), this);
    m_volumePct->setFont(PixelTheme::pixelFont(9));
    m_volumePct->setStyleSheet(QStringLiteral("color: #9A9A9A;"));
    m_volumePct->setFixedWidth(36);
    modeVol->addWidget(m_volumePct);
    modeVol->addStretch(1);
    infoCol->addLayout(modeVol);

    infoCol->addStretch(1); // keep the info block packed tight against the artwork

    leftCol->addWidget(m_artRow, 0, Qt::AlignTop); // art+info block, absolutely laid out inside

    // -- lyrics: small header + scroll panel (hidden when absent) --
    m_lyricsHeader = new QLabel(this);
    m_lyricsHeader->setObjectName(QStringLiteral("SectionTitle"));
    m_lyricsHeader->setFont(PixelTheme::bodyFont(9));
    m_lyricsHeader->setStyleSheet(QStringLiteral("color: #9A9A9A;"));
    m_lyricsHeader->hide();
    leftCol->addWidget(m_lyricsHeader);

    m_lyricsScroll = new QScrollArea(this);
    m_lyricsScroll->setObjectName(QStringLiteral("LyricsScroll"));
    m_lyricsScroll->setWidgetResizable(true);
    m_lyricsScroll->setFrameShape(QFrame::NoFrame);
    m_lyricsScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_lyricsScroll->setStyleSheet(QStringLiteral(
        "QScrollArea { background: #0B0B0C; border: 2px solid #2A2A2C; }"
        "QScrollBar:vertical { background: #0B0B0C; width: 10px; margin: 0; }"
        "QScrollBar::handle:vertical { background: #3F3F3F; min-height: 18px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: none; }"));
    m_lyrics = new QLabel(m_lyricsScroll);
    m_lyrics->setFont(PixelTheme::pixelFont(11));
    m_lyrics->setWordWrap(true);
    m_lyrics->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_lyrics->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_lyrics->setStyleSheet(QStringLiteral("background: transparent; color: #9A9A9A;"));
    m_lyricsScroll->setWidget(m_lyrics);
    m_lyricsScroll->setMinimumHeight(80);
    m_lyricsScroll->hide();
    leftCol->addWidget(m_lyricsScroll, 2);

    mainRow->addLayout(leftCol, 1);

    m_upnext = new UpNextPanel(this);
    mainRow->addWidget(m_upnext, 0);

    root->addLayout(mainRow, 1);

    // wiring
    connect(m_prevBtn, &QPushButton::clicked, this, &NowPlayingPage::prevRequested);
    connect(m_playBtn, &QPushButton::clicked, this, &NowPlayingPage::playPauseRequested);
    connect(m_nextBtn, &QPushButton::clicked, this, &NowPlayingPage::nextRequested);

    connect(m_seek, &PixelSlider::sliderPressed, this, [this] {
        m_dragging = true;
        updateTimeLabel();
    });
    connect(m_seek, &PixelSlider::sliderMoved, this, [this] { updateTimeLabel(); });
    connect(m_seek, &PixelSlider::sliderReleased, this, [this] {
        m_dragging = false;
        emit seekRequested(m_seek->value());
    });
    connect(m_seek, &PixelSlider::valueChanged, this, [this] { updateTimeLabel(); });

    connect(m_volume, &PixelSlider::valueChanged, this, [this](int v) {
        m_volumePct->setText(QString::number(v) + QStringLiteral("%"));
        emit volumeChanged(v);
    });
    connect(m_muteBtn, &QPushButton::clicked, this, &NowPlayingPage::muteRequested);
    connect(m_repeatBtn, &QPushButton::clicked, this, &NowPlayingPage::repeatRequested);
    connect(m_shuffleBtn, &QPushButton::toggled, this, [this](bool on) {
        m_shuffle = on;
        m_shuffleBtn->setIcon(on ? PixelIcons::shuffleGreen() : PixelIcons::shuffleDim());
    });
    connect(m_favBtn, &QPushButton::toggled, this, [this](bool on) {
        m_favorite = on;
        m_favBtn->setIcon(on ? PixelIcons::heartRed() : PixelIcons::heartDim());
    });

    m_viz->lower();
    retranslate();
}

void NowPlayingPage::resizeEvent(QResizeEvent* e)
{
    QWidget::resizeEvent(e);
    if (m_viz)
        m_viz->setGeometry(rect());
    if (!m_artRow)
        return;
    // The artwork + info column live in their own container, pinned in
    // PHYSICAL coordinates, so the composition is identical in both layout
    // directions — RTL mirroring only swaps the two outer page columns
    // (leftCol / UpNext), never the player block itself.
    // leftCol width is the same formula in both directions.
    const int uw = m_upnext ? m_upnext->width() : 232;
    const int leftColW = width() - 40 - uw;
    // Artwork dominates: ~h/2 wide, bounded, never crowded, and always
    // leaves room for the info column (min 220) to its right.
    int side = qBound(200, int(qMin(width() * 0.32, height() * 0.52)), 320);
    side = qMin(side, qMax(200, leftColW - 16 - 220));
    m_artRow->setFixedHeight(side);
    m_cover->setFixedSize(side, side);
    m_infoBox->setGeometry(side + 16, 0, qBound(220, leftColW - side - 16, 640), side);
    // The art block reserving its row changes the left column's children
    // (lyrics etc. below), so relayout synchronously instead of waiting for
    // the deferred LayoutRequest that may not run before painting/dumping.
    if (layout())
        layout()->activate();
}

void NowPlayingPage::applyDirection()
{
    // Alignments are text-direction aware (physLeft/physRight cover the
    // bidi mirroring), and the player block itself is never mirrored — only
    // the outer columns (leftCol / UpNext) flip with the layout direction.
    m_title->setAlignment(physLeft(m_title->text()));
    m_subtitle->setAlignment(physLeft(m_subtitle->text()));
    m_meta->setAlignment(physLeft(m_meta->text()));
    m_timeCur->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_timeDur->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
}

void NowPlayingPage::retranslate()
{
    m_header->setText(I18n::t(QStringLiteral("NOW PLAYING"), QStringLiteral("الآن يعمل")));
    m_prevBtn->setToolTip(I18n::t(QStringLiteral("Previous"), QStringLiteral("السابق")));
    m_playBtn->setToolTip(I18n::t(QStringLiteral("Play / Pause"), QStringLiteral("تشغيل / إيقاف مؤقت")));
    m_nextBtn->setToolTip(I18n::t(QStringLiteral("Next"), QStringLiteral("التالي")));
    m_favBtn->setToolTip(I18n::t(QStringLiteral("Favorite"), QStringLiteral("المفضلة")));
    m_dlBtn->setToolTip(I18n::t(QStringLiteral("Download"), QStringLiteral("تحميل")));
    m_moreBtn->setToolTip(I18n::t(QStringLiteral("More"), QStringLiteral("المزيد")));
    m_muteBtn->setToolTip(I18n::t(QStringLiteral("Mute"), QStringLiteral("كتم الصوت")));
    m_volume->setToolTip(I18n::t(QStringLiteral("Volume"), QStringLiteral("مستوى الصوت")));
    m_lyricsHeader->setText(I18n::t(QStringLiteral("- LYRICS"), QStringLiteral("- الكلمات")));
    if (m_upnext)
        m_upnext->retranslate();
    applyDirection();
    updateModes();
}

void NowPlayingPage::setSong(const Song& song, const QImage& cover, bool playing)
{
    m_cover->setImage(cover);

    static const QFontMetrics fmTitle(PixelTheme::pixelFont(14));
    static const QFontMetrics fmSub(PixelTheme::bodyFont(10));
    static const QFontMetrics fmMeta(PixelTheme::bodyFont(9));

    const QString title = song.title.isEmpty()
                              ? I18n::t(QStringLiteral("Unknown title"), QStringLiteral("عنوان غير معروف"))
                              : song.title;
    m_title->setText(fmTitle.elidedText(title, Qt::ElideRight, 420));
    m_title->setAlignment(physLeft(title));

    QString sub = song.artist;
    if (!song.album.isEmpty())
        sub = sub.isEmpty() ? song.album : sub + QStringLiteral("  —  ") + song.album;
    if (sub.isEmpty())
        sub = I18n::t(QStringLiteral("Unknown artist"), QStringLiteral("فنان غير معروف"));
    m_subtitle->setText(fmSub.elidedText(sub, Qt::ElideRight, 420));
    m_subtitle->setAlignment(physLeft(sub));

    QStringList meta;
    if (song.durationSec > 0)
        meta << fmtSec(song.durationSec);
    if (!song.genre.isEmpty())
        meta << song.genre.toUpper();
    if (song.year > 0)
        meta << QString::number(song.year);
    if (song.track > 0)
        meta << I18n::t(QStringLiteral("TRACK %1"), QStringLiteral("مقطع %1")).arg(song.track);
    const QString metaText = meta.join(QStringLiteral("  •  "));
    m_meta->setText(fmMeta.elidedText(metaText, Qt::ElideRight, 420));
    m_meta->setAlignment(physLeft(metaText));
    m_meta->setVisible(!metaText.isEmpty());

    // lyrics: show the panel only when the song actually has some
    m_lyricsData = song.lyrics;
    m_activeLine = -1;
    m_lyricsVisible = m_lyricsData.valid();
    m_lyricsHeader->setVisible(m_lyricsVisible);
    m_lyricsScroll->setVisible(m_lyricsVisible);
    if (m_lyricsVisible) {
        renderLyrics();
        if (qEnvironmentVariableIsSet("SADAUDIO_DEBUG")) {
            int synced = 0;
            for (const LyricsLine& l : m_lyricsData.lines)
                if (l.timeMs >= 0)
                    ++synced;
            fprintf(stderr, "[D] lyrics: %d lines (%d synced)\n", m_lyricsData.lines.size(), synced);
        }
    } else if (qEnvironmentVariableIsSet("SADAUDIO_DEBUG")) {
        fprintf(stderr, "[D] lyrics: none\n");
    }

    // reset per-song transient state
    m_favorite = false;
    m_favBtn->setChecked(false);
    m_favBtn->setIcon(PixelIcons::heartDim());

    setPlaying(playing);
}

void NowPlayingPage::setQueue(const QVector<Song>& songs, int currentRow)
{
    if (m_upnext)
        m_upnext->setQueue(songs, currentRow);
}

void NowPlayingPage::setPlaying(bool playing)
{
    m_playBtn->setIcon(playing ? PixelIcons::pauseGreen() : PixelIcons::playRed());
    m_meter->setPlaying(playing);
    if (m_viz)
        m_viz->setPlaying(playing);
    if (m_eq)
        m_eq->setPlaying(playing);
}

void NowPlayingPage::setPosition(int seconds, int durationSeconds)
{
    m_duration = durationSeconds;
    m_current = seconds;
    const int max = qMax(1, durationSeconds);
    if (m_seek->maximum() != max)
        m_seek->setRange(0, max);
    if (!m_dragging)
        m_seek->setValue(seconds);
    m_seek->setEnabled(durationSeconds > 0);

    // synced lyrics: highlight the active line as playback advances
    if (m_lyricsVisible) {
        const int ms = seconds * 1000;
        int idx = -1;
        const QVector<LyricsLine>& lines = m_lyricsData.lines;
        for (int i = 0; i < lines.size(); ++i)
            if (lines[i].timeMs >= 0 && lines[i].timeMs <= ms)
                idx = i;
        if (idx != m_activeLine && idx >= 0) {
            m_activeLine = idx;
            renderLyrics();
        }
    }

    updateTimeLabel();
}

void NowPlayingPage::renderLyrics()
{
    QStringList out;
    const QVector<LyricsLine>& lines = m_lyricsData.lines;
    for (int i = 0; i < lines.size(); ++i) {
        const QString text = lines[i].text.trimmed();
        if (text.isEmpty())
            continue;
        const bool active = m_activeLine >= 0 && i == m_activeLine;
        out << QStringLiteral("<span style='color:%1;'>%2</span>")
                   .arg(active ? QStringLiteral("#39FF14") : QStringLiteral("#9A9A9A"),
                        text.toHtmlEscaped());
    }
    m_lyrics->setText(out.join(QStringLiteral("<br/>")));
    bool rtlBlock = false;
    for (const LyricsLine& ln : lines)
        if (ln.text.isRightToLeft()) { rtlBlock = true; break; }
    // right-anchor the lyrics block physically in BOTH layout directions
    m_lyrics->setAlignment(Qt::AlignTop | (rtlBlock ? Qt::AlignLeft : Qt::AlignRight));

    // keep the active line roughly centered when scrolling
    if (m_activeLine >= 0 && lines.size() > 1) {
        if (QScrollBar* sb = m_lyricsScroll->verticalScrollBar()) {
            const double f = double(m_activeLine) / (lines.size() - 1);
            sb->setValue(qRound(f * sb->maximum()));
        }
    }
}

void NowPlayingPage::updateTimeLabel()
{
    m_timeCur->setText(fmtSec(m_dragging ? m_seek->value() : m_current));
    if (m_duration > 0 && m_current <= m_duration)
        m_timeDur->setText(QStringLiteral("-") + fmtSec(m_duration - m_current));
    else
        m_timeDur->setText(QStringLiteral("0:00"));
}

void NowPlayingPage::setVolume(int volume)
{
    m_volume->setValue(qBound(0, volume, 100));
    m_volumePct->setText(QString::number(m_volume->value()) + QStringLiteral("%"));
}

void NowPlayingPage::setMuted(bool on)
{
    m_muteBtn->setChecked(on);
    m_muteBtn->setIcon(on ? PixelIcons::muteRed() : PixelIcons::volumeDim());
}

void NowPlayingPage::setRepeatMode(NowPlayingBar::RepeatMode mode)
{
    m_repeat = mode;
    m_repeatBtn->setIcon(mode == NowPlayingBar::RepeatOff ? PixelIcons::repeatDim() : PixelIcons::repeatGreen());
    updateModes();
}

void NowPlayingPage::setShuffle(bool on)
{
    m_shuffle = on;
    m_shuffleBtn->setChecked(on);
}

void NowPlayingPage::updateModes()
{
    m_shuffleBtn->setChecked(m_shuffle);
    m_shuffleBtn->setIcon(m_shuffle ? PixelIcons::shuffleGreen() : PixelIcons::shuffleDim());
    m_repeatBtn->setToolTip(I18n::t(
        m_repeat == NowPlayingBar::RepeatOff ? QStringLiteral("Repeat: Off") : (m_repeat == NowPlayingBar::RepeatAll ? QStringLiteral("Repeat: All") : QStringLiteral("Repeat: One")),
        m_repeat == NowPlayingBar::RepeatOff ? QStringLiteral("التكرار: إيقاف") : (m_repeat == NowPlayingBar::RepeatAll ? QStringLiteral("التكرار: الكل") : QStringLiteral("التكرار: أغنية واحدة"))));
    m_shuffleBtn->setToolTip(I18n::t(QStringLiteral("Shuffle"), QStringLiteral("تشغيل عشوائي")));
}