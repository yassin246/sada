#include "ui/Sidebar.h"

#include "core/I18n.h"
#include "ui/PixelIcons.h"
#include "ui/PixelTheme.h"

#include <QDir>
#include <QFileSystemModel>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QTreeView>
#include <QTreeWidget>
#include <QVBoxLayout>

Sidebar::Sidebar(QWidget* parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("HardPanel"));
    setFixedWidth(232);

    auto* vl = new QVBoxLayout(this);
    vl->setContentsMargins(8, 8, 8, 8);
    vl->setSpacing(6);

    // Folders section
    m_folderTitle = new QLabel(this);
    m_folderTitle->setObjectName(QStringLiteral("SectionTitle"));
    vl->addWidget(m_folderTitle);

    m_fsModel = new QFileSystemModel(this);
    m_fsModel->setRootPath(QDir::homePath());
    m_fsModel->setNameFilterDisables(false);
    m_fsModel->setNameFilters({ QStringLiteral("*.mp3"), QStringLiteral("*.flac"), QStringLiteral("*.ogg"),
                                QStringLiteral("*.opus"), QStringLiteral("*.m4a"), QStringLiteral("*.mp4"),
                                QStringLiteral("*.aac"), QStringLiteral("*.wav"), QStringLiteral("*.wma") });

    m_folderView = new QTreeView(this);
    m_folderView->setModel(m_fsModel);
    m_folderView->setRootIndex(m_fsModel->index(QDir::homePath()));
    for (int c = 1; c < m_fsModel->columnCount(); ++c)
        m_folderView->hideColumn(c);
    m_folderView->setHeaderHidden(true);
    m_folderView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_folderView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_folderView->setIndentation(14);
    vl->addWidget(m_folderView, 2);

    // Artist section
    m_artistTitle = new QLabel(this);
    m_artistTitle->setObjectName(QStringLiteral("SectionTitle"));
    vl->addWidget(m_artistTitle);

    m_artistTree = new QTreeWidget(this);
    m_artistTree->setHeaderHidden(true);
    m_artistTree->setIndentation(14);
    vl->addWidget(m_artistTree, 3);

    // Actions row
    auto* row = new QHBoxLayout;
    auto* browse = new QPushButton(this);
    browse->setIcon(PixelIcons::folderDim());
    browse->setObjectName(QStringLiteral("IconButton"));
    browse->setFixedSize(30, 30);
    browse->setToolTip(I18n::t(QStringLiteral("Browse folder…"), QStringLiteral("تصفح مجلد…")));
    auto* rescan = new QPushButton(this);
    rescan->setIcon(PixelIcons::repeatDim());
    rescan->setObjectName(QStringLiteral("IconButton"));
    rescan->setFixedSize(30, 30);
    rescan->setToolTip(I18n::t(QStringLiteral("Rescan library"), QStringLiteral("إعادة مسح المكتبة")));
    m_scanLabel = new QLabel(QStringLiteral(" "), this);
    m_scanLabel->setFont(PixelTheme::bodyFont(8));
    m_scanLabel->setStyleSheet(QStringLiteral("color: #9A9A9A;"));
    row->addWidget(browse);
    row->addWidget(rescan);
    row->addWidget(m_scanLabel, 1);
    vl->addLayout(row);

    connect(m_folderView, &QTreeView::doubleClicked, this, [this](const QModelIndex& idx) {
        const QString path = m_fsModel->filePath(idx);
        emit folderActivated(path);
    });
    connect(browse, &QPushButton::clicked, this, &Sidebar::browseFolderRequested);
    connect(rescan, &QPushButton::clicked, this, &Sidebar::rescanRequested);

    connect(m_artistTree, &QTreeWidget::itemDoubleClicked, this, [this](QTreeWidgetItem* item, int col) {
        Q_UNUSED(col);
        if (!item)
            return;
        if (item->data(0, Qt::UserRole).toString() == QStringLiteral("__ALL__")) {
            emit clearArtistRequested();
            return;
        }
        QString artist = item->data(0, Qt::UserRole).toString();
        if (artist.isEmpty())
            artist = item->parent() ? item->parent()->text(0) : item->text(0);
        emit artistActivated(artist);
    });

    retranslate();
}

void Sidebar::retranslate()
{
    m_folderTitle->setText(I18n::t(QStringLiteral("FOLDERS"), QStringLiteral("المجلدات")));
    m_artistTitle->setText(I18n::t(QStringLiteral("ARTISTS"), QStringLiteral("الفنانون")));
    rebuildArtistTree();
}

void Sidebar::setDatabaseSongs(const QVector<Song>& songs)
{
    m_songs = songs;
    rebuildArtistTree();
}

void Sidebar::setScanning(bool on, int scanned, int total)
{
    if (!on) {
        m_scanLabel->setText(QStringLiteral(" "));
        return;
    }
    m_scanLabel->setText(total > 0 ? QStringLiteral("%1 / %2").arg(scanned).arg(total) : QStringLiteral("…"));
}

void Sidebar::rebuildArtistTree()
{
    m_artistTree->clear();

    auto* all = new QTreeWidgetItem(m_artistTree);
    all->setText(0, I18n::t(QStringLiteral("ALL"), QStringLiteral("الكل")));
    all->setData(0, Qt::UserRole, QStringLiteral("__ALL__"));
    all->setForeground(0, PixelTheme::colors().green);

    QMap<QString, QMap<QString, int>> artists; // artist -> album -> count
    for (const Song& s : m_songs) {
        const QString artist = s.artist.isEmpty() ? I18n::t(QStringLiteral("Unknown"), QStringLiteral("مجهول")) : s.artist;
        const QString album = s.album.isEmpty() ? QStringLiteral("…") : s.album;
        artists[artist][album]++;
    }

    for (auto it = artists.begin(); it != artists.end(); ++it) {
        auto* artistItem = new QTreeWidgetItem(m_artistTree);
        const QMap<QString, int> albums = it.value();
        int total = 0;
        for (int cnt : albums)
            total += cnt;
        artistItem->setText(0, QStringLiteral("%1 (%2)").arg(it.key()).arg(total));
        artistItem->setData(0, Qt::UserRole, it.key());
        for (auto ait = albums.begin(); ait != albums.end(); ++ait) {
            auto* albumItem = new QTreeWidgetItem(artistItem);
            albumItem->setText(0, ait.key());
            albumItem->setData(0, Qt::UserRole, it.key());
        }
        artistItem->setExpanded(true);
    }
}