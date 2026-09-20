#pragma once

#include <QPixmap>
#include <QString>
#include <QWidget>

class QPropertyAnimation;
class QTimer;

// Small unmanaged notification shown near the bottom-right of the screen.
class Toast : public QWidget {
    Q_OBJECT
public:
    explicit Toast();

    void showToast(const QString& title, const QString& subtitle, const QPixmap& cover);

protected:
    void paintEvent(QPaintEvent*) override;

private:
    QString m_title;
    QString m_subtitle;
    QPixmap m_cover;
    QTimer* m_hideTimer = nullptr;
    QPropertyAnimation* m_fade = nullptr;
};