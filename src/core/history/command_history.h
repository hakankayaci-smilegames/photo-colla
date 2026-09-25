#pragma once

#include <QObject>
#include <QUndoStack>
#include <memory>

namespace PhotoColla::Core {

/**
 * @brief Manages undo/redo stacks, action states, and command dispatching.
 */
class CommandHistory : public QObject {
    Q_OBJECT

public:
    explicit CommandHistory(QObject* parent = nullptr);
    ~CommandHistory() override = default;

    void push(QUndoCommand* cmd);
    void undo();
    void redo();
    void clear();

    [[nodiscard]] bool canUndo() const { return m_undoStack->canUndo(); }
    [[nodiscard]] bool canRedo() const { return m_undoStack->canRedo(); }
    [[nodiscard]] QString undoText() const { return m_undoStack->undoText(); }
    [[nodiscard]] QString redoText() const { return m_undoStack->redoText(); }

    QUndoStack* rawStack() { return m_undoStack.get(); }

signals:
    void canUndoChanged(bool canUndo);
    void canRedoChanged(bool canRedo);

private:
    std::unique_ptr<QUndoStack> m_undoStack;
};

} // namespace PhotoColla::Core
