#pragma once

#include "../common/Types.h"
#include <memory>
#include <string>
#include <vector>

namespace cnc {

/**
 * @brief Model metadata
 */
struct ModelMetadata {
    std::string author;
    std::string description;
    std::string version;
    std::vector<std::string> tags;
};

/**
 * @brief Represents the desired final part geometry
 * 
 * Defines the target model that will be machined from the stock.
 * Contains geometry source information and coordinate system alignment.
 */
class TargetModel {
public:
    /**
     * @brief Coordinate system alignment mode
     */
    enum class Alignment {
        StockOrigin,     ///< Align with stock origin
        StockCenter,     ///< Align with stock center
        ModelOrigin,     ///< Use model's native origin
        Custom           ///< Custom transformation
    };

    virtual ~TargetModel() = default;

    /**
     * @brief Get model identifier
     */
    virtual std::string getId() const = 0;

    /**
     * @brief Get model display name
     */
    virtual std::string getName() const = 0;

    /**
     * @brief Get path to source geometry file
     * 
     * Returns the file path to the mesh/model file (STL, STEP, OBJ, etc.)
     */
    virtual std::string getSourcePath() const = 0;

    /**
     * @brief Get file format of source geometry
     */
    virtual std::string getFormat() const = 0;

    /**
     * @brief Get coordinate system alignment
     */
    virtual Alignment getAlignment() const = 0;

    /**
     * @brief Get custom transformation matrix (if alignment is Custom)
     * 
     * Returns a 4x4 transformation matrix as a 16-element vector (row-major)
     */
    virtual std::vector<double> getCustomTransform() const = 0;

    /**
     * @brief Get measurement units of the model
     */
    virtual Unit getUnits() const = 0;

    /**
     * @brief Get model bounding box in model coordinate system
     */
    virtual AABB getBoundingBox() const = 0;

    /**
     * @brief Get model bounding box in stock coordinate system
     * 
     * This applies the alignment transformation to get the bounding box
     * in the coordinate system of the stock.
     */
    virtual AABB getBoundingBoxInStockCoords() const = 0;

    /**
     * @brief Check if model is valid and loadable
     */
    virtual bool isValid() const = 0;

    /**
     * @brief Get model scale factor
     * 
     * Useful for unit conversion or scaling operations.
     */
    virtual double getScale() const = 0;

    /**
     * @brief Get model metadata
     */
    virtual ModelMetadata getMetadata() const = 0;

    /**
     * @brief Create a copy of this model
     */
    virtual std::unique_ptr<TargetModel> clone() const = 0;
};

} // namespace cnc
