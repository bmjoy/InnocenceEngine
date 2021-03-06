#include "VoxelizationPass.h"
#include "../DefaultGPUBuffers/DefaultGPUBuffers.h"

#include "../../Engine/Interface/IModuleManager.h"

INNO_ENGINE_API extern IModuleManager* g_pModuleManager;

using namespace DefaultGPUBuffers;

struct VoxelizationConstantBuffer
{
	Vec4 volumeCenter;
	Vec4 volumeExtend;
	Vec4 voxelResolution;
	Vec4 padding;
};

namespace VoxelizationPass
{
	bool setupVoxelizationPass();
	bool setupVisualizationPass();

	bool voxelization();
	bool visualization();

	GPUBufferDataComponent* m_voxelizationGBDC;

	RenderPassDataComponent* m_voxelizationRPDC;
	ShaderProgramComponent* m_voxelizationSPC;

	RenderPassDataComponent* m_visualizationRPDC;
	ShaderProgramComponent* m_visualizationSPC;

	uint32_t voxelizationResolution = 128;
}

bool VoxelizationPass::setupVoxelizationPass()
{
	m_voxelizationGBDC = g_pModuleManager->getRenderingServer()->AddGPUBufferDataComponent("VoxelizationPassGPUBuffer/");
	m_voxelizationGBDC->m_ElementCount = 1;
	m_voxelizationGBDC->m_ElementSize = sizeof(VoxelizationConstantBuffer);
	m_voxelizationGBDC->m_BindingPoint = 12;

	////
	m_voxelizationSPC = g_pModuleManager->getRenderingServer()->AddShaderProgramComponent("VoxelizationPass/");

	m_voxelizationSPC->m_ShaderFilePaths.m_VSPath = "voxelizationPass.vert/";
	m_voxelizationSPC->m_ShaderFilePaths.m_GSPath = "voxelizationPass.geom/";
	m_voxelizationSPC->m_ShaderFilePaths.m_PSPath = "voxelizationPass.frag/";

	m_voxelizationRPDC = g_pModuleManager->getRenderingServer()->AddRenderPassDataComponent("VoxelizationPass/");

	auto l_RenderPassDesc = g_pModuleManager->getRenderingFrontend()->getDefaultRenderPassDesc();

	l_RenderPassDesc.m_RenderTargetCount = 1;
	l_RenderPassDesc.m_IsOffScreen = true;

	l_RenderPassDesc.m_RenderTargetDesc.Sampler = TextureSampler::Sampler3D;
	l_RenderPassDesc.m_RenderTargetDesc.Usage = TextureUsage::RawImage;
	l_RenderPassDesc.m_RenderTargetDesc.GPUAccessibility = Accessibility::ReadWrite;
	l_RenderPassDesc.m_RenderTargetDesc.Width = voxelizationResolution;
	l_RenderPassDesc.m_RenderTargetDesc.Height = voxelizationResolution;
	l_RenderPassDesc.m_RenderTargetDesc.DepthOrArraySize = voxelizationResolution;
	l_RenderPassDesc.m_GraphicsPipelineDesc.m_RasterizerDesc.m_UseCulling = false;
	l_RenderPassDesc.m_GraphicsPipelineDesc.m_ViewportDesc.m_Width = (float)voxelizationResolution;
	l_RenderPassDesc.m_GraphicsPipelineDesc.m_ViewportDesc.m_Height = (float)voxelizationResolution;

	m_voxelizationRPDC->m_RenderPassDesc = l_RenderPassDesc;

	m_voxelizationRPDC->m_ResourceBinderLayoutDescs.resize(5);
	m_voxelizationRPDC->m_ResourceBinderLayoutDescs[0].m_ResourceBinderType = ResourceBinderType::Buffer;
	m_voxelizationRPDC->m_ResourceBinderLayoutDescs[0].m_DescriptorSetIndex = 0;
	m_voxelizationRPDC->m_ResourceBinderLayoutDescs[0].m_DescriptorIndex = 0;

	m_voxelizationRPDC->m_ResourceBinderLayoutDescs[1].m_ResourceBinderType = ResourceBinderType::Buffer;
	m_voxelizationRPDC->m_ResourceBinderLayoutDescs[1].m_DescriptorSetIndex = 1;
	m_voxelizationRPDC->m_ResourceBinderLayoutDescs[1].m_DescriptorIndex = 1;

	m_voxelizationRPDC->m_ResourceBinderLayoutDescs[2].m_ResourceBinderType = ResourceBinderType::Buffer;
	m_voxelizationRPDC->m_ResourceBinderLayoutDescs[2].m_DescriptorSetIndex = 2;
	m_voxelizationRPDC->m_ResourceBinderLayoutDescs[2].m_DescriptorIndex = 2;

	m_voxelizationRPDC->m_ResourceBinderLayoutDescs[3].m_ResourceBinderType = ResourceBinderType::Buffer;
	m_voxelizationRPDC->m_ResourceBinderLayoutDescs[3].m_DescriptorSetIndex = 3;
	m_voxelizationRPDC->m_ResourceBinderLayoutDescs[3].m_DescriptorIndex = 9;

	m_voxelizationRPDC->m_ResourceBinderLayoutDescs[4].m_ResourceBinderType = ResourceBinderType::Image;
	m_voxelizationRPDC->m_ResourceBinderLayoutDescs[4].m_BinderAccessibility = Accessibility::ReadWrite;
	m_voxelizationRPDC->m_ResourceBinderLayoutDescs[4].m_ResourceAccessibility = Accessibility::ReadWrite;
	m_voxelizationRPDC->m_ResourceBinderLayoutDescs[4].m_DescriptorSetIndex = 4;
	m_voxelizationRPDC->m_ResourceBinderLayoutDescs[4].m_DescriptorIndex = 0;
	m_voxelizationRPDC->m_ResourceBinderLayoutDescs[4].m_IndirectBinding = true;

	m_voxelizationRPDC->m_ShaderProgram = m_voxelizationSPC;

	return true;
}

