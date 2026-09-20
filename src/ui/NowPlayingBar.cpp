#include "ui/NowPlayingBar.h"

#include "core/I18n.h"
#include "ui/PixelIcons.h"
#include "ui/PixelSlider.h"
#include "ui/PixelTheme.h"

#include <QFontMetrics>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QToolTip>
#include <QVBoxLayout>
#include <QtMath>

namespace {

QString fmtSec(int sec)
{
    if (sec < 0)
        sec = 0;
    return QStringLiteral("%1:%2").arg(sec / 60).arg(sec % 60, 2, 10, QLatin1Char('0'));
}

} // namespace

// ---------------- BeatMeter ----------------

BeatMeter::BeatMeter(QWidget* parent)
    : QWidget(parent)
{
    setFixedSize(58, 30);
    m_timer.setInterval(90);
    connect(&m_timer, &QTimer::timeout, this, [this] {
        for (int c = 0; c < 3; ++c) {
            ++m_phase;
            if (m_playing) {
                const double t = 0.5 + 0.5 * qSin(m_phase * 0.11 + c * 1.7);
                m_target[c] = 1.0 + 3.0 * t;
            } else {
                m_target[c] = 0.0;
            }
            m_level[c] += (m_target[c] - m_level[c]) * 0.4;
        }
        update();
    });
    m_timer.start();
}

void BeatMeter::setPlaying(bool playing)
{
    m_playing = playing;
    if (!playing)
        for (double& l : m_level)
            l = 0.0;
}

void BeatMeter::paintEvent(QPaintEvent*)
{
    const auto& C = PixelTheme::colors();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, false);
    p.fillRect(rect(), Qt::transparent);

    constexpr int cell = 6;
    constexpr int gap = 2;
    constexpr int cellsPerCol = 3;
    for (int c = 0; c < 3; ++c) {
        const int x = 4 + c * (cell + gap + 6);
        const int lit = qMin(cellsPerCol, int(qCeil(m_level[c])));
        for (int i = 0; i < cellsPerCol; ++i) {
            const bool on = i < lit;
            const bool isTop = i == cellsPerCol - 1;
            const QRect r(x, height() - 2 - (i + 1) * (cell + gap), cell, cell);
            if (on)
                p.fillRect(r, isTop ? C.red : C.green);
            else
                p.fillRect(r, C.border);
        }
    }
}

// ---------------- NowPlayingBar ----------------

namespace {

QPushButton* makeIconButton(const QIcon& icon, int size = 34, QWidget* parent = nullptr)
{
    auto* b = new QPushButton(parent);
    b->setObjectName(QStringLiteral("IconButton"));
    b->setIcon(icon);
    b->setFixedSize(size, size);
    b->setIconSize(QSize(size - 8, size - 8));
    b->setCursor(Qt::PointingHandCursor);
    return b;
}

} // namespace

