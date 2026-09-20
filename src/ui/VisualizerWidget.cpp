#include "ui/VisualizerWidget.h"

#include "ui/PixelTheme.h"

#include <QPainter>
#include <QRandomGenerator>
#include <QtMath>

namespace {
constexpr int kCell = 10;   // block height in px
constexpr int kGap = 2;     // block gap in px
constexpr double kEase = 0.28;
} // namespace

VisualizerWidget::VisualizerWidget(QWidget* parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("VizBG"));
    setAttribute(Qt::WA_TransparentForMouseEvents); // never steal clicks
    m_timer.setInterval(70);
    connect(&m_timer, &QTimer::timeout, this, &VisualizerWidget::tick);
    m_timer.start();
}

void VisualizerWidget::setPlaying(bool playing)
{
    m_playing = playing;
    if (!playing)
        m_level.fill(0.0);
}

int VisualizerWidget::barCount() const
{
    // bottom-up: chunky bars, ~26px per bar incl. gap
    return qMax(12, width() / 26);
}

void VisualizerWidget::tick()
{
    const int n = barCount();
    if (m_level.size() != n) {
        m_level.fill(0.0, n);
        m_target.fill(1.0, n);
    }

    ++m_phase;
    const int maxCells = qMax(3, int(height() * 0.45) / kCell);

    for (int i = 0; i < n; ++i) {
        double t = m_target[i];
        if (m_playing) {
            // organic random walk: wander up/down, occasionally jump
            const double r = QRandomGenerator::global()->generateDouble();
            if (r < 0.45) {
                t = qMin(double(maxCells), t + 1.0);
            } else if (r < 0.80) {
                t = qMax(1.0, t - 1.0);
            } else {
                t = 1.0 + QRandomGenerator::global()->bounded(maxCells);
            }
            // sine "breathing" keeps the whole bank moving together
            const double wave = 0.55 + 0.45 * qAbs(qSin(m_phase * 0.09 + i * 0.7));
            t = qBound(1.0, double(t * wave), double(maxCells));
        } else {
            t = qMax(0.0, t - 0.35); // idle decay
        }
        m_target[i] = t;
        m_level[i] += (t - m_level[i]) * kEase;
    }
    update();
}

void VisualizerWidget::paintEvent(QPaintEvent*)
{
    const auto& C = PixelTheme::colors();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, false);

    const int n = barCount();
    if (n <= 0 || height() <= 0)
        return;

    const int bw = qMax(5, width() / n - kGap);
    const int total = n * bw + (n - 1) * kGap;
    const int x0 = (width() - total) / 2;

    const QColor green = C.green;                     // #39FF14
    QColor greenDim = C.green;
    greenDim.setAlpha(70);
    const QColor red = C.red;
    QColor idle = C.border;                           // dark dim blocks
    idle.setAlpha(90);

    for (int i = 0; i < n && i < m_level.size(); ++i) {
        const int cells = qBound(0, int(qRound(m_level[i])), height() / kCell);
        const int x = x0 + i * (bw + kGap);
        for (int c = 0; c < cells; ++c) {
            const QRect r(x, height() - (c + 1) * kCell, bw, kCell - 1);
            if (m_playing) {
                const bool isPeak = c == cells - 1 && cells >= qMax(3, height() / kCell / 2);
                p.fillRect(r, isPeak ? red : greenDim);
            } else {
                p.fillRect(r, idle);
            }
        }
    }
}