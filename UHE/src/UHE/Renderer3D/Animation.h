#pragma once
#include "UHE/Core/Core.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>
#include <vector>

namespace UHE::RD3d {

struct Bone {
    std::string Name;
    int ID = -1;
    int ParentID = -1;
    glm::mat4 InverseBindMatrix{1.0f};
    glm::mat4 LocalTransform{1.0f};
};

struct Skeleton {
    std::vector<Bone> Bones;
    std::vector<int> JointNodes;
    int RootBoneID = -1;
};

template <typename T>
struct Keyframe {
    float Time;
    T Value;
};

struct VectorTrack {
    int TargetBoneID = -1;
    std::vector<Keyframe<glm::vec3>> Keyframes;
};

struct QuaternionTrack {
    int TargetBoneID = -1;
    std::vector<Keyframe<glm::quat>> Keyframes;
};

struct AnimationClip {
    std::string Name;
    float Duration = 0.0f;
    
    std::vector<VectorTrack> PositionTracks;
    std::vector<QuaternionTrack> RotationTracks;
    std::vector<VectorTrack> ScaleTracks;
};

} // namespace UHE::RD3d
