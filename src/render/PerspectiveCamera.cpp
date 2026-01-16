#include "PerspectiveCamera.h"
#include <QtMath>

namespace cnc {

PerspectiveCamera::PerspectiveCamera(float fov, float nearPlane, float farPlane)
    : fov_(fov),
      nearPlane_(nearPlane),
      farPlane_(farPlane),
      target_(0.0f, 0.0f, 0.0f),
      distance_(500.0f),
      azimuth_(0.0f),
      elevation_(qDegreesToRadians(30.0f)),  // 30 degrees up
      panOffset_(0.0f, 0.0f, 0.0f) {
    reset();
}

QMatrix4x4 PerspectiveCamera::getViewMatrix() const {
    return viewMatrix_;
}

QMatrix4x4 PerspectiveCamera::getProjectionMatrix(float viewportWidth, float viewportHeight) const {
    QMatrix4x4 projection;
    
    // Ensure valid viewport size
    if (viewportWidth <= 0.0f) viewportWidth = 800.0f;
    if (viewportHeight <= 0.0f) viewportHeight = 600.0f;
    
    // Perspective projection
    // Use logical pixel dimensions (not physical) for aspect ratio
    float aspect = viewportWidth / viewportHeight;
    projection.perspective(fov_, aspect, nearPlane_, farPlane_);
    
    return projection;
}

QMatrix4x4 PerspectiveCamera::getViewProjectionMatrix(float viewportWidth, float viewportHeight) const {
    // Matrix multiplication order: Projection * View
    // This transforms from world space -> view space -> clip space
    return getProjectionMatrix(viewportWidth, viewportHeight) * getViewMatrix();
}

void PerspectiveCamera::orbit(float deltaX, float deltaY) {
    azimuth_ += deltaX;
    elevation_ += deltaY;
    
    // Clamp elevation to prevent gimbal lock
    const float minElevation = qDegreesToRadians(-89.0f);
    const float maxElevation = qDegreesToRadians(89.0f);
    elevation_ = qBound(minElevation, elevation_, maxElevation);
    
    updateViewMatrix();
}

void PerspectiveCamera::pan(float deltaX, float deltaY) {
    // Calculate pan direction based on current camera orientation
    QVector3D forward = (target_ - getPosition()).normalized();
    QVector3D right = QVector3D::crossProduct(forward, QVector3D(0.0f, 1.0f, 0.0f)).normalized();
    QVector3D up = QVector3D::crossProduct(right, forward).normalized();
    
    // Apply pan offset
    panOffset_ += right * deltaX + up * deltaY;
    updateViewMatrix();
}

void PerspectiveCamera::zoom(float delta) {
    // Adjust distance based on delta
    float newDistance = distance_ * (1.0f - delta * 0.1f);
    setDistance(newDistance);
}

void PerspectiveCamera::setDistance(float distance) {
    if (distance > 1.0f && distance < 100000.0f) {
        distance_ = distance;
        updateViewMatrix();
    }
}

QVector3D PerspectiveCamera::getPosition() const {
    // Calculate position from spherical coordinates
    // Standard spherical coordinates:
    // - elevation: angle from horizontal plane (0 = horizontal, 90 = straight up)
    // - azimuth: rotation around Y axis (0 = +Z direction)
    // 
    // For orbit camera, we want:
    // - x = distance * cos(elevation) * sin(azimuth)  [horizontal component * sin(azimuth)]
    // - y = distance * sin(elevation)                [vertical component]
    // - z = distance * cos(elevation) * cos(azimuth) [horizontal component * cos(azimuth)]
    float x = distance_ * qCos(elevation_) * qSin(azimuth_);
    float y = distance_ * qSin(elevation_);
    float z = distance_ * qCos(elevation_) * qCos(azimuth_);
    
    // Camera position = target + panOffset + spherical offset
    // The panOffset moves both the target and camera together
    QVector3D position = target_ + panOffset_ + QVector3D(x, y, z);
    return position;
}

void PerspectiveCamera::setTarget(const QVector3D& target) {
    target_ = target;
    updateViewMatrix();
}

void PerspectiveCamera::reset() {
    target_ = QVector3D(0.0f, 0.0f, 0.0f);
    distance_ = 300.0f; // Closer default distance to see grid better
    azimuth_ = qDegreesToRadians(45.0f); // Start at 45 degrees for better view
    elevation_ = qDegreesToRadians(30.0f);
    panOffset_ = QVector3D(0.0f, 0.0f, 0.0f);
    updateViewMatrix();
}

void PerspectiveCamera::updateViewMatrix() {
    QVector3D position = getPosition();
    QVector3D up = QVector3D(0.0f, 1.0f, 0.0f);
    
    // Create view matrix
    viewMatrix_.setToIdentity();
    viewMatrix_.lookAt(position, target_ + panOffset_, up);
}

} // namespace cnc
