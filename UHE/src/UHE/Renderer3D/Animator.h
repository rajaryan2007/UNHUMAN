#pragma once
#include <vector>
#include "UHE/Core/Core.h"
#include "UHE/Renderer3D/Animation.h"
#include "UHE/Renderer3D/LoadModel.h"

namespace UHE::RD3d
{

class UHE_API Animator
{
public:
    Animator() = default;
    Animator(Ref<Model> model);

    void UpdateAnimation(float dt);
    void PlayAnimation(const std::string& name);
    void PlayAnimation(int index);

    const std::vector<glm::mat4>& GetFinalBoneMatrices() const { return m_FinalBoneMatrices; }
    Ref<Model> GetModel() const { return m_Model; }
    bool HasAnimation() const { return m_CurrentAnimationIndex != -1; }

private:
    void CalculateBoneTransform(const Skeleton& skeleton, int boneID, const glm::mat4& parentTransform);

    glm::vec3 InterpolatePosition(float time, const VectorTrack& track);
    glm::quat InterpolateRotation(float time, const QuaternionTrack& track);
    glm::vec3 InterpolateScale(float time, const VectorTrack& track);

private:
    Ref<Model> m_Model;
    int m_CurrentAnimationIndex = -1;
    float m_CurrentTime = 0.0f;
    std::vector<glm::mat4> m_GlobalTransforms;
    std::vector<glm::mat4> m_FinalBoneMatrices;
};

} // namespace UHE::RD3d