NowPlayingBar::NowPlayingBar(QWidget* parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("HardPanel"));
    setFixedHeight(62);

    auto* hl = new QHBoxLayout(this);
    hl->setContentsMargins(10, 5, 10, 5);
    hl->setSpacing(8);

    // Cover
    m_cover = new QLabel(this);
    m_cover->setFixedSize(44, 44);
    m_cover->setAlignment(Qt::AlignCenter);
    m_cover->setStyleSheet(QStringLiteral("border: 2px solid #3F3F3F; background: #0B0B0C;"));
    hl->addWidget(m_cover);

    // Title / subtitle
    auto* textCol = new QVBoxLayout;
    textCol->setSpacing(2);
    m_title = new QLabel(this);
    m_title->setFont(PixelTheme::pixelFont(9));
    m_title->setStyleSheet(QStringLiteral("color: #39FF14;"));
    m_subtitle = new QLabel(this);
    m_subtitle->setFont(PixelTheme::bodyFont(8));
    m_subtitle->setStyleSheet(QStringLiteral("color: #9A9A9A;"));
    textCol->addWidget(m_title);
    textCol->addWidget(m_subtitle);
    auto* textWrap = new QWidget(this);
    textWrap->setLayout(textCol);
    textWrap->setMinimumWidth(160);
    textWrap->setMaximumWidth(280);
    hl->addWidget(textWrap);
    m_textWrap = textWrap;

    // Beat meter
    auto* meter = new BeatMeter(this);
    hl->addWidget(meter);

    // Transport
    m_prevBtn = makeIconButton(PixelIcons::prevDim(), 30, this);
    m_playBtn = makeIconButton(PixelIcons::playRed(), 38, this);
    m_nextBtn = makeIconButton(PixelIcons::nextDim(), 30, this);
    hl->addWidget(m_prevBtn);
    hl->addWidget(m_playBtn);
    hl->addWidget(m_nextBtn);

    // Seek
    m_seek = new PixelSlider(Qt::Horizontal, this);
    m_seek->setRange(0, 1);
    m_seek->setEnabled(false);
    m_seek->setMinimumWidth(140);
    hl->addWidget(m_seek, 1);

    m_time = new QLabel(QStringLiteral("0:00 / 0:00"), this);
    m_time->setFont(PixelTheme::pixelFont(8));
    m_time->setStyleSheet(QStringLiteral("color: #9A9A9A;"));
    m_time->setMinimumWidth(88);
    hl->addWidget(m_time);

    // Repeat / shuffle
    m_repeatBtn = makeIconButton(PixelIcons::repeatDim(), 26, this);
    m_shuffleBtn = makeIconButton(PixelIcons::shuffleDim(), 26, this);
    m_shuffleBtn->setCheckable(true);
    hl->addWidget(m_repeatBtn);
    hl->addWidget(m_shuffleBtn);

    // Volume
    m_volume = new PixelSlider(Qt::Horizontal, this);
    m_volume->setRange(0, 100);
    m_volume->setValue(70);
    m_volume->setFixedWidth(84);
    hl->addWidget(m_volume);

    m_muteBtn = makeIconButton(PixelIcons::volumeDim(), 26, this);
    m_muteBtn->setCheckable(true);
    m_eqBtn = makeIconButton(PixelIcons::eqGreen(), 26, this);
    m_miniBtn = makeIconButton(PixelIcons::miniDim(), 26, this);
    hl->addWidget(m_muteBtn);
    hl->addWidget(m_eqBtn);
    hl->addWidget(m_miniBtn);

    // Wiring
    connect(m_prevBtn, &QPushButton::clicked, this, &NowPlayingBar::prevRequested);
    connect(m_playBtn, &QPushButton::clicked, this, &NowPlayingBar::playPauseRequested);
    connect(m_nextBtn, &QPushButton::clicked, this, &NowPlayingBar::nextRequested);

    connect(m_seek, &PixelSlider::sliderPressed, this, [this] { m_dragging = true; emit seekPreview(m_seek->value()); });
    connect(m_seek, &PixelSlider::sliderMoved, this, [this] { emit seekPreview(m_seek->value()); });
    connect(m_seek, &PixelSlider::sliderReleased, this, [this] {
        m_dragging = false;
        emit seekRequested(m_seek->value());
    });
    connect(m_seek, &PixelSlider::valueChanged, this, &NowPlayingBar::updateTimeLabel);

    connect(m_volume, &PixelSlider::valueChanged, this, &NowPlayingBar::volumeChanged);
    connect(m_muteBtn, &QPushButton::clicked, this, &NowPlayingBar::muteRequested);
    connect(m_eqBtn, &QPushButton::clicked, this, &NowPlayingBar::equalizerRequested);
    connect(m_repeatBtn, &QPushButton::clicked, this, &NowPlayingBar::repeatRequested);
    connect(m_shuffleBtn, &QPushButton::toggled, this, [this](bool on) {
        m_shuffle = on;
        m_shuffleBtn->setIcon(on ? PixelIcons::shuffleGreen() : PixelIcons::shuffleDim());
    });
    connect(m_miniBtn, &QPushButton::clicked, this, &NowPlayingBar::miniRequested);

    retranslate();
}

void NowPlayingBar::mousePressEvent(QMouseEvent* e)
{
    // Spotify-style: tapping the cover or the title/artist info opens the
    // full now-playing page. Labels ignore mouse events by default, so a
    // click on them lands here.
    QWidget* const hit = childAt(e->pos());
    if (hit == m_cover || hit == m_title || hit == m_subtitle || hit == m_textWrap) {
        emit fullPageRequested();
        return;
    }
    QWidget::mousePressEvent(e);
}

