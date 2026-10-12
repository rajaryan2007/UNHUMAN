#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "UHE/Core/Core.h"

namespace UHE::RD3d
{

/**
 * @brief Output rotations computed by two-bone analytic inverse kinematics.
 */
struct UHE_API TwoBoneIKResult
{
    bool Solved = false;
    glm::quat RootRotation{1.0f, 0.0f, 0.0f, 0.0f}; ///< Global rotation adjustment for root bone
    glm::quat MidRotation{1.0f, 0.0f, 0.0f, 0.0f};  ///< Global rotation adjustment for middle bone (hinge)
};

/**
 * @brief Analytic inverse kinematics solver for 2-bone kinematic chains (e.g., arms, legs).
 */
class UHE_API IKSolver
{
public:
    /**
     * @brief Solves two-bone analytic IK targeting a 3D position with pole vector bending.
     *
     * Automatically clamps out-of-reach target distances to prevent geometric instability.
     *
     * @param rootPos World position of the root joint (e.g., shoulder/hip).
     * @param midPos World position of the hinge joint (e.g., elbow/knee).
     * @param endPos World position of the end-effector (e.g., wrist/ankle).
     * @param targetPos Desired world position for the end-effector.
     * @param poleVector Directional hint vector defining the hinge bending plane.
     * @return TwoBoneIKResult containing relative rotation corrections.
     */
    static TwoBoneIKResult SolveTwoBone(const glm::vec3& rootPos, const glm::vec3& midPos,
                                        const glm::vec3& endPos, const glm::vec3& targetPos,
                                        const glm::vec3& poleVector);
};

} // namespace UHE::RD3d
