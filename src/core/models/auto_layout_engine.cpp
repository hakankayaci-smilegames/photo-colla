#include "auto_layout_engine.h"
#include "collage_document.h"
#include "../commands/collage_commands.h"
#include <QImageReader>
#include <QDebug>
#include <cmath>
#include <algorithm>
#include <random>

namespace PhotoColla::Core {

struct ImageInfo {
    QString path;
    double aspectRatio;
};

static double evaluatePartition(const std::vector<ImageInfo>& items, const std::vector<int>& rowCounts, double targetH, double& outMaxCrop) {
    double totalH = 0.0;
    int idx = 0;
    std::vector<double> rowHeights;
    std::vector<double> rowAspects;

    for (int count : rowCounts) {
        double rowAspect = 0.0;
        for (int i = 0; i < count; ++i) {
            rowAspect += items[idx + i].aspectRatio;
        }
        double h = 1.0 / rowAspect;
        totalH += h;
        rowHeights.push_back(h);
        rowAspects.push_back(rowAspect);
        idx += count;
    }

    double maxCrop = 0.0;
    idx = 0;
    for (size_t r = 0; r < rowCounts.size(); ++r) {
        double actualRowH = rowHeights[r] / totalH * targetH;
        for (int i = 0; i < rowCounts[r]; ++i) {
            double actualW = (items[idx + i].aspectRatio / rowAspects[r]) * 1.0; // target width is 1.0
            double slotAspect = actualW / actualRowH;
            double imgAspect = items[idx + i].aspectRatio;
            double crop = std::max(slotAspect / imgAspect, imgAspect / slotAspect);
            if (crop > maxCrop) maxCrop = crop;
        }
        idx += rowCounts[r];
    }

    return maxCrop;
}

static std::vector<int> findBestRowPartition(const std::vector<ImageInfo>& items, double targetH) {
    int n = items.size();
    // DP to find partition that minimizes something?
    // Let's just use a simple recursive search since N is small (e.g. max 20-30 for collages usually).
    // Or DP over contiguous segments.
    // dp[i] = min cost to partition items 0..i-1.
    // Since we want to minimize the maximum crop factor globally, which depends on totalH,
    // it's not strictly DP-able. But totalH is close to targetH.
    // Let's just approximate by making sum(1/A_r) close to targetH.
    
    std::vector<double> dp(n + 1, 1e9);
    std::vector<int> prev(n + 1, -1);
    dp[0] = 0.0;
    
    for (int i = 1; i <= n; ++i) {
        double currentAspect = 0.0;
        for (int j = i - 1; j >= 0; --j) {
            currentAspect += items[j].aspectRatio;
            double rowH = 1.0 / currentAspect;
            // Cost is how much this row deviates from the "ideal" height if we had uniform rows
            // But we actually want sum of rowH to be targetH.
            // Let's just use a squared error approach:
            double error = dp[j] + rowH;
            // Wait, we want the sum to be exactly targetH. DP state doesn't track sum.
            // Let's use a simpler heuristic cost for the row.
            // Ideal row height is roughly targetH / sqrt(n).
            double idealRowH = targetH / std::sqrt(n);
            double cost = dp[j] + std::abs(rowH - idealRowH);
            
            if (cost < dp[i]) {
                dp[i] = cost;
                prev[i] = j;
            }
        }
    }
    
    std::vector<int> counts;
    int curr = n;
    while (curr > 0) {
        int p = prev[curr];
        counts.push_back(curr - p);
        curr = p;
    }
    std::reverse(counts.begin(), counts.end());
    return counts;
}

bool AutoLayoutEngine::generateAndApply(CollageDocument* doc, const QStringList& imagePaths) {
    if (imagePaths.isEmpty() || !doc) return false;

    std::vector<ImageInfo> items;
    for (const QString& path : imagePaths) {
        QImageReader reader(path);
        QSize sz = reader.size();
        if (!sz.isValid()) sz = QSize(1000, 1000); // fallback
        double aspect = static_cast<double>(sz.width()) / sz.height();
        items.push_back({path, aspect});
    }

    double targetW = 1.0;
    double targetH = doc->canvasSize().height() / doc->canvasSize().width();
    
    // We will try a few permutations (e.g. sorted by aspect, random) and pick the one with lowest max crop
    std::vector<std::vector<ImageInfo>> candidates;
    candidates.push_back(items); // original
    
    auto sortedAspect = items;
    std::sort(sortedAspect.begin(), sortedAspect.end(), [](const ImageInfo& a, const ImageInfo& b) {
        return a.aspectRatio > b.aspectRatio;
    });
    candidates.push_back(sortedAspect);

    std::random_device rd;
    std::mt19937 g(rd());
    for (int i = 0; i < 20; ++i) {
        auto shuffled = items;
        std::shuffle(shuffled.begin(), shuffled.end(), g);
        candidates.push_back(shuffled);
    }

    double bestCrop = 1e9;
    std::vector<int> bestPartition;
    std::vector<ImageInfo> bestItems;
    bool bestIsColumnBased = false;

    for (const auto& cand : candidates) {
        // Try Row based
        auto counts = findBestRowPartition(cand, targetH);
        double maxCrop = 0;
        evaluatePartition(cand, counts, targetH, maxCrop);
        if (maxCrop < bestCrop) {
            bestCrop = maxCrop;
            bestPartition = counts;
            bestItems = cand;
            bestIsColumnBased = false;
        }
        
        // Try Column based (transpose everything)
        // For column based, we want to stack items vertically into columns.
        // Aspect ratio of an item for column logic is 1.0 / aspect.
        std::vector<ImageInfo> transCand = cand;
        for (auto& item : transCand) item.aspectRatio = 1.0 / item.aspectRatio;
        
        auto colCounts = findBestRowPartition(transCand, targetW / targetH);
        double colMaxCrop = 0;
        evaluatePartition(transCand, colCounts, targetW / targetH, colMaxCrop);
        if (colMaxCrop < bestCrop) {
            bestCrop = colMaxCrop;
            bestPartition = colCounts;
            bestItems = cand; // keep original aspects
            bestIsColumnBased = true;
        }
    }

    // Now construct the final slots
    std::vector<Slot> newSlots;
    if (!bestIsColumnBased) {
        double totalH = 0.0;
        int idx = 0;
        std::vector<double> rowHeights;
        std::vector<double> rowAspects;

        for (int count : bestPartition) {
            double rowAspect = 0.0;
            for (int i = 0; i < count; ++i) {
                rowAspect += bestItems[idx + i].aspectRatio;
            }
            double h = 1.0 / rowAspect;
            totalH += h;
            rowHeights.push_back(h);
            rowAspects.push_back(rowAspect);
            idx += count;
        }

        double currentY = 0.0;
        idx = 0;
        for (size_t r = 0; r < bestPartition.size(); ++r) {
            double actualRowH = rowHeights[r] / totalH;
            double currentX = 0.0;
            for (int i = 0; i < bestPartition[r]; ++i) {
                double actualW = (bestItems[idx].aspectRatio / rowAspects[r]);
                
                Slot slot(QString("auto_%1").arg(idx), QRectF(currentX, currentY, actualW, actualRowH));
                QPixmap pm(bestItems[idx].path);
                slot.setImage(bestItems[idx].path, pm);
                slot.fitImageToSlot(doc->canvasSize());
                newSlots.push_back(slot);
                
                currentX += actualW;
                idx++;
            }
            currentY += actualRowH;
        }
    } else {
        // Column based
        double totalW = 0.0;
        int idx = 0;
        std::vector<double> colWidths;
        std::vector<double> colAspects; // sums of (1/A)

        for (int count : bestPartition) {
            double colAspect = 0.0;
            for (int i = 0; i < count; ++i) {
                colAspect += (1.0 / bestItems[idx + i].aspectRatio);
            }
            double w = 1.0 / colAspect;
            totalW += w;
            colWidths.push_back(w);
            colAspects.push_back(colAspect);
            idx += count;
        }

        double currentX = 0.0;
        idx = 0;
        for (size_t c = 0; c < bestPartition.size(); ++c) {
            double actualColW = colWidths[c] / totalW;
            double currentY = 0.0;
            for (int i = 0; i < bestPartition[c]; ++i) {
                double actualH = (1.0 / bestItems[idx].aspectRatio) / colAspects[c];
                
                Slot slot(QString("auto_%1").arg(idx), QRectF(currentX, currentY, actualColW, actualH));
                QPixmap pm(bestItems[idx].path);
                slot.setImage(bestItems[idx].path, pm);
                slot.fitImageToSlot(doc->canvasSize());
                newSlots.push_back(slot);
                
                currentY += actualH;
                idx++;
            }
            currentX += actualColW;
        }
    }

    doc->setSlots(newSlots);
    return true;
}

} // namespace PhotoColla::Core
