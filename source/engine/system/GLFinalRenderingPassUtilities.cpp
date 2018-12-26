#include "GLRenderingSystemUtilities.h"
#include "GLFinalRenderingPassUtilities.h"
#include "../component/GLFinalRenderPassComponent.h"
#include "../component/RenderingSystemComponent.h"
#include "../component/GLRenderingSystemComponent.h"
#include "../component/PhysicsSystemComponent.h"
#include "../component/GameSystemComponent.h"
#include "../component/GLGeometryRenderPassComponent.h"
#include "../component/GLTerrainRenderPassComponent.h"
#include "../component/GLLightRenderPassComponent.h"

#include "ICoreSystem.h"

extern ICoreSystem* g_pCoreSystem;

INNO_PRIVATE_SCOPE GLRenderingSystemNS
{
	void initializeSkyPass();
	void initializeTAAPass();
	void initializeBloomExtractPass();
	void initializeBloomBlurPass();
	void initializeMotionBlurPass();
	void initializeBillboardPass();
	void initializeDebuggerPass();
	void initializeFinalBlendPass();

	GLTextureDataComponent* updateSkyPass();
	GLTextureDataComponent* updatePreTAAPass();
	GLTextureDataComponent* updateTAAPass(GLTextureDataComponent* inputGLTDC);
	GLTextureDataComponent* updateTAASharpenPass(GLTextureDataComponent* inputGLTDC);
	GLTextureDataComponent* updateBloomExtractPass(GLTextureDataComponent* inputGLTDC);
	GLTextureDataComponent* updateBloomBlurPass(GLTextureDataComponent* inputGLTDC);
	GLTextureDataComponent* updateMotionBlurPass(GLTextureDataComponent * inputGLTDC);
	GLTextureDataComponent* updateBillboardPass();
	GLTextureDataComponent* updateDebuggerPass();
	GLTextureDataComponent* updateFinalBlendPass();
}

void GLRenderingSystemNS::initializeFinalPass()
{
	initializeSkyPass();
	initializeTAAPass();
	initializeBloomExtractPass();
	initializeBloomBlurPass();
	initializeMotionBlurPass();
	initializeBillboardPass();
	initializeDebuggerPass();
	initializeFinalBlendPass();
}

void GLRenderingSystemNS::initializeSkyPass()
{
	GLFinalRenderPassComponent::get().m_skyPassGLRPC = addGLRenderPassComponent(1, GLRenderingSystemComponent::get().deferredPassFBDesc, GLRenderingSystemComponent::get().deferredPassTextureDesc);

	// shader programs and shaders
	auto rhs = addGLShaderProgramComponent(0);

	initializeGLShaderProgramComponent(rhs, GLFinalRenderPassComponent::get().m_skyPassShaderFilePaths);

	GLFinalRenderPassComponent::get().m_skyPass_uni_p = getUniformLocation(
		rhs->m_program,
		"uni_p");
	GLFinalRenderPassComponent::get().m_skyPass_uni_r = getUniformLocation(
		rhs->m_program,
		"uni_r");
	GLFinalRenderPassComponent::get().m_skyPass_uni_viewportSize = getUniformLocation(
		rhs->m_program,
		"uni_viewportSize");
	GLFinalRenderPassComponent::get().m_skyPass_uni_eyePos = getUniformLocation(
		rhs->m_program,
		"uni_eyePos");
	GLFinalRenderPassComponent::get().m_skyPass_uni_lightDir = getUniformLocation(
		rhs->m_program,
		"uni_lightDir");

	GLFinalRenderPassComponent::get().m_skyPassSPC = rhs;
}

