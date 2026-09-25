#include "slot.h"
#include <algorithm>

namespace PhotoColla::Core {

Slot::Slot()
    : m_id("slot_default")
{
}

Slot::Slot(const QString& id, const QRectF& relativeRect)
    : m_id(id)
    , m_relativeRect(relativeRect)
{
}

QRectF Slot::calculatePixelRect(const QSizeF& canvasSize) const
{
    return QRectF(
        m_relativeRect.x() * canvasSize.width(),
        m_relativeRect.y() * canvasSize.height(),
        m_relativeRect.width() * canvasSize.width(),
        m_relativeRect.height() * canvasSize.height()
    );
}

QRectF Slot::calculateInnerRect(const QSizeF& canvasSize) const
{
    QRectF rawRect = calculatePixelRect(canvasSize);
    double inset = m_margin + m_padding;
    QRectF inner = rawRect.adjusted(inset, inset, -inset, -inset);

    if (inner.width() < 1.0) {
        inner.setWidth(1.0);
    }
    if (inner.height() < 1.0) {
        inner.setHeight(1.0);
    }
    return inner;
}

QPainterPath Slot::calculateClipPath(const QSizeF& canvasSize) const
{
    QRectF inner = calculateInnerRect(canvasSize);
    QPainterPath path;
    double maxRadius = std::min(inner.width(), inner.height()) / 2.0;
    double clampedRadius = std::clamp(m_borderRadius, 0.0, maxRadius);
    path.addRoundedRect(inner, clampedRadius, clampedRadius);
    return path;
}

void Slot::setImage(const QString& path, const QPixmap& pixmap)
{
    m_imagePath = path;
    m_pixmap = pixmap;
    m_imageOffset = QPointF(0.0, 0.0);
    m_imageScale = 1.0;
    m_imageRotation = 0.0;
}

void Slot::clearImage()
{
    m_imagePath.clear();
    m_pixmap = QPixmap();
    m_imageOffset = QPointF(0.0, 0.0);
    m_imageScale = 1.0;
}

bool Slot::containsPoint(const QPointF& canvasPoint, const QSizeF& canvasSize) const
{
    return calculateClipPath(canvasSize).contains(canvasPoint);
}

void Slot::fitImageToSlot(const QSizeF& canvasSize)
{
    if (m_pixmap.isNull()) {
        return;
    }

    QRectF inner = calculateInnerRect(canvasSize);
    double scaleX = inner.width() / static_cast<double>(m_pixmap.width());
    double scaleY = inner.height() / static_cast<double>(m_pixmap.height());

    // Scale to cover completely (Figma / CSS object-fit: cover)
    m_imageScale = std::max(scaleX, scaleY);
    m_imageOffset = QPointF(0.0, 0.0);
}

} // namespace PhotoColla::Core
