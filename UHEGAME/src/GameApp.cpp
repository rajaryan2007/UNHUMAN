#include <UHE.h>
#include <UHE/Core/EntryPoint.h>
#include "AimLabLayer.h"

class UHEGameApp : public UHE::Application {
public:
	UHEGameApp() {
		PushLayer(new AimLabLayer());
	}
	~UHEGameApp() {}
};

UHE::Application* UHE::CreateApplication() {
	return new UHEGameApp();
}
