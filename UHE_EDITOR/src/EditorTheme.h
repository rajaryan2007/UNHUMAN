#pragma once

#include "imgui.h"

namespace EditorTheme
{
    enum class EditorThemeId
    {
        DarkViolet = 0,
        Midnight,
        Graphite,
        Light,
        COUNT
    };

    // Applies the theme to the ImGui style (colors + metrics).
    void Apply(EditorThemeId id);

    // Applies the persisted selection once, on the first ImGui frame.
    void EnsureApplied();

    EditorThemeId GetSelected();
    const char* GetName(EditorThemeId id);

    // Applies and persists the selection.
    void SetSelected(EditorThemeId id);

    // Reads the persisted theme file (defaults to DarkViolet when missing).
    void LoadSelected();

    // Palette accessors for panels with hand-drawn accents.
    ImVec4 Accent();
    ImVec4 AccentHover();
    ImVec4 AccentActive();
    ImVec4 AccentMuted();
    ImVec4 ToolbarBg();
} // namespace EditorTheme
