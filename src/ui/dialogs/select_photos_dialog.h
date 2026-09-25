#pragma once

#include <QDialog>
#include <QStringList>
#include <QListWidget>

namespace PhotoColla::UI {

class SelectPhotosDialog : public QDialog {
    Q_OBJECT
public:
    explicit SelectPhotosDialog(const QStringList& availablePhotos, QWidget* parent = nullptr);
    ~SelectPhotosDialog() override = default;

    QStringList selectedPhotos() const;

private:
    QListWidget* m_listWidget;
};

} // namespace PhotoColla::UI
