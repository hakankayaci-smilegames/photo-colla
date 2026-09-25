#include "main_window.h"
#include "dialogs/export_dialog.h"
#include "export/exporter.h"
#include "core/commands/collage_commands.h"
#include "core/models/auto_layout_engine.h"

#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QKeySequence>

namespace PhotoColla::UI {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_document(std::make_unique<Core::CollageDocument>(this))
    , m_history(std::make_unique<Core::CommandHistory>(this))
{
    setWindowTitle("PhotoColla - Studio Edition");
    resize(1400, 900);

    setupUi();
    setupMenusAndToolbars();
    setupDocks();
    setupConnections();

    // Initial status bar sync
    m_statusSizeLabel->setText(QString("%1 x %2 px")
        .arg(m_document->canvasSize().width())
        .arg(m_document->canvasSize().height()));
}

void MainWindow::setupUi()
{
    // Central Canvas Viewport
    m_canvas = new CollageCanvas(m_document.get(), m_history.get(), this);
    setCentralWidget(m_canvas);

    // Modern Status Bar
    auto* sBar = statusBar();
    m_statusSlotLabel = new QLabel(tr("No slot selected"), this);
    m_statusZoomLabel = new QLabel("Zoom: 100%", this);
    m_statusSizeLabel = new QLabel("1200 x 1200 px", this);

    sBar->addWidget(m_statusSlotLabel, 1);
    sBar->addPermanentWidget(m_statusSizeLabel);
    sBar->addPermanentWidget(m_statusZoomLabel);
}

void MainWindow::setupMenusAndToolbars()
{
    // -------------------------------------------------------------
    // Actions Definition
    // -------------------------------------------------------------
    m_undoAction = new QAction(tr("Undo"), this);
    m_undoAction->setShortcut(QKeySequence::Undo);
    m_undoAction->setEnabled(false);
    connect(m_undoAction, &QAction::triggered, this, &MainWindow::onUndo);

    m_redoAction = new QAction(tr("Redo"), this);
    m_redoAction->setShortcut(QKeySequence::Redo);
    m_redoAction->setEnabled(false);
    connect(m_redoAction, &QAction::triggered, this, &MainWindow::onRedo);

    m_zoomInAction = new QAction(tr("Zoom In"), this);
    m_zoomInAction->setShortcut(QKeySequence::ZoomIn);
    connect(m_zoomInAction, &QAction::triggered, m_canvas, &CollageCanvas::zoomIn);

    m_zoomOutAction = new QAction(tr("Zoom Out"), this);
    m_zoomOutAction->setShortcut(QKeySequence::ZoomOut);
    connect(m_zoomOutAction, &QAction::triggered, m_canvas, &CollageCanvas::zoomOut);

    m_fitScreenAction = new QAction(tr("Fit to Screen"), this);
    m_fitScreenAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_0));
    connect(m_fitScreenAction, &QAction::triggered, m_canvas, &CollageCanvas::fitToScreen);

    m_exportAction = new QAction(tr("Export Collage..."), this);
    m_exportAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_E));
    connect(m_exportAction, &QAction::triggered, this, &MainWindow::onExport);

    // -------------------------------------------------------------
    // Menu Bar
    // -------------------------------------------------------------
    auto* mBar = menuBar();

    // File Menu
    auto* fileMenu = mBar->addMenu(tr("&File"));
    auto* importAction = fileMenu->addAction(tr("Import Photos..."), this, &MainWindow::onImportPhotos);
    importAction->setShortcut(QKeySequence::Open);
    fileMenu->addSeparator();
    fileMenu->addAction(m_exportAction);
    fileMenu->addSeparator();
    fileMenu->addAction(tr("Exit"), QKeySequence::Quit, this, &QWidget::close);

    // Edit Menu
    auto* editMenu = mBar->addMenu(tr("&Edit"));
    editMenu->addAction(m_undoAction);
    editMenu->addAction(m_redoAction);

    // View Menu
    auto* viewMenu = mBar->addMenu(tr("&View"));
    viewMenu->addAction(m_zoomInAction);
    viewMenu->addAction(m_zoomOutAction);
    viewMenu->addAction(m_fitScreenAction);

    // -------------------------------------------------------------
    // Main Toolbar
    // -------------------------------------------------------------
    auto* toolBar = addToolBar(tr("Main Toolbar"));
    toolBar->setMovable(false);
    toolBar->addAction(m_undoAction);
    toolBar->addAction(m_redoAction);
    toolBar->addSeparator();
    toolBar->addAction(m_zoomInAction);
    toolBar->addAction(m_zoomOutAction);
    toolBar->addAction(m_fitScreenAction);
    toolBar->addSeparator();

    auto* exportBtn = new QPushButton(tr("✨ Export Image"), this);
    exportBtn->setObjectName("primaryButton");
    exportBtn->setFixedHeight(28);
    connect(exportBtn, &QPushButton::clicked, this, &MainWindow::onExport);
    toolBar->addWidget(exportBtn);
}

