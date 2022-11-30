#pragma once

#include "imgui.h"

enum ImGuiToggleFlags_
{
    ImGuiToggleFlags_None                   = 0,
    ImGuiToggleFlags_Animated               = 1 << 0, // The toggle should be animated. Mutually exclusive with ImGuiToggleFlags_Static.
    ImGuiToggleFlags_Static                 = 1 << 1, // The toggle should not animate. Mutually exclusive with ImGuiToggleFlags_Animated.
    ImGuiToggleFlags_BorderedFrame          = 1 << 2, // The toggle should have a border drawn on the frame.
    ImGuiToggleFlags_BorderedKnob           = 1 << 3, // The toggle should have a border drawn on the knob.
    ImGuiToggleFlags_HighlightEnabledKnob   = 1 << 4, // The toggle's knob should be drawn in a highlight color when enabled.
    ImGuiToggleFlags_A11yLabels             = 1 << 8, // The toggle should draw on and off labels to indicate its state.
    ImGuiToggleFlags_Bordered               = ImGuiToggleFlags_BorderedFrame | ImGuiToggleFlags_BorderedKnob, // Shorthand for bordered frame and knob.
    ImGuiToggleFlags_Default                = ImGuiToggleFlags_Static, // The default flags used when no ImGuiToggleFlags_ are specified.
};

typedef int ImGuiToggleFlags;               // -> enum ImGuiToggleFlags_        // Flags: for Toggle() modes


namespace ImGuiToggleConstants
{
    constexpr float Phi = 1.6180339887498948482045f;
    constexpr float DiameterToRadiusRatio = 0.5f;

    constexpr float AnimationSpeedDisabled = 0.0f;
    constexpr float AnimationSpeedDefault = 1.0f;
    constexpr float FrameRoundingDefault = 1.0f;
    constexpr float KnobRoundingDefault = 1.0f;
    constexpr float WidthRatioDefault = Phi;
    constexpr float KnobInsetDefault = 1.5f;

    constexpr float BorderThicknessDefault = 1.0f;

    constexpr float AnimationSpeedMinimum = 0.0f;

    constexpr float FrameRoundingMinimum = 0.0f;
    constexpr float FrameRoundingMaximum = 1.0f;

    constexpr float KnobRoundingMinimum = 0.0f;
    constexpr float KnobRoundingMaximum = 1.0f;

    constexpr float WidthRatioMinimum = 1.1f;
    constexpr float WidthRatioMaximum = 10.0f;

    constexpr float KnobInsetMinimum = -100.0f;
    constexpr float KnobInsetMaximum = 100.0f;
}

struct ImGuiToggleConfig
{
    ImGuiToggleFlags Flags = ImGuiToggleFlags_Default;

    float AnimationSpeed = ImGuiToggleConstants::AnimationSpeedDefault;
    float FrameRounding = ImGuiToggleConstants::FrameRoundingDefault;
    float KnobRounding = ImGuiToggleConstants::KnobRoundingDefault;

    float WidthRatio = ImGuiToggleConstants::WidthRatioDefault;
    float KnobInset = ImGuiToggleConstants::KnobInsetDefault;

    float FrameBorderThickness = ImGuiToggleConstants::BorderThicknessDefault;
    float KnobBorderThickness = ImGuiToggleConstants::BorderThicknessDefault;

    const char* OnLabel = "1";
    const char* OffLabel = "0";

    ImVec2 Size = ImVec2(0.0f, 0.0f);
};

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
    IMGUI_API bool Toggle(const char* label, bool* v, const ImGuiToggleConfig& config);

} // namespace ImGui
