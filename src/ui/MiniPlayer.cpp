#include "ui/MiniPlayer.h"

#include "ui/PixelIcons.h"
#include "ui/PixelTheme.h"

#include <QFontMetrics>
#include <QMouseEvent>
#include <QPainter>
#include <QTimer>

MiniPlayer::MiniPlayer(QWidget* parent)
    : QWidget(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground, false);
    setFixedSize(340, 58);
    setCursor(Qt::PointingHandCursor);
}

void MiniPlayer::setSong(const QString& title, const QString& subtitle, const QPixmap& cover)
{
    m_title = title;
    m_subtitle = subtitle;
    m_cover = cover;
    update();
}

void MiniPlayer::setPlaying(bool playing)
{
    m_playing = playing;
    update();
}

QRect MiniPlayer::buttonGeometry(Button btn) const
{
    const int h = 26;
    const int y = (height() - h) / 2;
    switch (btn) {
    case BtnPrev:
        return QRect(width() - 106, y, h, h);
    case BtnPlay:
        return QRect(width() - 76, y, h + 4, h);
    case BtnNext:
        return QRect(width() - 44, y, h, h);
    default:
        return {};
    }
}

void MiniPlayer::paintEvent(QPaintEvent*)
{
    const auto& C = PixelTheme::colors();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, false);

    // frame
    p.fillRect(rect(), C.panel);
    p.setPen(QPen(C.green, 2));
    p.drawRect(rect().adjusted(1, 1, -1, -1));

    // cover
    const QRect coverRect(6, 7, 44, 44);
    if (!m_cover.isNull()) {
        p.drawPixmap(coverRect, m_cover);
    } else {
        p.fillRect(coverRect, C.bg);
        p.drawPixmap(coverRect.adjusted(8, 8, -8, -8), PixelIcons::musicDim().pixmap(28, 28));
    }
    p.setPen(QPen(C.borderLight, 1));
    p.drawRect(coverRect.adjusted(0, 0, -1, -1));

    // texts
    const QRect textRect(58, 8, width() - 58 - 112, 22);
    static const QFontMetrics fmTitle(PixelTheme::pixelFont(9));
    static const QFontMetrics fmSub(PixelTheme::bodyFont(8));
    p.setPen(C.green);
    p.setFont(PixelTheme::pixelFont(9));
    p.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, fmTitle.elidedText(m_title, Qt::ElideRight, textRect.width()));
    p.setPen(C.textDim);
    p.setFont(PixelTheme::bodyFont(8));
    p.drawText(QRect(textRect.left(), 32, textRect.width(), 18), Qt::AlignLeft | Qt::AlignVCenter,
               fmSub.elidedText(m_subtitle, Qt::ElideRight, textRect.width()));

    paintButtons(p);
}

void MiniPlayer::paintButtons(QPainter& p)
{
    const auto& C = PixelTheme::colors();
    const QIcon icons[BtnCount] = {
        PixelIcons::prevDim(),
        m_playing ? PixelIcons::pauseGreen() : PixelIcons::playGreen(),
        PixelIcons::nextDim(),
    };
    for (int i = 0; i < BtnCount; ++i) {
        const QRect r = buttonGeometry(Button(i));
        p.setPen(QPen(C.borderLight, 2));
        p.setBrush(C.panel2);
        p.drawRect(r);
        const QPixmap px = icons[i].pixmap(r.width() - 8, r.height() - 8);
        p.drawPixmap(r.center() - QPoint(px.width() / 2, px.height() / 2), px);
    }
}

void MiniPlayer::mousePressEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton) {
        m_dragOffset = e->globalPosition().toPoint() - frameGeometry().topLeft();
        m_dragging = true;
    }
}

void MiniPlayer::mouseMoveEvent(QMouseEvent* e)
{
    if (m_dragging)
        move(e->globalPosition().toPoint() - m_dragOffset);
}

void MiniPlayer::mouseReleaseEvent(QMouseEvent* e)
{
    if (!m_dragging)
        return;
    m_dragging = false;
    const QPoint pos = e->pos();
    for (int i = 0; i < BtnCount; ++i) {
        if (buttonGeometry(Button(i)).contains(pos)) {
            switch (Button(i)) {
            case BtnPrev:
                emit prevRequested();
                break;
            case BtnPlay:
                emit playPauseRequested();
                break;
            case BtnNext:
                emit nextRequested();
                break;
            default:
                break;
            }
            return;
        }
    }
}

void MiniPlayer::mouseDoubleClickEvent(QMouseEvent*)
{
    // avoid expanding when double-clicking a button
    emit expandRequested();
}

void MiniPlayer::wheelEvent(QWheelEvent* e)
{
    m_volume = qBound(0, m_volume + (e->angleDelta().y() > 0 ? 5 : -5), 100);
    emit volumeChanged(m_volume);
    update();
    e->accept();
}