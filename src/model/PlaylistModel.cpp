#include "model/PlaylistModel.h"

#include "core/I18n.h"
#include "core/TagReader.h"
#include "ui/PixelTheme.h"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QFont>
#include <QMimeData>
#include <QSaveFile>
#include <QStandardPaths>
#include <QTextStream>
#include <QUrl>

namespace {

bool isAudioFile(const QString& path)
{
    static const QStringList exts = { "mp3", "flac", "ogg", "opus", "m4a", "mp4", "m4b", "aac", "wav", "wma", "mpc", "ape" };
    return exts.contains(QFileInfo(path).suffix().toLower());
}

QString fmtDuration(int totalSec)
{
    if (totalSec <= 0)
        return QStringLiteral("--:--");
    const int m = totalSec / 60;
    const int s = totalSec % 60;
    return QStringLiteral("%1:%2").arg(m).arg(s, 2, 10, QLatin1Char('0'));
}

} // namespace

PlaylistModel::PlaylistModel(QObject* parent)
    : QAbstractTableModel(parent)
{
}

int PlaylistModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : songs_.size();
}

QVariant PlaylistModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= songs_.size())
        return {};

    const Song& song = songs_.at(index.row());
    const bool isCurrent = index.row() == currentRow_;
    const auto& C = PixelTheme::colors();

    switch (role) {
    case Qt::DisplayRole:
        switch (index.column()) {
        case ColTitle:
            return isCurrent ? QStringLiteral("▶ ") + song.title : song.title;
        case ColArtist:
            return song.artist;
        case ColAlbum:
            return song.album;
        case ColDuration:
            return fmtDuration(song.durationSec);
        }
        return {};
    case Qt::ForegroundRole:
        if (isCurrent)
            return C.green;
        if (index.column() == ColDuration)
            return C.textDim;
        return C.text;
    case Qt::FontRole: {
        static const QFont f = PixelTheme::bodyFont(9);
        return f;
    }
    case Qt::ToolTipRole:
        return index.column() == ColTitle ? song.path : QVariant();
    case Qt::TextAlignmentRole:
        if (index.column() == ColDuration)
            return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
    default:
        return {};
    }
}

QVariant PlaylistModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return {};
    static const QString names[ColumnCount][2] = {
        { QStringLiteral("TRACK"), QStringLiteral("الأغنية") },
        { QStringLiteral("ARTIST"), QStringLiteral("الفنان") },
        { QStringLiteral("ALBUM"), QStringLiteral("الألبوم") },
        { QStringLiteral("TIME"), QStringLiteral("الزمن") },
    };
    if (section < 0 || section >= ColumnCount)
        return {};
    return I18n::t(names[section][0], names[section][1]);
}

Qt::ItemFlags PlaylistModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::ItemIsDropEnabled;
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

Qt::DropActions PlaylistModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

QStringList PlaylistModel::mimeTypes() const
{
    return { QStringLiteral("text/uri-list"), QStringLiteral("application/x-qabstractitemmodeldatalist") };
}

QMimeData* PlaylistModel::mimeData(const QModelIndexList& indexes) const
{
    return QAbstractTableModel::mimeData(indexes);
}

bool PlaylistModel::canDropMimeData(const QMimeData* data, Qt::DropAction, int, int, const QModelIndex&) const
{
    return data && (data->hasUrls() || data->hasFormat(QStringLiteral("application/x-qabstractitemmodeldatalist")));
}

bool PlaylistModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column,
                                 const QModelIndex& parent)
{
    if (data && data->hasUrls()) {
        QStringList paths;
        for (const QUrl& url : data->urls())
            if (url.isLocalFile())
                paths << url.toLocalFile();
        if (!paths.isEmpty()) {
            addFiles(paths);
            return true;
        }
        return false;
    }
    // internal reordering handled by the base implementation
    return QAbstractTableModel::dropMimeData(data, action, row, column, parent);
}

