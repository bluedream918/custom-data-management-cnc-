#pragma once

#include "../common/Types.h"
#include <memory>
#include <string>

namespace cnc {

/**
 * @brief Represents raw stock material before machining
 * 
 * Defines the initial workpiece dimensions, material properties, and
 * coordinate system alignment for the manufacturing process.
 */
class Stock {
public:
    /**
     * @brief Stock origin position
     */
    enum class Origin {
        BottomCenter,    ///< Origin at bottom center of stock
        BottomCorner,    ///< Origin at bottom corner (typically front-left)
        Center,          ///< Origin at geometric center
        Custom           ///< Custom origin position
    };

    virtual ~Stock() = default;

    /**
     * @brief Get stock identifier
     */
    virtual std::string getId() const = 0;

    /**
     * @brief Get stock display name
     */
    virtual std::string getName() const = 0;

    /**
     * @brief Get stock dimensions (width, height, length)
     * 
     * Dimensions are in the order: X (width), Y (length), Z (height)
     */
    virtual Vec3 getDimensions() const = 0;

    /**
     * @brief Get stock bounding box in stock coordinate system
     */
    virtual AABB getBoundingBox() const = 0;

    /**
     * @brief Get stock origin position
     */
    virtual Origin getOrigin() const = 0;

    /**
     * @brief Get custom origin position (if origin is Custom)
     */
    virtual Vec3 getCustomOrigin() const = 0;

    /**
     * @brief Get material properties
     */
    virtual MaterialProperties getMaterial() const = 0;

    /**
     * @brief Get measurement units
     */
    virtual Unit getUnits() const = 0;

    /**
     * @brief Get recommended voxel resolution for simulation
     * 
     * This is a hint for the simulation system about appropriate
     * voxel size for material removal calculations.
     */
    virtual double getRecommendedVoxelSize() const = 0;

    /**
     * @brief Get initial stock geometry identifier
     * 
     * Returns a path or identifier to the initial stock mesh/geometry.
     * Empty string means rectangular stock.
     */
    virtual std::string getInitialGeometryPath() const = 0;

    /**
     * @brief Check if stock has custom initial geometry
     */
    virtual bool hasCustomGeometry() const = 0;

    /**
     * @brief Get stock density (for mass calculations)
     */
    virtual double getDensity() const = 0;

    /**
     * @brief Create a copy of this stock
     */
    virtual std::unique_ptr<Stock> clone() const = 0;
};

} // namespace cnc
