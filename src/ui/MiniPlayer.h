#pragma once

#include <QPixmap>
#include <QString>
#include <QWidget>

// Frameless always-on-top mini strip with cover, title and transport.
// Drag to move; wheel changes volume; double click restores the window.
class MiniPlayer : public QWidget {
    Q_OBJECT
public:
    explicit MiniPlayer(QWidget* parent = nullptr);

    void setSong(const QString& title, const QString& subtitle, const QPixmap& cover);
    void setPlaying(bool playing);
    void setVolume(int volume) { m_volume = volume; update(); }

signals:
    void playPauseRequested();
    void nextRequested();
    void prevRequested();
    void expandRequested();
    void volumeChanged(int volume);

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void mouseDoubleClickEvent(QMouseEvent*) override;
    void wheelEvent(QWheelEvent*) override;

private:
    enum Button { BtnPrev = 0, BtnPlay, BtnNext, BtnCount };
    QRect buttonGeometry(Button btn) const;
    void paintButtons(QPainter& p);

    QString m_title;
    QString m_subtitle;
    QPixmap m_cover;
    bool m_playing = false;
    int m_volume = 70;
    QPoint m_dragOffset;
    bool m_dragging = false;
};