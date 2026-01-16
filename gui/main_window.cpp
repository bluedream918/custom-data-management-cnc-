#include "main_window.h"
#include "../src/render/Viewport3DWidget.h"
#include "../src/ui/ProjectDock.h"
#include "../src/ui/PropertiesDock.h"
#include "../src/ui/ConsoleDock.h"
#include <QApplication>

namespace cnc {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      fileMenu_(nullptr),
      viewMenu_(nullptr),
      simulationMenu_(nullptr),
      toolsMenu_(nullptr),
      helpMenu_(nullptr),
      newProjectAction_(nullptr),
      openProjectAction_(nullptr),
      exitAction_(nullptr),
      showProjectDockAction_(nullptr),
      showPropertiesDockAction_(nullptr),
      showConsoleDockAction_(nullptr),
      viewport3D_(nullptr),
      projectDock_(nullptr),
      propertiesDock_(nullptr),
      consoleDock_(nullptr) {
    
    // Set window properties
    setWindowTitle("CNC Simulation IDE");
    resize(1400, 900);

    // Initialize UI components
    setupMenuBar();
    setupStatusBar();
    setupCentralWidget();
    setupDockWidgets();
    setupViewMenu();
}

void MainWindow::setupMenuBar() {
    // File menu
    fileMenu_ = menuBar()->addMenu("&File");
    
    newProjectAction_ = new QAction("&New Project", this);
    newProjectAction_->setShortcut(QKeySequence::New);
    connect(newProjectAction_, &QAction::triggered, this, &MainWindow::onNewProject);
    fileMenu_->addAction(newProjectAction_);

    openProjectAction_ = new QAction("&Open Project", this);
    openProjectAction_->setShortcut(QKeySequence::Open);
    connect(openProjectAction_, &QAction::triggered, this, &MainWindow::onOpenProject);
    fileMenu_->addAction(openProjectAction_);

    fileMenu_->addSeparator();

    exitAction_ = new QAction("E&xit", this);
    exitAction_->setShortcut(QKeySequence::Quit);
    connect(exitAction_, &QAction::triggered, this, &MainWindow::onExit);
    fileMenu_->addAction(exitAction_);

    // View menu
    viewMenu_ = menuBar()->addMenu("&View");
    // View menu actions will be set up in setupViewMenu()

    // Simulation menu
    simulationMenu_ = menuBar()->addMenu("&Simulation");
    // Simulation menu actions will be added in future steps

    // Tools menu
    toolsMenu_ = menuBar()->addMenu("&Tools");
    // Tools menu actions will be added in future steps

    // Help menu
    helpMenu_ = menuBar()->addMenu("&Help");
    // Help menu actions will be added in future steps
}

void MainWindow::setupStatusBar() {
    statusBar()->showMessage("Ready");
    statusBar()->setEnabled(true);
}

void MainWindow::setupCentralWidget() {
    // Create 3D viewport widget
    viewport3D_ = new Viewport3DWidget(this);
    
    // Set viewport as central widget
    setCentralWidget(viewport3D_);
    
    // Ensure viewport is visible and gets focus
    viewport3D_->setFocusPolicy(Qt::StrongFocus);
    viewport3D_->show();
}

void MainWindow::setupDockWidgets() {
    // Create project dock (left)
    projectDock_ = new ProjectDock(this);
    addDockWidget(Qt::LeftDockWidgetArea, projectDock_);
    
    // Create properties dock (right)
    propertiesDock_ = new PropertiesDock(this);
    addDockWidget(Qt::RightDockWidgetArea, propertiesDock_);
    
    // Create console dock (bottom)
    consoleDock_ = new ConsoleDock(this);
    addDockWidget(Qt::BottomDockWidgetArea, consoleDock_);
}

void MainWindow::setupViewMenu() {
    // Project dock toggle
    showProjectDockAction_ = projectDock_->toggleViewAction();
    showProjectDockAction_->setText("&Project");
    viewMenu_->addAction(showProjectDockAction_);
    
    // Properties dock toggle
    showPropertiesDockAction_ = propertiesDock_->toggleViewAction();
    showPropertiesDockAction_->setText("&Properties");
    viewMenu_->addAction(showPropertiesDockAction_);
    
    // Console dock toggle
    showConsoleDockAction_ = consoleDock_->toggleViewAction();
    showConsoleDockAction_->setText("&Console");
    viewMenu_->addAction(showConsoleDockAction_);
}

void MainWindow::onNewProject() {
    // TODO: Implement new project creation
    statusBar()->showMessage("New Project - Not yet implemented", 2000);
}

void MainWindow::onOpenProject() {
    // TODO: Implement project opening
    statusBar()->showMessage("Open Project - Not yet implemented", 2000);
}

void MainWindow::onExit() {
    QApplication::quit();
}

} // namespace cnc