void GLRenderingSystemNS::initializeTAAPass()
{
	// pre mix pass
	GLFinalRenderPassComponent::get().m_preTAAPassGLRPC = addGLRenderPassComponent(1, GLRenderingSystemComponent::get().deferredPassFBDesc, GLRenderingSystemComponent::get().deferredPassTextureDesc);

	// Ping pass
	GLFinalRenderPassComponent::get().m_TAAPingPassGLRPC = addGLRenderPassComponent(1, GLRenderingSystemComponent::get().deferredPassFBDesc, GLRenderingSystemComponent::get().deferredPassTextureDesc);

	// Pong pass
	GLFinalRenderPassComponent::get().m_TAAPongPassGLRPC = addGLRenderPassComponent(1, GLRenderingSystemComponent::get().deferredPassFBDesc, GLRenderingSystemComponent::get().deferredPassTextureDesc);

	// Sharpen pass
	GLFinalRenderPassComponent::get().m_TAASharpenPassGLRPC = addGLRenderPassComponent(1, GLRenderingSystemComponent::get().deferredPassFBDesc, GLRenderingSystemComponent::get().deferredPassTextureDesc);

	// shader programs and shaders
	// pre mix pass
	auto rhs = addGLShaderProgramComponent(0);

	initializeGLShaderProgramComponent(rhs, GLFinalRenderPassComponent::get().m_preTAAPassShaderFilePaths);

	updateTextureUniformLocations(rhs->m_program, GLFinalRenderPassComponent::get().m_preTAAPassUniformNames);

	GLFinalRenderPassComponent::get().m_preTAAPassSPC = rhs;

	// TAA pass	
	rhs = addGLShaderProgramComponent(0);

	initializeGLShaderProgramComponent(rhs, GLFinalRenderPassComponent::get().m_TAAPassShaderFilePaths);

	updateTextureUniformLocations(rhs->m_program, GLFinalRenderPassComponent::get().m_TAAPassUniformNames);

	GLFinalRenderPassComponent::get().m_TAAPass_uni_renderTargetSize = getUniformLocation(
		rhs->m_program,
		"uni_renderTargetSize");

	GLFinalRenderPassComponent::get().m_TAAPassSPC = rhs;

	// Sharpen pass
	rhs = addGLShaderProgramComponent(0);

	initializeGLShaderProgramComponent(rhs, GLFinalRenderPassComponent::get().m_TAASharpenPassShaderFilePaths);

	updateTextureUniformLocations(rhs->m_program, GLFinalRenderPassComponent::get().m_TAASharpenPassUniformNames);

	GLFinalRenderPassComponent::get().m_TAASharpenPass_uni_renderTargetSize = getUniformLocation(
		rhs->m_program,
		"uni_renderTargetSize");

	GLFinalRenderPassComponent::get().m_TAASharpenPassSPC = rhs;
}

void GLRenderingSystemNS::initializeBloomExtractPass()
{
	GLFinalRenderPassComponent::get().m_bloomExtractPassGLRPC = addGLRenderPassComponent(1, GLRenderingSystemComponent::get().deferredPassFBDesc, GLRenderingSystemComponent::get().deferredPassTextureDesc);

	// shader programs and shaders
	ShaderFilePaths m_ShaderFilePaths;

	m_ShaderFilePaths.m_VSPath = "GL4.0//bloomExtractPassVertex.sf";
	m_ShaderFilePaths.m_FSPath = "GL4.0//bloomExtractPassFragment.sf";

	auto rhs = addGLShaderProgramComponent(0); initializeGLShaderProgramComponent(rhs, m_ShaderFilePaths);

	GLFinalRenderPassComponent::get().m_bloomExtractPass_uni_TAAPassRT0 = getUniformLocation(
		rhs->m_program,
		"uni_TAAPassRT0");
	updateUniform(
		GLFinalRenderPassComponent::get().m_bloomExtractPass_uni_TAAPassRT0,
		0);

	GLFinalRenderPassComponent::get().m_bloomExtractPassSPC = rhs;
}

void GLRenderingSystemNS::initializeBloomBlurPass()
{
	//Ping pass
	GLFinalRenderPassComponent::get().m_bloomBlurPingPassGLRPC = addGLRenderPassComponent(1, GLRenderingSystemComponent::get().deferredPassFBDesc, GLRenderingSystemComponent::get().deferredPassTextureDesc);

	//Pong pass
	GLFinalRenderPassComponent::get().m_bloomBlurPongPassGLRPC = addGLRenderPassComponent(1, GLRenderingSystemComponent::get().deferredPassFBDesc, GLRenderingSystemComponent::get().deferredPassTextureDesc);

	// shader programs and shaders
	ShaderFilePaths m_ShaderFilePaths;

	m_ShaderFilePaths.m_VSPath = "GL4.0//bloomBlurPassVertex.sf";
	m_ShaderFilePaths.m_FSPath = "GL4.0//bloomBlurPassFragment.sf";

	auto rhs = addGLShaderProgramComponent(0); initializeGLShaderProgramComponent(rhs, m_ShaderFilePaths);

	GLFinalRenderPassComponent::get().m_bloomBlurPass_uni_bloomExtractPassRT0 = getUniformLocation(
		rhs->m_program,
		"uni_bloomExtractPassRT0");
	updateUniform(
		GLFinalRenderPassComponent::get().m_bloomBlurPass_uni_bloomExtractPassRT0,
		0);
	GLFinalRenderPassComponent::get().m_bloomBlurPass_uni_horizontal = getUniformLocation(
		rhs->m_program,
		"uni_horizontal");

	GLFinalRenderPassComponent::get().m_bloomBlurPassSPC = rhs;
}

