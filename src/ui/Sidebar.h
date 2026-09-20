#pragma once

#include "core/Song.h"

#include <QVector>
#include <QWidget>

class QFileSystemModel;
class QLabel;
class QTreeView;
class QTreeWidget;

// Left panel: file-system browser + artist/album tree of the scanned library.
class Sidebar : public QWidget {
    Q_OBJECT
public:
    explicit Sidebar(QWidget* parent = nullptr);

    void setDatabaseSongs(const QVector<Song>& songs);
    void setScanning(bool on, int scanned = 0, int total = 0);
    void retranslate();

signals:
    void folderActivated(const QString& path);
    void artistActivated(const QString& artist);
    void clearArtistRequested();
    void browseFolderRequested();
    void rescanRequested();

private:
    void rebuildArtistTree();

    QFileSystemModel* m_fsModel = nullptr;
    QTreeView* m_folderView = nullptr;
    QLabel* m_folderTitle = nullptr;
    QLabel* m_artistTitle = nullptr;
    QTreeWidget* m_artistTree = nullptr;
    QLabel* m_scanLabel = nullptr;
    QVector<Song> m_songs;
};