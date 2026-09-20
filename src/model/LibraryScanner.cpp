#include "model/LibraryScanner.h"

#include "core/TagReader.h"

#include <QDirIterator>
#include <QElapsedTimer>
#include <QFileInfo>

LibraryScanner::LibraryScanner(QObject* parent)
    : QThread(parent)
{
}

void LibraryScanner::startScan(const QString& root)
{
    if (isRunning()) {
        requestStop();
        wait();
    }
    m_stop.storeRelease(0);
    m_root = root;
    start();
}

void LibraryScanner::run()
{
    QVector<Song> batch;
    QStringList files;

    {
        QDirIterator it(m_root, QDir::Files | QDir::NoDotAndDotDot,
                        QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);
        while (it.hasNext()) {
            if (m_stop.loadAcquire())
                return;
            const QString f = it.next();
            const QString ext = QFileInfo(f).suffix().toLower();
            static const QStringList exts = { "mp3", "flac", "ogg", "opus", "m4a", "mp4", "m4b", "aac", "wav", "wma" };
            if (exts.contains(ext))
                files << f;
        }
        files.sort(Qt::CaseInsensitive);
    }

    const int total = files.size();
    for (int i = 0; i < total; ++i) {
        if (m_stop.loadAcquire())
            return;
        batch << TagReader::readSong(files.at(i));
        emit progressChanged(i + 1, total);
        if (batch.size() >= 20) {
            emit songsFound(batch);
            batch.clear();
        }
    }

    if (!batch.isEmpty())
        emit songsFound(batch);
    emit scanFinished(total);
}