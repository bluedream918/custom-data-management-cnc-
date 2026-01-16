#pragma once

#include "../common/Types.h"
#include <memory>
#include <string>

namespace cnc {

/**
 * @brief Abstract interface for material representation
 * 
 * Represents the state of material in 3D space. Supports queries
 * for material presence and removal operations. Designed to be
 * extensible for different representations (voxel, octree, etc.).
 */
class MaterialGrid {
public:
    virtual ~MaterialGrid() = default;

    /**
     * @brief Check if a point contains material
     * @param point Point in grid coordinate system
     * @return True if material is present at this point
     */
    virtual bool isOccupied(const Vec3& point) const = 0;

    /**
     * @brief Check if a point is empty (no material)
     * @param point Point in grid coordinate system
     * @return True if point is empty
     */
    virtual bool isEmpty(const Vec3& point) const {
        return !isOccupied(point);
    }

    /**
     * @brief Remove material in a region
     * 
     * Performs a boolean subtraction operation, removing material
     * within the specified region.
     * 
     * @param region Bounding box of region to remove
     * @return True if any material was removed
     */
    virtual bool removeRegion(const AABB& region) = 0;

    /**
     * @brief Get bounding box of the material grid
     */
    virtual AABB getBoundingBox() const = 0;

    /**
     * @brief Get grid resolution (voxel size or equivalent)
     * 
     * Returns the smallest representable unit size in the grid.
     */
    virtual double getResolution() const = 0;

    /**
     * @brief Get total volume of remaining material
     */
    virtual double getRemainingVolume() const = 0;

    /**
     * @brief Check if grid is valid
     */
    virtual bool isValid() const = 0;

    /**
     * @brief Create a deep copy of this grid
     */
    virtual std::unique_ptr<MaterialGrid> clone() const = 0;

    /**
     * @brief Get grid type identifier
     */
    virtual std::string getType() const = 0;
};

/**
 * @brief Voxel-based material grid implementation
 * 
 * Placeholder class for voxel grid implementation.
 * Actual voxel math will be implemented later.
 */
class VoxelGrid : public MaterialGrid {
public:
    /**
     * @brief Construct voxel grid
     * @param bounds Bounding box of the grid
     * @param resolution Voxel size (must be > 0)
     */
    VoxelGrid(const AABB& bounds, double resolution);

    virtual ~VoxelGrid() = default;

    bool isOccupied(const Vec3& point) const override;
    bool removeRegion(const AABB& region) override;
    AABB getBoundingBox() const override;
    double getResolution() const override;
    double getRemainingVolume() const override;
    bool isValid() const override;
    std::unique_ptr<MaterialGrid> clone() const override;
    std::string getType() const override { return "VoxelGrid"; }

private:
    AABB bounds_;
    double resolution_;
    // TODO: Add voxel storage when implementing
};

// Inline implementations for VoxelGrid (placeholders)
inline VoxelGrid::VoxelGrid(const AABB& bounds, double resolution)
    : bounds_(bounds), resolution_(resolution) {
}

inline bool VoxelGrid::isOccupied(const Vec3& point) const {
    // TODO: Implement voxel lookup
    return bounds_.contains(point);
}

inline bool VoxelGrid::removeRegion(const AABB& region) {
    // TODO: Implement voxel removal
    return false;
}

inline AABB VoxelGrid::getBoundingBox() const {
    return bounds_;
}

inline double VoxelGrid::getResolution() const {
    return resolution_;
}

inline double VoxelGrid::getRemainingVolume() const {
    // TODO: Implement volume calculation
    return 0.0;
}

inline bool VoxelGrid::isValid() const {
    return bounds_.isValid() && resolution_ > 0.0;
}

inline std::unique_ptr<MaterialGrid> VoxelGrid::clone() const {
    return std::make_unique<VoxelGrid>(bounds_, resolution_);
}

} // namespace cnc
