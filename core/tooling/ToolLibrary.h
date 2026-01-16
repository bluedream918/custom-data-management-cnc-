#pragma once

#include "Tool.h"
#include <unordered_map>
#include <vector>
#include <string>
#include <algorithm>

namespace cnc {

/**
 * @brief Manages a collection of tools
 * 
 * Provides tool storage, lookup, and management for CAM operations.
 * Tools are stored by unique ID for fast lookup.
 * 
 * Serialization-ready structure:
 * - Tools can be serialized to JSON/YAML by ID
 * - Library structure is designed for persistence
 * - No file I/O in this layer (handled by higher layers)
 * 
 * Industrial control assumptions:
 * - Tool IDs must be unique
 * - Tools are immutable after addition
 * - Library is the source of truth for available tools
 */
class ToolLibrary {
public:
    /**
     * @brief Construct empty tool library
     */
    ToolLibrary() = default;

    /**
     * @brief Add tool to library
     * 
     * Adds a tool to the library. If a tool with the same ID
     * already exists, it is replaced.
     * 
     * @param tool Tool to add
     * @return True if tool was added, false if ID already exists (replaced)
     */
    bool addTool(const Tool& tool) {
        if (!tool.isValid()) {
            return false;
        }
        bool existed = tools_.find(tool.getId()) != tools_.end();
        tools_.insert_or_assign(tool.getId(), tool);
        return !existed;
    }

    /**
     * @brief Remove tool from library
     * 
     * @param toolId Tool identifier
     * @return True if tool was removed, false if not found
     */
    bool removeTool(const std::string& toolId) {
        auto it = tools_.find(toolId);
        if (it != tools_.end()) {
            tools_.erase(it);
            return true;
        }
        return false;
    }

    /**
     * @brief Get tool by ID
     * 
     * @param toolId Tool identifier
     * @return Pointer to tool, or nullptr if not found
     */
    const Tool* getTool(const std::string& toolId) const {
        auto it = tools_.find(toolId);
        if (it != tools_.end()) {
            return &it->second;
        }
        return nullptr;
    }

    /**
     * @brief Check if tool exists
     */
    bool hasTool(const std::string& toolId) const {
        return tools_.find(toolId) != tools_.end();
    }

    /**
     * @brief Get all tools
     * 
     * Returns a vector of all tools in the library.
     */
    std::vector<Tool> getAllTools() const {
        std::vector<Tool> result;
        result.reserve(tools_.size());
        for (const auto& pair : tools_) {
            result.push_back(pair.second);
        }
        return result;
    }

    /**
     * @brief Get tools by type
     * 
     * Returns all tools of the specified type.
     */
    std::vector<Tool> getToolsByType(ToolingType type) const {
        std::vector<Tool> result;
        for (const auto& pair : tools_) {
            if (pair.second.getType() == type) {
                result.push_back(pair.second);
            }
        }
        return result;
    }

    /**
     * @brief Get number of tools
     */
    std::size_t getToolCount() const {
        return tools_.size();
    }

    /**
     * @brief Check if library is empty
     */
    bool isEmpty() const {
        return tools_.empty();
    }

    /**
     * @brief Clear all tools
     */
    void clear() {
        tools_.clear();
    }

    /**
     * @brief Validate for duplicate IDs
     * 
     * Checks that all tools have unique IDs.
     * 
     * @return Vector of duplicate IDs (empty if no duplicates)
     */
    std::vector<std::string> validateDuplicates() const {
        std::vector<std::string> duplicates;
        std::unordered_map<std::string, int> idCounts;
        
        for (const auto& pair : tools_) {
            idCounts[pair.first]++;
        }
        
        for (const auto& pair : idCounts) {
            if (pair.second > 1) {
                duplicates.push_back(pair.first);
            }
        }
        
        return duplicates;
    }

    /**
     * @brief Check if library is valid
     * 
     * Validates that all tools are valid and there are no duplicates.
     */
    bool isValid() const {
        // Check for duplicates
        if (!validateDuplicates().empty()) {
            return false;
        }
        
        // Check all tools are valid
        for (const auto& pair : tools_) {
            if (!pair.second.isValid()) {
                return false;
            }
        }
        
        return true;
    }

private:
    std::unordered_map<std::string, Tool> tools_;  ///< Tools indexed by ID
};

} // namespace cnc
