#include "uhepch.h" // pragma
#include "VulkanGraphicPipeline.h"
#include "UHE/RHI/RHIDevice.h"
#include "UHE/RHI/RHITypes.h"
#include "VulkanDescriptorManager.h"
#include "VulkanLogicalDevice.h"
#include "VulkanShader.h"
#include "VulkanTypes.h"
#include "vulkan/vulkan.hpp"

namespace UHE::RHI::VULKAN
{

VulkanGraphicPipeline::VulkanGraphicPipeline() = default;
VulkanGraphicPipeline::~VulkanGraphicPipeline() = default;

void VulkanGraphicPipeline::Init() {}

void VulkanGraphicPipeline::Bind() {}

void VulkanGraphicPipeline::createGraphicsPipeline(VulkanLogicalDevice& Device,
                                                   VulkanDescriptorManager& descriptorManager,
                                                   const GraphicsPipelineDesc& desc)
{

    auto vertModule = reinterpret_cast<VulkanShader*>(desc.vertexShader)->GetModule();
    auto fragModule = reinterpret_cast<VulkanShader*>(desc.fragmentShader)->GetModule();

    vk::PipelineShaderStageCreateInfo shaderStages[] = {{{}, vk::ShaderStageFlagBits::eVertex, vertModule, "main"},
                                                        {{}, vk::ShaderStageFlagBits::eFragment, fragModule, "main"}};

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo = CreateVertexInputState(desc.vertexLayout);

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.topology = MapTopology(desc.topology);
    inputAssembly.primitiveRestartEnable = vk::False;

    vk::PipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.depthClampEnable = vk::False;
    rasterizer.rasterizerDiscardEnable = vk::False;
    rasterizer.polygonMode = vk::PolygonMode::eFill; // can add Wireframe configuration if supported by
                                                     // desc
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = vk::CullModeFlagBits::eBack;
    rasterizer.frontFace = vk::FrontFace::eClockwise;
    rasterizer.depthBiasEnable = vk::False;

    auto globalLayout = descriptorManager.GetLayoutHandle();
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &globalLayout;

    vk::PushConstantRange pushConstantRange{};
    if (desc.pushConstantSize > 0)
    {
        pushConstantRange.stageFlags = vk::ShaderStageFlagBits::eAllGraphics;
        pushConstantRange.size = desc.pushConstantSize;
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    }

    m_PipelineLayout = vk::raii::PipelineLayout(Device.getLogicalDevice(), pipelineLayoutInfo);

    vk::PipelineMultisampleStateCreateInfo multisampling{};
    multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
    multisampling.sampleShadingEnable = vk::False;

    vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                                          vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

    if (desc.blendMode == BlendMode::Alpha)
    {
        colorBlendAttachment.blendEnable = vk::True;
        colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
        colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
        colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
        colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
        colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
        colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;
    }
    else if (desc.blendMode == BlendMode::Additive)
    {
        colorBlendAttachment.blendEnable = vk::True;
        colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eOne;
        colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eOne;
        colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
        colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
        colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eOne;
        colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;
    }
    else
    {
        colorBlendAttachment.blendEnable = vk::False;
    }

    vk::PipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.logicOpEnable = vk::False;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    vk::PipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.depthTestEnable = desc.depthTest ? vk::True : vk::False;
    depthStencil.depthWriteEnable = desc.depthWrite ? vk::True : vk::False;
    depthStencil.depthCompareOp = vk::CompareOp::eLess;
    depthStencil.depthBoundsTestEnable = vk::False;
    depthStencil.stencilTestEnable = vk::False;

    std::vector<vk::DynamicState> dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
    vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
    dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();

    vk::PipelineViewportStateCreateInfo viewportState{};
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    vk::Format colorFormat = MapTextureFormat(desc.colorFormat);
    vk::Format depthFormat = MapTextureFormat(desc.depthFormat);

    vk::PipelineRenderingCreateInfo pipelineRenderCreateInfo;
    pipelineRenderCreateInfo.colorAttachmentCount = 1;
    pipelineRenderCreateInfo.pColorAttachmentFormats = &colorFormat;
    if (desc.depthTest || desc.depthWrite)
    {
        pipelineRenderCreateInfo.depthAttachmentFormat = depthFormat;
    }

    vk::GraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.pNext = &pipelineRenderCreateInfo;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages; // Changed from pStage to pStages
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pDynamicState = &dynamicStateCreateInfo;
    pipelineInfo.layout = *m_PipelineLayout; // Extracts the raw vk::PipelineLayout handle
    pipelineInfo.renderPass = nullptr;       // Correct for Dynamic Rendering

    m_GraphicsPipeline = vk::raii::Pipeline(Device.getLogicalDevice(), nullptr, pipelineInfo);
}

vk::PipelineVertexInputStateCreateInfo VulkanGraphicPipeline::CreateVertexInputState(const BufferLayout& layout)
{
    m_BindingDescription.clear();
    m_AttributeDescription.clear();

    if (layout.GetElements().empty())
    {
        return vk::PipelineVertexInputStateCreateInfo({}, 0, nullptr, 0, nullptr);
    }

    vk::VertexInputBindingDescription bindingDesc{};
    bindingDesc.binding = 0;
    bindingDesc.stride = layout.GetStride();
    bindingDesc.inputRate = vk::VertexInputRate::eVertex;
    m_BindingDescription.push_back(bindingDesc);

    u32 location = 0;

    for (const auto& elements : layout.GetElements())
    {
        u32 slotcount = 1;
        u32 slotsize = 0;

        if (elements.Type == ShaderDataType::Mat3)
        {
            slotcount = 3;
            slotsize = 4 * 3;
        }
        if (elements.Type == ShaderDataType::Mat4)
        {
            slotcount = 4;
            slotcount = 4 * 4;
        }

        for (u32 i = 0; i < slotcount; i++)
        {
            vk::VertexInputAttributeDescription attributeDsec{};
            attributeDsec.binding = 0;
            attributeDsec.location = location;
            attributeDsec.format = ShaderDataTypeToVulkanFormat(elements.Type);

            attributeDsec.offset = elements.Offset + (i * slotsize);
            m_AttributeDescription.push_back(attributeDsec);
            location++;
        }
    }

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.vertexBindingDescriptionCount = static_cast<u32>(m_BindingDescription.size());
    vertexInputInfo.pVertexBindingDescriptions = m_BindingDescription.data();
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<u32>(m_AttributeDescription.size());
    vertexInputInfo.pVertexAttributeDescriptions = m_AttributeDescription.data();

    return vertexInputInfo;
};

} // namespace UHE::RHI::VULKAN
