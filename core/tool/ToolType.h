#pragma once

namespace cnc {

/**
 * @brief Tool category enumeration
 * 
 * Defines the fundamental categories of cutting tools used in CNC machining.
 * Each category has distinct geometric and cutting characteristics.
 * 
 * Note: This complements the ToolType enum in Types.h with additional
 * categorization for tool system organization.
 */
enum class ToolCategory {
    EndMill,        ///< Standard end mill (flat bottom)
    BallEndMill,    ///< Ball nose end mill (rounded tip)
    Drill,          ///< Twist drill (pointed tip)
    Tap,            ///< Tapping tool (thread cutting)
    Reamer,         ///< Reaming tool (hole finishing)
    Boring,         ///< Boring bar (internal diameter)
    FaceMill,       ///< Face milling cutter (large diameter)
    SlotMill,       ///< Slot cutter (T-slot, keyway)
    Custom          ///< Custom tool geometry
};

/**
 * @brief Tool tip geometry type
 * 
 * Describes the shape of the tool tip, which affects cutting behavior
 * and material removal simulation.
 */
enum class ToolTipType {
    Flat,           ///< Flat bottom (end mill)
    Ball,           ///< Ball nose (ball end mill)
    Point,          ///< Pointed tip (drill)
    Chamfer,        ///< Chamfered tip
    Custom          ///< Custom tip geometry
};

} // namespace cnc
