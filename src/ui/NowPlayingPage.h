#pragma once

#include "core/Song.h"
#include "ui/NowPlayingBar.h" // NowPlayingBar::RepeatMode + BeatMeter

#include <QLabel>
#include <QTimer>
#include <QVector>
#include <QWidget>

class PixelSlider;
class QImage;
class QMouseEvent;
class QPushButton;
class QScrollArea;
class QVBoxLayout;
class VisualizerWidget;

// Full-quality cover widget: paints the original artwork scaled to the widget
// with smooth scaling — no pixelation, no downscale of the source image.
class CoverLabel : public QWidget {
    Q_OBJECT
public:
    explicit CoverLabel(QWidget* parent = nullptr);
    void setImage(const QImage& image);

protected:
    void paintEvent(QPaintEvent*) override;

private:
    QPixmap m_pixmap;
};

// Decorative spectrum strip: a wide row of green pixel bars with a red cap on
// each lit bar. Mirrors the identity of BeatMeter / VisualizerWidget (random
// walk — no real audio spectrum) so the page stays functional-looking.
class EQStrip : public QWidget {
    Q_OBJECT
public:
    explicit EQStrip(QWidget* parent = nullptr);
    void setPlaying(bool playing);

protected:
    void paintEvent(QPaintEvent*) override;

private:
    QTimer m_timer;
    bool m_playing = false;
    int m_phase = 0;
    QVector<double> m_level;
    QVector<double> m_target;
};

// "Up Next" queue panel: upcoming tracks with a cover thumbnail, title /
// artist and the track duration, in a bordered scroll column on the right.
class UpNextPanel : public QWidget {
    Q_OBJECT
public:
    explicit UpNextPanel(QWidget* parent = nullptr);
    void setQueue(const QVector<Song>& songs, int currentRow);
    void retranslate();

private:
    void rebuild();

    QLabel* m_header = nullptr;
    QScrollArea* m_scroll = nullptr;
    QWidget* m_host = nullptr;
    QVBoxLayout* m_rows = nullptr;
    QVector<Song> m_songs;
    int m_currentRow = -1;
    static constexpr int kMaxRows = 7;
};

// Spotify-style full now-playing page: dominant artwork with metadata and
// actions beside it, seek timeline with remaining time, strong transport,
// EQ strip, repeat/shuffle/volume, an "Up Next" queue column and synced
// lyrics — all on the pixel theme.
class NowPlayingPage : public QWidget {
    Q_OBJECT
public:
    explicit NowPlayingPage(QWidget* parent = nullptr);

    void setSong(const Song& song, const QImage& cover, bool playing);
    void setQueue(const QVector<Song>& songs, int currentRow);
    void setPlaying(bool playing);
    void setPosition(int seconds, int durationSeconds);
    void setVolume(int volume);
    void setMuted(bool on);
    void setRepeatMode(NowPlayingBar::RepeatMode mode);
    void setShuffle(bool on);
    void retranslate();

signals:
    void playPauseRequested();
    void prevRequested();
    void nextRequested();
    void seekRequested(int seconds);
    void volumeChanged(int volume);
    void muteRequested();
    void repeatRequested();
    void shuffleRequested();

protected:
    void resizeEvent(QResizeEvent*) override;

private:
    void renderLyrics();
    void updateTimeLabel();
    void updateModes();
    void applyDirection();

    CoverLabel* m_cover = nullptr;
    QWidget* m_artRow = nullptr; // owns cover + info column, laid out absolutely
    QWidget* m_infoBox = nullptr;
    QLabel* m_header = nullptr;
    QLabel* m_title = nullptr;
    QLabel* m_subtitle = nullptr;
    QLabel* m_meta = nullptr;
    QLabel* m_timeCur = nullptr;
    QLabel* m_timeDur = nullptr; // remaining time ("-m:ss")
    QWidget* m_seekRow = nullptr;
    PixelSlider* m_seek = nullptr;
    QPushButton* m_prevBtn = nullptr;
    QPushButton* m_playBtn = nullptr;
    QPushButton* m_nextBtn = nullptr;
    QPushButton* m_favBtn = nullptr;
    QPushButton* m_dlBtn = nullptr;
    QPushButton* m_moreBtn = nullptr;
    QPushButton* m_repeatBtn = nullptr;
    QPushButton* m_shuffleBtn = nullptr;
    QPushButton* m_muteBtn = nullptr;
    PixelSlider* m_volume = nullptr;
    QLabel* m_volumePct = nullptr;
    BeatMeter* m_meter = nullptr;
    VisualizerWidget* m_viz = nullptr;
    EQStrip* m_eq = nullptr;
    UpNextPanel* m_upnext = nullptr;
    QLabel* m_lyricsHeader = nullptr;
    QScrollArea* m_lyricsScroll = nullptr;
    QLabel* m_lyrics = nullptr;
    Lyrics m_lyricsData;
    int m_activeLine = -1;
    bool m_lyricsVisible = false;

    NowPlayingBar::RepeatMode m_repeat = NowPlayingBar::RepeatAll;
    bool m_shuffle = false;
    bool m_dragging = false;
    bool m_favorite = false;
    int m_duration = 0;
    int m_current = 0;
};