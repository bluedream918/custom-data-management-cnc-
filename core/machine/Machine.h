#pragma once

#include "AxisType.h"
#include "Axis.h"
#include "Spindle.h"
#include "ToolChanger.h"
#include "../tooling/ToolType.h"
#include "../common/Types.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <cmath>

namespace cnc {

/**
 * @brief Full CNC machine definition
 * 
 * Represents a complete CNC machine configuration including axes,
 * spindle, tool changer, and capabilities. This is an immutable
 * machine definition used for toolpath planning, simulation, and
 * G-code generation.
 * 
 * Machine-Toolpath Integration:
 * - Machine axes determine available motion types
 * - Spindle capabilities determine cutting parameters
 * - Tool changer determines tool change operations
 * - Work envelope determines reachable positions
 * 
 * Machine-Simulation Integration:
 * - Axis limits determine valid positions
 * - Spindle RPM range determines valid speeds
 * - Tool changer determines tool availability
 * - Machine kinematics determine tool tip pose
 * 
 * Machine-G-code Integration:
 * - Machine axes map to G-code axes (X, Y, Z, A, B, C)
 * - Spindle commands map to M3/M4/M5
 * - Tool changer maps to M6
 * - Work envelope determines safe operating zone
 * 
 * Future 5-axis Kinematics Extension:
 * - Add rotary axis kinematics (A, B, C)
 * - Extend work envelope to include rotary limits
 * - Add tool orientation support
 * - Extend validation for 5-axis operations
 * 
 * Industrial control assumptions:
 * - Machine is immutable after construction
 * - All parameters are deterministic
 * - Thread-safe read-only access
 * - Comparable by machine ID
 */
class Machine {
public:
    /**
     * @brief Construct machine definition
     * @param id Unique machine identifier
     * @param name Machine display name
     * @param axes Map of axis definitions (AxisType -> Axis)
     * @param spindle Spindle definition
     * @param toolChanger Tool changer definition
     * @param workEnvelope Work envelope (XYZ bounds)
     * @param supportedToolTypes Vector of supported tool types
     */
    Machine(
        std::string id,
        std::string name,
        std::unordered_map<AxisType, AxisDefinition> axes,
        Spindle spindle,
        ToolChanger toolChanger,
        AABB workEnvelope,
        const std::vector<ToolingType>& supportedToolTypes = {}
    ) : id_(std::move(id)),
        name_(std::move(name)),
        axes_(std::move(axes)),
        spindle_(spindle),
        toolChanger_(toolChanger),
        workEnvelope_(workEnvelope),
        supportedToolTypes_(supportedToolTypes) {
    }

    /**
     * @brief Get machine identifier
     */
    const std::string& getId() const {
        return id_;
    }

    /**
     * @brief Get machine display name
     */
    const std::string& getName() const {
        return name_;
    }

    /**
     * @brief Get axis definition
     * 
     * @param type Axis type
     * @return Pointer to axis definition, or nullptr if not present
     */
    const AxisDefinition* getAxis(AxisType type) const {
        auto it = axes_.find(type);
        if (it != axes_.end()) {
            return &it->second;
        }
        return nullptr;
    }

    /**
     * @brief Get all axes
     */
    const std::unordered_map<AxisType, AxisDefinition>& getAxes() const {
        return axes_;
    }

    /**
     * @brief Check if axis is present
     */
    bool hasAxis(AxisType type) const {
        return axes_.find(type) != axes_.end();
    }

    /**
     * @brief Get number of axes
     */
    std::size_t getAxisCount() const {
        return axes_.size();
    }

    /**
     * @brief Get spindle definition
     */
    const Spindle& getSpindle() const {
        return spindle_;
    }

    /**
     * @brief Get tool changer definition
     */
    const ToolChanger& getToolChanger() const {
        return toolChanger_;
    }

    /**
     * @brief Get work envelope
     */
    const AABB& getWorkEnvelope() const {
        return workEnvelope_;
    }

    /**
     * @brief Get supported tool types
     */
    const std::vector<ToolingType>& getSupportedToolTypes() const {
        return supportedToolTypes_;
    }

    /**
     * @brief Check if tool type is supported
     */
    bool supportsToolType(ToolingType toolType) const {
        if (supportedToolTypes_.empty()) {
            return true; // Empty list means all tools supported
        }
        return std::find(supportedToolTypes_.begin(), supportedToolTypes_.end(), toolType) != supportedToolTypes_.end();
    }

    /**
     * @brief Get machine type description
     * 
     * Returns a string describing the machine type based on axis count.
     */
    std::string getMachineType() const {
        int linearCount = 0;
        int rotaryCount = 0;

        for (const auto& pair : axes_) {
            if (isLinearAxis(pair.first)) {
                linearCount++;
            } else if (isRotaryAxis(pair.first)) {
                rotaryCount++;
            }
        }

        if (linearCount == 3 && rotaryCount == 0) {
            return "3-Axis";
        } else if (linearCount == 3 && rotaryCount == 1) {
            return "4-Axis";
        } else if (linearCount == 3 && rotaryCount == 2) {
            return "5-Axis";
        } else if (linearCount == 2 && rotaryCount == 0) {
            return "2-Axis";
        } else {
            return "Custom";
        }
    }

    /**
     * @brief Check if machine is valid
     */
    bool isValid() const {
        return !id_.empty() &&
               !name_.empty() &&
               !axes_.empty() &&
               spindle_.isValid() &&
               toolChanger_.isValid() &&
               workEnvelope_.isValid();
    }

    /**
     * @brief Equality comparison (by ID)
     */
    bool operator==(const Machine& other) const {
        return id_ == other.id_;
    }

    /**
     * @brief Inequality comparison (by ID)
     */
    bool operator!=(const Machine& other) const {
        return id_ != other.id_;
    }

    /**
     * @brief Less-than comparison (by ID, for sorting)
     */
    bool operator<(const Machine& other) const {
        return id_ < other.id_;
    }

private:
    std::string id_;                                    ///< Unique machine identifier
    std::string name_;                                  ///< Machine display name
    std::unordered_map<AxisType, AxisDefinition> axes_; ///< Axis definitions
    Spindle spindle_;                                    ///< Spindle definition
    ToolChanger toolChanger_;                           ///< Tool changer definition
    AABB workEnvelope_;                                 ///< Work envelope (XYZ bounds)
    std::vector<ToolingType> supportedToolTypes_;       ///< Supported tool types
};

} // namespace cnc
