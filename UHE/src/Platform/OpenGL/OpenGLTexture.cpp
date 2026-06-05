#include "uhepch.h"
#include "OpenGLTexture.h"

#include <stb_image.h>

#include <glad/glad.h>


namespace UHE {
	OpenGLTexture2D::OpenGLTexture2D(const std::string& path)
		: m_Path(path)
	{
		UHE_PROFILE_FUNCTION();
		int width, height, channels;
		unsigned char* data = nullptr;
		stbi_set_flip_vertically_on_load(1);
		{
			UHE_PROFILE_SCOPE("OpenGL Texture::OpenGL");
			data = stbi_load(path.c_str(), &width, &height, &channels, 0);
		}
		VG_CORE_ASSERT(data, "Failed to load image!");
		m_Width = width;
		m_Height = height;
		m_InternalFormat = 0, m_DataFormat = 0;
		if (channels == 4)
		{
			m_InternalFormat = GL_RGBA8;
			m_DataFormat = GL_RGBA;
		}
		else if (channels == 3)
		{
			m_InternalFormat = GL_RGB8;
			m_DataFormat = GL_RGB;
		}
		
		
		
		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);
		
		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
		
		// Ensure correct row alignment for RGB (3 bytes/pixel) textures.
		// Default GL_UNPACK_ALIGNMENT is 4, which misaligns rows whose
		// byte-count isn't divisible by 4, producing pixel corruption.
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glTextureSubImage2D(
			m_RendererID,
			0,
			0, 0,
			m_Width, m_Height,
			m_DataFormat,
			GL_UNSIGNED_BYTE,
			data
		);

		// Restore default alignment
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		
		stbi_image_free(data);
	}

	OpenGLTexture2D::OpenGLTexture2D(u32 width, u32 hight)
		: m_Width(width),m_Height(hight)
	{
		UHE_PROFILE_FUNCTION();
		
		m_InternalFormat = GL_RGBA8, m_DataFormat = GL_RGBA;
		
		
		VG_CORE_ASSERT(m_InternalFormat & m_DataFormat, "Format not supported!");

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
		//glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);

		//stbi_image_free(data);
	}

	void OpenGLTexture2D::SetData(void* data, u32 size)
	{   
		UHE_PROFILE_FUNCTION();
		u32 bpp = m_DataFormat == GL_RGBA ? 4 : 3;
		VG_CORE_ASSERT(size == m_Width * m_Height * bpp,"Data must be entire texture");

		glTextureSubImage2D(m_RendererID,0,0,0,m_Width,m_Height,m_DataFormat,GL_UNSIGNED_BYTE,data);
	}

	OpenGLTexture2D::~OpenGLTexture2D()
	{
		UHE_PROFILE_FUNCTION();
		glDeleteTextures(1, &m_RendererID);
	}
	void OpenGLTexture2D::Bind(u32 slot) const
	{
		UHE_PROFILE_FUNCTION();
		glBindTextureUnit(slot, m_RendererID);
	}
}






