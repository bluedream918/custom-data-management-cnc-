#include "GridRenderer.h"
#include <QOpenGLShaderProgram>
#include <QDebug>
#include <vector>
#include <cmath>

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

GridRenderer::GridRenderer(double size, double spacing)
    : size_(size),
      spacing_(spacing),
      vbo_(0),
      vao_(0),
      vertexCount_(0),
      initialized_(false),
      shaderProgram_(nullptr),
      shaderInitialized_(false) {
}

GridRenderer::~GridRenderer() {
    delete shaderProgram_;
    // VBO and VAO cleanup handled by OpenGL context
}

void GridRenderer::setSize(double size) {
    if (size > 0.0) {
        size_ = size;
        initialized_ = false; // Force re-initialization
    }
}

void GridRenderer::setSpacing(double spacing) {
    if (spacing > 0.0) {
        spacing_ = spacing;
        initialized_ = false; // Force re-initialization
    }
}

void GridRenderer::render(QOpenGLFunctions_3_3_Core* gl, const QMatrix4x4& viewProjectionMatrix) {
    if (!gl) {
        return;
    }

    // Initialize shader if needed
    if (!shaderInitialized_) {
        initializeShader(gl);
    }

    // Initialize geometry if needed
    if (!initialized_) {
        initializeGeometry(gl);
    }

    if (vbo_ == 0 || vao_ == 0 || vertexCount_ == 0 || !shaderProgram_) {
        static bool warned = false;
        if (!warned) {
            qWarning() << "GridRenderer: Missing resources - VBO:" << vbo_ << "VAO:" << vao_ 
                       << "Vertices:" << vertexCount_ << "Shader:" << (shaderProgram_ != nullptr);
            warned = true;
        }
        return;
    }

    // Use cached shader program
    if (!shaderProgram_->bind()) {
        qWarning() << "GridRenderer: Shader binding failed";
        return;
    }

    // Get uniform locations (check if they exist)
    GLint mvpLoc = shaderProgram_->uniformLocation("mvpMatrix");
    GLint colorLoc = shaderProgram_->uniformLocation("color");
    
    if (mvpLoc == -1 || colorLoc == -1) {
        qWarning() << "GridRenderer: Uniform location not found - mvpMatrix:" << mvpLoc << "color:" << colorLoc;
        shaderProgram_->release();
        return;
    }
    
    // Set uniforms
    shaderProgram_->setUniformValue(mvpLoc, viewProjectionMatrix);
    // Use bright white for maximum visibility
    shaderProgram_->setUniformValue(colorLoc, QVector3D(1.0f, 1.0f, 1.0f));

    // Clear any previous errors
    GLenum prevError = gl->glGetError();
    if (prevError != GL_NO_ERROR) {
        qWarning() << "GridRenderer: OpenGL error before draw:" << prevError;
        // Clear the error
        while (gl->glGetError() != GL_NO_ERROR) {}
    }

    // Bind VAO and draw
    gl->glBindVertexArray(vao_);
    
    // Ensure vertex attribute is enabled (VAO should handle this, but be safe)
    gl->glEnableVertexAttribArray(0);
    
    // Set line width before drawing (macOS may limit this to 1.0)
    // Use 1.0 to avoid GL_INVALID_VALUE error on macOS
    GLfloat currentLineWidth;
    gl->glGetFloatv(GL_LINE_WIDTH, &currentLineWidth);
    gl->glLineWidth(1.0f);
    
    // Verify vertex count is valid
    if (vertexCount_ <= 0 || vertexCount_ > 100000) {
        qWarning() << "GridRenderer: Invalid vertex count:" << vertexCount_;
        gl->glBindVertexArray(0);
        shaderProgram_->release();
        return;
    }
    
    // Debug: Log before draw
    static bool firstDraw = true;
    if (firstDraw) {
        firstDraw = false;
        qDebug() << "GridRenderer: Drawing" << vertexCount_ << "vertices with VAO:" << vao_ << "VBO:" << vbo_;
    }
    
    gl->glDrawArrays(GL_LINES, 0, vertexCount_);
    
    // Check for immediate errors
    GLenum immediateError = gl->glGetError();
    if (immediateError != GL_NO_ERROR) {
        qWarning() << "GridRenderer: Immediate OpenGL error after glDrawArrays:" << immediateError;
    }
    
    gl->glBindVertexArray(0);
    
    shaderProgram_->release();
    
    // Check for OpenGL errors
    GLenum error = gl->glGetError();
    if (error != GL_NO_ERROR) {
        qWarning() << "GridRenderer: OpenGL error after draw:" << error << "(0x" << Qt::hex << error << Qt::dec << ")";
    }
}

