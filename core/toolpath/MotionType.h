#pragma once

namespace cnc {

/**
 * @brief Toolpath motion type enumeration
 * 
 * Defines the fundamental motion types used in CNC toolpaths.
 * Each type maps directly to G-code commands for post-processing.
 * 
 * G-code mapping:
 * - Rapid: G0 (rapid positioning)
 * - Linear: G1 (linear interpolation)
 * - ArcCW: G2 (circular interpolation clockwise)
 * - ArcCCW: G3 (circular interpolation counter-clockwise)
 * - Dwell: G4 (dwell/pause)
 * - ToolChange: M6 (tool change)
 */
enum class MotionType {
    Rapid,      ///< Rapid positioning (G0) - non-cutting movement
    Linear,     ///< Linear interpolation (G1) - straight cutting movement
    ArcCW,      ///< Circular arc clockwise (G2) - arc cutting movement
    ArcCCW,     ///< Circular arc counter-clockwise (G3) - arc cutting movement
    Dwell,      ///< Dwell/pause (G4) - time delay
    ToolChange  ///< Tool change (M6) - tool swap operation
};

/**
 * @brief Check if motion type is a cutting motion
 * 
 * Cutting motions remove material (Linear, ArcCW, ArcCCW).
 * Non-cutting motions are positioning only (Rapid, Dwell, ToolChange).
 */
inline bool isCuttingMotion(MotionType type) {
    return type == MotionType::Linear ||
           type == MotionType::ArcCW ||
           type == MotionType::ArcCCW;
}

/**
 * @brief Check if motion type is an arc motion
 */
inline bool isArcMotion(MotionType type) {
    return type == MotionType::ArcCW || type == MotionType::ArcCCW;
}

/**
 * @brief Check if motion type requires feedrate
 * 
 * Cutting motions require feedrate. Rapid and Dwell do not.
 */
inline bool requiresFeedrate(MotionType type) {
    return isCuttingMotion(type);
}

} // namespace cnc
