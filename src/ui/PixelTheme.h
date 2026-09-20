#pragma once

#include <QColor>
#include <QFont>
#include <QImage>
#include <QString>

class QApplication;

// Pixel-art theme: palette, fonts and the global style sheet.
namespace PixelTheme {

struct Colors {
    QColor bg{0x0B, 0x0B, 0x0C};          // main background
    QColor panel{0x16, 0x16, 0x17};       // panels / cards
    QColor panel2{0x1D, 0x1D, 0x1E};      // raised surfaces
    QColor border{0x2E, 0x2E, 0x2E};      // hard borders
    QColor borderLight{0x3F, 0x3F, 0x3F}; // highlight edges
    QColor text{0xE8, 0xEA, 0xED};        // primary text
    QColor textDim{0x9A, 0x9A, 0x9A};     // secondary text
    QColor green{0x39, 0xFF, 0x14};       // primary accent (live/interactive)
    QColor red{0xFF, 0x3B, 0x3B};         // playback state / warnings
    QColor yellow{0xFF, 0xD6, 0x0A};      // optional highlight
};

const Colors& colors();

// Registers bundled pixel fonts; returns true when at least one loaded.
bool registerFonts(QString* errorOut = nullptr);

// The big title font ("Press Start 2P" style) for headings / status.
QFont pixelFont(int pixelSize);
// The body font ("Silkscreen") for tables, labels, buttons.
QFont bodyFont(int pixelSize = 9);

// Applies Fusion base style + palette + the pixel-art stylesheet.
void apply(QApplication* app);

// Pixelates an image: downscale to low-res then upscale (transformation-free).
QImage pixelate(const QImage& src, int lowRes = 24);

} // namespace PixelTheme