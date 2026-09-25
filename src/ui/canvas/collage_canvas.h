#pragma once

#include <QWidget>
#include <QPointF>
#include <memory>
#include "../../core/models/collage_document.h"
#include "../../core/history/command_history.h"

namespace PhotoColla::UI {

/**
 * @brief Interactive GPU-backed viewport supporting infinite canvas panning,
 * mouse-centered zooming, drag-and-drop, and in-slot image transformation with
 * undoable actions.
 */
class CollageCanvas : public QWidget {
    Q_OBJECT

public:
    explicit CollageCanvas(Core::CollageDocument* document, Core::CommandHistory* history, QWidget* parent = nullptr);
    ~CollageCanvas() override = default;

    // Viewport Controls
    void zoomIn();
    void zoomOut();
    void resetZoom();
    void fitToScreen();

    [[nodiscard]] double zoomLevel() const { return m_zoom; }

signals:
    void zoomChanged(double zoom);
    void cursorCoordinatesChanged(const QPointF& docPos);
    void slotImageRequested(int slotIndex);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

    // Drag and Drop
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private:
    [[nodiscard]] QPointF mapViewportToDocument(const QPointF& viewPos) const;
    [[nodiscard]] QPointF mapDocumentToViewport(const QPointF& docPos) const;
    [[nodiscard]] QRectF documentRectInViewport() const;

    enum class DragState {
        None,
        PanningCanvas,
        TransformingSlotImage,
        DraggingSlotToSlot
    };

    Core::CollageDocument* m_document;
    Core::CommandHistory* m_history;

    // Viewport transforms
    double m_zoom{1.0};
    QPointF m_panOffset{0.0, 0.0};
    bool m_spacePressed{false};

    // Drag interaction tracking
    DragState m_dragState{DragState::None};
    QPointF m_dragStartPos;
    QPointF m_lastMousePos;
    QPointF m_dragStartOffset;
    double m_dragStartScale{1.0};

    // Drop target feedback
    int m_dropTargetSlotIndex{-1};
    int m_dragSourceSlotIndex{-1};
};

} // namespace PhotoColla::UI
