#pragma once

#include <QSortFilterProxyModel>
#include <QString>

// Filters the library table by free text and/or a fixed artist name.
class LibraryProxy : public QSortFilterProxyModel {
    Q_OBJECT
public:
    explicit LibraryProxy(QObject* parent = nullptr);

    void setFilterText(const QString& text);
    void setArtistFilter(const QString& artist);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;
    bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;

private:
    QString text_;
    QString artist_;
};