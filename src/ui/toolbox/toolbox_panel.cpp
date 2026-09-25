#include "toolbox_panel.h"
#include <QLabel>
#include <QGridLayout>
#include <QListWidgetItem>
#include <QFileInfo>
#include <QIcon>
#include <QScrollArea>
#include <QUrl>
#include <QMimeData>
#include <QDrag>
#include <QPainter>
#include <QMouseEvent>
#include <QApplication>

namespace PhotoColla::UI {

// ---------------------------------------------------------
// PhotoListWidget Implementation
// ---------------------------------------------------------
PhotoListWidget::PhotoListWidget(QWidget* parent) : QListWidget(parent)
{
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDragDropMode(QAbstractItemView::DragDrop);
}

void PhotoListWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragStartPos = event->pos();
    }
    QListWidget::mousePressEvent(event);
}

void PhotoListWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (!(event->buttons() & Qt::LeftButton)) {
        QListWidget::mouseMoveEvent(event);
        return;
    }
    if ((event->pos() - m_dragStartPos).manhattanLength() < QApplication::startDragDistance()) {
        QListWidget::mouseMoveEvent(event);
        return;
    }

    QListWidgetItem* item = itemAt(m_dragStartPos);
    if (!item) {
        QListWidget::mouseMoveEvent(event);
        return;
    }

    startDrag(Qt::CopyAction);
}

void PhotoListWidget::startDrag(Qt::DropActions /*supportedActions*/)
{
    QList<QListWidgetItem*> selected = selectedItems();
    if (selected.isEmpty()) return;

    QList<QUrl> urls;
    for (auto* item : selected) {
        QString filePath = item->data(Qt::UserRole).toString();
        urls.append(QUrl::fromLocalFile(filePath));
    }

    auto* mimeData = new QMimeData();
    mimeData->setUrls(urls);

    auto* drag = new QDrag(this);
    drag->setMimeData(mimeData);

    // Create a sleek ghost thumbnail for the drag operation
    QPixmap iconPixmap = selected.first()->icon().pixmap(72, 72);
    QPixmap dragPixmap(iconPixmap.size());
    dragPixmap.fill(Qt::transparent);
    QPainter painter(&dragPixmap);
    painter.setOpacity(0.85);
    painter.drawPixmap(0, 0, iconPixmap);
    painter.setPen(QPen(QColor(56, 189, 248), 2.0));
    painter.drawRoundedRect(dragPixmap.rect().adjusted(1, 1, -1, -1), 4, 4);
    painter.end();

    drag->setPixmap(dragPixmap);
    drag->setHotSpot(dragPixmap.rect().center());

    drag->exec(Qt::CopyAction);
}

void PhotoListWidget::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void PhotoListWidget::dragMoveEvent(QDragMoveEvent* event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void PhotoListWidget::dropEvent(QDropEvent* event)
{
    const QMimeData* mime = event->mimeData();
    if (mime->hasUrls()) {
        QStringList filePaths;
        for (const QUrl& url : mime->urls()) {
            if (url.isLocalFile()) {
                filePaths.append(url.toLocalFile());
            }
        }
        if (!filePaths.isEmpty()) {
            emit filesDropped(filePaths);
        }
        event->acceptProposedAction();
    }
}

// ---------------------------------------------------------
// ToolboxPanel Implementation
// ---------------------------------------------------------
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

    auto* autoLayoutContainer = new QWidget();
    auto* autoLayoutContLayout = new QVBoxLayout(autoLayoutContainer);
    autoLayoutContLayout->setContentsMargins(8, 8, 8, 0);
    auto* autoLayoutBtn = new QPushButton(tr("✨ Auto Smart Collage..."), this);
    autoLayoutBtn->setObjectName("primaryButton");
    autoLayoutBtn->setFixedHeight(36);
    autoLayoutBtn->setStyleSheet("background-color: #3b82f6; color: white; font-weight: bold; border-radius: 4px;");
    connect(autoLayoutBtn, &QPushButton::clicked, this, &ToolboxPanel::autoLayoutRequested);
    autoLayoutContLayout->addWidget(autoLayoutBtn);

    tab1Layout->addWidget(autoLayoutContainer);
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

    m_photoListWidget = new PhotoListWidget(this);
    m_photoListWidget->setIconSize(QSize(72, 72));
    m_photoListWidget->setViewMode(QListView::IconMode);
    m_photoListWidget->setMovement(QListView::Static);
    m_photoListWidget->setResizeMode(QListWidget::Adjust);
    m_photoListWidget->setSpacing(8);
    m_photoListWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_photoListWidget->setStyleSheet("background-color: #17171a; border: 1px solid #2d2d32; border-radius: 6px;");
    connect(m_photoListWidget, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* item) {
        if (item) {
            emit photoChosen(item->data(Qt::UserRole).toString());
        }
    });
    connect(m_photoListWidget, &PhotoListWidget::filesDropped, this, [this](const QStringList& files) {
        for (const QString& file : files) {
            addPhotoToLibrary(file);
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
    // Prevent duplicates
    for (int i = 0; i < m_photoListWidget->count(); ++i) {
        if (m_photoListWidget->item(i)->data(Qt::UserRole).toString() == filePath) {
            return; // Already exists
        }
    }

    QFileInfo info(filePath);
    QPixmap thumb(filePath);
    if (thumb.isNull()) return;

    QIcon icon(thumb.scaled(120, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    auto* item = new QListWidgetItem(icon, info.fileName());
    item->setData(Qt::UserRole, filePath);
    item->setToolTip(filePath);
    item->setFlags(item->flags() | Qt::ItemIsDragEnabled | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    m_photoListWidget->addItem(item);
}



QStringList ToolboxPanel::getLibraryPhotos() const
{
    QStringList photos;
    for (int i = 0; i < m_photoListWidget->count(); ++i) {
        photos.append(m_photoListWidget->item(i)->data(Qt::UserRole).toString());
    }
    return photos;
}
} // namespace PhotoColla::UI