bool PlaylistModel::moveRows(const QModelIndex& sourceParent, int sourceRow, int count,
                             const QModelIndex& destinationParent, int destinationChild)
{
    if (sourceParent.isValid() || destinationParent.isValid())
        return false;
    if (sourceRow < 0 || count <= 0 || sourceRow + count > songs_.size())
        return false;
    if (destinationChild < 0 || destinationChild > songs_.size())
        return false;
    const int sourceLast = sourceRow + count - 1;
    if (destinationChild > sourceLast)
        destinationChild -= count;
    if (destinationChild == sourceRow)
        return false;

    beginMoveRows(QModelIndex(), sourceRow, sourceLast, QModelIndex(), destinationChild + count);
    const QVector<Song> moved = songs_.mid(sourceRow, count);
    songs_.remove(sourceRow, count);
    for (int i = 0; i < moved.size(); ++i)
        songs_.insert(destinationChild + i, moved.at(i));
    // keep the current-row pointer roughly in place
    if (currentRow_ >= sourceRow && currentRow_ < sourceLast + 1) {
        currentRow_ = destinationChild + (currentRow_ - sourceRow);
    } else if (currentRow_ > sourceLast) {
        currentRow_ -= count;
    }
    endMoveRows();
    return true;
}

bool PlaylistModel::insertRows(int row, int count, const QModelIndex& parent)
{
    if (parent.isValid() || row < 0 || row > songs_.size() || count <= 0)
        return false;
    beginInsertRows(QModelIndex(), row, row + count - 1);
    for (int i = 0; i < count; ++i)
        songs_.insert(row, Song());
    endInsertRows();
    return true;
}

bool PlaylistModel::removeRows(int row, int count, const QModelIndex& parent)
{
    if (parent.isValid() || row < 0 || count <= 0 || row + count > songs_.size())
        return false;
    beginRemoveRows(QModelIndex(), row, row + count - 1);
    songs_.remove(row, count);
    if (currentRow_ >= row) {
        if (currentRow_ < row + count)
            currentRow_ = -1;
        else
            currentRow_ -= count;
    }
    endRemoveRows();
    emit currentRowChanged(currentRow_);
    return true;
}

void PlaylistModel::setSongs(const QVector<Song>& songs)
{
    beginResetModel();
    songs_ = songs;
    currentRow_ = -1;
    endResetModel();
    emit currentRowChanged(-1);
}

void PlaylistModel::appendSongs(const QVector<Song>& songs)
{
    if (songs.isEmpty())
        return;
    const int start = songs_.size();
    beginInsertRows(QModelIndex(), start, start + songs.size() - 1);
    songs_ += songs;
    endInsertRows();
}

void PlaylistModel::appendSong(const Song& song)
{
    appendSongs({ song });
}

void PlaylistModel::setCurrentRow(int row)
{
    row = qBound(-1, row, songs_.size() - 1);
    if (row == currentRow_)
        return;
    const int old = currentRow_;
    currentRow_ = row;
    if (old >= 0 && old < songs_.size())
        emit dataChanged(index(old, 0), index(old, ColumnCount - 1));
    if (row >= 0 && row < songs_.size())
        emit dataChanged(index(row, 0), index(row, ColumnCount - 1));
    emit currentRowChanged(currentRow_);
}

void PlaylistModel::addFiles(const QStringList& paths)
{
    for (const QString& path : paths) {
        const QFileInfo fi(path);
        if (fi.isDir()) {
            QStringList files;
            QDirIterator it(path, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);
            while (it.hasNext()) {
                const QString f = it.next();
                if (isAudioFile(f))
                    files << f;
            }
            files.sort(Qt::CaseInsensitive);
            for (const QString& f : files)
                appendSong(TagReader::readSong(f));
        } else if (fi.isFile() && isAudioFile(path)) {
            appendSong(TagReader::readSong(path));
        }
    }
}

bool PlaylistModel::saveM3u(const QString& file) const
{
    if (songs_.isEmpty())
        return false;
    QSaveFile out(file);
    if (!out.open(QIODevice::WriteOnly))
        return false;
    QTextStream ts(&out);
    ts.setEncoding(QStringConverter::Utf8);
    ts << "#EXTM3U\n";
    for (const Song& s : songs_) {
        ts << "#EXTINF:" << s.durationSec << "," << s.title << "\n";
        ts << s.path << "\n";
    }
    return out.commit();
}

bool PlaylistModel::loadM3u(const QString& file)
{
    QFile in(file);
    if (!in.open(QIODevice::ReadOnly))
        return false;
    QTextStream ts(&in);
    ts.setEncoding(QStringConverter::Utf8);

    const QDir base = QFileInfo(file).absoluteDir();
    while (!ts.atEnd()) {
        QString line = ts.readLine().trimmed();
        if (line.isEmpty() || line.startsWith(QLatin1Char('#')))
            continue;
        QFileInfo fi(line);
        const QString resolved = fi.isAbsolute() ? line : base.absoluteFilePath(line);
        if (isAudioFile(resolved))
            appendSong(TagReader::readSong(resolved));
    }
    return true;
}