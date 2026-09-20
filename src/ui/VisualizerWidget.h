#pragma once

#include <QTimer>
#include <QWidget>

// Full-page background pixel-art equalizer for the now-playing page. Bars are
// bottom-anchored, "dance" while the song plays (smooth pseudo-random walk)
// and settle to a dim idle level when paused. Purely decorative — mpv has no
// per-band spectrum exposed here, so it is driven by the beat-walk logic used
// in BeatMeter. Painted behind the page controls (objectName "VizBG").
class VisualizerWidget : public QWidget {
    Q_OBJECT
public:
    explicit VisualizerWidget(QWidget* parent = nullptr);
    void setPlaying(bool playing);

protected:
    void paintEvent(QPaintEvent*) override;

private:
    void tick();
    int barCount() const;

    QTimer m_timer;
    bool m_playing = false;
    int m_phase = 0;
    QVector<double> m_level;
    QVector<double> m_target;
};