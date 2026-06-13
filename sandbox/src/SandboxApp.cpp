#include <UHE.h>
#include <UHE/Core/EntryPoint.h>
#include "ModelTestLayer.h"

class Sandbox : public UHE::Application {
public:
	Sandbox() {
		PushLayer(new ModelTestLayer());
	}
	~Sandbox() {
	}
};

UHE::Application* UHE::CreateApplication() {
	return new Sandbox();
}
