#include "ui/AboutDialog.h"

#include "core/I18n.h"
#include "ui/PixelIcons.h"
#include "ui/PixelTheme.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

AboutDialog::AboutDialog(QWidget* parent)
    : QDialog(parent)
{
    setObjectName(QStringLiteral("HardPanel"));
    setWindowTitle(QStringLiteral("SADAUDIO"));
    setModal(true);
    setMinimumWidth(360);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(18, 18, 18, 16);
    root->setSpacing(10);

    // logo + title
    auto* logoRow = new QHBoxLayout;
    logoRow->setSpacing(10);
    auto* logo = new QLabel(this);
    logo->setPixmap(PixelIcons::appLogo().pixmap(40, 40));
    logoRow->addWidget(logo);
    auto* title = new QLabel(QStringLiteral("SADAUDIO"), this);
    title->setFont(PixelTheme::pixelFont(16));
    title->setStyleSheet(QStringLiteral("color: #39FF14;"));
    logoRow->addWidget(title);
    logoRow->addStretch(1);
    root->addLayout(logoRow);

    auto* ver = new QLabel(I18n::t(QStringLiteral("v0.1.0 — pixel-art audio player"), QStringLiteral("v0.1.0 — مشغّل صوتي بفن البكسل")), this);
    ver->setFont(PixelTheme::bodyFont(9));
    ver->setStyleSheet(QStringLiteral("color: #9A9A9A;"));
    root->addWidget(ver);

    auto* credits = new QLabel(I18n::t(QStringLiteral("CREDITS: N0RM4N"), QStringLiteral("إهداء: N0RM4N")), this);
    credits->setFont(PixelTheme::pixelFont(12));
    credits->setStyleSheet(QStringLiteral("color: #E8EAED;"));
    root->addWidget(credits);

    auto* ok = new QPushButton(I18n::t(QStringLiteral("OK"), QStringLiteral("حسنًا")), this);
    ok->setFixedHeight(34);
    ok->setCursor(Qt::PointingHandCursor);
    connect(ok, &QPushButton::clicked, this, &QDialog::accept);
    auto* btnRow = new QHBoxLayout;
    btnRow->addStretch(1);
    btnRow->addWidget(ok);
    root->addLayout(btnRow);
}