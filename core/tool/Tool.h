#pragma once

#include "ToolGeometry.h"
#include "../common/Types.h"
#include <string>
#include <memory>

namespace cnc {

/**
 * @brief Logical tool definition
 * 
 * Represents a complete tool specification including geometry,
 * type, and operational limits. This is an immutable value type
 * that can be safely shared and copied.
 * 
 * Industrial control assumptions:
 * - Tool is immutable after construction
 * - All parameters are deterministic
 * - Units are context-dependent (mm or inches)
 * - Max RPM and feedrate are safety limits
 */
class Tool {
public:
    /**
     * @brief Construct tool
     * @param id Unique tool identifier
     * @param name Tool display name
     * @param type Tool type classification
     * @param geometry Tool geometry
     * @param maxRPM Maximum spindle speed (RPM)
     * @param maxFeedrate Maximum feed rate (units/min)
     */
    Tool(
        std::string id,
        std::string name,
        ToolType type,
        ToolGeometry geometry,
        double maxRPM = 24000.0,
        double maxFeedrate = 10000.0
    ) : id_(std::move(id)),
        name_(std::move(name)),
        type_(type),
        geometry_(geometry),
        maxRPM_(maxRPM > 0.0 ? maxRPM : 24000.0),
        maxFeedrate_(maxFeedrate > 0.0 ? maxFeedrate : 10000.0) {
    }

    /**
     * @brief Get tool identifier
     */
    const std::string& getId() const {
        return id_;
    }

    /**
     * @brief Get tool display name
     */
    const std::string& getName() const {
        return name_;
    }

    /**
     * @brief Get tool type
     */
    ToolType getType() const {
        return type_;
    }

    /**
     * @brief Get tool geometry
     */
    const ToolGeometry& getGeometry() const {
        return geometry_;
    }

    /**
     * @brief Get maximum RPM
     */
    double getMaxRPM() const {
        return maxRPM_;
    }

    /**
     * @brief Get maximum feedrate
     */
    double getMaxFeedrate() const {
        return maxFeedrate_;
    }

    /**
     * @brief Get tool diameter (convenience)
     */
    double getDiameter() const {
        return geometry_.getDiameter();
    }

    /**
     * @brief Get tool length (convenience - flute length)
     */
    double getLength() const {
        return geometry_.getFluteLength();
    }

    /**
     * @brief Get total tool length (convenience)
     */
    double getTotalLength() const {
        return geometry_.getOverallLength();
    }

    /**
     * @brief Get shank diameter (convenience)
     */
    double getShankDiameter() const {
        return geometry_.getShankDiameter();
    }

    /**
     * @brief Get tool bounding box
     */
    AABB getBoundingBox() const {
        return geometry_.getBoundingBox();
    }

    /**
     * @brief Check if tool is valid
     */
    bool isValid() const {
        return !id_.empty() &&
               !name_.empty() &&
               geometry_.isValid() &&
               maxRPM_ > 0.0 &&
               maxFeedrate_ > 0.0;
    }

    /**
     * @brief Check if tool is ball end mill
     */
    bool isBallEndMill() const {
        return type_ == ToolType::BallEndMill || geometry_.isBallTip();
    }

    /**
     * @brief Check if tool is end mill
     */
    bool isEndMill() const {
        return type_ == ToolType::EndMill || type_ == ToolType::BallEndMill;
    }

    /**
     * @brief Check if tool is drill
     */
    bool isDrill() const {
        return type_ == ToolType::Drill;
    }

private:
    std::string id_;         ///< Unique tool identifier
    std::string name_;       ///< Tool display name
    ToolType type_;         ///< Tool type classification
    ToolGeometry geometry_; ///< Tool geometry
    double maxRPM_;         ///< Maximum spindle speed (RPM)
    double maxFeedrate_;    ///< Maximum feed rate (units/min)
};

} // namespace cnc
