#pragma once

#include "imgui.h"

enum ImGuiToggleFlags_
{
    ImGuiToggleFlags_None               = 0,
    ImGuiToggleFlags_Animated           = 1 << 0, // The toggle should be animated. Mutually exclusive with ImGuiToggleFlags_Static.
    ImGuiToggleFlags_Static             = 1 << 1, // The toggle should not animate. Mutually exclusive with ImGuiToggleFlags_Animated.
    ImGuiToggleFlags_Default            = ImGuiToggleFlags_Static, // The default flags used when no ImGuiToggleFlags_ are specified.
};

typedef int ImGuiToggleFlags;           // -> enum ImGuiToggleFlags_        // Flags: for Toggle() modes

namespace ImGui
{
    // Widgets: Toggle Switches
    // - Toggles behave similarly to ImGui::Checkbox()
    // - Sometimes called a toggle switch, see also: https://en.wikipedia.org/wiki/Toggle_switch_(widget)
    // - They represent two mutually exclusive states, with an optional animation on the UI when toggled.
    // Optional parameters:
    // - flags: Values from the ImGuiToggleFlags_ enumeration to set toggle modes.
    // - speed: Animation speed scalar. (0,...] default: 1.0f (Overloads with this parameter imply ImGuiToggleFlags_Animated)
    // - frame_rounding: A scalar that controls how rounded the toggle frame is. 0 is square, 1 is round. (0, 1) default 1.0f
    // - knob_rounding: A scalar that controls how rounded the toggle knob is. 0 is square, 1 is round. (0, 1) default 1.0f
    IMGUI_API bool Toggle(const char* label, bool* v);
    IMGUI_API bool Toggle(const char* label, bool* v, ImGuiToggleFlags flags);
    IMGUI_API bool Toggle(const char* label, bool* v, ImGuiToggleFlags flags, float speed);
    IMGUI_API bool Toggle(const char* label, bool* v, ImGuiToggleFlags flags, float frame_rounding, float knob_rounding);
    IMGUI_API bool Toggle(const char* label, bool* v, ImGuiToggleFlags flags, float speed, float frame_rounding, float knob_rounding);

} // namespace ImGui
