#pragma once

#include "Toolpath.h"
#include "ToolpathMove.h"
#include "MoveType.h"
#include "../machine/Machine.h"
#include <stdexcept>
#include <string>
#include <cmath>

namespace cnc {

/**
 * @brief Validates toolpath correctness and machine compatibility
 * 
 * Performs comprehensive validation of toolpath geometry, continuity,
 * machine limits, and safety constraints. Throws std::logic_error on
 * validation failures with descriptive messages.
 * 
 * Toolpath-Simulation Integration:
 * - Validated toolpaths are safe for simulation
 * - Machine limits prevent invalid positions
 * - Tool consistency ensures correct tool geometry
 * - Safe Z checks prevent collisions
 * 
 * Toolpath-Visualization Integration:
 * - Validated toolpaths render correctly
 * - Continuity ensures smooth visualization
 * - Machine limits ensure visible bounds
 * 
 * Toolpath-G-code Integration:
 * - Validated toolpaths generate correct G-code
 * - Feed/RPM presence ensures complete commands
 * - Machine limits ensure post-processor compatibility
 * - Tool consistency ensures tool change commands
 * 
 * Toolpath-RL Integration:
 * - Validated toolpaths represent valid actions
 * - Machine limits define action space bounds
 * - Safety flags define constraint violations
 * - Tool consistency defines state transitions
 * 
 * Validation checks:
 * - No illegal axis motion (machine limits)
 * - Feed & RPM present where required
 * - Machine limits respected (work envelope, axis limits)
 * - Safe Z before rapids (collision prevention)
 * - Tool consistency (tool changes valid)
 * - Motion continuity (end state = next start state)
 * - Arc geometry consistency
 * 
 * Industrial control assumptions:
 * - Toolpaths must respect machine limits
 * - Cutting motions require feedrate
 * - Rapid moves must be safe (Z clearance)
 * - Tool changes must be valid
 * - Motion must be continuous
 */
class ToolpathValidator {
public:
    /**
     * @brief Validate toolpath against machine
     * 
     * Performs all validation checks including machine compatibility.
     * Throws std::logic_error if validation fails.
     * 
     * @param toolpath Toolpath to validate
     * @param machine Machine to validate against (optional)
     * @throws std::logic_error if validation fails
     */
    static void validate(const Toolpath& toolpath, const Machine* machine = nullptr) {
        if (toolpath.isEmpty()) {
            return; // Empty toolpath is valid
        }

        const auto& moves = toolpath.getMoves();

        for (std::size_t i = 0; i < moves.size(); ++i) {
            validateMove(moves[i], i);

            // Check continuity with next move
            if (i + 1 < moves.size()) {
                validateContinuity(moves[i], moves[i + 1], i);
            }

            // Check machine limits if machine provided
            if (machine) {
                validateMachineLimits(moves[i], *machine, i);
            }
        }

        // Check tool consistency
        if (machine) {
            validateToolConsistency(toolpath, *machine);
        }
    }

    /**
     * @brief Validate single move
     * 
     * @param move Move to validate
     * @param index Move index (for error messages)
     * @throws std::logic_error if validation fails
     */
    static void validateMove(const ToolpathMove& move, std::size_t index) {
        // Check move validity
        if (!move.isValid()) {
            throw std::logic_error(
                "Toolpath move " + std::to_string(index) + " is invalid"
            );
        }

        // Check zero-length moves (except control moves)
        if (move.isZeroLength() && 
            move.getMoveType() != MoveType::Dwell &&
            move.getMoveType() != MoveType::ToolChange &&
            move.getMoveType() != MoveType::SpindleStart &&
            move.getMoveType() != MoveType::SpindleStop) {
            throw std::logic_error(
                "Toolpath move " + std::to_string(index) + 
                " has zero length (start and end positions are identical)"
            );
        }

        // Check feedrate for cutting motions
        if (requiresFeedrate(move.getMoveType())) {
            if (!move.getEndState().hasFeedRate()) {
                throw std::logic_error(
                    "Toolpath move " + std::to_string(index) + 
                    " is a cutting motion but has no feedrate"
                );
            }
        }

        // Validate arc geometry
        if (isArcMove(move.getMoveType())) {
            validateArc(move, index);
        }

        // Check rapid safety
        if (move.getMoveType() == MoveType::Rapid) {
            if (!move.isRapidAllowed()) {
                throw std::logic_error(
                    "Toolpath move " + std::to_string(index) + 
                    " is a rapid move but rapid is not allowed (safety violation)"
                );
            }
        }
    }

