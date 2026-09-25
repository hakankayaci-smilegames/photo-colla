#pragma once

#include <QUndoCommand>
#include <QPointF>
#include <QColor>
#include <QVariant>
#include <vector>
#include "../models/slot.h"

namespace PhotoColla::Core {
class CollageDocument;

/**
 * @brief Undoable command for assigning or replacing an image in a slot.
 */
class SetSlotImageCommand : public QUndoCommand {
public:
    SetSlotImageCommand(CollageDocument* doc, int slotIndex, const QString& newPath, const QPixmap& newPixmap, QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;

private:
    CollageDocument* m_doc;
    int m_slotIndex;
    QString m_newPath;
    QPixmap m_newPixmap;
    QString m_oldPath;
    QPixmap m_oldPixmap;
    QPointF m_oldOffset;
    QPointF m_oldScale;
};

/**
 * @brief Undoable command for pan (offset) and zoom (scale) transforms of a slot image.
 */
class TransformSlotImageCommand : public QUndoCommand {
public:
    TransformSlotImageCommand(CollageDocument* doc, int slotIndex, const QPointF& oldOffset, const QPointF& oldScale,
                             const QPointF& newOffset, const QPointF& newScale, QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;
    int id() const override { return 1001; }
    bool mergeWith(const QUndoCommand* other) override;

private:
    CollageDocument* m_doc;
    int m_slotIndex;
    QPointF m_oldOffset;
    QPointF m_oldScale;
    QPointF m_newOffset;
    QPointF m_newScale;
};

/**
 * @brief Undoable command for updating layout styling properties (margins, borders, radii).
 */
class ChangeLayoutPropertyCommand : public QUndoCommand {
public:
    enum class PropertyType {
        Margin,
        Padding,
        BorderRadius,
        BorderWidth,
        BorderColor,
        BackgroundColor
    };

    ChangeLayoutPropertyCommand(CollageDocument* doc, int slotIndex, PropertyType propType,
                               const QVariant& oldValue, const QVariant& newValue,
                               bool applyToAll, QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;

private:
    void applyValue(const QVariant& val);

    CollageDocument* m_doc;
    int m_slotIndex;
    PropertyType m_propType;
    QVariant m_oldValue;
    QVariant m_newValue;
    bool m_applyToAll;
    std::vector<Slot> m_backupSlots; // Full backup in case of apply-to-all
};

} // namespace PhotoColla::Core
