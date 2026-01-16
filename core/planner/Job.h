#pragma once

#include "../machine/Machine.h"
#include "../tool/Tool.h"
#include "../material/Stock.h"
#include "../geometry/TargetModel.h"
#include "../common/Types.h"
#include <memory>
#include <string>
#include <vector>
#include <chrono>

namespace cnc {

/**
 * @brief Forward declarations for job outputs
 * These will be implemented in future modules
 */
class ProcessPlan;
class Toolpath;
class GCodeProgram;

/**
 * @brief Job metadata
 */
struct JobMetadata {
    std::string author;
    std::string description;
    std::string version;
    std::vector<std::string> tags;
};

/**
 * @brief Manufacturing job definition
 * 
 * Represents a complete manufacturing job that takes stock and produces
 * a finished part. Contains all inputs (machine, tools, stock, target)
 * and declares outputs (process plan, toolpaths, G-code).
 * 
 * This class is designed to be immutable-friendly and serializable.
 */
class Job {
public:
    /**
     * @brief Job status
     */
    enum class Status {
        Draft,          ///< Job is being defined
        Planned,        ///< Process plan has been generated
        ToolpathsReady, ///< Toolpaths have been generated
        GCodeReady,     ///< G-code has been generated
        Simulated,      ///< Simulation has been completed
        Ready,          ///< Job is ready for execution
        Error           ///< Job has errors
    };

    /**
     * @brief Construct a new job
     */
    Job(
        std::string id,
        std::string name,
        std::shared_ptr<const Machine> machine,
        std::vector<std::shared_ptr<const Tool>> tools,
        std::shared_ptr<const Stock> stock,
        std::shared_ptr<const TargetModel> targetModel
    ) : id_(std::move(id)),
        name_(std::move(name)),
        machine_(std::move(machine)),
        tools_(std::move(tools)),
        stock_(std::move(stock)),
        targetModel_(std::move(targetModel)),
        createdAt_(std::chrono::system_clock::now()),
        modifiedAt_(createdAt_) {}

    /**
     * @brief Get job identifier
     */
    std::string getId() const { return id_; }

    /**
     * @brief Get job display name
     */
    std::string getName() const { return name_; }

    /**
     * @brief Get machine for this job
     */
    std::shared_ptr<const Machine> getMachine() const { return machine_; }

    /**
     * @brief Get tools for this job
     */
    const std::vector<std::shared_ptr<const Tool>>& getTools() const { return tools_; }

    /**
     * @brief Get stock for this job
     */
    std::shared_ptr<const Stock> getStock() const { return stock_; }

    /**
     * @brief Get target model for this job
     */
    std::shared_ptr<const TargetModel> getTargetModel() const { return targetModel_; }

    /**
     * @brief Get current job status
     */
    Status getStatus() const { return status_; }

    /**
     * @brief Set job status
     */
    void setStatus(Status status) { status_ = status; }

    /**
     * @brief Get process plan (if generated)
     * 
     * Returns nullptr if process plan has not been generated yet.
     */
    std::shared_ptr<const ProcessPlan> getProcessPlan() const { return processPlan_; }

    /**
     * @brief Set process plan
     */
    void setProcessPlan(std::shared_ptr<const ProcessPlan> plan) {
        processPlan_ = std::move(plan);
        touch();
    }

    /**
     * @brief Get toolpaths (if generated)
     * 
     * Returns empty vector if toolpaths have not been generated yet.
     */
    const std::vector<std::shared_ptr<const Toolpath>>& getToolpaths() const { return toolpaths_; }

    /**
     * @brief Set toolpaths
     */
    void setToolpaths(std::vector<std::shared_ptr<const Toolpath>> toolpaths) {
        toolpaths_ = std::move(toolpaths);
        touch();
    }

    /**
     * @brief Get G-code program (if generated)
     * 
     * Returns nullptr if G-code has not been generated yet.
     */
    std::shared_ptr<const GCodeProgram> getGCode() const { return gcode_; }

    /**
     * @brief Set G-code program
     */
    void setGCode(std::shared_ptr<const GCodeProgram> gcode) {
        gcode_ = std::move(gcode);
        touch();
    }

    /**
     * @brief Validate job inputs
     * 
     * Checks that all required inputs are present and compatible.
     * @return True if job is valid, false otherwise
     */
    bool validate() const {
        return getValidationErrors().empty();
    }

    /**
     * @brief Get validation errors
     * 
     * Returns a list of error messages if validation fails.
     */
    std::vector<std::string> getValidationErrors() const {
        std::vector<std::string> errors;
        
        if (!machine_) {
            errors.push_back("Machine is not set");
        }
        
        if (tools_.empty()) {
            errors.push_back("No tools specified");
        }
        
        if (!stock_) {
            errors.push_back("Stock is not set");
        }
        
        if (!targetModel_) {
            errors.push_back("Target model is not set");
        }
        
        // Additional validation can be added here
        // - Check machine/tool compatibility
        // - Check stock/target model compatibility
        // - Check units consistency
        
        return errors;
    }

    /**
     * @brief Get job creation timestamp
     */
    std::chrono::system_clock::time_point getCreatedAt() const { return createdAt_; }

    /**
     * @brief Get job modification timestamp
     */
    std::chrono::system_clock::time_point getModifiedAt() const { return modifiedAt_; }

    /**
     * @brief Update modification timestamp
     */
    void touch() {
        modifiedAt_ = std::chrono::system_clock::now();
    }

    /**
     * @brief Get job metadata
     */
    JobMetadata getMetadata() const { return metadata_; }

    /**
     * @brief Set job metadata
     */
    void setMetadata(const JobMetadata& metadata) { metadata_ = metadata; }

private:
    std::string id_;
    std::string name_;
    std::shared_ptr<const Machine> machine_;
    std::vector<std::shared_ptr<const Tool>> tools_;
    std::shared_ptr<const Stock> stock_;
    std::shared_ptr<const TargetModel> targetModel_;
    
    Status status_ = Status::Draft;
    
    std::shared_ptr<const ProcessPlan> processPlan_;
    std::vector<std::shared_ptr<const Toolpath>> toolpaths_;
    std::shared_ptr<const GCodeProgram> gcode_;
    
    std::chrono::system_clock::time_point createdAt_;
    std::chrono::system_clock::time_point modifiedAt_;
    
    JobMetadata metadata_;
};

} // namespace cnc