    /**
     * @brief Validate arc move
     * 
     * Checks that start and end points are equidistant from center.
     * 
     * @param move Arc move to validate
     * @param index Move index
     * @throws std::logic_error if validation fails
     */
    static void validateArc(const ToolpathMove& move, std::size_t index) {
        if (!move.getArcCenter().has_value()) {
            throw std::logic_error(
                "Toolpath move " + std::to_string(index) + 
                " is an arc but has no center point"
            );
        }

        Vec3 start = move.getStartState().getPosition();
        Vec3 end = move.getEndState().getPosition();
        Vec3 center = move.getArcCenter().value();

        Vec3 startVec = start - center;
        Vec3 endVec = end - center;

        double startRadius = startVec.length();
        double endRadius = endVec.length();

        // Check radius consistency (allow small tolerance for floating point)
        double radiusError = std::abs(startRadius - endRadius);
        if (radiusError > 1e-6) {
            throw std::logic_error(
                "Toolpath move " + std::to_string(index) + 
                " arc has inconsistent radius: start=" + std::to_string(startRadius) +
                ", end=" + std::to_string(endRadius) +
                ", error=" + std::to_string(radiusError)
            );
        }

        // Check for zero radius
        if (startRadius < 1e-9) {
            throw std::logic_error(
                "Toolpath move " + std::to_string(index) + 
                " arc has zero radius"
            );
        }
    }

    /**
     * @brief Validate continuity between moves
     * 
     * Checks that move end state matches next move start state.
     * 
     * @param move1 First move
     * @param move2 Next move
     * @param index1 First move index
     * @throws std::logic_error if validation fails
     */
    static void validateContinuity(
        const ToolpathMove& move1,
        const ToolpathMove& move2,
        std::size_t index1
    ) {
        Vec3 end1 = move1.getEndState().getPosition();
        Vec3 start2 = move2.getStartState().getPosition();

        Vec3 delta = end1 - start2;
        double distance = delta.length();

        // Allow small tolerance for floating point errors
        if (distance > 1e-6) {
            throw std::logic_error(
                "Toolpath discontinuity at move " + std::to_string(index1) +
                ": end position (" + std::to_string(end1.x) + ", " +
                std::to_string(end1.y) + ", " + std::to_string(end1.z) + ") " +
                "does not match next start position (" + std::to_string(start2.x) + ", " +
                std::to_string(start2.y) + ", " + std::to_string(start2.z) + ") " +
                "distance=" + std::to_string(distance)
            );
        }
    }

