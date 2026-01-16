#pragma once

#include <QMainWindow>
#include <QMenuBar>
#include <QStatusBar>
#include <QAction>

// Forward declarations
namespace cnc {
    class Viewport3DWidget;
    class ProjectDock;
    class PropertiesDock;
    class ConsoleDock;
}

namespace cnc {

/**
 * @brief Main window for CNC Simulation IDE
 * 
 * Provides the main application window with menu bar, status bar,
 * and central widget area for the OpenGL viewport and other UI components.
 * 
 * This is a skeleton implementation - rendering and simulation logic
 * will be added in subsequent steps.
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    /**
     * @brief Construct main window
     * @param parent Parent widget (nullptr for top-level window)
     */
    explicit MainWindow(QWidget* parent = nullptr);

    /**
     * @brief Destructor
     */
    ~MainWindow() override = default;

private slots:
    /**
     * @brief Handle New Project action
     */
    void onNewProject();

    /**
     * @brief Handle Open Project action
     */
    void onOpenProject();

    /**
     * @brief Handle Exit action
     */
    void onExit();

private:
    /**
     * @brief Initialize menu bar
     */
    void setupMenuBar();

    /**
     * @brief Initialize status bar
     */
    void setupStatusBar();

    /**
     * @brief Initialize central widget
     */
    void setupCentralWidget();

    /**
     * @brief Initialize dock widgets
     */
    void setupDockWidgets();

    /**
     * @brief Setup View menu actions for dock widgets
     */
    void setupViewMenu();

    // Menu bar
    QMenu* fileMenu_;
    QMenu* viewMenu_;
    QMenu* simulationMenu_;
    QMenu* toolsMenu_;
    QMenu* helpMenu_;

    // File menu actions
    QAction* newProjectAction_;
    QAction* openProjectAction_;
    QAction* exitAction_;

    // View menu actions
    QAction* showProjectDockAction_;
    QAction* showPropertiesDockAction_;
    QAction* showConsoleDockAction_;

    // 3D viewport widget
    Viewport3DWidget* viewport3D_;

    // Dock widgets
    ProjectDock* projectDock_;
    PropertiesDock* propertiesDock_;
    ConsoleDock* consoleDock_;
};

} // namespace cnc
