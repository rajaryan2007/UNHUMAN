#include "EditorTheme.h"
#include <cstring>
#include <cstdio>
#include <filesystem>

#ifndef UHE_PROJECT_DIR
#define UHE_PROJECT_DIR "."
#endif

namespace EditorTheme
{
    namespace
    {
        // Deep desaturated slate base with a violet accent. Default theme,
        // close to the editor's original look but fully filled in.
        void StyleDarkViolet(ImGuiStyle& s)
        {
            ImVec4* c = s.Colors;
            c[ImGuiCol_Text]                 = {0.920f, 0.920f, 0.940f, 1.00f};
            c[ImGuiCol_TextDisabled]         = {0.480f, 0.480f, 0.540f, 1.00f};
            c[ImGuiCol_WindowBg]             = {0.095f, 0.095f, 0.115f, 1.00f};
            c[ImGuiCol_ChildBg]              = {0.100f, 0.100f, 0.120f, 1.00f};
            c[ImGuiCol_PopupBg]              = {0.110f, 0.110f, 0.135f, 0.98f};
            c[ImGuiCol_Border]               = {0.190f, 0.190f, 0.240f, 0.60f};
            c[ImGuiCol_BorderShadow]         = {0.000f, 0.000f, 0.000f, 0.00f};
            c[ImGuiCol_FrameBg]              = {0.135f, 0.135f, 0.165f, 1.00f};
            c[ImGuiCol_FrameBgHovered]       = {0.165f, 0.165f, 0.205f, 1.00f};
            c[ImGuiCol_FrameBgActive]        = {0.190f, 0.190f, 0.240f, 1.00f};
            c[ImGuiCol_TitleBg]              = {0.080f, 0.080f, 0.098f, 1.00f};
            c[ImGuiCol_TitleBgActive]        = {0.110f, 0.105f, 0.150f, 1.00f};
            c[ImGuiCol_TitleBgCollapsed]     = {0.070f, 0.070f, 0.085f, 1.00f};
            c[ImGuiCol_MenuBarBg]            = {0.105f, 0.105f, 0.130f, 1.00f};
            c[ImGuiCol_ScrollbarBg]          = {0.090f, 0.090f, 0.110f, 1.00f};
            c[ImGuiCol_ScrollbarGrab]        = {0.240f, 0.240f, 0.300f, 1.00f};
            c[ImGuiCol_ScrollbarGrabHovered] = {0.300f, 0.300f, 0.370f, 1.00f};
            c[ImGuiCol_ScrollbarGrabActive]  = {0.424f, 0.388f, 1.000f, 1.00f};
            c[ImGuiCol_CheckMark]            = {0.525f, 0.490f, 1.000f, 1.00f};
            c[ImGuiCol_SliderGrab]           = {0.424f, 0.388f, 1.000f, 0.80f};
            c[ImGuiCol_SliderGrabActive]     = {0.560f, 0.525f, 1.000f, 1.00f};
            c[ImGuiCol_Button]               = {0.170f, 0.170f, 0.215f, 1.00f};
            c[ImGuiCol_ButtonHovered]        = {0.230f, 0.225f, 0.300f, 1.00f};
            c[ImGuiCol_ButtonActive]         = {0.424f, 0.388f, 1.000f, 0.85f};
            c[ImGuiCol_Header]               = {0.200f, 0.195f, 0.270f, 1.00f};
            c[ImGuiCol_HeaderHovered]        = {0.424f, 0.388f, 1.000f, 0.35f};
            c[ImGuiCol_HeaderActive]         = {0.424f, 0.388f, 1.000f, 0.55f};
            c[ImGuiCol_Separator]            = {0.200f, 0.200f, 0.250f, 0.70f};
            c[ImGuiCol_SeparatorHovered]     = {0.424f, 0.388f, 1.000f, 0.60f};
            c[ImGuiCol_SeparatorActive]      = {0.424f, 0.388f, 1.000f, 0.90f};
            c[ImGuiCol_ResizeGrip]           = {0.220f, 0.220f, 0.280f, 0.50f};
            c[ImGuiCol_ResizeGripHovered]    = {0.424f, 0.388f, 1.000f, 0.60f};
            c[ImGuiCol_ResizeGripActive]     = {0.424f, 0.388f, 1.000f, 0.90f};
            c[ImGuiCol_InputTextCursor]      = {0.525f, 0.490f, 1.000f, 1.00f};
            c[ImGuiCol_TabHovered]           = {0.424f, 0.388f, 1.000f, 0.40f};
            c[ImGuiCol_Tab]                  = {0.120f, 0.120f, 0.150f, 1.00f};
            c[ImGuiCol_TabSelected]          = {0.185f, 0.180f, 0.250f, 1.00f};
            c[ImGuiCol_TabSelectedOverline]  = {0.424f, 0.388f, 1.000f, 1.00f};
            c[ImGuiCol_TabDimmed]            = {0.100f, 0.100f, 0.125f, 1.00f};
            c[ImGuiCol_TabDimmedSelected]    = {0.140f, 0.138f, 0.180f, 1.00f};
            c[ImGuiCol_TabDimmedSelectedOverline] = {0.300f, 0.280f, 0.500f, 1.00f};
            c[ImGuiCol_DockingPreview]       = {0.424f, 0.388f, 1.000f, 0.45f};
            c[ImGuiCol_DockingEmptyBg]       = {0.085f, 0.085f, 0.100f, 1.00f};
            c[ImGuiCol_PlotLines]            = {0.525f, 0.490f, 1.000f, 1.00f};
            c[ImGuiCol_PlotLinesHovered]     = {0.700f, 0.680f, 1.000f, 1.00f};
            c[ImGuiCol_PlotHistogram]        = {0.424f, 0.388f, 1.000f, 0.85f};
            c[ImGuiCol_PlotHistogramHovered] = {0.600f, 0.570f, 1.000f, 1.00f};
            c[ImGuiCol_TableHeaderBg]        = {0.140f, 0.140f, 0.175f, 1.00f};
            c[ImGuiCol_TableBorderStrong]    = {0.230f, 0.230f, 0.290f, 1.00f};
            c[ImGuiCol_TableBorderLight]     = {0.170f, 0.170f, 0.210f, 1.00f};
            c[ImGuiCol_TableRowBg]           = {0.000f, 0.000f, 0.000f, 0.00f};
            c[ImGuiCol_TableRowBgAlt]        = {1.000f, 1.000f, 1.000f, 0.025f};
            c[ImGuiCol_TextLink]             = {0.525f, 0.490f, 1.000f, 1.00f};
            c[ImGuiCol_TextSelectedBg]       = {0.424f, 0.388f, 1.000f, 0.35f};
            c[ImGuiCol_TreeLines]            = {0.260f, 0.260f, 0.330f, 1.00f};
            c[ImGuiCol_DragDropTarget]       = {0.424f, 0.388f, 1.000f, 0.95f};
            c[ImGuiCol_DragDropTargetBg]     = {0.424f, 0.388f, 1.000f, 0.10f};
            c[ImGuiCol_NavCursor]            = {0.424f, 0.388f, 1.000f, 0.90f};
            c[ImGuiCol_NavWindowingHighlight]= {1.000f, 1.000f, 1.000f, 0.70f};
            c[ImGuiCol_NavWindowingDimBg]    = {0.000f, 0.000f, 0.000f, 0.55f};
            c[ImGuiCol_ModalWindowDimBg]     = {0.000f, 0.000f, 0.000f, 0.55f};
        }

