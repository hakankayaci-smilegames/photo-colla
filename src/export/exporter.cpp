#include "exporter.h"
#include "../core/models/collage_document.h"
#include "../ui/canvas/slot_renderer.h"
#include <QPainter>
#include <QImageWriter>
#include <QDebug>
#include <cmath>

namespace PhotoColla::Export {

QImage Exporter::renderToImage(const Core::CollageDocument* document, const ExportOptions& options)
{
    if (!document || options.targetSize.isEmpty()) {
        return QImage();
    }

    QImage::Format imgFormat = (options.format.toLower() == "jpg" || options.format.toLower() == "jpeg")
        ? QImage::Format_RGB32
        : QImage::Format_ARGB32_Premultiplied;

    QImage outputImage(options.targetSize, imgFormat);

    // Set DPI metadata for commercial print workflows
    const double metersPerInch = 0.0254;
    const int dotsPerMeter = static_cast<int>(std::round(options.dpi / metersPerInch));
    outputImage.setDotsPerMeterX(dotsPerMeter);
    outputImage.setDotsPerMeterY(dotsPerMeter);

    // High quality painter
    QPainter painter(&outputImage);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);

    // Fill background
    painter.fillRect(outputImage.rect(), document->backgroundColor());

    // Scale painter to target dimensions
    QSizeF docSize = document->canvasSize();
    QSizeF targetSizeF = options.targetSize;

    double scaleX = targetSizeF.width() / docSize.width();
    double scaleY = targetSizeF.height() / docSize.height();

    painter.save();
    painter.scale(scaleX, scaleY);

    // Render slots cleanly without selection or edit handles
    for (const auto& slot : document->slotList()) {
        UI::SlotRenderer::renderSlot(painter, slot, docSize, false, false);
    }

    painter.restore();
    painter.end();

    return outputImage;
}

bool Exporter::exportToFile(const Core::CollageDocument* document, const ExportOptions& options)
{
    if (options.outputPath.isEmpty()) {
        qWarning() << "Export error: Output path is empty.";
        return false;
    }

    QImage img = renderToImage(document, options);
    if (img.isNull()) {
        qWarning() << "Export error: Failed to render image.";
        return false;
    }

    QImageWriter writer(options.outputPath, options.format.toUtf8());
    writer.setQuality(options.quality);

    if (!writer.write(img)) {
        qWarning() << "Export error: Failed to write to" << options.outputPath << ":" << writer.errorString();
        return false;
    }

    qInfo() << "Export successfully generated at:" << options.outputPath
            << "Dimensions:" << options.targetSize.width() << "x" << options.targetSize.height()
            << "DPI:" << options.dpi;
    return true;
}

} // namespace PhotoColla::Export
