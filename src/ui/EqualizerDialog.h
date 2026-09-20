#pragma once

#include <QVector>
#include <QWidget>

class PixelSlider;
class QCheckBox;
class QComboBox;

// 10-band equalizer panel (lives on a tab page). Bands in dB, presets, power.
class EqualizerDialog : public QWidget {
    Q_OBJECT
public:
    explicit EqualizerDialog(QWidget* parent = nullptr);

    QVector<float> gainsDb() const;
    bool isPowered() const;
    void retranslate();

signals:
    void gainsChanged(const QVector<float>& gainsDb);
    void powerChanged(bool on);

public slots:
    void setGains(const QVector<float>& gainsDb);
    void setPowered(bool on);

private slots:
    void onPresetSelected(int index);
    void onAnySliderChanged();
    void onReset();

private:
    QVector<float> m_gains{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    PixelSlider* m_sliders[10] = {};
    QComboBox* m_preset = nullptr;
    QCheckBox* m_power = nullptr;
};