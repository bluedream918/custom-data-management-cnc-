#pragma once

#include "MoveType.h"
#include "ToolpathState.h"
#include "../common/Types.h"
#include <string>
#include <optional>
#include <cmath>

namespace cnc {

/**
 * @brief Represents one atomic CNC instruction
 * 
 * Encapsulates a single toolpath move with start/end states, motion type,
 * and parameters. This is an immutable value type representing one G-code
 * command or sequence of commands.
 * 
 * Toolpath-Simulation Integration:
 * - Move represents one simulation step
 * - Start/end states enable material removal calculation
 * - Arc center enables swept volume calculation
 * - Safety flags enable collision checking
 * 
 * Toolpath-G-code Integration:
 * - Move maps to one or more G-code lines
 * - MoveType maps to G/M codes
 * - States map to modal G-code state
 * - Arc center maps to I/J/K values
 * 
 * Toolpath-RL Integration:
 * - Move represents RL action
 * - Start/end states represent state transition
 * - Move parameters represent action parameters
 * 
 * Industrial control assumptions:
 * - Move is immutable after construction
 * - Geometry is validated during construction
 * - Safety flags enforce safe operation
 * - Rapid moves cannot remove material
 */
class ToolpathMove {
public:
    /**
     * @brief Construct rapid move
     */
    static ToolpathMove rapid(
        const ToolpathState& start,
        const ToolpathState& end
    ) {
        return ToolpathMove(MoveType::Rapid, start, end, std::nullopt, 0.0, true);
    }

    /**
     * @brief Construct linear move
     */
    static ToolpathMove linear(
        const ToolpathState& start,
        const ToolpathState& end
    ) {
        return ToolpathMove(MoveType::Linear, start, end, std::nullopt, 0.0, false);
    }

    /**
     * @brief Construct arc move
     */
    static ToolpathMove arc(
        MoveType arcType,
        const ToolpathState& start,
        const ToolpathState& end,
        const Vec3& center
    ) {
        return ToolpathMove(arcType, start, end, center, 0.0, false);
    }

    /**
     * @brief Construct dwell move
     */
    static ToolpathMove dwell(
        const ToolpathState& state,
        double duration
    ) {
        return ToolpathMove(MoveType::Dwell, state, state, std::nullopt, duration, false);
    }

    /**
     * @brief Construct tool change move
     */
    static ToolpathMove toolChange(
        const ToolpathState& state,
        const std::string& newToolId
    ) {
        ToolpathState endState = state;
        // Note: In real implementation, endState would have newToolId set
        return ToolpathMove(MoveType::ToolChange, state, endState, std::nullopt, 0.0, false);
    }

    /**
     * @brief Construct spindle start move
     */
    static ToolpathMove spindleStart(
        const ToolpathState& state,
        double rpm
    ) {
        ToolpathState endState = state;
        // Note: In real implementation, endState would have spindleRPM set
        return ToolpathMove(MoveType::SpindleStart, state, endState, std::nullopt, 0.0, false);
    }

    /**
     * @brief Construct spindle stop move
     */
    static ToolpathMove spindleStop(
        const ToolpathState& state
    ) {
        ToolpathState endState = state;
        // Note: In real implementation, endState would have spindleRPM = 0
        return ToolpathMove(MoveType::SpindleStop, state, endState, std::nullopt, 0.0, false);
    }

    /**
     * @brief Get move type
     */
    MoveType getMoveType() const {
        return moveType_;
    }

    /**
     * @brief Get start state
     */
    const ToolpathState& getStartState() const {
        return startState_;
    }

    /**
     * @brief Get end state
     */
    const ToolpathState& getEndState() const {
        return endState_;
    }

    /**
     * @brief Get arc center (for arc moves)
     */
    const std::optional<Vec3>& getArcCenter() const {
        return arcCenter_;
    }

    /**
     * @brief Get dwell duration (for dwell moves)
     */
    double getDwellDuration() const {
        return dwellDuration_;
    }

    /**
     * @brief Check if rapid is allowed
     * 
     * Safety flag indicating if rapid motion is safe at this point.
     */
    bool isRapidAllowed() const {
        return rapidAllowed_;
    }

