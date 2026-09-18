#pragma once

#include "core/Song.h"

#include <QAbstractTableModel>
#include <QStringList>
#include <QVector>

// Table model (Title, Artist, Album, Duration) over a list of songs.
// Supports: m3u save/load, drag&drop of files/folders, internal reordering,
// current track highlight.
class PlaylistModel : public QAbstractTableModel {
    Q_OBJECT
public:
    enum Column { ColTitle = 0, ColArtist, ColAlbum, ColDuration, ColumnCount };

    explicit PlaylistModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = {}) const override;
    int columnCount(const QModelIndex& parent = {}) const override { return ColumnCount; }
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    // drag & drop (external files + internal moves)
    Qt::DropActions supportedDropActions() const override;
    QStringList mimeTypes() const override;
    QMimeData* mimeData(const QModelIndexList& indexes) const override;
    bool canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column,
                         const QModelIndex& parent) const override;
    bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column,
                      const QModelIndex& parent) override;
    bool moveRows(const QModelIndex& sourceParent, int sourceRow, int count, const QModelIndex& destinationParent,
                  int destinationChild) override;
    bool insertRows(int row, int count, const QModelIndex& parent = {}) override;
    bool removeRows(int row, int count, const QModelIndex& parent = {}) override;

    void setSongs(const QVector<Song>& songs);
    void appendSongs(const QVector<Song>& songs);
    void appendSong(const Song& song);
    void clear() { setSongs({}); }
    Song songAt(int row) const { return (row >= 0 && row < songs_.size()) ? songs_.at(row) : Song(); }
    QVector<Song> songs() const { return songs_; }

    void addFiles(const QStringList& paths);
    bool saveM3u(const QString& file) const;
    bool loadM3u(const QString& file);

    int currentRow() const { return currentRow_; }
    void setCurrentRow(int row);

signals:
    void currentRowChanged(int row);

private:
    QVector<Song> songs_;
    int currentRow_ = -1;
};