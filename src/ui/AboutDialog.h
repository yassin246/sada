#pragma once

#include <QDialog>

// Pixel-styled about window: logo, version and credits (N0RM4N).
class AboutDialog : public QDialog {
    Q_OBJECT
public:
    explicit AboutDialog(QWidget* parent = nullptr);
};