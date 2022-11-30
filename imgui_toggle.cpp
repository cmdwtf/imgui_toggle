#include "imgui_toggle.h"

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif // IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

using namespace ImGuiToggleConstants;

// the actual logic takes place in ToggleInternal(), all the Toggle() calls are just interfaces for it.
static bool ToggleInternal(const char* label, bool* v, ImGuiToggleFlags flags, float speed, float frame_rounding, float knob_rounding);
static bool ToggleInternal(const char* label, bool* v, const ImGuiToggleConfig& config);

bool ImGui::Toggle(const char* label, bool* v)
{
    return ::ToggleInternal(label, v, ImGuiToggleFlags_Default, AnimationSpeedDisabled, FrameRoundingDefault, KnobRoundingDefault);
}

bool ImGui::Toggle(const char* label, bool* v, ImGuiToggleFlags flags)
{
    float speed = (flags & ImGuiToggleFlags_Animated) != 0 ? AnimationSpeedDefault : AnimationSpeedDisabled;
    return ::ToggleInternal(label, v, flags, speed, FrameRoundingDefault, KnobRoundingDefault);
}

bool ImGui::Toggle(const char* label, bool* v, ImGuiToggleFlags flags, float speed)
{
    // this overload implies the toggle should be animated.
    flags = (flags & ~ImGuiToggleFlags_Static) | (ImGuiToggleFlags_Animated);
    return ::ToggleInternal(label, v, flags, speed, FrameRoundingDefault, KnobRoundingDefault);
}

bool ImGui::Toggle(const char* label, bool* v, ImGuiToggleFlags flags, float frame_rounding, float knob_rounding)
{
    return ::ToggleInternal(label, v, flags, AnimationSpeedDisabled, frame_rounding, knob_rounding);
}

bool ImGui::Toggle(const char* label, bool* v, ImGuiToggleFlags flags, float speed, float frame_rounding, float knob_rounding)
{
    // this overload implies the toggle should be animated.
    flags = (flags & ~ImGuiToggleFlags_Static) | (ImGuiToggleFlags_Animated);
    return ::ToggleInternal(label, v, flags, speed, frame_rounding, knob_rounding);
}

bool ImGui::Toggle(const char* label, bool* v, const ImGuiToggleConfig& config)
{
    return ::ToggleInternal(label, v, config);
}

static bool ToggleInternal(const char* label, bool* v, ImGuiToggleFlags flags, float speed, float frame_rounding, float knob_rounding)
{
    // setup config structure
    static ImGuiToggleConfig config;
    config.Flags = flags;
    config.AnimationSpeed = speed;
    config.FrameRounding = frame_rounding;
    config.KnobRounding = knob_rounding;

    return ToggleInternal(label, v, config);
}

