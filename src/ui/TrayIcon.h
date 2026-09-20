#pragma once

#include <QIcon>
#include <QObject>
#include <QString>

class QAction;
class QMenu;
class QSystemTrayIcon;

// System tray icon with quick transport controls.
class TrayIcon : public QObject {
    Q_OBJECT
public:
    explicit TrayIcon(const QIcon& icon, QObject* parent = nullptr);

    bool available() const;
    void show();
    void setToolTipForTrack(const QString& tooltip, bool playing);
    void retranslate();

signals:
    void playPauseRequested();
    void nextRequested();
    void prevRequested();
    void quitRequested();

private:
    QSystemTrayIcon* m_tray = nullptr;
    QMenu* m_menu = nullptr;
    QAction* m_playAct = nullptr;
};