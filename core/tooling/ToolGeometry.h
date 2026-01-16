#pragma once

#include "../common/Types.h"
#include <cmath>
#include <algorithm>

namespace cnc {

/**
 * @brief Physical geometry of a cutting tool
 * 
 * Represents the complete geometric description of a tool's physical
 * dimensions. All measurements are unit-agnostic (caller decides mm/inch).
 * 
 * Coordinate system:
 * - Origin at tool tip
 * - Z-axis points up along tool centerline
 * - X/Y form horizontal plane
 * 
 * Industrial control assumptions:
 * - All dimensions are positive and finite
 * - Lengths are consistent (overall >= flute + shoulder)
 * - Corner radius is 0 for flat end mills
 * - Tolerance represents manufacturing precision
 */
class ToolGeometry {
public:
    /**
     * @brief Construct tool geometry
     * @param diameter Tool cutting diameter
     * @param fluteLength Length of cutting flutes
     * @param overallLength Total tool length (tip to shank end)
     * @param shoulderLength Length of shoulder (non-cutting portion between flutes and shank)
     * @param cornerRadius Corner radius at tool tip (0 for flat end mills)
     * @param tolerance Manufacturing tolerance
     */
    ToolGeometry(
        double diameter,
        double fluteLength,
        double overallLength,
        double shoulderLength = 0.0,
        double cornerRadius = 0.0,
        double tolerance = 0.001
    ) : diameter_(diameter > 0.0 ? diameter : 0.0),
        fluteLength_(fluteLength > 0.0 ? fluteLength : 0.0),
        overallLength_(overallLength > 0.0 ? overallLength : 0.0),
        shoulderLength_(shoulderLength >= 0.0 ? shoulderLength : 0.0),
        cornerRadius_(cornerRadius >= 0.0 ? cornerRadius : 0.0),
        tolerance_(tolerance >= 0.0 ? tolerance : 0.001) {
        // Ensure overall length >= flute length + shoulder length
        double minLength = fluteLength_ + shoulderLength_;
        if (overallLength_ < minLength) {
            overallLength_ = minLength;
        }
        // Corner radius cannot exceed tool radius
        double maxRadius = diameter_ * 0.5;
        if (cornerRadius_ > maxRadius) {
            cornerRadius_ = maxRadius;
        }
    }

    /**
     * @brief Get tool cutting diameter
     */
    double getDiameter() const {
        return diameter_;
    }

    /**
     * @brief Get flute length (cutting length)
     */
    double getFluteLength() const {
        return fluteLength_;
    }

    /**
     * @brief Get overall tool length
     */
    double getOverallLength() const {
        return overallLength_;
    }

    /**
     * @brief Get shoulder length
     */
    double getShoulderLength() const {
        return shoulderLength_;
    }

    /**
     * @brief Get corner radius
     */
    double getCornerRadius() const {
        return cornerRadius_;
    }

    /**
     * @brief Get tolerance
     */
    double getTolerance() const {
        return tolerance_;
    }

    /**
     * @brief Get tool radius
     */
    double getRadius() const {
        return diameter_ * 0.5;
    }

    /**
     * @brief Get shank length (non-cutting portion)
     */
    double getShankLength() const {
        return overallLength_ - fluteLength_ - shoulderLength_;
    }

    /**
     * @brief Check if tool has flat tip (corner radius = 0)
     */
    bool isFlatTip() const {
        return cornerRadius_ < 1e-9;
    }

    /**
     * @brief Check if tool has rounded tip (corner radius > 0)
     */
    bool isRoundedTip() const {
        return cornerRadius_ > 1e-9;
    }

    /**
     * @brief Get effective cutting radius at given depth
     * 
     * For flat end mills, returns constant radius.
     * For ball end mills, returns radius at depth from tip.
     * 
     * @param depth Depth from tool tip
     * @return Effective cutting radius
     */
    double getEffectiveRadius(double depth) const {
        if (isFlatTip()) {
            return getRadius();
        }
        // For rounded tip, calculate effective radius
        // This is a simplified model - actual ball end mill geometry is more complex
        if (depth <= cornerRadius_) {
            double r = cornerRadius_;
            return std::sqrt(r * r - (r - depth) * (r - depth));
        }
        return getRadius();
    }

    /**
     * @brief Get bounding box in tool coordinate system
     */
    AABB getBoundingBox() const {
        double radius = getRadius();
        return AABB(
            Vec3(-radius, -radius, -overallLength_),
            Vec3(radius, radius, 0.0)
        );
    }

    /**
     * @brief Check if geometry is valid
     */
    bool isValid() const {
        return diameter_ > 0.0 &&
               fluteLength_ > 0.0 &&
               overallLength_ > 0.0 &&
               shoulderLength_ >= 0.0 &&
               cornerRadius_ >= 0.0 &&
               tolerance_ >= 0.0 &&
               overallLength_ >= (fluteLength_ + shoulderLength_) &&
               cornerRadius_ <= (diameter_ * 0.5) &&
               std::isfinite(diameter_) &&
               std::isfinite(fluteLength_) &&
               std::isfinite(overallLength_) &&
               std::isfinite(shoulderLength_) &&
               std::isfinite(cornerRadius_) &&
               std::isfinite(tolerance_);
    }

private:
    double diameter_;        ///< Cutting diameter
    double fluteLength_;     ///< Length of cutting flutes
    double overallLength_;   ///< Total tool length
    double shoulderLength_;  ///< Shoulder length (between flutes and shank)
    double cornerRadius_;    ///< Corner radius at tool tip
    double tolerance_;        ///< Manufacturing tolerance
};

} // namespace cnc
