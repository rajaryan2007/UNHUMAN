#pragma once
#include "UHE.h"

#include "Panel/SceneHierachyPanel.h"
#include "UHE/Renderer/EditorCamera.h"
#include "Panel/ContentBrowserPanel.h"
#include "UHE/Renderer/Framebuffer.h"

namespace UHE
{
	class Editor : public Layer
	{
	public:
		Editor();
		virtual ~Editor() = default;

		void OnAttach() override;
		void OnDetach() override;

		void OnUpdate(Timestep ts) override;
		void OnEvent(Event& event) override;
        
		virtual void OnImGuiRender() override;
		bool OnMouseButtonPressedEvent(MouseButtonPressedEvent& e);

		void UI_Toolbar();
	private:

		void OnScreenPlay();
		void OnSceneStop();
		bool onKeyPressed(KeyPressedEvent& e);
		void NewScene();
		void OpenScene();
		void SaveSceneAs();
		void OpenScene(const std::filesystem::path& path);
	private:
		Entity m_Square;
		Entity m_CameraEntity;
		Entity m_SecondCamera;

		Entity m_HoverdEntity;
		bool primaryCamera = false;
		EditorCamera m_EditorCamera;
		bool m_ViewPortFocused = false;
		bool m_ViewPortHover = false;
		
		OrthographicCameraContoroller m_CameraController;


		Ref<Scene> m_ActiveScene;
		Ref<Scene> m_EditorScene;

		glm::vec4 blueColor = { 0.2f, 0.3f, 0.8f, 1.0f };
		glm::vec3 m_Transform;
		glm::vec4 redColor = { 0.8f, 0.2f, 0.3f, 1.0f };

		u32 m_mapWidth, m_mapHeight;

		glm::vec2 m_ViewPortSize;
		glm::vec2 m_ViewPortBounds[2];
		int m_GizmoType = -1;

		SceneHierarchyPanel m_SceneHireacyPanel;
		ContentBrowserPanel m_ContentBrowserPanel;
		Ref<Framebuffer> m_Framebuffer;
		Ref<Texture2D> m_IconPlay;
		Ref<Texture2D> m_IconStop;
		uint32_t m_FramesSinceResize = 0;
	    
		enum class SceneState
		{
			Edit = 0, Play = 1,
		};

		SceneState m_SceneState = SceneState::Edit;
	};
}
