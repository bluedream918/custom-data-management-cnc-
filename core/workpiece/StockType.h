#pragma once

namespace cnc {

/**
 * @brief Raw stock shape enumeration
 * 
 * Defines the fundamental shapes of raw material stock used in CNC machining.
 * Each type has distinct geometric properties and coordinate system conventions.
 */
enum class StockType {
    Block,      ///< Rectangular block (most common)
    Cylinder,   ///< Cylindrical stock (future)
    Custom      ///< Custom geometry (future)
};

} // namespace cnc
