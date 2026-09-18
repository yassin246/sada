#include "core/Player.h"

#include <mpv/client.h>

#include <QDebug>
#include <QFile>
#include <QtMath>

#include <cstdio>
#include <cstring>

Player::Player(QObject* parent)
    : QObject(parent)
{
    mpv_ = mpv_create();
    if (!mpv_)
        return;

    mpv_set_option_string(mpv_, "input-default-bindings", "no");
    mpv_set_option_string(mpv_, "input-builtin-bindings", "no");
    mpv_set_option_string(mpv_, "idle", "yes");
    mpv_set_option_string(mpv_, "video", "no"); // audio-only
    mpv_set_option_string(mpv_, "audio-display", "no");
    mpv_set_option_string(mpv_, "audio-client-name", "sadaudio");

    const QString ao = qEnvironmentVariable("SADAUDIO_AO");
    if (!ao.isEmpty())
        mpv_set_option_string(mpv_, "ao", ao.toUtf8().constData());

    const int initRc = mpv_initialize(mpv_);
    if (initRc < 0) {
        qWarning().noquote() << "mpv_initialize failed:"
                             << QString::fromUtf8(mpv_error_string(initRc));
        mpv_destroy(mpv_);
        mpv_ = nullptr;
        return;
    }

    mpv_observe_property(mpv_, 1, "pause", MPV_FORMAT_FLAG);
    mpv_observe_property(mpv_, 2, "volume", MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv_, 3, "mute", MPV_FORMAT_FLAG);
    mpv_observe_property(mpv_, 4, "time-pos", MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv_, 5, "duration", MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv_, 6, "media-title", MPV_FORMAT_STRING);
    mpv_observe_property(mpv_, 7, "idle-active", MPV_FORMAT_FLAG);

    mpv_request_log_messages(mpv_, "w");
    mpv_set_wakeup_callback(mpv_, &Player::wakeup, this);

    if (qEnvironmentVariableIsSet("SADAUDIO_DEBUG"))
        fprintf(stderr, "[D] player: initialized ok\n");
}

Player::~Player()
{
    if (mpv_)
        mpv_terminate_destroy(mpv_);
}

void Player::wakeup(void* ctx)
{
    auto* self = static_cast<Player*>(ctx);
    if (qEnvironmentVariableIsSet("SADAUDIO_DEBUG"))
        fprintf(stderr, "[D] wakeup\n");
    QMetaObject::invokeMethod(self, "processEvents", Qt::QueuedConnection);
}

void Player::processEvents()
{
    if (!mpv_)
        return;

    int count = 0;
    while (true) {
        mpv_event* e = mpv_wait_event(mpv_, 0);
        if (e->event_id == MPV_EVENT_NONE)
            break;
        ++count;

        switch (e->event_id) {
        case MPV_EVENT_END_FILE: {
            auto* ef = static_cast<mpv_event_end_file*>(e->data);
            if (ef->reason == MPV_END_FILE_REASON_EOF) {
                state_ = State::Stopped;
                emit stateChanged(state_);
                emit endOfFile();
            } else if (ef->reason == MPV_END_FILE_REASON_ERROR) {
                emit logMessage(QStringLiteral("mpv: %1")
                                    .arg(QString::fromUtf8(ef->error ? mpv_error_string(ef->error) : "unknown error")));
            }
            break;
        }
        case MPV_EVENT_PROPERTY_CHANGE: {
            auto* p = static_cast<mpv_event_property*>(e->data);
            if (strcmp(p->name, "pause") == 0 || strcmp(p->name, "idle-active") == 0) {
                updateState();
            } else if (strcmp(p->name, "volume") == 0 || strcmp(p->name, "mute") == 0) {
                emit volumeChanged(volume(), muted());
            } else if (strcmp(p->name, "time-pos") == 0) {
                if (p->format == MPV_FORMAT_DOUBLE && p->data)
                    emit positionChanged(*static_cast<double*>(p->data));
            } else if (strcmp(p->name, "duration") == 0) {
                if (p->format == MPV_FORMAT_DOUBLE && p->data)
                    emit durationChanged(*static_cast<double*>(p->data));
            } else if (strcmp(p->name, "media-title") == 0) {
                emit mediaTitleChanged(propertyString("media-title"));
            }
            break;
        }
        case MPV_EVENT_LOG_MESSAGE: {
            auto* m = static_cast<mpv_event_log_message*>(e->data);
            emit logMessage(QStringLiteral("[mpv:%1] %2")
                                .arg(QString::fromUtf8(m->prefix), QString::fromUtf8(m->text).trimmed()));
            break;
        }
        default:
            break;
        }
    }
    if (qEnvironmentVariableIsSet("SADAUDIO_DEBUG"))
        fprintf(stderr, "[D] processEvents: %d events\n", count);
}

void Player::setPaused(bool on)
{
    if (!mpv_)
        return;
    const int flag = on ? 1 : 0;
    mpv_set_property(mpv_, "pause", MPV_FORMAT_FLAG, const_cast<int*>(&flag));
    updateState();
}

void Player::playPause()
{
    if (!mpv_)
        return;
    const bool was = propertyFlag("pause", true);
    setPaused(!was);
}

