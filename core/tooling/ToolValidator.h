#pragma once

#include "Tool.h"
#include "../toolpath/MotionType.h"
#include <stdexcept>
#include <string>
#include <cmath>

namespace cnc {

/**
 * @brief Validates tool correctness and usability
 * 
 * Performs comprehensive validation of tool geometry, holder compatibility,
 * and operational safety. Throws std::logic_error on validation failures.
 * 
 * Tool-Toolpath Interaction:
 * - Tool geometry must match intended motion types
 * - End mills are suitable for linear and arc motions
 * - Drills are suitable for plunge motions
 * - Ball mills enable 3D contouring
 * - Tool validation ensures toolpaths can be executed safely
 * 
 * Extension for 5-axis & Probing:
 * - Tool geometry validation supports orientation-dependent operations
 * - Holder compatibility checks enable multi-axis tool changes
 * - Future: Add probing tool validation with touch-off requirements
 * - Future: Add tool orientation validation for 5-axis operations
 * 
 * Industrial control assumptions:
 * - Tools must be safe for intended operations
 * - Max RPM must not exceed holder limits
 * - Tool geometry must be consistent
 * - Default parameters must be reasonable
 */
class ToolValidator {
public:
    /**
     * @brief Validate tool
     * 
     * Performs all validation checks on a tool.
     * Throws std::logic_error if validation fails.
     * 
     * @param tool Tool to validate
     * @throws std::logic_error if validation fails
     */
    static void validate(const Tool& tool) {
        validateGeometry(tool);
        validateHolder(tool);
        validateRPM(tool);
        validateParameters(tool);
    }

    /**
     * @brief Validate tool geometry
     * 
     * @param tool Tool to validate
     * @throws std::logic_error if validation fails
     */
    static void validateGeometry(const Tool& tool) {
        const ToolGeometry& geom = tool.getGeometry();
        
        if (!geom.isValid()) {
            throw std::logic_error(
                "Tool '" + tool.getId() + "' has invalid geometry"
            );
        }

        // Check diameter consistency
        if (geom.getDiameter() <= 0.0) {
            throw std::logic_error(
                "Tool '" + tool.getId() + "' has invalid diameter: " +
                std::to_string(geom.getDiameter())
            );
        }

        // Check length consistency
        if (geom.getOverallLength() < geom.getFluteLength()) {
            throw std::logic_error(
                "Tool '" + tool.getId() + "' overall length (" +
                std::to_string(geom.getOverallLength()) + ") is less than flute length (" +
                std::to_string(geom.getFluteLength()) + ")"
            );
        }

        // Check corner radius consistency
        double maxRadius = geom.getRadius();
        if (geom.getCornerRadius() > maxRadius) {
            throw std::logic_error(
                "Tool '" + tool.getId() + "' corner radius (" +
                std::to_string(geom.getCornerRadius()) + ") exceeds tool radius (" +
                std::to_string(maxRadius) + ")"
            );
        }
    }

    /**
     * @brief Validate tool holder
     * 
     * @param tool Tool to validate
     * @throws std::logic_error if validation fails
     */
    static void validateHolder(const Tool& tool) {
        const ToolHolder& holder = tool.getHolder();
        
        if (!holder.isValid()) {
            throw std::logic_error(
                "Tool '" + tool.getId() + "' has invalid holder"
            );
        }

        // Check gauge length
        if (holder.getGaugeLength() <= 0.0) {
            throw std::logic_error(
                "Tool '" + tool.getId() + "' holder has invalid gauge length: " +
                std::to_string(holder.getGaugeLength())
            );
        }
    }

    /**
     * @brief Validate RPM safety
     * 
     * Checks that tool spindle speed does not exceed holder max RPM.
     * 
     * @param tool Tool to validate
     * @throws std::logic_error if validation fails
     */
    static void validateRPM(const Tool& tool) {
        double toolRPM = tool.getDefaultSpindleSpeed();
        double holderMaxRPM = tool.getHolder().getMaxRPM();

        if (toolRPM > holderMaxRPM) {
            throw std::logic_error(
                "Tool '" + tool.getId() + "' default spindle speed (" +
                std::to_string(toolRPM) + " RPM) exceeds holder maximum (" +
                std::to_string(holderMaxRPM) + " RPM)"
            );
        }
    }

    /**
     * @brief Validate tool parameters
     * 
     * @param tool Tool to validate
     * @throws std::logic_error if validation fails
     */
    static void validateParameters(const Tool& tool) {
        if (tool.getId().empty()) {
            throw std::logic_error("Tool has empty ID");
        }

        if (tool.getName().empty()) {
            throw std::logic_error("Tool '" + tool.getId() + "' has empty name");
        }

        if (tool.getDefaultFeedrate() <= 0.0) {
            throw std::logic_error(
                "Tool '" + tool.getId() + "' has invalid default feedrate: " +
                std::to_string(tool.getDefaultFeedrate())
            );
        }

        if (tool.getDefaultSpindleSpeed() <= 0.0) {
            throw std::logic_error(
                "Tool '" + tool.getId() + "' has invalid default spindle speed: " +
                std::to_string(tool.getDefaultSpindleSpeed())
            );
        }
    }

    /**
     * @brief Check if tool is usable for motion type
     * 
     * Determines if a tool is suitable for a given motion type.
     * 
     * @param tool Tool to check
     * @param motionType Motion type to check
     * @return True if tool is suitable
     */
    static bool isUsableForMotion(const Tool& tool, MotionType motionType) {
        ToolingType toolType = tool.getType();

        switch (motionType) {
            case MotionType::Rapid:
                return true; // Any tool can do rapid moves

            case MotionType::Linear:
            case MotionType::ArcCW:
            case MotionType::ArcCCW:
                // Cutting motions require cutting tools
                return toolType == ToolingType::EndMill ||
                       toolType == ToolingType::BallMill ||
                       toolType == ToolingType::FlatMill ||
                       toolType == ToolingType::Chamfer;

            case MotionType::Dwell:
                return true; // Any tool can dwell

            case MotionType::ToolChange:
                return true; // Tool change is independent

            default:
                return false;
        }
    }

    /**
     * @brief Check if tool is valid (non-throwing)
     * 
     * Returns true if tool passes all validation checks.
     * 
     * @param tool Tool to check
     * @return True if valid, false otherwise
     */
    static bool isValid(const Tool& tool) {
        try {
            validate(tool);
            return true;
        } catch (const std::logic_error&) {
            return false;
        }
    }

    /**
     * @brief Validate tool for specific motion type
     * 
     * Checks if tool is suitable and safe for a motion type.
     * 
     * @param tool Tool to validate
     * @param motionType Motion type to check
     * @throws std::logic_error if tool is not suitable
     */
    static void validateForMotion(const Tool& tool, MotionType motionType) {
        validate(tool);

        if (!isUsableForMotion(tool, motionType)) {
            throw std::logic_error(
                "Tool '" + tool.getId() + "' (type: " +
                std::to_string(static_cast<int>(tool.getType())) +
                ") is not suitable for motion type: " +
                std::to_string(static_cast<int>(motionType))
            );
        }
    }
};

} // namespace cnc
