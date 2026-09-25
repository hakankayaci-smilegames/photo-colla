#pragma once

#include <QPainter>
#include <QSizeF>
#include "../../core/models/slot.h"

namespace PhotoColla::UI {

/**
 * @brief High-performance hardware-accelerated renderer for collage slots,
 * implementing dual-pass clipping mask and out-of-bounds 30% alpha visual preview.
 */
class SlotRenderer {
public:
    static void renderSlot(
        QPainter& painter,
        const Core::Slot& slot,
        const QSizeF& canvasSize,
        bool isSelected,
        bool isEditing
    );

private:
    static void drawSlotPlaceholder(
        QPainter& painter,
        const Core::Slot& slot,
        const QSizeF& canvasSize,
        const QRectF& innerRect,
        const QPainterPath& clipPath
    );

    static void drawSlotImageNormal(
        QPainter& painter,
        const Core::Slot& slot,
        const QSizeF& canvasSize,
        const QRectF& innerRect,
        const QPainterPath& clipPath
    );

    static void drawSlotImageEditing(
        QPainter& painter,
        const Core::Slot& slot,
        const QSizeF& canvasSize,
        const QRectF& innerRect,
        const QPainterPath& clipPath
    );

    static void drawTransformHandles(
        QPainter& painter,
        const QRectF& rect
    );
};

} // namespace PhotoColla::UI