        // Cool near-black blue with a cyan accent.
        void StyleMidnight(ImGuiStyle& s)
        {
            ImVec4* c = s.Colors;
            const ImVec4 accent{0.260f, 0.780f, 0.870f, 1.00f};
            c[ImGuiCol_Text]                 = {0.880f, 0.910f, 0.940f, 1.00f};
            c[ImGuiCol_TextDisabled]         = {0.430f, 0.470f, 0.520f, 1.00f};
            c[ImGuiCol_WindowBg]             = {0.070f, 0.085f, 0.105f, 1.00f};
            c[ImGuiCol_ChildBg]              = {0.075f, 0.090f, 0.110f, 1.00f};
            c[ImGuiCol_PopupBg]              = {0.085f, 0.100f, 0.125f, 0.98f};
            c[ImGuiCol_Border]               = {0.150f, 0.190f, 0.230f, 0.60f};
            c[ImGuiCol_BorderShadow]         = {0.000f, 0.000f, 0.000f, 0.00f};
            c[ImGuiCol_FrameBg]              = {0.110f, 0.130f, 0.160f, 1.00f};
            c[ImGuiCol_FrameBgHovered]       = {0.140f, 0.165f, 0.200f, 1.00f};
            c[ImGuiCol_FrameBgActive]        = {0.170f, 0.200f, 0.245f, 1.00f};
            c[ImGuiCol_TitleBg]              = {0.058f, 0.072f, 0.090f, 1.00f};
            c[ImGuiCol_TitleBgActive]        = {0.080f, 0.110f, 0.150f, 1.00f};
            c[ImGuiCol_TitleBgCollapsed]     = {0.050f, 0.062f, 0.078f, 1.00f};
            c[ImGuiCol_MenuBarBg]            = {0.085f, 0.100f, 0.125f, 1.00f};
            c[ImGuiCol_ScrollbarBg]          = {0.070f, 0.082f, 0.100f, 1.00f};
            c[ImGuiCol_ScrollbarGrab]        = {0.200f, 0.240f, 0.290f, 1.00f};
            c[ImGuiCol_ScrollbarGrabHovered] = {0.260f, 0.310f, 0.370f, 1.00f};
            c[ImGuiCol_ScrollbarGrabActive]  = {accent.x, accent.y, accent.z, 1.00f};
            c[ImGuiCol_CheckMark]            = {accent.x, accent.y, accent.z, 1.00f};
            c[ImGuiCol_SliderGrab]           = {accent.x, accent.y, accent.z, 0.80f};
            c[ImGuiCol_SliderGrabActive]     = {0.450f, 0.880f, 0.950f, 1.00f};
            c[ImGuiCol_Button]               = {0.130f, 0.155f, 0.190f, 1.00f};
            c[ImGuiCol_ButtonHovered]        = {0.175f, 0.210f, 0.260f, 1.00f};
            c[ImGuiCol_ButtonActive]         = {accent.x, accent.y, accent.z, 0.85f};
            c[ImGuiCol_Header]               = {0.150f, 0.185f, 0.230f, 1.00f};
            c[ImGuiCol_HeaderHovered]        = {accent.x, accent.y, accent.z, 0.35f};
            c[ImGuiCol_HeaderActive]         = {accent.x, accent.y, accent.z, 0.55f};
            c[ImGuiCol_Separator]            = {0.160f, 0.195f, 0.240f, 0.70f};
            c[ImGuiCol_SeparatorHovered]     = {accent.x, accent.y, accent.z, 0.60f};
            c[ImGuiCol_SeparatorActive]      = {accent.x, accent.y, accent.z, 0.90f};
            c[ImGuiCol_ResizeGrip]           = {0.180f, 0.215f, 0.265f, 0.50f};
            c[ImGuiCol_ResizeGripHovered]    = {accent.x, accent.y, accent.z, 0.60f};
            c[ImGuiCol_ResizeGripActive]     = {accent.x, accent.y, accent.z, 0.90f};
            c[ImGuiCol_InputTextCursor]      = {accent.x, accent.y, accent.z, 1.00f};
            c[ImGuiCol_TabHovered]           = {accent.x, accent.y, accent.z, 0.40f};
            c[ImGuiCol_Tab]                  = {0.095f, 0.115f, 0.140f, 1.00f};
            c[ImGuiCol_TabSelected]          = {0.145f, 0.175f, 0.215f, 1.00f};
            c[ImGuiCol_TabSelectedOverline]  = {accent.x, accent.y, accent.z, 1.00f};
            c[ImGuiCol_TabDimmed]            = {0.080f, 0.095f, 0.118f, 1.00f};
            c[ImGuiCol_TabDimmedSelected]    = {0.115f, 0.140f, 0.170f, 1.00f};
            c[ImGuiCol_TabDimmedSelectedOverline] = {0.200f, 0.400f, 0.480f, 1.00f};
            c[ImGuiCol_DockingPreview]       = {accent.x, accent.y, accent.z, 0.45f};
            c[ImGuiCol_DockingEmptyBg]       = {0.065f, 0.078f, 0.095f, 1.00f};
            c[ImGuiCol_PlotLines]            = {accent.x, accent.y, accent.z, 1.00f};
            c[ImGuiCol_PlotLinesHovered]     = {0.600f, 0.920f, 0.980f, 1.00f};
            c[ImGuiCol_PlotHistogram]        = {accent.x, accent.y, accent.z, 0.85f};
            c[ImGuiCol_PlotHistogramHovered] = {0.500f, 0.850f, 0.930f, 1.00f};
            c[ImGuiCol_TableHeaderBg]        = {0.115f, 0.138f, 0.168f, 1.00f};
            c[ImGuiCol_TableBorderStrong]    = {0.190f, 0.230f, 0.280f, 1.00f};
            c[ImGuiCol_TableBorderLight]     = {0.140f, 0.170f, 0.205f, 1.00f};
            c[ImGuiCol_TableRowBg]           = {0.000f, 0.000f, 0.000f, 0.00f};
            c[ImGuiCol_TableRowBgAlt]        = {1.000f, 1.000f, 1.000f, 0.025f};
            c[ImGuiCol_TextLink]             = {accent.x, accent.y, accent.z, 1.00f};
            c[ImGuiCol_TextSelectedBg]       = {accent.x, accent.y, accent.z, 0.35f};
            c[ImGuiCol_TreeLines]            = {0.210f, 0.255f, 0.310f, 1.00f};
            c[ImGuiCol_DragDropTarget]       = {accent.x, accent.y, accent.z, 0.95f};
            c[ImGuiCol_DragDropTargetBg]     = {accent.x, accent.y, accent.z, 0.10f};
            c[ImGuiCol_NavCursor]            = {accent.x, accent.y, accent.z, 0.90f};
            c[ImGuiCol_NavWindowingHighlight]= {1.000f, 1.000f, 1.000f, 0.70f};
            c[ImGuiCol_NavWindowingDimBg]    = {0.000f, 0.000f, 0.000f, 0.55f};
            c[ImGuiCol_ModalWindowDimBg]     = {0.000f, 0.000f, 0.000f, 0.55f};
        }