void GLRenderingSystemNS::initializeMotionBlurPass()
{
	GLFinalRenderPassComponent::get().m_motionBlurPassGLRPC = addGLRenderPassComponent(1, GLRenderingSystemComponent::get().deferredPassFBDesc, GLRenderingSystemComponent::get().deferredPassTextureDesc);

	// shader programs and shaders
	ShaderFilePaths m_ShaderFilePaths;

	m_ShaderFilePaths.m_VSPath = "GL4.0//motionBlurPassVertex.sf";
	m_ShaderFilePaths.m_FSPath = "GL4.0//motionBlurPassFragment.sf";

	auto rhs = addGLShaderProgramComponent(0); initializeGLShaderProgramComponent(rhs, m_ShaderFilePaths);

	GLFinalRenderPassComponent::get().m_motionBlurPass_uni_motionVectorTexture = getUniformLocation(
		rhs->m_program,
		"uni_motionVectorTexture");
	updateUniform(
		GLFinalRenderPassComponent::get().m_motionBlurPass_uni_motionVectorTexture,
		0);
	GLFinalRenderPassComponent::get().m_motionBlurPass_uni_TAAPassRT0 = getUniformLocation(
		rhs->m_program,
		"uni_TAAPassRT0");
	updateUniform(
		GLFinalRenderPassComponent::get().m_motionBlurPass_uni_TAAPassRT0,
		1);

	GLFinalRenderPassComponent::get().m_motionBlurPassSPC = rhs;
}

void GLRenderingSystemNS::initializeBillboardPass()
{
	GLFinalRenderPassComponent::get().m_billboardPassGLRPC = addGLRenderPassComponent(1, GLRenderingSystemComponent::get().deferredPassFBDesc, GLRenderingSystemComponent::get().deferredPassTextureDesc);

	// shader programs and shaders
	ShaderFilePaths m_ShaderFilePaths;

	m_ShaderFilePaths.m_VSPath = "GL4.0//billboardPassVertex.sf";
	m_ShaderFilePaths.m_FSPath = "GL4.0//billboardPassFragment.sf";

	auto rhs = addGLShaderProgramComponent(0); initializeGLShaderProgramComponent(rhs, m_ShaderFilePaths);

	GLFinalRenderPassComponent::get().m_billboardPass_uni_texture = getUniformLocation(
		rhs->m_program,
		"uni_texture");
	updateUniform(
		GLFinalRenderPassComponent::get().m_billboardPass_uni_texture,
		0);
	GLFinalRenderPassComponent::get().m_billboardPass_uni_p = getUniformLocation(
		rhs->m_program,
		"uni_p");
	GLFinalRenderPassComponent::get().m_billboardPass_uni_r = getUniformLocation(
		rhs->m_program,
		"uni_r");
	GLFinalRenderPassComponent::get().m_billboardPass_uni_t = getUniformLocation(
		rhs->m_program,
		"uni_t");
	GLFinalRenderPassComponent::get().m_billboardPass_uni_pos = getUniformLocation(
		rhs->m_program,
		"uni_pos");
	GLFinalRenderPassComponent::get().m_billboardPass_uni_albedo = getUniformLocation(
		rhs->m_program,
		"uni_albedo");
	GLFinalRenderPassComponent::get().m_billboardPass_uni_size = getUniformLocation(
		rhs->m_program,
		"uni_size");

	GLFinalRenderPassComponent::get().m_billboardPassSPC = rhs;
}

