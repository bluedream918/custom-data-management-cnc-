#pragma once

#include "../workpiece/Workpiece.h"
#include "../workpiece/WorkOffset.h"
#include "../geometry/Transform.h"
#include "../common/Types.h"
#include <memory>
#include <optional>
#include <unordered_map>

namespace cnc {

/**
 * @brief Attaches a workpiece to a machine
 * 
 * Manages workpiece mounting and work offset coordination. The machine
 * owns the workpiece mount, and only one workpiece can be active at a time.
 * 
 * Coordinate System Management:
 * - Machine coordinates: Absolute machine position
 * - Workpiece coordinates: Position relative to workpiece origin
 * - Work offset: Transform between machine and workpiece coordinates
 * 
 * G54-Style Offset Behavior:
 * - Work offset defines workpiece origin in machine coordinates
 * - Active offset can be changed (G54, G55, etc.)
 * - Multiple offsets can be defined but only one is active
 * - Offset is applied: machine_coords = workpiece_coords + offset
 * 
 * Industrial control assumptions:
 * - Machine owns the mount (not shared)
 * - Only one active workpiece at a time
 * - Work offsets are mutable (can be set/probed)
 * - Active offset can be changed during operation
 */
class WorkpieceMount {
public:
    /**
     * @brief Construct empty workpiece mount
     */
    WorkpieceMount() : activeWorkpiece_(nullptr),
                       activeOffsetId_(WorkOffsetId::G54) {
    }

    /**
     * @brief Mount workpiece
     * 
     * Attaches a workpiece to the machine. Replaces any existing workpiece.
     * 
     * @param workpiece Workpiece to mount (takes ownership)
     */
    void mountWorkpiece(std::unique_ptr<Workpiece> workpiece) {
        if (workpiece && workpiece->isValid()) {
            activeWorkpiece_ = std::move(workpiece);
        }
    }

    /**
     * @brief Unmount workpiece
     * 
     * Removes the current workpiece. Machine has no workpiece after this.
     */
    void unmountWorkpiece() {
        activeWorkpiece_.reset();
    }

    /**
     * @brief Check if workpiece is mounted
     */
    bool hasWorkpiece() const {
        return activeWorkpiece_ != nullptr;
    }

    /**
     * @brief Get active workpiece
     */
    const Workpiece* getWorkpiece() const {
        return activeWorkpiece_.get();
    }

    /**
     * @brief Set work offset
     * 
     * Defines or updates a work offset (G54, G55, etc.).
     * 
     * @param offset Work offset to set
     */
    void setWorkOffset(const WorkOffset& offset) {
        workOffsets_[offset.getId()] = offset;
    }

