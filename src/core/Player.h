#pragma once

#include <QObject>
#include <QString>
#include <QVector>

struct mpv_handle;

// Thin libmpv wrapper. Audio-only. All mpv calls happen on the thread that
// created this object (the Qt main thread); the wakeup callback only pokes
// the Qt event loop so the real work runs in processEvents().
class Player : public QObject {
    Q_OBJECT
public:
    enum class State { Stopped, Playing, Paused };
    Q_ENUM(State)

    explicit Player(QObject* parent = nullptr);
    ~Player() override;

    bool valid() const { return mpv_ != nullptr; }

    void loadFile(const QString& path);
    void playPause();
    void play() { setPaused(false); }
    void pause() { setPaused(true); }
    void stop();
    void seek(double seconds);
    void seekBy(double deltaSeconds);

    void setVolume(int volume);
    int volume() const;
    void setMute(bool on);
    bool muted() const;

    // 10-band equalizer gains in dB (range typically -12..+12).
    // Mapped onto superequalizer's 18 bands with interpolation.
    void setGains10(const QVector<float>& db);
    void clearEqualizer();

    double position() const;
    double duration() const;
    State state() const { return state_; }
    bool isPlaying() const { return state_ == State::Playing; }
    QString mediaTitle() const;

signals:
    void endOfFile();
    void stateChanged(Player::State state);
    void positionChanged(double seconds);
    void durationChanged(double seconds);
    void volumeChanged(int volume, bool muted);
    void mediaTitleChanged(const QString& title);
    void logMessage(const QString& text);

private slots:
    void processEvents();

private:
    static void wakeup(void* ctx);
    void setPaused(bool on);
    void updateState();
    double propertyDouble(const char* name, double dflt = 0.0) const;
    bool propertyFlag(const char* name, bool dflt = false) const;
    QString propertyString(const char* name) const;
    void applyEqualizer();

    mpv_handle* mpv_ = nullptr;
    State state_ = State::Stopped;
    QVector<float> gains10_;
};