void GLRenderingSystemNS::initializeDebuggerPass()
{
	GLFinalRenderPassComponent::get().m_debuggerPassGLRPC = addGLRenderPassComponent(1, GLRenderingSystemComponent::get().deferredPassFBDesc, GLRenderingSystemComponent::get().deferredPassTextureDesc);

	// shader programs and shaders
	ShaderFilePaths m_ShaderFilePaths;

	//m_ShaderFilePaths.m_VSPath = "GL4.0//debuggerPassVertex.sf";
	//m_ShaderFilePaths.m_GSPath = "GL4.0//debuggerPassGeometry.sf";
	//m_ShaderFilePaths.m_FSPath = "GL4.0//debuggerPassFragment.sf";

	m_ShaderFilePaths.m_VSPath = "GL4.0//wireframeOverlayPassVertex.sf";
	m_ShaderFilePaths.m_FSPath = "GL4.0//wireframeOverlayPassFragment.sf";

	auto rhs = addGLShaderProgramComponent(0); initializeGLShaderProgramComponent(rhs, m_ShaderFilePaths);

	//GLFinalRenderPassComponent::get().m_debuggerPass_uni_normalTexture = getUniformLocation(
	//	rhs->m_program,
	//	"uni_normalTexture");
	//updateUniform(
	//	GLFinalRenderPassComponent::get().m_debuggerPass_uni_normalTexture,
	//	0);
	GLFinalRenderPassComponent::get().m_debuggerPass_uni_p = getUniformLocation(
		rhs->m_program,
		"uni_p");
	GLFinalRenderPassComponent::get().m_debuggerPass_uni_r = getUniformLocation(
		rhs->m_program,
		"uni_r");
	GLFinalRenderPassComponent::get().m_debuggerPass_uni_t = getUniformLocation(
		rhs->m_program,
		"uni_t");
	GLFinalRenderPassComponent::get().m_debuggerPass_uni_m = getUniformLocation(
		rhs->m_program,
		"uni_m");

	GLFinalRenderPassComponent::get().m_debuggerPassSPC = rhs;
}

void GLRenderingSystemNS::initializeFinalBlendPass()
{
	GLFinalRenderPassComponent::get().m_finalBlendPassGLRPC = addGLRenderPassComponent(1, GLRenderingSystemComponent::get().deferredPassFBDesc, GLRenderingSystemComponent::get().deferredPassTextureDesc);

	// shader programs and shaders
	ShaderFilePaths m_ShaderFilePaths;

	m_ShaderFilePaths.m_VSPath = "GL4.0//finalBlendPassVertex.sf";
	m_ShaderFilePaths.m_FSPath = "GL4.0//finalBlendPassFragment.sf";

	auto rhs = addGLShaderProgramComponent(0); initializeGLShaderProgramComponent(rhs, m_ShaderFilePaths);

	GLFinalRenderPassComponent::get().m_uni_motionBlurPassRT0 = getUniformLocation(
		rhs->m_program,
		"uni_motionBlurPassRT0");
	updateUniform(
		GLFinalRenderPassComponent::get().m_uni_motionBlurPassRT0,
		0);
	GLFinalRenderPassComponent::get().m_uni_bloomPassRT0 = getUniformLocation(
		rhs->m_program,
		"uni_bloomPassRT0");
	updateUniform(
		GLFinalRenderPassComponent::get().m_uni_bloomPassRT0,
		1);
	GLFinalRenderPassComponent::get().m_uni_billboardPassRT0 = getUniformLocation(
		rhs->m_program,
		"uni_billboardPassRT0");
	updateUniform(
		GLFinalRenderPassComponent::get().m_uni_billboardPassRT0,
		2);
	GLFinalRenderPassComponent::get().m_uni_debuggerPassRT0 = getUniformLocation(
		rhs->m_program,
		"uni_debuggerPassRT0");
	updateUniform(
		GLFinalRenderPassComponent::get().m_uni_debuggerPassRT0,
		3);
	GLFinalRenderPassComponent::get().m_finalBlendPassSPC = rhs;
}

void GLRenderingSystemNS::updateFinalRenderPass()
{
	auto skyPassResult = updateSkyPass();

	auto preTAAPassResult = updatePreTAAPass();

	GLTextureDataComponent* bloomInputGLTDC;

	if (RenderingSystemComponent::get().m_useTAA)
	{
		auto TAAPassResult = updateTAAPass(preTAAPassResult);

		auto TAASharpenPassResult = updateTAASharpenPass(TAAPassResult);

		bloomInputGLTDC = TAASharpenPassResult;
	}
	else
	{
		bloomInputGLTDC = preTAAPassResult;
	}

	if (RenderingSystemComponent::get().m_useBloom)
	{
		auto bloomExtractPassResult = updateBloomExtractPass(bloomInputGLTDC);

		//glEnable(GL_STENCIL_TEST);
		//glClear(GL_STENCIL_BUFFER_BIT);

		//glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		//glStencilFunc(GL_EQUAL, 0x02, 0xFF);
		//glStencilMask(0x00);

		//copyColorBuffer(GLLightRenderPassComponent::get().m_GLRPC->m_GLFBC,
		//	GLFinalRenderPassComponent::get().m_bloomExtractPassGLRPC->m_GLFBC);

		//glDisable(GL_STENCIL_TEST);

		auto bloomBlurPassResult = updateBloomBlurPass(bloomExtractPassResult);
	}
	else
	{
		cleanFBC(GLFinalRenderPassComponent::get().m_bloomExtractPassGLRPC->m_GLFBC);
		cleanFBC(GLFinalRenderPassComponent::get().m_bloomBlurPingPassGLRPC->m_GLFBC);
		cleanFBC(GLFinalRenderPassComponent::get().m_bloomBlurPongPassGLRPC->m_GLFBC);
	}

	updateMotionBlurPass(bloomInputGLTDC);

	updateBillboardPass();

	if (RenderingSystemComponent::get().m_drawOverlapWireframe)
	{
		updateDebuggerPass();
	}
	else
	{
		cleanFBC(GLFinalRenderPassComponent::get().m_debuggerPassGLRPC->m_GLFBC);
	}

	updateFinalBlendPass();
}

