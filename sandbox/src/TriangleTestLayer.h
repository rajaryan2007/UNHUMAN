#pragma once
#include "UHE.h"

class TriangleTestLayer : public UHE::Layer
{
public:
    TriangleTestLayer();
    virtual ~TriangleTestLayer() = default;

    virtual void OnAttach() override;
    virtual void OnDetach() override;

    void OnUpdate(UHE::Timestep ts) override;
    virtual void OnImGuiRender() override;
    void OnEvent(UHE::Event& e) override;

private:
    UHE::RHI::ShaderHandle m_VertexShader = nullptr;
    UHE::RHI::ShaderHandle m_FragmentShader = nullptr;
    UHE::RHI::PipelineHandle m_Pipeline = nullptr;
    UHE::RHI::BufferHandle m_VertexBuffer = nullptr;
};
