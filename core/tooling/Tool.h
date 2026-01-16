#pragma once

#include "ToolType.h"
#include "ToolGeometry.h"
#include "ToolHolder.h"
#include "../common/Types.h"
#include <string>

namespace cnc {

/**
 * @brief Full tool assembly (geometry + holder)
 * 
 * Represents a complete tool ready for use in CAM operations.
 * Combines tool geometry, holder, and operational parameters.
 * 
 * Tool-Toolpath Interaction:
 * - Tool geometry determines cutting capabilities
 * - Tool type determines suitable motion types
 * - Default feedrate/spindle speed are starting points for optimization
 * - Coolant mode affects material removal and tool life
 * 
 * Extension for 5-axis & Probing:
 * - Tool geometry supports orientation-dependent cutting
 * - Holder gauge length enables accurate tool tip calculation
 * - Tool ID enables tool change commands in toolpaths
 * - Future: Add probing tool support with touch-off capabilities
 * 
 * Industrial control assumptions:
 * - Tool is immutable after creation
 * - All parameters are deterministic
 * - Units are context-dependent (mm or inches)
 * - Default parameters are conservative starting points
 */
class Tool {
public:
    /**
     * @brief Construct tool
     * @param id Unique tool identifier
     * @param name Tool display name
     * @param type Tool type
     * @param geometry Tool geometry
     * @param holder Tool holder
     * @param defaultFeedrate Default feedrate (units/min)
     * @param defaultSpindleSpeed Default spindle speed (RPM)
     * @param coolantMode Coolant mode
     */
    Tool(
        std::string id,
        std::string name,
        ToolingType type,
        ToolGeometry geometry,
        ToolHolder holder,
        double defaultFeedrate = 1000.0,
        double defaultSpindleSpeed = 10000.0,
        CoolantMode coolantMode = CoolantMode::Flood
    ) : id_(std::move(id)),
        name_(std::move(name)),
        type_(type),
        geometry_(geometry),
        holder_(holder),
        defaultFeedrate_(defaultFeedrate > 0.0 ? defaultFeedrate : 1000.0),
        defaultSpindleSpeed_(defaultSpindleSpeed > 0.0 ? defaultSpindleSpeed : 10000.0),
        coolantMode_(coolantMode) {
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
    ToolingType getType() const {
        return type_;
    }

    /**
     * @brief Get tool geometry
     */
    const ToolGeometry& getGeometry() const {
        return geometry_;
    }

    /**
     * @brief Get tool holder
     */
    const ToolHolder& getHolder() const {
        return holder_;
    }

    /**
     * @brief Get default feedrate
     */
    double getDefaultFeedrate() const {
        return defaultFeedrate_;
    }

    /**
     * @brief Get default spindle speed
     */
    double getDefaultSpindleSpeed() const {
        return defaultSpindleSpeed_;
    }

    /**
     * @brief Get coolant mode
     */
    CoolantMode getCoolantMode() const {
        return coolantMode_;
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
     * @brief Get total length from spindle to tool tip
     * 
     * Returns gauge length plus tool overall length.
     */
    double getTotalLengthFromSpindle() const {
        return holder_.getGaugeLength() + geometry_.getOverallLength();
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
               holder_.isValid() &&
               defaultFeedrate_ > 0.0 &&
               defaultSpindleSpeed_ > 0.0;
    }

    /**
     * @brief Check if tool is end mill
     */
    bool isEndMill() const {
        return type_ == ToolingType::EndMill ||
               type_ == ToolingType::BallMill ||
               type_ == ToolingType::FlatMill;
    }

    /**
     * @brief Check if tool is ball mill
     */
    bool isBallMill() const {
        return type_ == ToolingType::BallMill;
    }

    /**
     * @brief Check if tool is drill
     */
    bool isDrill() const {
        return type_ == ToolingType::Drill;
    }

    /**
     * @brief Equality comparison (by ID)
     */
    bool operator==(const Tool& other) const {
        return id_ == other.id_;
    }

    /**
     * @brief Inequality comparison (by ID)
     */
    bool operator!=(const Tool& other) const {
        return id_ != other.id_;
    }

    /**
     * @brief Less-than comparison (by ID, for sorting)
     */
    bool operator<(const Tool& other) const {
        return id_ < other.id_;
    }

private:
    std::string id_;                    ///< Unique tool identifier
    std::string name_;                  ///< Tool display name
    ToolingType type_;                  ///< Tool type
    ToolGeometry geometry_;             ///< Tool geometry
    ToolHolder holder_;                  ///< Tool holder
    double defaultFeedrate_;            ///< Default feedrate (units/min)
    double defaultSpindleSpeed_;        ///< Default spindle speed (RPM)
    CoolantMode coolantMode_;           ///< Coolant mode
};

} // namespace cnc
