#pragma once
#include "UHE/AssestsManager/VfsSystem.h"

extern UHE::Application* UHE::CreateApplication();

int main(int argc, char** argv)
{
    UHE::FileSystem::Get().Initialize(argv[0]);
	UHE::Log::Init();
    auto app = UHE::CreateApplication();
    
    app->Run();

    delete app;
    return 0;
}
