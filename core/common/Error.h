#pragma once

#include <string>
#include <cstdint>

namespace cnc {

/**
 * @brief Error severity levels
 */
enum class ErrorSeverity {
    Info,       ///< Informational message
    Warning,    ///< Warning that doesn't stop execution
    Error,      ///< Error that prevents operation
    Fatal       ///< Fatal error that requires termination
};

/**
 * @brief Error codes for simulation and CNC operations
 */
enum class ErrorCode : std::uint32_t {
    // Success
    Success = 0,

    // Simulation errors (1000-1999)
    SimulationInvalidState = 1000,
    SimulationOutOfBounds = 1001,
    SimulationToolCollision = 1002,
    SimulationMaterialError = 1003,
    SimulationStepFailed = 1004,
    SimulationInvalidTool = 1005,
    SimulationInvalidMachine = 1006,

    // Geometry errors (2000-2999)
    GeometryInvalidTransform = 2000,
    GeometryInvalidBounds = 2001,
    GeometryInvalidOperation = 2002,

    // Material errors (3000-3999)
    MaterialGridInvalid = 3000,
    MaterialGridOutOfBounds = 3001,
    MaterialGridResolutionError = 3002,

    // Machine errors (4000-4999)
    MachineInvalidPosition = 4000,
    MachineKinematicsError = 4001,
    MachineLimitExceeded = 4002,

    // Tool errors (5000-5999)
    ToolInvalidGeometry = 5000,
    ToolInvalidParameters = 5001,

    // General errors (9000-9999)
    InvalidArgument = 9000,
    InvalidState = 9001,
    NotImplemented = 9002,
    UnknownError = 9999
};

/**
 * @brief Strong error type for CNC operations
 * 
 * Provides structured error information with severity, code, message,
 * and recoverability flag. Designed for deterministic error handling
 * and suitable for RL environments.
 */
class Error {
public:
    /**
     * @brief Construct a success error (no error)
     */
    Error() : code_(ErrorCode::Success), severity_(ErrorSeverity::Info), recoverable_(true) {}

    /**
     * @brief Construct an error
     */
    Error(ErrorCode code, ErrorSeverity severity, std::string message, bool recoverable = false)
        : code_(code), severity_(severity), message_(std::move(message)), recoverable_(recoverable) {}

    /**
     * @brief Check if this represents success (no error)
     */
    bool isSuccess() const {
        return code_ == ErrorCode::Success;
    }

    /**
     * @brief Check if this represents an error
     */
    bool isError() const {
        return code_ != ErrorCode::Success;
    }

    /**
     * @brief Get error code
     */
    ErrorCode getCode() const {
        return code_;
    }

    /**
     * @brief Get error severity
     */
    ErrorSeverity getSeverity() const {
        return severity_;
    }

    /**
     * @brief Get error message
     */
    const std::string& getMessage() const {
        return message_;
    }

    /**
     * @brief Check if error is recoverable
     */
    bool isRecoverable() const {
        return recoverable_;
    }

    /**
     * @brief Check if error is fatal
     */
    bool isFatal() const {
        return severity_ == ErrorSeverity::Fatal;
    }

    /**
     * @brief Create a success error
     */
    static Error success() {
        return Error();
    }

    /**
     * @brief Create an error from code and message
     */
    static Error make(ErrorCode code, const std::string& message, bool recoverable = false) {
        ErrorSeverity severity = ErrorSeverity::Error;
        if (code >= ErrorCode::SimulationInvalidState && code <= ErrorCode::SimulationInvalidMachine) {
            severity = ErrorSeverity::Error;
        } else if (code >= ErrorCode::GeometryInvalidTransform && code <= ErrorCode::GeometryInvalidOperation) {
            severity = ErrorSeverity::Error;
        } else if (code >= ErrorCode::MaterialGridInvalid && code <= ErrorCode::MaterialGridResolutionError) {
            severity = ErrorSeverity::Error;
        } else if (code == ErrorCode::MachineLimitExceeded) {
            severity = ErrorSeverity::Warning;
        }
        return Error(code, severity, message, recoverable);
    }

private:
    ErrorCode code_ = ErrorCode::Success;
    ErrorSeverity severity_ = ErrorSeverity::Info;
    std::string message_;
    bool recoverable_ = true;
};

} // namespace cnc
