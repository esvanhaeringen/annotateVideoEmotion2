#include "annotateVideoEmotion.h"
#include <QtCore/QCoreApplication>
#include <QtWidgets/QApplication>
#include <QtCore/QCommandLineParser>
#include <QtCore/QCommandLineOption>

int main(int argc, char* argv[])
{
    QCoreApplication::setApplicationName("Annotate Video Emotion");
    QCoreApplication::setOrganizationName("Erik van Haeringen");
    QGuiApplication::setApplicationDisplayName(QCoreApplication::applicationName());
    QCoreApplication::setApplicationVersion(QT_VERSION_STR);
    QCoreApplication::setAttribute(Qt::AA_X11InitThreads);
    QCoreApplication::setAttribute(Qt::AA_DisableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_DisableWindowContextHelpButton);
    if (qgetenv("QT_FONT_DPI").isEmpty()) {
        qputenv("QT_FONT_DPI", "96");
    }
    QApplication app(argc, argv);
    annotateVideoEmotion mainWindow;
    mainWindow.show();

    int result = app.exec();
    return result;
}
