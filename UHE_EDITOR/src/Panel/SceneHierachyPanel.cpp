#include "SceneHierachyPanel.h"
#include "glm/trigonometric.hpp"
#include "imgui/imgui_internal.h"
#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// todo divide them in other file as usage after 3d support
namespace UHE {
static constexpr ImVec4 kAccent{0.424f, 0.388f, 1.000f, 1.00f};
static constexpr ImVec4 kAccentHover{0.525f, 0.490f, 1.000f, 1.00f};
static constexpr ImVec4 kAccentActive{0.350f, 0.318f, 0.900f, 1.00f};
static constexpr ImVec4 kAccentMuted{0.424f, 0.388f, 1.000f, 0.15f};
static constexpr ImVec4 kAxisX{0.75f, 0.22f, 0.28f, 1.0f};
static constexpr ImVec4 kAxisXHover{0.85f, 0.32f, 0.35f, 1.0f};
static constexpr ImVec4 kAxisXActive{0.65f, 0.15f, 0.20f, 1.0f};
static constexpr ImVec4 kAxisY{0.30f, 0.68f, 0.35f, 1.0f};
static constexpr ImVec4 kAxisYHover{0.38f, 0.78f, 0.42f, 1.0f};
static constexpr ImVec4 kAxisYActive{0.22f, 0.58f, 0.28f, 1.0f};
static constexpr ImVec4 kAxisZ{0.25f, 0.42f, 0.82f, 1.0f};
static constexpr ImVec4 kAxisZHover{0.35f, 0.52f, 0.92f, 1.0f};
static constexpr ImVec4 kAxisZActive{0.18f, 0.32f, 0.72f, 1.0f};

SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene> &context) {
  SetContext(context);
}

void SceneHierarchyPanel::SetContext(const Ref<Scene> &context) {
  m_Context = context;
  m_SelectionContext = {};
}

// panel renderer st from here
void SceneHierarchyPanel::OnImGuiRender() {
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6, 8));
  ImGui::Begin("Scene Hierarchy");
  ImGuiIO &io = ImGui::GetIO();
  ImFont *boldFont = io.Fonts->Fonts.Size > 1 ? io.Fonts->Fonts[1] : nullptr;
  if (boldFont)
    ImGui::PushFont(boldFont);
  ImGui::TextColored(ImVec4(0.878f, 0.878f, 0.926f, 0.60f), "ENTITIES");
  if (boldFont)
    ImGui::PopFont();
  ImGui::Spacing();
  ImGui::Separator();
  ImGui::Spacing();
  auto view = m_Context->m_registry.view<TagComponent>();
  int rowIdx = 0;
  for (auto entityHandle : view) {
    Entity entity{entityHandle, m_Context.get()};
    if (rowIdx % 2 == 1) {
      ImVec2 cursorPos = ImGui::GetCursorScreenPos();
      ImVec2 regionAvail = ImGui::GetContentRegionAvail();
      float rowHeight =
          ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f + 2.0f;
      ImGui::GetWindowDrawList()->AddRectFilled(
          cursorPos,
          ImVec2(cursorPos.x + regionAvail.x, cursorPos.y + rowHeight),
          IM_COL32(255, 255, 255, 6), 0.0f);
    }
    DrawEntityNode(entity);
    rowIdx++;
  }
  if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered() &&
      !ImGui::IsAnyItemHovered())
    m_SelectionContext = {};
  if (ImGui::BeginPopupContextWindow("HierarchyContextMenu",
                                     ImGuiPopupFlags_MouseButtonRight |
                                         ImGuiPopupFlags_NoOpenOverItems)) {
    if (ImGui::MenuItem("  Create Empty Entity"))
      m_Context->CreateEntity("Empty Entity");
    ImGui::EndPopup();
  }
  ImGui::End();
  ImGui::PopStyleVar();
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
  ImGui::Begin("Properties");
  if (m_SelectionContext) {
    DrawComponents(m_SelectionContext);
  } else {
    ImGui::TextColored(ImVec4(0.376f, 0.376f, 0.627f, 1.0f),
                       "Select an entity to view properties");
  }
  ImGui::End();
  ImGui::PopStyleVar();
}

