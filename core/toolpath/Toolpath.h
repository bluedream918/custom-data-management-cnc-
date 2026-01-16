#pragma once

#include "ToolpathMove.h"
#include "../common/Types.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <numeric>

namespace cnc {

/**
 * @brief Container for ordered toolpath moves
 * 
 * Represents a complete toolpath as a sequence of atomic CNC moves.
 * Provides append-only construction, read-only iteration, and analysis
 * capabilities including bounding box, total length, and time estimation.
 * 
 * Toolpath-Simulation Integration:
 * - Moves represent simulation steps
 * - State transitions enable material removal
 * - Tool usage enables tool wear modeling
 * - Time estimation enables simulation timing
 * 
 * Toolpath-Visualization Integration:
 * - Moves provide geometry for rendering
 * - Bounding box enables viewport calculation
 * - Tool changes enable visualization updates
 * - State transitions enable animation
 * 
 * Toolpath-G-code Integration:
 * - Moves map to G-code commands
 * - Machine ID determines post-processor
 * - Tool usage determines tool change commands
 * - State transitions determine modal G-code
 * 
 * Toolpath-RL Integration:
 * - Moves represent action sequence
 * - State transitions represent state space
 * - Tool usage represents action parameters
 * - Time estimation represents reward signal
 * 
 * Industrial control assumptions:
 * - Moves are ordered sequentially (no branching)
 * - Append-only construction (immutable after append)
 * - Thread-safe for read-only access
 * - Deterministic iteration
 * - Machine-agnostic (no machine-specific logic)
 */
class Toolpath {
public:
    /**
     * @brief Construct empty toolpath
     * @param id Unique toolpath identifier
     * @param machineId Machine identifier this toolpath targets
     */
    Toolpath(
        const std::string& id = "",
        const std::string& machineId = ""
    ) : id_(id),
        machineId_(machineId) {
    }

    /**
     * @brief Get toolpath identifier
     */
    const std::string& getId() const {
        return id_;
    }

    /**
     * @brief Get machine identifier
     */
    const std::string& getMachineId() const {
        return machineId_;
    }

    /**
     * @brief Append move to toolpath
     * 
     * Adds a move to the end of the toolpath.
     * This is the only way to modify the toolpath after construction.
     * 
     * @param move Move to append
     */
    void appendMove(const ToolpathMove& move) {
        moves_.push_back(move);
        // Update tool usage
        if (move.getEndState().hasActiveTool()) {
            const std::string& toolId = move.getEndState().getActiveToolId();
            toolUsage_[toolId]++;
        }
    }

    /**
     * @brief Get number of moves
     */
    std::size_t getMoveCount() const {
        return moves_.size();
    }

    /**
     * @brief Check if toolpath is empty
     */
    bool isEmpty() const {
        return moves_.empty();
    }

    /**
     * @brief Get move by index
     */
    const ToolpathMove& getMove(std::size_t index) const {
        return moves_[index];
    }

    /**
     * @brief Get all moves (read-only)
     */
    const std::vector<ToolpathMove>& getMoves() const {
        return moves_;
    }

    /**
     * @brief Calculate bounding box
     * 
     * Returns the axis-aligned bounding box encompassing all
     * toolpath positions (start and end states of each move).
     */
    AABB getBoundingBox() const {
        if (moves_.empty()) {
            return AABB();
        }

        Vec3 firstPos = moves_[0].getStartState().getPosition();
        Vec3 minCorner = firstPos;
        Vec3 maxCorner = firstPos;

        for (const auto& move : moves_) {
            Vec3 start = move.getStartState().getPosition();
            Vec3 end = move.getEndState().getPosition();

            minCorner.x = std::min({minCorner.x, start.x, end.x});
            minCorner.y = std::min({minCorner.y, start.y, end.y});
            minCorner.z = std::min({minCorner.z, start.z, end.z});

            maxCorner.x = std::max({maxCorner.x, start.x, end.x});
            maxCorner.y = std::max({maxCorner.y, start.y, end.y});
            maxCorner.z = std::max({maxCorner.z, start.z, end.z});
        }

        return AABB(minCorner, maxCorner);
    }

    /**
     * @brief Calculate total toolpath length
     * 
     * Sums the length of all moves.
     */
    double getTotalLength() const {
        double total = 0.0;
        for (const auto& move : moves_) {
            total += move.getLength();
        }
        return total;
    }

    /**
     * @brief Calculate estimated machining time
     * 
     * Sums the estimated time of all moves.
     * This is a hook for time estimation - actual time depends on
     * machine acceleration, deceleration, and controller behavior.
     * 
     * @param defaultRapidRate Default rapid rate (units/min)
     */
    double getEstimatedMachiningTime(double defaultRapidRate = 10000.0) const {
        double total = 0.0;
        for (const auto& move : moves_) {
            total += move.getEstimatedTime(defaultRapidRate);
        }
        return total;
    }

    /**
     * @brief Get tool usage summary
     * 
     * Returns a map of tool ID to usage count (number of moves using that tool).
     */
    const std::unordered_map<std::string, std::size_t>& getToolUsageSummary() const {
        return toolUsage_;
    }

    /**
     * @brief Get unique tool IDs used in this toolpath
     */
    std::vector<std::string> getUsedToolIds() const {
        std::vector<std::string> toolIds;
        toolIds.reserve(toolUsage_.size());
        for (const auto& pair : toolUsage_) {
            toolIds.push_back(pair.first);
        }
        return toolIds;
    }

    /**
     * @brief Get first state (start of toolpath)
     */
    ToolpathState getFirstState() const {
        if (moves_.empty()) {
            Vec3 origin(0.0, 0.0, 0.0);
            return ToolpathState(origin);
        }
        return moves_[0].getStartState();
    }

    /**
     * @brief Get last state (end of toolpath)
     */
    ToolpathState getLastState() const {
        if (moves_.empty()) {
            Vec3 origin(0.0, 0.0, 0.0);
            return ToolpathState(origin);
        }
        return moves_.back().getEndState();
    }

    /**
     * @brief Check if toolpath is valid
     * 
     * Checks that all moves are valid.
     */
    bool isValid() const {
        for (const auto& move : moves_) {
            if (!move.isValid()) {
                return false;
            }
        }
        return true;
    }

private:
    std::string id_;                                    ///< Toolpath identifier
    std::string machineId_;                             ///< Machine identifier
    std::vector<ToolpathMove> moves_;                   ///< Ordered sequence of moves
    std::unordered_map<std::string, std::size_t> toolUsage_;  ///< Tool usage count
};

} // namespace cnc
