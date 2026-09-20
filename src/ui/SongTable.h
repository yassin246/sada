#pragma once

#include <QTableView>

// Themed table view used for both the queue and the library.
class SongTable : public QTableView {
    Q_OBJECT
public:
    explicit SongTable(bool allowReorder, QWidget* parent = nullptr);

    void scrollToRow(int row);
};