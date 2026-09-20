#include "ui/SongTable.h"

#include "ui/PixelTheme.h"

#include <QHeaderView>

SongTable::SongTable(bool allowReorder, QWidget* parent)
    : QTableView(parent)
{
    setObjectName(QStringLiteral("HardPanel"));
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setShowGrid(false);
    setAlternatingRowColors(true);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setWordWrap(false);

    horizontalHeader()->setStretchLastSection(false);
    horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    horizontalHeader()->setMinimumSectionSize(60);
    verticalHeader()->setVisible(false);
    verticalHeader()->setDefaultSectionSize(24);

    setDragDropMode(allowReorder ? QAbstractItemView::InternalMove : QAbstractItemView::DragDrop);
    setDefaultDropAction(Qt::MoveAction);
    setDropIndicatorShown(true);
    setAcceptDrops(true);
}

void SongTable::scrollToRow(int row)
{
    if (row >= 0)
        scrollTo(model()->index(row, 0), QAbstractItemView::PositionAtCenter);
}