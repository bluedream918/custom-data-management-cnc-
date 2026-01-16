#pragma once

#include "../geometry/Transform.h"
#include "../common/Types.h"
#include <cstdint>

namespace cnc {

/**
 * @brief CNC work offset identifier
 * 
 * Standard G-code work offset codes (G54-G59, G54.1 P1-P300, etc.)
 */
enum class WorkOffsetId : std::int32_t {
    G54 = 1,    ///< Work offset 1 (G54)
    G55 = 2,    ///< Work offset 2 (G55)
    G56 = 3,    ///< Work offset 3 (G56)
    G57 = 4,    ///< Work offset 4 (G57)
    G58 = 5,    ///< Work offset 5 (G58)
    G59 = 6,    ///< Work offset 6 (G59)
    G59_1 = 7,  ///< Extended work offset 1 (G54.1 P1)
    G59_2 = 8,  ///< Extended work offset 2 (G54.1 P2)
    G59_3 = 9   ///< Extended work offset 3 (G54.1 P3)
};

/**
 * @brief Represents a CNC work offset (G54/G55 style)
 * 
 * Work offsets define the relationship between machine coordinates
 * and workpiece coordinates. They are applied as a transform between
 * the machine coordinate system and the workpiece coordinate system.
 * 
 * Coordinate system behavior:
 * - Machine coordinates: Absolute machine position
 * - Workpiece coordinates: Position relative to workpiece origin
 * - Work offset: Transform from workpiece origin to machine origin
 * 
 * G-code convention:
 * - G54-G59: Standard work offsets
 * - G54.1 P1-P300: Extended work offsets (for multiple fixtures)
 * - Work offset is applied: machine_coords = workpiece_coords + offset
 * 
 * Industrial control assumptions:
 * - Offset is typically translation only (rotation is optional)
 * - Offset is set during setup/probing
 * - Active offset can be changed during program execution
 */
class WorkOffset {
public:
    /**
     * @brief Default constructor (creates G54 offset at origin)
     */
    WorkOffset() : id_(WorkOffsetId::G54),
                   transform_(Transform::identity()) {
    }

    /**
     * @brief Construct work offset
     * @param id Work offset identifier (G54, G55, etc.)
     * @param translation Translation vector (machine origin -> workpiece origin)
     * @param rotation Optional rotation (default: no rotation)
     */
    WorkOffset(
        WorkOffsetId id,
        const Vec3& translation,
        const Quaternion& rotation = Quaternion::identity()
    ) : id_(id),
        transform_(Transform(translation, rotation)) {
    }

    /**
     * @brief Construct work offset from transform
     * @param id Work offset identifier
     * @param transform Transform from workpiece to machine coordinates
     */
    WorkOffset(
        WorkOffsetId id,
        const Transform& transform
    ) : id_(id),
        transform_(transform) {
    }

    /**
     * @brief Get work offset identifier
     */
    WorkOffsetId getId() const {
        return id_;
    }

    /**
     * @brief Get transform (workpiece -> machine)
     */
    const Transform& getTransform() const {
        return transform_;
    }

    /**
     * @brief Set transform
     */
    void setTransform(const Transform& transform) {
        transform_ = transform;
    }

    /**
     * @brief Get translation component
     */
    Vec3 getTranslation() const {
        return transform_.getPosition();
    }

    /**
     * @brief Set translation component
     */
    void setTranslation(const Vec3& translation) {
        transform_ = Transform(translation, transform_.getRotation());
    }

    /**
     * @brief Get rotation component
     */
    Quaternion getRotation() const {
        return transform_.getRotation();
    }

    /**
     * @brief Set rotation component
     */
    void setRotation(const Quaternion& rotation) {
        transform_ = Transform(transform_.getPosition(), rotation);
    }

    /**
     * @brief Convert point from workpiece coordinates to machine coordinates
     * 
     * Applies the work offset transform.
     */
    Vec3 workpieceToMachine(const Vec3& workpiecePoint) const {
        return transform_.transformPoint(workpiecePoint);
    }

    /**
     * @brief Convert point from machine coordinates to workpiece coordinates
     * 
     * Applies the inverse work offset transform.
     */
    Vec3 machineToWorkpiece(const Vec3& machinePoint) const {
        Transform inverse = transform_.inverse();
        return inverse.transformPoint(machinePoint);
    }

    /**
     * @brief Check if offset is translation-only
     */
    bool isTranslationOnly() const {
        Quaternion rot = transform_.getRotation();
        return std::abs(rot.w - 1.0) < 1e-9 &&
               std::abs(rot.x) < 1e-9 &&
               std::abs(rot.y) < 1e-9 &&
               std::abs(rot.z) < 1e-9;
    }

    /**
     * @brief Check if offset is valid
     */
    bool isValid() const {
        return transform_.getPosition().x == transform_.getPosition().x && // Check for NaN
               transform_.getPosition().y == transform_.getPosition().y &&
               transform_.getPosition().z == transform_.getPosition().z;
    }

private:
    WorkOffsetId id_;       ///< Work offset identifier
    Transform transform_;    ///< Transform from workpiece to machine coordinates
};

} // namespace cnc