bool VoxelizationPass::setupVisualizationPass()
{
	m_visualizationSPC = g_pModuleManager->getRenderingServer()->AddShaderProgramComponent("VoxelVisualizationPass/");

	m_visualizationSPC->m_ShaderFilePaths.m_VSPath = "voxelVisualizationPass.vert/";
	m_visualizationSPC->m_ShaderFilePaths.m_GSPath = "voxelVisualizationPass.geom/";
	m_visualizationSPC->m_ShaderFilePaths.m_PSPath = "voxelVisualizationPass.frag/";

	m_visualizationRPDC = g_pModuleManager->getRenderingServer()->AddRenderPassDataComponent("VoxelVisualizationPass/");

	auto l_RenderPassDesc = g_pModuleManager->getRenderingFrontend()->getDefaultRenderPassDesc();
	auto l_viewportSize = g_pModuleManager->getRenderingFrontend()->getScreenResolution();

	l_RenderPassDesc.m_RenderTargetCount = 1;

	l_RenderPassDesc.m_RenderTargetDesc.Sampler = TextureSampler::Sampler2D;
	l_RenderPassDesc.m_RenderTargetDesc.Usage = TextureUsage::ColorAttachment;
	l_RenderPassDesc.m_RenderTargetDesc.Width = l_viewportSize.x;
	l_RenderPassDesc.m_RenderTargetDesc.Height = l_viewportSize.y;
	l_RenderPassDesc.m_GraphicsPipelineDesc.m_DepthStencilDesc.m_UseDepthBuffer = true;
	l_RenderPassDesc.m_GraphicsPipelineDesc.m_DepthStencilDesc.m_AllowDepthWrite = true;
	l_RenderPassDesc.m_GraphicsPipelineDesc.m_DepthStencilDesc.m_DepthComparisionFunction = ComparisionFunction::LessEqual;
	l_RenderPassDesc.m_GraphicsPipelineDesc.m_RasterizerDesc.m_PrimitiveTopology = PrimitiveTopology::Point;
	l_RenderPassDesc.m_GraphicsPipelineDesc.m_RasterizerDesc.m_UseCulling = true;
	l_RenderPassDesc.m_GraphicsPipelineDesc.m_ViewportDesc.m_Width = (float)l_viewportSize.x;
	l_RenderPassDesc.m_GraphicsPipelineDesc.m_ViewportDesc.m_Height = (float)l_viewportSize.y;

	m_visualizationRPDC->m_RenderPassDesc = l_RenderPassDesc;

	m_visualizationRPDC->m_ResourceBinderLayoutDescs.resize(3);

	m_visualizationRPDC->m_ResourceBinderLayoutDescs[0].m_ResourceBinderType = ResourceBinderType::Image;
	m_visualizationRPDC->m_ResourceBinderLayoutDescs[0].m_BinderAccessibility = Accessibility::ReadOnly;
	m_visualizationRPDC->m_ResourceBinderLayoutDescs[0].m_ResourceAccessibility = Accessibility::ReadWrite;
	m_visualizationRPDC->m_ResourceBinderLayoutDescs[0].m_DescriptorSetIndex = 0;
	m_visualizationRPDC->m_ResourceBinderLayoutDescs[0].m_DescriptorIndex = 0;
	m_visualizationRPDC->m_ResourceBinderLayoutDescs[0].m_IndirectBinding = true;

	m_visualizationRPDC->m_ResourceBinderLayoutDescs[1].m_ResourceBinderType = ResourceBinderType::Buffer;
	m_visualizationRPDC->m_ResourceBinderLayoutDescs[1].m_DescriptorSetIndex = 1;
	m_visualizationRPDC->m_ResourceBinderLayoutDescs[1].m_DescriptorIndex = 0;

	m_visualizationRPDC->m_ResourceBinderLayoutDescs[2].m_ResourceBinderType = ResourceBinderType::Buffer;
	m_visualizationRPDC->m_ResourceBinderLayoutDescs[2].m_DescriptorSetIndex = 2;
	m_visualizationRPDC->m_ResourceBinderLayoutDescs[2].m_DescriptorIndex = 9;

	m_visualizationRPDC->m_ShaderProgram = m_visualizationSPC;

	return true;
}

