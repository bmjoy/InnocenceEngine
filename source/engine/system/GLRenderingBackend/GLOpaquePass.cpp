#include "GLOpaquePass.h"
#include "GLEarlyZPass.h"
#include "GLRenderingSystemUtilities.h"
#include "../../component/GLRenderingSystemComponent.h"
#include "../../component/RenderingFrontendSystemComponent.h"

using namespace GLRenderingSystemNS;

#include "../ICoreSystem.h"

extern ICoreSystem* g_pCoreSystem;

INNO_PRIVATE_SCOPE GLOpaquePass
{
	void initializeShaders();

	EntityID m_entityID;

	GLRenderPassComponent* m_GLRPC;

	GLShaderProgramComponent* m_GLSPC;

	ShaderFilePaths m_shaderFilePaths = { "GL//opaquePass.vert/" , "", "", "", "GL//opaquePass.frag/" };
}

bool GLOpaquePass::initialize()
{
	m_entityID = InnoMath::createEntityID();

	m_GLRPC = addGLRenderPassComponent(m_entityID, "OpaquePassGLRPC/");
	m_GLRPC->m_renderPassDesc = GLRenderingSystemComponent::get().m_deferredRenderPassDesc;
	m_GLRPC->m_renderPassDesc.RTNumber = 4;
	m_GLRPC->m_renderPassDesc.useDepthAttachment = true;
	m_GLRPC->m_renderPassDesc.useStencilAttachment = true;

	initializeGLRenderPassComponent(m_GLRPC);

	initializeShaders();

	return true;
}

void GLOpaquePass::initializeShaders()
{
	// shader programs and shaders
	auto rhs = addGLShaderProgramComponent(m_entityID);

	initializeGLShaderProgramComponent(rhs, m_shaderFilePaths);

	m_GLSPC = rhs;
}

bool GLOpaquePass::update()
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_FALSE);

	glEnable(GL_DEPTH_CLAMP);

	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glStencilMask(0xFF);
	glStencilFunc(GL_ALWAYS, 0x01, 0xFF);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	activateRenderPass(m_GLRPC);

	copyDepthBuffer(GLEarlyZPass::getGLRPC(), m_GLRPC);

	activateShaderProgram(m_GLSPC);

	for (unsigned int i = 0; i < RenderingFrontendSystemComponent::get().m_opaquePassDrawcallCount; i++)
	{
		auto l_opaquePassGPUData = RenderingFrontendSystemComponent::get().m_opaquePassGPUDatas[i];
		auto l_meshGPUData = RenderingFrontendSystemComponent::get().m_opaquePassMeshGPUDatas[i];
		auto l_materialGPUData = RenderingFrontendSystemComponent::get().m_opaquePassMaterialGPUDatas[i];

		if (l_materialGPUData.useNormalTexture)
		{
			activateTexture(reinterpret_cast<GLTextureDataComponent*>(l_opaquePassGPUData.normalTDC), 0);
		}
		if (l_materialGPUData.useAlbedoTexture)
		{
			activateTexture(reinterpret_cast<GLTextureDataComponent*>(l_opaquePassGPUData.albedoTDC), 1);
		}
		if (l_materialGPUData.useMetallicTexture)
		{
			activateTexture(reinterpret_cast<GLTextureDataComponent*>(l_opaquePassGPUData.metallicTDC), 2);
		}
		if (l_materialGPUData.useRoughnessTexture)
		{
			activateTexture(reinterpret_cast<GLTextureDataComponent*>(l_opaquePassGPUData.roughnessTDC), 3);
		}
		if (l_materialGPUData.useAOTexture)
		{
			activateTexture(reinterpret_cast<GLTextureDataComponent*>(l_opaquePassGPUData.AOTDC), 4);
		}

		updateUBO(GLRenderingSystemComponent::get().m_meshUBO, l_meshGPUData);
		updateUBO(GLRenderingSystemComponent::get().m_materialUBO, l_materialGPUData);

		drawMesh(reinterpret_cast<GLMeshDataComponent*>(l_opaquePassGPUData.MDC));
	}

	glDisable(GL_CULL_FACE);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_DEPTH_CLAMP);
	glDisable(GL_DEPTH_TEST);

	return true;
}

bool GLOpaquePass::resize(unsigned int newSizeX, unsigned int newSizeY)
{
	resizeGLRenderPassComponent(m_GLRPC, newSizeX, newSizeY);

	return true;
}

bool GLOpaquePass::reloadShader()
{
	deleteShaderProgram(m_GLSPC);

	initializeGLShaderProgramComponent(m_GLSPC, m_shaderFilePaths);

	return true;
}

GLRenderPassComponent * GLOpaquePass::getGLRPC()
{
	return m_GLRPC;
}

GLShaderProgramComponent * GLOpaquePass::getGLSPC()
{
	return m_GLSPC;
}