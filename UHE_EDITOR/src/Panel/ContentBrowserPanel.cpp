#include "ContentBrowserPanel.h"
#include "UHE/AssestsManager/VfsSystem.h"
#include "imgui.h"
#include "uhepch.h"
#include <filesystem>
namespace UHE {
ContentBrowserPanel::ContentBrowserPanel() {
  m_BaseDirectory = FileSystem::Get().GetRootPath() / "assets";
  m_CurrentDirectory = m_BaseDirectory;
  auto rootPath = FileSystem::Get().GetRootPath();
  m_DirectoryIcon = Texture2D::Create((rootPath / "assets/icon/folder.png").string());
  m_FileIcon = Texture2D::Create((rootPath / "assets/icon/file.png").string());
  m_PngIcon = Texture2D::Create((rootPath / "assets/icon/png.png").string());
  m_modelPng = Texture2D::Create((rootPath / "assets/icon/3dmodel.png").string());
}
void ContentBrowserPanel::DrawDirectoryTree(
    const std::filesystem::path &directoryPath) {
  for (auto &directoryEntry : std::filesystem::directory_iterator(directoryPath)) {
    if (!directoryEntry.is_directory())
      continue;
    const auto &path = directoryEntry.path();
    std::string pathString = path.string();
    std::string filenameString = path.filename().string();
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow |
                               ImGuiTreeNodeFlags_OpenOnDoubleClick |
                               ImGuiTreeNodeFlags_SpanAvailWidth;
    if (m_CurrentDirectory == path) {
      flags |= ImGuiTreeNodeFlags_Selected;
    }
    bool opened = ImGui::TreeNodeEx(pathString.c_str(), flags, "%s",
                                    filenameString.c_str());
    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
      m_CurrentDirectory = path;
    }
    if (opened) {
      DrawDirectoryTree(path);
      ImGui::TreePop();
    }
  }
}
void ContentBrowserPanel::OnImGuiRender() {
  ImGui::Begin("Content Browser");

  if (ImGui::BeginTable("ContentBrowserLayout_v2", 2,
                        ImGuiTableFlags_Resizable |
                            ImGuiTableFlags_BordersInnerV)) {
    ImGui::TableSetupColumn("Tree", ImGuiTableColumnFlags_WidthFixed, 150.0f);
    ImGui::TableSetupColumn("Content", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGuiTreeNodeFlags rootFlags = ImGuiTreeNodeFlags_OpenOnArrow |
                                   ImGuiTreeNodeFlags_SpanAvailWidth |
                                   ImGuiTreeNodeFlags_DefaultOpen;
    if (m_CurrentDirectory == m_BaseDirectory) {
      rootFlags |= ImGuiTreeNodeFlags_Selected;
    }
    std::string baseDirString = m_BaseDirectory.string();
    bool rootOpened =
        ImGui::TreeNodeEx(baseDirString.c_str(), rootFlags, "Assets");
    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
      m_CurrentDirectory = m_BaseDirectory;
    }
    if (rootOpened) {
      DrawDirectoryTree(m_BaseDirectory);
      ImGui::TreePop();
    }
    ImGui::TableSetColumnIndex(1);
    // Padding and dimension settings
    static float padding = 16.0f;
    static float thumbnailSize = 64.0f;
    float cellSize = thumbnailSize + padding;
    float panelWidth = ImGui::GetContentRegionAvail().x;
    int columnCount = (int)(panelWidth / cellSize);
    if (columnCount < 1)
      columnCount = 1;
    
    if (m_CurrentDirectory != m_BaseDirectory) {
      if (ImGui::Button("<- Back", ImVec2(70, 0))) {
        m_CurrentDirectory = m_CurrentDirectory.parent_path();
      }
      ImGui::SameLine();
      ImGui::Text("%s", m_CurrentDirectory.string().c_str());
      ImGui::Separator();
    }
   
    ImGui::BeginChild("ContentGrid");
    ImGui::Columns(columnCount, 0, false);
    for (auto &directoryEntry :
         std::filesystem::directory_iterator(m_CurrentDirectory)) {
      const auto &path = directoryEntry.path();
      std::string filenameString = path.filename().string();
      ImGui::PushID(filenameString.c_str());
   
      ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
      if (directoryEntry.is_directory()) {
        ImTextureID dirTexID =
            (ImTextureID)(intptr_t)m_DirectoryIcon->GetImGuiTextureID();
        if (ImGui::ImageButton(filenameString.c_str(), dirTexID,{thumbnailSize, thumbnailSize}, {0, 1},
{1, 0})) {
          m_CurrentDirectory /= path.filename();
        }
      } else {
        ImTextureID fileTexID;
        if (path.extension() == ".png" || path.extension() == ".jpg") {
          std::string absolutePath = path.string();
          
          if (m_TextureCache.find(absolutePath) == m_TextureCache.end()) {
              m_TextureCache[absolutePath] = Texture2D::Create(absolutePath);
          }
         
          if (m_TextureCache[absolutePath]) {
              fileTexID = (ImTextureID)(intptr_t)m_TextureCache[absolutePath]->GetImGuiTextureID();
          } else {
              fileTexID = (ImTextureID)(intptr_t)m_PngIcon->GetImGuiTextureID();
          }
        } 
        else if (path.extension() == ".glb" || path.extension() == ".fbx" || path.extension() == ".gltf" || path.extension() == ".obj")
        {
            fileTexID = (ImTextureID)(intptr_t)m_modelPng->GetImGuiTextureID();
        }
        else {
          fileTexID = (ImTextureID)(intptr_t)m_FileIcon->GetImGuiTextureID();
        }
        ImGui::ImageButton(filenameString.c_str(), fileTexID,
                           {thumbnailSize, thumbnailSize}, {0, 1},
                           {1, 0});
        if (ImGui::BeginDragDropSource())
        {   
            std::string pathStr = path.string();
            // Pass string length + 1 for null terminator
            ImGui::SetDragDropPayload("Content_Browser_item", pathStr.c_str(), pathStr.length() + 1, ImGuiCond_Once);
            ImGui::EndDragDropSource();
        }
      }
      ImGui::PopStyleColor(); 
      
      float textWidth = ImGui::CalcTextSize(filenameString.c_str()).x;
      float textOffset = (thumbnailSize - textWidth) * 0.5f;
      if (textOffset > 0.0f) {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + textOffset);
      }
      ImGui::TextWrapped("%s", filenameString.c_str());
      ImGui::NextColumn();
      ImGui::PopID();
    }
    ImGui::Columns(1);
    ImGui::EndChild();
    ImGui::EndTable();
  }
  ImGui::End();
}
} // namespace UHE