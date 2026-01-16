#include "Viewport3DWidget.h"
#include "PerspectiveCamera.h"
#include "GridRenderer.h"
#include "AxisRenderer.h"
#include <QOpenGLContext>
#include <QSurfaceFormat>
#include <QDebug>
#include <QtMath>

namespace cnc {

Viewport3DWidget::Viewport3DWidget(QWidget* parent)
    : QOpenGLWidget(parent),
      camera_(nullptr),
      gridRenderer_(nullptr),
      axisRenderer_(nullptr),
      viewportWidth_(800.0f),
      viewportHeight_(600.0f),
      isOrbiting_(false),
      isPanning_(false) {
    
    // Configure OpenGL format for macOS compatibility
    // macOS requires forward compatible, core profile for 3.3+
    QSurfaceFormat format;
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setOption(QSurfaceFormat::DeprecatedFunctions, false); // Forward compatible
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setSamples(4); // 4x MSAA for anti-aliasing
    
    setFormat(format);
    
    // Enable mouse tracking for smooth interaction
    setMouseTracking(true);
}

Viewport3DWidget::~Viewport3DWidget() {
    makeCurrent(); // Ensure OpenGL context is current for cleanup
    delete camera_;
    delete gridRenderer_;
    delete axisRenderer_;
    doneCurrent();
}

void Viewport3DWidget::initializeGL() {
    // Initialize OpenGL function pointers
    // This is critical: must be called first to load OpenGL functions
    initializeOpenGLFunctions();
    
    // Verify OpenGL version
    QOpenGLContext* context = QOpenGLContext::currentContext();
    if (!context) {
        return; // No context available
    }
    
    QSurfaceFormat format = context->format();
    // Verify we have at least OpenGL 3.3
    if (format.majorVersion() < 3 || (format.majorVersion() == 3 && format.minorVersion() < 3)) {
        // OpenGL version too low - would need fallback
        return;
    }
    
    // Enable depth testing (we'll selectively disable it for overlay-style line rendering)
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_TRUE);
    glClearDepth(1.0);
    
    // Disable face culling for lines (lines don't have faces)
    glDisable(GL_CULL_FACE);
    
    // macOS core profile line smoothing is unreliable and can cause "invisible lines".
    // Keep it off; we'll render crisp 1px lines (the only width typically supported on macOS).
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_BLEND);
    
    // Set line width (macOS typically limits to 1.0)
    // Get the actual supported range first
    GLfloat lineWidthRange[2];
    glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, lineWidthRange);
    qDebug() << "OpenGL Line Width Range:" << lineWidthRange[0] << "to" << lineWidthRange[1];
    
    // Use the maximum supported line width (usually 1.0 on macOS)
    float maxLineWidth = qMin(lineWidthRange[1], 1.0f);
    glLineWidth(maxLineWidth);
    qDebug() << "Using line width:" << maxLineWidth;
    
    // Set clear color to dark background (slightly lighter for visibility)
    glClearColor(0.15f, 0.15f, 0.18f, 1.0f);
    
    // Verify VAO support
    GLint maxVertexAttribs;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxVertexAttribs);
    qDebug() << "OpenGL Max Vertex Attribs:" << maxVertexAttribs;
    qDebug() << "OpenGL Version:" << format.majorVersion() << "." << format.minorVersion();
    
    // Initialize camera and renderers
    // These must be created after OpenGL context is initialized
    // Camera: 45 degrees FOV, near=0.1, far=10000
    // Start closer to see the grid better
    camera_ = new PerspectiveCamera(45.0f, 0.1f, 10000.0f);
    // Camera will use reset() which sets distance to 300 and azimuth to 45 degrees
    gridRenderer_ = new GridRenderer(500.0, 10.0);
    axisRenderer_ = new AxisRenderer(300.0); // Larger so it's unmistakable
    
    // Set initial viewport size
    viewportWidth_ = static_cast<float>(width());
    viewportHeight_ = static_cast<float>(height());
    if (viewportWidth_ <= 0.0f) viewportWidth_ = 800.0f;
    if (viewportHeight_ <= 0.0f) viewportHeight_ = 600.0f;
    
    // Force initial geometry initialization
    // This ensures VAOs are created with valid OpenGL context
    if (gridRenderer_ && axisRenderer_) {
        // Geometry will be initialized on first render call
        // when OpenGL context is guaranteed to be current
    }
    
    // Force initial repaint to ensure viewport renders immediately
    update();
}