    /**
     * @brief Get work offset
     * 
     * @param id Work offset identifier
     * @return Work offset, or nullopt if not defined
     */
    std::optional<WorkOffset> getWorkOffset(WorkOffsetId id) const {
        auto it = workOffsets_.find(id);
        if (it != workOffsets_.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    /**
     * @brief Set active work offset
     * 
     * Changes the active work offset (like G54, G55 G-code commands).
     * 
     * @param id Work offset identifier to activate
     */
    void setActiveWorkOffset(WorkOffsetId id) {
        activeOffsetId_ = id;
    }

    /**
     * @brief Get active work offset identifier
     */
    WorkOffsetId getActiveWorkOffsetId() const {
        return activeOffsetId_;
    }

    /**
     * @brief Get active work offset
     * 
     * @return Active work offset, or nullopt if not defined
     */
    std::optional<WorkOffset> getActiveWorkOffset() const {
        return getWorkOffset(activeOffsetId_);
    }

    /**
     * @brief Convert point from workpiece coordinates to machine coordinates
     * 
     * Applies both workpiece transform and active work offset.
     * 
     * Transform chain:
     * 1. Workpiece local transform (workpiece frame -> workpiece world)
     * 2. Active work offset (workpiece world -> machine)
     * 
     * @param workpiecePoint Point in workpiece coordinates
     * @return Point in machine coordinates
     */
    Vec3 workpieceToMachine(const Vec3& workpiecePoint) const {
        if (!activeWorkpiece_) {
            return workpiecePoint; // No workpiece, assume machine coords
        }

        // Apply workpiece transform
        Vec3 workpieceWorld = activeWorkpiece_->workpieceToMachine(workpiecePoint);

        // Apply active work offset
        auto offset = getActiveWorkOffset();
        if (offset) {
            return offset->workpieceToMachine(workpieceWorld);
        }

        // No offset defined, return workpiece world position
        return workpieceWorld;
    }

    /**
     * @brief Convert point from machine coordinates to workpiece coordinates
     * 
     * Applies inverse of active work offset and workpiece transform.
     * 
     * @param machinePoint Point in machine coordinates
     * @return Point in workpiece coordinates
     */
    Vec3 machineToWorkpiece(const Vec3& machinePoint) const {
        if (!activeWorkpiece_) {
            return machinePoint; // No workpiece, assume machine coords
        }

        // Apply inverse of active work offset
        Vec3 workpieceWorld = machinePoint;
        auto offset = getActiveWorkOffset();
        if (offset) {
            workpieceWorld = offset->machineToWorkpiece(machinePoint);
        }

        // Apply inverse of workpiece transform
        return activeWorkpiece_->machineToWorkpiece(workpieceWorld);
    }

    /**
     * @brief Get workpiece bounding box in machine coordinates
     * 
     * Returns the workpiece bounding box transformed to machine coordinates,
     * accounting for active work offset.
     */
    AABB getWorkpieceBoundingBoxInMachineCoords() const {
        if (!activeWorkpiece_) {
            return AABB();
        }

        // Get bounding box in workpiece coordinates
        AABB workpieceBounds = activeWorkpiece_->getBoundingBoxInWorkpieceCoords();

        // Transform corners to machine coordinates
        Vec3 corners[8] = {
            workpieceBounds.min,
            Vec3(workpieceBounds.max.x, workpieceBounds.min.y, workpieceBounds.min.z),
            Vec3(workpieceBounds.max.x, workpieceBounds.max.y, workpieceBounds.min.z),
            Vec3(workpieceBounds.min.x, workpieceBounds.max.y, workpieceBounds.min.z),
            Vec3(workpieceBounds.min.x, workpieceBounds.min.y, workpieceBounds.max.z),
            Vec3(workpieceBounds.max.x, workpieceBounds.min.y, workpieceBounds.max.z),
            workpieceBounds.max,
            Vec3(workpieceBounds.min.x, workpieceBounds.max.y, workpieceBounds.max.z)
        };

        Vec3 minCorner = workpieceToMachine(corners[0]);
        Vec3 maxCorner = minCorner;

        for (int i = 1; i < 8; ++i) {
            Vec3 machineCorner = workpieceToMachine(corners[i]);
            minCorner.x = std::min(minCorner.x, machineCorner.x);
            minCorner.y = std::min(minCorner.y, machineCorner.y);
            minCorner.z = std::min(minCorner.z, machineCorner.z);
            maxCorner.x = std::max(maxCorner.x, machineCorner.x);
            maxCorner.y = std::max(maxCorner.y, machineCorner.y);
            maxCorner.z = std::max(maxCorner.z, machineCorner.z);
        }

        return AABB(minCorner, maxCorner);
    }

    /**
     * @brief Check if mount is valid
     */
    bool isValid() const {
        if (activeWorkpiece_ && !activeWorkpiece_->isValid()) {
            return false;
        }
        return true;
    }

private:
    std::unique_ptr<Workpiece> activeWorkpiece_;                    ///< Mounted workpiece
    std::unordered_map<WorkOffsetId, WorkOffset> workOffsets_;     ///< Defined work offsets
    WorkOffsetId activeOffsetId_;                                   ///< Active work offset
};

} // namespace cnc
