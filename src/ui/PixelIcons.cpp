#include "ui/PixelIcons.h"

#include "ui/PixelTheme.h"

#include <QPainter>
#include <QPixmap>
#include <QPolygon>

namespace PixelIcons {

namespace {

QPixmap canvas()
{
    QPixmap pm(32, 32);
    pm.fill(Qt::transparent);
    return pm;
}

void setupPainter(QPainter& p, const QColor& c)
{
    p.setRenderHint(QPainter::Antialiasing, false);
    p.setPen(Qt::NoPen);
    p.setBrush(c);
}

QPolygon tri(int x0, int y0, int x1, int y1, int x2, int y2)
{
    QPolygon poly;
    poly << QPoint(x0, y0) << QPoint(x1, y1) << QPoint(x2, y2);
    return poly;
}

QIcon fromPixmap(const QPixmap& pm)
{
    return QIcon(pm);
}

QColor dim()
{
    return PixelTheme::colors().textDim;
}

} // namespace

QIcon play(const QColor& c)
{
    QPixmap pm = canvas();
    QPainter p(&pm); setupPainter(p, c);
    p.drawPolygon(tri(10, 7, 26, 16, 10, 25));
    return fromPixmap(pm);
}

QIcon pause(const QColor& c)
{
    QPixmap pm = canvas();
    QPainter p(&pm); setupPainter(p, c);
    p.drawRect(9, 7, 5, 18);
    p.drawRect(18, 7, 5, 18);
    return fromPixmap(pm);
}

QIcon next(const QColor& c)
{
    QPixmap pm = canvas();
    QPainter p(&pm); setupPainter(p, c);
    p.drawRect(8, 8, 4, 16);
    p.drawPolygon(tri(14, 8, 25, 16, 14, 24));
    return fromPixmap(pm);
}

QIcon prev(const QColor& c)
{
    QPixmap pm = canvas();
    QPainter p(&pm); setupPainter(p, c);
    p.drawRect(20, 8, 4, 16);
    p.drawPolygon(tri(18, 8, 7, 16, 18, 24));
    return fromPixmap(pm);
}

QIcon stop(const QColor& c)
{
    QPixmap pm = canvas();
    QPainter p(&pm); setupPainter(p, c);
    p.drawRect(8, 8, 16, 16);
    return fromPixmap(pm);
}

QIcon folder(const QColor& c)
{
    QPixmap pm = canvas();
    QPainter p(&pm); setupPainter(p, c);
    p.drawRect(4, 12, 24, 12);
    p.drawRect(4, 10, 10, 3);
    p.setBrush(dim());
    p.drawRect(4, 13, 24, 3); // flap
    return fromPixmap(pm);
}

QIcon music(const QColor& c)
{
    QPixmap pm = canvas();
    QPainter p(&pm); setupPainter(p, c);
    p.drawRect(10, 5, 3, 15);   // stem
    p.drawEllipse(7, 19, 8, 8); // head
    p.setBrush(Qt::NoBrush);
    QPen pen(c, 3);
    pen.setCosmetic(false);
    p.setPen(pen);
    p.drawLine(12, 5, 23, 8);   // flag
    p.drawLine(20, 8, 20, 14);
    return fromPixmap(pm);
}

QIcon eq(const QColor& c)
{
    QPixmap pm = canvas();
    QPainter p(&pm); setupPainter(p, c);
    p.drawRect(4, 14, 4, 10);
    p.drawRect(10, 8, 4, 16);
    p.drawRect(16, 12, 4, 12);
    p.drawRect(22, 5, 4, 19);
    return fromPixmap(pm);
}

QIcon repeat(const QColor& c)
{
    QPixmap pm = canvas();
    QPainter p(&pm); setupPainter(p, c);
    p.drawRect(6, 8, 20, 5);   // top bar
    p.drawPolygon(tri(24, 6, 24, 15, 28, 10));
    p.drawRect(6, 19, 20, 5);   // bottom bar
    p.drawPolygon(tri(8, 17, 8, 26, 4, 21));
    return fromPixmap(pm);
}

QIcon shuffle(const QColor& c)
{
    QPixmap pm = canvas();
    QPainter p(&pm); setupPainter(p, c);
    QPen pen(c, 3);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);
    p.drawLine(5, 7, 27, 25);
    p.setBrush(c);
    p.setPen(Qt::NoPen);
    p.drawPolygon(tri(24, 22, 27, 25, 22, 27));
    p.drawPolygon(tri(8, 10, 5, 7, 10, 5));
    p.drawLine(5, 25, 27, 7);
    p.drawPolygon(tri(24, 10, 27, 7, 22, 5));
    return fromPixmap(pm);
}

QIcon volume(const QColor& c)
{
    QPixmap pm = canvas();
    QPainter p(&pm); setupPainter(p, c);
    p.drawPolygon(tri(4, 13, 11, 8, 11, 24));
    p.drawPolygon(tri(11, 8, 16, 5, 16, 27));
    p.drawRect(11, 9, 4, 14);
    p.setBrush(Qt::NoBrush);
    QPen pen(c, 3);
    p.setPen(pen);
    p.drawArc(16, 9, 10, 10, -65 * 16, 130 * 16);
    p.drawArc(20, 4, 14, 14, -65 * 16, 130 * 16);
    return fromPixmap(pm);
}

QIcon mute(const QColor& c)
{
    QPixmap pm = canvas();
    QPainter p(&pm); setupPainter(p, c);
    p.setBrush(c);
    p.drawPolygon(tri(4, 13, 11, 8, 11, 24));
    p.drawRect(11, 9, 4, 14);
    QPen pen(c, 3);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);
    p.drawLine(17, 11, 26, 21);
    p.drawLine(26, 11, 17, 21);
    return fromPixmap(pm);
}

