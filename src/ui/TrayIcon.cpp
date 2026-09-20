#include "ui/TrayIcon.h"

#include "core/I18n.h"

#include <QAction>
#include <QApplication>
#include <QMenu>
#include <QSystemTrayIcon>

TrayIcon::TrayIcon(const QIcon& icon, QObject* parent)
    : QObject(parent)
{
    if (!QSystemTrayIcon::isSystemTrayAvailable())
        return;

    m_tray = new QSystemTrayIcon(icon, this);
    m_menu = new QMenu;

    auto* play = new QAction(this);
    auto* next = new QAction(this);
    auto* prev = new QAction(this);
    m_menu->addAction(prev);
    m_menu->addAction(play);
    m_menu->addAction(next);
    m_menu->addSeparator();
    auto* quit = new QAction(this);
    m_menu->addAction(quit);

    m_playAct = play;

    connect(play, &QAction::triggered, this, &TrayIcon::playPauseRequested);
    connect(next, &QAction::triggered, this, &TrayIcon::nextRequested);
    connect(prev, &QAction::triggered, this, &TrayIcon::prevRequested);
    connect(quit, &QAction::triggered, qApp, &QApplication::quit);

    m_tray->setContextMenu(m_menu);
    retranslate();
}

bool TrayIcon::available() const
{
    return m_tray != nullptr;
}

void TrayIcon::show()
{
    if (m_tray)
        m_tray->show();
}

void TrayIcon::retranslate()
{
    if (!m_menu)
        return;
    const auto actions = m_menu->actions();
    if (actions.size() >= 4) {
        // Guarded index-by-index: never touch an out-of-bounds slot,
        // even if the menu layout changes later.
        if (actions.size() > 0) actions.at(0)->setText(I18n::t(QStringLiteral("Previous"), QStringLiteral("السابق")));
        if (actions.size() > 1) actions.at(1)->setText(I18n::t(QStringLiteral("Play / Pause"), QStringLiteral("تشغيل / إيقاف")));
        if (actions.size() > 2) actions.at(2)->setText(I18n::t(QStringLiteral("Next"), QStringLiteral("التالي")));
        if (actions.size() > 4) actions.at(4)->setText(I18n::t(QStringLiteral("Quit"), QStringLiteral("خروج")));
    }
}

void TrayIcon::setToolTipForTrack(const QString& tooltip, bool playing)
{
    if (!m_tray)
        return;
    m_tray->setToolTip(tooltip);
    if (m_playAct)
        m_playAct->setText(playing ? I18n::t(QStringLiteral("Pause"), QStringLiteral("إيقاف")) : I18n::t(QStringLiteral("Play"), QStringLiteral("تشغيل")));
}