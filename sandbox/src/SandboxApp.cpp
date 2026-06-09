#include <UHE.h>
#include <UHE/Core/EntryPoint.h>
#include "TriangleTestLayer.h"

class Sandbox : public UHE::Application {
public:
	Sandbox() {
		PushLayer(new TriangleTestLayer());
	}
	~Sandbox() {
	}
};

UHE::Application* UHE::CreateApplication() {
	return new Sandbox();
}
