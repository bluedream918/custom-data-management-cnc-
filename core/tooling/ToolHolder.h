#pragma once

#include "../common/Types.h"
#include <string>

namespace cnc {

/**
 * @brief Tool holder type enumeration
 */
enum class HolderType {
    BT30,       ///< BT30 taper (30mm)
    BT40,       ///< BT40 taper (40mm)
    BT50,       ///< BT50 taper (50mm)
    HSK63,      ///< HSK63 (63mm)
    HSK100,     ///< HSK100 (100mm)
    ER32,       ///< ER32 collet chuck
    ER40,       ///< ER40 collet chuck
    Custom      ///< Custom holder type
};

/**
 * @brief Represents tool holder geometry and specifications
 * 
 * Encapsulates holder physical properties and operational limits.
 * Used for collision detection, tool mounting, and safety checks.
 * 
 * Industrial control assumptions:
 * - Gauge length is fixed (rigid holder)
 * - Max RPM is a safety limit
 * - Collision radius is for interference checking
 */
class ToolHolder {
public:
    /**
     * @brief Construct tool holder
     * @param type Holder type identifier
     * @param gaugeLength Gauge length (spindle face to tool tip reference)
     * @param maxRPM Maximum safe spindle speed (RPM)
     * @param collisionRadius Collision radius for interference checking
     */
    ToolHolder(
        HolderType type,
        double gaugeLength,
        double maxRPM = 24000.0,
        double collisionRadius = 50.0
    ) : type_(type),
        gaugeLength_(gaugeLength > 0.0 ? gaugeLength : 0.0),
        maxRPM_(maxRPM > 0.0 ? maxRPM : 24000.0),
        collisionRadius_(collisionRadius > 0.0 ? collisionRadius : 50.0) {
    }

    /**
     * @brief Get holder type
     */
    HolderType getType() const {
        return type_;
    }

    /**
     * @brief Get holder type name
     */
    std::string getTypeName() const {
        switch (type_) {
            case HolderType::BT30: return "BT30";
            case HolderType::BT40: return "BT40";
            case HolderType::BT50: return "BT50";
            case HolderType::HSK63: return "HSK63";
            case HolderType::HSK100: return "HSK100";
            case HolderType::ER32: return "ER32";
            case HolderType::ER40: return "ER40";
            case HolderType::Custom: return "Custom";
        }
        return "Unknown";
    }

    /**
     * @brief Get gauge length
     * 
     * Distance from spindle face to tool tip reference point.
     */
    double getGaugeLength() const {
        return gaugeLength_;
    }

    /**
     * @brief Get maximum RPM
     * 
     * Maximum safe spindle speed for this holder.
     */
    double getMaxRPM() const {
        return maxRPM_;
    }

    /**
     * @brief Get collision radius
     * 
     * Radius for collision detection and interference checking.
     */
    double getCollisionRadius() const {
        return collisionRadius_;
    }

    /**
     * @brief Check if holder is valid
     */
    bool isValid() const {
        return gaugeLength_ > 0.0 &&
               maxRPM_ > 0.0 &&
               collisionRadius_ > 0.0 &&
               std::isfinite(gaugeLength_) &&
               std::isfinite(maxRPM_) &&
               std::isfinite(collisionRadius_);
    }

    /**
     * @brief Check if holder is compatible with another holder type
     * 
     * Returns true if holders can be interchanged (same taper type).
     */
    bool isCompatibleWith(HolderType otherType) const {
        if (type_ == otherType) {
            return true;
        }
        // BT series are compatible within size constraints
        if ((type_ == HolderType::BT30 || type_ == HolderType::BT40 || type_ == HolderType::BT50) &&
            (otherType == HolderType::BT30 || otherType == HolderType::BT40 || otherType == HolderType::BT50)) {
            return true; // Same taper family
        }
        // HSK series are compatible within size constraints
        if ((type_ == HolderType::HSK63 || type_ == HolderType::HSK100) &&
            (otherType == HolderType::HSK63 || otherType == HolderType::HSK100)) {
            return true; // Same taper family
        }
        return false;
    }

private:
    HolderType type_;            ///< Holder type
    double gaugeLength_;         ///< Gauge length
    double maxRPM_;              ///< Maximum RPM
    double collisionRadius_;      ///< Collision radius
};

} // namespace cnc
