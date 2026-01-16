#pragma once

#include <QDockWidget>
#include <QTreeWidget>

namespace cnc {

/**
 * @brief Project dock widget
 * 
 * Displays the project structure in a tree view.
 * This is a placeholder implementation for IDE scaffolding.
 */
class ProjectDock : public QDockWidget {
    Q_OBJECT

public:
    /**
     * @brief Construct project dock
     * @param parent Parent widget
     */
    explicit ProjectDock(QWidget* parent = nullptr);

    /**
     * @brief Destructor
     */
    ~ProjectDock() override = default;

    /**
     * @brief Get the tree widget
     */
    QTreeWidget* getTreeWidget() const { return treeWidget_; }

private:
    QTreeWidget* treeWidget_;
};

} // namespace cnc
