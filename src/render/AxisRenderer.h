#pragma once

#include <QOpenGLFunctions_3_3_Core>
#include <QMatrix4x4>
#include <QOpenGLShaderProgram>

namespace cnc {

/**
 * @brief Renders axis gizmo (X, Y, Z axes)
 * 
 * Draws colored axes at the origin:
 * - X axis: Red
 * - Y axis: Green
 * - Z axis: Blue
 * 
 * Architecture:
 * - Uses modern OpenGL with VBOs
 * - Configurable axis length
 */
class AxisRenderer {
public:
    /**
     * @brief Construct axis renderer
     * @param length Axis length (default 100 units)
     */
    explicit AxisRenderer(double length = 100.0);

    /**
     * @brief Destructor
     */
    ~AxisRenderer();

    /**
     * @brief Render the axes
     * @param gl OpenGL functions
     * @param viewProjectionMatrix View-projection matrix
     */
    void render(QOpenGLFunctions_3_3_Core* gl, const QMatrix4x4& viewProjectionMatrix);

    /**
     * @brief Set axis length
     * @param length Axis length
     */
    void setLength(double length);

    /**
     * @brief Get axis length
     */
    double getLength() const { return length_; }

private:
    /**
     * @brief Initialize axis geometry
     */
    void initializeGeometry(QOpenGLFunctions_3_3_Core* gl);

    /**
     * @brief Initialize shader program
     */
    void initializeShader(QOpenGLFunctions_3_3_Core* gl);

    double length_;
    GLuint vbo_;
    GLuint vao_;
    bool initialized_;
    
    QOpenGLShaderProgram* shaderProgram_; ///< Cached shader program
    bool shaderInitialized_; ///< Whether shader is initialized
};

} // namespace cnc
