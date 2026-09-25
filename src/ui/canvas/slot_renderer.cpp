#include "slot_renderer.h"
#include <QPen>
#include <QFont>
#include <cmath>

namespace PhotoColla::UI {

void SlotRenderer::renderSlot(
    QPainter& painter,
    const Core::Slot& slot,
    const QSizeF& canvasSize,
    bool isSelected,
    bool isEditing)
{
    QRectF innerRect = slot.calculateInnerRect(canvasSize);
    QPainterPath clipPath = slot.calculateClipPath(canvasSize);

    // Save initial state
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    if (!slot.hasImage()) {
        drawSlotPlaceholder(painter, slot, canvasSize, innerRect, clipPath);
    } else {
        if (isEditing) {
            drawSlotImageEditing(painter, slot, canvasSize, innerRect, clipPath);
        } else {
            drawSlotImageNormal(painter, slot, canvasSize, innerRect, clipPath);
        }
    }

    // Draw slot border stroke
    if (slot.borderWidth() > 0.0) {
        QPen borderPen(slot.borderColor(), slot.borderWidth());
        borderPen.setJoinStyle(Qt::RoundJoin);
        borderPen.setCapStyle(Qt::RoundCap);
        painter.setPen(borderPen);
        painter.setBrush(Qt::NoBrush);
        painter.drawPath(clipPath);
    }

    // Selection & Edit mode outlines
    if (isEditing) {
        QPen editPen(QColor(59, 130, 246), 2.0, Qt::DashLine);
        painter.setPen(editPen);
        painter.setBrush(Qt::NoBrush);
        painter.drawPath(clipPath);
        drawTransformHandles(painter, innerRect);
    } else if (isSelected) {
        QPen selPen(QColor(96, 165, 250), 2.0, Qt::SolidLine);
        painter.setPen(selPen);
        painter.setBrush(Qt::NoBrush);
        painter.drawPath(clipPath);
    }

    painter.restore();
}

void SlotRenderer::drawSlotPlaceholder(
    QPainter& painter,
    const Core::Slot& slot,
    const QSizeF& /*canvasSize*/,
    const QRectF& innerRect,
    const QPainterPath& clipPath)
{
    // Fill slot background
    painter.fillPath(clipPath, slot.slotBackgroundColor());

    // Subtle dashed guide border
    QPen dashPen(QColor(82, 82, 91, 160), 1.5, Qt::DashLine);
    painter.setPen(dashPen);
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(clipPath);

    // Draw plus icon and text centered
    painter.setPen(QColor(161, 161, 170));
    QFont font = painter.font();
    font.setPixelSize(13);
    font.setWeight(QFont::Medium);
    painter.setFont(font);

    QString text = QString("+ %1\n(Double-click to add)").arg(slot.id());
    painter.drawText(innerRect, Qt::AlignCenter, text);
}

void SlotRenderer::drawSlotImageNormal(
    QPainter& painter,
    const Core::Slot& slot,
    const QSizeF& /*canvasSize*/,
    const QRectF& innerRect,
    const QPainterPath& clipPath)
{
    const QPixmap& pixmap = slot.pixmap();
    if (pixmap.isNull()) return;

    painter.save();
    // 100% Opacity clipped strictly to slot boundary
    painter.setClipPath(clipPath);
    painter.setOpacity(1.0);

    QPointF center = innerRect.center() + slot.imageOffset();
    painter.translate(center);
    if (std::abs(slot.imageRotation()) > 0.001) {
        painter.rotate(slot.imageRotation());
    }
    painter.scale(slot.imageScale().x(), slot.imageScale().y());

    QRectF targetRect(-pixmap.width() / 2.0, -pixmap.height() / 2.0, pixmap.width(), pixmap.height());
    painter.drawPixmap(targetRect.toRect(), pixmap);

    painter.restore();
}

void SlotRenderer::drawSlotImageEditing(
    QPainter& painter,
    const Core::Slot& slot,
    const QSizeF& /*canvasSize*/,
    const QRectF& innerRect,
    const QPainterPath& clipPath)
{
    const QPixmap& pixmap = slot.pixmap();
    if (pixmap.isNull()) return;

    QPointF center = innerRect.center() + slot.imageOffset();

    // -------------------------------------------------------------
    // PASS 1: Out-of-bounds preview at 30% alpha (unclipped)
    // -------------------------------------------------------------
    painter.save();
    painter.setOpacity(0.30); // 30% semi-transparent ghost view
    painter.translate(center);
    if (std::abs(slot.imageRotation()) > 0.001) {
        painter.rotate(slot.imageRotation());
    }
    painter.scale(slot.imageScale().x(), slot.imageScale().y());

    QRectF targetRect(-pixmap.width() / 2.0, -pixmap.height() / 2.0, pixmap.width(), pixmap.height());
    painter.drawPixmap(targetRect.toRect(), pixmap);
    painter.restore();

    // -------------------------------------------------------------
    // PASS 2: In-bounds active view at 100% opacity (clipped to slot)
    // -------------------------------------------------------------
    painter.save();
    painter.setClipPath(clipPath);
    painter.setOpacity(1.0); // 100% crisp opaque
    painter.translate(center);
    if (std::abs(slot.imageRotation()) > 0.001) {
        painter.rotate(slot.imageRotation());
    }
    painter.scale(slot.imageScale().x(), slot.imageScale().y());

    painter.drawPixmap(targetRect.toRect(), pixmap);
    painter.restore();
}

void SlotRenderer::drawTransformHandles(QPainter& painter, const QRectF& rect)
{
    const double handleSize = 8.0;
    const double halfSize = handleSize / 2.0;

    const std::vector<QPointF> points = {
        rect.topLeft(),
        rect.topRight(),
        rect.bottomLeft(),
        rect.bottomRight()
    };

    painter.setPen(QPen(QColor(255, 255, 255), 1.5));
    painter.setBrush(QColor(59, 130, 246));

    for (const auto& pt : points) {
        painter.drawRect(QRectF(pt.x() - halfSize, pt.y() - halfSize, handleSize, handleSize));
    }
}

} // namespace PhotoColla::UI
