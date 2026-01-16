#pragma once

#include "Types.h"
#include <cstdint>
#include <functional>
#include <string>
#include <cstring>

namespace cnc {

/**
 * @brief Deterministic random number generator
 * 
 * Simple linear congruential generator for deterministic randomness.
 * Suitable for simulation where reproducibility is required.
 */
class DeterministicRNG {
public:
    /**
     * @brief Construct with seed
     * @param seed Initial seed value
     */
    explicit DeterministicRNG(std::uint64_t seed = 0)
        : state_(seed == 0 ? 1 : seed) {
    }

    /**
     * @brief Generate next random value
     * @return Random value in [0, 2^64-1]
     */
    std::uint64_t next() {
        // Linear congruential generator (LCG)
        // Using constants from Numerical Recipes
        state_ = state_ * 1664525ULL + 1013904223ULL;
        return state_;
    }

    /**
     * @brief Generate random double in [0, 1)
     */
    double nextDouble() {
        return static_cast<double>(next()) / static_cast<double>(UINT64_MAX);
    }

    /**
     * @brief Generate random double in [min, max)
     */
    double nextDouble(double min, double max) {
        return min + (max - min) * nextDouble();
    }

    /**
     * @brief Get current state (seed)
     */
    std::uint64_t getState() const {
        return state_;
    }

    /**
     * @brief Set state (seed)
     */
    void setState(std::uint64_t seed) {
        state_ = seed == 0 ? 1 : seed;
    }

    /**
     * @brief Reset to initial seed
     */
    void reset(std::uint64_t seed) {
        setState(seed);
    }

private:
    std::uint64_t state_;
};

/**
 * @brief Hash function for simulation state
 * 
 * Provides a hash value for simulation state snapshots.
 * Useful for state comparison, caching, and RL state representation.
 */
class StateHasher {
public:
    /**
     * @brief Hash a 64-bit value
     */
    static std::uint64_t hash(std::uint64_t value) {
        // FNV-1a hash
        std::uint64_t hash = 14695981039346656037ULL;
        hash ^= value;
        hash *= 1099511628211ULL;
        return hash;
    }

    /**
     * @brief Hash a double value
     */
    static std::uint64_t hash(double value) {
        // Hash double by bit pattern
        std::uint64_t bits = 0;
        std::memcpy(&bits, &value, sizeof(double));
        return hash(bits);
    }

    /**
     * @brief Hash a Vec3
     */
    static std::uint64_t hash(const Vec3& v) {
        std::uint64_t h = hash(v.x);
        h = combine(h, hash(v.y));
        h = combine(h, hash(v.z));
        return h;
    }

    /**
     * @brief Combine two hash values
     */
    static std::uint64_t combine(std::uint64_t h1, std::uint64_t h2) {
        return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
    }

    /**
     * @brief Hash a sequence of values
     */
    template<typename Iterator>
    static std::uint64_t hashRange(Iterator begin, Iterator end) {
        std::uint64_t h = 14695981039346656037ULL;
        for (auto it = begin; it != end; ++it) {
            h = combine(h, hash(*it));
        }
        return h;
    }

private:
    static_assert(sizeof(double) == 8, "Double must be 8 bytes for deterministic hashing");
};

/**
 * @brief Reproducibility guard
 * 
 * Ensures deterministic execution by tracking and validating state.
 * Useful for debugging and RL environment verification.
 */
class ReproducibilityGuard {
public:
    /**
     * @brief Construct with initial seed
     */
    explicit ReproducibilityGuard(std::uint64_t seed = 0)
        : initialSeed_(seed),
          currentSeed_(seed),
          stepCount_(0) {
    }

    /**
     * @brief Get initial seed
     */
    std::uint64_t getInitialSeed() const {
        return initialSeed_;
    }

    /**
     * @brief Get current seed
     */
    std::uint64_t getCurrentSeed() const {
        return currentSeed_;
    }

    /**
     * @brief Get step count
     */
    std::uint64_t getStepCount() const {
        return stepCount_;
    }

    /**
     * @brief Advance to next step
     * 
     * Updates seed based on step count for deterministic progression.
     */
    void step() {
        ++stepCount_;
        // Update seed deterministically based on step count
        currentSeed_ = initialSeed_ + stepCount_;
    }

    /**
     * @brief Reset to initial state
     */
    void reset() {
        currentSeed_ = initialSeed_;
        stepCount_ = 0;
    }

    /**
     * @brief Reset with new seed
     */
    void reset(std::uint64_t seed) {
        initialSeed_ = seed;
        currentSeed_ = seed;
        stepCount_ = 0;
    }

    /**
     * @brief Get RNG instance for this guard
     */
    DeterministicRNG getRNG() const {
        return DeterministicRNG(currentSeed_);
    }

    /**
     * @brief Check if state matches expected values
     * 
     * Useful for verifying reproducibility across runs.
     */
    bool verify(std::uint64_t expectedSeed, std::uint64_t expectedStepCount) const {
        return currentSeed_ == expectedSeed && stepCount_ == expectedStepCount;
    }

private:
    std::uint64_t initialSeed_;
    std::uint64_t currentSeed_;
    std::uint64_t stepCount_;
};

} // namespace cnc
