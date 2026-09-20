#pragma once

#include "core/Song.h"

#include <QLabel>
#include <QTimer>
#include <QWidget>

class PixelSlider;
class QMouseEvent;
class QPushButton;

// Decorative reactive beat meter: three columns of pixel blocks.
class BeatMeter : public QWidget {
    Q_OBJECT
public:
    explicit BeatMeter(QWidget* parent = nullptr);
    void setPlaying(bool playing);

protected:
    void paintEvent(QPaintEvent*) override;

private:
    QTimer m_timer;
    bool m_playing = false;
    double m_level[3] = { 0.0, 0.0, 0.0 };
    double m_target[3] = { 0.0, 0.0, 0.0 };
    int m_phase = 0;
};

// Bottom bar: cover art, now-playing info, transport controls, seek + volume.
class NowPlayingBar : public QWidget {
    Q_OBJECT
public:
    enum RepeatMode { RepeatOff = 0, RepeatAll, RepeatOne };

    explicit NowPlayingBar(QWidget* parent = nullptr);

    void setSong(const Song& song, const QPixmap& cover, bool playing);
    void setPlaying(bool playing);
    void setPosition(int seconds, int durationSeconds);
    void setVolume(int volume);
    void setMuted(bool on);
    void setRepeatMode(RepeatMode mode);
    void setShuffle(bool on);
    void retranslate();

signals:
    void playPauseRequested();
    void prevRequested();
    void nextRequested();
    void seekPreview(int seconds);
    void seekRequested(int seconds);
    void volumeChanged(int volume);
    void muteRequested();
    void equalizerRequested();
    void repeatRequested();
    void shuffleRequested();
    void miniRequested();
    void fullPageRequested();

protected:
    // Clicking the cover / title opens the full now-playing page.
    void mousePressEvent(QMouseEvent*) override;

private slots:
    void updateTimeLabel();

private:
    QLabel* m_cover = nullptr;
    QLabel* m_title = nullptr;
    QLabel* m_subtitle = nullptr;
    QWidget* m_textWrap = nullptr;
    QPushButton* m_prevBtn = nullptr;
    QPushButton* m_playBtn = nullptr;
    QPushButton* m_nextBtn = nullptr;
    PixelSlider* m_seek = nullptr;
    QLabel* m_time = nullptr;
    PixelSlider* m_volume = nullptr;
    QPushButton* m_muteBtn = nullptr;
    QPushButton* m_eqBtn = nullptr;
    QPushButton* m_repeatBtn = nullptr;
    QPushButton* m_shuffleBtn = nullptr;
    QPushButton* m_miniBtn = nullptr;

    RepeatMode m_repeat = RepeatOff;
    bool m_shuffle = false;
    bool m_dragging = false;
    int m_duration = 0;
    int m_current = 0;
};