#include "Camera.h"
#include <QtMath>

namespace cnc {

Camera::Camera(ViewPreset viewPreset, double zoomLevel)
    : viewPreset_(viewPreset),
      zoomLevel_(zoomLevel),
      position_(0.0f, 0.0f, 0.0f),
      target_(0.0f, 0.0f, 0.0f),
      up_(0.0f, 1.0f, 0.0f),
      panOffset_(0.0f, 0.0f, 0.0f) {
    reset();
}

QMatrix4x4 Camera::getViewMatrix() const {
    return viewMatrix_;
}

QMatrix4x4 Camera::getProjectionMatrix(double viewportWidth, double viewportHeight) const {
    QMatrix4x4 projection;
    
    // Ensure valid viewport size
    if (viewportWidth <= 0.0) viewportWidth = 800.0;
    if (viewportHeight <= 0.0) viewportHeight = 600.0;
    
    // Orthographic projection
    // Scale by zoom level - larger zoom = smaller visible area
    double halfWidth = (viewportWidth / 2.0) / zoomLevel_;
    double halfHeight = (viewportHeight / 2.0) / zoomLevel_;
    
    // Use a reasonable depth range
    double nearPlane = -10000.0;
    double farPlane = 10000.0;
    
    projection.ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, nearPlane, farPlane);
    
    return projection;
}

QMatrix4x4 Camera::getViewProjectionMatrix(double viewportWidth, double viewportHeight) const {
    return getProjectionMatrix(viewportWidth, viewportHeight) * getViewMatrix();
}

void Camera::setViewPreset(ViewPreset preset) {
    viewPreset_ = preset;
    updateViewMatrix();
}

void Camera::zoom(double delta, double minZoom, double maxZoom) {
    double newZoom = zoomLevel_ * (1.0 + delta * 0.1);
    setZoom(newZoom);
    
    // Clamp zoom level
    if (zoomLevel_ < minZoom) {
        zoomLevel_ = minZoom;
    } else if (zoomLevel_ > maxZoom) {
        zoomLevel_ = maxZoom;
    }
}

void Camera::setZoom(double zoomLevel) {
    if (zoomLevel > 0.0) {
        zoomLevel_ = zoomLevel;
    }
}

void Camera::pan(double deltaX, double deltaY) {
    // Calculate pan direction based on current view
    QVector3D right, up;
    
    QVector3D forward = (target_ - position_).normalized();
    right = QVector3D::crossProduct(forward, up_).normalized();
    up = QVector3D::crossProduct(right, forward).normalized();
    
    // Apply pan offset
    panOffset_ += right * deltaX + up * deltaY;
    updateViewMatrix();
}

void Camera::reset() {
    panOffset_ = QVector3D(0.0f, 0.0f, 0.0f);
    zoomLevel_ = 1.0;
    updateViewMatrix();
}

void Camera::updateViewMatrix() {
    // Set up camera based on view preset
    switch (viewPreset_) {
        case ViewPreset::Top:
            position_ = QVector3D(0.0f, 500.0f, 0.0f) + panOffset_;
            target_ = QVector3D(0.0f, 0.0f, 0.0f) + panOffset_;
            up_ = QVector3D(0.0f, 0.0f, -1.0f);
            break;
            
        case ViewPreset::Front:
            position_ = QVector3D(0.0f, 0.0f, 500.0f) + panOffset_;
            target_ = QVector3D(0.0f, 0.0f, 0.0f) + panOffset_;
            up_ = QVector3D(0.0f, 1.0f, 0.0f);
            break;
            
        case ViewPreset::Side:
            position_ = QVector3D(500.0f, 0.0f, 0.0f) + panOffset_;
            target_ = QVector3D(0.0f, 0.0f, 0.0f) + panOffset_;
            up_ = QVector3D(0.0f, 1.0f, 0.0f);
            break;
            
        case ViewPreset::Iso:
            // Isometric view (orthographic, angled)
            position_ = QVector3D(300.0f, 300.0f, 300.0f) + panOffset_;
            target_ = QVector3D(0.0f, 0.0f, 0.0f) + panOffset_;
            up_ = QVector3D(0.0f, 1.0f, 0.0f);
            break;
    }
    
    // Create view matrix
    viewMatrix_.setToIdentity();
    viewMatrix_.lookAt(position_, target_, up_);
}

} // namespace cnc
