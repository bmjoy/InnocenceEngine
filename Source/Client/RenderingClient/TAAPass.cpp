#include "TAAPass.h"
#include "DefaultGPUBuffers.h"

#include "PreTAAPass.h"
#include "OpaquePass.h"

#include "../../Engine/ModuleManager/IModuleManager.h"

INNO_ENGINE_API extern IModuleManager* g_pModuleManager;

using namespace DefaultGPUBuffers;

namespace TAAPass
{
	RenderPassDataComponent* m_RPDC_A;
	RenderPassDataComponent* m_RPDC_B;
	ShaderProgramComponent* m_SPC;
	SamplerDataComponent* m_SDC;

	bool l_isPassA = true;
}

bool TAAPass::Setup()
{
	m_SPC = g_pModuleManager->getRenderingServer()->AddShaderProgramComponent("TAAPass/");

	m_SPC->m_ShaderFilePaths.m_VSPath = "2DImageProcess.vert/";
	m_SPC->m_ShaderFilePaths.m_PSPath = "TAAPass.frag/";

	g_pModuleManager->getRenderingServer()->InitializeShaderProgramComponent(m_SPC);

	m_RPDC_A = g_pModuleManager->getRenderingServer()->AddRenderPassDataComponent("TAAPassA/");
	m_RPDC_B = g_pModuleManager->getRenderingServer()->AddRenderPassDataComponent("TAAPassB/");

	auto l_RenderPassDesc = g_pModuleManager->getRenderingFrontend()->getDefaultRenderPassDesc();

	l_RenderPassDesc.m_RenderTargetCount = 1;

	l_RenderPassDesc.m_GraphicsPipelineDesc.m_RasterizerDesc.m_UseCulling = false;

	m_RPDC_A->m_RenderPassDesc = l_RenderPassDesc;
	m_RPDC_B->m_RenderPassDesc = l_RenderPassDesc;

	std::vector<ResourceBinderLayoutDesc> l_ResourceBinderLayoutDescs(4);
	l_ResourceBinderLayoutDescs[0].m_ResourceBinderType = ResourceBinderType::Image;
	l_ResourceBinderLayoutDescs[0].m_GlobalSlot = 0;
	l_ResourceBinderLayoutDescs[0].m_LocalSlot = 0;
	l_ResourceBinderLayoutDescs[0].m_IsRanged = true;

	l_ResourceBinderLayoutDescs[1].m_ResourceBinderType = ResourceBinderType::Image;
	l_ResourceBinderLayoutDescs[1].m_GlobalSlot = 1;
	l_ResourceBinderLayoutDescs[1].m_LocalSlot = 1;
	l_ResourceBinderLayoutDescs[1].m_IsRanged = true;

	l_ResourceBinderLayoutDescs[2].m_ResourceBinderType = ResourceBinderType::Image;
	l_ResourceBinderLayoutDescs[2].m_GlobalSlot = 2;
	l_ResourceBinderLayoutDescs[2].m_LocalSlot = 2;
	l_ResourceBinderLayoutDescs[2].m_IsRanged = true;

	l_ResourceBinderLayoutDescs[3].m_ResourceBinderType = ResourceBinderType::Sampler;
	l_ResourceBinderLayoutDescs[3].m_GlobalSlot = 3;
	l_ResourceBinderLayoutDescs[3].m_LocalSlot = 0;
	l_ResourceBinderLayoutDescs[3].m_IsRanged = true;

	m_RPDC_A->m_ResourceBinderLayoutDescs = l_ResourceBinderLayoutDescs;
	m_RPDC_B->m_ResourceBinderLayoutDescs = l_ResourceBinderLayoutDescs;

	m_RPDC_A->m_ShaderProgram = m_SPC;
	m_RPDC_B->m_ShaderProgram = m_SPC;

	g_pModuleManager->getRenderingServer()->InitializeRenderPassDataComponent(m_RPDC_A);
	g_pModuleManager->getRenderingServer()->InitializeRenderPassDataComponent(m_RPDC_B);

	m_SDC = g_pModuleManager->getRenderingServer()->AddSamplerDataComponent("TAAPass/");

	g_pModuleManager->getRenderingServer()->InitializeSamplerDataComponent(m_SDC);

	return true;
}

bool TAAPass::Initialize()
{
	return true;
}

bool TAAPass::PrepareCommandList()
{
	RenderPassDataComponent* l_WriteRPDC;
	RenderPassDataComponent* l_ReadRPDC;

	if (l_isPassA)
	{
		l_WriteRPDC = m_RPDC_A;
		l_ReadRPDC = m_RPDC_B;
		l_isPassA = false;
	}
	else
	{
		l_WriteRPDC = m_RPDC_B;
		l_ReadRPDC = m_RPDC_A;
		l_isPassA = true;
	}

	g_pModuleManager->getRenderingServer()->CommandListBegin(l_WriteRPDC, 0);
	g_pModuleManager->getRenderingServer()->BindRenderPassDataComponent(l_WriteRPDC);
	g_pModuleManager->getRenderingServer()->CleanRenderTargets(l_WriteRPDC);
	g_pModuleManager->getRenderingServer()->ActivateResourceBinder(l_WriteRPDC, ShaderStage::Pixel, m_SDC->m_ResourceBinder, 3, 0);

	g_pModuleManager->getRenderingServer()->ActivateResourceBinder(l_WriteRPDC, ShaderStage::Pixel, PreTAAPass::GetRPDC()->m_RenderTargetsResourceBinders[0], 0, 0);
	g_pModuleManager->getRenderingServer()->ActivateResourceBinder(l_WriteRPDC, ShaderStage::Pixel, l_ReadRPDC->m_RenderTargetsResourceBinders[0], 1, 1);
	g_pModuleManager->getRenderingServer()->ActivateResourceBinder(l_WriteRPDC, ShaderStage::Pixel, OpaquePass::GetRPDC()->m_RenderTargetsResourceBinders[3], 2, 2);

	auto l_mesh = g_pModuleManager->getRenderingFrontend()->getMeshDataComponent(MeshShapeType::Quad);

	g_pModuleManager->getRenderingServer()->DispatchDrawCall(l_WriteRPDC, l_mesh);

	g_pModuleManager->getRenderingServer()->DeactivateResourceBinder(l_WriteRPDC, ShaderStage::Pixel, PreTAAPass::GetRPDC()->m_RenderTargetsResourceBinders[0], 0, 0);
	g_pModuleManager->getRenderingServer()->DeactivateResourceBinder(l_WriteRPDC, ShaderStage::Pixel, l_ReadRPDC->m_RenderTargetsResourceBinders[0], 1, 1);
	g_pModuleManager->getRenderingServer()->ActivateResourceBinder(l_WriteRPDC, ShaderStage::Pixel, OpaquePass::GetRPDC()->m_RenderTargetsResourceBinders[3], 2, 2);

	g_pModuleManager->getRenderingServer()->CommandListEnd(l_WriteRPDC);

	g_pModuleManager->getRenderingServer()->ExecuteCommandList(l_WriteRPDC);

	g_pModuleManager->getRenderingServer()->WaitForFrame(l_WriteRPDC);

	return true;
}

RenderPassDataComponent * TAAPass::GetRPDC()
{
	if (l_isPassA)
	{
		return m_RPDC_A;
	}
	else
	{
		return m_RPDC_B;
	}
}

ShaderProgramComponent * TAAPass::GetSPC()
{
	return m_SPC;
}