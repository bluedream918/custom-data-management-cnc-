#pragma once

#include "Machine.h"
#include "../tooling/Tool.h"
#include <stdexcept>
#include <string>

namespace cnc {

/**
 * @brief Validates machine correctness and compatibility
 * 
 * Performs comprehensive validation of machine configuration including
 * axis consistency, spindle capabilities, tool changer, and tool compatibility.
 * Throws std::logic_error on validation failures.
 * 
 * Machine-Toolpath Integration:
 * - Validated machines ensure toolpaths can be executed
 * - Axis limits determine reachable positions
 * - Spindle RPM range determines valid speeds
 * - Tool changer capacity determines available tools
 * 
 * Machine-Simulation Integration:
 * - Validated machines ensure simulation accuracy
 * - Axis definitions enable kinematic calculations
 * - Spindle capabilities enable material removal simulation
 * - Work envelope enables collision detection
 * 
 * Machine-G-code Integration:
 * - Validated machines ensure correct G-code generation
 * - Axis configuration maps to G-code axes
 * - Spindle commands map to M-codes
 * - Tool changer maps to M6 commands
 * 
 * Future 5-axis Kinematics Extension:
 * - Add rotary axis validation
 * - Validate rotary axis limits (typically +/-360 degrees or continuous)
 * - Validate tool orientation reachability
 * - Validate 5-axis toolpath compatibility
 * 
 * Industrial control assumptions:
 * - Machines must be safe for operation
 * - All parameters must be consistent
 * - Tool compatibility must be verified
 */
class MachineValidator {
public:
    /**
     * @brief Validate machine
     * 
     * Performs all validation checks on a machine.
     * Throws std::logic_error if validation fails.
     * 
     * @param machine Machine to validate
     * @throws std::logic_error if validation fails
     */
    static void validate(const Machine& machine) {
        validateBasic(machine);
        validateAxes(machine);
        validateSpindle(machine);
        validateToolChanger(machine);
        validateWorkEnvelope(machine);
    }

    /**
     * @brief Validate basic machine properties
     * 
     * @param machine Machine to validate
     * @throws std::logic_error if validation fails
     */
    static void validateBasic(const Machine& machine) {
        if (machine.getId().empty()) {
            throw std::logic_error("Machine has empty ID");
        }

        if (machine.getName().empty()) {
            throw std::logic_error("Machine '" + machine.getId() + "' has empty name");
        }

        if (machine.getAxisCount() == 0) {
            throw std::logic_error("Machine '" + machine.getId() + "' has no axes");
        }
    }

    /**
     * @brief Validate axis configuration
     * 
     * @param machine Machine to validate
     * @throws std::logic_error if validation fails
     */
    static void validateAxes(const Machine& machine) {
        // Check that all axes are valid
        for (const auto& pair : machine.getAxes()) {
            const AxisDefinition& axis = pair.second;
            if (!axis.isValid()) {
                throw std::logic_error(
                    "Machine '" + machine.getId() + "' has invalid axis: " +
                    std::to_string(static_cast<int>(axis.getType()))
                );
            }
        }

        // Check axis count matches machine type
        int linearCount = 0;
        int rotaryCount = 0;

        for (const auto& pair : machine.getAxes()) {
            if (isLinearAxis(pair.first)) {
                linearCount++;
            } else if (isRotaryAxis(pair.first)) {
                rotaryCount++;
            }
        }

        // 3-axis machines should have X, Y, Z
        if (linearCount == 3 && rotaryCount == 0) {
            if (!machine.hasAxis(AxisType::X) ||
                !machine.hasAxis(AxisType::Y) ||
                !machine.hasAxis(AxisType::Z)) {
                throw std::logic_error(
                    "Machine '" + machine.getId() + "' is 3-axis but missing X, Y, or Z axis"
                );
            }
        }

        // 5-axis machines should have X, Y, Z and at least 2 rotary axes
        if (linearCount == 3 && rotaryCount >= 2) {
            if (!machine.hasAxis(AxisType::X) ||
                !machine.hasAxis(AxisType::Y) ||
                !machine.hasAxis(AxisType::Z)) {
                throw std::logic_error(
                    "Machine '" + machine.getId() + "' is 5-axis but missing X, Y, or Z axis"
                );
            }
        }
    }

    /**
     * @brief Validate spindle configuration
     * 
     * @param machine Machine to validate
     * @throws std::logic_error if validation fails
     */
    static void validateSpindle(const Machine& machine) {
        const Spindle& spindle = machine.getSpindle();

        if (!spindle.isValid()) {
            throw std::logic_error(
                "Machine '" + machine.getId() + "' has invalid spindle"
            );
        }

        if (spindle.getMaxRPM() <= 0.0) {
            throw std::logic_error(
                "Machine '" + machine.getId() + "' spindle has invalid max RPM: " +
                std::to_string(spindle.getMaxRPM())
            );
        }

        if (spindle.getMinRPM() > spindle.getMaxRPM()) {
            throw std::logic_error(
                "Machine '" + machine.getId() + "' spindle min RPM (" +
                std::to_string(spindle.getMinRPM()) + ") exceeds max RPM (" +
                std::to_string(spindle.getMaxRPM()) + ")"
            );
        }
    }

