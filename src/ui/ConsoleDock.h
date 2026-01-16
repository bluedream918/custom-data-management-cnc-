#pragma once

#include <QDockWidget>
#include <QTextEdit>

namespace cnc {

/**
 * @brief Console dock widget
 * 
 * Displays console output in a read-only text edit.
 * This is a placeholder implementation for IDE scaffolding.
 */
class ConsoleDock : public QDockWidget {
    Q_OBJECT

public:
    /**
     * @brief Construct console dock
     * @param parent Parent widget
     */
    explicit ConsoleDock(QWidget* parent = nullptr);

    /**
     * @brief Destructor
     */
    ~ConsoleDock() override = default;

    /**
     * @brief Get the text edit widget
     */
    QTextEdit* getTextEdit() const { return textEdit_; }

    /**
     * @brief Append text to console
     * @param text Text to append
     */
    void appendText(const QString& text);

public slots:
    /**
     * @brief Clear console
     */
    void clear();

private:
    QTextEdit* textEdit_;
};

} // namespace cnc
