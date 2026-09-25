#pragma once

#include <QSplashScreen>
#include <QProgressBar>
#include <QLabel>
#include <QVBoxLayout>
#include <QPointer>
#include <QThread>

namespace PhotoColla::UI {

/**
 * @brief Background worker for pre-loading application assets, templates and caching.
 */
class AssetLoaderWorker : public QObject {
    Q_OBJECT

public:
    explicit AssetLoaderWorker(QObject* parent = nullptr);

public slots:
    void run();

signals:
    void progressUpdated(int percentage, const QString& statusMessage);
    void finished();
};

/**
 * @brief Studio-grade asynchronous splash screen showing asset loading progress.
 */
class SplashScreen : public QWidget {
    Q_OBJECT

public:
    explicit SplashScreen(QWidget* parent = nullptr);
    ~SplashScreen() override;

    void startInitialization();

signals:
    void initializationComplete();

protected:
    void paintEvent(QPaintEvent* event) override;

private slots:
    void onProgressUpdated(int percentage, const QString& statusMessage);
    void onWorkerFinished();

private:
    QLabel* m_titleLabel{nullptr};
    QLabel* m_subtitleLabel{nullptr};
    QLabel* m_statusLabel{nullptr};
    QProgressBar* m_progressBar{nullptr};

    QThread* m_workerThread{nullptr};
    AssetLoaderWorker* m_worker{nullptr};
};

} // namespace PhotoColla::UI
