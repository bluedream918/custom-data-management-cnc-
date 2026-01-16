#include "ProjectDock.h"

namespace cnc {

ProjectDock::ProjectDock(QWidget* parent)
    : QDockWidget(parent),
      treeWidget_(nullptr) {
    
    setWindowTitle("Project");
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    
    // Create tree widget
    treeWidget_ = new QTreeWidget(this);
    treeWidget_->setHeaderLabel("Project Structure");
    
    // Set as dock widget content
    setWidget(treeWidget_);
    
    // Placeholder: Add a root item
    QTreeWidgetItem* rootItem = new QTreeWidgetItem(treeWidget_);
    rootItem->setText(0, "Project");
    treeWidget_->expandItem(rootItem);
}

} // namespace cnc
