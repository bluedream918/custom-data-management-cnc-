#pragma once

#include <QMatrix4x4>
#include <QVector3D>
#include <QQuaternion>

namespace cnc {

/**
 * @brief Perspective camera with orbit controls
 * 
 * Provides perspective projection with orbit, pan, and zoom capabilities.
 * Designed for 3D viewport navigation.
 * 
 * Architecture:
 * - Perspective projection only (fixed FOV)
 * - Orbit around target point
 * - Pan via translation
 * - Zoom via distance adjustment
 * - No orthographic mode
 */
class PerspectiveCamera {
public:
    /**
     * @brief Construct perspective camera
     * @param fov Field of view in degrees (default 45.0)
     * @param nearPlane Near clipping plane (default 0.1)
     * @param farPlane Far clipping plane (default 10000.0)
     */
    explicit PerspectiveCamera(
        float fov = 45.0f,
        float nearPlane = 0.1f,
        float farPlane = 10000.0f
    );

    /**
     * @brief Get view matrix
     */
    QMatrix4x4 getViewMatrix() const;

    /**
     * @brief Get projection matrix
     * @param viewportWidth Viewport width
     * @param viewportHeight Viewport height
     */
    QMatrix4x4 getProjectionMatrix(float viewportWidth, float viewportHeight) const;

    /**
     * @brief Get combined view-projection matrix
     * @param viewportWidth Viewport width
     * @param viewportHeight Viewport height
     */
    QMatrix4x4 getViewProjectionMatrix(float viewportWidth, float viewportHeight) const;

    /**
     * @brief Orbit camera around target
     * @param deltaX Horizontal rotation delta (in radians)
     * @param deltaY Vertical rotation delta (in radians)
     */
    void orbit(float deltaX, float deltaY);

    /**
     * @brief Pan camera
     * @param deltaX Pan delta in X direction (in world space)
     * @param deltaY Pan delta in Y direction (in world space)
     */
    void pan(float deltaX, float deltaY);

    /**
     * @brief Zoom camera (adjust distance to target)
     * @param delta Zoom delta (positive = zoom in, negative = zoom out)
     */
    void zoom(float delta);

    /**
     * @brief Set camera distance from target
     * @param distance Distance from target
     */
    void setDistance(float distance);

    /**
     * @brief Get camera distance from target
     */
    float getDistance() const { return distance_; }

    /**
     * @brief Get camera position
     */
    QVector3D getPosition() const;

    /**
     * @brief Get camera target (look-at point)
     */
    QVector3D getTarget() const { return target_; }

    /**
     * @brief Set camera target
     * @param target Target point
     */
    void setTarget(const QVector3D& target);

    /**
     * @brief Reset camera to default position
     */
    void reset();

private:
    /**
     * @brief Update view matrix based on current orbit and pan
     */
    void updateViewMatrix();

    float fov_;
    float nearPlane_;
    float farPlane_;
    
    QVector3D target_;           ///< Look-at target point
    float distance_;             ///< Distance from target
    float azimuth_;              ///< Horizontal rotation (radians)
    float elevation_;            ///< Vertical rotation (radians)
    QVector3D panOffset_;        ///< Pan offset from target
    
    QMatrix4x4 viewMatrix_;
};

} // namespace cnc
