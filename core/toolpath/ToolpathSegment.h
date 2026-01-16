#pragma once

#include "MotionType.h"
#include "ToolpathPoint.h"
#include "../common/Types.h"
#include <string>
#include <optional>
#include <cmath>

namespace cnc {

/**
 * @brief Arc plane selection for circular interpolation
 */
enum class ArcPlane {
    XY,  ///< Arc in XY plane (most common)
    XZ,  ///< Arc in XZ plane
    YZ   ///< Arc in YZ plane
};

/**
 * @brief Represents one motion command in a toolpath
 * 
 * Encapsulates a single motion segment with start/end points,
 * motion type, and parameters. This is an immutable value type
 * that represents one G-code command.
 * 
 * G-code mapping:
 * - Rapid: G0 X... Y... Z...
 * - Linear: G1 X... Y... Z... F...
 * - ArcCW: G2 X... Y... Z... I... J... K... F...
 * - ArcCCW: G3 X... Y... Z... I... J... K... F...
 * - Dwell: G4 P...
 * - ToolChange: M6 T...
 * 
 * Industrial control assumptions:
 * - Segment is immutable after construction
 * - Geometry is validated during construction
 * - Arc center is in plane coordinates (I, J, K in G-code)
 * - Feedrate is required for cutting motions
 */
class ToolpathSegment {
public:
    /**
     * @brief Construct rapid motion segment
     */
    static ToolpathSegment rapid(
        const ToolpathPoint& start,
        const ToolpathPoint& end,
        const std::string& comment = ""
    ) {
        return ToolpathSegment(MotionType::Rapid, start, end, std::nullopt, ArcPlane::XY, 0.0, comment);
    }

    /**
     * @brief Construct linear motion segment
     */
    static ToolpathSegment linear(
        const ToolpathPoint& start,
        const ToolpathPoint& end,
        double feedrate,
        const std::string& comment = ""
    ) {
        return ToolpathSegment(MotionType::Linear, start, end, std::nullopt, ArcPlane::XY, feedrate, comment);
    }

    /**
     * @brief Construct arc motion segment
     */
    static ToolpathSegment arc(
        MotionType arcType,
        const ToolpathPoint& start,
        const ToolpathPoint& end,
        const Vec3& center,
        ArcPlane plane,
        double feedrate,
        const std::string& comment = ""
    ) {
        return ToolpathSegment(arcType, start, end, center, plane, feedrate, comment);
    }

    /**
     * @brief Construct dwell segment
     */
    static ToolpathSegment dwell(
        const ToolpathPoint& point,
        double duration,
        const std::string& comment = ""
    ) {
        ToolpathSegment seg(MotionType::Dwell, point, point, std::nullopt, ArcPlane::XY, 0.0, comment);
        seg.dwellDuration_ = duration;
        return seg;
    }

    /**
     * @brief Construct tool change segment
     */
    static ToolpathSegment toolChange(
        const ToolpathPoint& point,
        int toolNumber,
        const std::string& comment = ""
    ) {
        ToolpathSegment seg(MotionType::ToolChange, point, point, std::nullopt, ArcPlane::XY, 0.0, comment);
        seg.toolNumber_ = toolNumber;
        return seg;
    }

    /**
     * @brief Get motion type
     */
    MotionType getMotionType() const {
        return motionType_;
    }

    /**
     * @brief Get start point
     */
    const ToolpathPoint& getStartPoint() const {
        return startPoint_;
    }

    /**
     * @brief Get end point
     */
    const ToolpathPoint& getEndPoint() const {
        return endPoint_;
    }

    /**
     * @brief Get arc center (for arc motions)
     */
    const std::optional<Vec3>& getArcCenter() const {
        return arcCenter_;
    }

    /**
     * @brief Get arc plane (for arc motions)
     */
    ArcPlane getArcPlane() const {
        return arcPlane_;
    }

    /**
     * @brief Get feedrate
     */
    double getFeedrate() const {
        return feedrate_;
    }

    /**
     * @brief Get comment
     */
    const std::string& getComment() const {
        return comment_;
    }

    /**
     * @brief Get dwell duration (for dwell segments)
     */
    double getDwellDuration() const {
        return dwellDuration_;
    }