void MainWindow::setupDocks()
{
    setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

    // Left Toolbox Dock
    m_toolboxDock = new QDockWidget(tr("Toolbox & Assets"), this);
    m_toolboxDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    m_toolboxPanel = new ToolboxPanel(m_toolboxDock);
    m_toolboxDock->setWidget(m_toolboxPanel);
    m_toolboxDock->setMinimumWidth(280);
    addDockWidget(Qt::LeftDockWidgetArea, m_toolboxDock);

    // Right Properties Dock
    m_propertiesDock = new QDockWidget(tr("Properties"), this);
    m_propertiesDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    m_propertiesPanel = new PropertiesPanel(m_propertiesDock);
    m_propertiesDock->setWidget(m_propertiesPanel);
    m_propertiesDock->setMinimumWidth(280);
    addDockWidget(Qt::RightDockWidgetArea, m_propertiesDock);
}

void MainWindow::setupConnections()
{
    // History signals -> Action states
    connect(m_history.get(), &Core::CommandHistory::canUndoChanged, m_undoAction, &QAction::setEnabled);
    connect(m_history.get(), &Core::CommandHistory::canRedoChanged, m_redoAction, &QAction::setEnabled);

    // Canvas signals
    connect(m_canvas, &CollageCanvas::zoomChanged, this, [this](double zoom) {
        m_statusZoomLabel->setText(QString("Zoom: %1%").arg(static_cast<int>(zoom * 100)));
    });
    connect(m_canvas, &CollageCanvas::slotImageRequested, this, &MainWindow::onSlotImageRequested);

    // Toolbox signals
    connect(m_toolboxPanel, &ToolboxPanel::templateSelected, this, [this](const QString& id) {
        m_document->loadTemplateFromJson(id);
    });
    connect(m_toolboxPanel, &ToolboxPanel::gridLayoutRequested, this, [this](int r, int c) {
        m_document->createGridTemplate(r, c);
    });
    connect(m_toolboxPanel, &ToolboxPanel::importPhotosRequested, this, &MainWindow::onImportPhotos);
    connect(m_toolboxPanel, &ToolboxPanel::autoLayoutRequested, this, &MainWindow::onAutoLayoutRequested);
    connect(m_toolboxPanel, &ToolboxPanel::photoChosen, this, &MainWindow::onPhotoChosenFromLibrary);

    connect(m_toolboxPanel, &ToolboxPanel::fitActiveSlotRequested, this, [this]() {
        int sel = m_document->selectedSlotIndex();
        if (sel != -1) {
            if (auto* slot = m_document->slotAt(sel)) {
                slot->fitImageToSlot(m_document->canvasSize());
                emit m_document->documentChanged();
            }
        }
    });

    connect(m_toolboxPanel, &ToolboxPanel::clearActiveSlotRequested, this, [this]() {
        int sel = m_document->selectedSlotIndex();
        if (sel != -1) {
            m_history->push(new Core::SetSlotImageCommand(m_document.get(), sel, QString(), QPixmap()));
        }
    });

    // Selection synchronization
    connect(m_document.get(), &Core::CollageDocument::selectionChanged, this, &MainWindow::onSelectionChanged);

    // Properties panel signals
    connect(m_propertiesPanel, &PropertiesPanel::applyToAllChanged, this, [this](bool val) {
        m_applyPropertiesToAll = val;
    });
    connect(m_propertiesPanel, &PropertiesPanel::marginChanged, this, &MainWindow::onMarginChanged);
    connect(m_propertiesPanel, &PropertiesPanel::paddingChanged, this, &MainWindow::onPaddingChanged);
    connect(m_propertiesPanel, &PropertiesPanel::borderRadiusChanged, this, &MainWindow::onBorderRadiusChanged);
    connect(m_propertiesPanel, &PropertiesPanel::borderWidthChanged, this, &MainWindow::onBorderWidthChanged);
    connect(m_propertiesPanel, &PropertiesPanel::borderColorChanged, this, &MainWindow::onBorderColorChanged);
    connect(m_propertiesPanel, &PropertiesPanel::backgroundColorChanged, this, &MainWindow::onBackgroundColorChanged);
}