GLTextureDataComponent* GLRenderingSystemNS::updateSkyPass()
{
	if (RenderingSystemComponent::get().m_drawSky)
	{
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

		//glEnable(GL_CULL_FACE);
		//glFrontFace(GL_CW);
		//glCullFace(GL_FRONT);

		// bind to framebuffer
		auto l_FBC = GLFinalRenderPassComponent::get().m_skyPassGLRPC->m_GLFBC;
		bindFBC(l_FBC);

		activateShaderProgram(GLFinalRenderPassComponent::get().m_skyPassSPC);

		updateUniform(
			GLFinalRenderPassComponent::get().m_skyPass_uni_p,
			GLRenderingSystemComponent::get().m_CamProjOriginal);
		updateUniform(
			GLFinalRenderPassComponent::get().m_skyPass_uni_r,
			GLRenderingSystemComponent::get().m_CamRot);
		updateUniform(
			GLFinalRenderPassComponent::get().m_skyPass_uni_viewportSize,
			(float)GLRenderingSystemComponent::get().deferredPassFBDesc.sizeX, (float)GLRenderingSystemComponent::get().deferredPassFBDesc.sizeY);
		updateUniform(
			GLFinalRenderPassComponent::get().m_skyPass_uni_eyePos,
			GLRenderingSystemComponent::get().m_CamGlobalPos.x, GLRenderingSystemComponent::get().m_CamGlobalPos.y, GLRenderingSystemComponent::get().m_CamGlobalPos.z);
		updateUniform(
			GLFinalRenderPassComponent::get().m_skyPass_uni_lightDir,
			GLRenderingSystemComponent::get().m_sunDir.x, GLRenderingSystemComponent::get().m_sunDir.y, GLRenderingSystemComponent::get().m_sunDir.z);

		auto l_MDC = g_pCoreSystem->getAssetSystem()->getMeshDataComponent(MeshShapeType::CUBE);
		drawMesh(l_MDC);

		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
	}
	else
	{
		GLRenderingSystemNS::cleanFBC(GLFinalRenderPassComponent::get().m_skyPassGLRPC->m_GLFBC);
	}

	return GLFinalRenderPassComponent::get().m_skyPassGLRPC->m_GLTDCs[0];
}

GLTextureDataComponent* GLRenderingSystemNS::updatePreTAAPass()
{
	auto l_FBC = GLFinalRenderPassComponent::get().m_preTAAPassGLRPC->m_GLFBC;
	bindFBC(l_FBC);

	activateShaderProgram(GLFinalRenderPassComponent::get().m_preTAAPassSPC);

	activateTexture(
		GLLightRenderPassComponent::get().m_GLRPC->m_GLTDCs[0],
		0);
	activateTexture(
		GLGeometryRenderPassComponent::get().m_transparentPass_GLRPC->m_GLTDCs[0],
		1);
	activateTexture(
		GLFinalRenderPassComponent::get().m_skyPassGLRPC->m_GLTDCs[0],
		2);
	activateTexture(
		GLTerrainRenderPassComponent::get().m_GLRPC->m_GLTDCs[0],
		3);

	auto l_MDC = g_pCoreSystem->getAssetSystem()->getMeshDataComponent(MeshShapeType::QUAD);
	drawMesh(l_MDC);

	return GLFinalRenderPassComponent::get().m_preTAAPassGLRPC->m_GLTDCs[0];
}

