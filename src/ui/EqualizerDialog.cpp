#include "ui/EqualizerDialog.h"

#include "core/I18n.h"
#include "ui/PixelIcons.h"
#include "ui/PixelSlider.h"
#include "ui/PixelTheme.h"

#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace {

const double kFreqs[10] = { 65, 160, 370, 740, 1000, 3000, 6000, 12000, 16000, 20000 };

QString freqLabel(double f)
{
    if (f >= 1000)
        return QStringLiteral("%1K").arg(f / 1000.0, 0, 'g', 3);
    return QString::number(int(f));
}

// dB per band for the presets
const QVector<float>& preset(int i)
{
    static const QVector<QVector<float>> presets = {
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },                                                // Flat
        { 4, 2, -1, -1, 0, 2, 4, 4, 3, 2 },                                              // Rock
        { -1, 1, 3, 4, 3, -1, -1, 1, 2, 2 },                                             // Pop
        { 3, 2, 0, 2, 3, 2, 0, 1, 1, 0 },                                                // Jazz
        { 4, 3, 2, 1, 1, 0, 0, 0, 1, 2 },                                                // Classical
        { 7, 5, 3, 1, 0, 0, 0, 0, 0, 0 },                                                // Bass Boost
        { -1, 1, 2, 2, 3, 3, 2, 1, 0, 0 },                                               // Vocal
    };
    return presets.at(qBound(0, i, presets.size() - 1));
}

} // namespace

EqualizerDialog::EqualizerDialog(QWidget* parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("HardPanel"));
    setWindowTitle(I18n::t(QStringLiteral("Equalizer"), QStringLiteral("المعادل الصوتي")));

    auto* vl = new QVBoxLayout(this);
    vl->setContentsMargins(14, 14, 14, 14);
    vl->setSpacing(10);

    // Top row: power + presets + reset
    auto* top = new QHBoxLayout;
    m_power = new QCheckBox(this);
    m_preset = new QComboBox(this);
    m_preset->setMinimumWidth(150);
    auto* resetBtn = new QPushButton(this);
    resetBtn->setIcon(PixelIcons::repeatDim());
    resetBtn->setToolTip(I18n::t(QStringLiteral("Reset"), QStringLiteral("إعادة الضبط")));
    top->addWidget(m_power);
    top->addWidget(m_preset);
    top->addStretch(1);
    top->addWidget(resetBtn);
    vl->addLayout(top);

    // Slider grid
    auto* grid = new QGridLayout;
    grid->setHorizontalSpacing(6);
    grid->setVerticalSpacing(4);

    static const QString presets[] = {
        QStringLiteral("Flat"),         QStringLiteral("Rock"),     QStringLiteral("Pop"),
        QStringLiteral("Jazz"),         QStringLiteral("Classical"), QStringLiteral("Bass Boost"),
        QStringLiteral("Vocal"),
    };
    for (int i = 0; i < 10; ++i) {
        auto* slider = new PixelSlider(Qt::Vertical, this);
        slider->setRange(-12, 12);
        slider->setValue(int(m_gains.at(i)));
        slider->setSegments(16);
        slider->setFixedHeight(180);
        m_sliders[i] = slider;

        auto* fl = new QLabel(freqLabel(kFreqs[i]), this);
        fl->setFont(PixelTheme::bodyFont(8));
        fl->setStyleSheet(QStringLiteral("color: #9A9A9A;"));
        fl->setAlignment(Qt::AlignHCenter);

        auto* value = new QLabel(this);
        value->setFont(PixelTheme::pixelFont(8));
        value->setStyleSheet(QStringLiteral("color: #39FF14;"));
        value->setAlignment(Qt::AlignHCenter);
        value->setMinimumWidth(34);
        const int idx = i;
        connect(slider, &PixelSlider::valueChanged, this, [this, value, idx](int v) {
            value->setText(QStringLiteral("%1").arg(v));
            onAnySliderChanged();
        });

        grid->addWidget(value, 0, i);
        grid->addWidget(slider, 1, i);
        grid->addWidget(fl, 2, i);
    }
    vl->addLayout(grid);

    for (int i = 0; i < 7; ++i)
        m_preset->addItem(I18n::t(presets[i], presets[i]));

    connect(m_preset, &QComboBox::currentIndexChanged, this, &EqualizerDialog::onPresetSelected);
    connect(resetBtn, &QPushButton::clicked, this, &EqualizerDialog::onReset);
    connect(m_power, &QCheckBox::toggled, this, &EqualizerDialog::powerChanged);

    retranslate();
}

void EqualizerDialog::retranslate()
{
    m_power->setText(I18n::t(QStringLiteral("Enable Equalizer"), QStringLiteral("تفعيل المعادل")));
    setWindowTitle(I18n::t(QStringLiteral("Equalizer"), QStringLiteral("المعادل الصوتي")));
}

QVector<float> EqualizerDialog::gainsDb() const
{
    return m_gains;
}

bool EqualizerDialog::isPowered() const
{
    return m_power->isChecked();
}

void EqualizerDialog::setGains(const QVector<float>& gainsDb)
{
    if (gainsDb.size() != 10)
        return;
    for (int i = 0; i < 10; ++i) {
        const QSignalBlocker blocker(m_sliders[i]);
        m_sliders[i]->setValue(qRound(gainsDb.at(i)));
    }
    m_gains = gainsDb;
}

void EqualizerDialog::setPowered(bool on)
{
    m_power->setChecked(on);
}

void EqualizerDialog::onPresetSelected(int index)
{
    if (index < 0)
        return;
    const QVector<float>& p = preset(index);
    for (int i = 0; i < 10; ++i) {
        const QSignalBlocker blocker(m_sliders[i]);
        m_sliders[i]->setValue(qRound(p.at(i)));
        m_gains[i] = p.at(i);
    }
    emit gainsChanged(m_gains);
}

void EqualizerDialog::onAnySliderChanged()
{
    for (int i = 0; i < 10; ++i)
        m_gains[i] = float(m_sliders[i]->value());
    emit gainsChanged(m_gains);
}

void EqualizerDialog::onReset()
{
    m_preset->setCurrentIndex(0);
    onPresetSelected(0);
}