void MainWindow::onUndo()
{
    m_history->undo();
}

void MainWindow::onRedo()
{
    m_history->redo();
}

void MainWindow::onSelectionChanged(int slotIndex)
{
    if (slotIndex >= 0) {
        const auto* slot = m_document->slotAt(slotIndex);
        m_statusSlotLabel->setText(tr("Active Slot: %1").arg(slot ? slot->id() : QString::number(slotIndex)));
        m_propertiesPanel->updateFromSlot(slot, m_document->backgroundColor());
    } else {
        m_statusSlotLabel->setText(tr("No slot selected (Canvas View)"));
        m_propertiesPanel->updateFromDocumentDefaults(m_document->backgroundColor());
    }
}

void MainWindow::onImportPhotos()
{
    QStringList files = QFileDialog::getOpenFileNames(
        this,
        tr("Import Photos to Library"),
        QString(),
        tr("Images (*.png *.jpg *.jpeg *.webp *.bmp *.tiff)")
    );

    for (const auto& file : files) {
        m_toolboxPanel->addPhotoToLibrary(file);
    }
}

void MainWindow::onAutoLayoutRequested()
{
    QStringList files = QFileDialog::getOpenFileNames(
        this,
        tr("Select Photos for Auto Collage"),
        QString(),
        tr("Images (*.png *.jpg *.jpeg *.webp *.bmp *.tiff)")
    );

    if (files.isEmpty()) return;

    if (Core::AutoLayoutEngine::generateAndApply(m_document.get(), files)) {
        // Clear undo stack since we completely rewrote slots without commands
        m_history->clear();
        emit m_document->documentChanged();
    }
}

void MainWindow::onSlotImageRequested(int slotIndex)
{
    QString file = QFileDialog::getOpenFileName(
        this,
        tr("Select Image for Slot"),
        QString(),
        tr("Images (*.png *.jpg *.jpeg *.webp *.bmp *.tiff)")
    );

    if (!file.isEmpty()) {
        QPixmap pix(file);
        if (!pix.isNull()) {
            m_history->push(new Core::SetSlotImageCommand(m_document.get(), slotIndex, file, pix));
            m_toolboxPanel->addPhotoToLibrary(file);
        }
    }
}

void MainWindow::onPhotoChosenFromLibrary(const QString& filePath)
{
    int activeSlot = m_document->selectedSlotIndex();
    if (activeSlot == -1 && m_document->slotCount() > 0) {
        activeSlot = 0; // Default to first slot if none selected
    }

    if (activeSlot != -1) {
        QPixmap pix(filePath);
        if (!pix.isNull()) {
            m_history->push(new Core::SetSlotImageCommand(m_document.get(), activeSlot, filePath, pix));
        }
    }
}

