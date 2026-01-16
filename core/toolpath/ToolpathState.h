#pragma once

#include "../common/Types.h"
#include <string>
#include <optional>
#include <array>
#include <cmath>

namespace cnc {

/**
 * @brief Coordinate mode for toolpath
 */
enum class CoordinateMode {
    Absolute,   ///< Absolute coordinates (G90)
    Incremental ///< Incremental coordinates (G91)
};

/**
 * @brief Coolant state
 */
enum class CoolantState {
    Off,        ///< Coolant off
    Flood,      ///< Flood coolant (M8)
    Mist,       ///< Mist coolant (M7)
    Through     ///< Through-tool coolant
};

/**
 * @brief Represents machine state at any point in a toolpath
 * 
 * Encapsulates complete machine state including position, feed rate,
 * spindle speed, active tool, and control states. This is an immutable
 * value object that represents a snapshot of machine state.
 * 
 * Toolpath-Simulation Integration:
 * - State represents simulation checkpoint
 * - Position enables material removal calculation
 * - Feed/RPM enable time estimation
 * - Tool ID enables tool geometry lookup
 * 
 * Toolpath-G-code Integration:
 * - State maps to G-code modal state
 * - Position maps to X/Y/Z/A/B/C coordinates
 * - Feed/RPM map to F/S values
 * - Tool ID maps to T value
 * 
 * Toolpath-RL Integration:
 * - State represents RL state space
 * - Immutable design enables state comparison
 * - Comparable for state hashing
 * - Serializable for state persistence
 * 
 * Industrial control assumptions:
 * - State is immutable (value object)
 * - All positions are in machine coordinates
 * - Feed rate is in units per minute
 * - Spindle speed is in RPM
 * - Coordinate mode applies to all axes
 */
class ToolpathState {
public:
    /**
     * @brief Construct toolpath state
     * @param position Position (X, Y, Z, optional A, B, C)
     * @param feedRate Feed rate (units/min, 0 = not set)
     * @param spindleRPM Spindle speed (RPM, 0 = stopped)
     * @param activeToolId Active tool identifier (empty = no tool)
     * @param coolantState Coolant state
     * @param coordinateMode Coordinate mode (absolute/incremental)
     */
    ToolpathState(
        const Vec3& position,
        double feedRate = 0.0,
        double spindleRPM = 0.0,
        const std::string& activeToolId = "",
        CoolantState coolantState = CoolantState::Off,
        CoordinateMode coordinateMode = CoordinateMode::Absolute
    ) : position_(position),
        rotaryAxes_{0.0, 0.0, 0.0},  // A, B, C default to 0
        feedRate_(feedRate >= 0.0 ? feedRate : 0.0),
        spindleRPM_(spindleRPM >= 0.0 ? spindleRPM : 0.0),
        activeToolId_(activeToolId),
        coolantState_(coolantState),
        coordinateMode_(coordinateMode) {
    }

    /**
     * @brief Construct toolpath state with rotary axes
     */
    ToolpathState(
        const Vec3& position,
        const std::array<double, 3>& rotaryAxes,  // A, B, C
        double feedRate = 0.0,
        double spindleRPM = 0.0,
        const std::string& activeToolId = "",
        CoolantState coolantState = CoolantState::Off,
        CoordinateMode coordinateMode = CoordinateMode::Absolute
    ) : position_(position),
        rotaryAxes_(rotaryAxes),
        feedRate_(feedRate >= 0.0 ? feedRate : 0.0),
        spindleRPM_(spindleRPM >= 0.0 ? spindleRPM : 0.0),
        activeToolId_(activeToolId),
        coolantState_(coolantState),
        coordinateMode_(coordinateMode) {
    }

    /**
     * @brief Get position (X, Y, Z)
     */
    const Vec3& getPosition() const {
        return position_;
    }

    /**
     * @brief Get rotary axes (A, B, C)
     */
    const std::array<double, 3>& getRotaryAxes() const {
        return rotaryAxes_;
    }

    /**
     * @brief Get A-axis position
     */
    double getA() const {
        return rotaryAxes_[0];
    }

    /**
     * @brief Get B-axis position
     */
    double getB() const {
        return rotaryAxes_[1];
    }

    /**
     * @brief Get C-axis position
     */
    double getC() const {
        return rotaryAxes_[2];
    }

    /**
     * @brief Get feed rate
     */
    double getFeedRate() const {
        return feedRate_;
    }

    /**
     * @brief Check if feed rate is set
     */
    bool hasFeedRate() const {
        return feedRate_ > 0.0;
    }

    /**
     * @brief Get spindle RPM
     */
    double getSpindleRPM() const {
        return spindleRPM_;
    }

    /**
     * @brief Check if spindle is running
     */
    bool isSpindleRunning() const {
        return spindleRPM_ > 0.0;
    }

    /**
     * @brief Get active tool ID
     */
    const std::string& getActiveToolId() const {
        return activeToolId_;
    }

    /**
     * @brief Check if tool is active
     */
    bool hasActiveTool() const {
        return !activeToolId_.empty();
    }

    /**
     * @brief Get coolant state
     */
    CoolantState getCoolantState() const {
        return coolantState_;
    }

    /**
     * @brief Check if coolant is on
     */
    bool isCoolantOn() const {
        return coolantState_ != CoolantState::Off;
    }

    /**
     * @brief Get coordinate mode
     */
    CoordinateMode getCoordinateMode() const {
        return coordinateMode_;
    }

    /**
     * @brief Check if using absolute coordinates
     */
    bool isAbsoluteMode() const {
        return coordinateMode_ == CoordinateMode::Absolute;
    }

    /**
     * @brief Check if using incremental coordinates
     */
    bool isIncrementalMode() const {
        return coordinateMode_ == CoordinateMode::Incremental;
    }

    /**
     * @brief Check if state is valid
     */
    bool isValid() const {
        return std::isfinite(position_.x) &&
               std::isfinite(position_.y) &&
               std::isfinite(position_.z) &&
               std::isfinite(rotaryAxes_[0]) &&
               std::isfinite(rotaryAxes_[1]) &&
               std::isfinite(rotaryAxes_[2]) &&
               std::isfinite(feedRate_) &&
               std::isfinite(spindleRPM_);
    }

    /**
     * @brief Equality comparison
     */
    bool operator==(const ToolpathState& other) const {
        return position_.x == other.position_.x &&
               position_.y == other.position_.y &&
               position_.z == other.position_.z &&
               rotaryAxes_[0] == other.rotaryAxes_[0] &&
               rotaryAxes_[1] == other.rotaryAxes_[1] &&
               rotaryAxes_[2] == other.rotaryAxes_[2] &&
               feedRate_ == other.feedRate_ &&
               spindleRPM_ == other.spindleRPM_ &&
               activeToolId_ == other.activeToolId_ &&
               coolantState_ == other.coolantState_ &&
               coordinateMode_ == other.coordinateMode_;
    }

    /**
     * @brief Inequality comparison
     */
    bool operator!=(const ToolpathState& other) const {
        return !(*this == other);
    }

private:
    Vec3 position_;                     ///< Position (X, Y, Z)
    std::array<double, 3> rotaryAxes_;  ///< Rotary axes (A, B, C)
    double feedRate_;                   ///< Feed rate (units/min)
    double spindleRPM_;                ///< Spindle speed (RPM)
    std::string activeToolId_;          ///< Active tool identifier
    CoolantState coolantState_;         ///< Coolant state
    CoordinateMode coordinateMode_;     ///< Coordinate mode
};

} // namespace cnc
