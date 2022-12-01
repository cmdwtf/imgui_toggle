#include "imgui_toggle.h"

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif // IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

using namespace ImGuiToggleConstants;

namespace
{
    // the actual logic takes place in `ToggleInternal()`, all the `Toggle()` calls are just interfaces for it.
    bool ToggleInternal(const char* label, bool* v, const ImGuiToggleConfig& config);

    // Helpers to break up the logic from `ToggleInternal()` into logical chunks.
    const ImGuiTogglePalette* GetPalette(const ImGuiTogglePalette* candidate, const ImGuiStyle& style);
    void DrawToggleLabel(const ImRect& toggle_bb, const char* label, bool* v, bool is_mixed_value, float x_offset);
    void DrawToggle(const ImRect& toggle_bb, ImGuiID id, bool* v, bool is_mixed_value, ImGuiToggleConfig& config);
    void DrawToggleRectangleKnob(ImDrawList* draw_list, const ImRect& bb, const ImGuiToggleConfig& config, float radius, float t, ImU32 color_knob, ImU32 color_knob_border);
    void DrawToggleCircleKnob(ImDrawList* draw_list, const ImRect& bb, const ImGuiToggleConfig& config, float radius, float t, ImU32 color_knob, ImU32 color_knob_border);
    void DrawToggleA11yLabels(ImDrawList* draw_list, const ImRect& bb, const ImGuiToggleConfig& config, const ImVec2& frame_padding);
    void DrawToggleFrame(ImDrawList* draw_list, const ImRect& bb, const ImGuiToggleConfig& config, ImU32 color_frame, ImU32 color_frame_border);
    float GetToggleAnimationT(ImGuiID id, bool v, bool is_mixed_value, const ImGuiToggleConfig& config);
    bool ToggleBehavior(const ImRect bb, const ImGuiID id, const char* label, bool* v);
    void ToggleValidateConfig(ImGuiToggleConfig& config);

    // inline helpers

    // lerp, but backwards!
    template<typename T> constexpr inline T ImInvLerp(T a, T b, float value) { return (T)((value - a) / (b - a)); }

    // sets the given config structure's values to the
    // default ones used by the `Toggle()` overloads.
    inline void SetToAliasDefaults(ImGuiToggleConfig& config)
    {
        config.Flags = ImGuiToggleFlags_Default;
        config.AnimationDuration = AnimationDurationDisabled;
        config.FrameRounding = FrameRoundingDefault;
        config.KnobRounding = KnobRoundingDefault;
    }

    // shorthand for `ImGui::GetColorU32()`.
    inline ImU32 AsColor(const ImVec4& v)
    {
        return ImGui::GetColorU32(v);
    }

    // gets color a or b, or interpolates between them by t if interpolate is true.
    inline ImU32 AsColorInterpolated(const ImVec4& color_a, const ImVec4& color_b, float t, bool interpolate)
    {
        return ImGui::GetColorU32(interpolate
            ? ImLerp(color_a, color_b, t)
            : (t < 0.5f ? color_a : color_b));
    }

    // helpers for checking if an ImVec4 is zero or not.
    constexpr inline bool IsZero(const ImVec4& v) { return v.w == 0 && v.x == 0 && v.y == 0 && v.z == 0; }
    constexpr inline bool IsNonZero(const ImVec4& v) { return v.w != 0 || v.x != 0 || v.y != 0 || v.z != 0; }

    // thread-local data for the `Toggle()` functions to easily call `ToggleInternal()`.
    static thread_local ImGuiToggleConfig _internalConfig;
} // namespace

bool ImGui::Toggle(const char* label, bool* v, const ImVec2& size /*= ImVec2()*/)
{
    ::SetToAliasDefaults(::_internalConfig);
    return ::ToggleInternal(label, v, ::_internalConfig);
}