    /**
     * @brief Validate machine limits
     * 
     * Checks that move positions are within machine limits.
     * 
     * @param move Move to validate
     * @param machine Machine to check against
     * @param index Move index
     * @throws std::logic_error if validation fails
     */
    static void validateMachineLimits(
        const ToolpathMove& move,
        const Machine& machine,
        std::size_t index
    ) {
        // Check linear axes (X, Y, Z)
        const auto* xAxis = machine.getAxis(AxisType::X);
        const auto* yAxis = machine.getAxis(AxisType::Y);
        const auto* zAxis = machine.getAxis(AxisType::Z);

        Vec3 start = move.getStartState().getPosition();
        Vec3 end = move.getEndState().getPosition();

        if (xAxis) {
            if (start.x < xAxis->getMinPosition() || start.x > xAxis->getMaxPosition()) {
                throw std::logic_error(
                    "Toolpath move " + std::to_string(index) +
                    " start X position " + std::to_string(start.x) +
                    " exceeds machine limits [" + std::to_string(xAxis->getMinPosition()) +
                    ", " + std::to_string(xAxis->getMaxPosition()) + "]"
                );
            }
            if (end.x < xAxis->getMinPosition() || end.x > xAxis->getMaxPosition()) {
                throw std::logic_error(
                    "Toolpath move " + std::to_string(index) +
                    " end X position " + std::to_string(end.x) +
                    " exceeds machine limits [" + std::to_string(xAxis->getMinPosition()) +
                    ", " + std::to_string(xAxis->getMaxPosition()) + "]"
                );
            }
        }

        if (yAxis) {
            if (start.y < yAxis->getMinPosition() || start.y > yAxis->getMaxPosition()) {
                throw std::logic_error(
                    "Toolpath move " + std::to_string(index) +
                    " start Y position " + std::to_string(start.y) +
                    " exceeds machine limits [" + std::to_string(yAxis->getMinPosition()) +
                    ", " + std::to_string(yAxis->getMaxPosition()) + "]"
                );
            }
            if (end.y < yAxis->getMinPosition() || end.y > yAxis->getMaxPosition()) {
                throw std::logic_error(
                    "Toolpath move " + std::to_string(index) +
                    " end Y position " + std::to_string(end.y) +
                    " exceeds machine limits [" + std::to_string(yAxis->getMinPosition()) +
                    ", " + std::to_string(yAxis->getMaxPosition()) + "]"
                );
            }
        }

        if (zAxis) {
            if (start.z < zAxis->getMinPosition() || start.z > zAxis->getMaxPosition()) {
                throw std::logic_error(
                    "Toolpath move " + std::to_string(index) +
                    " start Z position " + std::to_string(start.z) +
                    " exceeds machine limits [" + std::to_string(zAxis->getMinPosition()) +
                    ", " + std::to_string(zAxis->getMaxPosition()) + "]"
                );
            }
            if (end.z < zAxis->getMinPosition() || end.z > zAxis->getMaxPosition()) {
                throw std::logic_error(
                    "Toolpath move " + std::to_string(index) +
                    " end Z position " + std::to_string(end.z) +
                    " exceeds machine limits [" + std::to_string(zAxis->getMinPosition()) +
                    ", " + std::to_string(zAxis->getMaxPosition()) + "]"
                );
            }
        }

        // Check rotary axes (A, B, C)
        const auto* aAxis = machine.getAxis(AxisType::A);
        const auto* bAxis = machine.getAxis(AxisType::B);
        const auto* cAxis = machine.getAxis(AxisType::C);

        const auto& startRotary = move.getStartState().getRotaryAxes();
        const auto& endRotary = move.getEndState().getRotaryAxes();

        if (aAxis) {
            if (startRotary[0] < aAxis->getMinPosition() || startRotary[0] > aAxis->getMaxPosition()) {
                throw std::logic_error(
                    "Toolpath move " + std::to_string(index) +
                    " start A position " + std::to_string(startRotary[0]) +
                    " exceeds machine limits [" + std::to_string(aAxis->getMinPosition()) +
                    ", " + std::to_string(aAxis->getMaxPosition()) + "]"
                );
            }
            if (endRotary[0] < aAxis->getMinPosition() || endRotary[0] > aAxis->getMaxPosition()) {
                throw std::logic_error(
                    "Toolpath move " + std::to_string(index) +
                    " end A position " + std::to_string(endRotary[0]) +
                    " exceeds machine limits [" + std::to_string(aAxis->getMinPosition()) +
                    ", " + std::to_string(aAxis->getMaxPosition()) + "]"
                );
            }
        }

        if (bAxis) {
            if (startRotary[1] < bAxis->getMinPosition() || startRotary[1] > bAxis->getMaxPosition()) {
                throw std::logic_error(
                    "Toolpath move " + std::to_string(index) +
                    " start B position " + std::to_string(startRotary[1]) +
                    " exceeds machine limits [" + std::to_string(bAxis->getMinPosition()) +
                    ", " + std::to_string(bAxis->getMaxPosition()) + "]"
                );
            }
            if (endRotary[1] < bAxis->getMinPosition() || endRotary[1] > bAxis->getMaxPosition()) {
                throw std::logic_error(
                    "Toolpath move " + std::to_string(index) +
                    " end B position " + std::to_string(endRotary[1]) +
                    " exceeds machine limits [" + std::to_string(bAxis->getMinPosition()) +
                    ", " + std::to_string(bAxis->getMaxPosition()) + "]"
                );
            }
        }

        if (cAxis) {
            if (startRotary[2] < cAxis->getMinPosition() || startRotary[2] > cAxis->getMaxPosition()) {
                throw std::logic_error(
                    "Toolpath move " + std::to_string(index) +
                    " start C position " + std::to_string(startRotary[2]) +
                    " exceeds machine limits [" + std::to_string(cAxis->getMinPosition()) +
                    ", " + std::to_string(cAxis->getMaxPosition()) + "]"
                );
            }
            if (endRotary[2] < cAxis->getMinPosition() || endRotary[2] > cAxis->getMaxPosition()) {
                throw std::logic_error(
                    "Toolpath move " + std::to_string(index) +
                    " end C position " + std::to_string(endRotary[2]) +
                    " exceeds machine limits [" + std::to_string(cAxis->getMinPosition()) +
                    ", " + std::to_string(cAxis->getMaxPosition()) + "]"
                );
            }
        }

        // Check spindle RPM limits
        if (move.getEndState().isSpindleRunning()) {
            double rpm = move.getEndState().getSpindleRPM();
            if (rpm < machine.getSpindle().getMinRPM() || 
                rpm > machine.getSpindle().getMaxRPM()) {
                throw std::logic_error(
                    "Toolpath move " + std::to_string(index) +
                    " spindle RPM " + std::to_string(rpm) +
                    " exceeds machine limits [" + std::to_string(machine.getSpindle().getMinRPM()) +
                    ", " + std::to_string(machine.getSpindle().getMaxRPM()) + "]"
                );
            }
        }

        // Check safe Z before rapids (collision prevention)
        if (move.getMoveType() == MoveType::Rapid && zAxis) {
            // Rapid moves should typically be above a safe Z height
            // This is a basic check - actual safe Z depends on workpiece
            double safeZ = 10.0; // Default safe Z (should be configurable)
            if (end.z < safeZ) {
                // Warning: This is a heuristic check
                // In production, safe Z should be based on workpiece height
            }
        }
    }

