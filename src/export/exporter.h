#pragma once

#include <QString>
#include <QSize>
#include <QImage>

namespace PhotoColla::Core {
class CollageDocument;
}

namespace PhotoColla::Export {

struct ExportOptions {
    QSize targetSize{3000, 3000};
    int dpi{300};
    QString format{"png"}; // png, jpg, webp
    int quality{95};       // 1 - 100
    QString outputPath;
};

/**
 * @brief High-resolution print-ready image export pipeline supporting
 * bicubic/smooth resampling and 300 DPI metadata.
 */
class Exporter {
public:
    static QImage renderToImage(const Core::CollageDocument* document, const ExportOptions& options);
    static bool exportToFile(const Core::CollageDocument* document, const ExportOptions& options);
};

} // namespace PhotoColla::Export