GLTextureDataComponent* GLRenderingSystemNS::updateTAAPass(GLTextureDataComponent* inputGLTDC)
{
	GLTextureDataComponent* l_currentFrameTAAGLTDC = GLFinalRenderPassComponent::get().m_TAAPingPassGLRPC->m_GLTDCs[0];
	GLTextureDataComponent* l_lastFrameTAAGLTDC = GLFinalRenderPassComponent::get().m_TAAPongPassGLRPC->m_GLTDCs[0];

	activateShaderProgram(GLFinalRenderPassComponent::get().m_TAAPassSPC);

	GLFrameBufferComponent* l_FBC;

	if (RenderingSystemComponent::get().m_isTAAPingPass)
	{
		l_currentFrameTAAGLTDC = GLFinalRenderPassComponent::get().m_TAAPingPassGLRPC->m_GLTDCs[0];
		l_lastFrameTAAGLTDC = GLFinalRenderPassComponent::get().m_TAAPongPassGLRPC->m_GLTDCs[0];

		l_FBC = GLFinalRenderPassComponent::get().m_TAAPingPassGLRPC->m_GLFBC;

		RenderingSystemComponent::get().m_isTAAPingPass = false;
	}
	else
	{
		l_currentFrameTAAGLTDC = GLFinalRenderPassComponent::get().m_TAAPongPassGLRPC->m_GLTDCs[0];
		l_lastFrameTAAGLTDC = GLFinalRenderPassComponent::get().m_TAAPingPassGLRPC->m_GLTDCs[0];

		l_FBC = GLFinalRenderPassComponent::get().m_TAAPongPassGLRPC->m_GLFBC;

		RenderingSystemComponent::get().m_isTAAPingPass = true;
	}

	bindFBC(l_FBC);

	activateTexture(inputGLTDC,
		0);
	activateTexture(
		l_lastFrameTAAGLTDC,
		1);
	activateTexture(
		GLGeometryRenderPassComponent::get().m_opaquePass_GLRPC->m_GLTDCs[3],
		2);

	updateUniform(
		GLFinalRenderPassComponent::get().m_TAAPass_uni_renderTargetSize,
		(float)GLRenderingSystemComponent::get().deferredPassFBDesc.sizeX, (float)GLRenderingSystemComponent::get().deferredPassFBDesc.sizeY);

	auto l_MDC = g_pCoreSystem->getAssetSystem()->getMeshDataComponent(MeshShapeType::QUAD);
	drawMesh(l_MDC);

	return l_currentFrameTAAGLTDC;
}

GLTextureDataComponent* GLRenderingSystemNS::updateTAASharpenPass(GLTextureDataComponent * inputGLTDC)
{
	auto l_FBC = GLFinalRenderPassComponent::get().m_TAASharpenPassGLRPC->m_GLFBC;
	bindFBC(l_FBC);
	activateShaderProgram(GLFinalRenderPassComponent::get().m_TAASharpenPassSPC);

	activateTexture(
		inputGLTDC,
		0);

	updateUniform(
		GLFinalRenderPassComponent::get().m_TAASharpenPass_uni_renderTargetSize,
		(float)GLRenderingSystemComponent::get().deferredPassFBDesc.sizeX, (float)GLRenderingSystemComponent::get().deferredPassFBDesc.sizeY);

	auto l_MDC = g_pCoreSystem->getAssetSystem()->getMeshDataComponent(MeshShapeType::QUAD);
	drawMesh(l_MDC);

	return GLFinalRenderPassComponent::get().m_TAASharpenPassGLRPC->m_GLTDCs[0];
}

GLTextureDataComponent* GLRenderingSystemNS::updateBloomExtractPass(GLTextureDataComponent * inputGLTDC)
{
	auto l_FBC = GLFinalRenderPassComponent::get().m_bloomExtractPassGLRPC->m_GLFBC;
	bindFBC(l_FBC);

	activateShaderProgram(GLFinalRenderPassComponent::get().m_bloomExtractPassSPC);

	activateTexture(inputGLTDC, 0);

	auto l_MDC = g_pCoreSystem->getAssetSystem()->getMeshDataComponent(MeshShapeType::QUAD);
	drawMesh(l_MDC);

	return GLFinalRenderPassComponent::get().m_bloomExtractPassGLRPC->m_GLTDCs[0];
}

