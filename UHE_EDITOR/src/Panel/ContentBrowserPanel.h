#pragma once


// #include "UHE/Renderer/Texture.h"
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

		void* m_DirectoryIcon;
		void* m_FileIcon;
		void* m_PngIcon;
		void* m_modelPng;

		// Dynamic texture cache for image previews
		std::unordered_map<std::string, void*> m_TextureCache;
	};
}