void SceneHierarchyPanel::SetSelectedEntity(Entity entity) {
  m_SelectionContext = entity;
}
void SceneHierarchyPanel::DrawEntityNode(Entity entity) {
  auto &tc = entity.GetComponent<TagComponent>();
  ImGui::PushID((uint32_t)entity);
  ImGuiTreeNodeFlags flags =
      ((m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) |
      ImGuiTreeNodeFlags_OpenOnArrow;
  flags |= ImGuiTreeNodeFlags_SpanFullWidth;
  if (m_SelectionContext == entity) {
    ImGui::PushStyleColor(ImGuiCol_Header, kAccentMuted);
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered,
                          ImVec4(0.424f, 0.388f, 1.000f, 0.25f));
  }
  bool opened = ImGui::TreeNodeEx(tc.Tag.c_str(), flags);
  if (m_SelectionContext == entity)
    ImGui::PopStyleColor(2);
  if (ImGui::IsItemClicked())
    m_SelectionContext = entity;
  bool entityDeleted = false;
  if (ImGui::BeginPopupContextItem()) {
    ImGui::TextColored(ImVec4(0.92f, 0.92f, 0.92f, 0.50f), "Entity Actions");
    ImGui::Separator();
    if (ImGui::MenuItem("  Delete Entity"))
      entityDeleted = true;
    ImGui::EndPopup();
  }
  if (opened)
    ImGui::TreePop();
  ImGui::PopID();
  if (entityDeleted) {
    m_Context->DestroyEntity(entity);
    if (m_SelectionContext == entity)
      m_SelectionContext = {};
  }
}
static void DrawVec3Control(const std::string &label, glm::vec3 &value,
                            f32 resetValue = 0.0f, f32 columeWidth = 100.0f) {
  ImGuiIO &io = ImGui::GetIO();
  ImFont *boldFont = io.Fonts->Fonts.Size > 1 ? io.Fonts->Fonts[1] : nullptr;
  ImGui::PushID(label.c_str());
  ImGui::Columns(2);
  ImGui::SetColumnWidth(0, columeWidth);
  ImGui::TextColored(ImVec4(0.878f, 0.878f, 0.926f, 0.70f), "%s",
                     label.c_str());
  ImGui::NextColumn();
  ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
  float lineHeight =
      ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
  ImVec2 buttonSize = {lineHeight + 2.0f, lineHeight};
  ImGui::PushStyleColor(ImGuiCol_Button, kAxisX);
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, kAxisXHover);
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, kAxisXActive);
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
  if (boldFont)
    ImGui::PushFont(boldFont);
  if (ImGui::Button("X", buttonSize))
    value.x = resetValue;
  if (boldFont)
    ImGui::PopFont();
  ImGui::PopStyleVar();
  ImGui::PopStyleColor(3);
  ImGui::SameLine();
  ImGui::DragFloat("##x", &value.x, 0.01f, 0.0f, 0.0f, "%.2f");
  ImGui::PopItemWidth();
  ImGui::SameLine();
  ImGui::PushStyleColor(ImGuiCol_Button, kAxisY);
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, kAxisYHover);
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, kAxisYActive);
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
  if (boldFont)
    ImGui::PushFont(boldFont);
  if (ImGui::Button("Y", buttonSize))
    value.y = resetValue;
  if (boldFont)
    ImGui::PopFont();
  ImGui::PopStyleVar();
  ImGui::PopStyleColor(3);
  ImGui::SameLine();
  ImGui::DragFloat("##y", &value.y, 0.01f, 0.0f, 0.0f, "%.2f");
  ImGui::PopItemWidth();
  ImGui::SameLine();
  ImGui::PushStyleColor(ImGuiCol_Button, kAxisZ);
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, kAxisZHover);
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, kAxisZActive);
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
  if (boldFont)
    ImGui::PushFont(boldFont);
  if (ImGui::Button("Z", buttonSize))
    value.z = resetValue;
  if (boldFont)
    ImGui::PopFont();
  ImGui::PopStyleVar();
  ImGui::PopStyleColor(3);
  ImGui::SameLine();
  ImGui::DragFloat("##z", &value.z, 0.01f, 0.0f, 0.0f, "%.2f");
  ImGui::PopItemWidth();
  ImGui::PopStyleVar();
  ImGui::Columns(1);
  ImGui::PopID();
}
const ImGuiTreeNodeFlags TreeNodeFlag =
    ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
    ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowOverlap |
    ImGuiTreeNodeFlags_FramePadding;