GLTextureDataComponent* GLRenderingSystemNS::updateBloomBlurPass(GLTextureDataComponent * inputGLTDC)
{
	GLTextureDataComponent* l_currentFrameBloomBlurGLTDC = GLFinalRenderPassComponent::get().m_bloomBlurPingPassGLRPC->m_GLTDCs[0];
	GLTextureDataComponent* l_lastFrameBloomBlurGLTDC = GLFinalRenderPassComponent::get().m_bloomBlurPongPassGLRPC->m_GLTDCs[0];

	activateShaderProgram(GLFinalRenderPassComponent::get().m_bloomBlurPassSPC);

	bool l_isPing = true;
	bool l_isFirstIteration = true;

	auto l_MDC = g_pCoreSystem->getAssetSystem()->getMeshDataComponent(MeshShapeType::QUAD);

	for (size_t i = 0; i < 5; i++)
	{
		if (l_isPing)
		{
			l_currentFrameBloomBlurGLTDC = GLFinalRenderPassComponent::get().m_bloomBlurPingPassGLRPC->m_GLTDCs[0];
			l_lastFrameBloomBlurGLTDC = GLFinalRenderPassComponent::get().m_bloomBlurPongPassGLRPC->m_GLTDCs[0];

			auto l_FBC = GLFinalRenderPassComponent::get().m_bloomBlurPingPassGLRPC->m_GLFBC;
			bindFBC(l_FBC);

			updateUniform(
				GLFinalRenderPassComponent::get().m_bloomBlurPass_uni_horizontal,
				true);

			if (l_isFirstIteration)
			{
				activateTexture(
					inputGLTDC,
					0);
				l_isFirstIteration = false;
			}
			else
			{
				activateTexture(
					l_lastFrameBloomBlurGLTDC,
					0);
			}

			drawMesh(l_MDC);

			l_isPing = false;
		}
		else
		{
			l_currentFrameBloomBlurGLTDC = GLFinalRenderPassComponent::get().m_bloomBlurPongPassGLRPC->m_GLTDCs[0];
			l_lastFrameBloomBlurGLTDC = GLFinalRenderPassComponent::get().m_bloomBlurPingPassGLRPC->m_GLTDCs[0];

			auto l_FBC = GLFinalRenderPassComponent::get().m_bloomBlurPongPassGLRPC->m_GLFBC;
			bindFBC(l_FBC);

			updateUniform(
				GLFinalRenderPassComponent::get().m_bloomBlurPass_uni_horizontal,
				false);

			activateTexture(
				l_lastFrameBloomBlurGLTDC,
				0);

			drawMesh(l_MDC);

			l_isPing = true;
		}
	}

	return l_currentFrameBloomBlurGLTDC;
}

GLTextureDataComponent* GLRenderingSystemNS::updateMotionBlurPass(GLTextureDataComponent * inputGLTDC)
{
	auto l_FBC = GLFinalRenderPassComponent::get().m_motionBlurPassGLRPC->m_GLFBC;
	bindFBC(l_FBC);

	activateShaderProgram(GLFinalRenderPassComponent::get().m_motionBlurPassSPC);

	activateTexture(
		GLGeometryRenderPassComponent::get().m_opaquePass_GLRPC->m_GLTDCs[3],
		0);
	activateTexture(inputGLTDC, 1);

	auto l_MDC = g_pCoreSystem->getAssetSystem()->getMeshDataComponent(MeshShapeType::QUAD);
	drawMesh(l_MDC);

	return GLFinalRenderPassComponent::get().m_motionBlurPassGLRPC->m_GLTDCs[0];
}

