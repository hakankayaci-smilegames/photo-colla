#pragma once

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QTabWidget>
#include <QMimeData>
#include <QDrag>
#include <QDropEvent>

namespace PhotoColla::UI {

/**
 * @brief Custom list widget to support drag and drop of photos
 * both internal (to canvas) and external (from file manager).
 */
class PhotoListWidget : public QListWidget {
    Q_OBJECT
public:
    explicit PhotoListWidget(QWidget* parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void startDrag(Qt::DropActions supportedActions) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;

signals:
    void filesDropped(const QStringList& files);

private:
    QPoint m_dragStartPos;
};

/**
 * @brief Left sidebar toolbox containing collage layout presets, grid generators,
 * and a photo asset library.
 */
class ToolboxPanel : public QWidget {
    Q_OBJECT

public:
    explicit ToolboxPanel(QWidget* parent = nullptr);
    ~ToolboxPanel() override = default;

    void addPhotoToLibrary(const QString& filePath);
    QStringList getLibraryPhotos() const;

signals:
    void templateSelected(const QString& templateId);
    void gridLayoutRequested(int rows, int cols);
    void importPhotosRequested();
    void autoLayoutRequested();
    void photoChosen(const QString& filePath);
    void fitActiveSlotRequested();
    void clearActiveSlotRequested();

private:
    void setupUi();
    void createTemplateCards();

    QTabWidget* m_tabWidget{nullptr};
    PhotoListWidget* m_photoListWidget{nullptr};
    QVBoxLayout* m_templatesLayout{nullptr};
};

} // namespace PhotoColla::UI