void Player::updateState()
{
    if (!mpv_)
        return;
    const bool idle = propertyFlag("idle-active");
    const bool paused = propertyFlag("pause");
    State s;
    if (idle)
        s = State::Stopped;
    else
        s = paused ? State::Paused : State::Playing;
    if (s != state_) {
        state_ = s;
        emit stateChanged(s);
    }
}

void Player::loadFile(const QString& path)
{
    if (!mpv_)
        return;
    const QByteArray p = QFile::encodeName(path);
    const char* args[] = { "loadfile", p.constData(), nullptr };
    mpv_command(mpv_, args);
    const int play = 0;
    mpv_set_property(mpv_, "pause", MPV_FORMAT_FLAG, const_cast<int*>(&play));
    updateState();
}

void Player::stop()
{
    if (!mpv_)
        return;
    const char* args[] = { "stop", nullptr };
    mpv_command(mpv_, args);
    updateState();
}

void Player::seek(double seconds)
{
    if (!mpv_)
        return;
    mpv_set_property(mpv_, "time-pos", MPV_FORMAT_DOUBLE, &seconds);
}

void Player::seekBy(double deltaSeconds)
{
    if (!mpv_)
        return;
    const char* args[] = { "seek", QString::number(deltaSeconds).toUtf8().constData(), "relative" };
    mpv_command(mpv_, args);
}

void Player::setVolume(int volume)
{
    if (!mpv_)
        return;
    if (qAbs(this->volume() - volume) < 1)
        return;
    double v = qBound(0, volume, 150);
    mpv_set_property(mpv_, "volume", MPV_FORMAT_DOUBLE, &v);
}

int Player::volume() const
{
    return qRound(propertyDouble("volume", 70));
}

void Player::setMute(bool on)
{
    if (!mpv_)
        return;
    const int flag = on ? 1 : 0;
    mpv_set_property(mpv_, "mute", MPV_FORMAT_FLAG, const_cast<int*>(&flag));
}

bool Player::muted() const
{
    return propertyFlag("mute");
}

double Player::position() const
{
    return propertyDouble("time-pos");
}

double Player::duration() const
{
    return propertyDouble("duration");
}

QString Player::mediaTitle() const
{
    return propertyString("media-title");
}

double Player::propertyDouble(const char* name, double dflt) const
{
    if (!mpv_)
        return dflt;
    double v = dflt;
    mpv_get_property(mpv_, name, MPV_FORMAT_DOUBLE, &v);
    return v;
}

bool Player::propertyFlag(const char* name, bool dflt) const
{
    if (!mpv_)
        return dflt;
    int v = dflt ? 1 : 0;
    mpv_get_property(mpv_, name, MPV_FORMAT_FLAG, &v);
    return v != 0;
}

QString Player::propertyString(const char* name) const
{
    if (!mpv_)
        return {};
    char* val = nullptr;
    if (mpv_get_property(mpv_, name, MPV_FORMAT_STRING, &val) >= 0 && val) {
        QString s = QString::fromUtf8(val);
        mpv_free(val);
        return s;
    }
    return {};
}

void Player::setGains10(const QVector<float>& db)
{
    gains10_ = db.size() == 10 ? db : QVector<float>(10, 0.0f);
    applyEqualizer();
}

void Player::clearEqualizer()
{
    gains10_.clear();
    if (mpv_)
        mpv_set_property_string(mpv_, "af", "");
}

void Player::applyEqualizer()
{
    if (!mpv_)
        return;

    // superequalizer band indices that our 10 GUI bands map to (1:1).
    static const int anchors[10] = { 0, 3, 5, 7, 8, 11, 13, 15, 16, 17 };

    double bandDb[18];
    for (int k = 0; k < 18; ++k) {
        if (k <= anchors[0]) {
            bandDb[k] = gains10_.value(0, 0.0);
        } else if (k >= anchors[9]) {
            bandDb[k] = gains10_.value(9, 0.0);
        } else {
            // find surrounding anchors and interpolate linearly in dB
            for (int a = 0; a < 9; ++a) {
                const int lo = anchors[a];
                const int hi = anchors[a + 1];
                if (k > lo && k <= hi) {
                    const double t = double(k - lo) / (hi - lo);
                    bandDb[k] = gains10_.value(a, 0.0) * (1.0 - t) + gains10_.value(a + 1, 0.0) * t;
                    break;
                }
            }
        }
    }

    QString graph = QStringLiteral("superequalizer=");
    for (int j = 0; j < 18; ++j) {
        if (j > 0)
            graph += QLatin1Char(':');
        const double mul = qPow(10.0, bandDb[j] / 20.0);
        graph += QStringLiteral("%1b=%2").arg(j + 1).arg(mul, 0, 'f', 4);
    }
    const QString af = QStringLiteral("lavfi=[%1]").arg(graph);
    const int rc = mpv_set_property_string(mpv_, "af", af.toUtf8().constData());
    if (rc != 0)
        emit logMessage(QStringLiteral("EQ apply failed: %1 (%2)").arg(QString::fromUtf8(mpv_error_string(rc)), af));
}