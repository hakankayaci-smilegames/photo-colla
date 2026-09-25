#include "collage_document.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

namespace PhotoColla::Core {

CollageDocument::CollageDocument(QObject* parent)
    : QObject(parent)
{
    // Initialize with a default 2x2 grid layout
    createGridTemplate(2, 2);
}

void CollageDocument::setCanvasSize(const QSizeF& size)
{
    if (m_canvasSize != size) {
        m_canvasSize = size;
        emit documentChanged();
    }
}

void CollageDocument::setBackgroundColor(const QColor& color)
{
    if (m_backgroundColor != color) {
        m_backgroundColor = color;
        emit documentChanged();
    }
}

const Slot* CollageDocument::slotAt(int index) const
{
    if (index >= 0 && index < static_cast<int>(m_slots.size())) {
        return &m_slots[static_cast<size_t>(index)];
    }
    return nullptr;
}

Slot* CollageDocument::slotAt(int index)
{
    if (index >= 0 && index < static_cast<int>(m_slots.size())) {
        return &m_slots[static_cast<size_t>(index)];
    }
    return nullptr;
}

void CollageDocument::addSlot(const Slot& slot)
{
    m_slots.push_back(slot);
    emit documentChanged();
}

void CollageDocument::clearSlots()
{
    m_slots.clear();
    m_selectedSlotIndex = -1;
    m_editingSlotIndex = -1;
    emit selectionChanged(-1);
    emit editModeChanged(-1);
    emit documentChanged();
}

void CollageDocument::setSlots(const std::vector<Slot>& newSlots)
{
    m_slots = newSlots;
    m_selectedSlotIndex = -1;
    m_editingSlotIndex = -1;
    emit selectionChanged(-1);
    emit editModeChanged(-1);
    emit documentChanged();
}

void CollageDocument::setSelectedSlotIndex(int index)
{
    if (index < -1 || index >= static_cast<int>(m_slots.size())) {
        index = -1;
    }
    if (m_selectedSlotIndex != index) {
        m_selectedSlotIndex = index;
        if (m_editingSlotIndex != -1 && m_editingSlotIndex != index) {
            setEditingSlotIndex(-1);
        }
        emit selectionChanged(m_selectedSlotIndex);
        emit documentChanged();
    }
}

void CollageDocument::setEditingSlotIndex(int index)
{
    if (index < -1 || index >= static_cast<int>(m_slots.size())) {
        index = -1;
    }
    if (m_editingSlotIndex != index) {
        m_editingSlotIndex = index;
        if (m_editingSlotIndex != -1) {
            m_selectedSlotIndex = m_editingSlotIndex;
            emit selectionChanged(m_selectedSlotIndex);
        }
        emit editModeChanged(m_editingSlotIndex);
        emit documentChanged();
    }
}

int CollageDocument::findSlotAt(const QPointF& canvasPoint) const
{
    for (int i = static_cast<int>(m_slots.size()) - 1; i >= 0; --i) {
        if (m_slots[static_cast<size_t>(i)].containsPoint(canvasPoint, m_canvasSize)) {
            return i;
        }
    }
    return -1;
}

void CollageDocument::createGridTemplate(int rows, int cols)
{
    if (rows <= 0 || cols <= 0) return;

    m_slots.clear();
    double cellW = 1.0 / static_cast<double>(cols);
    double cellH = 1.0 / static_cast<double>(rows);

    int idCount = 1;
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            QRectF relRect(c * cellW, r * cellH, cellW, cellH);
            Slot slot(QString("slot_%1").arg(idCount++), relRect);
            m_slots.push_back(slot);
        }
    }

    m_selectedSlotIndex = -1;
    m_editingSlotIndex = -1;
    emit selectionChanged(-1);
    emit editModeChanged(-1);
    emit documentChanged();
}

bool CollageDocument::loadTemplateFromJson(const QString& templateId, const QString& jsonFilePath)
{
    QFile file(jsonFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open template file:" << jsonFilePath;
        return false;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) {
        qWarning() << "Invalid template JSON format";
        return false;
    }

    QJsonArray templates = doc.object()["templates"].toArray();
    for (const auto& item : templates) {
        QJsonObject tObj = item.toObject();
        if (tObj["id"].toString() == templateId) {
            QJsonArray slotsArray = tObj["slots"].toArray();
            std::vector<Slot> loadedSlots;
            int count = 1;
            for (const auto& slotVal : slotsArray) {
                QJsonObject sObj = slotVal.toObject();
                double x = sObj["x"].toDouble(0.0);
                double y = sObj["y"].toDouble(0.0);
                double w = sObj["width"].toDouble(0.5);
                double h = sObj["height"].toDouble(0.5);
                Slot slot(QString("slot_%1").arg(count++), QRectF(x, y, w, h));
                loadedSlots.push_back(slot);
            }
            setSlots(loadedSlots);
            emit templateLoaded(templateId);
            return true;
        }
    }

    qWarning() << "Template ID not found:" << templateId;
    return false;
}

void CollageDocument::setGlobalMargin(double margin)
{
    for (auto& slot : m_slots) {
        slot.setMargin(margin);
    }
    emit documentChanged();
}

void CollageDocument::setGlobalPadding(double padding)
{
    for (auto& slot : m_slots) {
        slot.setPadding(padding);
    }
    emit documentChanged();
}

void CollageDocument::setGlobalBorderRadius(double radius)
{
    for (auto& slot : m_slots) {
        slot.setBorderRadius(radius);
    }
    emit documentChanged();
}

void CollageDocument::setGlobalBorderWidth(double width)
{
    for (auto& slot : m_slots) {
        slot.setBorderWidth(width);
    }
    emit documentChanged();
}

void CollageDocument::setGlobalBorderColor(const QColor& color)
{
    for (auto& slot : m_slots) {
        slot.setBorderColor(color);
    }
    emit documentChanged();
}

} // namespace PhotoColla::Core
