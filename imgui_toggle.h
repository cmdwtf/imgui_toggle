#pragma once

#include "imgui.h"

// Type forward declarations, definitions below in this file.
typedef int ImGuiToggleFlags;               // -> enum ImGuiToggleFlags_        // Flags: for Toggle() modes
struct ImGuiToggleConfig;                   // Configuration data to fully customize a toggle.
struct ImGuiTogglePalette;                  // Color data to adjust how a toggle is drawn.

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
    // - size: A width and height to draw the toggle at. Defaults to `ImGui::GetFrameHeight()` and that height * Phi for the width.
    // Config structure:
    // - The overload taking a reference to an ImGuiToggleConfig structure allows for more complete control over the widget.
    IMGUI_API bool Toggle(const char* label, bool* v, const ImVec2& size = ImVec2());
    IMGUI_API bool Toggle(const char* label, bool* v, ImGuiToggleFlags flags, const ImVec2& size = ImVec2());
    IMGUI_API bool Toggle(const char* label, bool* v, ImGuiToggleFlags flags, float speed, const ImVec2& size = ImVec2());
    IMGUI_API bool Toggle(const char* label, bool* v, ImGuiToggleFlags flags, float frame_rounding, float knob_rounding, const ImVec2& size = ImVec2());
    IMGUI_API bool Toggle(const char* label, bool* v, ImGuiToggleFlags flags, float speed, float frame_rounding, float knob_rounding, const ImVec2& size = ImVec2());
    IMGUI_API bool Toggle(const char* label, bool* v, const ImGuiToggleConfig& config);

} // namespace ImGui


// ImGuiToggleFlags: A set of flags that adjust behavior and display for ImGui::Toggle().
enum ImGuiToggleFlags_
{
    ImGuiToggleFlags_None                   = 0,
    ImGuiToggleFlags_AnimatedKnob           = 1 << 0, // The toggle's knob should be animated.
    ImGuiToggleFlags_AnimatedFrameColor     = 1 << 1, // The toggle's frame should interpolate color when it animates.
    ImGuiToggleFlags_AnimatedKnobColor      = 1 << 2, // The toggle's knob should interpolate color when it animates.
    ImGuiToggleFlags_BorderedFrame          = 1 << 3, // The toggle should have a border drawn on the frame.
    ImGuiToggleFlags_BorderedKnob           = 1 << 4, // The toggle should have a border drawn on the knob.
                                                      // Bits 5-7 reserved.
    ImGuiToggleFlags_A11yLabels             = 1 << 8, // The toggle should draw on and off labels to indicate its state.
    ImGuiToggleFlags_Bordered               = ImGuiToggleFlags_BorderedFrame | ImGuiToggleFlags_BorderedKnob, // Shorthand for bordered frame and knob.
    ImGuiToggleFlags_Animated               = ImGuiToggleFlags_AnimatedKnob |
                                                ImGuiToggleFlags_AnimatedFrameColor |
                                                ImGuiToggleFlags_AnimatedKnobColor, // Shorthand for animated and interpolated colors.
    ImGuiToggleFlags_Default                = ImGuiToggleFlags_None, // The default flags used when no ImGuiToggleFlags_ are specified.
};

// ImGuiToggleConstants: A set of defaults and limits used by ImGuiToggleConfig
namespace ImGuiToggleConstants
{
    // The golden ratio.
    constexpr float Phi = 1.6180339887498948482045f;

    // d = 2r
    constexpr float DiameterToRadiusRatio = 0.5f;

    // Animation is disabled with a speed of 0.
    constexpr float AnimationSpeedDisabled = 0.0f;

    // The default animation speed. (1.0f: Default speed.)
    constexpr float AnimationSpeedDefault = 1.0f;

    // The lowest allowable value for animation speed. (0.0f: Disabled animation.)
    constexpr float AnimationSpeedMinimum = AnimationSpeedDisabled;

    // The default frame rounding value. (1.0f: Full rounding.)
    constexpr float FrameRoundingDefault = 1.0f;

    // The minimum frame rounding value. (0.0f: Full rectangle.)
    constexpr float FrameRoundingMinimum = 0.0f;

    // The maximum frame rounding value. (1.0f: Full rounding.)
    constexpr float FrameRoundingMaximum = 1.0f;

    // The default knob rounding value. (1.0f: Full rounding.)
    constexpr float KnobRoundingDefault = 1.0f;

    // The minimum knob rounding value. (0.0f: Full rectangle.)
    constexpr float KnobRoundingMinimum = 0.0f;

    // The maximum knob rounding value. (1.0f: Full rounding.)
    constexpr float KnobRoundingMaximum = 1.0f;

    // The default height to width ratio. (Phi: The golden ratio.)
    constexpr float WidthRatioDefault = Phi;

