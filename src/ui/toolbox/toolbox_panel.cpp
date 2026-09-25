#include "toolbox_panel.h"
#include <QLabel>
#include <QGridLayout>
#include <QListWidgetItem>
#include <QFileInfo>
#include <QIcon>
#include <QScrollArea>

namespace PhotoColla::UI {

ToolboxPanel::ToolboxPanel(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
}

void ToolboxPanel::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(8);

    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setStyleSheet(
        "QTabWidget::pane { border: 1px solid #2d2d32; background: #202023; border-radius: 6px; }"
        "QTabBar::tab { background: #1a1a1c; color: #a1a1aa; padding: 8px 16px; border-top-left-radius: 4px; border-top-right-radius: 4px; }"
        "QTabBar::tab:selected { background: #202023; color: #ffffff; font-weight: 600; border-bottom: 2px solid #3b82f6; }"
    );

    // Tab 1: Layouts & Templates
    auto* templatesContainer = new QWidget();
    auto* templatesScroll = new QScrollArea();
    templatesScroll->setWidgetResizable(true);
    templatesScroll->setFrameShape(QFrame::NoFrame);

    auto* templatesScrollWidget = new QWidget();
    m_templatesLayout = new QVBoxLayout(templatesScrollWidget);
    m_templatesLayout->setContentsMargins(8, 8, 8, 8);
    m_templatesLayout->setSpacing(8);

    createTemplateCards();
    m_templatesLayout->addStretch();
    templatesScroll->setWidget(templatesScrollWidget);

    auto* tab1Layout = new QVBoxLayout(templatesContainer);
    tab1Layout->setContentsMargins(0, 0, 0, 0);
    tab1Layout->addWidget(templatesScroll);

    // Tab 2: Photos Library
    auto* photosContainer = new QWidget();
    auto* photosLayout = new QVBoxLayout(photosContainer);
    photosLayout->setContentsMargins(8, 8, 8, 8);
    photosLayout->setSpacing(8);

    auto* importBtn = new QPushButton(tr("📁 Import Photos..."), this);
    importBtn->setObjectName("primaryButton");
    importBtn->setFixedHeight(36);
    connect(importBtn, &QPushButton::clicked, this, &ToolboxPanel::importPhotosRequested);
    photosLayout->addWidget(importBtn);

    m_photoListWidget = new QListWidget(this);
    m_photoListWidget->setIconSize(QSize(72, 72));
    m_photoListWidget->setViewMode(QListView::IconMode);
    m_photoListWidget->setMovement(QListView::Static);
    m_photoListWidget->setResizeMode(QListWidget::Adjust);
    m_photoListWidget->setSpacing(8);
    m_photoListWidget->setStyleSheet("background-color: #17171a; border: 1px solid #2d2d32; border-radius: 6px;");
    connect(m_photoListWidget, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* item) {
        if (item) {
            emit photoChosen(item->data(Qt::UserRole).toString());
        }
    });
    photosLayout->addWidget(m_photoListWidget);

    auto* slotActionsLayout = new QHBoxLayout();
    auto* fitBtn = new QPushButton(tr("Fit to Slot"), this);
    auto* clearBtn = new QPushButton(tr("Remove Image"), this);
    connect(fitBtn, &QPushButton::clicked, this, &ToolboxPanel::fitActiveSlotRequested);
    connect(clearBtn, &QPushButton::clicked, this, &ToolboxPanel::clearActiveSlotRequested);
    slotActionsLayout->addWidget(fitBtn);
    slotActionsLayout->addWidget(clearBtn);
    photosLayout->addLayout(slotActionsLayout);

    m_tabWidget->addTab(templatesContainer, tr("Templates"));
    m_tabWidget->addTab(photosContainer, tr("Photos"));

    mainLayout->addWidget(m_tabWidget);
}

void ToolboxPanel::createTemplateCards()
{
    struct TemplateItem {
        QString id;
        QString name;
        int rows;
        int cols;
    };

    const std::vector<TemplateItem> items = {
        {"grid_2x2", tr("Classic 2x2 Grid"), 2, 2},
        {"grid_3x3", tr("Instagram 3x3 Grid"), 3, 3},
        {"split_1_2", tr("Editorial 1 Hero + 2 Side"), 0, 0},
        {"triple_horizontal", tr("Panoramic 3 Triptych"), 0, 0},
        {"grid_1x2", tr("Side-by-Side 1x2"), 1, 2},
        {"grid_4x4", tr("Micro Mosaic 4x4"), 4, 4},
    };

    for (const auto& item : items) {
        auto* btn = new QPushButton(item.name, this);
        btn->setFixedHeight(44);
        btn->setStyleSheet(
            "QPushButton { text-align: left; padding-left: 14px; background: #26262b; border: 1px solid #333338; border-radius: 6px; font-weight: 500; }"
            "QPushButton:hover { background: #323238; border-color: #3b82f6; }"
        );

        if (item.rows > 0 && item.cols > 0) {
            connect(btn, &QPushButton::clicked, this, [this, r = item.rows, c = item.cols]() {
                emit gridLayoutRequested(r, c);
            });
        } else {
            connect(btn, &QPushButton::clicked, this, [this, id = item.id]() {
                emit templateSelected(id);
            });
        }
        m_templatesLayout->addWidget(btn);
    }
}

void ToolboxPanel::addPhotoToLibrary(const QString& filePath)
{
    QFileInfo info(filePath);
    QPixmap thumb(filePath);
    if (thumb.isNull()) return;

    QIcon icon(thumb.scaled(120, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    auto* item = new QListWidgetItem(icon, info.fileName());
    item->setData(Qt::UserRole, filePath);
    item->setToolTip(filePath);
    m_photoListWidget->addItem(item);
}

} // namespace PhotoColla::UI
