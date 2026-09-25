#include "export_dialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDir>

namespace PhotoColla::UI {

ExportDialog::ExportDialog(const QSize& initialSize, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Export Collage"));
    setModal(true);
    setFixedSize(460, 480);
    setStyleSheet("background-color: #202023; color: #ffffff;");

    setupUi();

    m_widthSpin->setValue(initialSize.width() > 0 ? initialSize.width() : 2400);
    m_heightSpin->setValue(initialSize.height() > 0 ? initialSize.height() : 2400);

    QString defaultDir = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    m_pathEdit->setText(QDir(defaultDir).filePath("collage_export.png"));
}

void ExportDialog::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(14);

    auto* titleLabel = new QLabel(tr("Export Settings"), this);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: 700; color: #ffffff;");
    mainLayout->addWidget(titleLabel);

    auto* gridLayout = new QGridLayout();
    gridLayout->setHorizontalSpacing(12);
    gridLayout->setVerticalSpacing(10);

    // 1. Preset Selector
    gridLayout->addWidget(new QLabel(tr("Preset:"), this), 0, 0);
    m_presetCombo = new QComboBox(this);
    m_presetCombo->addItem(tr("Custom Dimensions"), 0);
    m_presetCombo->addItem(tr("Instagram Square (1080 x 1080)"), 1);
    m_presetCombo->addItem(tr("Instagram Story (1080 x 1920)"), 2);
    m_presetCombo->addItem(tr("Full HD (1920 x 1080)"), 3);
    m_presetCombo->addItem(tr("4K UHD (3840 x 2160)"), 4);
    m_presetCombo->addItem(tr("A4 Print 300 DPI (2480 x 3508)"), 5);
    m_presetCombo->addItem(tr("A3 Print 300 DPI (3508 x 4960)"), 6);
    connect(m_presetCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ExportDialog::onPresetChanged);
    gridLayout->addWidget(m_presetCombo, 0, 1);

    // 2. Resolution (Width & Height)
    gridLayout->addWidget(new QLabel(tr("Width (px):"), this), 1, 0);
    m_widthSpin = new QSpinBox(this);
    m_widthSpin->setRange(100, 16000);
    gridLayout->addWidget(m_widthSpin, 1, 1);

    gridLayout->addWidget(new QLabel(tr("Height (px):"), this), 2, 0);
    m_heightSpin = new QSpinBox(this);
    m_heightSpin->setRange(100, 16000);
    gridLayout->addWidget(m_heightSpin, 2, 1);

    // 3. DPI / Print Quality
    gridLayout->addWidget(new QLabel(tr("Resolution / DPI:"), this), 3, 0);
    m_dpiCombo = new QComboBox(this);
    m_dpiCombo->addItem(tr("72 DPI (Standard Screen)"), 72);
    m_dpiCombo->addItem(tr("150 DPI (Medium Quality)"), 150);
    m_dpiCombo->addItem(tr("300 DPI (Commercial Print Quality)"), 300);
    m_dpiCombo->addItem(tr("600 DPI (Ultra Fine Art Print)"), 600);
    m_dpiCombo->setCurrentIndex(2); // Default to 300 DPI
    gridLayout->addWidget(m_dpiCombo, 3, 1);

    // 4. Format
    gridLayout->addWidget(new QLabel(tr("Format:"), this), 4, 0);
    m_formatCombo = new QComboBox(this);
    m_formatCombo->addItem("PNG (Lossless)", "png");
    m_formatCombo->addItem("JPG (Standard)", "jpg");
    m_formatCombo->addItem("WebP (Modern)", "webp");
    connect(m_formatCombo, &QComboBox::currentTextChanged, this, &ExportDialog::onFormatChanged);
    gridLayout->addWidget(m_formatCombo, 4, 1);

    // 5. Quality Slider
    auto* qualityLabel = new QLabel(tr("Quality / Compression:"), this);
    gridLayout->addWidget(qualityLabel, 5, 0);

