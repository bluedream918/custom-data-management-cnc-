#include "AxisRenderer.h"
#include <QOpenGLShaderProgram>
#include <QDebug>

namespace cnc {

// Simple shader source for line rendering
static const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 position;
uniform mat4 mvpMatrix;

void main() {
    gl_Position = mvpMatrix * vec4(position, 1.0);
}
)";

static const char* fragmentShaderSource = R"(
#version 330 core
out vec4 fragColor;
uniform vec3 color;

void main() {
    fragColor = vec4(color, 1.0);
}
)";

AxisRenderer::AxisRenderer(double length)
    : length_(length),
      vbo_(0),
      vao_(0),
      initialized_(false),
      shaderProgram_(nullptr),
      shaderInitialized_(false) {
}

AxisRenderer::~AxisRenderer() {
    delete shaderProgram_;
    // VBO and VAO cleanup will be handled by OpenGL context
}

void AxisRenderer::render(QOpenGLFunctions_3_3_Core* gl, const QMatrix4x4& viewProjectionMatrix) {
    if (!gl) {
        return;
    }

    // Initialize shader if needed
    if (!shaderInitialized_) {
        initializeShader(gl);
    }

    if (!initialized_) {
        initializeGeometry(gl);
    }

    if (vbo_ == 0 || vao_ == 0 || !shaderProgram_) {
        static bool warned = false;
        if (!warned) {
            qWarning() << "AxisRenderer: Missing resources - VBO:" << vbo_ << "VAO:" << vao_ 
                       << "Shader:" << (shaderProgram_ != nullptr);
            warned = true;
        }
        return;
    }

    // Use cached shader program
    if (!shaderProgram_->bind()) {
        qWarning() << "AxisRenderer: Shader binding failed";
        return;
    }

    // Get uniform locations (check if they exist)
    GLint mvpLoc = shaderProgram_->uniformLocation("mvpMatrix");
    GLint colorLoc = shaderProgram_->uniformLocation("color");
    
    if (mvpLoc == -1 || colorLoc == -1) {
        qWarning() << "AxisRenderer: Uniform location not found - mvpMatrix:" << mvpLoc << "color:" << colorLoc;
        shaderProgram_->release();
        return;
    }
    
    // Set uniform for MVP matrix
    shaderProgram_->setUniformValue(mvpLoc, viewProjectionMatrix);

    // Clear any previous errors
    GLenum prevError = gl->glGetError();
    if (prevError != GL_NO_ERROR) {
        qWarning() << "AxisRenderer: OpenGL error before draw:" << prevError;
        // Clear the error
        while (gl->glGetError() != GL_NO_ERROR) {}
    }

    // Bind VAO
    gl->glBindVertexArray(vao_);
    
    // Ensure vertex attribute is enabled (VAO should handle this, but be safe)
    gl->glEnableVertexAttribArray(0);
    
    // Set line width before drawing (macOS may limit this to 1.0)
    // Use 1.0 to avoid GL_INVALID_VALUE error on macOS
    gl->glLineWidth(1.0f);
    
    // Debug: Log before draw
    static bool firstDraw = true;
    if (firstDraw) {
        firstDraw = false;
        qDebug() << "AxisRenderer: Drawing 6 vertices with VAO:" << vao_ << "VBO:" << vbo_;
    }

    // Draw X axis (red)
    shaderProgram_->setUniformValue(colorLoc, QVector3D(1.0f, 0.0f, 0.0f));
    gl->glDrawArrays(GL_LINES, 0, 2);
    
    GLenum err1 = gl->glGetError();
    if (err1 != GL_NO_ERROR) {
        qWarning() << "AxisRenderer: Error after X axis draw:" << err1;
    }

    // Draw Y axis (green)
    shaderProgram_->setUniformValue(colorLoc, QVector3D(0.0f, 1.0f, 0.0f));
    gl->glDrawArrays(GL_LINES, 2, 2);
    
    GLenum err2 = gl->glGetError();
    if (err2 != GL_NO_ERROR) {
        qWarning() << "AxisRenderer: Error after Y axis draw:" << err2;
    }

    // Draw Z axis (blue)
    shaderProgram_->setUniformValue(colorLoc, QVector3D(0.0f, 0.0f, 1.0f));
    gl->glDrawArrays(GL_LINES, 4, 2);
    
    GLenum err3 = gl->glGetError();
    if (err3 != GL_NO_ERROR) {
        qWarning() << "AxisRenderer: Error after Z axis draw:" << err3;
    }

    gl->glBindVertexArray(0);
    
    shaderProgram_->release();
    
    // Check for OpenGL errors
    GLenum error = gl->glGetError();
    if (error != GL_NO_ERROR) {
        qWarning() << "AxisRenderer: OpenGL error after draw:" << error << "(0x" << Qt::hex << error << Qt::dec << ")";
    }
}

