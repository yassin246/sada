#pragma once

#include "ui/PixelTheme.h"

#include <QMouseEvent>
#include <QPainter>
#include <QWidget>
#include <QtMath>

// A chunky pixel-art slider. The track is drawn as square segments, the
// handle is a hard-shadowed block. Works horizontal and vertical.
//
// The segments and the handle share ONE geometry model (see Geom), so the
// pointer always sits exactly at the end of the filled blocks — no more
// floating past the last block on wide sliders.
class PixelSlider : public QWidget {
    Q_OBJECT
public:
    explicit PixelSlider(Qt::Orientation orientation = Qt::Horizontal, QWidget* parent = nullptr)
        : QWidget(parent)
        , m_orientation(orientation)
    {
        setMinimumSize(orientation == Qt::Horizontal ? QSize(100, 26) : QSize(26, 100));
        setMouseTracking(true);
        setFocusPolicy(Qt::NoFocus);
    }

    void setRange(int min, int max)
    {
        if (min > max)
            qSwap(min, max);
        m_min = min;
        m_max = max;
        setValue(m_value);
        update();
    }
    void setMinimum(int v) { setRange(v, m_max); }
    void setMaximum(int v) { setRange(m_min, v); }
    int minimum() const { return m_min; }
    int maximum() const { return m_max; }

    void setValue(int v)
    {
        v = qBound(m_min, m_max, v);
        if (v == m_value)
            return;
        m_value = v;
        update();
        emit valueChanged(m_value);
    }
    int value() const { return m_value; }

    void setSegments(int n) { m_segments = qMax(4, n); update(); }

signals:
    void valueChanged(int value);
    void sliderPressed();
    void sliderMoved(int value);
    void sliderReleased();

protected:
    void paintEvent(QPaintEvent*) override
    {
        const auto& C = PixelTheme::colors();
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, false);
        p.fillRect(rect(), C.bg);

        const Geom g = geometry();
        const bool h = m_orientation == Qt::Horizontal;
        const double frac = double(m_value - m_min) / qMax(1, m_max - m_min);
        const int filled = qRound(frac * g.n);

        // segment blocks (anchored at the low side: left for horizontal,
        // bottom for vertical)
        for (int i = 0; i < g.n; ++i) {
            const bool on = i < filled;
            QRect block;
            if (h)
                block = QRect(g.track.left() + g.off + i * g.pitch, g.track.top() + 1, g.seg, g.track.height() - 2);
            else
                block = QRect(g.track.left() + 1, g.track.bottom() - g.off - (i + 1) * g.pitch + 2, g.track.width() - 2, g.seg);
            p.fillRect(block, on ? C.green : C.border);
        }

        // handle: square block with hard shadow, centered on the end of the
        // filled run (same axis coordinate as the block grid)
        const int d = qBound(0, filled * g.pitch - 2, g.used);
        const int hs = 16;
        QRect handle;
        if (h)
            handle = QRect(g.track.left() + g.off + d - hs / 2, g.track.center().y() - hs / 2, hs, hs);
        else
            handle = QRect(g.track.center().x() - hs / 2, g.track.bottom() - g.off - d - hs / 2, hs, hs);
        p.fillRect(handle.translated(3, 3), QColor(0, 0, 0, 160));
        p.fillRect(handle, C.panel2);
        p.setPen(QPen(C.green, 2));
        p.drawRect(handle.adjusted(1, 1, -1, -1));
    }

    void mousePressEvent(QMouseEvent* e) override
    {
        if (e->button() == Qt::LeftButton) {
            m_pressed = true;
            setValueFromPos(e->pos());
            emit sliderPressed();
        }
        QWidget::mousePressEvent(e);
    }

    void mouseMoveEvent(QMouseEvent* e) override
    {
        if (m_pressed) {
            setValueFromPos(e->pos());
            emit sliderMoved(m_value);
        }
        QWidget::mouseMoveEvent(e);
    }

    void mouseReleaseEvent(QMouseEvent* e) override
    {
        if (m_pressed) {
            m_pressed = false;
            emit sliderReleased();
        }
        QWidget::mouseReleaseEvent(e);
    }

    void wheelEvent(QWheelEvent* e) override
    {
        const int step = qMax(1, (m_max - m_min) / 100);
        setValue(m_value + (e->angleDelta().y() > 0 ? step : -step));
        e->accept();
    }

private:
    QRect sliderArea() const
    {
        if (m_orientation == Qt::Horizontal)
            return rect().adjusted(12, 2, -12, -2);
        return rect().adjusted(2, 12, -2, -12);
    }

    // One shared geometry for painting AND input mapping, so the pointer is
    // always on the filled blocks.
    struct Geom {
        QRect track;
        int n = 0;     // number of blocks
        int pitch = 0; // block advance along the axis
        int seg = 0;   // visible block size (pitch - gap)
        int used = 0;  // total grid length along the axis
        int off = 0;   // centering offset inside the track
    };

    Geom geometry() const
    {
        Geom g;
        const QRect area = sliderArea();
        if (m_orientation == Qt::Horizontal)
            g.track = QRect(area.left(), area.center().y() - 5, area.width(), 10);
        else
            g.track = QRect(area.center().x() - 5, area.top(), 10, area.height());
        const int span = (m_orientation == Qt::Horizontal) ? g.track.width() : g.track.height();

        // count the 8px blocks (+2px gap each) needed to cover the span, and
        // honor segment requests as long as blocks stay chunky (>= 6px pitch)
        const int natural = qMax(4, (span + 2) / 10);
        g.n = qMin(qMax(m_segments, natural), qMax(4, span / 6));
        if (g.n <= 0)
            g.n = 4;
        g.pitch = qMax(6, span / g.n);
        g.seg = g.pitch - 2;
        g.used = g.n * g.pitch;
        g.off = qMax(0, (span - g.used) / 2);
        return g;
    }

    void setValueFromPos(const QPoint& pos)
    {
        const Geom g = geometry();
        const double span = qMax(1, g.used);
        double frac = 0.0;
        if (m_orientation == Qt::Horizontal)
            frac = double(pos.x() - (g.track.left() + g.off)) / span;
        else
            frac = double((g.track.bottom() - g.off) - pos.y()) / span;
        frac = qBound(0.0, frac, 1.0);
        setValue(m_min + qRound(frac * (m_max - m_min)));
    }

    Qt::Orientation m_orientation = Qt::Horizontal;
    int m_min = 0;
    int m_max = 100;
    int m_value = 0;
    int m_segments = 40;
    bool m_pressed = false;
};