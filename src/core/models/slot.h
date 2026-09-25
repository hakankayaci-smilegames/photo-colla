#pragma once

#include <QString>
#include <QRectF>
#include <QSizeF>
#include <QPointF>
#include <QColor>
#include <QPainterPath>
#include <QPixmap>
#include <memory>

namespace PhotoColla::Core {

/**
 * @brief Represents a single collage slot/cell with its geometry, assigned image,
 * and visual styling properties.
 */
class Slot {
public:
    Slot();
    explicit Slot(const QString& id, const QRectF& relativeRect);

    // Identity
    [[nodiscard]] QString id() const { return m_id; }
    void setId(const QString& id) { m_id = id; }

    // Geometry (Normalized coordinates 0.0 - 1.0 relative to canvas size)
    [[nodiscard]] QRectF relativeRect() const { return m_relativeRect; }
    void setRelativeRect(const QRectF& rect) { m_relativeRect = rect; }

    [[nodiscard]] QRectF calculatePixelRect(const QSizeF& canvasSize) const;
    [[nodiscard]] QRectF calculateInnerRect(const QSizeF& canvasSize) const;
    [[nodiscard]] QPainterPath calculateClipPath(const QSizeF& canvasSize) const;

    // Image & Transformation
    [[nodiscard]] bool hasImage() const { return !m_pixmap.isNull(); }
    [[nodiscard]] QString imagePath() const { return m_imagePath; }
    [[nodiscard]] const QPixmap& pixmap() const { return m_pixmap; }
    void setImage(const QString& path, const QPixmap& pixmap);
    void clearImage();

    [[nodiscard]] QPointF imageOffset() const { return m_imageOffset; }
    void setImageOffset(const QPointF& offset) { m_imageOffset = offset; }

    [[nodiscard]] double imageScale() const { return m_imageScale; }
    void setImageScale(double scale) { m_imageScale = scale; }

    [[nodiscard]] double imageRotation() const { return m_imageRotation; }
    void setImageRotation(double degrees) { m_imageRotation = degrees; }

    // Styling properties
    [[nodiscard]] double margin() const { return m_margin; }
    void setMargin(double margin) { m_margin = margin; }

    [[nodiscard]] double padding() const { return m_padding; }
    void setPadding(double padding) { m_padding = padding; }

    [[nodiscard]] double borderRadius() const { return m_borderRadius; }
    void setBorderRadius(double radius) { m_borderRadius = radius; }

    [[nodiscard]] double borderWidth() const { return m_borderWidth; }
    void setBorderWidth(double width) { m_borderWidth = width; }

    [[nodiscard]] QColor borderColor() const { return m_borderColor; }
    void setBorderColor(const QColor& color) { m_borderColor = color; }

    [[nodiscard]] QColor slotBackgroundColor() const { return m_slotBackgroundColor; }
    void setSlotBackgroundColor(const QColor& color) { m_slotBackgroundColor = color; }

    // Hit testing
    [[nodiscard]] bool containsPoint(const QPointF& canvasPoint, const QSizeF& canvasSize) const;

    // Reset image transform to best fit inside inner rect
    void fitImageToSlot(const QSizeF& canvasSize);

private:
    QString m_id;
    QRectF m_relativeRect{0.0, 0.0, 1.0, 1.0}; // Normalized 0..1

    // Image data & transform
    QString m_imagePath;
    QPixmap m_pixmap;
    QPointF m_imageOffset{0.0, 0.0}; // Relative translation inside inner rect
    double m_imageScale{1.0};        // Zoom multiplier
    double m_imageRotation{0.0};     // Angle in degrees

    // Border & Spacing
    double m_margin{6.0};            // Outer gap in pixels
    double m_padding{0.0};           // Inner padding in pixels
    double m_borderRadius{12.0};     // Corner curvature in pixels
    double m_borderWidth{2.0};       // Border stroke in pixels
    QColor m_borderColor{60, 60, 65, 255};
    QColor m_slotBackgroundColor{30, 30, 34, 255};
};

} // namespace PhotoColla::Core
