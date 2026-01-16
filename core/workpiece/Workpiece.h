#pragma once

#include "StockType.h"
#include "StockDimensions.h"
#include "../geometry/Transform.h"
#include "../common/Types.h"
#include <string>

namespace cnc {

/**
 * @brief Represents raw material mounted on the machine
 * 
 * Encapsulates the physical stock material with its dimensions, type,
 * and pose in machine coordinates. The workpiece has both:
 * - Immutable dimensions (physical size)
 * - Mutable pose (position and orientation in machine coordinates)
 * 
 * Coordinate system:
 * - Workpiece frame: Local coordinate system with origin at stock corner
 * - Machine frame: Global machine coordinate system
 * - Transform: Converts from workpiece frame to machine frame
 * 
 * Industrial control assumptions:
 * - Dimensions are immutable (physical property)
 * - Pose is mutable (workpiece can be repositioned)
 * - Workpiece coordinate system: origin at corner, axes aligned with stock edges
 */
class Workpiece {
public:
    /**
     * @brief Construct workpiece
     * @param id Workpiece identifier
     * @param name Workpiece display name
     * @param type Stock shape type
     * @param dimensions Physical dimensions
     * @param worldTransform Transform from workpiece frame to machine frame
     */
    Workpiece(
        std::string id,
        std::string name,
        StockType type,
        StockDimensions dimensions,
        const Transform& worldTransform = Transform::identity()
    ) : id_(std::move(id)),
        name_(std::move(name)),
        type_(type),
        dimensions_(dimensions),
        worldTransform_(worldTransform) {
    }

    /**
     * @brief Get workpiece identifier
     */
    const std::string& getId() const {
        return id_;
    }

    /**
     * @brief Get workpiece display name
     */
    const std::string& getName() const {
        return name_;
    }

    /**
     * @brief Get stock type
     */
    StockType getType() const {
        return type_;
    }

    /**
     * @brief Get stock dimensions (immutable)
     */
    const StockDimensions& getDimensions() const {
        return dimensions_;
    }

    /**
     * @brief Get world transform (workpiece frame -> machine frame)
     */
    const Transform& getWorldTransform() const {
        return worldTransform_;
    }

    /**
     * @brief Set world transform
     */
    void setWorldTransform(const Transform& transform) {
        worldTransform_ = transform;
    }

    /**
     * @brief Get bounding box in machine coordinates
     * 
     * Returns the workpiece bounding box transformed to machine coordinates.
     */
    AABB getBoundingBoxInMachineCoords() const {
        AABB localBounds = dimensions_.getBoundingBox();
        
        // Transform all corners to machine coordinates
        Vec3 corners[8] = {
            localBounds.min,
            Vec3(localBounds.max.x, localBounds.min.y, localBounds.min.z),
            Vec3(localBounds.max.x, localBounds.max.y, localBounds.min.z),
            Vec3(localBounds.min.x, localBounds.max.y, localBounds.min.z),
            Vec3(localBounds.min.x, localBounds.min.y, localBounds.max.z),
            Vec3(localBounds.max.x, localBounds.min.y, localBounds.max.z),
            localBounds.max,
            Vec3(localBounds.min.x, localBounds.max.y, localBounds.max.z)
        };

        Vec3 minCorner = worldTransform_.transformPoint(corners[0]);
        Vec3 maxCorner = minCorner;

        for (int i = 1; i < 8; ++i) {
            Vec3 worldCorner = worldTransform_.transformPoint(corners[i]);
            minCorner.x = std::min(minCorner.x, worldCorner.x);
            minCorner.y = std::min(minCorner.y, worldCorner.y);
            minCorner.z = std::min(minCorner.z, worldCorner.z);
            maxCorner.x = std::max(maxCorner.x, worldCorner.x);
            maxCorner.y = std::max(maxCorner.y, worldCorner.y);
            maxCorner.z = std::max(maxCorner.z, worldCorner.z);
        }

        return AABB(minCorner, maxCorner);
    }

    /**
     * @brief Get bounding box in workpiece coordinates
     */
    AABB getBoundingBoxInWorkpieceCoords() const {
        return dimensions_.getBoundingBox();
    }

    /**
     * @brief Convert point from workpiece coordinates to machine coordinates
     */
    Vec3 workpieceToMachine(const Vec3& workpiecePoint) const {
        return worldTransform_.transformPoint(workpiecePoint);
    }

    /**
     * @brief Convert point from machine coordinates to workpiece coordinates
     */
    Vec3 machineToWorkpiece(const Vec3& machinePoint) const {
        Transform inverse = worldTransform_.inverse();
        return inverse.transformPoint(machinePoint);
    }

    /**
     * @brief Check if workpiece is valid
     */
    bool isValid() const {
        return !id_.empty() &&
               !name_.empty() &&
               dimensions_.isValid();
    }

private:
    std::string id_;                    ///< Workpiece identifier
    std::string name_;                  ///< Workpiece display name
    StockType type_;                    ///< Stock shape type
    StockDimensions dimensions_;        ///< Physical dimensions (immutable)
    Transform worldTransform_;          ///< Transform to machine coordinates (mutable)
};

} // namespace cnc
