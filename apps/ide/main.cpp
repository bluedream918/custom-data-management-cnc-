// IDE Application entry point

#include <QApplication>
#include "main_window.h"

int main(int argc, char* argv[]) {
    // Set OpenGL attribute before creating QApplication
    // This ensures desktop OpenGL is used (important for macOS)
    QApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
    
    QApplication app(argc, argv);

    // Set application properties
    app.setApplicationName("CNC Simulation IDE");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("CNC IDE");

    // Create and show main window
    cnc::MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}