    // The minimum allowable width ratio. (1.0f: Toggle width==height, not very useful but interesting.)
    constexpr float WidthRatioMinimum = 1.0f;

    // The maximum allowable width ratio. (10.0f: It starts to get silly quickly.)
    constexpr float WidthRatioMaximum = 10.0f;

    // The default amount of pixels the knob should be inset into the toggle frame. (1.5f: Pleasing to the eye.)
    constexpr float KnobInsetDefault = 1.5f;

    // The minimum amount of pixels the knob should be negatively inset (outset) from the toggle frame. (-100.0f: Big overgrown toggle.)
    constexpr float KnobInsetMinimum = -100.0f;

    // The maximum amount of pixels the knob should be inset into the toggle frame. (100.0f: Toggle likely invisible!)
    constexpr float KnobInsetMaximum = 100.0f;

    /// The default thickness for borders drawn on the toggle frame and knob.
    constexpr float BorderThicknessDefault = 1.0f;
}

// ImGuiToggleConfig: A collection of data used to customize the appearance and behavior of a toggle widget.
struct ImGuiToggleConfig
{
    // Flags that control the toggle's behavior and display.
    ImGuiToggleFlags Flags = ImGuiToggleFlags_Default;

    // The a speed multiplier the toggle should animate at. If 0, animation will be disabled.
    float AnimationSpeed = ImGuiToggleConstants::AnimationSpeedDefault;

    // A scalar that controls how rounded the toggle frame is. 0 is square, 1 is round. (0, 1) default 1.0f
    float FrameRounding = ImGuiToggleConstants::FrameRoundingDefault;

    //A scalar that controls how rounded the toggle knob is. 0 is square, 1 is round. (0, 1) default 1.0f
    float KnobRounding = ImGuiToggleConstants::KnobRoundingDefault;

    // A ratio that controls how wide the toggle is compared to it's height. If `Size.x` is non-zero, this value is ignored.
    float WidthRatio = ImGuiToggleConstants::WidthRatioDefault;

    // The number of pixels the knob should be inset into the toggle frame. A negative value will cause the knob to draw outside the frame.
    float KnobInset = ImGuiToggleConstants::KnobInsetDefault;

    // The thickness the border should be drawn on the frame when ImGuiToggleFlags_BorderedFrame is specified in `Flags`.
    float FrameBorderThickness = ImGuiToggleConstants::BorderThicknessDefault;

    // The thickness the border should be drawn on the frame when ImGuiToggleFlags_BorderedKnob is specified in `Flags`.
    float KnobBorderThickness = ImGuiToggleConstants::BorderThicknessDefault;

    // The label drawn on the left side of the toggle to show the toggle is in the on state when ImGuiToggleFlags_A11yLabels is specified in `Flags`.
    const char* OnLabel = "1";

    // The label drawn on the right side of the toggle to show the toggle is in the on state when ImGuiToggleFlags_A11yLabels is specified in `Flags`.
    const char* OffLabel = "0";

    // A custom size to draw the toggle with. Defaults to (0, 0). If `Size.x` is zero, `WidthRatio` will control the toggle width.
    // If `Size.y` is zero, the toggle height will be set by `ImGui::GetFrameHeight()`.
    ImVec2 Size = ImVec2(0.0f, 0.0f);

    // A custom amount of pixels to offset the knob by. Positive x values will move the knob towards the inside, negative the outside.
    ImVec2 KnobOffset = ImVec2(0.0f, 0.0f);

    // An optional custom palette to use for the colors to use when drawing the toggle. If left null, theme colors will be used.
    // If any of the values in the palette are zero, those specific colors will default to theme colors.
    const ImGuiTogglePalette* Palette = nullptr;
};

// ImGuiTogglePalette: A collection of colors used to customize the rendering of a toggle widget.
//   Leaving any ImVec4 as default (zero) will allow the theme color to be used for that member.
struct ImGuiTogglePalette
{
    // The default knob color, used when when the knob is off.
    ImVec4 KnobOff;

    // The default knob color, used when when the knob is off and hovered.
    ImVec4 KnobOffHover;

    // The default knob color, used when when the knob is on.
    ImVec4 KnobOn;

    // The default knob color, used when when the knob is on and hovered.
    ImVec4 KnobOnHover;

    // The background color of the toggle frame when the toggle is off.
    ImVec4 FrameOff;

    // The background color of the toggle frame when the toggle is off and hovered.
    ImVec4 FrameOffHover;

    // The background color of the toggle frame when the toggle is on.
    ImVec4 FrameOn;

    // The background color of the toggle frame when the toggle is on and hovered.
    ImVec4 FrameOnHover;

    // The background color of the toggle frame's border used when ImGuiToggleFlags_BorderedFrame is specified.
    ImVec4 FrameBorder;

    // The background color of the toggle knob's border used when ImGuiToggleFlags_BorderedKnob is specified.
    ImVec4 KnobBorder;
};
