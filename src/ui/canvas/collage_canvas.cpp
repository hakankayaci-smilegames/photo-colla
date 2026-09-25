#include "collage_canvas.h"
#include "slot_renderer.h"
#include "../../core/commands/collage_commands.h"
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QUrl>
#include <QFileDialog>
#include <algorithm>
#include <cmath>

namespace PhotoColla::UI {

CollageCanvas::CollageCanvas(Core::CollageDocument* document, Core::CommandHistory* history, QWidget* parent)
    : QWidget(parent)
    , m_document(document)
    , m_history(history)
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    setAcceptDrops(true);

    connect(m_document, &Core::CollageDocument::documentChanged, this, QOverload<>::of(&CollageCanvas::update));
    connect(m_document, &Core::CollageDocument::selectionChanged, this, [this](int) { update(); });
    connect(m_document, &Core::CollageDocument::editModeChanged, this, [this](int) { update(); });
}

void CollageCanvas::zoomIn()
{
    m_zoom = std::clamp(m_zoom * 1.15, 0.1, 10.0);
    emit zoomChanged(m_zoom);
    update();
}

void CollageCanvas::zoomOut()
{
    m_zoom = std::clamp(m_zoom / 1.15, 0.1, 10.0);
    emit zoomChanged(m_zoom);
    update();
}

void CollageCanvas::resetZoom()
{
    m_zoom = 1.0;
    emit zoomChanged(m_zoom);
    update();
}

void CollageCanvas::fitToScreen()
{
    if (!m_document) return;

    QSizeF docSize = m_document->canvasSize();
    if (docSize.isEmpty() || width() <= 0 || height() <= 0) return;

    double padding = 40.0;
    double availW = std::max(width() - padding * 2, 50.0);
    double availH = std::max(height() - padding * 2, 50.0);

    double scaleX = availW / docSize.width();
    double scaleY = availH / docSize.height();
    m_zoom = std::clamp(std::min(scaleX, scaleY), 0.05, 5.0);

    // Center document
    double docViewW = docSize.width() * m_zoom;
    double docViewH = docSize.height() * m_zoom;
    m_panOffset = QPointF((width() - docViewW) / 2.0, (height() - docViewH) / 2.0);

    emit zoomChanged(m_zoom);
    update();
}

QPointF CollageCanvas::mapViewportToDocument(const QPointF& viewPos) const
{
    return (viewPos - m_panOffset) / m_zoom;
}

QPointF CollageCanvas::mapDocumentToViewport(const QPointF& docPos) const
{
    return (docPos * m_zoom) + m_panOffset;
}

QRectF CollageCanvas::documentRectInViewport() const
{
    QSizeF docSize = m_document ? m_document->canvasSize() : QSizeF(1000, 1000);
    return QRectF(m_panOffset, docSize * m_zoom);
}

void CollageCanvas::resizeEvent(QResizeEvent* /*event*/)
{
    // If opening or centered for first time
    static bool firstShow = true;
    if (firstShow && width() > 100 && height() > 100) {
        fitToScreen();
        firstShow = false;
    }
}

void CollageCanvas::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    // 1. Draw Viewport Background (Studio Slate Neutral)
    painter.fillRect(rect(), QColor(24, 24, 27));

    if (!m_document) return;

    QSizeF docSize = m_document->canvasSize();
    QRectF docViewRect = documentRectInViewport();

    // 2. Draw Subtle Drop Shadow for the Canvas Document
    painter.save();
    QRectF shadowRect = docViewRect.adjusted(-2, -2, 4, 4);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 80));
    painter.drawRoundedRect(shadowRect, 4, 4);
    painter.restore();

    // 3. Setup Document Transform Matrix
    painter.save();
    painter.translate(m_panOffset);
    painter.scale(m_zoom, m_zoom);

    // 4. Fill Document Background
    painter.fillRect(QRectF(QPointF(0, 0), docSize), m_document->backgroundColor());

    // 5. Render Slots
    const auto& slotItems = m_document->slotList();
    int activeEdit = m_document->editingSlotIndex();
    int activeSel = m_document->selectedSlotIndex();

    // Pass 1: Render all non-editing slots first
    for (size_t i = 0; i < slotItems.size(); ++i) {
        if (static_cast<int>(i) == activeEdit) continue;
        bool isSel = (static_cast<int>(i) == activeSel);
        SlotRenderer::renderSlot(painter, slotItems[i], docSize, isSel, false);
    }

    // Pass 2: Render active editing slot last so out-of-bounds alpha floats cleanly on top
    if (activeEdit >= 0 && activeEdit < static_cast<int>(slotItems.size())) {
        SlotRenderer::renderSlot(painter, slotItems[static_cast<size_t>(activeEdit)], docSize, true, true);
    }

    painter.restore();
}

