# 🔘 imgui_toggle

A small Dear ImGui widget that implements a modern toggle style switch.

## Overview

Based on the discussion in [https://github.com/ocornut/imgui/issues/1537](https://github.com/ocornut/imgui/issues/1537), and on the implementation of `ImGui::Checkbox()`,
this small widget collection implements a customizable "Toggle" style button for Dear ImGui. A toggle represents a boolean on/off much like a checkbox, but is better suited
to some particular paradigms depending on the UI designer's goals. It can often more clearly indicate an enabled/disabled state.

`imgui_toggle` also supports an optional small animation, similar to that seen in mobile OS and web applications, which can further aid in user feedback.

Internally, `imgui_toggle` functions very similarly to `ImGui::Checkbox()`, with the exception that it activates on mouse down rather than the release. It supports drawing
a label in the same way, and toggling the value by clicking that associated label. The label can be hidden [as on other controls](https://github.com/ocornut/imgui/blob/master/docs/FAQ.md#q-how-can-i-have-widgets-with-an-empty-label).

## Preview

![`imgui_toggle` example animated gif](./.meta/imgui_toggle_example.gif)

_An example of `imgui_toggle`, produced by the usage code below, as an animated gif._

## Usage

Add `imgui_toggle.cpp` and `imgui_toggle.h` to your project, and include `imgui_toggle.h` in the source file you wish to use toggles.

See `imgui_toggle.h` for the API, or below for an example.

```cpp
static bool toggle_a = true;
static bool toggle_b = true;
static bool toggle_c = true;
static bool toggle_d = true;
static bool toggle_e = true;
static bool toggle_f = true;
static bool toggle_custom = true;

ImVec4 green(0.16f, 0.66f, 0.45f, 1.0f);
ImVec4 green_hover(0.0f, 1.0f, 0.57f, 1.0f);

ImVec4 purple(0.4f, 0.08f, 0.97f, 1.0f);
ImVec4 purple_dim(0.78f, 0.65f, 0.99f, 1.0f);
ImVec4 purple_hover(0.53f, 0.08f, 1.0f, 1.0f);

// use some lovely gray backgrounds for "off" toggles
// the default will use your theme's frame background colors.
ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.45f, 0.45f, 0.45f, 1.0f));
ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.65f, 0.65f, 0.65f, 1.0f));

// a default and default animated toggle
ImGui::Toggle("Default Toggle", &toggle_a);
ImGui::Toggle("Animated Toggle", &toggle_b, ImGuiToggleFlags_Animated);

// this toggle draws a simple border around it's frame and knob
ImGui::Toggle("Bordered Knob", &toggle_c, ImGuiToggleFlags_Bordered, 1.0f);

// this toggle uses stack-pushed style colors to change the way it displays
ImGui::PushStyleColor(ImGuiCol_Button, green);
ImGui::PushStyleColor(ImGuiCol_ButtonHovered, green_hover);
ImGui::Toggle("Green Toggle", &toggle_d);
ImGui::PopStyleColor(2);

ImGui::Toggle("Toggle with A11y Labels", &toggle_e, ImGuiToggleFlags_A11yLabels);

// this toggle shows no label
ImGui::Toggle("##Toggle With Hidden Label", &toggle_f);

//////////////////////////////////////////////////////////////////////////
// Custom Toggle
ImGui::Separator();
ImGui::Text("Custom Toggle");

static ImGuiToggleConfig config;
ImGui::Toggle("Customized Toggle", &toggle_custom, config);
ImGui::NewLine();

// some values to use for slider limits
const float max_height = config.Size.y > 0 ? config.Size.y : ImGui::GetFrameHeight();
const float half_max_height = max_height * 0.5f;

// animation duration controls how long the toggle animates, in seconds. if set to 0, animation is disabled.
if (ImGui::SliderFloat("Animation Duration (seconds)", &config.AnimationDuration, ImGuiToggleConstants::AnimationDurationMinimum, 2.0f))
{
    // if the user adjusted the animation duration slider, go ahead and turn on the animation flags.
    config.Flags |= ImGuiToggleFlags_Animated;
}

// frame rounding sets how round the frame is when drawn, where 0 is a rectangle, and 1 is a circle.
ImGui::SliderFloat("Frame Rounding (scale)", &config.FrameRounding, ImGuiToggleConstants::FrameRoundingMinimum, ImGuiToggleConstants::FrameRoundingMaximum);

// knob rounding sets how round the knob is when drawn, where 0 is a rectangle, and 1 is a circle.
ImGui::SliderFloat("Knob Rounding (scale)", &config.KnobRounding, ImGuiToggleConstants::KnobRoundingMinimum, ImGuiToggleConstants::KnobRoundingMaximum);

// size controls the width and the height of the toggle frame
ImGui::SliderFloat2("Size (px)", &config.Size.x, 0.0f, 100.0f, "%.0f");

// knob offset controls how far into or out of the frame the knob should draw.
ImGui::SliderFloat2("Knob Offset (px)", &config.KnobOffset.x, -half_max_height, half_max_height);

// width ratio sets how wide the toggle is with relation to the frame height. if Size is non-zero, this is unused.
ImGui::SliderFloat("Width Ratio (scale)", &config.WidthRatio, ImGuiToggleConstants::WidthRatioMinimum, ImGuiToggleConstants::WidthRatioMaximum);

// knob inset controls how many pixels the knob is set into the frame. negative values will cause it to grow outside the frame.
ImGui::SliderFloat("Knob Inset (px)", &config.KnobInset, ImGuiToggleConstants::KnobInsetMinimum, half_max_height);

// how thick should the frame border be (if enabled)
ImGui::SliderFloat("Frame Border Thickness (px)", &config.FrameBorderThickness, 0.0f, 3.0f);

// how thick should the knob border be (if enabled)
ImGui::SliderFloat("Knob Border Thickness (px)", &config.KnobBorderThickness, 0.0f, 3.0f);

// flags for various toggle features
ImGui::Columns(2);
ImGui::Text("Meta Flags");
ImGui::CheckboxFlags("Bordered", &config.Flags, ImGuiToggleFlags_Bordered);
ImGui::CheckboxFlags("Animated", &config.Flags, ImGuiToggleFlags_Animated);
ImGui::NextColumn();
ImGui::CheckboxFlags("AnimatedKnob", &config.Flags, ImGuiToggleFlags_AnimatedKnob);
ImGui::CheckboxFlags("AnimatedFrameColor", &config.Flags, ImGuiToggleFlags_AnimatedFrameColor);
ImGui::CheckboxFlags("AnimatedKnobColor", &config.Flags, ImGuiToggleFlags_AnimatedKnobColor);
ImGui::CheckboxFlags("BorderedFrame", &config.Flags, ImGuiToggleFlags_BorderedFrame);
ImGui::CheckboxFlags("BorderedKnob", &config.Flags, ImGuiToggleFlags_BorderedKnob);
ImGui::CheckboxFlags("A11yLabels", &config.Flags, ImGuiToggleFlags_A11yLabels);
ImGui::Columns();

if (ImGui::Button("Reset Config"))
{
    config = ImGuiToggleConfig();
}
ImGui::SameLine();

if (ImGui::Button("Rectangle Style"))
{
    // setup config
    config = ImGuiToggleConfig();
    config.Flags |= ImGuiToggleFlags_Animated;
    config.FrameRounding = 0.3f;
    config.KnobRounding = 0.3f;
    config.AnimationDuration = 1.5f;
}
ImGui::SameLine();

if (ImGui::Button("iOS Style"))
{
    // setup colors
    static ImGuiTogglePalette ios_palette;
    ios_palette.FrameOn = green;
    ios_palette.FrameOnHover = green_hover;

    // setup config
    config = ImGuiToggleConfig();
    config.Flags |= ImGuiToggleFlags_A11yLabels | ImGuiToggleFlags_Animated;
    config.Palette = &ios_palette;
}
ImGui::SameLine();

if (ImGui::Button("Material Style"))
{
    // setup colors
    static ImGuiTogglePalette material_palette;
    material_palette.FrameOn = purple_dim;
    material_palette.FrameOnHover = purple_dim;
    material_palette.KnobOn = purple;
    material_palette.KnobOnHover = purple_hover;

    // setup config
    config = ImGuiToggleConfig();
    config.Flags |= ImGuiToggleFlags_Animated;
    config.Size = ImVec2(37, 16);
    config.KnobInset = -2.5f;
    config.KnobOffset = ImVec2(-2.0f, 0);
    config.Palette = &material_palette;
}

// End Custom Toggle
//////////////////////////////////////////////////////////////////////////

// pop the FrameBg/FrameBgHover color styles
ImGui::PopStyleColor(2);
```

## Styling

While `imgui_toggle` maintains a simple API for quick and easy toggles, a more complex one exists to allow the user to better customize the widget.

By using the overload that takes a `const ImGuiToggleConfig&`, a structure can be provided that provides a multitude of configuration parameters.

Notably this also allows providing a pointer to a `ImGuiTogglePalette` structure, which allows changing all the colors used to draw the widget. However, this method of configuration is not strictly necessary, as `imgui_toggle` will follow your theme colors as defined below if no palette or color replacement is specified.

### Theme Colors

Since `imgui_toggle` isn't part of Dear ImGui proper, it doesn't have any direct references in `ImGuiCol_` for styling. `imgui_toggle` in addition to the method described above, you can use `ImGui::PushStyleColor()` and `ImGui::PopStyleColor()` to adjust the following theme colors around your call to `ImGui::Toggle()`:

- `ImGuiCol_Text`: Will be used as the color of the knob portion of the toggle.
- `ImGuiCol_Button`: Will be used as the frame color of the toggle when it is in the "on" position, and the widget is not hovered.
- `ImGuiCol_ButtonHovered`: Will be used as the frame color of the toggle when it is in the "on" position, and the widget is hovered over.
- `ImGuiCol_FrameBg`: Will be used as the frame color of the toggle when it is in the "off" position, and the widget is not hovered.
- `ImGuiCol_FrameBgHovered`: Will be used as the frame color of the toggle when it is in the "off" position, and the widget is hovered over.
- `ImGuiCol_Border`: Will be used as the color drawn as the border on the frame and knob if the related flags are passed.

Unfortunately, the dark gray and light gray used while the toggle is in the "off" position are currently defined by the widget code itself and not by any theme color.

## Future Considerations

As brought up by [ocornut](https://github.com/ocornut/imgui/issues/1537#issuecomment-355562097), if `imgui_toggle` were to be part of mainline Dear ImGui in the future,
there are some questions that should likely be answered. Most notably for me are the following:

- Should the toggle get it's own enums in the style system?
  - If so, should which colors should it define, and which would it be okay sharing?
  - If I were choosing, I feel the button and hovered styles as the "on" coloring are acceptable, and perhaps adding three more shared styles `ImGuiCol_Knob`, `ImGuiCol_FrameBgOff`, and `ImGuiCol_FrameBgOffHover` for use as the foreground knob color, and the "off" background states. (An `Active` may be needed too if switching to operate on input release instead of press.)
- Is the rendering quality good enough?
- Is the dependence on the `LastActiveIdTimer` acceptable for animation, and the user must accept that clicking quickly would skip previous animations?
- Should the toggle behave *exactly* like `ImGui::Checkbox()`, toggling on release rather than press?

----

## 📝 License

`imgui_toggle` is [licensed](./LICENSE) under the Zero-Clause BSD License (SPDX-License-Identifier: 0BSD). If you're interested in `imgui_toggle` under other terms, please contact the author.

Copyright © 2022 [Chris Marc Dailey](https://cmd.wtf)

Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

## Acknowledgements

`imgui_toggle` drew inspiration from [ocornut's original share](https://github.com/ocornut/imgui/issues/1537#issuecomment-355562097),
his [animated variant](https://github.com/ocornut/imgui/issues/1537#issuecomment-355569554), [nerdtronik's shift to theme colors](https://github.com/ocornut/imgui/issues/1537#issuecomment-780262461),
and [ashifolfi's squircle](https://github.com/ocornut/imgui/issues/1537#issuecomment-1272612641) concept. Inspiration for border drawing came from [moebiussurfing](https://github.com/cmdwtf/imgui_toggle/issues/1#issue-1441329209).

As well, inspiration was derived from [Dear ImGui's current `Checkbox` implementation](https://github.com/ocornut/imgui/blob/529cba19b09cf6db206de2b9eaa3152ecb2feff8/imgui_widgets.cpp#L1102),
for the behavior, label drawing, and hopefully preparing to [handle mixed values/indeterminate states](https://github.com/ocornut/imgui/issues/2644) (albeit unsupported as of yet).
