#include "imgui_toggle.h"

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif // IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

namespace
{
    constexpr float ToggleDefaultAnimationSpeed = 1.0f;
    constexpr float ToggleFrameRoundingDefault = 1.0f;
    constexpr float ToggleKnobRoundingDefault = 1.0f;
}

// the actual logic takes place in ToggleInternal(), all the Toggle() calls are just interfaces for it.
static bool ToggleInternal(const char* label, bool* v, ImGuiToggleFlags flags, float speed, float frame_rounding, float knob_rounding);

bool ImGui::Toggle(const char* label, bool* v)
{
    return ::ToggleInternal(label, v, ImGuiToggleFlags_Default, ::ToggleDefaultAnimationSpeed, ::ToggleFrameRoundingDefault, ::ToggleKnobRoundingDefault);
}

bool ImGui::Toggle(const char* label, bool* v, ImGuiToggleFlags flags)
{
    return ::ToggleInternal(label, v, flags, ::ToggleDefaultAnimationSpeed, ::ToggleFrameRoundingDefault, ::ToggleKnobRoundingDefault);
}

bool ImGui::Toggle(const char* label, bool* v, ImGuiToggleFlags flags, float speed)
{
    // this overload implies the toggle should be animated.
    flags = (flags & ~ImGuiToggleFlags_Static) | (ImGuiToggleFlags_Animated);
    return ::ToggleInternal(label, v, flags, speed, ToggleFrameRoundingDefault, ::ToggleKnobRoundingDefault);
}

bool ImGui::Toggle(const char* label, bool* v, ImGuiToggleFlags flags, float frame_rounding, float knob_rounding)
{
    return ::ToggleInternal(label, v, flags, ::ToggleDefaultAnimationSpeed, frame_rounding, knob_rounding);
}

bool ImGui::Toggle(const char* label, bool* v, ImGuiToggleFlags flags, float speed, float frame_rounding, float knob_rounding)
{
    // this overload implies the toggle should be animated.
    flags = (flags & ~ImGuiToggleFlags_Static) | (ImGuiToggleFlags_Animated);
    return ::ToggleInternal(label, v, flags, speed, frame_rounding, knob_rounding);
}

