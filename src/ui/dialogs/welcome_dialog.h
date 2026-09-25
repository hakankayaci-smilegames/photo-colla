#pragma once

#include <QDialog>
#include <QString>
#include <QVBoxLayout>
#include <QPushButton>
#include <QListWidget>

namespace PhotoColla::UI {

class WelcomeDialog : public QDialog {
    Q_OBJECT
public:
    enum class ResultAction {
        NewCollage,
        OpenCollage,
        Exit
    };

    explicit WelcomeDialog(QWidget* parent = nullptr);
    
    ResultAction userAction() const { return m_action; }
    QString selectedProjectPath() const { return m_selectedPath; }

private slots:
    void onNewClicked();
    void onOpenClicked();
    void onRecentSelected(QListWidgetItem* item);

private:
    void loadRecentProjects();
    void setupUi();

    ResultAction m_action{ResultAction::Exit};
    QString m_selectedPath;
    QListWidget* m_recentList{nullptr};
};

} // namespace PhotoColla::UI
