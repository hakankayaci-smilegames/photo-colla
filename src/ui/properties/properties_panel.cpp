#include "properties_panel.h"
#include "../../core/models/slot.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QColorDialog>
#include <QScrollArea>
#include <QGroupBox>

namespace PhotoColla::UI {

PropertiesPanel::PropertiesPanel(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
}

void PropertiesPanel::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(14);

    m_headerLabel = new QLabel(tr("CANVAS PROPERTIES"), this);
    m_headerLabel->setStyleSheet("font-size: 11px; font-weight: 700; color: #3b82f6; letter-spacing: 1px;");
    mainLayout->addWidget(m_headerLabel);

    m_applyToAllCheck = new QCheckBox(tr("Apply changes to all slots"), this);
    m_applyToAllCheck->setChecked(true);
    m_applyToAllCheck->setStyleSheet("color: #d1d5db; font-size: 12px;");
    connect(m_applyToAllCheck, &QCheckBox::toggled, this, &PropertiesPanel::applyToAllChanged);
    mainLayout->addWidget(m_applyToAllCheck);

    auto* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    auto* scrollWidget = new QWidget();
    auto* contentLayout = new QVBoxLayout(scrollWidget);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(16);

    // Helper lambda to create linked Slider + DoubleSpinBox
    auto createControlRow = [this, contentLayout](
        const QString& labelText,
        double minVal, double maxVal, double defaultVal,
        QSlider*& slider, QDoubleSpinBox*& spinBox,
        auto signalEmitter
    ) {
        auto* group = new QWidget();
        auto* gLayout = new QVBoxLayout(group);
        gLayout->setContentsMargins(0, 0, 0, 0);
        gLayout->setSpacing(4);

        auto* headerRow = new QHBoxLayout();
        auto* label = new QLabel(labelText, group);
        label->setStyleSheet("color: #9ca3af; font-size: 12px; font-weight: 500;");

        spinBox = new QDoubleSpinBox(group);
        spinBox->setRange(minVal, maxVal);
        spinBox->setValue(defaultVal);
        spinBox->setSingleStep(1.0);
        spinBox->setDecimals(1);
        spinBox->setFixedWidth(64);

        headerRow->addWidget(label);
        headerRow->addStretch();
        headerRow->addWidget(spinBox);
        gLayout->addLayout(headerRow);

        slider = new QSlider(Qt::Horizontal, group);
        slider->setRange(static_cast<int>(minVal * 10), static_cast<int>(maxVal * 10));
        slider->setValue(static_cast<int>(defaultVal * 10));
        gLayout->addWidget(slider);

        // Sync Slider and SpinBox
        connect(slider, &QSlider::valueChanged, this, [spinBox, this, signalEmitter](int val) {
            double dVal = static_cast<double>(val) / 10.0;
            if (spinBox->value() != dVal) {
                spinBox->blockSignals(true);
                spinBox->setValue(dVal);
                spinBox->blockSignals(false);
            }
            signalEmitter(dVal);
        });

        connect(spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [slider, this, signalEmitter](double val) {
            int iVal = static_cast<int>(val * 10.0);
            if (slider->value() != iVal) {
                slider->blockSignals(true);
                slider->setValue(iVal);
                slider->blockSignals(false);
            }
            signalEmitter(val);
        });

        contentLayout->addWidget(group);
    };

    // 1. Margin (Dış Boşluk)
    createControlRow(tr("Outer Margin (Gap)"), 0.0, 50.0, 6.0, m_marginSlider, m_marginSpin, [this](double val) {
        emit marginChanged(val);
    });

    // 2. Padding (İç Boşluk)
    createControlRow(tr("Inner Padding"), 0.0, 50.0, 0.0, m_paddingSlider, m_paddingSpin, [this](double val) {
        emit paddingChanged(val);
    });

    // 3. Border Radius (Köşe Yuvarlatma)
    createControlRow(tr("Corner Radius"), 0.0, 100.0, 12.0, m_radiusSlider, m_radiusSpin, [this](double val) {
        emit borderRadiusChanged(val);
    });

    // 4. Border Width (Çizgi Kalınlığı)
    createControlRow(tr("Border Stroke Width"), 0.0, 30.0, 2.0, m_strokeSlider, m_strokeSpin, [this](double val) {
        emit borderWidthChanged(val);
    });

    // 5. Colors Group
    auto* colorGroup = new QWidget();
    auto* colorLayout = new QVBoxLayout(colorGroup);
    colorLayout->setContentsMargins(0, 0, 0, 0);
    colorLayout->setSpacing(8);

    auto* colorTitle = new QLabel(tr("Colors & Appearance"), colorGroup);
    colorTitle->setStyleSheet("color: #9ca3af; font-size: 12px; font-weight: 500;");
    colorLayout->addWidget(colorTitle);

