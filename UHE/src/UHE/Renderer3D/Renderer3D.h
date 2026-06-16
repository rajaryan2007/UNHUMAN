#pragma once
#include "UHE/Core/Core.h"
#include "UHE/Renderer3D/LoadModel.h"
#include "UHE/Renderer/EditorCamera.h"
#include "UHE/Renderer/Camera.h"
#include <glm/glm.hpp>

namespace UHE
{
class UHE_API Renderer3D
{
public:
    static void Init();
    static void Shutdown();

    static void BeginScene(const EditorCamera& camera);
    static void BeginScene(const Camera& camera, const glm::mat4& transform);
    static void EndScene();

    static void SubmitModel(const RD3d::Model& model, const glm::mat4& transform = glm::mat4(1.0f), int entityID = -1);
    
    static void DrawGrid();
};
} // namespace UHE