void NowPlayingBar::retranslate()
{
    m_prevBtn->setToolTip(I18n::t(QStringLiteral("Previous"), QStringLiteral("السابق")));
    m_playBtn->setToolTip(I18n::t(QStringLiteral("Play / Pause"), QStringLiteral("تشغيل / إيقاف مؤقت")));
    m_nextBtn->setToolTip(I18n::t(QStringLiteral("Next"), QStringLiteral("التالي")));
    m_muteBtn->setToolTip(I18n::t(QStringLiteral("Mute"), QStringLiteral("كتم الصوت")));
    m_eqBtn->setToolTip(I18n::t(QStringLiteral("Equalizer"), QStringLiteral("المعادل")));
    m_miniBtn->setToolTip(I18n::t(QStringLiteral("Mini mode"), QStringLiteral("الوضع المصغّر")));
    m_volume->setToolTip(I18n::t(QStringLiteral("Volume"), QStringLiteral("مستوى الصوت")));
    updateTimeLabel();
}

void NowPlayingBar::setSong(const Song& song, const QPixmap& cover, bool playing)
{
    if (!cover.isNull()) {
        m_cover->setPixmap(cover);
    } else {
        QPixmap ph(44, 44);
        ph.fill(PixelTheme::colors().bg);
        QPainter p(&ph);
        p.drawPixmap(10, 10, PixelIcons::musicDim().pixmap(24, 24));
        m_cover->setPixmap(ph);
    }

    static const QFontMetrics fmTitle(PixelTheme::pixelFont(10));
    static const QFontMetrics fmSub(PixelTheme::bodyFont(9));
    m_title->setText(fmTitle.elidedText(song.title, Qt::ElideRight, 240));
    QString sub = song.artist;
    if (!song.album.isEmpty())
        sub = sub.isEmpty() ? song.album : sub + QStringLiteral(" — ") + song.album;
    if (sub.isEmpty())
        sub = I18n::t(QStringLiteral("Unknown artist"), QStringLiteral("فنان غير معروف"));
    m_subtitle->setText(fmSub.elidedText(sub, Qt::ElideRight, 240));

    setPlaying(playing);
}

void NowPlayingBar::setPlaying(bool playing)
{
    m_playBtn->setIcon(playing ? PixelIcons::pauseGreen() : PixelIcons::playRed());
    if (auto* meter = findChild<BeatMeter*>())
        meter->setPlaying(playing);
}

void NowPlayingBar::setPosition(int seconds, int durationSeconds)
{
    m_duration = durationSeconds;
    m_current = seconds;
    const int max = qMax(1, durationSeconds);
    if (m_seek->maximum() != max)
        m_seek->setRange(0, max);
    if (!m_dragging)
        m_seek->setValue(seconds);
    m_seek->setEnabled(durationSeconds > 0);
    updateTimeLabel();
}

void NowPlayingBar::updateTimeLabel()
{
    m_time->setText(fmtSec(m_dragging ? m_seek->value() : m_current) + QStringLiteral(" / ") + fmtSec(m_duration));
}

void NowPlayingBar::setVolume(int volume)
{
    m_volume->setValue(volume);
}

void NowPlayingBar::setMuted(bool on)
{
    m_muteBtn->setChecked(on);
    m_muteBtn->setIcon(on ? PixelIcons::muteRed() : PixelIcons::volumeDim());
}

void NowPlayingBar::setRepeatMode(RepeatMode mode)
{
    m_repeat = mode;
    m_repeatBtn->setIcon(mode == RepeatOff ? PixelIcons::repeatDim() : PixelIcons::repeatGreen());
    m_repeatBtn->setToolTip(I18n::t(
        mode == RepeatOff ? QStringLiteral("Repeat: Off") : (mode == RepeatAll ? QStringLiteral("Repeat: All") : QStringLiteral("Repeat: One")),
        mode == RepeatOff ? QStringLiteral("التكرار: إيقاف") : (mode == RepeatAll ? QStringLiteral("التكرار: الكل") : QStringLiteral("التكرار: أغنية واحدة"))));
}

void NowPlayingBar::setShuffle(bool on)
{
    m_shuffle = on;
    m_shuffleBtn->setChecked(on);
}