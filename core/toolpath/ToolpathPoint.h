#pragma once

#include "../common/Types.h"
#include "../geometry/Transform.h"
#include <optional>

namespace cnc {

/**
 * @brief Represents a single tool position in a toolpath
 * 
 * Encapsulates tool position, optional orientation (for 5-axis),
 * and motion parameters. This is a value type used throughout
 * the toolpath system.
 * 
 * Coordinate system:
 * - Position: Tool tip position in workpiece coordinates
 * - Orientation: Tool orientation (quaternion) for 5-axis machines
 * - Default orientation: Vertical (Z-axis direction) for 3-axis
 * 
 * Industrial control assumptions:
 * - Position is always required
 * - Orientation is optional (defaults to vertical for 3-axis)
 * - Feedrate and spindle speed are optional (use defaults if not set)
 * - Flags indicate motion characteristics
 */
class ToolpathPoint {
public:
    /**
     * @brief Motion flags
     */
    struct Flags {
        bool isRapid;      ///< Rapid positioning (non-cutting)
        bool isCutting;    ///< Cutting motion (material removal)
        bool isPlunge;     ///< Plunge move (vertical entry)
        bool isRetract;    ///< Retract move (vertical exit)

        Flags() : isRapid(false), isCutting(false), isPlunge(false), isRetract(false) {}
    };

    /**
     * @brief Construct toolpath point
     * @param position Tool tip position
     * @param orientation Optional tool orientation (defaults to vertical)
     * @param feedrate Optional feedrate override (units/min)
     * @param spindleSpeed Optional spindle speed override (RPM)
     * @param flags Motion flags
     */
    ToolpathPoint(
        const Vec3& position,
        const std::optional<Quaternion>& orientation = std::nullopt,
        const std::optional<double>& feedrate = std::nullopt,
        const std::optional<double>& spindleSpeed = std::nullopt,
        const Flags& flags = Flags()
    ) : position_(position),
        orientation_(orientation.value_or(Quaternion::identity())),
        feedrate_(feedrate),
        spindleSpeed_(spindleSpeed),
        flags_(flags) {
    }

    /**
     * @brief Get tool tip position
     */
    const Vec3& getPosition() const {
        return position_;
    }

    /**
     * @brief Get tool orientation
     */
    const Quaternion& getOrientation() const {
        return orientation_;
    }

    /**
     * @brief Check if orientation is set (non-default)
     */
    bool hasOrientation() const {
        // Check if orientation is not identity (vertical)
        const Quaternion& q = orientation_;
        return !(std::abs(q.w - 1.0) < 1e-9 &&
                 std::abs(q.x) < 1e-9 &&
                 std::abs(q.y) < 1e-9 &&
                 std::abs(q.z) < 1e-9);
    }

    /**
     * @brief Get feedrate override
     */
    const std::optional<double>& getFeedrate() const {
        return feedrate_;
    }

    /**
     * @brief Check if feedrate is set
     */
    bool hasFeedrate() const {
        return feedrate_.has_value();
    }

    /**
     * @brief Get spindle speed override
     */
    const std::optional<double>& getSpindleSpeed() const {
        return spindleSpeed_;
    }

    /**
     * @brief Check if spindle speed is set
     */
    bool hasSpindleSpeed() const {
        return spindleSpeed_.has_value();
    }

    /**
     * @brief Get motion flags
     */
    const Flags& getFlags() const {
        return flags_;
    }

    /**
     * @brief Get motion flags (mutable)
     */
    Flags& getFlags() {
        return flags_;
    }

    /**
     * @brief Get tool transform
     * 
     * Returns a transform representing the tool pose at this point.
     */
    Transform getToolTransform() const {
        return Transform(position_, orientation_);
    }

    /**
     * @brief Check if point is valid
     */
    bool isValid() const {
        return std::isfinite(position_.x) &&
               std::isfinite(position_.y) &&
               std::isfinite(position_.z);
    }

    /**
     * @brief Check if point is rapid
     */
    bool isRapid() const {
        return flags_.isRapid;
    }

    /**
     * @brief Check if point is cutting
     */
    bool isCutting() const {
        return flags_.isCutting;
    }

private:
    Vec3 position_;                          ///< Tool tip position
    Quaternion orientation_;                 ///< Tool orientation (default: vertical)
    std::optional<double> feedrate_;         ///< Feedrate override (units/min)
    std::optional<double> spindleSpeed_;     ///< Spindle speed override (RPM)
    Flags flags_;                            ///< Motion flags
};

} // namespace cnc
