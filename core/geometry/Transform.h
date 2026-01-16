#pragma once

#include "../common/Types.h"
#include <array>
#include <cmath>

namespace cnc {

/**
 * @brief Quaternion representation for rotations
 * 
 * Represents a rotation as (w, x, y, z) where w is the scalar part
 * and (x, y, z) is the vector part.
 */
struct Quaternion {
    double w = 1.0;  ///< Scalar part
    double x = 0.0;  ///< X component
    double y = 0.0;  ///< Y component
    double z = 0.0;  ///< Z component

    Quaternion() = default;
    Quaternion(double w_, double x_, double y_, double z_) : w(w_), x(x_), y(y_), z(z_) {}

    /**
     * @brief Create identity quaternion (no rotation)
     */
    static Quaternion identity() {
        return Quaternion(1.0, 0.0, 0.0, 0.0);
    }

    /**
     * @brief Create quaternion from axis-angle representation
     * @param axis Rotation axis (normalized)
     * @param angle Rotation angle in radians
     */
    static Quaternion fromAxisAngle(const Vec3& axis, double angle) {
        double halfAngle = angle * 0.5;
        double s = std::sin(halfAngle);
        return Quaternion(
            std::cos(halfAngle),
            axis.x * s,
            axis.y * s,
            axis.z * s
        );
    }

    /**
     * @brief Get quaternion magnitude
     */
    double magnitude() const {
        return std::sqrt(w * w + x * x + y * y + z * z);
    }

    /**
     * @brief Normalize quaternion
     */
    Quaternion normalized() const {
        double mag = magnitude();
        if (mag > 0.0) {
            return Quaternion(w / mag, x / mag, y / mag, z / mag);
        }
        return identity();
    }

    /**
     * @brief Get conjugate (inverse rotation)
     */
    Quaternion conjugate() const {
        return Quaternion(w, -x, -y, -z);
    }

    /**
     * @brief Multiply two quaternions (composition)
     */
    Quaternion operator*(const Quaternion& other) const {
        return Quaternion(
            w * other.w - x * other.x - y * other.y - z * other.z,
            w * other.x + x * other.w + y * other.z - z * other.y,
            w * other.y - x * other.z + y * other.w + z * other.x,
            w * other.z + x * other.y - y * other.x + z * other.w
        );
    }

    /**
     * @brief Rotate a vector by this quaternion
     */
    Vec3 rotate(const Vec3& v) const {
        Quaternion qv(0.0, v.x, v.y, v.z);
        Quaternion result = (*this) * qv * conjugate();
        return Vec3(result.x, result.y, result.z);
    }
};

/**
 * @brief Rigid transform (position + rotation)
 * 
 * Represents a 3D transformation with translation and rotation.
 * Used for tool poses, coordinate system transformations, etc.
 * 
 * Transform composition: T2 * T1 applies T1 first, then T2.
 */
class Transform {
public:
    /**
     * @brief Construct identity transform
     */
    Transform() : position_(0.0, 0.0, 0.0), rotation_(Quaternion::identity()) {}

    /**
     * @brief Construct transform from position and rotation
     */
    Transform(const Vec3& position, const Quaternion& rotation)
        : position_(position), rotation_(rotation.normalized()) {}

    /**
     * @brief Construct transform from position only (no rotation)
     */
    explicit Transform(const Vec3& position)
        : position_(position), rotation_(Quaternion::identity()) {}

    /**
     * @brief Get position component
     */
    const Vec3& getPosition() const {
        return position_;
    }

    /**
     * @brief Get rotation component
     */
    const Quaternion& getRotation() const {
        return rotation_;
    }

    /**
     * @brief Set position
     */
    void setPosition(const Vec3& pos) {
        position_ = pos;
    }

    /**
     * @brief Set rotation
     */
    void setRotation(const Quaternion& rot) {
        rotation_ = rot.normalized();
    }

    /**
     * @brief Transform a point (apply rotation then translation)
     */
    Vec3 transformPoint(const Vec3& point) const {
        return position_ + rotation_.rotate(point);
    }

    /**
     * @brief Transform a direction vector (rotation only, no translation)
     */
    Vec3 transformDirection(const Vec3& direction) const {
        return rotation_.rotate(direction);
    }

    /**
     * @brief Inverse transform (inverse rotation, then inverse translation)
     */
    Transform inverse() const {
        Quaternion invRot = rotation_.conjugate();
        Vec3 invPos = invRot.rotate(Vec3(-position_.x, -position_.y, -position_.z));
        return Transform(invPos, invRot);
    }

    /**
     * @brief Compose transforms: this * other (applies other first, then this)
     */
    Transform operator*(const Transform& other) const {
        Vec3 newPos = transformPoint(other.position_);
        Quaternion newRot = rotation_ * other.rotation_;
        return Transform(newPos, newRot);
    }

    /**
     * @brief Create identity transform
     */
    static Transform identity() {
        return Transform();
    }

    /**
     * @brief Create translation-only transform
     */
    static Transform translation(const Vec3& translation) {
        return Transform(translation);
    }

    /**
     * @brief Create rotation-only transform
     */
    static Transform rotation(const Quaternion& rotation) {
        return Transform(Vec3(0.0, 0.0, 0.0), rotation);
    }

    /**
     * @brief Create transform from position and axis-angle rotation
     */
    static Transform fromPositionAndAxisAngle(const Vec3& position, const Vec3& axis, double angle) {
        return Transform(position, Quaternion::fromAxisAngle(axis.normalized(), angle));
    }

private:
    Vec3 position_;
    Quaternion rotation_;
};

} // namespace cnc