        // Neutral warm graphite with an amber accent; soft, low-contrast.
        void StyleGraphite(ImGuiStyle& s)
        {
            ImVec4* c = s.Colors;
            const ImVec4 accent{0.960f, 0.640f, 0.240f, 1.00f};
            c[ImGuiCol_Text]                 = {0.900f, 0.890f, 0.870f, 1.00f};
            c[ImGuiCol_TextDisabled]         = {0.470f, 0.460f, 0.445f, 1.00f};
            c[ImGuiCol_WindowBg]             = {0.110f, 0.110f, 0.110f, 1.00f};
            c[ImGuiCol_ChildBg]              = {0.115f, 0.115f, 0.115f, 1.00f};
            c[ImGuiCol_PopupBg]              = {0.130f, 0.128f, 0.126f, 0.98f};
            c[ImGuiCol_Border]               = {0.220f, 0.218f, 0.214f, 0.60f};
            c[ImGuiCol_BorderShadow]         = {0.000f, 0.000f, 0.000f, 0.00f};
            c[ImGuiCol_FrameBg]              = {0.160f, 0.158f, 0.155f, 1.00f};
            c[ImGuiCol_FrameBgHovered]       = {0.195f, 0.192f, 0.188f, 1.00f};
            c[ImGuiCol_FrameBgActive]        = {0.230f, 0.226f, 0.220f, 1.00f};
            c[ImGuiCol_TitleBg]              = {0.092f, 0.091f, 0.090f, 1.00f};
            c[ImGuiCol_TitleBgActive]        = {0.135f, 0.130f, 0.122f, 1.00f};
            c[ImGuiCol_TitleBgCollapsed]     = {0.082f, 0.081f, 0.080f, 1.00f};
            c[ImGuiCol_MenuBarBg]            = {0.125f, 0.124f, 0.122f, 1.00f};
            c[ImGuiCol_ScrollbarBg]          = {0.105f, 0.104f, 0.102f, 1.00f};
            c[ImGuiCol_ScrollbarGrab]        = {0.270f, 0.266f, 0.260f, 1.00f};
            c[ImGuiCol_ScrollbarGrabHovered] = {0.330f, 0.325f, 0.318f, 1.00f};
            c[ImGuiCol_ScrollbarGrabActive]  = {accent.x, accent.y, accent.z, 1.00f};
            c[ImGuiCol_CheckMark]            = {accent.x, accent.y, accent.z, 1.00f};
            c[ImGuiCol_SliderGrab]           = {accent.x, accent.y, accent.z, 0.80f};
            c[ImGuiCol_SliderGrabActive]     = {1.000f, 0.720f, 0.380f, 1.00f};
            c[ImGuiCol_Button]               = {0.195f, 0.192f, 0.188f, 1.00f};
            c[ImGuiCol_ButtonHovered]        = {0.255f, 0.250f, 0.243f, 1.00f};
            c[ImGuiCol_ButtonActive]         = {accent.x, accent.y, accent.z, 0.85f};
            c[ImGuiCol_Header]               = {0.215f, 0.210f, 0.204f, 1.00f};
            c[ImGuiCol_HeaderHovered]        = {accent.x, accent.y, accent.z, 0.35f};
            c[ImGuiCol_HeaderActive]         = {accent.x, accent.y, accent.z, 0.55f};
            c[ImGuiCol_Separator]            = {0.235f, 0.231f, 0.226f, 0.70f};
            c[ImGuiCol_SeparatorHovered]     = {accent.x, accent.y, accent.z, 0.60f};
            c[ImGuiCol_SeparatorActive]      = {accent.x, accent.y, accent.z, 0.90f};
            c[ImGuiCol_ResizeGrip]           = {0.255f, 0.250f, 0.245f, 0.50f};
            c[ImGuiCol_ResizeGripHovered]    = {accent.x, accent.y, accent.z, 0.60f};
            c[ImGuiCol_ResizeGripActive]     = {accent.x, accent.y, accent.z, 0.90f};
            c[ImGuiCol_InputTextCursor]      = {accent.x, accent.y, accent.z, 1.00f};
            c[ImGuiCol_TabHovered]           = {accent.x, accent.y, accent.z, 0.40f};
            c[ImGuiCol_Tab]                  = {0.130f, 0.128f, 0.126f, 1.00f};
            c[ImGuiCol_TabSelected]          = {0.190f, 0.186f, 0.180f, 1.00f};
            c[ImGuiCol_TabSelectedOverline]  = {accent.x, accent.y, accent.z, 1.00f};
            c[ImGuiCol_TabDimmed]            = {0.112f, 0.110f, 0.108f, 1.00f};
            c[ImGuiCol_TabDimmedSelected]    = {0.152f, 0.149f, 0.145f, 1.00f};
            c[ImGuiCol_TabDimmedSelectedOverline] = {0.500f, 0.360f, 0.170f, 1.00f};
            c[ImGuiCol_DockingPreview]       = {accent.x, accent.y, accent.z, 0.45f};
            c[ImGuiCol_DockingEmptyBg]       = {0.098f, 0.097f, 0.096f, 1.00f};
            c[ImGuiCol_PlotLines]            = {accent.x, accent.y, accent.z, 1.00f};
            c[ImGuiCol_PlotLinesHovered]     = {1.000f, 0.780f, 0.450f, 1.00f};
            c[ImGuiCol_PlotHistogram]        = {accent.x, accent.y, accent.z, 0.85f};
            c[ImGuiCol_PlotHistogramHovered] = {1.000f, 0.740f, 0.400f, 1.00f};
            c[ImGuiCol_TableHeaderBg]        = {0.165f, 0.162f, 0.158f, 1.00f};
            c[ImGuiCol_TableBorderStrong]    = {0.260f, 0.255f, 0.248f, 1.00f};
            c[ImGuiCol_TableBorderLight]     = {0.195f, 0.191f, 0.186f, 1.00f};
            c[ImGuiCol_TableRowBg]           = {0.000f, 0.000f, 0.000f, 0.00f};
            c[ImGuiCol_TableRowBgAlt]        = {1.000f, 1.000f, 1.000f, 0.03f};
            c[ImGuiCol_TextLink]             = {accent.x, accent.y, accent.z, 1.00f};
            c[ImGuiCol_TextSelectedBg]       = {accent.x, accent.y, accent.z, 0.35f};
            c[ImGuiCol_TreeLines]            = {0.290f, 0.285f, 0.278f, 1.00f};
            c[ImGuiCol_DragDropTarget]       = {accent.x, accent.y, accent.z, 0.95f};
            c[ImGuiCol_DragDropTargetBg]     = {accent.x, accent.y, accent.z, 0.10f};
            c[ImGuiCol_NavCursor]            = {accent.x, accent.y, accent.z, 0.90f};
            c[ImGuiCol_NavWindowingHighlight]= {1.000f, 1.000f, 1.000f, 0.70f};
            c[ImGuiCol_NavWindowingDimBg]    = {0.000f, 0.000f, 0.000f, 0.55f};
            c[ImGuiCol_ModalWindowDimBg]     = {0.000f, 0.000f, 0.000f, 0.55f};
        }

