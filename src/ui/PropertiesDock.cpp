#include "PropertiesDock.h"
#include <QLabel>

namespace cnc {

PropertiesDock::PropertiesDock(QWidget* parent)
    : QDockWidget(parent),
      contentWidget_(nullptr),
      formLayout_(nullptr) {
    
    setWindowTitle("Properties");
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    
    // Create content widget with form layout
    contentWidget_ = new QWidget(this);
    formLayout_ = new QFormLayout(contentWidget_);
    formLayout_->setContentsMargins(10, 10, 10, 10);
    formLayout_->setSpacing(10);
    
    // Placeholder: Add a sample property
    QLabel* placeholderLabel = new QLabel("No selection", contentWidget_);
    formLayout_->addRow("Status:", placeholderLabel);
    
    // Set as dock widget content
    setWidget(contentWidget_);
}

} // namespace cnc
