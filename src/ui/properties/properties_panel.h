#pragma once

#include <QWidget>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QColor>

namespace PhotoColla::Core {
class Slot;
}

namespace PhotoColla::UI {

/**
 * @brief Right sidebar inspector panel controlling margins, paddings, corner radii,
 * border strokes and color styling with realtime bidirectional updates.
 */
class PropertiesPanel : public QWidget {
    Q_OBJECT

public:
    explicit PropertiesPanel(QWidget* parent = nullptr);
    ~PropertiesPanel() override = default;

    void updateFromSlot(const Core::Slot* slot, const QColor& canvasBg);
    void updateFromDocumentDefaults(const QColor& canvasBg);

signals:
    void marginChanged(double value);
    void paddingChanged(double value);
    void borderRadiusChanged(double value);
    void borderWidthChanged(double value);
    void borderColorChanged(const QColor& color);
    void backgroundColorChanged(const QColor& color);
    void applyToAllChanged(bool applyToAll);

private:
    void setupUi();
    void chooseBorderColor();
    void chooseBackgroundColor();

    QLabel* m_headerLabel{nullptr};
    QCheckBox* m_applyToAllCheck{nullptr};

    // Sliders & SpinBoxes
    QSlider* m_marginSlider{nullptr};
    QDoubleSpinBox* m_marginSpin{nullptr};

    QSlider* m_paddingSlider{nullptr};
    QDoubleSpinBox* m_paddingSpin{nullptr};

    QSlider* m_radiusSlider{nullptr};
    QDoubleSpinBox* m_radiusSpin{nullptr};

    QSlider* m_strokeSlider{nullptr};
    QDoubleSpinBox* m_strokeSpin{nullptr};

    // Colors
    QPushButton* m_borderColorBtn{nullptr};
    QPushButton* m_bgColorBtn{nullptr};

    QColor m_currentBorderColor{60, 60, 65};
    QColor m_currentBgColor{20, 20, 22};
};

} // namespace PhotoColla::UI
