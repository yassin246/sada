#include "ui/PixelTheme.h"

#include <QApplication>
#include <QFontDatabase>
#include <QPalette>
#include <QStyleFactory>

namespace PixelTheme {

static Colors s_colors;

const Colors& colors() { return s_colors; }

bool registerFonts(QString* errorOut)
{
    int loaded = 0;
    const auto load = [&](const char* res) {
        const int id = QFontDatabase::addApplicationFont(QLatin1String(res));
        if (id >= 0)
            ++loaded;
        else if (errorOut && errorOut->isEmpty())
            *errorOut = QLatin1String(res);
    };
    load(":/fonts/PressStart2P-Regular.ttf");
    load(":/fonts/Silkscreen-Regular.ttf");
    load(":/fonts/Silkscreen-Bold.ttf");
    return loaded > 0;
}

QFont pixelFont(int pixelSize)
{
    QFont f(QStringLiteral("Press Start 2P"));
    f.setPixelSize(pixelSize);
    f.setLetterSpacing(QFont::AbsoluteSpacing, 0.5);
    return f;
}

QFont bodyFont(int pixelSize)
{
    QFont f(QStringLiteral("Silkscreen"));
    f.setPixelSize(pixelSize);
    f.setStyleStrategy(QFont::PreferAntialias);
    return f;
}

QImage pixelate(const QImage& src, int lowRes)
{
    if (src.isNull())
        return src;
    QImage s = src;
    if (s.format() != QImage::Format_ARGB32_Premultiplied)
        s = s.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    const QImage small = s.scaled(lowRes, lowRes, Qt::KeepAspectRatio, Qt::FastTransformation);
    return small.scaled(s.width(), s.height(), Qt::KeepAspectRatio, Qt::FastTransformation);
}

void apply(QApplication* app)
{
    app->setStyle(QStyleFactory::create(QStringLiteral("Fusion")));

    QPalette pal;
    pal.setColor(QPalette::Window, s_colors.bg);
    pal.setColor(QPalette::WindowText, s_colors.text);
    pal.setColor(QPalette::Base, s_colors.panel);
    pal.setColor(QPalette::AlternateBase, s_colors.panel2);
    pal.setColor(QPalette::Text, s_colors.text);
    pal.setColor(QPalette::Button, s_colors.panel2);
    pal.setColor(QPalette::ButtonText, s_colors.text);
    pal.setColor(QPalette::Highlight, s_colors.green);
    pal.setColor(QPalette::HighlightedText, QColor(0x0B, 0x0B, 0x0C));
    pal.setColor(QPalette::ToolTipBase, s_colors.panel2);
    pal.setColor(QPalette::ToolTipText, s_colors.text);
    pal.setColor(QPalette::PlaceholderText, s_colors.textDim);
    pal.setColor(QPalette::Disabled, QPalette::Text, s_colors.textDim);
    pal.setColor(QPalette::Disabled, QPalette::ButtonText, s_colors.textDim);
    app->setPalette(pal);

    // Pixel-art stylesheet: sharp 90° corners, hard borders, blocky press.
    const QString qss = QStringLiteral(R"QSS(
* { outline: none; }

QMainWindow, QDialog { background: #0B0B0C; }

QWidget { font-family: "Silkscreen"; color: #E8EAED; }

QFrame#HardPanel, QWidget#HardPanel {
    background: #161617;
    border: 2px solid #2E2E2E;
}

QLabel#SectionTitle {
    font-family: "Silkscreen";
    font-size: 8pt;
    font-weight: bold;
    color: #39FF14;
    letter-spacing: 1px;
}

QPushButton {
    background: #1D1D1E;
    border: 2px solid #3F3F3F;
    border-bottom: 4px solid #2E2E2E;
    color: #E8EAED;
    padding: 4px 10px;
    font-family: "Silkscreen";
    min-height: 16px;
}
QPushButton:hover { border-color: #39FF14; }
QPushButton:pressed {
    background: #101010;
    border-bottom: 1px solid #2E2E2E;
    padding-top: 6px;
}
QPushButton:checked {
    background: #39FF14;
    color: #0B0B0C;
    border: 2px solid #39FF14;
    border-bottom: 4px solid #1C7A0A;
}
QPushButton:disabled { color: #555; border-color: #2E2E2E; }

QPushButton#IconButton {
    border: 2px solid #3F3F3F;
    border-bottom: 3px solid #2E2E2E;
    background: transparent;
    padding: 2px;
    min-width: 28px;
    min-height: 28px;
}
QPushButton#IconButton:hover { border-color: #39FF14; }
QPushButton#IconButton:pressed { border-bottom: 1px solid #2E2E2E; }
QPushButton#IconButton:checked { background: rgba(57,255,20,0.15); border-color: #39FF14; }
QPushButton#IconButton:disabled { border-color: #2E2E2E; }

QLineEdit, QComboBox, QSpinBox {
    background: #0B0B0C;
    border: 2px solid #3F3F3F;
    color: #E8EAED;
    padding: 4px 8px;
    selection-background-color: #39FF14;
    selection-color: #0B0B0C;
    font-family: "Silkscreen";
}
QLineEdit:focus, QComboBox:focus { border-color: #39FF14; }
QComboBox::drop-down { border: none; width: 22px; }
QComboBox::down-arrow { image: none; width: 0; }
QComboBox QAbstractItemView {
    background: #161617;
    border: 2px solid #3F3F3F;
    selection-background-color: #39FF14;
    selection-color: #0B0B0C;
}

QTableView, QTreeView {
    background: #0B0B0C;
    alternate-background-color: #101011;
    border: 2px solid #2E2E2E;
    gridline-color: #1A1A1A;
    selection-background-color: rgba(57,255,20,0.14);
    selection-color: #39FF14;
    font-family: "Silkscreen";
}
QTableView::item:hover, QTreeView::item:hover { background: rgba(57,255,20,0.07); }

QHeaderView::section {
    background: #1D1D1E;
    color: #9A9A9A;
    border: 1px solid #2E2E2E;
    border-top: none;
    border-right: 2px solid #2E2E2E;
    padding: 5px 8px;
    font-family: "Silkscreen";
    font-weight: bold;
    letter-spacing: 1px;
}
QHeaderView::section:hover { color: #39FF14; }
QHeaderView::section::down-arrow, QHeaderView::section::up-arrow {
    image: none;
    width: 0;
}

QMenu {
    background: #161617;
    border: 2px solid #3F3F3F;
    padding: 4px;
    font-family: "Silkscreen";
}
QMenu::item { padding: 5px 18px 5px 10px; color: #E8EAED; }
QMenu::item:selected { background: #39FF14; color: #0B0B0C; }
QMenu::item:disabled { color: #555; }
QMenu::separator { height: 2px; background: #2E2E2E; margin: 4px 6px; }

QToolTip {
    background: #161617;
    color: #E8EAED;
    border: 2px solid #39FF14;
    padding: 4px 8px;
    font-family: "Silkscreen";
}

QScrollBar:vertical { background: #0B0B0C; width: 14px; margin: 0; }
QScrollBar::handle:vertical { background: #2E2E2E; min-height: 24px; border: 2px solid #2E2E2E; }
QScrollBar::handle:vertical:hover { background: #39FF14; }
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: #161617; }
QScrollBar:horizontal { background: #0B0B0C; height: 14px; margin: 0; }
QScrollBar::handle:horizontal { background: #2E2E2E; min-width: 24px; border: 2px solid #2E2E2E; }
QScrollBar::handle:horizontal:hover { background: #39FF14; }
QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }
QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal { background: #161617; }

QStatusBar { background: #161617; border-top: 2px solid #2E2E2E; }

QCheckBox { spacing: 8px; font-family: "Silkscreen"; }
QCheckBox::indicator { width: 14px; height: 14px; border: 2px solid #3F3F3F; background: #0B0B0C; }
QCheckBox::indicator:checked { background: #39FF14; border-color: #39FF14; }
)QSS");
    app->setStyleSheet(qss);
    app->setFont(bodyFont());
}

} // namespace PixelTheme