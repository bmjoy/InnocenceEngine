#pragma once
#include "../common/InnoType.h"
#include "../system/GLHeaders.h"

struct GLTextureDataDesc
{
	GLenum textureType;
	GLenum textureWrapMethod;
	GLenum minFilterParam;
	GLenum magFilterParam;
	GLenum internalFormat;
	GLenum pixelDataFormat;
	GLenum pixelDataType;
};

class GLTextureDataComponent
{
public:
	GLTextureDataComponent() {};
	~GLTextureDataComponent() {};

	ObjectStatus m_objectStatus = ObjectStatus::SHUTDOWN;
	EntityID m_parentEntity = 0;

	GLuint m_TAO = 0;
	GLTextureDataDesc m_GLTextureDataDesc = GLTextureDataDesc();
};

