// Skin and animation parsing.
//
// Separate from the scene walk because it answers a different question: the
// walk places geometry, this builds the deformation data that Animator applies
// on top. Split out so the geometry path recompiles independently of it.

#include "uhepch.h"
#include "LoadModel.h"
#include "MeshoptDecode.h"
#include "fastgltf/math.hpp"
#include "fastgltf/tools.hpp"
#include "fastgltf/types.hpp"

namespace UHE::RD3d
{

namespace
{

// Same rationale as LoadModelGeometry.cpp: meshopt-compressed buffer views
// need on-the-fly decoding, and animations are commonly compressed in meshopt
// exports because their keyframe streams dominate file size.
struct MeshoptBufferDataAdapter
{
    const fastgltf::Asset* asset = nullptr;
    std::shared_ptr<std::vector<std::byte>> scratch = std::make_shared<std::vector<std::byte>>();

    std::span<const std::byte> operator()(const fastgltf::Asset& a, std::size_t bufferViewIndex) const
    {
        return GetBufferViewBytes(a, bufferViewIndex, *scratch);
    }
};

} // namespace

void Model::ParseSkins(const fastgltf::Asset& asset)
{
    if (asset.skins.empty())
        return;

    // Extract unified bone hierarchy across all nodes; individual skins bind
    // specific joint subsets and inverse bind matrices.
    m_Skeleton.Bones.resize(asset.nodes.size());
    m_Skeleton.RootBoneID = asset.skins[0].skeleton.has_value()
                                ? static_cast<int>(asset.skins[0].skeleton.value())
                                : (asset.skins[0].joints.empty() ? -1 : static_cast<int>(asset.skins[0].joints[0]));

    // 1. Build hierarchy mapping from asset.nodes
    for (size_t i = 0; i < asset.nodes.size(); ++i)
    {
        const auto& node = asset.nodes[i];
        m_Skeleton.Bones[i].ID = static_cast<int>(i);
        m_Skeleton.Bones[i].Name = node.name.empty() ? "Bone_" + std::to_string(i) : std::string(node.name);

        m_Skeleton.Bones[i].LocalTransform = NodeLocalTransform(node);

        // Set parent
        for (auto childIdx : node.children)
        {
            m_Skeleton.Bones[childIdx].ParentID = static_cast<int>(i);
        }
    }

    // 2. Every skin: joint list + inverse bind matrices.
    for (size_t s = 0; s < asset.skins.size(); ++s)
    {
        const auto& gltfSkin = asset.skins[s];
        Skin skin;
        skin.Name = gltfSkin.name.empty() ? "Skin_" + std::to_string(s) : std::string(gltfSkin.name);
        skin.JointNodes.clear();
        skin.JointNodes.reserve(gltfSkin.joints.size());
        for (auto joint : gltfSkin.joints)
            skin.JointNodes.push_back(static_cast<int>(joint));
        skin.InverseBindMatrices.resize(gltfSkin.joints.size(), glm::mat4(1.0f));

        if (gltfSkin.inverseBindMatrices.has_value())
        {
            auto& ibmAccessor = asset.accessors[gltfSkin.inverseBindMatrices.value()];
            size_t jointIdx = 0;
            fastgltf::iterateAccessor<fastgltf::math::fmat4x4>(asset, ibmAccessor,
                                                               [&](const fastgltf::math::fmat4x4& matrix)
                                                               {
                                                                   if (jointIdx < skin.InverseBindMatrices.size())
                                                                   {
                                                                       memcpy(&skin.InverseBindMatrices[jointIdx],
                                                                              matrix.data(), sizeof(glm::mat4));
                                                                   }
                                                                   jointIdx++;
                                                               }, MeshoptBufferDataAdapter{&asset});
        }

        // The first skin also feeds the legacy Skeleton fields so existing
        // call sites (GetSkeleton) keep working unchanged.
        if (s == 0)
        {
            m_Skeleton.JointNodes = skin.JointNodes;
            for (size_t j = 0; j < skin.JointNodes.size(); ++j)
                m_Skeleton.Bones[skin.JointNodes[j]].InverseBindMatrix = skin.InverseBindMatrices[j];
        }

        m_Skins.push_back(std::move(skin));
    }
}

// Returns the track already targeting this bone, or appends a new one. glTF
// may emit several channels for the same (bone, path) pair; merging them
// keeps the sampled pose well-defined.
template <typename TrackT>
static TrackT* FindOrAddTrack(std::vector<TrackT>& tracks, int targetBone)
{
    for (auto& track : tracks)
        if (track.TargetBoneID == targetBone)
            return &track;
    TrackT& track = tracks.emplace_back();
    track.TargetBoneID = targetBone;
    return &track;
}

// CUBICSPLINE output layout per glTF 2.0: for key k the accessor stores
// [inTangent_k, value_k, outTangent_k]; tangents are scaled by the segment
// duration at evaluation time.
template <typename T, typename GltfT, typename ToGlm>
static void AppendTrackKeys(const fastgltf::Asset& asset, const fastgltf::Accessor& valueAccessor,
                            const std::vector<float>& times, InterpolationMode interpolation,
                            std::vector<Keyframe<T>>& keys, std::vector<T>& inTangents,
                            std::vector<T>& outTangents, ToGlm toGlm)
{
    const bool spline = interpolation == InterpolationMode::CubicSpline;
    if (spline)
    {
        keys.resize(times.size());
        inTangents.resize(times.size());
        outTangents.resize(times.size());
        fastgltf::iterateAccessorWithIndex<GltfT>(asset, valueAccessor,
                                                  [&](GltfT v, size_t i)
                                                  {
                                                      const size_t key = i / 3;
                                                      if (key >= times.size())
                                                          return;
                                                      const size_t part = i % 3; // 0 in, 1 value, 2 out
                                                      if (part == 0)
                                                          inTangents[key] = toGlm(v);
                                                      else if (part == 1)
                                                          keys[key] = {times[key], toGlm(v)};
                                                      else
                                                          outTangents[key] = toGlm(v);
                                                  }, MeshoptBufferDataAdapter{&asset});
    }
    else
    {
        size_t idx = 0;
        fastgltf::iterateAccessor<GltfT>(asset, valueAccessor,
                                         [&](GltfT v)
                                         {
                                             if (idx < times.size())
                                                  keys.push_back({times[idx], toGlm(v)});
                                             ++idx;
                                         }, MeshoptBufferDataAdapter{&asset});
    }
}

void Model::ParseAnimations(const fastgltf::Asset& asset)
{
    if (asset.animations.empty())
        return;

    for (const auto& gltfAnim : asset.animations)
    {
        AnimationClip clip;
        clip.Name = gltfAnim.name.empty() ? "Anim_" + std::to_string(m_Animations.size()) : std::string(gltfAnim.name);

        for (const auto& channel : gltfAnim.channels)
        {
            if (!channel.nodeIndex.has_value())
                continue;
            int targetNode = static_cast<int>(channel.nodeIndex.value());

            if (channel.samplerIndex >= gltfAnim.samplers.size())
                continue;

            const auto& sampler = gltfAnim.samplers[channel.samplerIndex];

            // NOTE: Map sampler interpolation explicitly per track (Step, Linear, or CubicSpline).
            InterpolationMode interpolation = InterpolationMode::Linear;
            if (sampler.interpolation == fastgltf::AnimationInterpolation::Step)
                interpolation = InterpolationMode::Step;
            else if (sampler.interpolation == fastgltf::AnimationInterpolation::CubicSpline)
                interpolation = InterpolationMode::CubicSpline;

            // Extract times (one per key, also for CUBICSPLINE).
            std::vector<float> times;
            auto& timeAccessor = asset.accessors[sampler.inputAccessor];
            fastgltf::iterateAccessor<float>(asset, timeAccessor,
                                             [&](float t)
                                             {
                                                 times.push_back(t);
                                                 clip.Duration = std::max(clip.Duration, t);
                                             }, MeshoptBufferDataAdapter{&asset});
            if (times.empty())
                continue;

            auto& valueAccessor = asset.accessors[sampler.outputAccessor];

            if (channel.path == fastgltf::AnimationPath::Translation)
            {
                VectorTrack* track = FindOrAddTrack(clip.PositionTracks, targetNode);
                track->Interpolation = interpolation;
                AppendTrackKeys<glm::vec3, fastgltf::math::fvec3>(
                    asset, valueAccessor, times, interpolation, track->Keyframes, track->InTangents,
                    track->OutTangents,
                    [](const fastgltf::math::fvec3& v) { return glm::vec3(v.x(), v.y(), v.z()); });
            }
            else if (channel.path == fastgltf::AnimationPath::Rotation)
            {
                QuaternionTrack* track = FindOrAddTrack(clip.RotationTracks, targetNode);
                track->Interpolation = interpolation;
                AppendTrackKeys<glm::quat, fastgltf::math::fvec4>(
                    asset, valueAccessor, times, interpolation, track->Keyframes, track->InTangents,
                    track->OutTangents,
                    [](const fastgltf::math::fvec4& v)
                    {
                        // glTF rotation is x,y,z,w. GLM quat constructor takes w,x,y,z.
                        return glm::normalize(glm::quat(v.w(), v.x(), v.y(), v.z()));
                    });
            }
            else if (channel.path == fastgltf::AnimationPath::Scale)
            {
                VectorTrack* track = FindOrAddTrack(clip.ScaleTracks, targetNode);
                track->Interpolation = interpolation;
                AppendTrackKeys<glm::vec3, fastgltf::math::fvec3>(
                    asset, valueAccessor, times, interpolation, track->Keyframes, track->InTangents,
                    track->OutTangents,
                    [](const fastgltf::math::fvec3& v) { return glm::vec3(v.x(), v.y(), v.z()); });
            }
        }

        m_Animations.push_back(clip);
    }
}
} // namespace UHE::RD3d
