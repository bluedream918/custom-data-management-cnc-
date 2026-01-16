#pragma once

#include "MachineKinematics.h"
#include "../geometry/Transform.h"
#include "../common/Types.h"
#include <memory>
#include <array>

namespace cnc {

/**
 * @brief Standard 3-axis Cartesian CNC machine kinematics
 * 
 * Implements forward and inverse kinematics for a standard 3-axis
 * CNC machine with X, Y, and Z linear axes.
 * 
 * Coordinate system:
 * - X: Horizontal (typically left-right)
 * - Y: Horizontal (typically front-back)
 * - Z: Vertical (typically up-down)
 * - Tool orientation: Always vertical (Z-axis direction)
 * 
 * Industrial control assumptions:
 * - Direct 1:1 mapping between axis positions and tool position
 * - No rotary axes (A, B, C are unused)
 * - Tool orientation is fixed (no tilting)
 * - All calculations are deterministic
 */
class Cartesian3Axis : public MachineKinematics {
public:
    /**
     * @brief Construct 3-axis machine
     * @param xLimits X-axis travel limits [min, max]
     * @param yLimits Y-axis travel limits [min, max]
     * @param zLimits Z-axis travel limits [min, max]
     */
    Cartesian3Axis(
        std::pair<double, double> xLimits = {-1000.0, 1000.0},
        std::pair<double, double> yLimits = {-1000.0, 1000.0},
        std::pair<double, double> zLimits = {-100.0, 100.0}
    ) : xLimits_(xLimits),
        yLimits_(yLimits),
        zLimits_(zLimits) {
    }

    virtual ~Cartesian3Axis() = default;

    AxisConfig getAxisConfig() const override {
        AxisConfig config;
        config.hasX = true;
        config.hasY = true;
        config.hasZ = true;
        config.hasA = false;
        config.hasB = false;
        config.hasC = false;
        return config;
    }

    std::vector<std::pair<double, double>> getAxisLimits() const override {
        return {xLimits_, yLimits_, zLimits_,
                {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}}; // A, B, C unused
    }

    ForwardKinematicsResult forwardKinematics(
        const std::array<double, 6>& axisPositions) const override {
        ForwardKinematicsResult result;
        
        // Extract X, Y, Z positions
        double x = axisPositions[static_cast<int>(Axis::X)];
        double y = axisPositions[static_cast<int>(Axis::Y)];
        double z = axisPositions[static_cast<int>(Axis::Z)];

        // Check limits
        if (x < xLimits_.first || x > xLimits_.second ||
            y < yLimits_.first || y > yLimits_.second ||
            z < zLimits_.first || z > zLimits_.second) {
            result.valid = false;
            return result;
        }

        // Tool position is directly the axis positions
        Vec3 toolPosition(x, y, z);

        // Tool orientation is fixed: pointing down in Z direction
        // (standard CNC convention: tool tip at position, pointing down)
        Quaternion toolRotation = Quaternion::identity(); // No rotation for 3-axis

        result.toolPose = Transform(toolPosition, toolRotation);
        result.axisPositions = axisPositions;
        result.valid = true;

        return result;
    }

    std::vector<InverseKinematicsResult> inverseKinematics(
        const Transform& targetPose) const override {
        std::vector<InverseKinematicsResult> solutions;

        Vec3 targetPosition = targetPose.getPosition();

        // Extract X, Y, Z from target position
        double x = targetPosition.x;
        double y = targetPosition.y;
        double z = targetPosition.z;

        // Check if position is within limits
        if (x < xLimits_.first || x > xLimits_.second ||
            y < yLimits_.first || y > yLimits_.second ||
            z < zLimits_.first || z > zLimits_.second) {
            // Position unreachable
            return solutions;
        }

        // For 3-axis, inverse kinematics is trivial: axis positions = tool position
        InverseKinematicsResult solution;
        solution.axisPositions.fill(0.0);
        solution.axisPositions[static_cast<int>(Axis::X)] = x;
        solution.axisPositions[static_cast<int>(Axis::Y)] = y;
        solution.axisPositions[static_cast<int>(Axis::Z)] = z;
        solution.axisPositions[static_cast<int>(Axis::A)] = 0.0;
        solution.axisPositions[static_cast<int>(Axis::B)] = 0.0;
        solution.axisPositions[static_cast<int>(Axis::C)] = 0.0;

        // Verify by forward kinematics
        auto fkResult = forwardKinematics(solution.axisPositions);
        solution.toolPose = fkResult.toolPose;
        solution.valid = fkResult.valid;

        solutions.push_back(solution);
        return solutions;
    }

    AABB getWorkEnvelope() const override {
        return AABB(
            Vec3(xLimits_.first, yLimits_.first, zLimits_.first),
            Vec3(xLimits_.second, yLimits_.second, zLimits_.second)
        );
    }

    std::unique_ptr<MachineKinematics> clone() const override {
        return std::make_unique<Cartesian3Axis>(xLimits_, yLimits_, zLimits_);
    }

    std::string getType() const override {
        return "Cartesian3Axis";
    }

    bool isValid() const override {
        return xLimits_.first < xLimits_.second &&
               yLimits_.first < yLimits_.second &&
               zLimits_.first < zLimits_.second;
    }

    /**
     * @brief Get X-axis limits
     */
    std::pair<double, double> getXLimits() const {
        return xLimits_;
    }

    /**
     * @brief Get Y-axis limits
     */
    std::pair<double, double> getYLimits() const {
        return yLimits_;
    }

    /**
     * @brief Get Z-axis limits
     */
    std::pair<double, double> getZLimits() const {
        return zLimits_;
    }

private:
    std::pair<double, double> xLimits_;
    std::pair<double, double> yLimits_;
    std::pair<double, double> zLimits_;
};

} // namespace cnc