GLTextureDataComponent* GLRenderingSystemNS::updateBillboardPass()
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	auto l_FBC = GLFinalRenderPassComponent::get().m_billboardPassGLRPC->m_GLFBC;
	bindFBC(l_FBC);

	// copy depth buffer from G-Pass
	copyDepthBuffer(GLGeometryRenderPassComponent::get().m_opaquePass_GLRPC->m_GLFBC, l_FBC);

	activateShaderProgram(GLFinalRenderPassComponent::get().m_billboardPassSPC);

	updateUniform(
		GLFinalRenderPassComponent::get().m_billboardPass_uni_p,
		GLRenderingSystemComponent::get().m_CamProjOriginal);
	updateUniform(
		GLFinalRenderPassComponent::get().m_billboardPass_uni_r,
		GLRenderingSystemComponent::get().m_CamRot);
	updateUniform(
		GLFinalRenderPassComponent::get().m_billboardPass_uni_t,
		GLRenderingSystemComponent::get().m_CamTrans);

	if (GameSystemComponent::get().m_VisibleComponents.size() > 0)
	{
		// draw each visibleComponent
		for (auto& l_visibleComponent : GameSystemComponent::get().m_VisibleComponents)
		{
			if (l_visibleComponent->m_visiblilityType == VisiblilityType::INNO_BILLBOARD)
			{
				auto l_GlobalPos = g_pCoreSystem->getGameSystem()->get<TransformComponent>(l_visibleComponent->m_parentEntity)->m_globalTransformVector.m_pos;

				updateUniform(
					GLFinalRenderPassComponent::get().m_billboardPass_uni_pos,
					l_GlobalPos.x, l_GlobalPos.y, l_GlobalPos.z);

				auto l_distanceToCamera = (GLRenderingSystemComponent::get().m_CamGlobalPos - l_GlobalPos).length();
				if (l_distanceToCamera > 1.0f)
				{
					updateUniform(
						GLFinalRenderPassComponent::get().m_billboardPass_uni_size,
						(1.0f / l_distanceToCamera) * (9.0f / 16.0f), (1.0f / l_distanceToCamera));
				}
				else
				{
					updateUniform(
						GLFinalRenderPassComponent::get().m_billboardPass_uni_size,
						(9.0f / 16.0f), 1.0f);
				}
				// draw each graphic data of visibleComponent
				for (auto& l_modelPair : l_visibleComponent->m_modelMap)
				{
					// draw meshes
					auto l_MDC = l_modelPair.first;
					if (l_MDC)
					{
						auto l_textureMap = l_modelPair.second;
						// any normal?
						auto l_TDC = l_textureMap->m_texturePack.m_normalTDC.second;
						if (l_TDC)
						{
							activateTexture(l_TDC, 0);
						}
						else
						{
							activateTexture(GLRenderingSystemComponent::get().m_basicNormalGLTDC, 0);
						}
						drawMesh(l_MDC);
					}
				}
			}
		}
	}

	glDisable(GL_DEPTH_TEST);

	return GLFinalRenderPassComponent::get().m_billboardPassGLRPC->m_GLTDCs[0];
}

GLTextureDataComponent* GLRenderingSystemNS::updateDebuggerPass()
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	auto l_FBC = GLFinalRenderPassComponent::get().m_debuggerPassGLRPC->m_GLFBC;
	bindFBC(l_FBC);

	// copy depth buffer from G-Pass
	copyDepthBuffer(GLGeometryRenderPassComponent::get().m_opaquePass_GLRPC->m_GLFBC, l_FBC);

	activateShaderProgram(GLFinalRenderPassComponent::get().m_debuggerPassSPC);

	updateUniform(
		GLFinalRenderPassComponent::get().m_debuggerPass_uni_p,
		GLRenderingSystemComponent::get().m_CamProjOriginal);
	updateUniform(
		GLFinalRenderPassComponent::get().m_debuggerPass_uni_r,
		GLRenderingSystemComponent::get().m_CamRot);
	updateUniform(
		GLFinalRenderPassComponent::get().m_debuggerPass_uni_t,
		GLRenderingSystemComponent::get().m_CamTrans);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	for (auto& AABBWireframeDataPack : PhysicsSystemComponent::get().m_AABBWireframeDataPack)
	{
		updateUniform(
			GLFinalRenderPassComponent::get().m_debuggerPass_uni_m,
			AABBWireframeDataPack.m);
		drawMesh(AABBWireframeDataPack.MDC);
	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_DEPTH_TEST);

	return GLFinalRenderPassComponent::get().m_debuggerPassGLRPC->m_GLTDCs[0];
}

GLTextureDataComponent* GLRenderingSystemNS::updateFinalBlendPass()
{
	auto l_FBC = GLFinalRenderPassComponent::get().m_finalBlendPassGLRPC->m_GLFBC;
	bindFBC(l_FBC);

	activateShaderProgram(GLFinalRenderPassComponent::get().m_finalBlendPassSPC);

	// motion blur pass rendering target
	activateTexture(
		GLFinalRenderPassComponent::get().m_motionBlurPassGLRPC->m_GLTDCs[0],
		0);
	// bloom pass rendering target
	activateTexture(
		GLFinalRenderPassComponent::get().m_bloomBlurPongPassGLRPC->m_GLTDCs[0],
		1);
	// billboard pass rendering target
	activateTexture(
		GLFinalRenderPassComponent::get().m_billboardPassGLRPC->m_GLTDCs[0],
		2);
	// debugger pass rendering target
	activateTexture(
		GLFinalRenderPassComponent::get().m_debuggerPassGLRPC->m_GLTDCs[0],
		3);
	// draw final pass rectangle
	auto l_MDC = g_pCoreSystem->getAssetSystem()->getMeshDataComponent(MeshShapeType::QUAD);
	drawMesh(l_MDC);

	// draw again for game build
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	drawMesh(l_MDC);

	return GLFinalRenderPassComponent::get().m_finalBlendPassGLRPC->m_GLTDCs[0];
}