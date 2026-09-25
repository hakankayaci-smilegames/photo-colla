#include "select_photos_dialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFileInfo>
#include <QIcon>
#include <QListWidgetItem>
#include <QPixmap>

namespace PhotoColla::UI {

SelectPhotosDialog::SelectPhotosDialog(const QStringList& availablePhotos, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Select Photos for Smart Collage"));
    setMinimumSize(400, 500);

    auto* mainLayout = new QVBoxLayout(this);

    auto* titleLabel = new QLabel(tr("Otomatik kolaj için kullanılacak fotoğrafları seçin:"), this);
    mainLayout->addWidget(titleLabel);

    m_listWidget = new QListWidget(this);
    m_listWidget->setSelectionMode(QAbstractItemView::MultiSelection);
    m_listWidget->setIconSize(QSize(64, 64));
    m_listWidget->setAlternatingRowColors(true);
    
    for (const QString& path : availablePhotos) {
        auto* item = new QListWidgetItem(m_listWidget);
        item->setText(QFileInfo(path).fileName());
        item->setIcon(QIcon(path));
        item->setData(Qt::UserRole, path);
        // Varsayılan olarak hepsini seçili yapalım
        item->setSelected(true);
    }
    
    mainLayout->addWidget(m_listWidget);

    auto* buttonLayout = new QHBoxLayout();
    
    auto* btnSelectAll = new QPushButton(tr("Tümünü Seç"), this);
    connect(btnSelectAll, &QPushButton::clicked, m_listWidget, &QListWidget::selectAll);
    
    auto* btnClear = new QPushButton(tr("Seçimi Temizle"), this);
    connect(btnClear, &QPushButton::clicked, m_listWidget, &QListWidget::clearSelection);
    
    buttonLayout->addWidget(btnSelectAll);
    buttonLayout->addWidget(btnClear);
    buttonLayout->addStretch();
    
    auto* btnCancel = new QPushButton(tr("İptal"), this);
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
    
    auto* btnOk = new QPushButton(tr("Tamam"), this);
    btnOk->setDefault(true);
    connect(btnOk, &QPushButton::clicked, this, &QDialog::accept);
    
    buttonLayout->addWidget(btnCancel);
    buttonLayout->addWidget(btnOk);

    mainLayout->addLayout(buttonLayout);
}

QStringList SelectPhotosDialog::selectedPhotos() const
{
    QStringList selected;
    for (int i = 0; i < m_listWidget->count(); ++i) {
        auto* item = m_listWidget->item(i);
        if (item->isSelected()) {
            selected.append(item->data(Qt::UserRole).toString());
        }
    }
    return selected;
}

} // namespace PhotoColla::UI
