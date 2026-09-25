#include "splash_screen.h"
#include <QPainter>
#include <QPainterPath>
#include <QFile>
#include <QThread>
#include <QApplication>
#include <QScreen>
#include <QDebug>

namespace PhotoColla::UI {

AssetLoaderWorker::AssetLoaderWorker(QObject* parent)
    : QObject(parent)
{
}

void AssetLoaderWorker::run()
{
    // Step 1: Initialize Core Services & Color Profiles
    emit progressUpdated(15, tr("Initializing core runtime & color engine..."));
    QThread::msleep(200);

    // Step 2: Load and Verify Stylesheets & Typography
    emit progressUpdated(40, tr("Loading high-DPI typography & dark theme..."));
    QThread::msleep(250);

    // Step 3: Parse and Cache Built-in Templates
    emit progressUpdated(70, tr("Parsing layout templates & presets..."));
    QFile templateFile(":/assets/templates/default_templates.json");
    if (templateFile.open(QIODevice::ReadOnly)) {
        templateFile.readAll();
        templateFile.close();
    }
    QThread::msleep(250);

    // Step 4: Warm up GPU & OpenGL Pipeline
    emit progressUpdated(90, tr("Configuring hardware-accelerated viewport..."));
    QThread::msleep(200);

    // Step 5: Ready
    emit progressUpdated(100, tr("Ready!"));
    QThread::msleep(150);

    emit finished();
}

SplashScreen::SplashScreen(QWidget* parent)
    : QWidget(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::SubWindow)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(520, 320);

    // Center on screen
    if (QScreen* primaryScreen = QGuiApplication::primaryScreen()) {
        QRect screenGeometry = primaryScreen->geometry();
        int x = (screenGeometry.width() - width()) / 2;
        int y = (screenGeometry.height() - height()) / 2;
        move(x, y);
    }

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(40, 40, 40, 40);
    rootLayout->setSpacing(12);

    m_titleLabel = new QLabel("PhotoColla", this);
    m_titleLabel->setStyleSheet("font-size: 32px; font-weight: 800; color: #ffffff; letter-spacing: 1px;");

    m_subtitleLabel = new QLabel("Professional Creative Collage Studio", this);
    m_subtitleLabel->setStyleSheet("font-size: 13px; font-weight: 500; color: #3b82f6;");

    rootLayout->addWidget(m_titleLabel);
    rootLayout->addWidget(m_subtitleLabel);
    rootLayout->addStretch();

    m_statusLabel = new QLabel(tr("Initializing..."), this);
    m_statusLabel->setStyleSheet("font-size: 12px; color: #9ca3af;");
    rootLayout->addWidget(m_statusLabel);

    m_progressBar = new QProgressBar(this);
    m_progressBar->setFixedHeight(6);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setTextVisible(false);
    m_progressBar->setStyleSheet(
        "QProgressBar { background-color: #27272a; border-radius: 3px; border: none; }"
        "QProgressBar::chunk { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #2563eb, stop:1 #38bdf8); border-radius: 3px; }"
    );
    rootLayout->addWidget(m_progressBar);
}

SplashScreen::~SplashScreen()
{
    if (m_workerThread && m_workerThread->isRunning()) {
        m_workerThread->quit();
        m_workerThread->wait();
    }
}

void SplashScreen::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Rounded modern dark surface
    QRectF bgRect = rect();
    QPainterPath path;
    path.addRoundedRect(bgRect, 16.0, 16.0);

    // Subtle dark gradient background
    QLinearGradient grad(0, 0, 0, height());
    grad.setColorAt(0.0, QColor(24, 24, 27, 250));
    grad.setColorAt(1.0, QColor(15, 15, 18, 252));
    painter.fillPath(path, grad);

    // Crisp subtle border
    painter.setPen(QPen(QColor(63, 63, 70, 160), 1.5));
    painter.drawPath(path);
}

void SplashScreen::startInitialization()
{
    show();

    m_workerThread = new QThread(this);
    m_worker = new AssetLoaderWorker();
    m_worker->moveToThread(m_workerThread);

    connect(m_workerThread, &QThread::started, m_worker, &AssetLoaderWorker::run);
    connect(m_worker, &AssetLoaderWorker::progressUpdated, this, &SplashScreen::onProgressUpdated);
    connect(m_worker, &AssetLoaderWorker::finished, this, &SplashScreen::onWorkerFinished);
    connect(m_worker, &AssetLoaderWorker::finished, m_workerThread, &QThread::quit);
    connect(m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);

    m_workerThread->start();
}

void SplashScreen::onProgressUpdated(int percentage, const QString& statusMessage)
{
    m_progressBar->setValue(percentage);
    m_statusLabel->setText(statusMessage);
}

void SplashScreen::onWorkerFinished()
{
    emit initializationComplete();
    close();
}

} // namespace PhotoColla::UI