    /**
     * @brief Get tool number (for tool change segments)
     */
    int getToolNumber() const {
        return toolNumber_;
    }

    /**
     * @brief Get segment length
     * 
     * Calculates the geometric length of the segment.
     * For arcs, calculates arc length. For linear, calculates straight distance.
     */
    double getLength() const {
        if (motionType_ == MotionType::Dwell || motionType_ == MotionType::ToolChange) {
            return 0.0;
        }

        if (isArcMotion(motionType_)) {
            return calculateArcLength();
        }

        // Linear or rapid
        Vec3 delta = endPoint_.getPosition() - startPoint_.getPosition();
        return delta.length();
    }

    /**
     * @brief Get estimated execution time
     * 
     * Estimates time based on length and feedrate.
     * For rapid moves, uses a default rapid rate.
     * 
     * @param defaultRapidRate Default rapid rate for rapid moves (units/min)
     */
    double getEstimatedTime(double defaultRapidRate = 10000.0) const {
        if (motionType_ == MotionType::Dwell) {
            return dwellDuration_;
        }

        if (motionType_ == MotionType::ToolChange) {
            return 5.0; // Estimate 5 seconds for tool change
        }

        double length = getLength();
        double rate = motionType_ == MotionType::Rapid ? defaultRapidRate : feedrate_;

        if (rate <= 0.0) {
            return 0.0;
        }

        return (length / rate) * 60.0; // Convert to seconds
    }

    /**
     * @brief Check if segment is valid
     */
    bool isValid() const {
        if (!startPoint_.isValid() || !endPoint_.isValid()) {
            return false;
        }

        // Check feedrate for cutting motions
        if (requiresFeedrate(motionType_) && feedrate_ <= 0.0) {
            return false;
        }

        // Check arc center for arc motions
        if (isArcMotion(motionType_)) {
            if (!arcCenter_.has_value()) {
                return false;
            }
        }

        return true;
    }

    /**
     * @brief Check if segment is zero-length
     */
    bool isZeroLength() const {
        if (motionType_ == MotionType::Dwell || motionType_ == MotionType::ToolChange) {
            return false; // Dwell and tool change are valid at a point
        }

        Vec3 delta = endPoint_.getPosition() - startPoint_.getPosition();
        return delta.lengthSquared() < 1e-12;
    }

private:
    /**
     * @brief Private constructor
     */
    ToolpathSegment(
        MotionType motionType,
        const ToolpathPoint& start,
        const ToolpathPoint& end,
        const std::optional<Vec3>& arcCenter,
        ArcPlane plane,
        double feedrate,
        const std::string& comment
    ) : motionType_(motionType),
        startPoint_(start),
        endPoint_(end),
        arcCenter_(arcCenter),
        arcPlane_(plane),
        feedrate_(feedrate),
        comment_(comment),
        dwellDuration_(0.0),
        toolNumber_(0) {
    }

    /**
     * @brief Calculate arc length
     */
    double calculateArcLength() const {
        if (!arcCenter_.has_value()) {
            return 0.0;
        }

        Vec3 start = startPoint_.getPosition();
        Vec3 end = endPoint_.getPosition();
        Vec3 center = arcCenter_.value();

        // Calculate radius
        Vec3 startVec = start - center;
        Vec3 endVec = end - center;
        double radius = startVec.length();

        if (radius < 1e-9) {
            return 0.0;
        }

        // Calculate angle
        double dot = (startVec.x * endVec.x + startVec.y * endVec.y + startVec.z * endVec.z) / (radius * radius);
        dot = std::max(-1.0, std::min(1.0, dot)); // Clamp to [-1, 1]
        double angle = std::acos(dot);

        // Arc length = radius * angle
        return radius * angle;
    }

    MotionType motionType_;
    ToolpathPoint startPoint_;
    ToolpathPoint endPoint_;
    std::optional<Vec3> arcCenter_;  ///< Arc center (for arc motions)
    ArcPlane arcPlane_;              ///< Arc plane (for arc motions)
    double feedrate_;                 ///< Feedrate (units/min)
    std::string comment_;             ///< Comment string
    double dwellDuration_;             ///< Dwell duration in seconds
    int toolNumber_;                  ///< Tool number for tool change
};

} // namespace cnc