template <typename T, typename UIFunction>
static void DrawComponents(const std::string &name, Entity entity,
                           UIFunction function) {
  if (entity.HasComponent<T>()) {
    auto &components = entity.GetComponent<T>();
    ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{5, 5});
    float lineHeight =
        ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4.0f);
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Header,
                          ImVec4{0.165f, 0.165f, 0.340f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered,
                          ImVec4{0.200f, 0.200f, 0.420f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_HeaderActive,
                          ImVec4{0.240f, 0.240f, 0.500f, 1.0f});
    ImGuiIO &io = ImGui::GetIO();
    ImFont *boldFont = io.Fonts->Fonts.Size > 1 ? io.Fonts->Fonts[1] : nullptr;
    if (boldFont)
      ImGui::PushFont(boldFont);
    bool open = ImGui::TreeNodeEx((void *)typeid(T).hash_code(), TreeNodeFlag,
                                  name.c_str());
    if (boldFont)
      ImGui::PopFont();
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();
    ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
    ImGui::PushID(name.c_str());
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                          ImVec4(0.424f, 0.388f, 1.0f, 0.25f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                          ImVec4(0.424f, 0.388f, 1.0f, 0.40f));
    if (ImGui::Button("...", ImVec2{lineHeight, lineHeight})) {
      ImGui::OpenPopup("ComponentSettings");
    }
    ImGui::PopStyleColor(3);
    bool removeComponent = false;
    if (ImGui::BeginPopup("ComponentSettings")) {
      ImGui::TextColored(ImVec4(0.878f, 0.878f, 0.926f, 0.50f), "Component");
      ImGui::Separator();
      if (ImGui::MenuItem("  Remove Component"))
        removeComponent = true;
      ImGui::EndPopup();
    }
    ImGui::PopID();
    if (open) {
      ImGui::Spacing();
      function(components);
      ImGui::Spacing();
      ImGui::TreePop();
    }
    if (removeComponent)
      entity.RemoveComponent<T>();
  }
}
void SceneHierarchyPanel::DrawComponents(Entity entity) {
  if (entity.HasComponent<TagComponent>()) {
    auto &tag = entity.GetComponent<TagComponent>().Tag;
    char buffer[256];
    memset(buffer, 0, sizeof(buffer));
    strncpy(buffer, tag.c_str(), sizeof(buffer) - 1); buffer[sizeof(buffer) - 1] = '\0';
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 5));
    if (ImGui::InputText("##Tag", buffer, sizeof(buffer))) {
      tag = std::string(buffer);
    }
    ImGui::PopStyleVar(2);
  }
  ImGui::SameLine();
  ImGui::PushStyleColor(ImGuiCol_Button, kAccent);
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, kAccentHover);
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, kAccentActive);
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
  if (ImGui::Button("+ Add"))
    ImGui::OpenPopup("AddComponents");
  ImGui::PopStyleVar();
  ImGui::PopStyleColor(3);
  if (ImGui::BeginPopup("AddComponents")) {
    ImGui::TextColored(ImVec4(0.878f, 0.878f, 0.926f, 0.50f), "Components");
    ImGui::Separator();
    if (ImGui::MenuItem("  Camera")) {
      m_SelectionContext.AddComponent<CameraComponent>();
      ImGui::CloseCurrentPopup();
    }
    if (ImGui::MenuItem("  Sprite Renderer")) {
      m_SelectionContext.AddComponent<SpriteRendererComponent>();
      ImGui::CloseCurrentPopup();
    }
    if (ImGui::MenuItem("  Sprite Animation")) {
      m_SelectionContext.AddComponent<SpriteAnimationComponent>();
      ImGui::CloseCurrentPopup();
    }
    if (ImGui::MenuItem("  Rigid Body 2D")) {
      m_SelectionContext.AddComponent<RigidBody2DComponent>();
      ImGui::CloseCurrentPopup();
    }
    if (ImGui::MenuItem("  Box Collider 2D")) {
      m_SelectionContext.AddComponent<BoxColliderComponent>();
      ImGui::CloseCurrentPopup();
    }
    if (ImGui::MenuItem("  Model 3D")) {
      m_SelectionContext.AddComponent<Model3DComponent>();
      ImGui::CloseCurrentPopup();
    }
    if (ImGui::MenuItem("  Rigid Body 3D")) {
      m_SelectionContext.AddComponent<RigidBody3DComponent>();
      ImGui::CloseCurrentPopup();
    }
    if (ImGui::MenuItem("  Box Collider 3D")) {
      m_SelectionContext.AddComponent<BoxCollider3DComponent>();
      ImGui::CloseCurrentPopup();
    }
    if (ImGui::MenuItem("  Sphere Collider 3D")) {
      m_SelectionContext.AddComponent<SphereCollider3DComponent>();
      ImGui::CloseCurrentPopup();
    }
    if (ImGui::MenuItem("  Capsule Collider 3D")) {
      m_SelectionContext.AddComponent<CapsuleCollider3DComponent>();
      ImGui::CloseCurrentPopup();
    }
    if (ImGui::MenuItem("  Animator 3D")) {
      m_SelectionContext.AddComponent<AnimatorComponent>();
      ImGui::CloseCurrentPopup();
    }
    if (ImGui::MenuItem("  Directional Light")) {
      m_SelectionContext.AddComponent<DirectionalLightComponent>();
      ImGui::CloseCurrentPopup();
    }
    if (ImGui::MenuItem("  Point Light")) {
      m_SelectionContext.AddComponent<PointLightComponent>();
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }
  ImGui::Spacing();
  ::UHE::DrawComponents<TransformComponent>(
      "Transform", entity, [](auto &components) {
        DrawVec3Control("Translation", components.Translation);
        glm::vec3 rotation = glm::degrees(components.Rotation);
        DrawVec3Control("Rotation", rotation);
        components.Rotation = glm::radians(rotation);
        DrawVec3Control("Scale", components.Scale, 1.0f);
      });

  ::UHE::DrawComponents<RigidBody2DComponent>(
      "Rigid Body 2D", entity, [](auto &components) {
        // To this (correct scoping):
        const char *bodyTypeStrings[] = {"Static", "Kinematic", "Dynamic"};
        const char *currentBodyTypeString =
            bodyTypeStrings[(int)components.Type];

        if (ImGui::BeginCombo("Body Type", currentBodyTypeString)) {
          for (int i = 0; i < 3; i++) {
            bool isSelected = currentBodyTypeString == bodyTypeStrings[i];
            if (ImGui::Selectable(bodyTypeStrings[i], isSelected)) {
              currentBodyTypeString = bodyTypeStrings[i];
              // Access via the Component Type, not the instance
              components.Type = (RigidBody2DComponent::BodyType)i;
            }
            if (isSelected)
              ImGui::SetItemDefaultFocus();
          }
          ImGui::EndCombo();
        }

        ImGui::Checkbox("Fixed Rotation", &components.FixedRotation);
      });

  ::UHE::DrawComponents<BoxColliderComponent>(
      "Box Collider 2D", entity, [](auto &components) {
        ImGui::DragFloat2("Offset", glm::value_ptr(components.Offset), 0.01f);
        ImGui::DragFloat2("Size", glm::value_ptr(components.Size), 0.01f, 0.01f, 100.0f);
        ImGui::DragFloat("Density", &components.Density, 0.1f, 0.0f, 100.0f);
        ImGui::DragFloat("Friction", &components.Friction, 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat("Restitution", &components.Restitution, 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat("Restitution Threshold", &components.RestitutionThreshold, 0.01f, 0.0f, 10.0f);
      });

  ::UHE::DrawComponents<RigidBody3DComponent>(
      "Rigid Body 3D", entity, [](auto &components) {
        const char *bodyTypeStrings[] = {"Static", "Kinematic", "Dynamic"};
        const char *currentBodyTypeString = bodyTypeStrings[(int)components.Type];

        if (ImGui::BeginCombo("Body Type", currentBodyTypeString)) {
          for (int i = 0; i < 3; i++) {
            bool isSelected = currentBodyTypeString == bodyTypeStrings[i];
            if (ImGui::Selectable(bodyTypeStrings[i], isSelected)) {
              currentBodyTypeString = bodyTypeStrings[i];
              components.Type = (RigidBody3DComponent::BodyType)i;
            }
            if (isSelected) ImGui::SetItemDefaultFocus();
          }
          ImGui::EndCombo();
        }

        ImGui::DragFloat("Mass", &components.Mass, 0.1f, 0.0f, 10000.0f);
        ImGui::DragFloat("Linear Damping", &components.LinearDamping, 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat("Angular Damping", &components.AngularDamping, 0.01f, 0.0f, 1.0f);
        ImGui::Checkbox("Is Sensor", &components.IsSensor);
        
        ImGui::Text("Allowed Degrees of Freedom");
        ImGui::Checkbox("Trans X", &components.AllowedDOFs[0]); ImGui::SameLine();
        ImGui::Checkbox("Trans Y", &components.AllowedDOFs[1]); ImGui::SameLine();
        ImGui::Checkbox("Trans Z", &components.AllowedDOFs[2]);
        ImGui::Checkbox("Rot X", &components.AllowedDOFs[3]); ImGui::SameLine();
        ImGui::Checkbox("Rot Y", &components.AllowedDOFs[4]); ImGui::SameLine();
        ImGui::Checkbox("Rot Z", &components.AllowedDOFs[5]);
      });

  ::UHE::DrawComponents<BoxCollider3DComponent>(
      "Box Collider 3D", entity, [](auto &components) {
        DrawVec3Control("Offset", components.Offset);
        DrawVec3Control("Half Extent", components.HalfExtent);
        ImGui::DragFloat("Friction", &components.Friction, 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat("Restitution", &components.Restitution, 0.01f, 0.0f, 1.0f);
      });

  ::UHE::DrawComponents<SphereCollider3DComponent>(
      "Sphere Collider 3D", entity, [](auto &components) {
        DrawVec3Control("Offset", components.Offset);
        ImGui::DragFloat("Radius", &components.Radius, 0.01f, 0.0f, 100.0f);
        ImGui::DragFloat("Friction", &components.Friction, 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat("Restitution", &components.Restitution, 0.01f, 0.0f, 1.0f);
      });

  ::UHE::DrawComponents<CapsuleCollider3DComponent>(
      "Capsule Collider 3D", entity, [](auto &components) {
        DrawVec3Control("Offset", components.Offset);
        ImGui::DragFloat("Radius", &components.Radius, 0.01f, 0.0f, 100.0f);
        ImGui::DragFloat("Half Height", &components.HalfHeight, 0.01f, 0.0f, 100.0f);
        ImGui::DragFloat("Friction", &components.Friction, 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat("Restitution", &components.Restitution, 0.01f, 0.0f, 1.0f);
      });

  ::UHE::DrawComponents<CameraComponent>(
      "Camera", entity, [](auto &components) {
        auto &camera = components.Camera;
        ImGui::Checkbox("Primary", &components.Primary);
        const char *projectionTypeString[] = {"Projection", "Orthographic"};
        const char *currnetProjectionTypeString =
            projectionTypeString[(int)(camera).GetProjectionType()];
        if (ImGui::BeginCombo("Projection", currnetProjectionTypeString)) {
          for (int i{0}; i < 2; i++) {
            bool isSelected =
                currnetProjectionTypeString == projectionTypeString[i];
            if (ImGui::Selectable(projectionTypeString[i], isSelected)) {
              currnetProjectionTypeString = projectionTypeString[i];
              camera.SetProjectionType((SceneCamera::ProjectionType)i);
            }
            if (isSelected)
              ImGui::SetItemDefaultFocus();
          }
          ImGui::EndCombo();
        }
        if (camera.GetProjectionType() ==
            SceneCamera::ProjectionType::Perpective) {
          f32 ProjFOV = glm::degrees(camera.GetProjectionFOV());
          if (ImGui::DragFloat("FOV", &ProjFOV)) {
            camera.SetPerspectiveFOV(glm::radians(ProjFOV));
          }
          f32 ProjNear = camera.GetProjectionNear();
          if (ImGui::DragFloat("FOV Near", &ProjNear)) {
            camera.SetPerpecstiveNear(ProjNear);
          }
          f32 ProjFar = camera.GetProjectionFar();
          if (ImGui::DragFloat("FOV Far", &ProjFar)) {
            camera.SetPerspecstiveFar(ProjFar);
          }
        }
        if (camera.GetProjectionType() ==
            SceneCamera::ProjectionType::Orhtographic) {
          f32 orthoSize = camera.GetOrthographicSize();
          if (ImGui::DragFloat("Size", &orthoSize)) {
            camera.SetOrthographicSize(orthoSize);
          }
          f32 orthoNear = camera.GetOrthographicNearClip();
          if (ImGui::DragFloat("Near", &orthoNear)) {
            camera.SetOrthoGraphicNearClip(orthoNear);
          }
          f32 orthoFar = camera.GetOrthographicFarClip();
          if (ImGui::DragFloat("Far", &orthoFar)) {
            camera.SetOrthoGraphicFarClip(orthoFar);
          }
          ImGui::Checkbox("Fixed aspect ratio", &components.FixedAspectRatio);
        }
      });
  ::UHE::DrawComponents<SpriteRendererComponent>(
      "Sprite Renderer", entity, [](SpriteRendererComponent &src) {
        ImGui::ColorEdit4("Color", glm::value_ptr(src.Color));
        char texBuf[256];
        memset(texBuf, 0, sizeof(texBuf));
        strncpy(texBuf, src.TexturePath.c_str(), sizeof(texBuf) - 1); texBuf[sizeof(texBuf) - 1] = '\0';
        float inputWidth = ImGui::GetContentRegionAvail().x - 70.0f -
                           ImGui::CalcTextSize("Texture").x - 10.0f;
        ImGui::PushItemWidth(inputWidth > 10.0f ? inputWidth : 10.0f);
        if (ImGui::InputText("##TextureInput", texBuf, sizeof(texBuf))) {
          src.TexturePath = std::string(texBuf);
          src.Texture = nullptr;
        }
        ImGui::PopItemWidth();
        ImGui::SameLine();
        if (ImGui::Button("Browse##Tex", ImVec2(60, 0))) {
          std::string path =
              FileDialogs::OpenFile("Image (*.png;*.jpg)\0*.png;*.jpg\0");
          if (!path.empty()) {
            src.TexturePath = path;
            src.Texture = Texture2D::Create(path);
          }
        }
        
        ImGui::Spacing();
        if (src.Texture) {
          ImGui::Image(
              (ImTextureID)(intptr_t)src.Texture->GetImGuiTextureID(),
              ImVec2(100.0f, 100.0f), ImVec2(0, 1), ImVec2(1, 0));
        } else {
          ImGui::Button("Drop Texture Here", ImVec2(100.0f, 100.0f));
        }
        
        if (ImGui::BeginDragDropTarget()) {
          if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Content_Browser_item")) {
            const char* path = (const char*)payload->Data;
            std::filesystem::path itemPath(path);
            UHE_CORE_INFO("Accepted DragDropPayload: {0}", path);
            if (itemPath.extension() == ".png" || itemPath.extension() == ".jpg") {
              src.TexturePath = path;
              src.Texture = Texture2D::Create(path);
              if (src.Texture) {
                  UHE_CORE_INFO("Successfully loaded texture from payload!");
              } else {
                  UHE_CORE_ERROR("Failed to load texture from payload!");
              }
            } else {
                UHE_CORE_WARN("Payload extension not supported: {0}", itemPath.extension().string());
            }
          }
          ImGui::EndDragDropTarget();
        }

        ImGui::SameLine();
        ImGui::Text("Texture");
        ImGui::DragFloat("Tiling", &src.TilingFactor, 0.1f, 0.0f, 100.0f);
        ImGui::Checkbox("Use SubTexture", &src.UseSubTexture);
        if (src.UseSubTexture) {
          ImGui::DragFloat2("Coords", glm::value_ptr(src.SubTextureCoords),
                            1.0f);
          ImGui::DragFloat2("Cell Size", glm::value_ptr(src.SubTextureCellSize),
                            1.0f);
          ImGui::DragFloat2("Sprite Size",
                            glm::value_ptr(src.SubTextureSpriteSize), 1.0f);
          if (src.Texture) {
            float texWidth = static_cast<float>(src.Texture->GetWidth());
            float texHeight = static_cast<float>(src.Texture->GetHeight());
            // SubTexture2D::CreateFromCoords math
            glm::vec2 min = {
                (src.SubTextureCoords.x * src.SubTextureCellSize.x) / texWidth,
                (src.SubTextureCoords.y * src.SubTextureCellSize.y) /
                    texHeight};
            glm::vec2 max = {
                ((src.SubTextureCoords.x + src.SubTextureSpriteSize.x) *
                 src.SubTextureCellSize.x) /
                    texWidth,
                ((src.SubTextureCoords.y + src.SubTextureSpriteSize.y) *
                 src.SubTextureCellSize.y) /
                    texHeight};
            ImGui::Image((ImTextureID)src.Texture->GetImGuiTextureID(),
                         ImVec2(64, 64), ImVec2(min.x, max.y),
                         ImVec2(max.x, min.y));
            ImGui::SameLine();
            ImGui::Text("SubTexture Preview");
          }
        } else if (src.Texture) {
          ImGui::Image((ImTextureID)src.Texture->GetImGuiTextureID(),
                       ImVec2(64, 64), ImVec2(0, 1), ImVec2(1, 0));
          ImGui::SameLine();
          ImGui::Text("Texture Preview");
        }
      });
  ::UHE::DrawComponents<SpriteAnimationComponent>(
      "Sprite Animation", entity, [](SpriteAnimationComponent &animComp) {
        char sheetBuf[256];
        memset(sheetBuf, 0, sizeof(sheetBuf));
        strncpy(sheetBuf, animComp.SpriteSheetPath.c_str(), sizeof(sheetBuf) - 1); sheetBuf[sizeof(sheetBuf) - 1] = '\0';
        ImGui::Text("SpriteSheet");
        if (ImGui::InputText("##SpritesheetInput", sheetBuf,
                             sizeof(sheetBuf))) {
          animComp.SpriteSheetPath = std::string(sheetBuf);
          animComp.Animation.SpriteSheet = nullptr;
        }

        ImGui::PopItemWidth();
        ImGui::SameLine();
        if (ImGui::Button("Browse##Anim", ImVec2(60, 0))) {
          std::string path =
              FileDialogs::OpenFile("Image (*.png;*.jpg)\0*.png;*.jpg\0");
          if (!path.empty()) {
            animComp.SpriteSheetPath = path;
            animComp.Animation.SpriteSheet = Texture2D::Create(path);
          }
        }
        
        ImGui::Spacing();
        // Spritesheet preview
        if (animComp.Animation.SpriteSheet) {
          ImGui::Image((ImTextureID)(intptr_t)animComp.Animation.SpriteSheet->GetImGuiTextureID(),
                       ImVec2(64, 64), ImVec2(0, 1), ImVec2(1, 0));
          ImGui::SameLine();
          ImGui::Text("Atlas Preview");
        } else {
          ImGui::Button("Drop Texture Here##Anim", ImVec2(100.0f, 100.0f));
        }
        
        if (ImGui::BeginDragDropTarget()) {
          if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Content_Browser_item")) {
            const char* path = (const char*)payload->Data;
            std::filesystem::path itemPath(path);
            UHE_CORE_INFO("Accepted DragDropPayload (Anim): {0}", path);
            if (itemPath.extension() == ".png" || itemPath.extension() == ".jpg") {
              animComp.SpriteSheetPath = path;
              animComp.Animation.SpriteSheet = Texture2D::Create(path);
            }
          }
          ImGui::EndDragDropTarget();
        }
        ImGui::DragFloat2("Frame Size",
                          glm::value_ptr(animComp.Animation.FrameSize), 1.0f,
                          1.0f, 4096.0f);
        ImGui::DragInt("Frame Count", &animComp.Animation.FrameCount, 1, 1,
                       1000);
        ImGui::DragFloat("Frame Duration", &animComp.Animation.FrameDuration,
                         0.01f, 0.001f, 10.0f, "%.3f s");
        ImGui::Checkbox("Loop", &animComp.Animation.Loop);
        ImGui::ColorEdit4("Color", glm::value_ptr(animComp.Color));
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Playback Controls");
        if (ImGui::Button(animComp.Animation.Playing ? "Pause" : "Play"))
          animComp.Animation.Playing = !animComp.Animation.Playing;
        ImGui::SameLine();
        if (ImGui::Button("Reset")) {
          animComp.Animation.CurrentFrameIndex = 0;
          animComp.Animation.Timer = 0.0f;
        }
        ImGui::Text("Frame: %d / %d", animComp.Animation.CurrentFrameIndex,
                    animComp.Animation.FrameCount);
      });

  ::UHE::DrawComponents<Model3DComponent>(
      "3D Model", entity, [](Model3DComponent &mc) {
        char pathBuf[256];
        memset(pathBuf, 0, sizeof(pathBuf));
        strncpy(pathBuf, mc.ModelPath.c_str(), sizeof(pathBuf) - 1); pathBuf[sizeof(pathBuf) - 1] = '\0';
        float inputWidth = ImGui::GetContentRegionAvail().x - 70.0f -
                           ImGui::CalcTextSize("Model").x - 10.0f;
        ImGui::PushItemWidth(inputWidth > 10.0f ? inputWidth : 10.0f);
        if (ImGui::InputText("##ModelInput", pathBuf, sizeof(pathBuf))) {
          mc.ModelPath = std::string(pathBuf);
        }
        ImGui::SameLine();
        if (ImGui::Button("Load"))
        {
          mc.IsLoaded = mc.ModelData->loadModel(mc.ModelPath);
        }
        ImGui::PopItemWidth();
        ImGui::SameLine();
        if (ImGui::Button("Browse##Model", ImVec2(60, 0))) {
          std::string path =
              FileDialogs::OpenFile("GLTF/GLB (*.gltf;*.glb)\0*.gltf;*.glb\0");
          if (!path.empty()) {
            mc.ModelPath = path;
            mc.IsLoaded = mc.ModelData->loadModel(path);
          }
        }
        
        ImGui::Spacing();
        if (mc.IsLoaded) {
          ImGui::Text("Model Loaded Successfully");
        } else {
          ImGui::Button("Drop 3D Model Here", ImVec2(100.0f, 100.0f));
        }
        
        if (ImGui::BeginDragDropTarget()) {
          if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Content_Browser_item")) {
            const char* path = (const char*)payload->Data;
            std::filesystem::path itemPath(path);
            UHE_CORE_INFO("Accepted DragDropPayload (3D Model): {0}", path);
            if (itemPath.extension() == ".gltf" || itemPath.extension() == ".glb") {
              mc.ModelPath = path;
              mc.IsLoaded = mc.ModelData->loadModel(path);
              if (mc.IsLoaded) {
                  UHE_CORE_INFO("Successfully loaded 3D model from payload!");
              } else {
                  UHE_CORE_ERROR("Failed to load 3D model from payload!");
              }
            } else {
                UHE_CORE_WARN("Payload extension not supported: {0}", itemPath.extension().string());
            }
          }
          ImGui::EndDragDropTarget();
        }
      });

  ::UHE::DrawComponents<DirectionalLightComponent>(
      "Directional Light", entity, [](DirectionalLightComponent &light) {
        ImGui::ColorEdit3("Color", glm::value_ptr(light.Color));
        ImGui::DragFloat("Intensity", &light.Intensity, 0.1f, 0.0f, 100.0f);
        ImGui::TextDisabled("Rotate the entity's Transform to change light direction.");
      });

  ::UHE::DrawComponents<PointLightComponent>(
      "Point Light", entity, [](PointLightComponent &light) {
        ImGui::ColorEdit3("Color", glm::value_ptr(light.Color));
        ImGui::DragFloat("Intensity", &light.Intensity, 0.1f, 0.0f, 100.0f);
        ImGui::DragFloat("Radius", &light.Radius, 0.5f, 0.0f, 1000.0f);
      });

  ::UHE::DrawComponents<AnimatorComponent>(
      "Animator 3D", entity, [entity](AnimatorComponent &anim) {
        UHE::Entity ent = entity;
        if (!anim.Animator) {
            // Check if we have a model component
            if (ent.HasComponent<Model3DComponent>()) {
                auto& mc = ent.GetComponent<Model3DComponent>();
                if (mc.IsLoaded && mc.ModelData) {
                    if (ImGui::Button("Initialize Animator")) {
                        anim.Animator = CreateRef<RD3d::Animator>(mc.ModelData);
                    }
                } else {
                    ImGui::TextDisabled("Model not loaded yet.");
                }
            } else {
                ImGui::TextDisabled("Requires a Model3DComponent on the entity.");
            }
        }

        if (anim.Animator) {
            auto model = anim.Animator->GetModel();
            if (model && !model->GetAnimations().empty()) {
                const auto& animations = model->GetAnimations();
                
                if (ImGui::BeginCombo("Animation", anim.CurrentAnimationName.empty() ? "None" : anim.CurrentAnimationName.c_str())) {
                    for (const auto& a : animations) {
                        bool isSelected = (anim.CurrentAnimationName == a.Name);
                        if (ImGui::Selectable(a.Name.c_str(), isSelected)) {
                            anim.CurrentAnimationName = a.Name;
                            anim.Animator->PlayAnimation(a.Name);
                            anim.IsPlaying = true;
                        }
                        if (isSelected) {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Text("Playback Controls");
                if (ImGui::Button(anim.IsPlaying ? "Pause" : "Play")) {
                    anim.IsPlaying = !anim.IsPlaying;
                }
                
                ImGui::SameLine();
                if (ImGui::Button("Restart")) {
                    anim.Animator->PlayAnimation(anim.CurrentAnimationName);
                    anim.IsPlaying = true;
                }
                
                ImGui::DragFloat("Playback Speed", &anim.PlaybackSpeed, 0.1f, 0.0f, 5.0f);
            } else {
                ImGui::TextDisabled("Model has no animations.");
            }
        }
      });
}

} // namespace UHE
