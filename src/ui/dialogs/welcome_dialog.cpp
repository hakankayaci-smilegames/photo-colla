#include "welcome_dialog.h"
#include <QLabel>
#include <QFileDialog>
#include <QSettings>
#include <QFileInfo>

namespace PhotoColla::UI {

WelcomeDialog::WelcomeDialog(QWidget* parent) : QDialog(parent)
{
    setWindowTitle(tr("PhotoColla - Welcome"));
    setFixedSize(600, 400);
    setStyleSheet("background-color: #1a1a1c; color: white;");
    setupUi();
    loadRecentProjects();
}

void WelcomeDialog::setupUi()
{
    auto* mainLayout = new QHBoxLayout(this);
    
    // Left side: Actions
    auto* leftPanel = new QWidget();
    auto* leftLayout = new QVBoxLayout(leftPanel);
    
    auto* title = new QLabel(tr("PhotoColla"));
    title->setStyleSheet("font-size: 28px; font-weight: bold; color: #3b82f6; margin-bottom: 20px;");
    leftLayout->addWidget(title);
    
    auto* newBtn = new QPushButton(tr("✨ Yeni Kolaj (New)"));
    newBtn->setFixedHeight(45);
    newBtn->setStyleSheet("background-color: #3b82f6; border-radius: 6px; font-weight: bold; font-size: 14px;");
    connect(newBtn, &QPushButton::clicked, this, &WelcomeDialog::onNewClicked);
    
    auto* openBtn = new QPushButton(tr("📂 Proje Aç (Open...)"));
    openBtn->setFixedHeight(45);
    openBtn->setStyleSheet("background-color: #2d2d32; border-radius: 6px; font-weight: bold; font-size: 14px;");
    connect(openBtn, &QPushButton::clicked, this, &WelcomeDialog::onOpenClicked);
    
    leftLayout->addWidget(newBtn);
    leftLayout->addWidget(openBtn);
    leftLayout->addStretch();
    
    // Right side: Recent projects
    auto* rightPanel = new QWidget();
    auto* rightLayout = new QVBoxLayout(rightPanel);
    
    auto* recentTitle = new QLabel(tr("Son Projeler (Recent)"));
    recentTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #a1a1aa;");
    
    m_recentList = new QListWidget();
    m_recentList->setStyleSheet(
        "QListWidget { background-color: #202023; border: 1px solid #2d2d32; border-radius: 6px; outline: none; }"
        "QListWidget::item { padding: 10px; border-bottom: 1px solid #2d2d32; }"
        "QListWidget::item:selected { background-color: #3b82f6; }"
        "QListWidget::item:hover { background-color: #2d2d32; }"
    );
    connect(m_recentList, &QListWidget::itemClicked, this, &WelcomeDialog::onRecentSelected);
    
    rightLayout->addWidget(recentTitle);
    rightLayout->addWidget(m_recentList);
    
    mainLayout->addWidget(leftPanel, 1);
    mainLayout->addWidget(rightPanel, 2);
}

void WelcomeDialog::loadRecentProjects()
{
    QSettings settings("PhotoColla", "Studio");
    QStringList recentFiles = settings.value("RecentProjects").toStringList();
    
    for (const QString& path : recentFiles) {
        if (QFile::exists(path)) {
            QFileInfo fi(path);
            auto* item = new QListWidgetItem(fi.fileName() + "\n" + path);
            item->setData(Qt::UserRole, path);
            m_recentList->addItem(item);
        }
    }
}

void WelcomeDialog::onNewClicked()
{
    m_action = ResultAction::NewCollage;
    accept();
}

void WelcomeDialog::onOpenClicked()
{
    m_selectedPath = QFileDialog::getOpenFileName(this, tr("Proje Aç"), QString(), tr("PhotoColla Projects (*.pcolla)"));
    if (!m_selectedPath.isEmpty()) {
        m_action = ResultAction::OpenCollage;
        accept();
    }
}

void WelcomeDialog::onRecentSelected(QListWidgetItem* item)
{
    m_selectedPath = item->data(Qt::UserRole).toString();
    m_action = ResultAction::OpenCollage;
    accept();
}

} // namespace PhotoColla::UI
