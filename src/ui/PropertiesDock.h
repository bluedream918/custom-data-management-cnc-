#pragma once

#include <QDockWidget>
#include <QWidget>
#include <QFormLayout>

namespace cnc {

/**
 * @brief Properties dock widget
 * 
 * Displays properties of selected items in a form layout.
 * This is a placeholder implementation for IDE scaffolding.
 */
class PropertiesDock : public QDockWidget {
    Q_OBJECT

public:
    /**
     * @brief Construct properties dock
     * @param parent Parent widget
     */
    explicit PropertiesDock(QWidget* parent = nullptr);

    /**
     * @brief Destructor
     */
    ~PropertiesDock() override = default;

    /**
     * @brief Get the form layout
     */
    QFormLayout* getFormLayout() const { return formLayout_; }

private:
    QWidget* contentWidget_;
    QFormLayout* formLayout_;
};

} // namespace cnc
