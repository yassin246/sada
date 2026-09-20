#include "core/I18n.h"

I18n* I18n::s_instance = nullptr;

I18n* I18n::instance()
{
    if (!s_instance)
        s_instance = new I18n;
    return s_instance;
}

QString I18n::t(const QString& en, const QString& ar)
{
    return instance()->lang() == Ar ? ar : en;
}

void I18n::setLang(Lang lang)
{
    if (lang_ == lang)
        return;
    lang_ = lang;
    emit languageChanged();
}