bool ImGui::Toggle(const char* label, bool* v, ImGuiToggleFlags flags, const ImVec2& size /*= ImVec2()*/)
{
    ::SetToAliasDefaults(::_internalConfig);
    ::_internalConfig.Flags = flags;
    ::_internalConfig.Size = size;

    // if the user is using any animation flags,
    // set the default duration.
    if ((flags & ImGuiToggleFlags_Animated) != 0)
    {
        _internalConfig.AnimationDuration = AnimationDurationDefault;
    }
    
    return ::ToggleInternal(label, v, ::_internalConfig);
}

bool ImGui::Toggle(const char* label, bool* v, ImGuiToggleFlags flags, float animation_duration, const ImVec2& size /*= ImVec2()*/)
{
    // this overload implies the toggle should be animated.
    if (animation_duration > 0 && (flags & ImGuiToggleFlags_AnimatedKnob) != 0)
    {
        // if the user didn't specify ImGuiToggleFlags_AnimatedKnob, enable all animations.
        flags = flags | (ImGuiToggleFlags_Animated);
    }

    ::SetToAliasDefaults(::_internalConfig);
    ::_internalConfig.Flags = flags;
    ::_internalConfig.AnimationDuration = animation_duration;
    ::_internalConfig.Size = size;

    return ::ToggleInternal(label, v, ::_internalConfig);
}

bool ImGui::Toggle(const char* label, bool* v, ImGuiToggleFlags flags, float frame_rounding, float knob_rounding, const ImVec2& size /*= ImVec2()*/)
{
    ::SetToAliasDefaults(::_internalConfig);
    ::_internalConfig.Flags = flags;
    ::_internalConfig.FrameRounding = frame_rounding;
    ::_internalConfig.KnobRounding = knob_rounding;
    ::_internalConfig.Size = size;

    return ::ToggleInternal(label, v, ::_internalConfig);
}

bool ImGui::Toggle(const char* label, bool* v, ImGuiToggleFlags flags, float animation_duration, float frame_rounding, float knob_rounding, const ImVec2& size /*= ImVec2()*/)
{
    // this overload implies the toggle should be animated.
    if (animation_duration > 0 && (flags & ImGuiToggleFlags_AnimatedKnob) != 0)
    {
        // if the user didn't specify ImGuiToggleFlags_AnimatedKnob, enable all animations.
        flags = flags | (ImGuiToggleFlags_Animated);
    }

    ::_internalConfig.Flags = flags;
    ::_internalConfig.AnimationDuration = animation_duration;
    ::_internalConfig.FrameRounding = frame_rounding;
    ::_internalConfig.KnobRounding = knob_rounding;
    ::_internalConfig.Size = size;

    return ::ToggleInternal(label, v, ::_internalConfig);
}

bool ImGui::Toggle(const char* label, bool* v, const ImGuiToggleConfig& config)
{
    return ::ToggleInternal(label, v, config);
}

namespace
{
    // a small helper to quickly check the mixed value flag.
    inline bool IsItemMixedValue()
    {
        return (GImGui->LastItemData.InFlags & ImGuiItemFlags_MixedValue) != 0;
    }

