#pragma once
#include <functional>
#include <vector>
#include "UHE/Core/Core.h"
#include "UHE/Renderer3D/Animation.h"
#include "UHE/Renderer3D/LoadModel.h"

namespace UHE::RD3d
{

/**
 * @brief Skeletal animation playback runtime and pose evaluation engine.
 *
 * Features:
 * - Multi-track keyframe sampling (Step, Linear, CubicSpline).
 * - Cross-fade clip blending with smooth Hermite weight curves.
 * - Timeline notify/event firing across playhead updates.
 * - Root motion extraction and translation detachment.
 * - Multi-skin mesh binding and bone matrix palette calculation.
 */
class UHE_API Animator
{
public:
    Animator() = default;
    Animator(Ref<Model> model);

    // --- Playback Controls ---
    void UpdateAnimation(f32 dt);
    void PlayAnimation(const std::string& name);
    void PlayAnimation(i32 index);
    void Play();
    void Pause();
    void Stop();
    bool IsPlaying() const { return m_IsPlaying; }

    void SetTimeScale(f32 scale) { m_TimeScale = scale; }
    f32 GetTimeScale() const { return m_TimeScale; }
    void SetLoopMode(LoopMode mode) { m_LoopMode = mode; }
    LoopMode GetLoopMode() const { return m_LoopMode; }
    void SetReversed(bool reversed) { m_Reversed = reversed; }
    bool IsReversed() const { return m_Reversed; }
    f32 GetCurrentTime() const { return m_CurrentTime; }
    f32 GetDuration() const;
    f32 GetNormalizedTime() const; ///< Normalized [0, 1] playback progress

    /**
     * @brief Jumps playhead to specified timestamp and immediately computes evaluated pose.
     * Useful for scrub controls and paused previewing.
     */
    void Seek(f32 time);

    // --- Cross-Fade Blending ---
    /**
     * @brief Smoothly blends from current active animation into target animation.
     * @param duration Transition duration in seconds.
     */
    void CrossFade(const std::string& name, f32 duration);
    void CrossFade(i32 index, f32 duration);
    bool IsBlending() const { return m_PrevAnimationIndex != -1; }
    f32 GetBlendWeight() const { return m_BlendWeight; } ///< 0 = source, 1 = target
    // ---- events / notifies ----
    using EventCallback = std::function<void(const AnimationEvent&)>;
    void SetEventCallback(EventCallback callback) { m_EventCallback = std::move(callback); }

    // --- Root Motion ---
    /**
     * @brief Enables root motion stripping from bone pose to drive entity movement.
     */
    void SetRootMotionEnabled(bool enable) { m_RootMotionEnabled = enable; }
    bool IsRootMotionEnabled() const { return m_RootMotionEnabled; }
    const glm::vec3& GetRootMotionDelta() const { return m_RootMotionDelta; }

    // --- Skin Selection ---
    /**
     * @brief Selects active skin index for multi-skin skinned mesh palettes.
     */
    void SetSkin(i32 index);
    i32 GetSkinIndex() const { return m_SkinIndex; }
    const std::vector<glm::mat4>& GetFinalBoneMatrices() const { return m_FinalBoneMatrices; }
    Ref<Model> GetModel() const { return m_Model; }
    bool HasAnimation() const { return m_CurrentAnimationIndex != -1; }
    i32 GetCurrentAnimationIndex() const { return m_CurrentAnimationIndex; }

private:
    void SampleClip(const AnimationClip& clip, f32 time, AnimationPose& pose) const;
    static void BlendPoses(const AnimationPose& from, const AnimationPose& to, f32 weight, AnimationPose& out);
    void CalculateBoneTransform(const Skeleton& skeleton, i32 boneID, const glm::mat4& parentTransform);
    // Fires events whose time was crossed between prevTime and newTime,
    // honouring play direction and loop wraps.
    void FireEvents(const AnimationClip& clip, f32 prevTime, f32 newTime, bool movedForward);
    // Extracts the root joint's translation change between two sample times.
    glm::vec3 ExtractRootMotion(const AnimationClip& clip, f32 prevTime, f32 newTime, bool wrapped, bool movedForward) const;
    glm::vec3 SamplePosition(f32 time, const VectorTrack& track) const;
    glm::quat SampleRotation(f32 time, const QuaternionTrack& track) const;
    glm::vec3 SampleScale(f32 time, const VectorTrack& track) const;
    // Hermite evaluation for CubicSpline tracks.
    template <typename T>
    static T Hermite(const Keyframe<T>& a, const Keyframe<T>& b, const T& inTangent, const T& outTangent, f32 time);
    void BuildChildLists();
    // Multiplies the global transforms with the selected skin's inverse binds.
    void ComputeFinalMatrices();
    void TraverseSkeletonRoots();
private:
    Ref<Model> m_Model;
    i32 m_CurrentAnimationIndex = -1;
    f32 m_CurrentTime = 0.0f;
    bool m_IsPlaying = true;
    f32 m_TimeScale = 1.0f;
    LoopMode m_LoopMode = LoopMode::Loop;
    bool m_Reversed = false;
    bool m_PingPongForward = true; // bounce direction for LoopMode::PingPong

    i32 m_SkinIndex = 0;
    // Cross-fade state: m_PrevAnimationIndex >= 0 while a fade is running.
    i32 m_PrevAnimationIndex = -1;
    f32 m_PrevTime = 0.0f;
    f32 m_BlendDuration = 0.0f;
    f32 m_BlendTime = 0.0f;
    f32 m_BlendWeight = 1.0f;

    AnimationPose m_RestPose;
    AnimationPose m_PrevPose;
    AnimationPose m_CurrentPose;
    AnimationPose m_BlendedPose;
    EventCallback m_EventCallback;

    bool m_RootMotionEnabled = false;
    glm::vec3 m_RootMotionDelta{0.0f};

    std::vector<std::vector<i32>> m_ChildBones; // per bone: child bone IDs
    std::vector<glm::mat4> m_GlobalTransforms;
    std::vector<glm::mat4> m_FinalBoneMatrices;
};

} // namespace UHE::RD3d
