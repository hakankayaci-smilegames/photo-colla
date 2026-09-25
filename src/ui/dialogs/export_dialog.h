#pragma once

#include <QDialog>
#include <QComboBox>
#include <QSpinBox>
#include <QSlider>
#include <QLineEdit>
#include <QLabel>
#include "../../export/exporter.h"

namespace PhotoColla::UI {

/**
 * @brief Studio export dialog offering resolutions, DPI presets, file formats,
 * compression controls and asynchronous rendering output.
 */
class ExportDialog : public QDialog {
    Q_OBJECT

public:
    explicit ExportDialog(const QSize& initialSize, QWidget* parent = nullptr);
    ~ExportDialog() override = default;

    [[nodiscard]] Export::ExportOptions getOptions() const;

private slots:
    void onPresetChanged(int index);
    void onFormatChanged(const QString& format);
    void browseOutputPath();

private:
    void setupUi();

    QComboBox* m_presetCombo{nullptr};
    QSpinBox* m_widthSpin{nullptr};
    QSpinBox* m_heightSpin{nullptr};
    QComboBox* m_dpiCombo{nullptr};
    QComboBox* m_formatCombo{nullptr};
    QSlider* m_qualitySlider{nullptr};
    QLabel* m_qualityValueLabel{nullptr};
    QLineEdit* m_pathEdit{nullptr};
};

} // namespace PhotoColla::UI
