#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QMouseEvent>
#include <QWheelEvent>

// Forward declarations
namespace cnc {
    class Camera;
    class GridRenderer;
    class AxisRenderer;
}

namespace cnc {

/**
 * @brief OpenGL viewport widget for CNC simulation visualization
 * 
 * Provides an OpenGL 3.3 Core Profile rendering context with CAD-style
 * orthographic camera, grid, and axis rendering.
 * 
 * Architecture:
 * - Camera logic in Camera class
 * - Grid rendering in GridRenderer
 * - Axis rendering in AxisRenderer
 * - GLViewport handles input and render calls
 * - No CNC logic
 * 
 * Controls:
 * - Mouse wheel: Zoom in/out
 * - Middle mouse drag: Pan camera
 * 
 * OpenGL Configuration:
 * - Version: 3.3 Core Profile
 * - Depth buffer: Enabled
 * - Orthographic projection only
 */
class GLViewport : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core {
    Q_OBJECT

public:
    /**
     * @brief Construct OpenGL viewport
     * @param parent Parent widget
     */
    explicit GLViewport(QWidget* parent = nullptr);

    /**
     * @brief Destructor
     */
    ~GLViewport() override;

protected:
    /**
     * @brief Initialize OpenGL context
     */
    void initializeGL() override;

    /**
     * @brief Handle viewport resize
     */
    void resizeGL(int width, int height) override;

    /**
     * @brief Render frame
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
    Camera* camera_;
    GridRenderer* gridRenderer_;
    AxisRenderer* axisRenderer_;
    
    int viewportWidth_;
    int viewportHeight_;
    
    // Mouse interaction state
    bool isPanning_;
    QPoint lastMousePos_;
};

} // namespace cnc