        // Clean light theme with a blue accent for daylight editing.
        void StyleLight(ImGuiStyle& s)
        {
            ImVec4* c = s.Colors;
            const ImVec4 accent{0.180f, 0.420f, 0.880f, 1.00f};
            c[ImGuiCol_Text]                 = {0.100f, 0.105f, 0.115f, 1.00f};
            c[ImGuiCol_TextDisabled]         = {0.550f, 0.560f, 0.580f, 1.00f};
            c[ImGuiCol_WindowBg]             = {0.940f, 0.942f, 0.948f, 1.00f};
            c[ImGuiCol_ChildBg]              = {0.950f, 0.952f, 0.956f, 1.00f};
            c[ImGuiCol_PopupBg]              = {0.975f, 0.976f, 0.980f, 0.99f};
            c[ImGuiCol_Border]               = {0.780f, 0.785f, 0.800f, 0.80f};
            c[ImGuiCol_BorderShadow]         = {0.000f, 0.000f, 0.000f, 0.00f};
            c[ImGuiCol_FrameBg]              = {0.890f, 0.893f, 0.900f, 1.00f};
            c[ImGuiCol_FrameBgHovered]       = {0.850f, 0.855f, 0.865f, 1.00f};
            c[ImGuiCol_FrameBgActive]        = {0.810f, 0.818f, 0.830f, 1.00f};
            c[ImGuiCol_TitleBg]              = {0.870f, 0.874f, 0.882f, 1.00f};
            c[ImGuiCol_TitleBgActive]        = {0.820f, 0.828f, 0.845f, 1.00f};
            c[ImGuiCol_TitleBgCollapsed]     = {0.870f, 0.874f, 0.882f, 1.00f};
            c[ImGuiCol_MenuBarBg]            = {0.900f, 0.903f, 0.910f, 1.00f};
            c[ImGuiCol_ScrollbarBg]          = {0.910f, 0.913f, 0.920f, 1.00f};
            c[ImGuiCol_ScrollbarGrab]        = {0.740f, 0.745f, 0.760f, 1.00f};
            c[ImGuiCol_ScrollbarGrabHovered] = {0.640f, 0.648f, 0.665f, 1.00f};
            c[ImGuiCol_ScrollbarGrabActive]  = {accent.x, accent.y, accent.z, 1.00f};
            c[ImGuiCol_CheckMark]            = {accent.x, accent.y, accent.z, 1.00f};
            c[ImGuiCol_SliderGrab]           = {accent.x, accent.y, accent.z, 0.80f};
            c[ImGuiCol_SliderGrabActive]     = {0.120f, 0.320f, 0.720f, 1.00f};
            c[ImGuiCol_Button]               = {0.860f, 0.864f, 0.872f, 1.00f};
            c[ImGuiCol_ButtonHovered]        = {0.790f, 0.800f, 0.818f, 1.00f};
            c[ImGuiCol_ButtonActive]         = {accent.x, accent.y, accent.z, 0.85f};
            c[ImGuiCol_Header]               = {0.800f, 0.830f, 0.905f, 1.00f};
            c[ImGuiCol_HeaderHovered]        = {accent.x, accent.y, accent.z, 0.30f};
            c[ImGuiCol_HeaderActive]         = {accent.x, accent.y, accent.z, 0.50f};
            c[ImGuiCol_Separator]            = {0.790f, 0.795f, 0.810f, 0.90f};
            c[ImGuiCol_SeparatorHovered]     = {accent.x, accent.y, accent.z, 0.60f};
            c[ImGuiCol_SeparatorActive]      = {accent.x, accent.y, accent.z, 0.90f};
            c[ImGuiCol_ResizeGrip]           = {0.760f, 0.768f, 0.785f, 0.60f};
            c[ImGuiCol_ResizeGripHovered]    = {accent.x, accent.y, accent.z, 0.60f};
            c[ImGuiCol_ResizeGripActive]     = {accent.x, accent.y, accent.z, 0.90f};
            c[ImGuiCol_InputTextCursor]      = {accent.x, accent.y, accent.z, 1.00f};
            c[ImGuiCol_TabHovered]           = {accent.x, accent.y, accent.z, 0.30f};
            c[ImGuiCol_Tab]                  = {0.885f, 0.888f, 0.895f, 1.00f};
            c[ImGuiCol_TabSelected]          = {0.955f, 0.957f, 0.962f, 1.00f};
            c[ImGuiCol_TabSelectedOverline]  = {accent.x, accent.y, accent.z, 1.00f};
            c[ImGuiCol_TabDimmed]            = {0.860f, 0.864f, 0.872f, 1.00f};
            c[ImGuiCol_TabDimmedSelected]    = {0.905f, 0.908f, 0.915f, 1.00f};
            c[ImGuiCol_TabDimmedSelectedOverline] = {0.550f, 0.650f, 0.850f, 1.00f};
            c[ImGuiCol_DockingPreview]       = {accent.x, accent.y, accent.z, 0.40f};
            c[ImGuiCol_DockingEmptyBg]       = {0.900f, 0.903f, 0.910f, 1.00f};
            c[ImGuiCol_PlotLines]            = {accent.x, accent.y, accent.z, 1.00f};
            c[ImGuiCol_PlotLinesHovered]     = {0.950f, 0.450f, 0.250f, 1.00f};
            c[ImGuiCol_PlotHistogram]        = {accent.x, accent.y, accent.z, 0.85f};
            c[ImGuiCol_PlotHistogramHovered] = {0.950f, 0.550f, 0.300f, 1.00f};
            c[ImGuiCol_TableHeaderBg]        = {0.870f, 0.875f, 0.885f, 1.00f};
            c[ImGuiCol_TableBorderStrong]    = {0.700f, 0.706f, 0.720f, 1.00f};
            c[ImGuiCol_TableBorderLight]     = {0.820f, 0.825f, 0.835f, 1.00f};
            c[ImGuiCol_TableRowBg]           = {0.000f, 0.000f, 0.000f, 0.00f};
            c[ImGuiCol_TableRowBgAlt]        = {0.000f, 0.000f, 0.000f, 0.03f};
            c[ImGuiCol_TextLink]             = {accent.x, accent.y, accent.z, 1.00f};
            c[ImGuiCol_TextSelectedBg]       = {accent.x, accent.y, accent.z, 0.30f};
            c[ImGuiCol_TreeLines]            = {0.760f, 0.765f, 0.780f, 1.00f};
            c[ImGuiCol_DragDropTarget]       = {accent.x, accent.y, accent.z, 0.95f};
            c[ImGuiCol_DragDropTargetBg]     = {accent.x, accent.y, accent.z, 0.08f};
            c[ImGuiCol_NavCursor]            = {accent.x, accent.y, accent.z, 0.90f};
            c[ImGuiCol_NavWindowingHighlight]= {0.000f, 0.000f, 0.000f, 0.70f};
            c[ImGuiCol_NavWindowingDimBg]    = {0.000f, 0.000f, 0.000f, 0.45f};
            c[ImGuiCol_ModalWindowDimBg]     = {0.000f, 0.000f, 0.000f, 0.45f};
        }

