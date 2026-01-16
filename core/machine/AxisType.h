#pragma once

namespace cnc {

/**
 * @brief Axis type enumeration for machine definition
 * 
 * Defines the fundamental axis types in a CNC machine.
 * Used for machine configuration and kinematics.
 */
enum class AxisType {
    X,      ///< Linear X-axis (typically horizontal)
    Y,      ///< Linear Y-axis (typically horizontal, perpendicular to X)
    Z,      ///< Linear Z-axis (typically vertical)
    A,      ///< Rotary A-axis (rotation around X)
    B,      ///< Rotary B-axis (rotation around Y)
    C,      ///< Rotary C-axis (rotation around Z)
    Custom  ///< Custom axis type (for extensions)
};

/**
 * @brief Check if axis type is linear
 */
inline bool isLinearAxis(AxisType type) {
    return type == AxisType::X || type == AxisType::Y || type == AxisType::Z;
}

/**
 * @brief Check if axis type is rotary
 */
inline bool isRotaryAxis(AxisType type) {
    return type == AxisType::A || type == AxisType::B || type == AxisType::C;
}

} // namespace cnc