void Viewport3DWidget::resizeGL(int width, int height) {
    // Ensure valid size
    if (width <= 0) width = 1;
    if (height <= 0) height = 1;
    
    viewportWidth_ = static_cast<float>(width);
    viewportHeight_ = static_cast<float>(height);
    
    // Update viewport to match widget size
    // On macOS Retina displays, devicePixelRatio() handles scaling
    // QOpenGLWidget size is in device-independent pixels; OpenGL viewport is in framebuffer pixels.
    const qreal dpr = devicePixelRatioF();
    glViewport(0, 0, static_cast<GLsizei>(width * dpr), static_cast<GLsizei>(height * dpr));
    
    // Trigger repaint after resize
    update();
}

void Viewport3DWidget::paintGL() {
    // Ensure we have a valid OpenGL context
    if (!isValid()) {
        return;
    }
    
    // Ensure viewport size is valid
    if (viewportWidth_ <= 0.0f || viewportHeight_ <= 0.0f) {
        viewportWidth_ = static_cast<float>(width());
        viewportHeight_ = static_cast<float>(height());
        if (viewportWidth_ <= 0.0f) viewportWidth_ = 800.0f;
        if (viewportHeight_ <= 0.0f) viewportHeight_ = 600.0f;
    }
    
    // Update viewport to match framebuffer size (important for Retina displays)
    const qreal dpr = devicePixelRatioF();
    glViewport(
        0,
        0,
        static_cast<GLsizei>(viewportWidth_ * dpr),
        static_cast<GLsizei>(viewportHeight_ * dpr)
    );
    
    // Clear color and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    if (!camera_ || !gridRenderer_ || !axisRenderer_) {
        return;
    }
    
    // Get view-projection matrix from camera
    QMatrix4x4 viewProj = camera_->getViewProjectionMatrix(viewportWidth_, viewportHeight_);
    
    // Debug: Log camera info on first frame
    static bool firstFrame = true;
    if (firstFrame) {
        firstFrame = false;
        QVector3D pos = camera_->getPosition();
        QVector3D target = camera_->getTarget();
        QMatrix4x4 view = camera_->getViewMatrix();
        QMatrix4x4 proj = camera_->getProjectionMatrix(viewportWidth_, viewportHeight_);
        
        qDebug() << "=== Camera Debug Info ===";
        qDebug() << "Camera position:" << pos;
        qDebug() << "Camera target:" << target;
        qDebug() << "Camera distance:" << camera_->getDistance();
        qDebug() << "Viewport size:" << viewportWidth_ << "x" << viewportHeight_;
        qDebug() << "View matrix (first row):" << view(0,0) << view(0,1) << view(0,2) << view(0,3);
        qDebug() << "Projection matrix (first row):" << proj(0,0) << proj(0,1) << proj(0,2) << proj(0,3);
        qDebug() << "View-Projection matrix (first row):" << viewProj(0,0) << viewProj(0,1) << viewProj(0,2) << viewProj(0,3);
        
        // Test: Check if a point at origin is in view frustum
        QVector4D testPoint(0.0f, 0.0f, 0.0f, 1.0f);
        QVector4D clipSpace = viewProj * testPoint;
        qDebug() << "Origin in clip space:" << clipSpace;
        qDebug() << "Origin visible? (w>0 and |x|,|y|,|z| < w):" 
                 << (clipSpace.w() > 0 && 
                     qAbs(clipSpace.x()) < clipSpace.w() && 
                     qAbs(clipSpace.y()) < clipSpace.w() && 
                     qAbs(clipSpace.z()) < clipSpace.w());
    }
    
    // TEST: Render a simple colored triangle AND a line to verify rendering works
    // This should be visible if the rendering pipeline is working
    static bool testRendered = false;
    if (!testRendered) {
        testRendered = true;
        
        // Create a simple shader program for testing
        QOpenGLShaderProgram testShader;
        const char* vs = "#version 330 core\n"
                         "layout(location = 0) in vec3 position;\n"
                         "uniform mat4 mvp;\n"
                         "void main() { gl_Position = mvp * vec4(position, 1.0); }\n";
        const char* fs = "#version 330 core\n"
                         "out vec4 fragColor;\n"
                         "void main() { fragColor = vec4(1.0, 0.0, 1.0, 1.0); }\n"; // Bright magenta
        
        if (testShader.addShaderFromSourceCode(QOpenGLShader::Vertex, vs) &&
            testShader.addShaderFromSourceCode(QOpenGLShader::Fragment, fs) &&
            testShader.link() && testShader.bind()) {
            
            // Test triangle vertices (should be visible at origin)
            float testVerts[] = {
                0.0f, 0.0f, 0.0f,
                50.0f, 0.0f, 0.0f,
                0.0f, 50.0f, 0.0f
            };
            
            GLuint testVAO, testVBO;
            glGenVertexArrays(1, &testVAO);
            glBindVertexArray(testVAO);
            glGenBuffers(1, &testVBO);
            glBindBuffer(GL_ARRAY_BUFFER, testVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(testVerts), testVerts, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
            
            testShader.setUniformValue("mvp", viewProj);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            
            // Also test a line from origin
            float testLine[] = {
                0.0f, 0.0f, 0.0f,
                100.0f, 100.0f, 0.0f
            };
            glBufferData(GL_ARRAY_BUFFER, sizeof(testLine), testLine, GL_STATIC_DRAW);
            glLineWidth(1.0f);
            glDrawArrays(GL_LINES, 0, 2);
            
            // Check for errors
            GLenum error = glGetError();
            if (error != GL_NO_ERROR) {
                qWarning() << "Test rendering: OpenGL error:" << error;
            }
            
            glBindVertexArray(0);
            glDeleteBuffers(1, &testVBO);
            glDeleteVertexArrays(1, &testVAO);
            testShader.release();
            
            qDebug() << "Test triangle and line rendered - if you see magenta, rendering works!";
        } else {
            qWarning() << "Test shader failed:" << testShader.log();
        }
    }
    
    // Render grid/axes in a known-good state.
    // On macOS core profile, line smoothing/variable line width is not reliable, and depth testing
    // can cause lines on the ground plane to disappear due to precision/z-fighting.
    // Disable depth test for grid/axes so they're always visible
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glDisable(GL_LINE_SMOOTH);
    glLineWidth(1.0f);

    // TEST: Render a simple large line first to verify line rendering works
    static bool testLineRendered = false;
    if (!testLineRendered) {
        testLineRendered = true;
        
        // Use the same shader as grid/axis for consistency
        QOpenGLShaderProgram testShader;
        const char* vs = "#version 330 core\n"
                         "layout(location = 0) in vec3 position;\n"
                         "uniform mat4 mvpMatrix;\n"
                         "void main() { gl_Position = mvpMatrix * vec4(position, 1.0); }\n";
        const char* fs = "#version 330 core\n"
                         "out vec4 fragColor;\n"
                         "uniform vec3 color;\n"
                         "void main() { fragColor = vec4(color, 1.0); }\n";
        
        if (testShader.addShaderFromSourceCode(QOpenGLShader::Vertex, vs) &&
            testShader.addShaderFromSourceCode(QOpenGLShader::Fragment, fs) &&
            testShader.link() && testShader.bind()) {
            
            // Large test line from (-200,0,0) to (200,0,0) - should be very visible
            float testLine[] = {
                -200.0f, 0.0f, 0.0f,
                 200.0f, 0.0f, 0.0f
            };
            
            GLuint testVAO, testVBO;
            glGenVertexArrays(1, &testVAO);
            glBindVertexArray(testVAO);
            glGenBuffers(1, &testVBO);
            glBindBuffer(GL_ARRAY_BUFFER, testVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(testLine), testLine, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
            
            GLint mvpLoc = testShader.uniformLocation("mvpMatrix");
            GLint colorLoc = testShader.uniformLocation("color");
            if (mvpLoc != -1 && colorLoc != -1) {
                testShader.setUniformValue(mvpLoc, viewProj);
                testShader.setUniformValue(colorLoc, QVector3D(1.0f, 1.0f, 0.0f)); // Bright yellow
                glLineWidth(1.0f);
                glDrawArrays(GL_LINES, 0, 2);
                qDebug() << "Test line rendered (bright yellow horizontal line) - if visible, line rendering works!";
            }
            
            glBindVertexArray(0);
            glDeleteBuffers(1, &testVBO);
            glDeleteVertexArrays(1, &testVAO);
            testShader.release();
        }
    }
    
    // Render grid first (so axes appear on top)
    
    bool gridRendered = false;
    bool axisRendered = false;
    
    gridRenderer_->render(this, viewProj);
    gridRendered = true;
    
    // Render axes
    axisRenderer_->render(this, viewProj);
    axisRendered = true;
    
    // Re-enable depth test for future 3D objects
    glEnable(GL_DEPTH_TEST);

    // Restore default depth test for future 3D rendering
    glEnable(GL_DEPTH_TEST);
    
    // Debug: Log render status
    static int frameCount = 0;
    frameCount++;
    if (frameCount == 1) {
        qDebug() << "First frame rendered - Grid:" << gridRendered << "Axis:" << axisRendered;
        qDebug() << "View-Projection matrix:" << viewProj;
    }
    
    // Check for OpenGL errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        qWarning() << "OpenGL Error in paintGL:" << error;
    }
}

