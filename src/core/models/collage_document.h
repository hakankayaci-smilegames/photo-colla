#pragma once

#include <QObject>
#include <QSizeF>
#include <QColor>
#include <vector>
#include "slot.h"

namespace PhotoColla::Core {

/**
 * @brief Manages the collage document data, slot collections, active selection,
 * template loading and document-level styling.
 */
class CollageDocument : public QObject {
    Q_OBJECT

public:
    explicit CollageDocument(QObject* parent = nullptr);
    ~CollageDocument() override = default;

    // Document Dimensions
    [[nodiscard]] QSizeF canvasSize() const { return m_canvasSize; }
    void setCanvasSize(const QSizeF& size);

    // Canvas Background
    [[nodiscard]] QColor backgroundColor() const { return m_backgroundColor; }
    void setBackgroundColor(const QColor& color);

    // Slot Access
    [[nodiscard]] const std::vector<Slot>& slotList() const { return m_slots; }
    [[nodiscard]] std::vector<Slot>& slotList() { return m_slots; }
    [[nodiscard]] size_t slotCount() const { return m_slots.size(); }
    [[nodiscard]] const Slot* slotAt(int index) const;
    [[nodiscard]] Slot* slotAt(int index);

    void addSlot(const Slot& slot);
    void clearSlots();
    void setSlots(const std::vector<Slot>& newSlots);

    // Selection & Edit Mode
    [[nodiscard]] int selectedSlotIndex() const { return m_selectedSlotIndex; }
    void setSelectedSlotIndex(int index);

    [[nodiscard]] int editingSlotIndex() const { return m_editingSlotIndex; }
    void setEditingSlotIndex(int index);
    [[nodiscard]] bool isEditingSlot(int index) const { return m_editingSlotIndex == index && index >= 0; }

    // Hit Testing
    [[nodiscard]] int findSlotAt(const QPointF& canvasPoint) const;

    // Template Engine
    bool loadTemplateFromJson(const QString& templateId, const QString& jsonFilePath = ":/assets/templates/default_templates.json");
    void createGridTemplate(int rows, int cols);

    // Project File (.colla) Support
    bool saveProject(const QString& filePath);
    bool loadProject(const QString& filePath);
    [[nodiscard]] QString currentFilePath() const { return m_currentFilePath; }
    void setCurrentFilePath(const QString& path) { m_currentFilePath = path; }

    // Global Layout Bulk Adjustments
    void setGlobalMargin(double margin);
    void setGlobalPadding(double padding);
    void setGlobalBorderRadius(double radius);
    void setGlobalBorderWidth(double width);
    void setGlobalBorderColor(const QColor& color);

signals:
    void documentChanged();
    void selectionChanged(int index);
    void editModeChanged(int index);
    void templateLoaded(const QString& templateId);

private:
    QSizeF m_canvasSize{1200.0, 1200.0};
    QColor m_backgroundColor{20, 20, 22, 255};
    std::vector<Slot> m_slots;
    int m_selectedSlotIndex{-1};
    int m_editingSlotIndex{-1};
    QString m_currentFilePath;
};

} // namespace PhotoColla::Core
