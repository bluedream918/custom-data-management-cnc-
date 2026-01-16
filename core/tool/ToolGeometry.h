#pragma once

#include "../common/Types.h"
#include "ToolType.h"
#include <cmath>

namespace cnc {

/**
 * @brief Pure geometric description of a cutting tool
 * 
 * Represents the physical geometry of a tool without any operational
 * parameters (RPM, feedrate, etc.). This is a value type that can be
 * used for collision detection, material removal simulation, and
 * tool path planning.
 * 
 * Coordinate system:
 * - Origin at tool tip
 * - Z-axis points up along tool centerline
 * - X/Y form horizontal plane
 * 
 * All dimensions are unitless (units determined by context).
 */
class ToolGeometry {
public:
    /**
     * @brief Construct tool geometry
     * @param diameter Tool cutting diameter
     * @param fluteLength Length of cutting flutes
     * @param overallLength Total tool length (tip to shank end)
     * @param shankDiameter Diameter of shank (non-cutting portion)
     * @param tipType Type of tool tip geometry
     */
    ToolGeometry(
        double diameter,
        double fluteLength,
        double overallLength,
        double shankDiameter,
        ToolTipType tipType = ToolTipType::Flat
    ) : diameter_(diameter > 0.0 ? diameter : 0.0),
        fluteLength_(fluteLength > 0.0 ? fluteLength : 0.0),
        overallLength_(overallLength > 0.0 ? overallLength : 0.0),
        shankDiameter_(shankDiameter > 0.0 ? shankDiameter : 0.0),
        tipType_(tipType) {
        // Ensure overall length >= flute length
        if (overallLength_ < fluteLength_) {
            overallLength_ = fluteLength_;
        }
        // Ensure shank diameter >= diameter (typical case)
        if (shankDiameter_ < diameter_) {
            shankDiameter_ = diameter_;
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
     * @brief Get shank diameter
     */
    double getShankDiameter() const {
        return shankDiameter_;
    }

    /**
     * @brief Get tool tip type
     */
    ToolTipType getTipType() const {
        return tipType_;
    }

    /**
     * @brief Get shank length (non-cutting portion)
     */
    double getShankLength() const {
        return overallLength_ - fluteLength_;
    }

    /**
     * @brief Get tool radius
     */
    double getRadius() const {
        return diameter_ * 0.5;
    }

    /**
     * @brief Get bounding box in tool coordinate system
     * 
     * Returns AABB with:
     * - Min: (-radius, -radius, -overallLength)
     * - Max: (radius, radius, 0) [tip at origin]
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
               shankDiameter_ > 0.0 &&
               overallLength_ >= fluteLength_ &&
               std::isfinite(diameter_) &&
               std::isfinite(fluteLength_) &&
               std::isfinite(overallLength_) &&
               std::isfinite(shankDiameter_);
    }

    /**
     * @brief Check if tool has ball tip
     */
    bool isBallTip() const {
        return tipType_ == ToolTipType::Ball;
    }

    /**
     * @brief Check if tool has flat tip
     */
    bool isFlatTip() const {
        return tipType_ == ToolTipType::Flat;
    }

    /**
     * @brief Check if tool has pointed tip
     */
    bool isPointedTip() const {
        return tipType_ == ToolTipType::Point;
    }

    /**
     * @brief Get tip radius (for ball end mills)
     * 
     * Returns tool radius for ball tips, 0 for flat/pointed tips.
     */
    double getTipRadius() const {
        if (tipType_ == ToolTipType::Ball) {
            return getRadius();
        }
        return 0.0;
    }

private:
    double diameter_;        ///< Cutting diameter
    double fluteLength_;     ///< Length of cutting flutes
    double overallLength_;   ///< Total tool length
    double shankDiameter_;   ///< Shank diameter
    ToolTipType tipType_;   ///< Tip geometry type
};

} // namespace cnc
