#pragma once

#include "../tooling/ToolHolder.h"
#include <vector>
#include <string>

namespace cnc {

/**
 * @brief Tool changer type
 */
enum class ToolChangerType {
    Fixed,      ///< Fixed tool magazine (tools in fixed positions)
    Carousel,   ///< Rotating carousel (tools rotate to change position)
    Chain,      ///< Chain-type magazine
    Custom      ///< Custom tool changer
};

/**
 * @brief Represents tool changer capabilities
 * 
 * Encapsulates tool changer specifications including capacity,
 * change time, and supported holder types. Used for toolpath
 * planning and cycle time estimation.
 * 
 * Industrial control assumptions:
 * - Tool change time is deterministic
 * - Capacity is fixed (cannot exceed)
 * - Supported holder types determine tool compatibility
 * - Tool change operations are atomic
 */
class ToolChanger {
public:
    /**
     * @brief Construct tool changer
     * @param type Tool changer type
     * @param maxToolSlots Maximum number of tool slots
     * @param toolChangeTime Tool change time in seconds
     * @param supportedHolders Vector of supported holder types
     */
    ToolChanger(
        ToolChangerType type,
        int maxToolSlots,
        double toolChangeTime = 5.0,
        const std::vector<HolderType>& supportedHolders = {}
    ) : type_(type),
        maxToolSlots_(maxToolSlots > 0 ? maxToolSlots : 0),
        toolChangeTime_(toolChangeTime >= 0.0 ? toolChangeTime : 5.0),
        supportedHolders_(supportedHolders) {
    }

    /**
     * @brief Get tool changer type
     */
    ToolChangerType getType() const {
        return type_;
    }

    /**
     * @brief Get tool changer type name
     */
    std::string getTypeName() const {
        switch (type_) {
            case ToolChangerType::Fixed: return "Fixed";
            case ToolChangerType::Carousel: return "Carousel";
            case ToolChangerType::Chain: return "Chain";
            case ToolChangerType::Custom: return "Custom";
        }
        return "Unknown";
    }

    /**
     * @brief Get maximum tool slots
     */
    int getMaxToolSlots() const {
        return maxToolSlots_;
    }

    /**
     * @brief Get tool change time
     */
    double getToolChangeTime() const {
        return toolChangeTime_;
    }

    /**
     * @brief Get supported holder types
     */
    const std::vector<HolderType>& getSupportedHolders() const {
        return supportedHolders_;
    }

    /**
     * @brief Check if holder type is supported
     */
    bool supportsHolder(HolderType holderType) const {
        if (supportedHolders_.empty()) {
            return true; // Empty list means all holders supported
        }
        return std::find(supportedHolders_.begin(), supportedHolders_.end(), holderType) != supportedHolders_.end();
    }

    /**
     * @brief Check if tool changer has capacity
     * 
     * @param currentToolCount Current number of tools loaded
     * @return True if there is capacity for more tools
     */
    bool hasCapacity(int currentToolCount) const {
        return currentToolCount < maxToolSlots_;
    }

    /**
     * @brief Check if tool changer is valid
     */
    bool isValid() const {
        return maxToolSlots_ > 0 &&
               toolChangeTime_ >= 0.0 &&
               std::isfinite(toolChangeTime_);
    }

    /**
     * @brief Check if tool changer is present
     * 
     * Returns false if machine has no tool changer (maxToolSlots = 0).
     */
    bool isPresent() const {
        return maxToolSlots_ > 0;
    }

private:
    ToolChangerType type_;                    ///< Tool changer type
    int maxToolSlots_;                        ///< Maximum tool slots
    double toolChangeTime_;                    ///< Tool change time (seconds)
    std::vector<HolderType> supportedHolders_; ///< Supported holder types
};

} // namespace cnc
