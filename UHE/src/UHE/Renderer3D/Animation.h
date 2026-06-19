#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>
#include <vector>
#include "UHE/Core/Core.h"

namespace UHE::RD3d
{

struct UHE_API Bone
{
    std::string Name;
    int ID = -1;
    int ParentID = -1;
    glm::mat4 InverseBindMatrix{1.0f};
    glm::mat4 LocalTransform{1.0f};
};

struct UHE_API Skeleton
{
    std::vector<Bone> Bones;
    std::vector<int> JointNodes;
    int RootBoneID = -1;
};

template <typename T> struct UHE_API Keyframe
{
    float Time;
    T Value;
};

struct UHE_API VectorTrack
{
    int TargetBoneID = -1;
    std::vector<Keyframe<glm::vec3>> Keyframes;
};

struct UHE_API QuaternionTrack
{
    int TargetBoneID = -1;
    std::vector<Keyframe<glm::quat>> Keyframes;
};

struct UHE_API AnimationClip
{
    std::string Name;
    float Duration = 0.0f;

    std::vector<VectorTrack> PositionTracks;
    std::vector<QuaternionTrack> RotationTracks;
    std::vector<VectorTrack> ScaleTracks;
};

} // namespace UHE::RD3d
