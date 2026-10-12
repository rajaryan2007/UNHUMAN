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
    void UpdateAnimation(float dt);
    void PlayAnimation(const std::string& name);
    void PlayAnimation(int index);
    void Play();
    void Pause();
    void Stop();
    bool IsPlaying() const { return m_IsPlaying; }

    void SetTimeScale(float scale) { m_TimeScale = scale; }
    float GetTimeScale() const { return m_TimeScale; }
    void SetLoopMode(LoopMode mode) { m_LoopMode = mode; }
    LoopMode GetLoopMode() const { return m_LoopMode; }
    void SetReversed(bool reversed) { m_Reversed = reversed; }
    bool IsReversed() const { return m_Reversed; }
    float GetCurrentTime() const { return m_CurrentTime; }
    float GetDuration() const;
    float GetNormalizedTime() const; ///< Normalized [0, 1] playback progress

    /**
     * @brief Jumps playhead to specified timestamp and immediately computes evaluated pose.
     * Useful for scrub controls and paused previewing.
     */
    void Seek(float time);

    // --- Cross-Fade Blending ---
    /**
     * @brief Smoothly blends from current active animation into target animation.
     * @param duration Transition duration in seconds.
     */
    void CrossFade(const std::string& name, float duration);
    void CrossFade(int index, float duration);
    bool IsBlending() const { return m_PrevAnimationIndex != -1; }
    float GetBlendWeight() const { return m_BlendWeight; } ///< 0 = source, 1 = target
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
    void SetSkin(int index);
    int GetSkinIndex() const { return m_SkinIndex; }
    const std::vector<glm::mat4>& GetFinalBoneMatrices() const { return m_FinalBoneMatrices; }
    Ref<Model> GetModel() const { return m_Model; }
    bool HasAnimation() const { return m_CurrentAnimationIndex != -1; }
    int GetCurrentAnimationIndex() const { return m_CurrentAnimationIndex; }

private:
    void SampleClip(const AnimationClip& clip, float time, AnimationPose& pose) const;
    static void BlendPoses(const AnimationPose& from, const AnimationPose& to, float weight, AnimationPose& out);
    void CalculateBoneTransform(const Skeleton& skeleton, int boneID, const glm::mat4& parentTransform);
    // Fires events whose time was crossed between prevTime and newTime,
    // honouring play direction and loop wraps.
    void FireEvents(const AnimationClip& clip, float prevTime, float newTime, bool movedForward);
    // Extracts the root joint's translation change between two sample times.
    glm::vec3 ExtractRootMotion(const AnimationClip& clip, float prevTime, float newTime, bool wrapped) const;

    glm::vec3 SamplePosition(float time, const VectorTrack& track) const;
    glm::quat SampleRotation(float time, const QuaternionTrack& track) const;
    glm::vec3 SampleScale(float time, const VectorTrack& track) const;
    // Hermite evaluation for CubicSpline tracks.
    template <typename T>
    static T Hermite(const Keyframe<T>& a, const Keyframe<T>& b, const T& inTangent, const T& outTangent, float time);

    void BuildChildLists();
    // Multiplies the global transforms with the selected skin's inverse binds.
    void ComputeFinalMatrices();

private:
    Ref<Model> m_Model;
    int m_CurrentAnimationIndex = -1;
    float m_CurrentTime = 0.0f;
    bool m_IsPlaying = true;
    float m_TimeScale = 1.0f;
    LoopMode m_LoopMode = LoopMode::Loop;
    bool m_Reversed = false;
    bool m_PingPongForward = true; // bounce direction for LoopMode::PingPong

    int m_SkinIndex = 0;

    // Cross-fade state: m_PrevAnimationIndex >= 0 while a fade is running.
    int m_PrevAnimationIndex = -1;
    float m_PrevTime = 0.0f;
    float m_BlendDuration = 0.0f;
    float m_BlendTime = 0.0f;
    float m_BlendWeight = 1.0f;

    AnimationPose m_PrevPose;
    AnimationPose m_CurrentPose;
    AnimationPose m_BlendedPose;

    EventCallback m_EventCallback;

    bool m_RootMotionEnabled = false;
    glm::vec3 m_RootMotionDelta{0.0f};

    std::vector<std::vector<int>> m_ChildBones; // per bone: child bone IDs
    std::vector<glm::mat4> m_GlobalTransforms;
    std::vector<glm::mat4> m_FinalBoneMatrices;
};

} // namespace UHE::RD3d
