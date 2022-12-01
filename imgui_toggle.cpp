#include "imgui_toggle.h"

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif // IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

using namespace ImGuiToggleConstants;

namespace
{
    // the actual logic takes place in ToggleInternal(), all the Toggle() calls are just interfaces for it.
    bool ToggleInternal(const char* label, bool* v, const ImGuiToggleConfig& config);
    const ImGuiTogglePalette* GetPalette(const ImGuiTogglePalette* candidate, const ImGuiStyle& style);

    // inline helpers

    // sets the given config structure's values to the
    // default ones used by the `Toggle()` overloads.
    inline void SetToAliasDefaults(ImGuiToggleConfig& config)
    {
        config.Flags = ImGuiToggleFlags_Default;
        config.AnimationSpeed = AnimationSpeedDisabled;
        config.FrameRounding = FrameRoundingDefault;
        config.KnobRounding = KnobRoundingDefault;
    }

    // shorthand for `ImGui::GetColorU32()`.
    inline ImU32 AsColor(const ImVec4& v)
    {
        return ImGui::GetColorU32(v);
    }

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
    ::_internalConfig.Size = size;
    

    return ::ToggleInternal(label, v, ::_internalConfig);
}

bool ImGui::Toggle(const char* label, bool* v, ImGuiToggleFlags flags, float speed, const ImVec2& size /*= ImVec2()*/)
{
    // this overload implies the toggle should be animated.
    if (speed > 0 && (flags & ImGuiToggleFlags_AnimatedKnob) != 0)
    {
        // if the user didn't specify ImGuiToggleFlags_AnimatedKnob, enable all animations.
        flags = flags | (ImGuiToggleFlags_Animated);
    }

    ::SetToAliasDefaults(::_internalConfig);
    ::_internalConfig.Flags = flags;
    ::_internalConfig.AnimationSpeed = speed;
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

bool ImGui::Toggle(const char* label, bool* v, ImGuiToggleFlags flags, float speed, float frame_rounding, float knob_rounding, const ImVec2& size /*= ImVec2()*/)
{
    // this overload implies the toggle should be animated.
    if (speed > 0 && (flags & ImGuiToggleFlags_AnimatedKnob) != 0)
    {
        // if the user didn't specify ImGuiToggleFlags_AnimatedKnob, enable all animations.
        flags = flags | (ImGuiToggleFlags_Animated);
    }

    ::_internalConfig.Flags = flags;
    ::_internalConfig.AnimationSpeed = speed;
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
    bool ToggleInternal(const char* label, bool* v, const ImGuiToggleConfig& config)
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
        ImGuiStyle& style = ImGui::GetStyle();
        const ImVec4* colors = style.Colors;
        const ImVec2 pos = window->DC.CursorPos;
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        const ImGuiID id = window->GetID(label);
        const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
        const ImGuiTogglePalette& palette = *GetPalette(cfg.Palette, style);

        // modes
        const bool is_circle_knob = cfg.KnobRounding >= 1.0f;
        const bool is_rectangle_knob = cfg.KnobRounding < 1.0f;
        const bool is_animated = (cfg.Flags & ImGuiToggleFlags_AnimatedKnob) != 0;
        const bool is_animated_frame_color = is_animated && (cfg.Flags & ImGuiToggleFlags_AnimatedFrameColor) != 0;
        const bool is_animated_knob_color = is_animated && (cfg.Flags & ImGuiToggleFlags_AnimatedKnobColor) != 0;
        const bool has_bordered_frame = (cfg.Flags & ImGuiToggleFlags_BorderedFrame) != 0;
        const bool has_bordered_knob = (cfg.Flags & ImGuiToggleFlags_BorderedKnob) != 0;
        const bool show_a11y_labels = (cfg.Flags & ImGuiToggleFlags_A11yLabels) != 0;

        // users likely don't intend to double up on modes.
        IM_ASSERT(!is_circle_knob || !is_rectangle_knob);

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

        const float one_minus_t = 1.0f - t;

        ImU32 color_frame;
        ImU32 color_knob;

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

        const ImVec2 frame_min = pos;
        const ImVec2 frame_max = ImVec2(pos.x + width, pos.y + height);

        ImVec2 knob_offset = cfg.KnobOffset;

        // draw frame background
        draw_list->AddRectFilled(frame_min, frame_max, color_frame, background_rounding);

        // draw frame border, if enabled
        if (has_bordered_frame)
        {
            draw_list->AddRect(frame_min, frame_max, AsColor(palette.FrameBorder), background_rounding, ImDrawFlags_None, cfg.FrameBorderThickness);
        }

        // draw accessibility labels, if enabled.
        if (show_a11y_labels)
        {
            ImVec2 on_pos = toggle_bb.GetTL();
            ImVec2 off_pos = toggle_bb.GetTR();
            const ImVec2 off_text_size = ImGui::CalcTextSize(cfg.OffLabel);

            // on text should be just bumped down and left into the frame.
            on_pos += style.FramePadding;

            // off text should be bumped down and right by padding, and right by it's length.
            off_pos.x -= (style.FramePadding.x + off_text_size.x);
            off_pos.y += style.FramePadding.y;

            // draw the labels.
            ImGui::RenderText(on_pos, cfg.OnLabel);
            ImGui::RenderText(off_pos, cfg.OffLabel);
        }

        if (is_circle_knob)
        {
            const float half_knob_x_offset = cfg.KnobOffset.x * 0.5f;
            const float knob_x = (pos.x + radius)
                + t * (width - double_radius - half_knob_x_offset)
                + (one_minus_t * half_knob_x_offset);
            const float knob_y = pos.y + radius + cfg.KnobOffset.y;
            const ImVec2 knob_center = ImVec2(knob_x, knob_y);
            const float knob_radius = radius - cfg.KnobInset;

            // draw circle knob
            draw_list->AddCircleFilled(knob_center, knob_radius, color_knob);

            // draw knob border, if enabled
            if (has_bordered_knob)
            {
                draw_list->AddCircle(knob_center, knob_radius, AsColor(palette.KnobBorder), 0, cfg.KnobBorderThickness);
            }
        }
        else if (is_rectangle_knob)
        {
            const float half_knob_x_offset = cfg.KnobOffset.x * 0.5f;
            const float knob_inset_double = cfg.KnobInset * 2.0f;
            const float knob_left = (t * (width - double_radius - half_knob_x_offset))
                + (one_minus_t * half_knob_x_offset)
                + cfg.KnobInset;
            const float knob_top = cfg.KnobInset + cfg.KnobOffset.y;
            const float knob_bottom = height - cfg.KnobInset + cfg.KnobOffset.y;
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

                draw_list->AddRect(knob_min, knob_max, AsColor(palette.KnobBorder), knob_rounded_radius, 0, cfg.KnobBorderThickness);
            }
        }
        else
        {
            // user didn't specify a knob mode, they get no knob.
            IM_ASSERT(false && "No toggle knob type to draw.");
        }

        const float label_x = toggle_bb.Max.x + style.ItemInnerSpacing.x;
        //const float label_y = toggle_bb.Min.y + style.FramePadding.y; // top-left align text
        const float label_y = toggle_bb.Min.y + half_height - (label_size.y * 0.5f); // middle-left align text
        const ImVec2 label_pos = ImVec2(label_x, label_y);

        if (g.LogEnabled)
        {
            ImGui::LogRenderedText(&label_pos, mixed_value ? "[~]" : *v ? "[x]" : "[ ]");
        }

        if (label_size.x > 0.0f)
        {
            ImGui::RenderText(label_pos, label);
            //ImGui::SameLine();
            //ImGui::AlignTextToFramePadding();
            //ImGui::Text(label);
        }

        IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags | ImGuiItemStatusFlags_Checkable | (*v ? ImGuiItemStatusFlags_Checked : 0));
        return pressed;
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