    // Border Color Button
    auto* borderRow = new QHBoxLayout();
    auto* borderLabel = new QLabel(tr("Border Color:"), colorGroup);
    borderLabel->setStyleSheet("color: #d1d5db; font-size: 12px;");
    m_borderColorBtn = new QPushButton(colorGroup);
    m_borderColorBtn->setFixedHeight(28);
    m_borderColorBtn->setStyleSheet(QString("background-color: %1; border: 1px solid #4b5563; border-radius: 4px;").arg(m_currentBorderColor.name()));
    connect(m_borderColorBtn, &QPushButton::clicked, this, &PropertiesPanel::chooseBorderColor);
    borderRow->addWidget(borderLabel);
    borderRow->addWidget(m_borderColorBtn);
    colorLayout->addLayout(borderRow);

    // Canvas Background Color Button
    auto* bgRow = new QHBoxLayout();
    auto* bgLabel = new QLabel(tr("Canvas Background:"), colorGroup);
    bgLabel->setStyleSheet("color: #d1d5db; font-size: 12px;");
    m_bgColorBtn = new QPushButton(colorGroup);
    m_bgColorBtn->setFixedHeight(28);
    m_bgColorBtn->setStyleSheet(QString("background-color: %1; border: 1px solid #4b5563; border-radius: 4px;").arg(m_currentBgColor.name()));
    connect(m_bgColorBtn, &QPushButton::clicked, this, &PropertiesPanel::chooseBackgroundColor);
    bgRow->addWidget(bgLabel);
    bgRow->addWidget(m_bgColorBtn);
    colorLayout->addLayout(bgRow);

    contentLayout->addWidget(colorGroup);
    contentLayout->addStretch();

    scrollArea->setWidget(scrollWidget);
    mainLayout->addWidget(scrollArea);
}

void PropertiesPanel::chooseBorderColor()
{
    QColor chosen = QColorDialog::getColor(m_currentBorderColor, this, tr("Select Border Color"));
    if (chosen.isValid()) {
        m_currentBorderColor = chosen;
        m_borderColorBtn->setStyleSheet(QString("background-color: %1; border: 1px solid #4b5563; border-radius: 4px;").arg(chosen.name()));
        emit borderColorChanged(chosen);
    }
}

void PropertiesPanel::chooseBackgroundColor()
{
    QColor chosen = QColorDialog::getColor(m_currentBgColor, this, tr("Select Canvas Background"));
    if (chosen.isValid()) {
        m_currentBgColor = chosen;
        m_bgColorBtn->setStyleSheet(QString("background-color: %1; border: 1px solid #4b5563; border-radius: 4px;").arg(chosen.name()));
        emit backgroundColorChanged(chosen);
    }
}

void PropertiesPanel::updateFromSlot(const Core::Slot* slot, const QColor& canvasBg)
{
    if (!slot) {
        updateFromDocumentDefaults(canvasBg);
        return;
    }

    m_headerLabel->setText(tr("SLOT PROPERTIES (%1)").arg(slot->id()));

    m_marginSpin->blockSignals(true);
    m_marginSlider->blockSignals(true);
    m_marginSpin->setValue(slot->margin());
    m_marginSlider->setValue(static_cast<int>(slot->margin() * 10.0));
    m_marginSpin->blockSignals(false);
    m_marginSlider->blockSignals(false);

    m_paddingSpin->blockSignals(true);
    m_paddingSlider->blockSignals(true);
    m_paddingSpin->setValue(slot->padding());
    m_paddingSlider->setValue(static_cast<int>(slot->padding() * 10.0));
    m_paddingSpin->blockSignals(false);
    m_paddingSlider->blockSignals(false);

    m_radiusSpin->blockSignals(true);
    m_radiusSlider->blockSignals(true);
    m_radiusSpin->setValue(slot->borderRadius());
    m_radiusSlider->setValue(static_cast<int>(slot->borderRadius() * 10.0));
    m_radiusSpin->blockSignals(false);
    m_radiusSlider->blockSignals(false);

    m_strokeSpin->blockSignals(true);
    m_strokeSlider->blockSignals(true);
    m_strokeSpin->setValue(slot->borderWidth());
    m_strokeSlider->setValue(static_cast<int>(slot->borderWidth() * 10.0));
    m_strokeSpin->blockSignals(false);
    m_strokeSlider->blockSignals(false);

    m_currentBorderColor = slot->borderColor();
    m_borderColorBtn->setStyleSheet(QString("background-color: %1; border: 1px solid #4b5563; border-radius: 4px;").arg(m_currentBorderColor.name()));

    m_currentBgColor = canvasBg;
    m_bgColorBtn->setStyleSheet(QString("background-color: %1; border: 1px solid #4b5563; border-radius: 4px;").arg(m_currentBgColor.name()));
}

void PropertiesPanel::updateFromDocumentDefaults(const QColor& canvasBg)
{
    m_headerLabel->setText(tr("CANVAS PROPERTIES"));
    m_currentBgColor = canvasBg;
    m_bgColorBtn->setStyleSheet(QString("background-color: %1; border: 1px solid #4b5563; border-radius: 4px;").arg(m_currentBgColor.name()));
}

} // namespace PhotoColla::UI