    bool ToggleInternal(const char* label, bool* v, const ImGuiToggleConfig& config)
    {
        // locals config that may need to be modified before drawing (clamping, etc)
        ImGuiToggleConfig cfg = config;
        ToggleValidateConfig(cfg);

        ImGuiWindow* window = ImGui::GetCurrentWindow();

        IM_ASSERT(window);

        if (window->SkipItems)
        {
            return false;
        }

        // context information
        const ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = ImGui::GetStyle();
        const ImVec2 widget_position = window->DC.CursorPos;
        const ImGuiID id = window->GetID(label);
        const ImVec2 label_size = ImGui::CalcTextSize(label, nullptr, true);

        // calculate some sizes for the widget
        const float height = cfg.Size.y > 0
            ? cfg.Size.y
            : ImGui::GetFrameHeight();
        const float width = cfg.Size.x > 0
            ? cfg.Size.x
            : height * cfg.WidthRatio;

        // if the knob is offset horizontally outside of the frame, we want to bump our label over.
        const float label_x_offset = ImMax(0.0f, -config.KnobOffset.x / 2.0f);

        // calculate bounding boxes for the toggle and the whole widget including the label
        const ImRect toggle_bb(widget_position, widget_position + ImVec2(width, height));
        const ImRect total_bb(widget_position,
            widget_position
            + ImVec2(
                width + (label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f) + label_x_offset,
                ImMax(height, label_size.y) + style.FramePadding.y * 2.0f
            ));

        // handle the toggle input behavior
        bool pressed = ::ToggleBehavior(total_bb, id, label, v);
        bool is_mixed_value = ::IsItemMixedValue();

        // draw the toggle itself and the label
        ::DrawToggle(toggle_bb, id, v, is_mixed_value, cfg);
        ::DrawToggleLabel(toggle_bb, label, v, is_mixed_value, label_x_offset);

        IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags | ImGuiItemStatusFlags_Checkable | (*v ? ImGuiItemStatusFlags_Checked : 0));
        return pressed;
    }

    void ToggleValidateConfig(ImGuiToggleConfig& config)
    {
        IM_ASSERT_USER_ERROR(config.Size.x >= 0, "Size.x specified was negative.");
        IM_ASSERT_USER_ERROR(config.Size.y >= 0, "Size.y specified was negative.");

        // if no flags were specified, use defaults.
        if (config.Flags == ImGuiToggleFlags_None)
        {
            config.Flags = ImGuiToggleFlags_Default;
        }

        // a zero or negative duration would prevent animation.
        config.AnimationDuration = ImMax(config.AnimationDuration, AnimationDurationMinimum);

        // keep our size/scale and rounding numbers sane.
        config.FrameRounding = ImClamp(config.FrameRounding, FrameRoundingMinimum, FrameRoundingMaximum);
        config.KnobRounding = ImClamp(config.KnobRounding, KnobRoundingMinimum, KnobRoundingMaximum);
        config.WidthRatio = ImClamp(config.WidthRatio, WidthRatioMinimum, WidthRatioMaximum);
        config.KnobInset = ImClamp(config.KnobInset, KnobInsetMinimum, KnobInsetMaximum);
    }

    bool ToggleBehavior(const ImRect bb, const ImGuiID id, const char* label, bool* v)
    {
        const ImGuiContext& g = *GImGui;
        const ImGuiWindow* window = ImGui::GetCurrentWindow();
        const ImGuiStyle& style = ImGui::GetStyle();

        ImGui::ItemSize(bb, style.FramePadding.y);
        if (!ImGui::ItemAdd(bb, id))
        {
            IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags | ImGuiItemStatusFlags_Checkable | (*v ? ImGuiItemStatusFlags_Checked : 0));
            return false;
        }

        // the meat and potatoes: the actual toggle button
        const ImGuiButtonFlags button_flags = ImGuiButtonFlags_PressedOnClick;
        bool hovered, held;
        bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, button_flags);
        if (pressed)
        {
            *v = !(*v);
            ImGui::MarkItemEdited(id);
        }

        return pressed;
    }

    float GetToggleAnimationT(ImGuiID id, bool v, bool is_mixed_value, const ImGuiToggleConfig& config)
    {
        const ImGuiContext& g = *GImGui;
        const bool is_animated = (config.Flags & ImGuiToggleFlags_AnimatedKnob) != 0 && config.AnimationDuration > 0;

        float t = is_mixed_value
            ? 0.5f
            : (v ? 1.0f : 0.0f);

        if (is_animated && g.LastActiveId == id)
        {
            const float t_anim = ImSaturate(ImInvLerp(0.0f, config.AnimationDuration, g.LastActiveIdTimer));
            t = v ? (t_anim) : (1.0f - t_anim);
        }

        return t;
    }

    void DrawToggleFrame(ImDrawList* draw_list, const ImRect& bb, const ImGuiToggleConfig& config, ImU32 color_frame, ImU32 color_frame_border)
    {
        const bool has_bordered_frame = (config.Flags & ImGuiToggleFlags_BorderedFrame) != 0;
        const float height = bb.GetHeight();
        const float background_rounding = config.FrameRounding >= 0
            ? height * config.FrameRounding
            : height * 0.5f;

        // draw frame background
        draw_list->AddRectFilled(bb.Min, bb.Max, color_frame, background_rounding);

        // draw frame border, if enabled
        if (has_bordered_frame)
        {
            draw_list->AddRect(bb.Min, bb.Max, color_frame_border, background_rounding, ImDrawFlags_None, config.FrameBorderThickness);
        }
    }

    void DrawToggleA11yLabels(ImDrawList* draw_list, const ImRect& bb, const ImGuiToggleConfig& config, const ImVec2& frame_padding)
    {
        const float half_height = bb.GetHeight() * 0.5f;

        ImVec2 on_pos = bb.GetTL();
        ImVec2 off_pos = bb.GetTR();
        const ImVec2 on_text_size = ImGui::CalcTextSize(config.OnLabel);
        const ImVec2 off_text_size = ImGui::CalcTextSize(config.OffLabel);

        // on text should be bumped down and left into the frame.
        on_pos.x += frame_padding.x;
        on_pos.y += half_height - (on_text_size.y * 0.5f);

        // off text should be bumped down and right by padding, and right by it's length.
        off_pos.x -= (frame_padding.x + off_text_size.x);
        off_pos.y += half_height - (off_text_size.y * 0.5f);

        // draw the labels.
        ImGui::RenderText(on_pos, config.OnLabel);
        ImGui::RenderText(off_pos, config.OffLabel);
    }

    void DrawToggleCircleKnob(ImDrawList* draw_list, const ImRect& bb, const ImGuiToggleConfig& config, float radius, float t, ImU32 color_knob, ImU32 color_knob_border)
    {
        const bool has_bordered_knob = (config.Flags & ImGuiToggleFlags_BorderedKnob) != 0;
        const float double_radius = radius * 2.0f;
        const ImVec2 widget_position = bb.Min;
        const float width = bb.GetWidth();
        const float one_minus_t = 1.0f - t;

        const float half_knob_x_offset = config.KnobOffset.x * 0.5f;
        const float knob_x = (widget_position.x + radius)
            + t * (width - double_radius - half_knob_x_offset)
            + (one_minus_t * half_knob_x_offset);
        const float knob_y = widget_position.y + radius + config.KnobOffset.y;
        const ImVec2 knob_center = ImVec2(knob_x, knob_y);
        const float knob_radius = radius - config.KnobInset;

        // draw circle knob
        draw_list->AddCircleFilled(knob_center, knob_radius, color_knob);

        // draw knob border, if enabled
        if (has_bordered_knob)
        {
            draw_list->AddCircle(knob_center, knob_radius, color_knob_border, 0, config.KnobBorderThickness);
        }
    }

    void DrawToggleRectangleKnob(ImDrawList* draw_list, const ImRect& bb, const ImGuiToggleConfig& config, float radius, float t, ImU32 color_knob, ImU32 color_knob_border)
    {
        const bool has_bordered_knob = (config.Flags & ImGuiToggleFlags_BorderedKnob) != 0;
        const float double_radius = radius * 2.0f;
        const ImVec2 widget_position = bb.Min;
        const float width = bb.GetWidth();
        const float height = bb.GetHeight();
        const float one_minus_t = 1.0f - t;

        const float half_knob_x_offset = config.KnobOffset.x * 0.5f;
        const float knob_inset_double = config.KnobInset * 2.0f;
        const float knob_left = (t * (width - double_radius - half_knob_x_offset))
            + (one_minus_t * half_knob_x_offset)
            + config.KnobInset;
        const float knob_top = config.KnobInset + config.KnobOffset.y;
        const float knob_bottom = height - config.KnobInset + config.KnobOffset.y;
        const float knob_right = knob_left + double_radius - knob_inset_double;

        const float knob_diameter_total = knob_bottom - knob_top;
        const float knob_rounded_radius = (knob_diameter_total * 0.5f) * config.KnobRounding;

        const ImVec2 knob_min = widget_position + ImVec2(knob_left, knob_top);
        const ImVec2 knob_max = widget_position + ImVec2(knob_right, knob_bottom);

        // draw rectangle/squircle knob 
        draw_list->AddRectFilled(knob_min, knob_max, color_knob, knob_rounded_radius);

        // draw knob border, if enabled
        if (has_bordered_knob)
        {
            draw_list->AddRect(knob_min, knob_max, color_knob_border, knob_rounded_radius, ImDrawFlags_None, config.KnobBorderThickness);
        }
    }

    void DrawToggle(const ImRect& toggle_bb, ImGuiID id, bool* v, bool is_mixed_value, ImGuiToggleConfig& config)
    {
        // context information
        const ImGuiWindow* window = ImGui::GetCurrentWindow();
        const ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = ImGui::GetStyle();
        const ImVec4* colors = style.Colors;
        const ImVec2 widget_position = window->DC.CursorPos;
        const ImGuiTogglePalette& palette = *::GetPalette(config.Palette, style);

        const float height = toggle_bb.GetHeight();
        const float width = toggle_bb.GetWidth();

        // drawing target.
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        // modes with readable names
        const bool is_circle_knob = config.KnobRounding >= 1.0f;
        const bool is_rectangle_knob = config.KnobRounding < 1.0f;
        const bool is_animated = (config.Flags & ImGuiToggleFlags_AnimatedKnob) != 0 && config.AnimationDuration > 0;
        const bool is_animated_frame_color = is_animated && (config.Flags & ImGuiToggleFlags_AnimatedFrameColor) != 0;
        const bool is_animated_knob_color = is_animated && (config.Flags & ImGuiToggleFlags_AnimatedKnobColor) != 0;
        const bool has_bordered_knob = (config.Flags & ImGuiToggleFlags_BorderedKnob) != 0;
        const bool show_a11y_labels = (config.Flags & ImGuiToggleFlags_A11yLabels) != 0;

        // radius is by default half the diameter
        const float radius = height * DiameterToRadiusRatio;

        // prevent an inset from causing the knob to be inverted
        config.KnobInset = ImMin(config.KnobInset, radius);

        // calculate the lerp percentage for animation,
        // but default to 1/0 for if we aren't animating at all,
        // or 0.5f if we have a mixed value. Also, trying to keep parity with
        // undocumented tristate/mixed/indeterminate checkbox (#2644)
        const float t = ::GetToggleAnimationT(id, *v, is_mixed_value, config);

        ImU32 color_frame;
        ImU32 color_knob;
        const ImU32 color_knob_border = AsColor(palette.KnobBorder);

        // select or interpolate the frame & knob colors.
        if (g.HoveredId == id)
        {
            color_frame = ::AsColorInterpolated(palette.FrameOffHover, palette.FrameOnHover, t, is_animated_frame_color);
            color_knob = ::AsColorInterpolated(palette.KnobOffHover, palette.KnobOnHover, t, is_animated_knob_color);
        }
        else
        {
            color_frame = ::AsColorInterpolated(palette.FrameOff, palette.FrameOn, t, is_animated_frame_color);
            color_knob = ::AsColorInterpolated(palette.KnobOff, palette.KnobOn, t, is_animated_knob_color);
        }

        // draw the background frame
        ::DrawToggleFrame(draw_list, toggle_bb, config, color_frame, AsColor(palette.FrameBorder));

        // draw accessibility labels, if enabled.
        if (show_a11y_labels)
        {
            ::DrawToggleA11yLabels(draw_list, toggle_bb, config, style.FramePadding);
        }

        // draw the knob
        if (is_circle_knob)
        {
            ::DrawToggleCircleKnob(draw_list, toggle_bb, config, radius, t, color_knob, color_knob_border);
        }
        else if (is_rectangle_knob)
        {
            ::DrawToggleRectangleKnob(draw_list, toggle_bb, config, radius, t, color_knob, color_knob_border);
        }
        else
        {
            // user didn't specify a knob mode, they get no knob.
            IM_ASSERT_USER_ERROR(false, "No toggle knob type to draw.");
        }
    }

    void DrawToggleLabel(const ImRect& toggle_bb, const char* label, bool* v, bool is_mixed_value, float x_offset)
    {
        const ImGuiContext& g = *GImGui;
        const ImGuiWindow* window = ImGui::GetCurrentWindow();
        const ImGuiStyle& style = ImGui::GetStyle();
        const ImVec2 label_size = ImGui::CalcTextSize(label, nullptr, true);

        const float half_height = toggle_bb.GetHeight() * 0.5f;
        const float label_x = toggle_bb.Max.x + style.ItemInnerSpacing.x + x_offset;
        const float label_y = toggle_bb.Min.y + half_height - (label_size.y * 0.5f);
        const ImVec2 label_pos = ImVec2(label_x, label_y);

        if (g.LogEnabled)
        {
            ImGui::LogRenderedText(&label_pos, is_mixed_value ? "[~]" : *v ? "[x]" : "[ ]");
        }

        if (label_size.x > 0.0f)
        {
            ImGui::RenderText(label_pos, label);
        }
    }


    const ImGuiTogglePalette* GetPalette(const ImGuiTogglePalette* candidate, const ImGuiStyle& style)
    {
        static ImGuiTogglePalette internal_palette;

        // populate the default palette with theme/style colors.
        const ImVec4* colors = style.Colors;

        internal_palette.KnobOff = colors[ImGuiCol_Text];
        internal_palette.KnobOffHover = colors[ImGuiCol_Text];
        internal_palette.KnobOn = colors[ImGuiCol_Text];
        internal_palette.KnobOnHover = colors[ImGuiCol_Text];
        internal_palette.FrameOff = colors[ImGuiCol_FrameBg];
        internal_palette.FrameOffHover = colors[ImGuiCol_FrameBgHovered];
        internal_palette.FrameOn = colors[ImGuiCol_Button];
        internal_palette.FrameOnHover = colors[ImGuiCol_ButtonHovered];
        internal_palette.FrameBorder = colors[ImGuiCol_Border];
        internal_palette.KnobBorder = colors[ImGuiCol_Border];

        // if the user didn't provide a candidate, just provide the theme colored palette.
        if (candidate == nullptr)
        {
            return &internal_palette;
        }

        // if the user did provide a candidate, populate all non-zero colors
#define GET_PALETTE_POPULATE_NONZERO(member) \
    do { \
        if (IsNonZero(candidate-> ## member)) \
        { \
            internal_palette. ## member = candidate-> ## member; \
        } \
    } while (0)

        GET_PALETTE_POPULATE_NONZERO(KnobOff);
        GET_PALETTE_POPULATE_NONZERO(KnobOffHover);
        GET_PALETTE_POPULATE_NONZERO(KnobOn);
        GET_PALETTE_POPULATE_NONZERO(KnobOnHover);
        GET_PALETTE_POPULATE_NONZERO(FrameOff);
        GET_PALETTE_POPULATE_NONZERO(FrameOffHover);
        GET_PALETTE_POPULATE_NONZERO(FrameOn);
        GET_PALETTE_POPULATE_NONZERO(FrameOnHover);
        GET_PALETTE_POPULATE_NONZERO(FrameBorder);
        GET_PALETTE_POPULATE_NONZERO(KnobBorder);

#undef GET_PALETTE_POPULATE_NONZERO

        return &internal_palette;
    }
} // namespace
