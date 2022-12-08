#include "imgui_toggle_presets.h"
#include "imgui_toggle_palette.h"


ImGuiToggleConfig ImGuiTogglePresets::DefaultStyle()
{
    return ImGuiToggleConfig();
}

ImGuiToggleConfig ImGuiTogglePresets::RectangleStyle()
{
    ImGuiToggleConfig config;
    config.Flags |= ImGuiToggleFlags_Animated;
    config.FrameRounding = 0.1f;
    config.KnobRounding = 0.3f;
    config.AnimationDuration = 0.5f;

    return config;
}

ImGuiToggleConfig ImGuiTogglePresets::iOSStyle(const float size_scale /*= 1.0f*/, bool light_mode /*= false*/)
{
    const ImVec4 frame_on(0.3f, 0.85f, 0.39f, 1.0f);
    const ImVec4 frame_on_hover(0.0f, 1.0f, 0.57f, 1.0f);
    const ImVec4 white(1.0f, 1.0f, 1.0f, 1.0f);
    const ImVec4 dark_mode_frame_off(0.22f, 0.22f, 0.24f, 1.0f);
    const ImVec4 light_mode_frame_off(0.91f, 0.91f, 0.92f, 1.0f);
    const ImVec4 dark_mode_frame_off_hover(0.4f, 0.4f, 0.4f, 1.0f);
    const ImVec4 light_mode_frame_off_hover(0.7f, 0.7f, 0.7f, 1.0f);
    const ImVec4 light_gray(0.89f, 0.89f, 0.89f, 1.0f);
    const ImVec4 a11y_glyph_on(1.0f, 1.0f, 1.0f, 1.0f);
    const ImVec4 a11y_glyph_off(0.4f, 0.4f, 0.4f, 1.0f);

    const float ios_width = 153 * size_scale;
    const float ios_height = 93 * size_scale;
    const float ios_frame_border_thickness = 0.0f * size_scale;
    const float ios_border_thickness = 0.0f * size_scale;
    const float ios_offset = 0.0f * size_scale;
    const float ios_inset = 6.0f * size_scale;

    // setup 'on' colors
    static ImGuiTogglePalette ios_palette_on;
    ios_palette_on.Knob = white;
    ios_palette_on.Frame = frame_on;
    ios_palette_on.FrameHover = frame_on_hover;
    ios_palette_on.KnobBorder = light_gray;
    ios_palette_on.FrameBorder = light_gray;
    ios_palette_on.A11yGlyph = a11y_glyph_on;

    // setup 'off' colors
    static ImGuiTogglePalette ios_palette_off;
    ios_palette_off.Knob = white;
    ios_palette_off.Frame = light_mode ? light_mode_frame_off : dark_mode_frame_off;
    ios_palette_off.FrameHover = light_mode ? light_mode_frame_off_hover : light_mode_frame_off_hover;
    ios_palette_off.KnobBorder = light_gray;
    ios_palette_off.FrameBorder = light_gray;
    ios_palette_off.A11yGlyph = a11y_glyph_off;

    // setup config
    ImGuiToggleConfig config;
    config.Size = ImVec2(ios_width, ios_height);
    config.Flags |= ImGuiToggleFlags_A11y
        | ImGuiToggleFlags_Animated
        | (light_mode ? ImGuiToggleFlags_Bordered : 0);
    config.A11yStyle = ImGuiToggleA11yStyle_Glyph;

    // setup 'on' config
    config.On.FrameBorderThickness = 0;
    config.On.KnobBorderThickness = ios_border_thickness;
    config.On.KnobOffset = ImVec2(ios_offset, 0);
    config.On.KnobInset = ios_inset;
    config.On.Palette = &ios_palette_on;

    // setup 'off' config
    config.Off.FrameBorderThickness = ios_frame_border_thickness;
    config.Off.KnobBorderThickness = ios_border_thickness;
    config.Off.KnobOffset = ImVec2(ios_offset, 0);
    config.Off.KnobInset = ios_inset;
    config.Off.Palette = &ios_palette_off;

    return config;
}

ImGuiToggleConfig ImGuiTogglePresets::MaterialStyle(float size_scale /*= 1.0f*/)
{
    const ImVec4 purple(0.4f, 0.08f, 0.97f, 1.0f);
    const ImVec4 purple_dim(0.78f, 0.65f, 0.99f, 1.0f);
    const ImVec4 purple_hover(0.53f, 0.08f, 1.0f, 1.0f);

    const ImVec2 material_size(37 * size_scale, 16 * size_scale);
    const float material_inset = -2.5f;
    const ImVec2 material_offset(-material_inset, 0);

    static ImGuiTogglePalette material_palette_on;
    material_palette_on.Frame = purple_dim;
    material_palette_on.FrameHover = purple_dim;
    material_palette_on.Knob = purple;
    material_palette_on.KnobHover = purple_hover;

    // setup config
    ImGuiToggleConfig config;
    config.Flags |= ImGuiToggleFlags_Animated;
    config.Size = material_size;
    config.On.KnobInset = config.Off.KnobInset = material_inset;
    config.On.KnobOffset = config.Off.KnobOffset = material_offset;
    config.On.Palette = &material_palette_on;

    return config;
}


