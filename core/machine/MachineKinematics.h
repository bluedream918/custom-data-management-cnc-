#pragma once

#include "../machine/Axis.h"
#include "../geometry/Transform.h"
#include "../common/Types.h"
#include <memory>
#include <vector>
#include <array>

namespace cnc {

/**
 * @brief Forward kinematics result
 * 
 * Contains tool pose and axis positions after forward kinematics calculation.
 */
struct ForwardKinematicsResult {
    Transform toolPose;              ///< Tool pose in machine coordinates
    std::array<double, 6> axisPositions; ///< Axis positions [X,Y,Z,A,B,C]
    bool valid = false;              ///< Whether the result is valid
};

/**
 * @brief Inverse kinematics result
 * 
 * Contains axis positions and tool pose after inverse kinematics calculation.
 * May have multiple solutions for some machine types.
 */
struct InverseKinematicsResult {
    std::array<double, 6> axisPositions; ///< Axis positions [X,Y,Z,A,B,C]
    Transform toolPose;              ///< Computed tool pose (for verification)
    bool valid = false;              ///< Whether the result is valid
};

/**
 * @brief Abstract base class for machine kinematics
 * 
 * Defines the interface for forward and inverse kinematics calculations.
 * Kinematics are stateless - all axis positions are passed explicitly.
 * 
 * This design allows:
 * - Multiple machine configurations
 * - Deterministic calculations
 * - Easy testing and validation
 * - RL state representation
 * 
 * Industrial control assumptions:
 * - Forward kinematics: axis positions -> tool pose
 * - Inverse kinematics: tool pose -> axis positions
 * - All calculations are deterministic
 * - No dynamic effects (no inertia, no vibration)
 */
class MachineKinematics {
public:
    virtual ~MachineKinematics() = default;

    /**
     * @brief Get axis configuration
     * 
     * Returns which axes are available on this machine.
     */
    virtual AxisConfig getAxisConfig() const = 0;

    /**
     * @brief Get axis limits
     * 
     * Returns travel limits for each axis.
     * 
     * @return Array of axis limits [minX, maxX, minY, maxY, ...]
     */
    virtual std::vector<std::pair<double, double>> getAxisLimits() const = 0;

    /**
     * @brief Forward kinematics: axis positions -> tool pose
     * 
     * Calculates the tool pose (position and orientation) given
     * the current axis positions.
     * 
     * @param axisPositions Current axis positions [X,Y,Z,A,B,C]
     *                      Unused axes should be 0.0
     * @return Forward kinematics result with tool pose
     */
    virtual ForwardKinematicsResult forwardKinematics(
        const std::array<double, 6>& axisPositions) const = 0;

    /**
     * @brief Inverse kinematics: tool pose -> axis positions
     * 
     * Calculates the required axis positions to achieve a target
     * tool pose. May return multiple solutions for some machine types.
     * 
     * @param targetPose Desired tool pose in machine coordinates
     * @return Vector of inverse kinematics solutions (may be empty if unreachable)
     */
    virtual std::vector<InverseKinematicsResult> inverseKinematics(
        const Transform& targetPose) const = 0;

    /**
     * @brief Check if a tool pose is reachable
     * 
     * Determines if the given tool pose can be achieved within
     * axis limits and machine constraints.
     * 
     * @param targetPose Tool pose to check
     * @return True if pose is reachable
     */
    virtual bool isPoseReachable(const Transform& targetPose) const {
        auto solutions = inverseKinematics(targetPose);
        return !solutions.empty() && solutions[0].valid;
    }

    /**
     * @brief Get work envelope
     * 
     * Returns the bounding box of all reachable tool positions.
     * This is a conservative estimate based on axis limits.
     */
    virtual AABB getWorkEnvelope() const = 0;

    /**
     * @brief Create a deep copy of this kinematics object
     */
    virtual std::unique_ptr<MachineKinematics> clone() const = 0;

    /**
     * @brief Get kinematics type identifier
     */
    virtual std::string getType() const = 0;

    /**
     * @brief Check if kinematics is valid
     */
    virtual bool isValid() const = 0;
};

} // namespace cnc
