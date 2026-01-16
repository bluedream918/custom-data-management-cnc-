#pragma once

#include <QMatrix4x4>
#include <QVector3D>

namespace cnc {

/**
 * @brief Orthographic camera for CAD-style viewport
 * 
 * Provides orthographic projection with zoom and pan capabilities.
 * Supports preset views: Top, Front, Side, and Isometric (orthographic).
 * 
 * Architecture:
 * - No perspective projection
 * - Zoom via scale factor
 * - Pan via translation
 * - View presets for common orientations
 */
class Camera {
public:
    /**
     * @brief Camera view preset
     */
    enum class ViewPreset {
        Top,      ///< Top view (looking down -Z)
        Front,    ///< Front view (looking down -Y)
        Side,     ///< Side view (looking down -X)
        Iso       ///< Isometric view (orthographic, angled)
    };

    /**
     * @brief Construct camera
     * @param viewPreset Initial view preset
     * @param zoomLevel Initial zoom level (default 1.0)
     */
    explicit Camera(ViewPreset viewPreset = ViewPreset::Iso, double zoomLevel = 1.0);

    /**
     * @brief Get view matrix
     */
    QMatrix4x4 getViewMatrix() const;

    /**
     * @brief Get projection matrix
     * @param viewportWidth Viewport width
     * @param viewportHeight Viewport height
     */
    QMatrix4x4 getProjectionMatrix(double viewportWidth, double viewportHeight) const;

    /**
     * @brief Get combined view-projection matrix
     * @param viewportWidth Viewport width
     * @param viewportHeight Viewport height
     */
    QMatrix4x4 getViewProjectionMatrix(double viewportWidth, double viewportHeight) const;

    /**
     * @brief Set view preset
     * @param preset View preset to apply
     */
    void setViewPreset(ViewPreset preset);

    /**
     * @brief Get current view preset
     */
    ViewPreset getViewPreset() const { return viewPreset_; }

    /**
     * @brief Zoom in/out
     * @param delta Zoom delta (positive = zoom in, negative = zoom out)
     * @param minZoom Minimum zoom level (default 0.1)
     * @param maxZoom Maximum zoom level (default 100.0)
     */
    void zoom(double delta, double minZoom = 0.1, double maxZoom = 100.0);

    /**
     * @brief Set zoom level
     * @param zoomLevel Zoom level
     */
    void setZoom(double zoomLevel);

    /**
     * @brief Get current zoom level
     */
    double getZoom() const { return zoomLevel_; }

    /**
     * @brief Pan camera
     * @param deltaX Pan delta in X direction (in world space)
     * @param deltaY Pan delta in Y direction (in world space)
     */
    void pan(double deltaX, double deltaY);

    /**
     * @brief Reset camera to default position
     */
    void reset();

    /**
     * @brief Get camera position
     */
    QVector3D getPosition() const { return position_; }

    /**
     * @brief Get camera target (look-at point)
     */
    QVector3D getTarget() const { return target_; }

private:
    /**
     * @brief Update view matrix based on current preset and pan
     */
    void updateViewMatrix();

    ViewPreset viewPreset_;
    double zoomLevel_;
    QVector3D position_;
    QVector3D target_;
    QVector3D up_;
    QVector3D panOffset_;  ///< Pan offset from default position
    QMatrix4x4 viewMatrix_;
};

} // namespace cnc
