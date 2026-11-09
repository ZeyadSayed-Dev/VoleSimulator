#include "MainWindow.h"

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QFont>
#include <QMessageBox>
#include <QRect>
#include <QScreen>
#include <QStyleFactory>
#include <QTextStream>

#include <cstdio>

#ifdef Q_OS_WIN
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <windows.h>
#endif

int main(int argc, char *argv[])
{
#ifdef Q_OS_WIN
    // When the program is started from a terminal, attach to that terminal so
    // --help and the simulator's screen writes are visible. This is a no-op
    // when the program is started from Explorer or CLion's run button.
    if (AttachConsole(ATTACH_PARENT_PROCESS))
    {
        (void)std::freopen("CONOUT$", "w", stdout);
        (void)std::freopen("CONOUT$", "w", stderr);
        (void)std::freopen("CONIN$", "r", stdin);
    }
#endif

    QApplication application(argc, argv);

    QApplication::setApplicationName(QStringLiteral("Vole Machine Simulator"));
    QApplication::setApplicationDisplayName(QStringLiteral("Vole Machine Simulator"));
    QApplication::setOrganizationName(QStringLiteral("VoleSimulator"));
    QApplication::setApplicationVersion(QStringLiteral("1.0.0"));

    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    QApplication::setFont(QFont(QStringLiteral("Segoe UI"), 10));

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Vole Machine Simulator (Qt 6 GUI)"));
    parser.addVersionOption();

    QCommandLineOption helpOption(
        { QStringLiteral("h"), QStringLiteral("help") },
        QStringLiteral("Displays this help."));
    parser.addOption(helpOption);

    QCommandLineOption programOption(
        { QStringLiteral("p"), QStringLiteral("program") },
        QStringLiteral("Load a Vole program file on startup."),
        QStringLiteral("file"));
    parser.addOption(programOption);

    QCommandLineOption runOption(
        QStringLiteral("run"),
        QStringLiteral("Start executing the loaded program immediately."));
    parser.addOption(runOption);

    QCommandLineOption screenFormatOption(
        QStringLiteral("screen-format"),
        QStringLiteral("Screen output format: hex, dec or ascii (default: hex)."),
        QStringLiteral("format"));
    parser.addOption(screenFormatOption);

    parser.process(application);

    if (parser.isSet(helpOption))
    {
        const QString helpText = parser.helpText();
        QTextStream(stdout) << helpText << '\n';
        QMessageBox::information(nullptr,
                                 QStringLiteral("Vole Machine Simulator"),
                                 helpText);
        return 0;
    }

    MainWindow window;

    // Fit the default size to the screen (also correct at 125%/150% DPI).
    const QRect available = QApplication::primaryScreen()->availableGeometry();
    window.resize(qMin(1400, available.width() - 20),
                  qMin(900, available.height() - 20));
    window.move(available.center() - window.rect().center());

    window.show();

    if (parser.isSet(screenFormatOption))
    {
        if (!window.setScreenFormat(parser.value(screenFormatOption)))
        {
            QTextStream(stderr)
                << "Unknown screen format: " << parser.value(screenFormatOption)
                << " (expected hex, dec or ascii)\n";
        }
    }

    if (parser.isSet(programOption))
    {
        window.openProgramFile(parser.value(programOption), parser.isSet(runOption));
    }

    return application.exec();
}
