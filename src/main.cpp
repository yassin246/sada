#include "core/I18n.h"
#include "core/Song.h"
#include "ui/MainWindow.h"
#include "ui/PixelTheme.h"

#include <QAbstractButton>
#include <QAbstractSlider>
#include <QApplication>
#include <QLabel>
#include <QSettings>
#include <QTimer>
#include <QVector>

#include <functional>

#include <clocale>

Q_DECLARE_METATYPE(QVector<float>)

int main(int argc, char** argv)
{
    setvbuf(stderr, nullptr, _IONBF, 0);
    QApplication app(argc, argv);

    if (qEnvironmentVariableIsSet("SADAUDIO_DEBUG"))
        fprintf(stderr, "[D] main: qapp constructed\n");

    // Qt re-applies the environment locale when QApplication starts; libmpv
    // requires the C numeric locale (dot decimal separator).
    setlocale(LC_NUMERIC, "C");
    QApplication::setApplicationName(QStringLiteral("sadaudio"));
    QApplication::setApplicationDisplayName(QStringLiteral("SADAUDIO"));
    QApplication::setOrganizationName(QStringLiteral("sadaudio"));
    QApplication::setOrganizationDomain(QStringLiteral("sadaudio.local"));

    QString fontError;
    PixelTheme::registerFonts(&fontError);
    if (!fontError.isEmpty())
        qWarning().noquote() << "Could not load font:" << fontError;

    PixelTheme::apply(&app);

    qRegisterMetaType<Song>();
    qRegisterMetaType<QVector<Song>>();
    qRegisterMetaType<QVector<float>>();

    MainWindow w;

    if (QSettings().value(QStringLiteral("main/lang"), QStringLiteral("en")).toString() == QStringLiteral("ar"))
        w.setLanguage(I18n::Ar);

    w.show();

    QStringList files;
    const QStringList args = app.arguments().mid(1);
    for (const QString& a : args)
        if (!a.startsWith(QLatin1Char('-')))
            files << a;
    if (!files.isEmpty())
        w.openFiles(files);
    if (qEnvironmentVariableIsSet("SADAUDIO_DEBUG")) {
        fprintf(stderr, "[D] main: files=%d playerValid=%d\n", files.size(), w.playerValid());
        fflush(stderr);
    }

    // Dev/QA: render the window to a PNG after a short settle, then exit.
    // Usage: SADAUDIO_SHOT=/tmp/shot.png [SADAUDIO_SHOT_TAB=3] sadaudio [song...]
    const QString shotPath = qEnvironmentVariable("SADAUDIO_SHOT");
    if (!shotPath.isEmpty()) {
        QTimer::singleShot(2400, &w, [&w, shotPath] {
            const int tab = qEnvironmentVariable("SADAUDIO_SHOT_TAB").toInt();
            if (tab > 0)
                w.debugOpenTab(tab);
            if (qEnvironmentVariableIsSet("SADAUDIO_DUMP")) {
                std::function<void(const QWidget*, int)> walk = [&walk](const QWidget* w, int depth) {
                    QString line(depth * 2, QLatin1Char(' '));
                    line += QStringLiteral("%1 [%2] geo=%3x%4+%5+%6 vis=%7")
                                .arg(QString::fromLatin1(w->metaObject()->className()))
                                .arg(w->objectName())
                                .arg(w->width())
                                .arg(w->height())
                                .arg(w->x())
                                .arg(w->y())
                                .arg(w->isVisible() ? 1 : 0);
                    if (const auto* lbl = qobject_cast<const QLabel*>(w)) {
                        line += QStringLiteral(" text=\"%1\"").arg(lbl->text());
                        if (!lbl->pixmap(Qt::ReturnByValue).isNull()) {
                            const auto pm = lbl->pixmap(Qt::ReturnByValue);
                            line += QStringLiteral(" pix=%1x%2").arg(pm.width()).arg(pm.height());
                        }
                    } else if (const auto* btn = qobject_cast<const QAbstractButton*>(w)) {
                        line += QStringLiteral(" btn=\"%1\" ck=%2").arg(btn->text()).arg(btn->isChecked() ? 1 : 0);
                    } else if (const auto* sl = qobject_cast<const QAbstractSlider*>(w)) {
                        line += QStringLiteral(" slider=%1/%2").arg(sl->value()).arg(sl->maximum());
                    }
                    fprintf(stderr, "%s\n", qPrintable(line));
                    const auto kids = w->children();
                    for (const QObject* k : kids)
                        if (const auto* kw = qobject_cast<const QWidget*>(k))
                            walk(kw, depth + 1);
                };
                walk(&w, 0);
            }
            const bool ok = w.grab().save(shotPath);
            const QString tabNow = qEnvironmentVariable("SADAUDIO_SHOT_TAB").isEmpty()
                ? QStringLiteral("lib")
                : qEnvironmentVariable("SADAUDIO_SHOT_TAB");
            fprintf(stderr, "[D] shot tab=%s saved=%d %s\n", qPrintable(tabNow), ok ? 1 : 0, ok ? qPrintable(shotPath) : "FAILED");
            fflush(stderr);
            QTimer::singleShot(150, &w, [&w] { w.close(); });
        });
    }

    return app.exec();
}