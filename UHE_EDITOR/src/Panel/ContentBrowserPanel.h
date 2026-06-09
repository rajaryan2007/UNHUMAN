#pragma once


#include "UHE/Renderer/Texture.h"
#include <filesystem>
#include <unordered_map>

namespace UHE {
	class ContentBrowserPanel
	{
	public:
		ContentBrowserPanel();

		void OnImGuiRender();
		
	private:
		void DrawDirectoryTree(const std::filesystem::path& directoryPath);

		std::filesystem::path m_BaseDirectory;
		std::filesystem::path m_CurrentDirectory;

		Ref<Texture2D> m_DirectoryIcon;
		Ref<Texture2D> m_FileIcon;
		Ref<Texture2D> m_PngIcon;
		Ref<Texture2D> m_modelPng;

		// Dynamic texture cache for image previews
		std::unordered_map<std::string, Ref<Texture2D>> m_TextureCache;
	};
}