        const char* kThemeNames[] = {"Dark Violet", "Midnight", "Graphite", "Light"};
        static_assert(sizeof(kThemeNames) / sizeof(kThemeNames[0]) == (int)EditorThemeId::COUNT,
                      "Theme name table out of sync with EditorThemeId");

        EditorThemeId s_Selected = EditorThemeId::DarkViolet;
        bool s_Applied = false;

        void ApplyMetrics(ImGuiStyle& s)
        {
            s.WindowRounding    = 4.0f;
            s.ChildRounding     = 4.0f;
            s.FrameRounding     = 4.0f;
            s.PopupRounding     = 4.0f;
            s.GrabRounding      = 4.0f;
            s.TabRounding       = 5.0f;
            s.ScrollbarRounding = 9.0f;
            s.WindowBorderSize  = 1.0f;
            s.ChildBorderSize   = 1.0f;
            s.PopupBorderSize   = 1.0f;
            s.FrameBorderSize   = 0.0f;
            s.FramePadding      = ImVec2(7, 4);
            s.ItemSpacing       = ImVec2(8, 5);
            s.ItemInnerSpacing  = ImVec2(6, 5);
            s.IndentSpacing     = 20.0f;
            s.ScrollbarSize     = 13.0f;
            s.GrabMinSize       = 9.0f;
            s.WindowPadding     = ImVec2(8, 8);
            s.DockingSeparatorSize = 2.0f;
        }