void AxisRenderer::setLength(double length) {
    if (length > 0.0) {
        length_ = length;
        initialized_ = false; // Force re-initialization
    }
}

void AxisRenderer::initializeGeometry(QOpenGLFunctions_3_3_Core* gl) {
    if (!gl || initialized_) {
        return;
    }

    // Clean up existing resources
    if (vbo_ != 0) {
        gl->glDeleteBuffers(1, &vbo_);
        vbo_ = 0;
    }
    if (vao_ != 0) {
        gl->glDeleteVertexArrays(1, &vao_);
        vao_ = 0;
    }

    // Create axis vertices: X (red), Y (green), Z (blue)
    std::vector<float> vertices = {
        // X axis (red) - from origin to +X
        0.0f, 0.0f, 0.0f,
        static_cast<float>(length_), 0.0f, 0.0f,
        
        // Y axis (green) - from origin to +Y
        0.0f, 0.0f, 0.0f,
        0.0f, static_cast<float>(length_), 0.0f,
        
        // Z axis (blue) - from origin to +Z
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, static_cast<float>(length_)
    };

    // Create VAO
    gl->glGenVertexArrays(1, &vao_);
    if (vao_ == 0) {
        qWarning() << "AxisRenderer: Failed to create VAO";
        return;
    }
    gl->glBindVertexArray(vao_);

    // Create VBO
    gl->glGenBuffers(1, &vbo_);
    if (vbo_ == 0) {
        qWarning() << "AxisRenderer: Failed to create VBO";
        gl->glDeleteVertexArrays(1, &vao_);
        vao_ = 0;
        return;
    }
    gl->glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    gl->glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    // Check for errors
    GLenum error = gl->glGetError();
    if (error != GL_NO_ERROR) {
        qWarning() << "AxisRenderer: OpenGL error during VBO creation:" << error;
    }
    
    // Set vertex attribute
    // CRITICAL: This must be done while VAO is bound and VBO is bound
    // The VAO will store this state
    gl->glEnableVertexAttribArray(0);
    gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    
    // Verify attribute was set correctly
    GLint enabled = 0;
    gl->glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &enabled);
    if (enabled == 0) {
        qWarning() << "AxisRenderer: Vertex attribute 0 not enabled after setup!";
    }
    
    // Unbind VBO first, then VAO (order matters for VAO state)
    gl->glBindBuffer(GL_ARRAY_BUFFER, 0);
    gl->glBindVertexArray(0);

    initialized_ = true;
    qDebug() << "AxisRenderer: Geometry initialized - VAO:" << vao_ << "VBO:" << vbo_;
}

void AxisRenderer::initializeShader(QOpenGLFunctions_3_3_Core* gl) {
    if (!gl || shaderInitialized_) {
        return;
    }

    if (shaderProgram_) {
        delete shaderProgram_;
    }

    shaderProgram_ = new QOpenGLShaderProgram();
    if (!shaderProgram_->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource)) {
        qWarning() << "AxisRenderer: Vertex shader compilation failed:" << shaderProgram_->log();
        delete shaderProgram_;
        shaderProgram_ = nullptr;
        return;
    }
    if (!shaderProgram_->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource)) {
        qWarning() << "AxisRenderer: Fragment shader compilation failed:" << shaderProgram_->log();
        delete shaderProgram_;
        shaderProgram_ = nullptr;
        return;
    }
    if (!shaderProgram_->link()) {
        qWarning() << "AxisRenderer: Shader linking failed:" << shaderProgram_->log();
        delete shaderProgram_;
        shaderProgram_ = nullptr;
        return;
    }

    shaderInitialized_ = true;
    qDebug() << "AxisRenderer: Shader initialized successfully";
}

} // namespace cnc
