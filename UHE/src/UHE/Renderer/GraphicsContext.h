#pragma once



namespace UHE
{
	class UHE_API GraphicsContext 
	{
	public:
		virtual void Init() = 0;
        virtual void SwapBuffers() = 0;
	};
}