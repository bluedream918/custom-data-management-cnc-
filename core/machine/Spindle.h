#pragma once

#include <string>
#include <cmath>

namespace cnc {

/**
 * @brief Spindle rotation direction
 */
enum class SpindleDirection {
    Clockwise,      ///< Clockwise rotation (CW)
    CounterClockwise ///< Counter-clockwise rotation (CCW)
};

/**
 * @brief Represents spindle properties and capabilities
 * 
 * Encapsulates spindle specifications including speed range, power,
 * and operational characteristics. Used for toolpath planning and
 * G-code generation.
 * 
 * Industrial control assumptions:
 * - RPM range is positive and finite
 * - Power is in kilowatts
 * - Torque curve is placeholder (future: actual torque vs RPM data)
 * - Direction is typically CW for standard operations
 */
class Spindle {
public:
    /**
     * @brief Construct spindle definition
     * @param maxRPM Maximum spindle speed (RPM)
     * @param minRPM Minimum spindle speed (RPM, default: 0)
     * @param power Power rating (kW)
     * @param direction Default rotation direction
     */
    Spindle(
        double maxRPM,
        double minRPM = 0.0,
        double power = 5.0,
        SpindleDirection direction = SpindleDirection::Clockwise
    ) : maxRPM_(maxRPM > 0.0 ? maxRPM : 0.0),
        minRPM_(minRPM >= 0.0 ? minRPM : 0.0),
        power_(power >= 0.0 ? power : 0.0),
        direction_(direction) {
        // Ensure min <= max
        if (minRPM_ > maxRPM_) {
            std::swap(minRPM_, maxRPM_);
        }
    }

    /**
     * @brief Get maximum RPM
     */
    double getMaxRPM() const {
        return maxRPM_;
    }

    /**
     * @brief Get minimum RPM
     */
    double getMinRPM() const {
        return minRPM_;
    }

    /**
     * @brief Get RPM range
     */
    double getRPMRange() const {
        return maxRPM_ - minRPM_;
    }

    /**
     * @brief Get power rating
     */
    double getPower() const {
        return power_;
    }

    /**
     * @brief Get default rotation direction
     */
    SpindleDirection getDirection() const {
        return direction_;
    }

    /**
     * @brief Check if RPM is within range
     */
    bool isRPMValid(double rpm) const {
        return rpm >= minRPM_ && rpm <= maxRPM_;
    }

    /**
     * @brief Clamp RPM to valid range
     */
    double clampRPM(double rpm) const {
        return std::max(minRPM_, std::min(maxRPM_, rpm));
    }

    /**
     * @brief Get estimated torque at RPM
     * 
     * Placeholder for torque curve. Returns constant torque estimate
     * based on power. Future: implement actual torque vs RPM curve.
     * 
     * @param rpm Spindle speed
     * @return Estimated torque (Nm)
     */
    double getEstimatedTorque(double rpm) const {
        if (rpm <= 0.0 || !isRPMValid(rpm)) {
            return 0.0;
        }
        // Simplified: Power = Torque * AngularVelocity
        // AngularVelocity (rad/s) = RPM * 2π / 60
        double angularVelocity = rpm * 2.0 * 3.14159265358979323846 / 60.0;
        if (angularVelocity > 0.0) {
            return (power_ * 1000.0) / angularVelocity; // Convert kW to W, then to Nm
        }
        return 0.0;
    }

    /**
     * @brief Check if spindle is valid
     */
    bool isValid() const {
        return maxRPM_ > 0.0 &&
               minRPM_ >= 0.0 &&
               minRPM_ <= maxRPM_ &&
               power_ >= 0.0 &&
               std::isfinite(maxRPM_) &&
               std::isfinite(minRPM_) &&
               std::isfinite(power_);
    }

private:
    double maxRPM_;              ///< Maximum spindle speed (RPM)
    double minRPM_;              ///< Minimum spindle speed (RPM)
    double power_;               ///< Power rating (kW)
    SpindleDirection direction_; ///< Default rotation direction
};

} // namespace cnc
