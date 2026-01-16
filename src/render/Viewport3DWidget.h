#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QMouseEvent>
#include <QWheelEvent>

// Forward declarations
namespace cnc {
    class PerspectiveCamera;
    class GridRenderer;
    class AxisRenderer;
}

namespace cnc {

/**
 * @brief 3D viewport widget with perspective camera and orbit controls
 * 
 * Provides a 3D OpenGL viewport with:
 * - Perspective projection
 * - Orbit controls (left mouse drag)
 * - Pan controls (middle mouse drag)
 * - Zoom controls (mouse wheel)
 * - Grid and axis rendering
 * 
 * Architecture:
 * - Uses OpenGL 3.3 Core Profile
 * - VBO + VAO for efficient rendering
 * - No immediate mode OpenGL
 * - Prepared for future voxel and toolpath rendering
 * 
 * macOS-specific notes:
 * - Uses forward compatible, core profile context
 * - Properly handles Retina display scaling
 */
class Viewport3DWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core {
    Q_OBJECT

public:
    /**
     * @brief Construct 3D viewport widget
     * @param parent Parent widget
     */
    explicit Viewport3DWidget(QWidget* parent = nullptr);

    /**
     * @brief Destructor
     */
    ~Viewport3DWidget() override;

protected:
    /**
     * @brief Initialize OpenGL context
     * 
     * Called once when the OpenGL context is first created.
     * Sets up OpenGL state, initializes function pointers,
     * and creates rendering resources.
     */
    void initializeGL() override;

    /**
     * @brief Handle viewport resize
     * 
     * Called when the widget is resized.
     * Updates the OpenGL viewport and camera projection.
     * 
     * @param width New widget width
     * @param height New widget height
     */
    void resizeGL(int width, int height) override;

    /**
     * @brief Render frame
     * 
     * Called whenever the widget needs to be repainted.
     * Renders grid and axes using VBO+VAO.
     */
    void paintGL() override;

    /**
     * @brief Handle mouse press events
     */
    void mousePressEvent(QMouseEvent* event) override;

    /**
     * @brief Handle mouse move events
     */
    void mouseMoveEvent(QMouseEvent* event) override;

    /**
     * @brief Handle mouse release events
     */
    void mouseReleaseEvent(QMouseEvent* event) override;

    /**
     * @brief Handle wheel events (zoom)
     */
    void wheelEvent(QWheelEvent* event) override;

private:
    PerspectiveCamera* camera_;
    GridRenderer* gridRenderer_;
    AxisRenderer* axisRenderer_;
    
    float viewportWidth_;
    float viewportHeight_;
    
    // Mouse interaction state
    bool isOrbiting_;
    bool isPanning_;
    QPoint lastMousePos_;
};

} // namespace cnc
