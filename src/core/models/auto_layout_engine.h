#pragma once

#include <QStringList>
#include <vector>

namespace PhotoColla::Core {

class CollageDocument;

/**
 * @brief Automatically generates an optimal masonry/justified grid layout
 * based on the aspect ratios of the provided images.
 */
class AutoLayoutEngine {
public:
    static bool generateAndApply(CollageDocument* doc, const QStringList& imagePaths);
};

} // namespace PhotoColla::Core