void Viewport3DWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        isOrbiting_ = true;
        lastMousePos_ = event->pos();
        setCursor(Qt::ClosedHandCursor);
    } else if (event->button() == Qt::MiddleButton) {
        isPanning_ = true;
        lastMousePos_ = event->pos();
        setCursor(Qt::SizeAllCursor);
    }
    
    QOpenGLWidget::mousePressEvent(event);
}

void Viewport3DWidget::mouseMoveEvent(QMouseEvent* event) {
    if (camera_) {
        QPoint delta = event->pos() - lastMousePos_;
        
        if (isOrbiting_) {
            // Orbit: convert pixel delta to rotation delta
            float sensitivity = 0.005f; // Rotation sensitivity
            float deltaX = static_cast<float>(delta.x()) * sensitivity;
            float deltaY = static_cast<float>(-delta.y()) * sensitivity; // Flip Y
            
            camera_->orbit(deltaX, deltaY);
            update(); // Trigger repaint
        } else if (isPanning_) {
            // Pan: convert pixel delta to world space delta
            float panScale = camera_->getDistance() * 0.001f;
            float deltaX = static_cast<float>(delta.x()) * panScale;
            float deltaY = static_cast<float>(-delta.y()) * panScale; // Flip Y
            
            camera_->pan(deltaX, deltaY);
            update(); // Trigger repaint
        }
        
        lastMousePos_ = event->pos();
    }
    
    QOpenGLWidget::mouseMoveEvent(event);
}

void Viewport3DWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        isOrbiting_ = false;
        setCursor(Qt::ArrowCursor);
    } else if (event->button() == Qt::MiddleButton) {
        isPanning_ = false;
        setCursor(Qt::ArrowCursor);
    }
    
    QOpenGLWidget::mouseReleaseEvent(event);
}

void Viewport3DWidget::wheelEvent(QWheelEvent* event) {
    if (camera_) {
        // Get wheel delta (positive = zoom in, negative = zoom out)
        float delta = static_cast<float>(event->angleDelta().y()) / 120.0f;
        camera_->zoom(delta);
        update(); // Trigger repaint
    }
    
    QOpenGLWidget::wheelEvent(event);
}

} // namespace cnc