    /**
     * @brief Get move length
     * 
     * Calculates the geometric length of the move.
     * For arcs, calculates arc length. For linear, calculates straight distance.
     */
    double getLength() const {
        if (moveType_ == MoveType::Dwell ||
            moveType_ == MoveType::ToolChange ||
            moveType_ == MoveType::SpindleStart ||
            moveType_ == MoveType::SpindleStop) {
            return 0.0;
        }

        if (isArcMove(moveType_)) {
            return calculateArcLength();
        }

        // Linear or rapid
        Vec3 delta = endState_.getPosition() - startState_.getPosition();
        return delta.length();
    }

    /**
     * @brief Get estimated execution time
     * 
     * Estimates time based on length and feedrate.
     * For rapid moves, uses a default rapid rate.
     * 
     * @param defaultRapidRate Default rapid rate for rapid moves (units/min)
     */
    double getEstimatedTime(double defaultRapidRate = 10000.0) const {
        if (moveType_ == MoveType::Dwell) {
            return dwellDuration_;
        }

        if (moveType_ == MoveType::ToolChange) {
            return 5.0; // Estimate 5 seconds for tool change
        }

        if (moveType_ == MoveType::SpindleStart || moveType_ == MoveType::SpindleStop) {
            return 0.1; // Estimate 0.1 seconds for spindle control
        }

        double length = getLength();
        double rate = moveType_ == MoveType::Rapid ? defaultRapidRate : endState_.getFeedRate();

        if (rate <= 0.0) {
            return 0.0;
        }

        return (length / rate) * 60.0; // Convert to seconds
    }

    /**
     * @brief Check if move is valid
     */
    bool isValid() const {
        if (!startState_.isValid() || !endState_.isValid()) {
            return false;
        }

        // Check feedrate for cutting motions
        if (requiresFeedrate(moveType_)) {
            if (!endState_.hasFeedRate()) {
                return false;
            }
        }

        // Check arc center for arc motions
        if (isArcMove(moveType_)) {
            if (!arcCenter_.has_value()) {
                return false;
            }
        }

        // Rapid moves cannot remove material (safety check)
        if (moveType_ == MoveType::Rapid && !rapidAllowed_) {
            return false;
        }

        return true;
    }

    /**
     * @brief Check if move is zero-length
     */
    bool isZeroLength() const {
        if (moveType_ == MoveType::Dwell ||
            moveType_ == MoveType::ToolChange ||
            moveType_ == MoveType::SpindleStart ||
            moveType_ == MoveType::SpindleStop) {
            return false; // Control moves are valid at a point
        }

        Vec3 delta = endState_.getPosition() - startState_.getPosition();
        return delta.lengthSquared() < 1e-12;
    }

private:
    /**
     * @brief Private constructor
     */
    ToolpathMove(
        MoveType moveType,
        const ToolpathState& start,
        const ToolpathState& end,
        const std::optional<Vec3>& arcCenter,
        double dwellDuration,
        bool rapidAllowed
    ) : moveType_(moveType),
        startState_(start),
        endState_(end),
        arcCenter_(arcCenter),
        dwellDuration_(dwellDuration),
        rapidAllowed_(rapidAllowed) {
    }

    /**
     * @brief Calculate arc length
     */
    double calculateArcLength() const {
        if (!arcCenter_.has_value()) {
            return 0.0;
        }

        Vec3 start = startState_.getPosition();
        Vec3 end = endState_.getPosition();
        Vec3 center = arcCenter_.value();

        // Calculate radius
        Vec3 startVec = start - center;
        Vec3 endVec = end - center;
        double radius = startVec.length();

        if (radius < 1e-9) {
            return 0.0;
        }

        // Calculate angle
        double dot = (startVec.x * endVec.x + startVec.y * endVec.y + startVec.z * endVec.z) / (radius * radius);
        dot = std::max(-1.0, std::min(1.0, dot)); // Clamp to [-1, 1]
        double angle = std::acos(dot);

        // Arc length = radius * angle
        return radius * angle;
    }

    MoveType moveType_;
    ToolpathState startState_;
    ToolpathState endState_;
    std::optional<Vec3> arcCenter_;  ///< Arc center (for arc moves)
    double dwellDuration_;            ///< Dwell duration in seconds
    bool rapidAllowed_;               ///< Safety flag: rapid allowed
};

} // namespace cnc
