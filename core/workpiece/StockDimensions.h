#pragma once

#include "../common/Types.h"
#include <algorithm>
#include <cmath>

namespace cnc {

/**
 * @brief Physical dimensions of raw stock
 * 
 * Represents the physical size of stock material. All dimensions are
 * unit-agnostic - the caller decides whether units are mm, inches, etc.
 * 
 * Coordinate system convention:
 * - Width: X-axis dimension
 * - Length: Y-axis dimension  
 * - Height: Z-axis dimension
 * 
 * Industrial control assumptions:
 * - All dimensions must be positive
 * - Dimensions are immutable (value type)
 * - Units are context-dependent
 */
class StockDimensions {
public:
    /**
     * @brief Construct stock dimensions
     * @param width X-axis dimension
     * @param length Y-axis dimension
     * @param height Z-axis dimension
     */
    StockDimensions(
        double width,
        double length,
        double height
    ) : width_(width > 0.0 ? width : 0.0),
        length_(length > 0.0 ? length : 0.0),
        height_(height > 0.0 ? height : 0.0) {
    }

    /**
     * @brief Get width (X-axis dimension)
     */
    double getWidth() const {
        return width_;
    }

    /**
     * @brief Get length (Y-axis dimension)
     */
    double getLength() const {
        return length_;
    }

    /**
     * @brief Get height (Z-axis dimension)
     */
    double getHeight() const {
        return height_;
    }

    /**
     * @brief Get dimensions as Vec3 (width, length, height)
     */
    Vec3 getDimensions() const {
        return Vec3(width_, length_, height_);
    }

    /**
     * @brief Get bounding box in stock coordinate system
     * 
     * Returns AABB with origin at (0,0,0) and max at (width, length, height).
     * This assumes stock coordinate system origin at one corner.
     */
    AABB getBoundingBox() const {
        return AABB(
            Vec3(0.0, 0.0, 0.0),
            Vec3(width_, length_, height_)
        );
    }

    /**
     * @brief Get volume
     */
    double getVolume() const {
        return width_ * length_ * height_;
    }

    /**
     * @brief Get center point
     */
    Vec3 getCenter() const {
        return Vec3(width_ * 0.5, length_ * 0.5, height_ * 0.5);
    }

    /**
     * @brief Check if dimensions are valid
     */
    bool isValid() const {
        return width_ > 0.0 &&
               length_ > 0.0 &&
               height_ > 0.0 &&
               std::isfinite(width_) &&
               std::isfinite(length_) &&
               std::isfinite(height_);
    }

    /**
     * @brief Check if dimensions are equal (within tolerance)
     */
    bool equals(const StockDimensions& other, double tolerance = 1e-9) const {
        return std::abs(width_ - other.width_) < tolerance &&
               std::abs(length_ - other.length_) < tolerance &&
               std::abs(height_ - other.height_) < tolerance;
    }

private:
    double width_;   ///< X-axis dimension
    double length_;  ///< Y-axis dimension
    double height_;  ///< Z-axis dimension
};

} // namespace cnc