    /**
     * @brief Validate tool changer configuration
     * 
     * @param machine Machine to validate
     * @throws std::logic_error if validation fails
     */
    static void validateToolChanger(const Machine& machine) {
        const ToolChanger& changer = machine.getToolChanger();

        if (!changer.isValid()) {
            throw std::logic_error(
                "Machine '" + machine.getId() + "' has invalid tool changer"
            );
        }

        // Tool changer is optional (maxToolSlots = 0 means no changer)
        // But if present, it must be valid
        if (changer.isPresent() && changer.getMaxToolSlots() <= 0) {
            throw std::logic_error(
                "Machine '" + machine.getId() + "' tool changer has invalid capacity: " +
                std::to_string(changer.getMaxToolSlots())
            );
        }
    }

    /**
     * @brief Validate work envelope
     * 
     * @param machine Machine to validate
     * @throws std::logic_error if validation fails
     */
    static void validateWorkEnvelope(const Machine& machine) {
        const AABB& envelope = machine.getWorkEnvelope();

        if (!envelope.isValid()) {
            throw std::logic_error(
                "Machine '" + machine.getId() + "' has invalid work envelope"
            );
        }

        // Check work envelope matches axis limits (if possible)
        const AxisDefinition* xAxis = machine.getAxis(AxisType::X);
        const AxisDefinition* yAxis = machine.getAxis(AxisType::Y);
        const AxisDefinition* zAxis = machine.getAxis(AxisType::Z);

        if (xAxis && yAxis && zAxis) {
            // Check envelope bounds are within axis limits
            if (envelope.min.x < xAxis->getMinPosition() ||
                envelope.max.x > xAxis->getMaxPosition()) {
                throw std::logic_error(
                    "Machine '" + machine.getId() + "' work envelope X bounds exceed axis limits"
                );
            }

            if (envelope.min.y < yAxis->getMinPosition() ||
                envelope.max.y > yAxis->getMaxPosition()) {
                throw std::logic_error(
                    "Machine '" + machine.getId() + "' work envelope Y bounds exceed axis limits"
                );
            }

            if (envelope.min.z < zAxis->getMinPosition() ||
                envelope.max.z > zAxis->getMaxPosition()) {
                throw std::logic_error(
                    "Machine '" + machine.getId() + "' work envelope Z bounds exceed axis limits"
                );
            }
        }
    }

    /**
     * @brief Validate tool compatibility with machine
     * 
     * Checks if a tool can be used on the machine.
     * 
     * @param machine Machine to check
     * @param tool Tool to check
     * @throws std::logic_error if tool is not compatible
     */
    static void validateToolCompatibility(const Machine& machine, const Tool& tool) {
        // Check tool type is supported
        if (!machine.supportsToolType(tool.getType())) {
            throw std::logic_error(
                "Tool '" + tool.getId() + "' type is not supported by machine '" +
                machine.getId() + "'"
            );
        }

        // Check tool holder is supported by tool changer
        const ToolChanger& changer = machine.getToolChanger();
        if (changer.isPresent()) {
            if (!changer.supportsHolder(tool.getHolder().getType())) {
                throw std::logic_error(
                    "Tool '" + tool.getId() + "' holder type is not supported by machine '" +
                    machine.getId() + "' tool changer"
                );
            }
        }

        // Check tool RPM does not exceed spindle max
        const Spindle& spindle = machine.getSpindle();
        if (tool.getDefaultSpindleSpeed() > spindle.getMaxRPM()) {
            throw std::logic_error(
                "Tool '" + tool.getId() + "' default spindle speed (" +
                std::to_string(tool.getDefaultSpindleSpeed()) + " RPM) exceeds machine '" +
                machine.getId() + "' spindle maximum (" +
                std::to_string(spindle.getMaxRPM()) + " RPM)"
            );
        }

        // Check tool RPM is within spindle range
        if (!spindle.isRPMValid(tool.getDefaultSpindleSpeed())) {
            throw std::logic_error(
                "Tool '" + tool.getId() + "' default spindle speed (" +
                std::to_string(tool.getDefaultSpindleSpeed()) + " RPM) is outside machine '" +
                machine.getId() + "' spindle range [" +
                std::to_string(spindle.getMinRPM()) + ", " +
                std::to_string(spindle.getMaxRPM()) + "]"
            );
        }
    }

    /**
     * @brief Check if machine is valid (non-throwing)
     * 
     * @param machine Machine to check
     * @return True if valid, false otherwise
     */
    static bool isValid(const Machine& machine) {
        try {
            validate(machine);
            return true;
        } catch (const std::logic_error&) {
            return false;
        }
    }

    /**
     * @brief Check if tool is compatible with machine (non-throwing)
     * 
     * @param machine Machine to check
     * @param tool Tool to check
     * @return True if compatible, false otherwise
     */
    static bool isToolCompatible(const Machine& machine, const Tool& tool) {
        try {
            validateToolCompatibility(machine, tool);
            return true;
        } catch (const std::logic_error&) {
            return false;
        }
    }
};

} // namespace cnc
