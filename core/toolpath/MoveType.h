#pragma once

namespace cnc {

/**
 * @brief CNC motion type enumeration
 * 
 * Defines all motion and control types used in CNC toolpaths.
 * Each type maps directly to G-code commands for post-processing.
 * 
 * G-code mapping:
 * - Rapid: G0 (rapid positioning)
 * - Linear: G1 (linear interpolation)
 * - ArcCW: G2 (circular interpolation clockwise)
 * - ArcCCW: G3 (circular interpolation counter-clockwise)
 * - Dwell: G4 (dwell/pause)
 * - ToolChange: M6 (tool change)
 * - SpindleStart: M3/M4 (spindle start CW/CCW)
 * - SpindleStop: M5 (spindle stop)
 */
enum class MoveType {
    Rapid,          ///< Rapid positioning (G0) - non-cutting movement
    Linear,         ///< Linear interpolation (G1) - straight cutting movement
    ArcCW,          ///< Circular arc clockwise (G2) - arc cutting movement
    ArcCCW,         ///< Circular arc counter-clockwise (G3) - arc cutting movement
    Dwell,          ///< Dwell/pause (G4) - time delay
    ToolChange,     ///< Tool change (M6) - tool swap operation
    SpindleStart,   ///< Spindle start (M3/M4) - start spindle rotation
    SpindleStop     ///< Spindle stop (M5) - stop spindle rotation
};

/**
 * @brief Check if move type is a cutting motion
 * 
 * Cutting motions remove material (Linear, ArcCW, ArcCCW).
 * Non-cutting motions are positioning or control only.
 */
inline bool isCuttingMove(MoveType type) {
    return type == MoveType::Linear ||
           type == MoveType::ArcCW ||
           type == MoveType::ArcCCW;
}

/**
 * @brief Check if move type is an arc motion
 */
inline bool isArcMove(MoveType type) {
    return type == MoveType::ArcCW || type == MoveType::ArcCCW;
}

/**
 * @brief Check if move type requires feedrate
 * 
 * Cutting motions require feedrate. Rapid, Dwell, and control moves do not.
 */
inline bool requiresFeedrate(MoveType type) {
    return isCuttingMove(type);
}

/**
 * @brief Check if move type is a control command
 * 
 * Control commands change machine state without motion.
 */
inline bool isControlMove(MoveType type) {
    return type == MoveType::ToolChange ||
           type == MoveType::SpindleStart ||
           type == MoveType::SpindleStop;
}

} // namespace cnc