void MainWindow::onMarginChanged(double value)
{
    int sel = m_document->selectedSlotIndex();
    double oldVal = (sel != -1 && m_document->slotAt(sel)) ? m_document->slotAt(sel)->margin() : 6.0;

    m_history->push(new Core::ChangeLayoutPropertyCommand(
        m_document.get(), sel, Core::ChangeLayoutPropertyCommand::PropertyType::Margin,
        oldVal, value, m_applyPropertiesToAll
    ));
}

void MainWindow::onPaddingChanged(double value)
{
    int sel = m_document->selectedSlotIndex();
    double oldVal = (sel != -1 && m_document->slotAt(sel)) ? m_document->slotAt(sel)->padding() : 0.0;

    m_history->push(new Core::ChangeLayoutPropertyCommand(
        m_document.get(), sel, Core::ChangeLayoutPropertyCommand::PropertyType::Padding,
        oldVal, value, m_applyPropertiesToAll
    ));
}

void MainWindow::onBorderRadiusChanged(double value)
{
    int sel = m_document->selectedSlotIndex();
    double oldVal = (sel != -1 && m_document->slotAt(sel)) ? m_document->slotAt(sel)->borderRadius() : 12.0;

    m_history->push(new Core::ChangeLayoutPropertyCommand(
        m_document.get(), sel, Core::ChangeLayoutPropertyCommand::PropertyType::BorderRadius,
        oldVal, value, m_applyPropertiesToAll
    ));
}

void MainWindow::onBorderWidthChanged(double value)
{
    int sel = m_document->selectedSlotIndex();
    double oldVal = (sel != -1 && m_document->slotAt(sel)) ? m_document->slotAt(sel)->borderWidth() : 2.0;

    m_history->push(new Core::ChangeLayoutPropertyCommand(
        m_document.get(), sel, Core::ChangeLayoutPropertyCommand::PropertyType::BorderWidth,
        oldVal, value, m_applyPropertiesToAll
    ));
}

void MainWindow::onBorderColorChanged(const QColor& color)
{
    int sel = m_document->selectedSlotIndex();
    QColor oldVal = (sel != -1 && m_document->slotAt(sel)) ? m_document->slotAt(sel)->borderColor() : QColor(60, 60, 65);

    m_history->push(new Core::ChangeLayoutPropertyCommand(
        m_document.get(), sel, Core::ChangeLayoutPropertyCommand::PropertyType::BorderColor,
        oldVal, color, m_applyPropertiesToAll
    ));
}

void MainWindow::onBackgroundColorChanged(const QColor& color)
{
    QColor oldVal = m_document->backgroundColor();
    m_history->push(new Core::ChangeLayoutPropertyCommand(
        m_document.get(), -1, Core::ChangeLayoutPropertyCommand::PropertyType::BackgroundColor,
        oldVal, color, true
    ));
}

void MainWindow::onExport()
{
    QSize initialSize = m_document->canvasSize().toSize();
    ExportDialog dlg(initialSize, this);
    if (dlg.exec() == QDialog::Accepted) {
        Export::ExportOptions opts = dlg.getOptions();
        bool ok = Export::Exporter::exportToFile(m_document.get(), opts);
        if (ok) {
            QMessageBox::information(
                this,
                tr("Export Successful"),
                tr("Your collage was successfully saved to:\n%1\nResolution: %2 x %3 (%4 DPI)")
                    .arg(opts.outputPath)
                    .arg(opts.targetSize.width())
                    .arg(opts.targetSize.height())
                    .arg(opts.dpi)
            );
        } else {
            QMessageBox::critical(this, tr("Export Failed"), tr("Could not write image to target destination."));
        }
    }
}

} // namespace PhotoColla::UI
