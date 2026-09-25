#pragma once

#include <QMainWindow>
#include <QDockWidget>
#include <QLabel>
#include <QAction>
#include <memory>

#include "canvas/collage_canvas.h"
#include "toolbox/toolbox_panel.h"
#include "properties/properties_panel.h"
#include "core/models/collage_document.h"
#include "core/history/command_history.h"

namespace PhotoColla::UI {

/**
 * @brief Studio Main Window organizing dockable toolboxes, canvas viewport,
 * property inspectors, toolbars and application commands.
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

private slots:
    void onUndo();
    void onRedo();
    void onExport();
    void onImportPhotos();
    void onAutoLayoutRequested();
    void onSlotImageRequested(int slotIndex);
    void onPhotoChosenFromLibrary(const QString& filePath);
    void onSelectionChanged(int slotIndex);

    // Property panel forwarders
    void onMarginChanged(double value);
    void onPaddingChanged(double value);
    void onBorderRadiusChanged(double value);
    void onBorderWidthChanged(double value);
    void onBorderColorChanged(const QColor& color);
    void onBackgroundColorChanged(const QColor& color);

private:
    void setupUi();
    void setupMenusAndToolbars();
    void setupDocks();
    void setupConnections();

    std::unique_ptr<Core::CollageDocument> m_document;
    std::unique_ptr<Core::CommandHistory> m_history;

    CollageCanvas* m_canvas{nullptr};
    ToolboxPanel* m_toolboxPanel{nullptr};
    PropertiesPanel* m_propertiesPanel{nullptr};

    QDockWidget* m_toolboxDock{nullptr};
    QDockWidget* m_propertiesDock{nullptr};

    // Actions
    QAction* m_undoAction{nullptr};
    QAction* m_redoAction{nullptr};
    QAction* m_zoomInAction{nullptr};
    QAction* m_zoomOutAction{nullptr};
    QAction* m_fitScreenAction{nullptr};
    QAction* m_exportAction{nullptr};

    // Status bar labels
    QLabel* m_statusZoomLabel{nullptr};
    QLabel* m_statusSlotLabel{nullptr};
    QLabel* m_statusSizeLabel{nullptr};

    bool m_applyPropertiesToAll{true};
};

} // namespace PhotoColla::UI