        const char* SettingsPath()
        {
            // Lives next to wherever the editor was launched from (the bin
            // output dir in dev), keeping the source tree clean.
            static std::filesystem::path path = std::filesystem::current_path() / "editor_theme.ini";
            return path.string().c_str();
        }
    } // namespace

    void Apply(EditorThemeId id)
    {
        ImGuiStyle& st = ImGui::GetStyle();
        ImGui::StyleColorsDark(&st); // sane base, then overwrite every slot

        switch (id)
        {
            case EditorThemeId::Midnight:  StyleMidnight(st);  break;
            case EditorThemeId::Graphite:  StyleGraphite(st);  break;
            case EditorThemeId::Light:     StyleLight(st);     break;
            case EditorThemeId::DarkViolet:
            default:                       StyleDarkViolet(st); break;
        }

        ApplyMetrics(st);
        s_Selected = id;
        s_Applied = true;
    }

    void EnsureApplied()
    {
        if (!s_Applied)
            Apply(s_Selected);
    }

    EditorThemeId GetSelected() { return s_Selected; }

    const char* GetName(EditorThemeId id)
    {
        int i = (int)id;
        if (i < 0 || i >= (int)EditorThemeId::COUNT)
            return "Unknown";
        return kThemeNames[i];
    }

    void SetSelected(EditorThemeId id)
    {
        Apply(id);

        // Persist best-effort; a read-only directory shouldn't break the UI.
        FILE* f = fopen(SettingsPath(), "w");
        if (f)
        {
            fprintf(f, "%d\n", (int)id);
            fclose(f);
        }
    }

    void LoadSelected()
    {
        if (FILE* f = fopen(SettingsPath(), "r"))
        {
            int value = -1;
            if (fscanf(f, "%d", &value) == 1 && value >= 0 && value < (int)EditorThemeId::COUNT)
                s_Selected = (EditorThemeId)value;
            fclose(f);
        }
        s_Applied = false; // re-apply on first frame with the loaded id
    }

    // ---- palette accessors ----

    namespace
    {
        ImVec4 GetCol(ImGuiCol idx)
        {
            const ImGuiStyle& st = ImGui::GetStyle();
            return st.Colors[idx];
        }

        ImVec4 WithAlpha(const ImVec4& base, float alpha)
        {
            return ImVec4(base.x, base.y, base.z, alpha);
        }
    }

    ImVec4 Accent()        { return GetCol(ImGuiCol_PlotLines); }
    ImVec4 AccentHover()   { return WithAlpha(Accent(), 0.85f); }
    ImVec4 AccentActive()  { return WithAlpha(Accent(), 0.70f); }
    ImVec4 AccentMuted()   { return WithAlpha(Accent(), 0.15f); }
    ImVec4 ToolbarBg()     { return GetCol(ImGuiCol_TitleBgActive); }
} // namespace EditorTheme
