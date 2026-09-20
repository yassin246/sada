#pragma once

#include <QObject>
#include <QString>

// Minimal bilingual (English / Arabic) support. A single global instance
// holds the current language; widgets retranslate when languageChanged fires.
class I18n : public QObject {
    Q_OBJECT
public:
    enum Lang { En, Ar };
    Q_ENUM(Lang)

    static I18n* instance();
    Lang lang() const { return lang_; }

    // Uses the current language to pick between the two provided strings.
    static QString t(const QString& en, const QString& ar);

public slots:
    void setLang(Lang lang);

signals:
    void languageChanged();

private:
    I18n() = default;
    static I18n* s_instance;
    Lang lang_ = En;
};