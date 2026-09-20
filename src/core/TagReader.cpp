#include "core/TagReader.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>

#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/audioproperties.h>
#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/attachedpictureframe.h>
#include <taglib/flacfile.h>
#include <taglib/flacpicture.h>
#include <taglib/mp4file.h>
#include <taglib/mp4tag.h>
#include <taglib/mp4coverart.h>
#include <taglib/unsynchronizedlyricsframe.h>
#include <taglib/vorbisfile.h>
#include <taglib/opusfile.h>
#include <taglib/xiphcomment.h>

namespace TagReader {

namespace {

QString ts(const TagLib::String& s)
{
    if (s.isEmpty())
        return {};
    return QString::fromStdString(s.to8Bit(true));
}

TagLib::FileName fileName(const QString& path)
{
    const QByteArray enc = QFile::encodeName(path);
    return TagLib::FileName(enc.constData());
}

QImage loadImage(const TagLib::ByteVector& data)
{
    if (data.isEmpty())
        return {};
    return QImage::fromData(reinterpret_cast<const uchar*>(data.data()), data.size());
}

QImage coverFromXiph(TagLib::Ogg::XiphComment* tag)
{
    if (!tag)
        return {};
    const auto pictures = tag->pictureList();
    for (const TagLib::FLAC::Picture* pic : pictures) {
        QImage img = loadImage(pic->data());
        if (!img.isNull())
            return img;
    }
    return {};
}

// "mm:ss" / "mm:ss.xx" / "mm:ss.xxx" -> milliseconds, or -1 if not a time.
int lrcTimeMs(const QString& s)
{
    const int colon = s.indexOf(QLatin1Char(':'));
    if (colon <= 0)
        return -1;
    bool okM = false;
    bool okS = false;
    const int m = s.left(colon).toInt(&okM);
    const QString secPart = s.mid(colon + 1);
    const int dot = secPart.indexOf(QLatin1Char('.'));
    const QString secStr = (dot >= 0) ? secPart.left(dot) : secPart;
    const int sec = secStr.toInt(&okS);
    if (!okM || !okS || m < 0 || sec < 0)
        return -1;
    int ms = (m * 60 + sec) * 1000;
    if (dot >= 0) {
        QString frac = secPart.mid(dot + 1);
        if (frac.size() > 3)
            frac = frac.left(3);
        while (frac.size() < 3)
            frac += QLatin1Char('0');
        ms += frac.toInt();
    }
    return ms;
}

// Parses a .lrc sidecar: "[mm:ss.xx]text" lines, multiple timestamps per
// line, "[ar:..]"/"[ti:..]" metadata lines, and untimed plain lines.
Lyrics lyricsFromLrc(const QString& lrcPath)
{
    Lyrics out;
    QFile f(lrcPath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return out;

    QVector<LyricsLine> ordered;  // keeps file order for mixed timed/untimed
    QStringList plainLines;
    bool anyTimed = false;

    while (!f.atEnd()) {
        const QString raw = QString::fromUtf8(f.readLine()).trimmed();
        if (raw.isEmpty())
            continue;

        QString rest = raw;
        QVector<int> times;
        while (rest.startsWith(QLatin1Char('['))) {
            const int close = rest.indexOf(QLatin1Char(']'));
            if (close < 0)
                break;
            const QString inner = rest.mid(1, close - 1);
            rest = rest.mid(close + 1).trimmed();
            const int ms = lrcTimeMs(inner);
            if (ms >= 0)
                times.append(ms);  // "[ar:..]" etc. (non-time) are ignored
        }

        if (times.isEmpty()) {
            if (!rest.isEmpty()) {
                plainLines << rest;
                ordered.append({ -1, rest });
            }
            continue;
        }
        anyTimed = true;
        for (int t : times)
            ordered.append({ t, rest });
    }

    if (anyTimed) {
        out.lines = ordered;
        QStringList all;
        for (const LyricsLine& l : out.lines)
            all << l.text;
        out.plain = all.join(QLatin1Char('\n'));
    } else {
        out.lines.clear();
        for (const QString& l : plainLines)
            out.lines.append({ -1, l });
        out.plain = plainLines.join(QLatin1Char('\n'));
    }
    return out;
}

// Xiph (FLAC / Ogg Vorbis / Opus) store lyrics in the "LYRICS" field.
QString xiphLyrics(const TagLib::Ogg::XiphComment* xc)
{
    if (!xc)
        return {};
    const TagLib::Ogg::FieldListMap& map = xc->fieldListMap();
    const auto it = map.find("LYRICS");
    if (it != map.end() && !it->second.isEmpty())
        return ts(it->second.front());
    return {};
}

// Lyrics resolution order:
//   1. ".lrc" sidecar next to the audio file (usually synced timestamps)
//   2. embedded tags: MP3 USLT (unsynchronized lyrics), Xiph "LYRICS" field
Lyrics readLyrics(const QString& path)
{
    const QFileInfo fi(path);
    const QString lrc = fi.dir().filePath(fi.completeBaseName() + QStringLiteral(".lrc"));
    if (QFileInfo::exists(lrc))
        return lyricsFromLrc(lrc);

    QString text;
    const QByteArray ext = fi.suffix().toLower().toUtf8();

    if (ext == "mp3") {
        TagLib::MPEG::File file(fileName(path));
        if (file.isValid() && file.isOpen()) {
            if (const TagLib::ID3v2::Tag* id3 = file.ID3v2Tag(false)) {
                for (const TagLib::ID3v2::Frame* frame : id3->frameList()) {
                    const auto* uslt = dynamic_cast<const TagLib::ID3v2::UnsynchronizedLyricsFrame*>(frame);
                    if (!uslt)
                        continue;
                    const TagLib::String txt = uslt->text();
                    if (!txt.isEmpty()) {
                        text = ts(txt).trimmed();
                        break;
                    }
                }
            }
        }
    } else if (ext == "flac") {
        TagLib::FLAC::File file(fileName(path));
        if (file.isValid() && file.isOpen())
            text = xiphLyrics(file.xiphComment());
    } else if (ext == "ogg") {
        TagLib::Ogg::Vorbis::File file(fileName(path));
        if (file.isValid() && file.isOpen())
            text = xiphLyrics(file.tag());
    } else if (ext == "opus") {
        TagLib::Ogg::Opus::File file(fileName(path));
        if (file.isValid() && file.isOpen())
            text = xiphLyrics(file.tag());
    }

    if (!text.isEmpty()) {
        Lyrics out;
        out.plain = text;
        const QStringList lines = out.plain.split(QLatin1Char('\n'));
        for (const QString& l : lines)
            out.lines.append({ -1, l });
        return out;
    }

    return {};
}

} // namespace

Song readSong(const QString& path)
{
    Song s;
    s.path = path;

    TagLib::FileRef ref(fileName(path), true);
    if (!ref.isNull()) {
        if (TagLib::Tag* tag = ref.tag()) {
            s.title = ts(tag->title()).trimmed();
            s.artist = ts(tag->artist()).trimmed();
            s.album = ts(tag->album()).trimmed();
            s.genre = ts(tag->genre()).trimmed();
            s.year = tag->year();
            s.track = tag->track();
        }
        if (TagLib::AudioProperties* ap = ref.audioProperties())
            s.durationSec = ap->lengthInSeconds();
        s.lyrics = readLyrics(path);
    }

    if (s.title.isEmpty())
        s.title = QFileInfo(path).completeBaseName();
    return s;
}

QImage coverImage(const QString& path)
{
    const QByteArray ext = QFileInfo(path).suffix().toLower().toUtf8();

    // MP3: ID3v2 APIC frames
    if (ext == "mp3") {
        TagLib::MPEG::File file(fileName(path));
        if (file.isValid() && file.isOpen()) {
            if (const TagLib::ID3v2::Tag* tag = file.ID3v2Tag(false)) {
                for (const TagLib::ID3v2::Frame* frame : tag->frameList()) {
                    const auto* apic = dynamic_cast<const TagLib::ID3v2::AttachedPictureFrame*>(frame);
                    if (!apic)
                        continue;
                    QImage img = loadImage(apic->picture());
                    if (!img.isNull())
                        return img;
                }
            }
        }
    }

    // FLAC: native picture blocks
    if (ext == "flac") {
        TagLib::FLAC::File file(fileName(path));
        if (file.isValid() && file.isOpen()) {
            for (const TagLib::FLAC::Picture* pic : file.pictureList()) {
                QImage img = loadImage(pic->data());
                if (!img.isNull())
                    return img;
            }
        }
    }

    // MP4/M4A: covr atom
    if (ext == "m4a" || ext == "mp4" || ext == "m4b") {
        TagLib::MP4::File file(fileName(path));
        if (file.isValid() && file.isOpen()) {
            if (TagLib::MP4::Tag* tag = file.tag()) {
                const TagLib::MP4::ItemMap& items = tag->itemMap();
                const auto it = items.find("covr");
                if (it != items.end()) {
                    const TagLib::MP4::CoverArtList list = it->second.toCoverArtList();
                    for (const TagLib::MP4::CoverArt& art : list) {
                        QImage img = loadImage(art.data());
                        if (!img.isNull())
                            return img;
                    }
                }
            }
        }
    }

    // Ogg Vorbis / Opus: METADATA_BLOCK_PICTURE
    if (ext == "ogg") {
        TagLib::Ogg::Vorbis::File file(fileName(path));
        if (file.isValid() && file.isOpen()) {
            QImage img = coverFromXiph(file.tag());
            if (!img.isNull())
                return img;
        }
    }
    if (ext == "opus") {
        TagLib::Ogg::Opus::File file(fileName(path));
        if (file.isValid() && file.isOpen()) {
            QImage img = coverFromXiph(file.tag());
            if (!img.isNull())
                return img;
        }
    }

    return {};
}

} // namespace TagReader