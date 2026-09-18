#pragma once

#include "core/Song.h"

#include <QAtomicInt>
#include <QThread>

// Scans a directory tree for audio files and reads their tags in a worker
// thread, emitting batches so the UI stays responsive.
class LibraryScanner : public QThread {
    Q_OBJECT
public:
    explicit LibraryScanner(QObject* parent = nullptr);

    void startScan(const QString& root);
    void requestStop() { m_stop.storeRelease(1); }

signals:
    void songsFound(const QVector<Song>& songs);
    void progressChanged(int scanned, int total);
    void scanFinished(int totalSongs);

protected:
    void run() override;

private:
    QString m_root;
    QAtomicInt m_stop{ 0 };
};