static bool ToggleInternal(const char* label, bool* v, const ImGuiToggleConfig& config)
{
    // locals config that may need to be modified before drawing (clamping, etc)
    ImGuiToggleConfig cfg = config;

    IM_ASSERT_USER_ERROR(cfg.Size.x >= 0, "Size.x specified was negative.");
    IM_ASSERT_USER_ERROR(cfg.Size.y >= 0, "Size.y specified was negative.");

    // a fixed value that works nicely for animation.
    constexpr float ToggleAnimationSpeedScaler = 0.08f;

    // if no flags were specified, use defaults.

    if (cfg.Flags == ImGuiToggleFlags_None)
    {
        cfg.Flags = ImGuiToggleFlags_Default;
    }

    if (cfg.AnimationSpeed > 0)
    {
        cfg.Flags = (cfg.Flags & ~ImGuiToggleFlags_Static) | (ImGuiToggleFlags_Animated);
    }

    // a zero or negative speed would prevent animation.
    cfg.AnimationSpeed = ImMax(cfg.AnimationSpeed, AnimationSpeedMinimum);

    // keep our size/scale and rounding numbers sane.
    cfg.FrameRounding = ImClamp(cfg.FrameRounding, FrameRoundingMinimum, FrameRoundingMaximum);
    cfg.KnobRounding = ImClamp(cfg.KnobRounding, KnobRoundingMinimum, KnobRoundingMaximum);
    cfg.WidthRatio = ImClamp(cfg.WidthRatio, WidthRatioMinimum, WidthRatioMaximum);
    cfg.KnobInset = ImClamp(cfg.KnobInset, KnobInsetMinimum, KnobInsetMaximum);

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
    {
        return false;
    }

    // context information
    ImGuiContext& g = *GImGui;
    const ImGuiStyle style = ImGui::GetStyle();
    const ImVec4* colors = style.Colors;
    const ImVec2 pos = window->DC.CursorPos;
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

    // modes
    const bool is_circle_knob = cfg.KnobRounding >= 1.0f;
    const bool is_rectangle_knob = cfg.KnobRounding < 1.0f;
    const bool is_animated = (cfg.Flags & ImGuiToggleFlags_Animated) != 0;
    const bool is_static = (cfg.Flags & ImGuiToggleFlags_Static) != 0;
    const bool has_bordered_frame = (cfg.Flags & ImGuiToggleFlags_BorderedFrame) != 0;
    const bool has_bordered_knob = (cfg.Flags & ImGuiToggleFlags_BorderedKnob) != 0;
    const bool show_a11y_labels = (cfg.Flags & ImGuiToggleFlags_A11yLabels) != 0;
    const bool highlight_enabled = (cfg.Flags & ImGuiToggleFlags_HighlightEnabledKnob) != 0;

    // users likely don't intend to double up on modes.
    IM_ASSERT(!is_circle_knob || !is_rectangle_knob);
    IM_ASSERT(!is_animated || !is_static);

    // calculate some sizes for the widget
    const float height = cfg.Size.y > 0
        ? cfg.Size.y
        : ImGui::GetFrameHeight();
    const float width = cfg.Size.x > 0
        ? cfg.Size.x
        : height * cfg.WidthRatio;

    // radius is by default half the diameter
    const float radius = height * DiameterToRadiusRatio;
    const float half_height = height * 0.5f;
    const float double_radius = radius * 2.0f;
    const float background_rounding = cfg.FrameRounding >= 0
        ? height * cfg.FrameRounding
        : half_height;

    // prevent an inset from causing the knob to be inverted
    if (cfg.KnobInset > radius)
    {
        cfg.KnobInset = radius;
    }

    // calculate total bounding box including the label
    const ImRect total_bb(pos, pos + ImVec2(width + (label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f), ImMax(height, label_size.y) + style.FramePadding.y * 2.0f));
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
        float speed_mul = 1.0f / cfg.AnimationSpeed;
        float t_anim = ImSaturate(g.LastActiveIdTimer / (speed_mul * ToggleAnimationSpeedScaler));
        t = *v ? (t_anim) : (1.0f - t_anim);
    }

    // FIXME: toggle knobs could get their own theme colors rather than using text & buttons.
    const ImU32 color_knob_default = ImGui::GetColorU32(colors[ImGuiCol_Text]);
    const ImU32 color_knob_enabled = highlight_enabled ? ImGui::GetColorU32(colors[ImGuiCol_CheckMark]) : color_knob_default;
    const ImU32 color_knob = *v ? color_knob_enabled : color_knob_default;
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
        draw_list->AddRect(frame_min, frame_max, color_border, background_rounding, ImDrawFlags_None, cfg.FrameBorderThickness);
    }

    // draw accessibility labels, if enabled.
    if (show_a11y_labels)
    {
        auto on_pos = toggle_bb.GetTL();
        auto off_pos = toggle_bb.GetTR();
        const ImVec2 off_text_size = ImGui::CalcTextSize(cfg.OffLabel);

        on_pos += style.FramePadding;
        off_pos.x -= (style.FramePadding.x + off_text_size.x);
        off_pos.y += style.FramePadding.y;


        ImGui::RenderText(on_pos, cfg.OnLabel);
        ImGui::RenderText(off_pos, cfg.OffLabel);
    }

    if (is_circle_knob)
    {
        const float knob_x = pos.x + radius + t * (width - double_radius);
        const float knob_y = pos.y + radius;
        const ImVec2 knob_center = ImVec2(knob_x, knob_y);
        const float knob_radius = radius - cfg.KnobInset;

        // draw circle knob
        draw_list->AddCircleFilled(knob_center, knob_radius, color_knob);

        // draw knob border, if enabled
        if (has_bordered_knob)
        {
            draw_list->AddCircle(knob_center, knob_radius, color_border, 0, cfg.KnobBorderThickness);
        }
    }
    else if (is_rectangle_knob)
    {

        const float knob_inset_double = cfg.KnobInset * 2.0f;
        const float knob_left = (t * (width - double_radius)) + cfg.KnobInset;
        const float knob_top = cfg.KnobInset;
        const float knob_bottom = height - cfg.KnobInset;
        const float knob_right = knob_left + double_radius - knob_inset_double;

        const float knob_diameter_total = knob_bottom - knob_top;
        const float knob_rounded_radius = (knob_diameter_total * 0.5f) * cfg.KnobRounding;

        const ImVec2 knob_min = pos + ImVec2(knob_left, knob_top);
        const ImVec2 knob_max = pos + ImVec2(knob_right, knob_bottom);

        // draw rectangle/squircle knob 
        draw_list->AddRectFilled(knob_min, knob_max, color_knob, knob_rounded_radius);

        // draw knob border, if enabled
        if (has_bordered_knob)
        {
            draw_list->AddRect(knob_min, knob_max, color_border, knob_rounded_radius, 0, cfg.KnobBorderThickness);
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
