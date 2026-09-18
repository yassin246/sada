#include "model/LibraryProxy.h"

#include "model/PlaylistModel.h"

LibraryProxy::LibraryProxy(QObject* parent)
    : QSortFilterProxyModel(parent)
{
    setFilterCaseSensitivity(Qt::CaseInsensitive);
    setSortCaseSensitivity(Qt::CaseInsensitive);
    setDynamicSortFilter(true);
}

void LibraryProxy::setFilterText(const QString& text)
{
    text_ = text.trimmed();
    invalidateFilter();
}

void LibraryProxy::setArtistFilter(const QString& artist)
{
    artist_ = artist.trimmed();
    invalidateFilter();
}

bool LibraryProxy::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    const auto* model = qobject_cast<const PlaylistModel*>(sourceModel());
    if (!model)
        return false;

    const Song song = model->songAt(sourceRow);

    if (!artist_.isEmpty() && song.artist.compare(artist_, Qt::CaseInsensitive) != 0)
        return false;

    if (text_.isEmpty())
        return true;

    return song.title.contains(text_, Qt::CaseInsensitive) || song.artist.contains(text_, Qt::CaseInsensitive)
        || song.album.contains(text_, Qt::CaseInsensitive) || song.genre.contains(text_, Qt::CaseInsensitive);
}

bool LibraryProxy::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
    const auto* model = qobject_cast<const PlaylistModel*>(sourceModel());
    if (!model)
        return QSortFilterProxyModel::lessThan(left, right);

    // Sort by real duration when the TIME column is used.
    if (left.column() == PlaylistModel::ColDuration) {
        const int l = model->songAt(left.row()).durationSec;
        const int r = model->songAt(right.row()).durationSec;
        if (l != r)
            return l < r;
        return QSortFilterProxyModel::lessThan(left, right);
    }
    return QSortFilterProxyModel::lessThan(left, right);
}