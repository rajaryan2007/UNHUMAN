#include "Animator.h"
#include <glm/gtx/matrix_decompose.hpp>

namespace UHE::RD3d {

Animator::Animator(Ref<Model> model)
    : m_Model(model)
{
    if (m_Model && !m_Model->GetSkeleton().Bones.empty())
    {
        m_GlobalTransforms.resize(m_Model->GetSkeleton().Bones.size(), glm::mat4(1.0f));
        m_FinalBoneMatrices.resize(m_Model->GetSkeleton().JointNodes.size(), glm::mat4(1.0f));
    }
}

void Animator::PlayAnimation(const std::string& name)
{
    if (!m_Model) return;
    const auto& animations = m_Model->GetAnimations();
    for (int i = 0; i < animations.size(); ++i)
    {
        if (animations[i].Name == name)
        {
            m_CurrentAnimationIndex = i;
            m_CurrentTime = 0.0f;
            return;
        }
    }
}

void Animator::PlayAnimation(int index)
{
    if (!m_Model) return;
    const auto& animations = m_Model->GetAnimations();
    if (index >= 0 && index < animations.size())
    {
        m_CurrentAnimationIndex = index;
        m_CurrentTime = 0.0f;
    }
}

void Animator::UpdateAnimation(float dt)
{
    if (!m_Model || m_CurrentAnimationIndex == -1 || m_CurrentAnimationIndex >= m_Model->GetAnimations().size() || m_Model->GetSkeleton().RootBoneID == -1)
        return;

    const auto& skeleton = m_Model->GetSkeleton();
    if (m_GlobalTransforms.size() != skeleton.Bones.size())
        m_GlobalTransforms.resize(skeleton.Bones.size(), glm::mat4(1.0f));
    
    if (m_FinalBoneMatrices.size() != skeleton.JointNodes.size())
        m_FinalBoneMatrices.resize(skeleton.JointNodes.size(), glm::mat4(1.0f));

    const auto& currentAnimation = m_Model->GetAnimations()[m_CurrentAnimationIndex];

    m_CurrentTime += dt;
    if (m_CurrentTime > currentAnimation.Duration)
    {
        m_CurrentTime = fmod(m_CurrentTime, currentAnimation.Duration);
    }

    CalculateBoneTransform(m_Model->GetSkeleton(), m_Model->GetSkeleton().RootBoneID, glm::mat4(1.0f));

    for (size_t i = 0; i < m_Model->GetSkeleton().JointNodes.size(); ++i) {
        int boneID = m_Model->GetSkeleton().JointNodes[i];
        m_FinalBoneMatrices[i] = m_GlobalTransforms[boneID] * m_Model->GetSkeleton().Bones[boneID].InverseBindMatrix;
    }
}

void Animator::CalculateBoneTransform(const Skeleton& skeleton, int boneID, const glm::mat4& parentTransform)
{
    const Bone& bone = skeleton.Bones[boneID];
    glm::mat4 nodeTransform = bone.LocalTransform;

    if (m_CurrentAnimationIndex != -1)
    {
        const auto& currentAnimation = m_Model->GetAnimations()[m_CurrentAnimationIndex];

        // Try to find tracks for this bone
        bool hasPosition = false;
        bool hasRotation = false;
        bool hasScale = false;

        glm::vec3 position;
        glm::quat rotation;
        glm::vec3 scale;

        for (const auto& track : currentAnimation.PositionTracks) {
            if (track.TargetBoneID == boneID) {
                position = InterpolatePosition(m_CurrentTime, track);
                hasPosition = true;
                break;
            }
        }

        for (const auto& track : currentAnimation.RotationTracks) {
            if (track.TargetBoneID == boneID) {
                rotation = InterpolateRotation(m_CurrentTime, track);
                hasRotation = true;
                break;
            }
        }

        for (const auto& track : currentAnimation.ScaleTracks) {
            if (track.TargetBoneID == boneID) {
                scale = InterpolateScale(m_CurrentTime, track);
                hasScale = true;
                break;
            }
        }

        if (hasPosition || hasRotation || hasScale)
        {
            // Fallback to local transform components if some tracks are missing
            glm::vec3 localPos, localScale, skew;
            glm::vec4 perspective;
            glm::quat localRot;
            glm::decompose(nodeTransform, localScale, localRot, localPos, skew, perspective);

            if (!hasPosition) position = localPos;
            if (!hasRotation) rotation = localRot;
            if (!hasScale) scale = localScale;

            nodeTransform = glm::translate(glm::mat4(1.0f), position) * glm::mat4_cast(rotation) * glm::scale(glm::mat4(1.0f), scale);
        }
    }

    glm::mat4 globalTransform = parentTransform * nodeTransform;
    m_GlobalTransforms[boneID] = globalTransform;

    // Traverse children
    for (size_t i = 0; i < skeleton.Bones.size(); ++i)
    {
        if (skeleton.Bones[i].ParentID == boneID)
        {
            CalculateBoneTransform(skeleton, skeleton.Bones[i].ID, globalTransform);
        }
    }
}

glm::vec3 Animator::InterpolatePosition(float time, const VectorTrack& track)
{
    if (track.Keyframes.size() == 1) return track.Keyframes[0].Value;

    for (size_t i = 0; i < track.Keyframes.size() - 1; ++i)
    {
        if (time < track.Keyframes[i + 1].Time)
        {
            float scale = (time - track.Keyframes[i].Time) / (track.Keyframes[i + 1].Time - track.Keyframes[i].Time);
            return glm::mix(track.Keyframes[i].Value, track.Keyframes[i + 1].Value, scale);
        }
    }
    return track.Keyframes.back().Value;
}

glm::quat Animator::InterpolateRotation(float time, const QuaternionTrack& track)
{
    if (track.Keyframes.size() == 1) return track.Keyframes[0].Value;

    for (size_t i = 0; i < track.Keyframes.size() - 1; ++i)
    {
        if (time < track.Keyframes[i + 1].Time)
        {
            float scale = (time - track.Keyframes[i].Time) / (track.Keyframes[i + 1].Time - track.Keyframes[i].Time);
            return glm::slerp(track.Keyframes[i].Value, track.Keyframes[i + 1].Value, scale);
        }
    }
    return track.Keyframes.back().Value;
}

glm::vec3 Animator::InterpolateScale(float time, const VectorTrack& track)
{
    if (track.Keyframes.size() == 1) return track.Keyframes[0].Value;

    for (size_t i = 0; i < track.Keyframes.size() - 1; ++i)
    {
        if (time < track.Keyframes[i + 1].Time)
        {
            float scale = (time - track.Keyframes[i].Time) / (track.Keyframes[i + 1].Time - track.Keyframes[i].Time);
            return glm::mix(track.Keyframes[i].Value, track.Keyframes[i + 1].Value, scale);
        }
    }
    return track.Keyframes.back().Value;
}

} // namespace UHE::RD3d
