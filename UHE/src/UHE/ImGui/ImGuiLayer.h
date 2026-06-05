#pragma once

#include "UHE/Core/Layer.h"
#include "UHE/Events/ApplicationEvent.h"
#include "UHE/Events/KeyEvent.h"
#include "UHE/Events/MouseEvent.h"

namespace UHE
{
	class UHE_API ImGuiLayer :public Layer {
	public:
		ImGuiLayer();
		~ImGuiLayer();

		virtual void OnDetach() override;
		virtual void OnAttach() override;
//		virtual void OnImGuiRender() ;
		virtual void OnEvent(Event& e) override;

		virtual void Begin() = 0;
		virtual void End() = 0;

		static ImGuiLayer* Create();

		void SetBlockEvent(bool block) { m_BlockEvents = block; }
	
		void SetDarkThemeColor();
	private:
        float m_Time = 0.0f;
		bool m_BlockEvents = true;
		
	};
}