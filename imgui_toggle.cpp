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
    const bool has_bordered_frame = (flags & ImGuiToggleFlags_BorderedFrame) != 0;
    const bool has_bordered_knob = (flags & ImGuiToggleFlags_BorderedKnob) != 0;

    // users likely don't intend to double up on modes.
    IM_ASSERT(!is_circle_knob || !is_rectangle_knob);
    IM_ASSERT(!is_animated || !is_static);

    // calculate some sizes for the widget
    const float height = ImGui::GetFrameHeight();
    const float width = height * ToggleWidthRatio;
    const float radius = height * ToggleRadiusRatio;
    const float half_height = height * 0.5f;
    const float double_radius = radius * 2.0f;
    const float background_rounding = frame_rounding >= 0
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
    const ImGuiButtonFlags button_flags = ImGuiButtonFlags_PressedOnClick;
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
    const ImVec4 color_frame_off = colors[ImGuiCol_FrameBg];
    const ImVec4 color_frame_off_hover = colors[ImGuiCol_FrameBgHovered];
    const ImVec4 color_frame_on = colors[ImGuiCol_Button];
    const ImVec4 color_frame_on_hover = colors[ImGuiCol_ButtonHovered];
    //const ImVec4 color_frame_on_active = colors[ImGuiCol_ButtonActive];
    const ImU32 color_border = ImGui::GetColorU32(colors[ImGuiCol_Border]);

    // select or interpolate the background color.
    ImU32 background_color;

    if (g.HoveredId == id)
    {
        background_color = ImGui::GetColorU32(is_animated
            ? ImLerp(color_frame_off_hover, color_frame_on_hover, t)
            : (*v ? color_frame_on_hover : color_frame_off_hover));
    }
    else
    {
        background_color = ImGui::GetColorU32(is_animated
            ? ImLerp(color_frame_off, color_frame_on, t)
            : (*v ? color_frame_on : color_frame_off));
    }

    const ImVec2 frame_min = pos;
    const ImVec2 frame_max = ImVec2(pos.x + width, pos.y + height);

    // draw frame background
    draw_list->AddRectFilled(frame_min, frame_max, background_color, background_rounding);

    // draw frame border, if enabled
    if (has_bordered_frame)
    {
        draw_list->AddRect(frame_min, frame_max, color_border, background_rounding);
    }

    if (is_circle_knob)
    {
        const ImVec2 knob_center = ImVec2(pos.x + radius + t * (width - double_radius), pos.y + radius);
        const float knob_radius = radius - ToggleKnobOffset;

        // draw circle knob
        draw_list->AddCircleFilled(knob_center, knob_radius, color_knob);

        // draw knob border, if enabled
        if (has_bordered_knob)
        {
            draw_list->AddCircle(knob_center, knob_radius, color_border);
        }
    }
    else if (is_rectangle_knob)
    {
        const float knob_rounded_height = height * knob_rounding;
        const float knob_left = (t * (width - double_radius)) + ToggleKnobOffset;
        const float knob_top = ToggleKnobOffset;
        const float knob_bottom = height - ToggleKnobOffset;
        const float knob_right = knob_left + double_radius - ToggleKnobDoubleOffset;

        const ImVec2 knob_min = pos + ImVec2(knob_left, knob_top);
        const ImVec2 knob_max = pos + ImVec2(knob_right, knob_bottom);

        // draw rectangle/squircle knob 
        draw_list->AddRectFilled(knob_min, knob_max, color_knob, knob_rounded_height);

        // draw knob border, if enabled
        if (has_bordered_knob)
        {
            draw_list->AddRect(knob_min, knob_max, color_border, knob_rounded_height);
        }
    }
    else
    {
        // user didn't specify a knob mode, they get no knob.
        IM_ASSERT(false && "No toggle knob type to draw.");
    }

    const ImVec2 label_pos = ImVec2(toggle_bb.Max.x + style.ItemInnerSpacing.x, toggle_bb.Min.y + style.FramePadding.y);

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