static bool ToggleInternal(const char* label, bool* v, ImGuiToggleFlags flags, float speed, float frame_rounding, float knob_rounding)
{
    // if no flags were specified, use defaults.
    if (flags == ImGuiToggleFlags_None)
    {
        flags = ImGuiToggleFlags_Default;
    }

    // a zero or negative speed would prevent animation.
    speed = ImMax(speed, 0.001f);

    // keep our rounding numbers sane.
    frame_rounding = ImClamp(frame_rounding, 0.0f, 1.0f);
    knob_rounding = ImClamp(knob_rounding, 0.0f, 1.0f);

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
    {
        return false;
    }

    // constants that could be part of a style/skin/etc.
    constexpr float ToggleAnimationSpeedScaler = 0.08f;
    constexpr float ToggleWidthRatio = 1.55f;
    constexpr float ToggleRadiusRatio = 0.50f;
    constexpr float ToggleKnobOffset = 1.5f;
    constexpr float ToggleKnobDoubleOffset = ToggleKnobOffset * 2.0f;

    // static colors
    // FIXME: should be part of style
    constexpr float dark_off_color = 0.45f;
    constexpr float light_off_color = 0.65f;
    constexpr ImVec4 color_gray_dark = ImVec4(dark_off_color, dark_off_color, dark_off_color, 1.0f);
    constexpr ImVec4 color_gray_light = ImVec4(light_off_color, light_off_color, light_off_color, 1.0f);

    // context information
    ImGuiContext& g = *GImGui;
    const ImGuiStyle style = ImGui::GetStyle();
    const ImVec4* colors = style.Colors;
    const ImVec2 pos = window->DC.CursorPos;
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

    // modes
    const bool is_circle_knob = knob_rounding >= 1.0f;
    const bool is_rectangle_knob = knob_rounding < 1.0f;
    const bool is_animated = (flags & ImGuiToggleFlags_Animated) != 0;
    const bool is_static = (flags & ImGuiToggleFlags_Static) != 0;

    // users likely don't intend to double up on modes.
    IM_ASSERT(!is_circle_knob || !is_rectangle_knob);
    IM_ASSERT(!is_animated || !is_static);

    // calculate some sizes for the widget
    const float height = ImGui::GetFrameHeight();
    const float width = height * ToggleWidthRatio;
    const float radius = height * ToggleRadiusRatio;
    const float half_height = height * 0.5f;
    const float double_radius = radius * 2.0f;
    const float frame_height = frame_rounding >= 0
        ? height * frame_rounding
        : half_height;

    // calculate total bounding box including the label
    const ImRect total_bb(pos, pos + ImVec2(width + (label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f), label_size.y + style.FramePadding.y * 2.0f));
    ImGui::ItemSize(total_bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(total_bb, id))
    {
        IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags | ImGuiItemStatusFlags_Checkable | (*v ? ImGuiItemStatusFlags_Checked : 0));
        return false;
    }

    // the meat and potatoes: the actual toggle button
    ImGuiButtonFlags button_flags = ImGuiButtonFlags_PressedOnClick;
    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(total_bb, id, &hovered, &held, button_flags);
    if (pressed)
    {
        *v = !(*v);
        ImGui::MarkItemEdited(id);
    }

    const ImRect toggle_bb(pos, pos + ImVec2(width, height));

    bool mixed_value = (g.LastItemData.InFlags & ImGuiItemFlags_MixedValue) != 0;

    // calculate the lerp percentage for animation,
    // but default to 1/0 for if we aren't animating at all,
    // or 0.5f if we have a mixed value. Also, trying to keep parity with
    // undocumented tristate/mixed/indeterminate checkbox (#2644)
    float t = mixed_value
        ? 0.5f
        : (*v ? 1.0f : 0.0f);

    if (is_animated && g.LastActiveId == id)
    {
        float speed_mul = 1.0f / (speed > 0 ? speed : 1.0f);
        float t_anim = ImSaturate(g.LastActiveIdTimer / (speed_mul * ToggleAnimationSpeedScaler));
        t = *v ? (t_anim) : (1.0f - t_anim);
    }

    // FIXME: toggle knobs could get their own theme colors rather than using text & buttons.
    const ImU32 color_knob = ImGui::GetColorU32(colors[ImGuiCol_Text]);
    const ImVec4 on_color_active = colors[ImGuiCol_ButtonActive];
    const ImVec4 on_color_hover = colors[ImGuiCol_ButtonHovered];
    const ImVec4 on_color = colors[ImGuiCol_Button];

    // select or interpolate the background color.
    ImU32 background_color;

    if (g.HoveredId == id)
    {
        background_color = ImGui::GetColorU32(is_animated
            ? ImLerp(color_gray_light, on_color_hover, t)
            : (*v ? on_color_hover : color_gray_light));
    }
    else
    {
        background_color = ImGui::GetColorU32(is_animated
            ? ImLerp(color_gray_dark, on_color, t)
            : (*v ? on_color : color_gray_dark));
    }

    // draw background
    draw_list->AddRectFilled(pos, ImVec2(pos.x + width, pos.y + height), background_color, frame_height);

    // draw knob
    if (is_circle_knob)
    {
        draw_list->AddCircleFilled(ImVec2(pos.x + radius + t * (width - double_radius), pos.y + radius), radius - ToggleKnobOffset, color_knob);
    }
    else if (is_rectangle_knob)
    {
        float knob_rounded_height = height * knob_rounding;

        float knob_left = (t * (width - double_radius)) + ToggleKnobOffset;
        float knob_top = ToggleKnobOffset;
        float knob_bottom = height - ToggleKnobOffset;
        float knob_right = knob_left + double_radius - ToggleKnobDoubleOffset;

        ImVec2 knob_min = pos + ImVec2(knob_left, knob_top);
        ImVec2 knob_max = pos + ImVec2(knob_right, knob_bottom);

        draw_list->AddRectFilled(knob_min, knob_max, color_knob, knob_rounded_height);
    }
    else
    {
        // user didn't specify a knob mode, they get no knob.
        IM_ASSERT(false && "No toggle knob type to draw.");
    }

    ImVec2 label_pos = ImVec2(toggle_bb.Max.x + style.ItemInnerSpacing.x, toggle_bb.Min.y + style.FramePadding.y);

    if (g.LogEnabled)
    {
        ImGui::LogRenderedText(&label_pos, mixed_value ? "[~]" : *v ? "[x]" : "[ ]");
    }

    if (label_size.x > 0.0f)
    {
        ImGui::RenderText(label_pos, label);
    }

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags | ImGuiItemStatusFlags_Checkable | (*v ? ImGuiItemStatusFlags_Checked : 0));
    return pressed;
}
