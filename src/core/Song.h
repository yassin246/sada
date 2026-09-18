#pragma once

#include <QMetaType>
#include <QString>
#include <QVector>

// One parsed lyrics line. timeMs < 0 means the line has no timestamp
// (plain-text lyrics / unsynced USLT). timeMs >= 0 means synced (LRC).
struct LyricsLine {
    int timeMs = -1;
    QString text;
};

// Lyrics attached to a song: either plain text (unsynchronized tag lyrics or
// an untimed .lrc sidecar) or timestamped lines for synced highlighting.
struct Lyrics {
    QString plain;              // full lyrics text, timestamps stripped
    QVector<LyricsLine> lines;  // parsed lines (timed or untimed)
    bool valid() const { return !plain.trimmed().isEmpty(); }
};

// A single audio track with its metadata (tags).
struct Song {
    QString path;
    QString title;
    QString artist;
    QString album;
    QString genre;
    int year = 0;
    int track = 0;
    int durationSec = 0;
    Lyrics lyrics;

    bool valid() const { return !path.isEmpty(); }
};

Q_DECLARE_METATYPE(Song)
Q_DECLARE_METATYPE(QVector<Song>)