    auto* qualityBox = new QHBoxLayout();
    m_qualitySlider = new QSlider(Qt::Horizontal, this);
    m_qualitySlider->setRange(1, 100);
    m_qualitySlider->setValue(95);
    m_qualityValueLabel = new QLabel("95%", this);
    m_qualityValueLabel->setFixedWidth(36);
    connect(m_qualitySlider, &QSlider::valueChanged, this, [this](int val) {
        m_qualityValueLabel->setText(QString("%1%").arg(val));
    });
    qualityBox->addWidget(m_qualitySlider);
    qualityBox->addWidget(m_qualityValueLabel);
    gridLayout->addLayout(qualityBox, 5, 1);

    mainLayout->addLayout(gridLayout);

    // 6. Output Path
    auto* pathLayout = new QVBoxLayout();
    pathLayout->addWidget(new QLabel(tr("Destination Path:"), this));
    auto* pathInputRow = new QHBoxLayout();
    m_pathEdit = new QLineEdit(this);
    auto* browseBtn = new QPushButton(tr("Browse..."), this);
    connect(browseBtn, &QPushButton::clicked, this, &ExportDialog::browseOutputPath);
    pathInputRow->addWidget(m_pathEdit);
    pathInputRow->addWidget(browseBtn);
    pathLayout->addLayout(pathInputRow);
    mainLayout->addLayout(pathLayout);

    mainLayout->addStretch();

    // Dialog Actions
    auto* btnRow = new QHBoxLayout();
    auto* cancelBtn = new QPushButton(tr("Cancel"), this);
    auto* exportBtn = new QPushButton(tr("Export Now"), this);
    exportBtn->setObjectName("primaryButton");
    exportBtn->setFixedHeight(36);
    cancelBtn->setFixedHeight(36);

    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(exportBtn, &QPushButton::clicked, this, &QDialog::accept);

    btnRow->addStretch();
    btnRow->addWidget(cancelBtn);
    btnRow->addWidget(exportBtn);
    mainLayout->addLayout(btnRow);
}

void ExportDialog::onPresetChanged(int index)
{
    switch (index) {
        case 1: // 1080x1080
            m_widthSpin->setValue(1080);
            m_heightSpin->setValue(1080);
            m_dpiCombo->setCurrentIndex(0);
            break;
        case 2: // 1080x1920
            m_widthSpin->setValue(1080);
            m_heightSpin->setValue(1920);
            m_dpiCombo->setCurrentIndex(0);
            break;
        case 3: // 1920x1080
            m_widthSpin->setValue(1920);
            m_heightSpin->setValue(1080);
            m_dpiCombo->setCurrentIndex(0);
            break;
        case 4: // 3840x2160
            m_widthSpin->setValue(3840);
            m_heightSpin->setValue(2160);
            m_dpiCombo->setCurrentIndex(1);
            break;
        case 5: // A4 300 DPI
            m_widthSpin->setValue(2480);
            m_heightSpin->setValue(3508);
            m_dpiCombo->setCurrentIndex(2); // 300 DPI
            break;
        case 6: // A3 300 DPI
            m_widthSpin->setValue(3508);
            m_heightSpin->setValue(4960);
            m_dpiCombo->setCurrentIndex(2); // 300 DPI
            break;
        default:
            break;
    }
}

void ExportDialog::onFormatChanged(const QString& /*format*/)
{
    QString fmt = m_formatCombo->currentData().toString();
    QString curPath = m_pathEdit->text();
    if (!curPath.isEmpty()) {
        QFileInfo info(curPath);
        QString newPath = info.dir().filePath(info.completeBaseName() + "." + fmt);
        m_pathEdit->setText(newPath);
    }
}

void ExportDialog::browseOutputPath()
{
    QString fmt = m_formatCombo->currentData().toString();
    QString filter = QString("%1 Files (*.%1)").arg(fmt.toUpper());
    QString path = QFileDialog::getSaveFileName(this, tr("Save Collage Image"), m_pathEdit->text(), filter);
    if (!path.isEmpty()) {
        m_pathEdit->setText(path);
    }
}

Export::ExportOptions ExportDialog::getOptions() const
{
    Export::ExportOptions opts;
    opts.targetSize = QSize(m_widthSpin->value(), m_heightSpin->value());
    opts.dpi = m_dpiCombo->currentData().toInt();
    opts.format = m_formatCombo->currentData().toString();
    opts.quality = m_qualitySlider->value();
    opts.outputPath = m_pathEdit->text();
    return opts;
}

} // namespace PhotoColla::UI