void GridRenderer::initializeGeometry(QOpenGLFunctions_3_3_Core* gl) {
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

    // Build grid line vertices
    std::vector<float> vertices;
    
    // Grid lines in X direction (along Z axis)
    for (double z = -size_; z <= size_; z += spacing_) {
        vertices.push_back(static_cast<float>(-size_));
        vertices.push_back(0.0f);
        vertices.push_back(static_cast<float>(z));
        vertices.push_back(static_cast<float>(size_));
        vertices.push_back(0.0f);
        vertices.push_back(static_cast<float>(z));
    }
    
    // Grid lines in Z direction (along X axis)
    for (double x = -size_; x <= size_; x += spacing_) {
        vertices.push_back(static_cast<float>(x));
        vertices.push_back(0.0f);
        vertices.push_back(static_cast<float>(-size_));
        vertices.push_back(static_cast<float>(x));
        vertices.push_back(0.0f);
        vertices.push_back(static_cast<float>(size_));
    }

    if (vertices.empty()) {
        return;
    }

    vertexCount_ = static_cast<GLsizei>(vertices.size() / 3);

    // Create VAO
    gl->glGenVertexArrays(1, &vao_);
    if (vao_ == 0) {
        qWarning() << "GridRenderer: Failed to create VAO";
        return;
    }
    gl->glBindVertexArray(vao_);

    // Create VBO
    gl->glGenBuffers(1, &vbo_);
    if (vbo_ == 0) {
        qWarning() << "GridRenderer: Failed to create VBO";
        gl->glDeleteVertexArrays(1, &vao_);
        vao_ = 0;
        return;
    }
    gl->glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    gl->glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    // Check for errors
    GLenum error = gl->glGetError();
    if (error != GL_NO_ERROR) {
        qWarning() << "GridRenderer: OpenGL error during VBO creation:" << error;
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
        qWarning() << "GridRenderer: Vertex attribute 0 not enabled after setup!";
    }
    
    // Unbind VBO first, then VAO (order matters for VAO state)
    gl->glBindBuffer(GL_ARRAY_BUFFER, 0);
    gl->glBindVertexArray(0);

    initialized_ = true;
    qDebug() << "GridRenderer: Geometry initialized - VAO:" << vao_ << "VBO:" << vbo_ << "Vertices:" << vertexCount_;
}

void GridRenderer::initializeShader(QOpenGLFunctions_3_3_Core* gl) {
    if (!gl || shaderInitialized_) {
        return;
    }

    if (shaderProgram_) {
        delete shaderProgram_;
    }

    shaderProgram_ = new QOpenGLShaderProgram();
    if (!shaderProgram_->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource)) {
        qWarning() << "GridRenderer: Vertex shader compilation failed:" << shaderProgram_->log();
        delete shaderProgram_;
        shaderProgram_ = nullptr;
        return;
    }
    if (!shaderProgram_->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource)) {
        qWarning() << "GridRenderer: Fragment shader compilation failed:" << shaderProgram_->log();
        delete shaderProgram_;
        shaderProgram_ = nullptr;
        return;
    }
    if (!shaderProgram_->link()) {
        qWarning() << "GridRenderer: Shader linking failed:" << shaderProgram_->log();
        delete shaderProgram_;
        shaderProgram_ = nullptr;
        return;
    }

    shaderInitialized_ = true;
    qDebug() << "GridRenderer: Shader initialized successfully";
}

} // namespace cnc
