#include "command_history.h"

namespace PhotoColla::Core {

CommandHistory::CommandHistory(QObject* parent)
    : QObject(parent)
    , m_undoStack(std::make_unique<QUndoStack>(this))
{
    connect(m_undoStack.get(), &QUndoStack::canUndoChanged, this, &CommandHistory::canUndoChanged);
    connect(m_undoStack.get(), &QUndoStack::canRedoChanged, this, &CommandHistory::canRedoChanged);
}

void CommandHistory::push(QUndoCommand* cmd)
{
    if (cmd) {
        m_undoStack->push(cmd);
    }
}

void CommandHistory::undo()
{
    m_undoStack->undo();
}

void CommandHistory::redo()
{
    m_undoStack->redo();
}

void CommandHistory::clear()
{
    m_undoStack->clear();
}

} // namespace PhotoColla::Core