void CollageCanvas::mousePressEvent(QMouseEvent* event)
{
    m_lastMousePos = event->position();

    // Middle button or Space+Left -> Pan Canvas
    if (event->button() == Qt::MiddleButton || (m_spacePressed && event->button() == Qt::LeftButton)) {
        m_dragState = DragState::PanningCanvas;
        setCursor(Qt::ClosedHandCursor);
        return;
    }

    if (event->button() == Qt::LeftButton) {
        QPointF docPos = mapViewportToDocument(event->position());
        int clickedSlot = m_document->findSlotAt(docPos);

        if (clickedSlot != -1) {
            int currentEdit = m_document->editingSlotIndex();
            if (currentEdit == clickedSlot) {
                // Dragging image inside active slot
                m_dragState = DragState::TransformingSlotImage;
                if (auto* slot = m_document->slotAt(clickedSlot)) {
                    m_dragStartOffset = slot->imageOffset();
                    m_dragStartScale = slot->imageScale();
                }
                setCursor(Qt::SizeAllCursor);
            } else {
                // Select slot
                m_document->setSelectedSlotIndex(clickedSlot);
            }
        } else {
            // Click outside clears selection and edit mode
            m_document->setSelectedSlotIndex(-1);
            m_document->setEditingSlotIndex(-1);
        }
    }
}

void CollageCanvas::mouseMoveEvent(QMouseEvent* event)
{
    QPointF currentPos = event->position();
    QPointF delta = currentPos - m_lastMousePos;
    m_lastMousePos = currentPos;

    QPointF docPos = mapViewportToDocument(currentPos);
    emit cursorCoordinatesChanged(docPos);

    if (m_dragState == DragState::PanningCanvas) {
        m_panOffset += delta;
        update();
        return;
    }

    if (m_dragState == DragState::TransformingSlotImage) {
        int editIndex = m_document->editingSlotIndex();
        if (auto* slot = m_document->slotAt(editIndex)) {
            // Adjust offset relative to canvas zoom
            QPointF newOffset = slot->imageOffset() + (delta / m_zoom);
            slot->setImageOffset(newOffset);
            update();
        }
        return;
    }

    // Cursor feedback
    if (m_spacePressed) {
        setCursor(Qt::OpenHandCursor);
    } else {
        int hoverSlot = m_document->findSlotAt(docPos);
        if (hoverSlot != -1) {
            setCursor(m_document->isEditingSlot(hoverSlot) ? Qt::SizeAllCursor : Qt::PointingHandCursor);
        } else {
            setCursor(Qt::ArrowCursor);
        }
    }
}

void CollageCanvas::mouseReleaseEvent(QMouseEvent* /*event*/)
{
    if (m_dragState == DragState::PanningCanvas) {
        m_dragState = DragState::None;
        setCursor(m_spacePressed ? Qt::OpenHandCursor : Qt::ArrowCursor);
        return;
    }

    if (m_dragState == DragState::TransformingSlotImage) {
        int editIndex = m_document->editingSlotIndex();
        if (auto* slot = m_document->slotAt(editIndex)) {
            QPointF currentOffset = slot->imageOffset();
            double currentScale = slot->imageScale();

            // Push undo command if moved
            if (currentOffset != m_dragStartOffset || currentScale != m_dragStartScale) {
                m_history->push(new Core::TransformSlotImageCommand(
                    m_document, editIndex, m_dragStartOffset, m_dragStartScale, currentOffset, currentScale
                ));
            }
        }
        m_dragState = DragState::None;
        setCursor(Qt::ArrowCursor);
    }
}

