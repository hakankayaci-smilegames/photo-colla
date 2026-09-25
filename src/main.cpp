#include <QApplication>
#include <QFile>
#include <QDebug>
#include <memory>

#include "ui/splash/splash_screen.h"
#include "ui/main_window.h"
#include "ui/dialogs/welcome_dialog.h"

int main(int argc, char *argv[])
{
    // High-DPI scaling is enabled by default in Qt 6
    QApplication app(argc, argv);
    app.setApplicationName("PhotoColla");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("PhotoCollaStudio");

    // Load Studio Dark Theme from Qt resources
    QFile themeFile(":/assets/styles/dark_theme.qss");
    if (themeFile.open(QFile::ReadOnly | QFile::Text)) {
        app.setStyleSheet(QString::fromUtf8(themeFile.readAll()));
    } else {
        qWarning() << "Warning: Dark theme stylesheet could not be loaded from resources.";
    }

    // Modern Asynchronous Splash Screen
    auto* splash = new PhotoColla::UI::SplashScreen();
    auto* mainWindow = new PhotoColla::UI::MainWindow();

    QObject::connect(splash, &PhotoColla::UI::SplashScreen::initializationComplete, [splash, mainWindow]() {
        splash->hide();

        PhotoColla::UI::WelcomeDialog welcome;
        if (welcome.exec() == QDialog::Accepted) {
            if (welcome.userAction() == PhotoColla::UI::WelcomeDialog::ResultAction::OpenCollage) {
                mainWindow->loadProject(welcome.selectedProjectPath());
            }
            mainWindow->show();
        } else {
            QApplication::quit();
        }
        
        splash->deleteLater();
    });

    splash->startInitialization();

    return app.exec();
}
