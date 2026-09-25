#pragma once

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QTabWidget>

namespace PhotoColla::UI {

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

signals:
    void templateSelected(const QString& templateId);
    void gridLayoutRequested(int rows, int cols);
    void importPhotosRequested();
    void photoChosen(const QString& filePath);
    void fitActiveSlotRequested();
    void clearActiveSlotRequested();

private:
    void setupUi();
    void createTemplateCards();

    QTabWidget* m_tabWidget{nullptr};
    QListWidget* m_photoListWidget{nullptr};
    QVBoxLayout* m_templatesLayout{nullptr};
};

} // namespace PhotoColla::UI