bool VoxelizationPass::Setup()
{
	setupVoxelizationPass();
	setupVisualizationPass();

	return true;
}

bool VoxelizationPass::Initialize()
{
	g_pModuleManager->getRenderingServer()->InitializeGPUBufferDataComponent(m_voxelizationGBDC);

	g_pModuleManager->getRenderingServer()->InitializeShaderProgramComponent(m_voxelizationSPC);
	g_pModuleManager->getRenderingServer()->InitializeRenderPassDataComponent(m_voxelizationRPDC);

	g_pModuleManager->getRenderingServer()->InitializeShaderProgramComponent(m_visualizationSPC);
	g_pModuleManager->getRenderingServer()->InitializeRenderPassDataComponent(m_visualizationRPDC);

	return true;
}

bool VoxelizationPass::voxelization()
{
	auto l_PerFrameCBufferGBDC = GetGPUBufferDataComponent(GPUBufferUsageType::PerFrame);
	auto l_MeshGBDC = GetGPUBufferDataComponent(GPUBufferUsageType::Mesh);
	auto l_MaterialGBDC = GetGPUBufferDataComponent(GPUBufferUsageType::Material);

	g_pModuleManager->getRenderingServer()->CommandListBegin(m_voxelizationRPDC, 0);
	g_pModuleManager->getRenderingServer()->BindRenderPassDataComponent(m_voxelizationRPDC);
	g_pModuleManager->getRenderingServer()->CleanRenderTargets(m_voxelizationRPDC);

	g_pModuleManager->getRenderingServer()->ActivateResourceBinder(m_voxelizationRPDC, ShaderStage::Vertex, l_PerFrameCBufferGBDC->m_ResourceBinder, 0, 0, Accessibility::ReadOnly);
	g_pModuleManager->getRenderingServer()->ActivateResourceBinder(m_voxelizationRPDC, ShaderStage::Geometry, m_voxelizationGBDC->m_ResourceBinder, 3, 9, Accessibility::ReadOnly);
	g_pModuleManager->getRenderingServer()->ActivateResourceBinder(m_voxelizationRPDC, ShaderStage::Pixel, m_voxelizationRPDC->m_RenderTargetsResourceBinders[0], 4, 0, Accessibility::ReadWrite);

	auto& l_drawCallInfo = g_pModuleManager->getRenderingFrontend()->getDrawCallInfo();
	auto l_drawCallCount = l_drawCallInfo.size();

	for (uint32_t i = 0; i < l_drawCallCount; i++)
	{
		auto l_drawCallData = l_drawCallInfo[i];
		if (l_drawCallData.visibilityType == VisibilityType::Opaque)
		{
			if (l_drawCallData.mesh->m_ObjectStatus == ObjectStatus::Activated)
			{
				g_pModuleManager->getRenderingServer()->ActivateResourceBinder(m_voxelizationRPDC, ShaderStage::Vertex, l_MeshGBDC->m_ResourceBinder, 1, 1, Accessibility::ReadOnly, l_drawCallData.meshConstantBufferIndex, 1);
				g_pModuleManager->getRenderingServer()->ActivateResourceBinder(m_voxelizationRPDC, ShaderStage::Pixel, l_MaterialGBDC->m_ResourceBinder, 2, 2, Accessibility::ReadOnly, l_drawCallData.materialConstantBufferIndex, 1);

				g_pModuleManager->getRenderingServer()->DispatchDrawCall(m_voxelizationRPDC, l_drawCallData.mesh);
			}
		}
	}

	g_pModuleManager->getRenderingServer()->DeactivateResourceBinder(m_voxelizationRPDC, ShaderStage::Pixel, m_voxelizationRPDC->m_RenderTargetsResourceBinders[0], 4, 0, Accessibility::ReadWrite);

	g_pModuleManager->getRenderingServer()->CommandListEnd(m_voxelizationRPDC);

	return true;
}