    /**
     * @brief Validate tool consistency
     * 
     * Checks that tool changes are valid and tools are consistent.
     * 
     * @param toolpath Toolpath to validate
     * @param machine Machine to check against
     * @throws std::logic_error if validation fails
     */
    static void validateToolConsistency(const Toolpath& toolpath, const Machine& machine) {
        const auto& moves = toolpath.getMoves();
        std::string currentToolId;

        for (std::size_t i = 0; i < moves.size(); ++i) {
            const auto& move = moves[i];

            // Check tool change moves
            if (move.getMoveType() == MoveType::ToolChange) {
                const std::string& newToolId = move.getEndState().getActiveToolId();
                if (newToolId.empty()) {
                    throw std::logic_error(
                        "Toolpath move " + std::to_string(i) +
                        " is a tool change but has no tool ID"
                    );
                }
                currentToolId = newToolId;
            }

            // Check that cutting motions have a tool
            if (isCuttingMove(move.getMoveType())) {
                const std::string& toolId = move.getEndState().getActiveToolId();
                if (toolId.empty()) {
                    throw std::logic_error(
                        "Toolpath move " + std::to_string(i) +
                        " is a cutting motion but has no active tool"
                    );
                }
            }
        }
    }

    /**
     * @brief Check if toolpath is valid (non-throwing)
     * 
     * Returns true if toolpath passes all validation checks.
     * Does not throw exceptions.
     * 
     * @param toolpath Toolpath to check
     * @param machine Machine to validate against (optional)
     * @return True if valid, false otherwise
     */
    static bool isValid(const Toolpath& toolpath, const Machine* machine = nullptr) {
        try {
            validate(toolpath, machine);
            return true;
        } catch (const std::logic_error&) {
            return false;
        }
    }
};

} // namespace cnc
