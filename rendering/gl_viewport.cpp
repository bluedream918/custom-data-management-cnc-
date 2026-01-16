#include "gl_viewport.h"
#include "../src/render/Camera.h"
#include "../src/render/GridRenderer.h"
#include "../src/render/AxisRenderer.h"
#include <QOpenGLContext>
#include <QSurfaceFormat>
#include <QWheelEvent>

namespace cnc {

GLViewport::GLViewport(QWidget* parent)
    : QOpenGLWidget(parent),
      camera_(nullptr),
      gridRenderer_(nullptr),
      axisRenderer_(nullptr),
      viewportWidth_(0),
      viewportHeight_(0),
      isPanning_(false) {
    
    // Configure OpenGL format
    QSurfaceFormat format;
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setDepthBufferSize(24);
    format.setSamples(4); // 4x MSAA for anti-aliasing
    
    setFormat(format);
    
    // Enable mouse tracking for panning
    setMouseTracking(true);
}

GLViewport::~GLViewport() {
    delete camera_;
    delete gridRenderer_;
    delete axisRenderer_;
}

void GLViewport::initializeGL() {
    // Initialize OpenGL function pointers
    initializeOpenGLFunctions();
    
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    
    // Enable line smoothing for better grid/axis visibility
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    
    // Set clear color to dark gray
    glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
    
    // Initialize camera and renderers
    camera_ = new Camera(Camera::ViewPreset::Iso, 1.0);
    gridRenderer_ = new GridRenderer(500.0, 10.0);
    axisRenderer_ = new AxisRenderer(100.0);
    
    // Set initial viewport size (will be updated in resizeGL)
    viewportWidth_ = width();
    viewportHeight_ = height();
    if (viewportWidth_ == 0) viewportWidth_ = 800;
    if (viewportHeight_ == 0) viewportHeight_ = 600;
}

void GLViewport::resizeGL(int width, int height) {
    // Ensure valid size
    if (width <= 0) width = 1;
    if (height <= 0) height = 1;
    
    viewportWidth_ = width;
    viewportHeight_ = height;
    
    // Update viewport to match widget size
    glViewport(0, 0, width, height);
    
    // Trigger repaint after resize
    update();
}

void GLViewport::paintGL() {
    // Ensure viewport size is valid
    if (viewportWidth_ <= 0 || viewportHeight_ <= 0) {
        viewportWidth_ = width();
        viewportHeight_ = height();
        if (viewportWidth_ <= 0) viewportWidth_ = 800;
        if (viewportHeight_ <= 0) viewportHeight_ = 600;
    }
    
    // Clear color and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    if (!camera_ || !gridRenderer_ || !axisRenderer_) {
        return;
    }
    
    // Get view-projection matrix from camera
    QMatrix4x4 viewProj = camera_->getViewProjectionMatrix(
        static_cast<double>(viewportWidth_),
        static_cast<double>(viewportHeight_)
    );
    
    // Render grid first (so axes appear on top)
    gridRenderer_->render(this, viewProj);
    
    // Render axes
    axisRenderer_->render(this, viewProj);
    
    // Check for OpenGL errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        // Silently handle - in production would log
    }
}

void GLViewport::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::MiddleButton) {
        isPanning_ = true;
        lastMousePos_ = event->pos();
        setCursor(Qt::ClosedHandCursor);
    }
    
    QOpenGLWidget::mousePressEvent(event);
}

void GLViewport::mouseMoveEvent(QMouseEvent* event) {
    if (isPanning_ && camera_) {
        QPoint delta = event->pos() - lastMousePos_;
        
        // Convert pixel delta to world space delta
        // Scale by current zoom level
        double panScale = 1.0 / camera_->getZoom();
        double deltaX = static_cast<double>(delta.x()) * panScale;
        double deltaY = static_cast<double>(-delta.y()) * panScale; // Flip Y
        
        camera_->pan(deltaX, deltaY);
        update(); // Trigger repaint
        
        lastMousePos_ = event->pos();
    }
    
    QOpenGLWidget::mouseMoveEvent(event);
}

void GLViewport::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::MiddleButton) {
        isPanning_ = false;
        setCursor(Qt::ArrowCursor);
    }
    
    QOpenGLWidget::mouseReleaseEvent(event);
}

void GLViewport::wheelEvent(QWheelEvent* event) {
    if (camera_) {
        // Get wheel delta (positive = zoom in, negative = zoom out)
        double delta = static_cast<double>(event->angleDelta().y()) / 120.0;
        camera_->zoom(delta);
        update(); // Trigger repaint
    }
    
    QOpenGLWidget::wheelEvent(event);
}

} // namespace cnc