bool VoxelizationPass::visualization()
{
	auto l_PerFrameCBufferGBDC = GetGPUBufferDataComponent(GPUBufferUsageType::PerFrame);

	g_pModuleManager->getRenderingServer()->CommandListBegin(m_visualizationRPDC, 0);
	g_pModuleManager->getRenderingServer()->BindRenderPassDataComponent(m_visualizationRPDC);
	g_pModuleManager->getRenderingServer()->CleanRenderTargets(m_visualizationRPDC);

	g_pModuleManager->getRenderingServer()->ActivateResourceBinder(m_visualizationRPDC, ShaderStage::Vertex, m_voxelizationRPDC->m_RenderTargetsResourceBinders[0], 0, 0, Accessibility::ReadOnly);
	g_pModuleManager->getRenderingServer()->ActivateResourceBinder(m_visualizationRPDC, ShaderStage::Geometry, l_PerFrameCBufferGBDC->m_ResourceBinder, 1, 0, Accessibility::ReadOnly);
	g_pModuleManager->getRenderingServer()->ActivateResourceBinder(m_visualizationRPDC, ShaderStage::Geometry, m_voxelizationGBDC->m_ResourceBinder, 2, 9, Accessibility::ReadOnly);

	g_pModuleManager->getRenderingServer()->DispatchDrawCall(m_visualizationRPDC, voxelizationResolution * voxelizationResolution * voxelizationResolution);

	g_pModuleManager->getRenderingServer()->DeactivateResourceBinder(m_visualizationRPDC, ShaderStage::Vertex, m_voxelizationRPDC->m_RenderTargetsResourceBinders[0], 0, 0, Accessibility::ReadOnly);

	g_pModuleManager->getRenderingServer()->CommandListEnd(m_visualizationRPDC);

	return true;
}

bool VoxelizationPass::PrepareCommandList()
{
	auto l_p = InnoMath::generateOrthographicMatrix(-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 2.0f);
	auto l_sceneAABB = g_pModuleManager->getPhysicsSystem()->getTotalSceneAABB();

	VoxelizationConstantBuffer l_voxelPassCB;
	l_voxelPassCB.volumeCenter = l_sceneAABB.m_center;
	l_voxelPassCB.volumeExtend = l_sceneAABB.m_extend;
	l_voxelPassCB.voxelResolution = Vec4((float)voxelizationResolution, (float)voxelizationResolution, (float)voxelizationResolution, 1.0f);

	g_pModuleManager->getRenderingServer()->UploadGPUBufferDataComponent(m_voxelizationGBDC, &l_voxelPassCB);

	voxelization();
	visualization();

	return true;
}

bool VoxelizationPass::ExecuteCommandList()
{
	g_pModuleManager->getRenderingServer()->ExecuteCommandList(m_voxelizationRPDC);
	g_pModuleManager->getRenderingServer()->WaitForFrame(m_voxelizationRPDC);

	g_pModuleManager->getRenderingServer()->ExecuteCommandList(m_visualizationRPDC);
	g_pModuleManager->getRenderingServer()->WaitForFrame(m_visualizationRPDC);

	return true;
}

bool VoxelizationPass::Terminate()
{
	g_pModuleManager->getRenderingServer()->DeleteRenderPassDataComponent(m_voxelizationRPDC);
	g_pModuleManager->getRenderingServer()->DeleteRenderPassDataComponent(m_visualizationRPDC);

	return true;
}

IResourceBinder* VoxelizationPass::GetVisualizationResult()
{
	return m_visualizationRPDC->m_RenderTargets[0]->m_ResourceBinder;
}