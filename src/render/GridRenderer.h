#pragma once

#include <QOpenGLFunctions_3_3_Core>
#include <QMatrix4x4>
#include <QOpenGLShaderProgram>

namespace cnc {

/**
 * @brief Renders a floor grid for 3D viewport
 * 
 * Draws a grid centered at the origin using VBO+VAO for efficient rendering.
 * Grid lines are thinner than axis lines and always visible.
 * 
 * Architecture:
 * - Uses VBO + VAO for FPS-friendly rendering
 * - No immediate mode OpenGL
 * - Cached geometry for performance
 * - Configurable grid size
 */
class GridRenderer {
public:
    /**
     * @brief Construct grid renderer
     * @param size Grid size (default +/-500 units)
     * @param spacing Grid spacing (default 10 units)
     */
    explicit GridRenderer(double size = 500.0, double spacing = 10.0);

    /**
     * @brief Destructor
     */
    ~GridRenderer();

    /**
     * @brief Render the grid
     * @param gl OpenGL functions
     * @param viewProjectionMatrix View-projection matrix
     */
    void render(QOpenGLFunctions_3_3_Core* gl, const QMatrix4x4& viewProjectionMatrix);

    /**
     * @brief Set grid size
     * @param size Grid size (half-extent)
     */
    void setSize(double size);

    /**
     * @brief Get grid size
     */
    double getSize() const { return size_; }

    /**
     * @brief Set grid spacing
     * @param spacing Grid spacing
     */
    void setSpacing(double spacing);

    /**
     * @brief Get grid spacing
     */
    double getSpacing() const { return spacing_; }

private:
    /**
     * @brief Initialize grid geometry (VBO + VAO)
     */
    void initializeGeometry(QOpenGLFunctions_3_3_Core* gl);

    /**
     * @brief Initialize shader program
     */
    void initializeShader(QOpenGLFunctions_3_3_Core* gl);

    double size_;      ///< Grid size (half-extent)
    double spacing_;   ///< Grid line spacing
    
    GLuint vbo_;       ///< Vertex buffer object
    GLuint vao_;       ///< Vertex array object
    GLsizei vertexCount_;  ///< Number of vertices
    bool initialized_; ///< Whether geometry is initialized
    
    QOpenGLShaderProgram* shaderProgram_; ///< Cached shader program
    bool shaderInitialized_; ///< Whether shader is initialized
};

} // namespace cnc
