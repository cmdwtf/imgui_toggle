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

See `imgui_toggle.h` for the API, or below for a brief example.

```cpp
static bool toggle_a = true;
static bool toggle_b = true;
static bool toggle_c = true;
static bool toggle_d = true;
static bool toggle_e = true;
static bool toggle_f = true;

static float frame_rounding = 0.2f;
static float knob_rounding = 0.2f;

// a default and default animated toggle
ImGui::Toggle("Default Toggle", &toggle_a);
ImGui::Toggle("Animated Toggle", &toggle_b, ImGuiToggleFlags_Animated);

// sliders for adjusting the rounding for the next two toggles.
ImGui::SliderFloat("frame_rounding", &frame_rounding, 0.0f, 1.0f);
ImGui::SliderFloat("knob_rounding", &knob_rounding, 0.0f, 1.0f);

// a default and animated toggle that can have their frames and knobs rounded
// a rounding of 0 is completely square, a rounding of 1 is fully rounded.
ImGui::Toggle("Square Toggle", &toggle_c, ImGuiToggleFlags_Default, frame_rounding, knob_rounding);
ImGui::Toggle("Animated Square Toggle", &toggle_d, ImGuiToggleFlags_Animated, frame_rounding, knob_rounding);

// this toggle uses stack-pushed style colors to change the way it displays
ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.16f, 0.66f, 0.45f, 1.0f));
ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 1.0f, 0.57f, 1.0f));
ImGui::Toggle("Green Toggle", &toggle_e);
ImGui::PopStyleColor(2);

// this toggle shows no label
ImGui::Toggle("##Toggle With Hidden Label", &toggle_f);
```

## Limitations

Since `imgui_toggle` isn't part of Dear ImGui proper, it doesn't have any direct references for styling. `imgui_toggle` makes use of a
few shared theme colors for styling. For now, it is easiest to use `ImGui::PushStyleColor()` and `ImGui::PopStyleColor()` to
adjust those these colors around your call to `ImGui::Toggle()`:

- `ImGuiCol_Text`: Will be used as the color of the knob portion of the toggle.
- `ImGuiCol_Button`: Will be used as the background color of the toggle when it is in the "on" position, and the widget is not hovered.
- `ImGuiCol_ButtonHovered`: Will be used as the background color of the toggle when it is in the "on" position, and the widget is hovered over.

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

Copyright © 2022 [Chris March Dailey](https://cmd.wtf)

Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

## Acknowledgements

`imgui_toggle` drew inspiration from [ocornut's original share](https://github.com/ocornut/imgui/issues/1537#issuecomment-355562097),
his [animated variant](https://github.com/ocornut/imgui/issues/1537#issuecomment-355569554), [nerdtronik's shift to theme colors](https://github.com/ocornut/imgui/issues/1537#issuecomment-780262461),
and [ashifolfi's squircle](https://github.com/ocornut/imgui/issues/1537#issuecomment-1272612641) concept.

As well, inspiration was derived from [Dear ImGui's current `Checkbox` implementation,](https://github.com/ocornut/imgui/blob/529cba19b09cf6db206de2b9eaa3152ecb2feff8/imgui_widgets.cpp#L1102),
for the behavior, label drawing, and hopefully preparing to [handle mixed values/indeterminate states](https://github.com/ocornut/imgui/issues/2644) (albeit unsupported as of yet).
