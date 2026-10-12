#pragma once
#include <cstddef>
#include "UHE/Core/Core.h"
#include "UHE/Renderer3D/Animation.h"

namespace UHE::Jobsystem
{
class UheJobsystem;
}

namespace UHE::RD3d
{

/**
 * @brief Keyframe reduction settings based on chordal error tolerances.
 *
 * Intermediate keyframes are culled if linear interpolation between surviving neighbors
 * produces an error strictly within the configured thresholds.
 * - Step tracks are preserved verbatim.
 * - Rotations are evaluated via angular difference in radians.
 * - Translations and scales are evaluated via Euclidean distance.
 */
struct UHE_API CompressSettings
{
    f32 PositionTolerance = 1e-3f; // linear units
    f32 ScaleTolerance = 1e-4f;    // scale units
    f32 RotationTolerance = 0.02f; // radians (~1.15 degrees)
};

struct UHE_API CompressionStats
{
    size_t KeysBefore = 0;
    size_t KeysAfter = 0;
    size_t TracksProcessed = 0;

    // Estimated payload: 16 B per vector key, 20 B per quaternion key.
    size_t BytesBefore = 0;
    size_t BytesAfter = 0;

    f32 KeyReduction() const
    {
        return KeysBefore > 0 ? 1.0f - static_cast<f32>(KeysAfter) / static_cast<f32>(KeysBefore) : 0.0f;
    }
    f32 ByteReduction() const
    {
        return BytesBefore > 0 ? 1.0f - static_cast<f32>(BytesAfter) / static_cast<f32>(BytesBefore) : 0.0f;
    }
};

/**
 * @brief In-place animation keyframe compressor and memory profiler.
 */
class UHE_API AnimationCompressor
{
public:
    /**
     * @brief Optimizes keyframe counts in-place.
     * @param clip Animation clip to compress.
     * @param settings Error tolerances.
     * @param jobs Optional JobSystem to parallelize per-track reduction passes.
     * @return Statistics describing key and memory reduction.
     */
    static CompressionStats CompressClip(AnimationClip& clip, const CompressSettings& settings = {},
                                         Jobsystem::UheJobsystem* jobs = nullptr);

    /**
     * @brief Measures potential compression gains without modifying the clip.
     */
    static CompressionStats MeasureClip(const AnimationClip& clip, const CompressSettings& settings = {});
};

} // namespace UHE::RD3d