QIcon mini(const QColor& c)
{
    QPixmap pm = canvas();
    QPainter p(&pm); setupPainter(p, c);
    p.drawRect(4, 11, 11, 11);
    p.drawRect(17, 11, 11, 11);
    p.drawRect(4, 6, 24, 3);
    return fromPixmap(pm);
}

QIcon plus(const QColor& c)
{
    QPixmap pm = canvas();
    QPainter p(&pm); setupPainter(p, c);
    p.drawRect(14, 6, 4, 20);
    p.drawRect(6, 14, 20, 4);
    return fromPixmap(pm);
}

QIcon trash(const QColor& c)
{
    QPixmap pm = canvas();
    QPainter p(&pm); setupPainter(p, c);
    p.drawRect(8, 10, 16, 15);
    p.drawRect(10, 5, 12, 3);
    p.drawRect(6, 8, 20, 2);
    p.setBrush(Qt::NoBrush);
    QPen pen(PixelTheme::colors().bg, 2);
    p.setPen(pen);
    p.drawRect(8, 10, 16, 15);
    return fromPixmap(pm);
}

QIcon search(const QColor& c)
{
    QPixmap pm = canvas();
    QPainter p(&pm); setupPainter(p, c);
    p.setBrush(Qt::NoBrush);
    QPen pen(c, 4);
    p.setPen(pen);
    p.drawEllipse(9, 9, 12, 12);
    pen.setWidth(5);
    p.setPen(pen);
    p.drawLine(18, 18, 26, 26);
    return fromPixmap(pm);
}

QIcon lang(const QColor& c)
{
    QPixmap pm = canvas();
    QPainter p(&pm); setupPainter(p, c);
    p.setBrush(Qt::NoBrush);
    QPen pen(c, 3);
    p.setPen(pen);
    p.drawText(QRect(2, 3, 28, 26), Qt::AlignCenter, QStringLiteral("عA"));
    pen.setWidth(3);
    p.setPen(pen);
    p.drawEllipse(6, 6, 20, 20);
    return fromPixmap(pm);
}

QIcon heart(const QColor& c)
{
    QPixmap pm = canvas();
    QPainter p(&pm); setupPainter(p, c);
    p.drawRect(7, 8, 6, 6);   // left lobe
    p.drawRect(19, 8, 6, 6);  // right lobe
    p.drawPolygon(tri(5, 13, 27, 13, 16, 26)); // point
    return fromPixmap(pm);
}

QIcon download(const QColor& c)
{
    QPixmap pm = canvas();
    QPainter p(&pm); setupPainter(p, c);
    p.drawRect(15, 5, 2, 9);              // stem
    p.drawPolygon(tri(9, 12, 23, 12, 16, 20)); // arrow head
    p.drawRect(7, 22, 18, 4);             // tray
    return fromPixmap(pm);
}

QIcon more(const QColor& c)
{
    QPixmap pm = canvas();
    QPainter p(&pm); setupPainter(p, c);
    p.drawRect(6, 14, 4, 4);
    p.drawRect(14, 14, 4, 4);
    p.drawRect(22, 14, 4, 4);
    return fromPixmap(pm);
}

QIcon appLogo()
{
    const auto& C = PixelTheme::colors();
    QPixmap pm = canvas();
    QPainter p(&pm); setupPainter(p, C.bg);
    p.drawRect(1, 1, 30, 30);
    p.setBrush(C.green);
    p.drawRect(3, 3, 26, 26);
    p.setBrush(C.bg);
    p.drawRect(5, 5, 22, 22);
    p.setBrush(C.green);
    p.drawPolygon(tri(10, 11, 10, 21, 21, 16));
    p.setBrush(C.red);
    p.drawRect(21, 21, 6, 6);
    return fromPixmap(pm);
}

// ---- tinted convenience variants ----
#define SIMPLE(fn, cn) \
    QIcon fn() { return cn(PixelTheme::colors().textDim); }

QIcon playGreen() { return play(PixelTheme::colors().green); }
QIcon playRed() { return play(PixelTheme::colors().red); }
QIcon pauseGreen() { return pause(PixelTheme::colors().green); }
QIcon pauseRed() { return pause(PixelTheme::colors().red); }
QIcon playDim() { return play(dim()); }
QIcon pauseDim() { return pause(dim()); }
QIcon nextDim() { return next(dim()); }
QIcon prevDim() { return prev(dim()); }
SIMPLE(folderDim, folder)
SIMPLE(musicDim, music)
QIcon eqGreen() { return eq(PixelTheme::colors().green); }
SIMPLE(repeatDim, repeat)
QIcon repeatGreen() { return repeat(PixelTheme::colors().green); }
SIMPLE(shuffleDim, shuffle)
QIcon shuffleGreen() { return shuffle(PixelTheme::colors().green); }
SIMPLE(volumeDim, volume)
QIcon muteRed() { return mute(PixelTheme::colors().red); }
SIMPLE(miniDim, mini)
QIcon plusGreen() { return plus(PixelTheme::colors().green); }
QIcon trashRed() { return trash(PixelTheme::colors().red); }
SIMPLE(searchDim, search)
SIMPLE(langDim, lang)
QIcon heartRed() { return heart(PixelTheme::colors().red); }
SIMPLE(heartDim, heart)
SIMPLE(downloadDim, download)
SIMPLE(moreDim, more)

#undef SIMPLE

} // namespace PixelIcons