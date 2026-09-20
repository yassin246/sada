#include "ui/Toast.h"

#include "ui/PixelIcons.h"
#include "ui/PixelTheme.h"

#include <QFontMetrics>
#include <QGuiApplication>
#include <QPainter>
#include <QPropertyAnimation>
#include <QScreen>
#include <QTimer>

Toast::Toast()
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::ToolTip);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(336, 74);

    m_fade = new QPropertyAnimation(this, "windowOpacity", this);
    m_fade->setDuration(220);
    m_fade->setStartValue(0.0);
    m_fade->setEndValue(1.0);

    m_hideTimer = new QTimer(this);
    m_hideTimer->setSingleShot(true);
    m_hideTimer->setInterval(2600);
    connect(m_hideTimer, &QTimer::timeout, this, [this] {
        QPropertyAnimation* out = new QPropertyAnimation(this, "windowOpacity", this);
        out->setDuration(300);
        out->setStartValue(windowOpacity());
        out->setEndValue(0.0);
        connect(out, &QPropertyAnimation::finished, this, &QWidget::hide);
        out->start(QAbstractAnimation::DeleteWhenStopped);
    });
}

void Toast::showToast(const QString& title, const QString& subtitle, const QPixmap& cover)
{
    m_title = title;
    m_subtitle = subtitle;
    m_cover = cover;

    if (QScreen* screen = QGuiApplication::primaryScreen()) {
        const QRect avail = screen->availableGeometry();
        move(avail.right() - width() - 20, avail.bottom() - height() - 64);
    }

    show();
    raise();
    m_fade->stop();
    m_fade->setStartValue(windowOpacity());
    m_fade->setEndValue(1.0);
    m_fade->start();
    m_hideTimer->start();
    update();
}

void Toast::paintEvent(QPaintEvent*)
{
    const auto& C = PixelTheme::colors();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, false);

    p.fillRect(rect(), QColor(0x16, 0x16, 0x17, 245));
    p.setPen(QPen(C.green, 2));
    p.drawRect(rect().adjusted(0, 0, -1, -1));

    const QRect coverRect(10, 11, 52, 52);
    if (!m_cover.isNull()) {
        p.drawPixmap(coverRect, m_cover);
    } else {
        p.fillRect(coverRect, C.bg);
        p.drawPixmap(coverRect.adjusted(10, 10, -10, -10), PixelIcons::musicDim().pixmap(32, 32));
    }

    const QRect textRect(72, 12, width() - 84, 26);
    static const QFontMetrics fmTitle(PixelTheme::pixelFont(9));
    static const QFontMetrics fmSub(PixelTheme::bodyFont(8));
    p.setPen(C.green);
    p.setFont(PixelTheme::pixelFont(9));
    p.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, fmTitle.elidedText(m_title, Qt::ElideRight, textRect.width()));
    p.setPen(C.textDim);
    p.setFont(PixelTheme::bodyFont(8));
    p.drawText(QRect(textRect.left(), 40, textRect.width(), 20), Qt::AlignLeft | Qt::AlignVCenter,
               fmSub.elidedText(m_subtitle, Qt::ElideRight, textRect.width()));
}