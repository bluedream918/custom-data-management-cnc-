#pragma once

#include "Types.h"
#include <string>

namespace cnc {

/**
 * @brief Centralized unit handling for toolpath system
 * 
 * Provides unit metadata and type safety for toolpath operations.
 * Does NOT perform automatic conversion - units are stored explicitly
 * and conversion is performed explicitly when needed.
 * 
 * Unit conventions:
 * - Linear units: Millimeters or Inches
 * - Feedrate: Units per minute (mm/min or in/min)
 * - Spindle speed: Revolutions per minute (RPM)
 * - Time: Seconds
 */
class ToolpathUnits {
public:
    /**
     * @brief Construct with linear units
     * @param linearUnit Linear measurement unit (mm or inches)
     */
    explicit ToolpathUnits(Unit linearUnit = Unit::Millimeter)
        : linearUnit_(linearUnit) {
    }

    /**
     * @brief Get linear unit
     */
    Unit getLinearUnit() const {
        return linearUnit_;
    }

    /**
     * @brief Get feedrate unit string
     */
    std::string getFeedrateUnit() const {
        return linearUnit_ == Unit::Millimeter ? "mm/min" : "in/min";
    }

    /**
     * @brief Get spindle speed unit string
     */
    std::string getSpindleSpeedUnit() const {
        return "RPM";
    }

    /**
     * @brief Check if using metric units
     */
    bool isMetric() const {
        return linearUnit_ == Unit::Millimeter;
    }

    /**
     * @brief Check if using imperial units
     */
    bool isImperial() const {
        return linearUnit_ == Unit::Inch;
    }

    /**
     * @brief Get unit name string
     */
    std::string getUnitName() const {
        return linearUnit_ == Unit::Millimeter ? "mm" : "in";
    }

private:
    Unit linearUnit_;  ///< Linear measurement unit
};

} // namespace cnc
