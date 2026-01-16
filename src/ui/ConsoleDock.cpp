#include "ConsoleDock.h"

namespace cnc {

ConsoleDock::ConsoleDock(QWidget* parent)
    : QDockWidget(parent),
      textEdit_(nullptr) {
    
    setWindowTitle("Console");
    setAllowedAreas(Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);
    
    // Create read-only text edit
    textEdit_ = new QTextEdit(this);
    textEdit_->setReadOnly(true);
    textEdit_->setFont(QFont("Courier", 10));
    
    // Placeholder: Add welcome message
    textEdit_->append("CNC Simulation IDE Console");
    textEdit_->append("Ready.");
    
    // Set as dock widget content
    setWidget(textEdit_);
}

void ConsoleDock::appendText(const QString& text) {
    if (textEdit_) {
        textEdit_->append(text);
    }
}

void ConsoleDock::clear() {
    if (textEdit_) {
        textEdit_->clear();
    }
}

} // namespace cnc
