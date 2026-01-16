#pragma once

#include <cstdint>
#include <array>
#include <string>
#include <memory>
#include <limits>
#include <cmath>

namespace cnc {

/**
 * @brief Measurement units for the CNC system
 */
enum class Unit {
    Millimeter,  ///< Metric system (mm)
    Inch         ///< Imperial system (inches)
};

/**
 * @brief CNC axis identifiers
 */
enum class Axis {
    X = 0,  ///< X-axis (typically horizontal)
    Y = 1,  ///< Y-axis (typically horizontal, perpendicular to X)
    Z = 2,  ///< Z-axis (typically vertical)
    A = 3,  ///< Rotary axis around X
    B = 4,  ///< Rotary axis around Y
    C = 5   ///< Rotary axis around Z
};

/**
 * @brief Tool type classification
 */
enum class ToolType {
    EndMill,      ///< Standard end mill
    BallEndMill,  ///< Ball nose end mill
    Drill,        ///< Twist drill
    Tap,          ///< Tapping tool
    Reamer,       ///< Reaming tool
    Boring,       ///< Boring bar
    FaceMill,     ///< Face milling cutter
    SlotMill,     ///< Slot cutter
    Custom        ///< Custom tool geometry
};

/**
 * @brief 3D vector with double precision
 */
struct Vec3 {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;

    Vec3() = default;
    Vec3(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}

    Vec3 operator+(const Vec3& other) const {
        return Vec3(x + other.x, y + other.y, z + other.z);
    }

    Vec3 operator-(const Vec3& other) const {
        return Vec3(x - other.x, y - other.y, z - other.z);
    }

    Vec3 operator*(double scalar) const {
        return Vec3(x * scalar, y * scalar, z * scalar);
    }

    double length() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    double lengthSquared() const {
        return x * x + y * y + z * z;
    }

    Vec3 normalized() const {
        double len = length();
        if (len > 0.0) {
            return Vec3(x / len, y / len, z / len);
        }
        return Vec3(0.0, 0.0, 0.0);
    }
};

/**
 * @brief Axis-aligned bounding box
 */
struct AABB {
    Vec3 min;  ///< Minimum corner
    Vec3 max;  ///< Maximum corner

    AABB() = default;
    AABB(const Vec3& min_, const Vec3& max_) : min(min_), max(max_) {}

    bool isValid() const {
        return min.x <= max.x && min.y <= max.y && min.z <= max.z;
    }

    Vec3 center() const {
        return Vec3(
            (min.x + max.x) * 0.5,
            (min.y + max.y) * 0.5,
            (min.z + max.z) * 0.5
        );
    }

    Vec3 size() const {
        return max - min;
    }

    bool contains(const Vec3& point) const {
        return point.x >= min.x && point.x <= max.x &&
               point.y >= min.y && point.y <= max.y &&
               point.z >= min.z && point.z <= max.z;
    }
};

/**
 * @brief Machine axis configuration
 */
struct AxisConfig {
    bool hasX = true;
    bool hasY = true;
    bool hasZ = true;
    bool hasA = false;
    bool hasB = false;
    bool hasC = false;

    int axisCount() const {
        int count = 0;
        if (hasX) count++;
        if (hasY) count++;
        if (hasZ) count++;
        if (hasA) count++;
        if (hasB) count++;
        if (hasC) count++;
        return count;
    }

    bool hasAxis(Axis axis) const {
        switch (axis) {
            case Axis::X: return hasX;
            case Axis::Y: return hasY;
            case Axis::Z: return hasZ;
            case Axis::A: return hasA;
            case Axis::B: return hasB;
            case Axis::C: return hasC;
        }
        return false;
    }
};

/**
 * @brief Controller limits for feed rates and accelerations
 */
struct ControllerLimits {
    double maxFeedRate = 1000.0;        ///< Maximum feed rate (mm/min or in/min)
    double maxRapidRate = 10000.0;      ///< Maximum rapid rate (mm/min or in/min)
    double maxAcceleration = 1000.0;    ///< Maximum acceleration (mm/s² or in/s²)
    double maxJerk = 100.0;             ///< Maximum jerk (mm/s³ or in/s³)

    // Per-axis limits (optional, empty means use global)
    std::array<double, 6> maxFeedRatePerAxis = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    std::array<double, 6> maxAccelPerAxis = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
};

/**
 * @brief Material properties
 */
struct MaterialProperties {
    std::string name;              ///< Material name (e.g., "Aluminum 6061")
    double density = 0.0;          ///< Density (g/cm³)
    double hardness = 0.0;         ///< Hardness value
    std::string category;          ///< Category (e.g., "Metal", "Plastic", "Wood")
};

} // namespace cnc