void CollageCanvas::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        QPointF docPos = mapViewportToDocument(event->position());
        int slotIndex = m_document->findSlotAt(docPos);

        if (slotIndex != -1) {
            auto* slot = m_document->slotAt(slotIndex);
            if (slot && !slot->hasImage()) {
                // Request image loading
                emit slotImageRequested(slotIndex);
            } else {
                // Toggle edit mode for fine-tuning pan and alpha view
                if (m_document->isEditingSlot(slotIndex)) {
                    m_document->setEditingSlotIndex(-1);
                } else {
                    m_document->setEditingSlotIndex(slotIndex);
                }
            }
        }
    }
}

void CollageCanvas::wheelEvent(QWheelEvent* event)
{
    QPointF mousePos = event->position();
    QPointF docPos = mapViewportToDocument(mousePos);
    int activeEdit = m_document->editingSlotIndex();

    // If hovering over or editing the active slot, scale the image directly
    if (activeEdit != -1 && m_document->slotAt(activeEdit)->containsPoint(docPos, m_document->canvasSize())) {
        auto* slot = m_document->slotAt(activeEdit);
        double oldScale = slot->imageScale();
        double factor = (event->angleDelta().y() > 0) ? 1.08 : 0.92;
        double newScale = std::clamp(oldScale * factor, 0.05, 20.0);

        slot->setImageScale(newScale);
        m_history->push(new Core::TransformSlotImageCommand(
            m_document, activeEdit, slot->imageOffset(), oldScale, slot->imageOffset(), newScale
        ));
        update();
        event->accept();
        return;
    }

    // Otherwise zoom canvas around mouse cursor
    double factor = (event->angleDelta().y() > 0) ? 1.15 : (1.0 / 1.15);
    double targetZoom = std::clamp(m_zoom * factor, 0.05, 10.0);

    // Anchor zoom around cursor position
    m_panOffset = mousePos - (mousePos - m_panOffset) * (targetZoom / m_zoom);
    m_zoom = targetZoom;

    emit zoomChanged(m_zoom);
    update();
    event->accept();
}

void CollageCanvas::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Space && !event->isAutoRepeat()) {
        m_spacePressed = true;
        setCursor(Qt::OpenHandCursor);
        return;
    }

    if (event->key() == Qt::Key_Escape) {
        if (m_document->editingSlotIndex() != -1) {
            m_document->setEditingSlotIndex(-1);
        } else if (m_document->selectedSlotIndex() != -1) {
            m_document->setSelectedSlotIndex(-1);
        }
        return;
    }

    if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) {
        int sel = m_document->selectedSlotIndex();
        if (sel != -1) {
            m_history->push(new Core::SetSlotImageCommand(m_document, sel, QString(), QPixmap()));
        }
        return;
    }

    QWidget::keyPressEvent(event);
}

void CollageCanvas::keyReleaseEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Space && !event->isAutoRepeat()) {
        m_spacePressed = false;
        if (m_dragState != DragState::PanningCanvas) {
            setCursor(Qt::ArrowCursor);
        }
        return;
    }
    QWidget::keyReleaseEvent(event);
}

void CollageCanvas::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void CollageCanvas::dragMoveEvent(QDragMoveEvent* event)
{
    event->acceptProposedAction();
}

void CollageCanvas::dropEvent(QDropEvent* event)
{
    const QMimeData* mimeData = event->mimeData();
    if (mimeData->hasUrls() && !mimeData->urls().isEmpty()) {
        QString filePath = mimeData->urls().first().toLocalFile();
        QPixmap pixmap(filePath);
        if (!pixmap.isNull()) {
            QPointF docPos = mapViewportToDocument(event->position());
            int targetSlot = m_document->findSlotAt(docPos);
            if (targetSlot != -1) {
                m_history->push(new Core::SetSlotImageCommand(m_document, targetSlot, filePath, pixmap));
            }
        }
    }
    event->acceptProposedAction();
}

} // namespace PhotoColla::UI
