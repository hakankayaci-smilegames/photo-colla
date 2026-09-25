#include "collage_commands.h"
#include "../models/collage_document.h"

namespace PhotoColla::Core {

// --- SetSlotImageCommand ---
SetSlotImageCommand::SetSlotImageCommand(CollageDocument* doc, int slotIndex, const QString& newPath, const QPixmap& newPixmap, QUndoCommand* parent)
    : QUndoCommand(parent)
    , m_doc(doc)
    , m_slotIndex(slotIndex)
    , m_newPath(newPath)
    , m_newPixmap(newPixmap)
    , m_oldScale(1.0)
{
    setText(QObject::tr("Set Slot Image"));
    if (auto* slot = m_doc->slotAt(m_slotIndex)) {
        m_oldPath = slot->imagePath();
        m_oldPixmap = slot->pixmap();
        m_oldOffset = slot->imageOffset();
        m_oldScale = slot->imageScale();
    }
}

void SetSlotImageCommand::undo()
{
    if (auto* slot = m_doc->slotAt(m_slotIndex)) {
        slot->setImage(m_oldPath, m_oldPixmap);
        slot->setImageOffset(m_oldOffset);
        slot->setImageScale(m_oldScale);
        emit m_doc->documentChanged();
    }
}

void SetSlotImageCommand::redo()
{
    if (auto* slot = m_doc->slotAt(m_slotIndex)) {
        slot->setImage(m_newPath, m_newPixmap);
        slot->fitImageToSlot(m_doc->canvasSize());
        emit m_doc->documentChanged();
    }
}

// --- TransformSlotImageCommand ---
TransformSlotImageCommand::TransformSlotImageCommand(CollageDocument* doc, int slotIndex,
                                                     const QPointF& oldOffset, double oldScale,
                                                     const QPointF& newOffset, double newScale,
                                                     QUndoCommand* parent)
    : QUndoCommand(parent)
    , m_doc(doc)
    , m_slotIndex(slotIndex)
    , m_oldOffset(oldOffset)
    , m_oldScale(oldScale)
    , m_newOffset(newOffset)
    , m_newScale(newScale)
{
    setText(QObject::tr("Transform Image"));
}

void TransformSlotImageCommand::undo()
{
    if (auto* slot = m_doc->slotAt(m_slotIndex)) {
        slot->setImageOffset(m_oldOffset);
        slot->setImageScale(m_oldScale);
        emit m_doc->documentChanged();
    }
}

void TransformSlotImageCommand::redo()
{
    if (auto* slot = m_doc->slotAt(m_slotIndex)) {
        slot->setImageOffset(m_newOffset);
        slot->setImageScale(m_newScale);
        emit m_doc->documentChanged();
    }
}

bool TransformSlotImageCommand::mergeWith(const QUndoCommand* other)
{
    if (other->id() != id()) return false;
    const auto* cmd = static_cast<const TransformSlotImageCommand*>(other);
    if (cmd->m_slotIndex != m_slotIndex) return false;

    m_newOffset = cmd->m_newOffset;
    m_newScale = cmd->m_newScale;
    return true;
}

// --- ChangeLayoutPropertyCommand ---
ChangeLayoutPropertyCommand::ChangeLayoutPropertyCommand(CollageDocument* doc, int slotIndex, PropertyType propType,
                                                         const QVariant& oldValue, const QVariant& newValue,
                                                         bool applyToAll, QUndoCommand* parent)
    : QUndoCommand(parent)
    , m_doc(doc)
    , m_slotIndex(slotIndex)
    , m_propType(propType)
    , m_oldValue(oldValue)
    , m_newValue(newValue)
    , m_applyToAll(applyToAll)
    , m_backupSlots(doc->slotList())
{
    setText(QObject::tr("Change Layout Property"));
}

void ChangeLayoutPropertyCommand::applyValue(const QVariant& val)
{
    if (m_propType == PropertyType::BackgroundColor) {
        m_doc->setBackgroundColor(val.value<QColor>());
        return;
    }

    auto applyToSlot = [this, &val](Slot& slot) {
        switch (m_propType) {
            case PropertyType::Margin:
                slot.setMargin(val.toDouble());
                break;
            case PropertyType::Padding:
                slot.setPadding(val.toDouble());
                break;
            case PropertyType::BorderRadius:
                slot.setBorderRadius(val.toDouble());
                break;
            case PropertyType::BorderWidth:
                slot.setBorderWidth(val.toDouble());
                break;
            case PropertyType::BorderColor:
                slot.setBorderColor(val.value<QColor>());
                break;
            default:
                break;
        }
    };

    if (m_applyToAll) {
        for (auto& s : m_doc->slotList()) {
            applyToSlot(s);
        }
    } else if (auto* s = m_doc->slotAt(m_slotIndex)) {
        applyToSlot(*s);
    }
    emit m_doc->documentChanged();
}

void ChangeLayoutPropertyCommand::undo()
{
    if (m_applyToAll && m_propType != PropertyType::BackgroundColor) {
        m_doc->setSlots(m_backupSlots);
    } else {
        applyValue(m_oldValue);
    }
}

void ChangeLayoutPropertyCommand::redo()
{
    applyValue(m_newValue);
}

} // namespace PhotoColla::Core
