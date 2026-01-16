#pragma once

namespace cnc {

/**
 * @brief Tool type enumeration for tooling system
 * 
 * Defines the fundamental tool types supported by the CAM system.
 * Each type has distinct cutting characteristics and usage patterns.
 */
enum class ToolingType {
    EndMill,    ///< Standard end mill (flat bottom)
    BallMill,   ///< Ball nose end mill (rounded tip)
    FlatMill,   ///< Flat end mill (explicit flat bottom)
    Drill,      ///< Twist drill (pointed tip)
    Chamfer,    ///< Chamfer tool (beveled edge)
    Custom      ///< Custom tool geometry
};

/**
 * @brief Coolant mode for tool operations
 */
enum class CoolantMode {
    None,       ///< No coolant
    Flood,      ///< Flood coolant
    Mist,       ///< Mist coolant
    Through     ///< Through-tool coolant
};

} // namespace cnc
