//
// Created by Александр Лебедев on 21.11.2023.
//

#pragma once
#define IM_VEC4_CLASS_EXTRA constexpr ImVec4 operator+(const ImVec4& rhs) const { return ImVec4(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w); } \
                            constexpr ImVec4 operator-(const ImVec4& rhs) const { return ImVec4(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w); }
#define IM_VEC2_CLASS_EXTRA constexpr ImVec2 operator+(const ImVec2& rhs) const { return ImVec2(x + rhs.x, y + rhs.y); } \
                            constexpr ImVec2 operator-(const ImVec2& rhs) const { return ImVec2(x - rhs.x, y - rhs.y); }
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include "imgui_internal.h"
#include <map>
namespace ImGui
{
    namespace Internal
    {
        static const char* PatchFormatStringFloatToInt(const char* fmt)
        {
            if (fmt[0] == '%' && fmt[1] == '.' && fmt[2] == '0' && fmt[3] == 'f' && fmt[4] == 0) // Fast legacy path for "%.0f" which is expected to be the most common case.
                return "%d";
            const char* fmt_start = ImParseFormatFindStart(fmt);    // Find % (if any, and ignore %%)
            const char* fmt_end = ImParseFormatFindEnd(fmt_start);  // Find end of format specifier, which itself is an exercise of confidence/recklessness (because snprintf is dependent on libc or user).
            if (fmt_end > fmt_start && fmt_end[-1] == 'f')
            {
#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
                if (fmt_start == fmt && fmt_end[0] == 0)
                    return "%d";
                ImGuiContext& g = *GImGui;
                ImFormatString(g.TempBuffer.begin(), g.TempBuffer.size(), "%.*s%%d%s", (int)(fmt_start - fmt), fmt, fmt_end); // Honor leading and trailing decorations, but lose alignment/precision.
                return g.TempBuffer.begin();
#else
                IM_ASSERT(0 && "DragInt(): Invalid format string!"); // Old versions used a default parameter of "%.0f", please replace with e.g. "%d"
#endif
            }
            return fmt;
        }
    }


    inline bool SpotifySliderScalar(const char* label, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format, ImGuiSliderFlags flags) {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems)
            return false;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(label);
        const float w = 331;
        const float f = 30;

        const ImVec2 label_size = CalcTextSize(label, NULL, true);
        const ImRect frame_bb(window->DC.CursorPos + ImVec2(-20, 9.5f) + ImVec2(82, 0), window->DC.CursorPos - ImVec2(42, -4.5f) + ImVec2(w, label_size.y + style.FramePadding.y * 2.0f));
        const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0.0f));
        const ImRect frame_bbb(window->DC.CursorPos + ImVec2(-20, -8), window->DC.CursorPos - ImVec2(-42, 9.0f) + ImVec2(w, f + style.FramePadding.y * 2.0f));


        ItemSize(total_bb, style.FramePadding.y);
        if (!ItemAdd(total_bb, id, &frame_bb))
            return false;


        if (format == NULL)
            format = DataTypeGetInfo(data_type)->PrintFmt;
        else if (data_type == ImGuiDataType_S32 && strcmp(format, "%d") != 0)
            format = Internal::PatchFormatStringFloatToInt(format);



        const bool hovered = ItemHoverable(frame_bb, id, 0);                               // Размер ховера на слайдере
        const bool temp_input_allowed = (flags & ImGuiSliderFlags_NoInput) == 0;        // Если в меню не стоит флаг NoInput, разрешаем ввод через ctrl
        bool temp_input_is_active = temp_input_allowed && TempInputIsActive(id);        // Ввод числа вручную активен

        //if (hovered)
        //    g.MouseCursor = ImGuiMouseCursor_ResizeEW;
        //if (hovered && g.IO.KeyCtrl)                          //  Раскоментите что-бы была смена курсора (в спотике её нет)
        //    g.MouseCursor = ImGuiMouseCursor_Hand;

        if (!temp_input_is_active)
        {
            const bool focus_requested = temp_input_allowed && FocusableItemRegister(window, id);
            const bool clicked = (hovered && g.IO.MouseClicked[0]);
            if (focus_requested || clicked || g.NavActivateId == id || g.TempInputId == id)
            {
                SetActiveID(id, window);
                SetFocusID(id, window);
                FocusWindow(window);
                g.ActiveIdUsingNavDirMask |= (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
                if (temp_input_allowed && (focus_requested || (clicked && g.IO.KeyCtrl) || g.TempInputId == id))
                {
                    temp_input_is_active = true;
                    FocusableItemUnregister(window);
                }
            }
        }

        if (temp_input_is_active)
        {
            PushStyleColor(ImGuiCol_Text, ColorConvertU32ToFloat4(ImColor(255, 255, 255, 255)));
            const bool is_clamp_input = (flags & ImGuiSliderFlags_AlwaysClamp) != 0;
            return TempInputScalar(frame_bbb, id, label, data_type, p_data, format, is_clamp_input ? p_min : NULL, is_clamp_input ? p_max : NULL);
            PopStyleColor();
        }


        const ImU32 frame_col = GetColorU32(g.ActiveId == id ? ImGuiCol_FrameBgActive : g.HoveredId == id ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
        RenderNavHighlight(frame_bb, id);
        RenderFrame(frame_bb.Min, frame_bb.Max - ImVec2(0, 1.8), ImColor(94, 94, 94, 255), true, 10);       // Фрейм незаполненного слайдера (более серый цвет).


        ImRect grab_bar;
        const bool item_value_changed = SliderBehavior(frame_bb, id, data_type, p_data, p_min, p_max, format, flags, &grab_bar);
        if (item_value_changed)
            MarkItemEdited(id);

        if (!hovered)
            RenderFrame(ImVec2(frame_bb.Min.x, frame_bb.Min.y), ImVec2(grab_bar.Max.x + 1, frame_bb.Max.y - 2.8), ImColor(255, 255, 255, 255), true, 10);    // Фрейм белого цвета (ин-актив состояние)




        RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x - 231, frame_bb.Min.y + style.FramePadding.y - 20), label);     // Название слайдера





        if (hovered || IsItemActive())      // Элементы в этом контейнере работают только при ховере и/или актив состоянии
        {
            RenderFrame(ImVec2(frame_bb.Min.x, frame_bb.Min.y), ImVec2(grab_bar.Max.x, frame_bb.Max.y - 2.8), ImColor(29, 185, 84, 255), true, 10);    // Фрейм зелёного цвета (заполненный)
            window->DrawList->AddCircleFilled(ImVec2(grab_bar.Min.x + 7, frame_bb.Min.y + 2), 5.5, ImColor(255, 255, 255, 255), 512);     // Кружочек :)
        }




        IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.ItemFlags);
        return item_value_changed;
    }



    inline bool SpotifySliderF(const char* label, float* v, float v_min, float v_max, const char* format, ImGuiSliderFlags flags)
    {
        return SpotifySliderScalar(label, ImGuiDataType_Float, v, &v_min, &v_max, format, flags);
    }





    inline bool DiscordSliderScalar(const char* label, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format, ImGuiSliderFlags flags) {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems)
            return false;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(label);
        const float w = 331;
        const float f = 30;

        const ImVec2 label_size = CalcTextSize(label, NULL, true);
        const ImRect frame_bb(window->DC.CursorPos + ImVec2(-20, 9.5f) + ImVec2(82, 0), window->DC.CursorPos - ImVec2(42, -4.5f) + ImVec2(w, label_size.y + style.FramePadding.y * 2.0f));
        const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0.0f));
        const ImRect frame_bbb(window->DC.CursorPos + ImVec2(-20, -8), window->DC.CursorPos - ImVec2(-42, 9.0f) + ImVec2(w, f + style.FramePadding.y * 2.0f));


        ItemSize(total_bb, style.FramePadding.y);
        if (!ItemAdd(total_bb, id, &frame_bb))
            return false;


        if (format == NULL)
            format = DataTypeGetInfo(data_type)->PrintFmt;
        else if (data_type == ImGuiDataType_S32 && strcmp(format, "%d") != 0)
            format = Internal::PatchFormatStringFloatToInt(format);



        static std::map<ImGuiID, float> animate;
        auto valuealpha = animate.find(id);
        if (valuealpha == animate.end())
        {
            animate.insert({ id, 0.0f });
            valuealpha = animate.find(id);
        }



        const bool hovered = ItemHoverable(frame_bb, id, 0);                               // Размер ховера на слайдере
        const bool temp_input_allowed = (flags & ImGuiSliderFlags_NoInput) == 0;        // Если в меню не стоит флаг NoInput, разрешаем ввод через ctrl
        bool temp_input_is_active = temp_input_allowed && TempInputIsActive(id);        // Ввод числа вручную активен

        //if (hovered)
        //    g.MouseCursor = ImGuiMouseCursor_ResizeEW;
        //if (hovered && g.IO.KeyCtrl)                          //  Раскоментите что-бы была смена курсора (как в дискорде)
        //    g.MouseCursor = ImGuiMouseCursor_Hand;

        if (!temp_input_is_active)
        {
            const bool focus_requested = temp_input_allowed && FocusableItemRegister(window, id);
            const bool clicked = (hovered && g.IO.MouseClicked[0]);
            if (focus_requested || clicked || g.NavActivateId == id || g.TempInputId == id)
            {
                SetActiveID(id, window);
                SetFocusID(id, window);
                FocusWindow(window);
                g.ActiveIdUsingNavDirMask |= (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
                if (temp_input_allowed && (focus_requested || (clicked && g.IO.KeyCtrl) || g.TempInputId == id))
                {
                    temp_input_is_active = true;
                    FocusableItemUnregister(window);
                }
            }
        }

        if (temp_input_is_active)
        {
            PushStyleColor(ImGuiCol_Text, ColorConvertU32ToFloat4(ImColor(255, 255, 255, 255)));
            const bool is_clamp_input = (flags & ImGuiSliderFlags_AlwaysClamp) != 0;
            return TempInputScalar(frame_bbb, id, label, data_type, p_data, format, is_clamp_input ? p_min : NULL, is_clamp_input ? p_max : NULL);
            PopStyleColor();
        }


        const ImU32 frame_col = GetColorU32(g.ActiveId == id ? ImGuiCol_FrameBgActive : g.HoveredId == id ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
        RenderNavHighlight(frame_bb, id);
        RenderFrame(frame_bb.Min, frame_bb.Max - ImVec2(0, 1.8), ImColor(79, 84, 92, 255), true, 10);       // Фрейм незаполненного слайдера (более серый цвет).


        ImRect grab_bar;
        const bool item_value_changed = SliderBehavior(frame_bb, id, data_type, p_data, p_min, p_max, format, flags, &grab_bar);
        if (item_value_changed)
            MarkItemEdited(id);


        RenderFrame(ImVec2(frame_bb.Min.x, frame_bb.Min.y), ImVec2(grab_bar.Max.x, frame_bb.Max.y - 1.8), ImColor(88, 101, 242, 255), true, 10);    // Фрейм лилового цвета (заполненный)
        window->DrawList->AddCircleFilled(ImVec2(grab_bar.Min.x + 7, frame_bb.Min.y + 2), 6.5, ImColor(255, 255, 255, 255), 512);     // Кружочек :)




        RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x - 231, frame_bb.Min.y + style.FramePadding.y - 20), label);     // Название слайдера




        char value_buf[64];
        const char* value_buf_end = value_buf + DataTypeFormatString(value_buf, IM_ARRAYSIZE(value_buf), data_type, p_data, format);
        if (hovered || IsItemActive())
        {
            if (valuealpha->second < 255.f)
                valuealpha->second += 25.f;
            RenderFrame(ImVec2(grab_bar.Min.x - 10, frame_bb.Min.y - 37.8) /* + ImVec2(209, -30)*/, ImVec2(grab_bar.Max.x + 12, frame_bb.Max.y - 20.8), ImColor(32, 34, 37, (int)valuealpha->second), true, 5);
            RenderArrow(window->DrawList, ImVec2(grab_bar.Min.x + 1, frame_bb.Min.y - 21), ImColor(32, 34, 37, (int)valuealpha->second), ImGuiDir_Down, 1.5f);
            PushStyleColor(ImGuiCol_Text, ColorConvertFloat4ToU32(ImVec4((int)valuealpha->second, (int)valuealpha->second, (int)valuealpha->second, (int)valuealpha->second)));
            RenderTextClipped(ImVec2(grab_bar.Min.x - 10, frame_bb.Min.y - 37.8), ImVec2(grab_bar.Max.x + 12, frame_bb.Max.y - 20.8), value_buf, value_buf_end, NULL, ImVec2(0.5f, 0.5f));
            PopStyleColor();
        }       // Тут идёт появление фрейма с числом над слайдером
        else
        {
            if (valuealpha->second > 0.f)
                valuealpha->second -= 25.f;
        }


        IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.ItemFlags);
        return item_value_changed;
    }


    inline bool DiscordSliderF(const char* label, float* v, float v_min, float v_max, const char* format, ImGuiSliderFlags flags)
    {
        return DiscordSliderScalar(label, ImGuiDataType_Float, v, &v_min, &v_max, format, flags);
    }



    inline bool WindowsSliderScalar(const char* label, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format, ImGuiSliderFlags flags) {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems)
            return false;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(label);
        const float w = 331;
        const float f = 30;

        const ImVec2 label_size = CalcTextSize(label, NULL, true);
        const ImRect frame_bb(window->DC.CursorPos + ImVec2(-20, 9.5f) + ImVec2(82, 0), window->DC.CursorPos - ImVec2(42, -4.5f) + ImVec2(w, label_size.y + style.FramePadding.y * 2.0f));
        const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0.0f));
        const ImRect frame_bbb(window->DC.CursorPos + ImVec2(-20, -8), window->DC.CursorPos - ImVec2(-42, 9.0f) + ImVec2(w, f + style.FramePadding.y * 2.0f));


        ItemSize(total_bb, style.FramePadding.y);
        if (!ItemAdd(total_bb, id, &frame_bb))
            return false;


        if (format == NULL)
            format = DataTypeGetInfo(data_type)->PrintFmt;
        else if (data_type == ImGuiDataType_S32 && strcmp(format, "%d") != 0)
            format = Internal::PatchFormatStringFloatToInt(format);



        const bool hovered = ItemHoverable(frame_bb, id, 0);                               // Размер ховера на слайдере
        const bool temp_input_allowed = (flags & ImGuiSliderFlags_NoInput) == 0;        // Если в меню не стоит флаг NoInput, разрешаем ввод через ctrl
        bool temp_input_is_active = temp_input_allowed && TempInputIsActive(id);        // Ввод числа вручную активен

        //if (hovered)
        //    g.MouseCursor = ImGuiMouseCursor_ResizeEW;
        //if (hovered && g.IO.KeyCtrl)                          //  Раскоментите что-бы была смена курсора при ховере (в слайдере винды её нет)
        //    g.MouseCursor = ImGuiMouseCursor_Hand;

        if (!temp_input_is_active)
        {
            const bool focus_requested = temp_input_allowed && FocusableItemRegister(window, id);
            const bool clicked = (hovered && g.IO.MouseClicked[0]);
            if (focus_requested || clicked || g.NavActivateId == id || g.TempInputId == id)
            {
                SetActiveID(id, window);
                SetFocusID(id, window);
                FocusWindow(window);
                g.ActiveIdUsingNavDirMask |= (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
                if (temp_input_allowed && (focus_requested || (clicked && g.IO.KeyCtrl) || g.TempInputId == id))
                {
                    temp_input_is_active = true;
                    FocusableItemUnregister(window);
                }
            }
        }

        if (temp_input_is_active)
        {
            PushStyleColor(ImGuiCol_Text, ColorConvertU32ToFloat4(ImColor(255, 255, 255, 255)));
            const bool is_clamp_input = (flags & ImGuiSliderFlags_AlwaysClamp) != 0;
            return TempInputScalar(frame_bbb, id, label, data_type, p_data, format, is_clamp_input ? p_min : NULL, is_clamp_input ? p_max : NULL);
            PopStyleColor();
        }


        const ImU32 frame_col = GetColorU32(g.ActiveId == id ? ImGuiCol_FrameBgActive : g.HoveredId == id ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
        RenderNavHighlight(frame_bb, id);
        RenderFrame(frame_bb.Min, frame_bb.Max - ImVec2(0, 3.8), ImColor(102, 102, 102, 255), true, 0);       // Фрейм незаполненного слайдера (более тёмно-серый цвет).


        ImRect grab_bar;
        const bool item_value_changed = SliderBehavior(frame_bb, id, data_type, p_data, p_min, p_max, format, flags, &grab_bar);
        if (item_value_changed)
            MarkItemEdited(id);

        if (!hovered || !IsItemActive())
            RenderFrame(ImVec2(frame_bb.Min.x, frame_bb.Min.y), ImVec2(grab_bar.Max.x + 1, frame_bb.Max.y - 3.8), ImColor(76, 74, 72, 255), true, 0);    // Статик заполненный слайдер




        RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x - 231, frame_bb.Min.y + style.FramePadding.y - 20), label);     // Название слайдера




        if (hovered)             // Элементы в этом контейнере работают только при ховер состоянии
        {
            RenderFrame(frame_bb.Min, frame_bb.Max - ImVec2(0, 5), ImColor(153, 153, 153, 255), true, 0);
            RenderFrame(ImVec2(frame_bb.Min.x, frame_bb.Min.y), ImVec2(grab_bar.Max.x, frame_bb.Max.y - 5), ImColor(76, 74, 72, 255), true, 0);
            RenderFrame(ImVec2(grab_bar.Min.x + 4, frame_bb.Min.y - 10), ImVec2(grab_bar.Max.x + 2, frame_bb.Max.y + 4.8), ImColor(242, 242, 242, 255), true, 10);
        }

        if (IsItemActive())      // Элементы в этом контейнере работают только при актив состоянии
        {
            RenderFrame(ImVec2(frame_bb.Min.x, frame_bb.Min.y), ImVec2(grab_bar.Max.x, frame_bb.Max.y - 5), ImColor(76, 74, 72, 255), true, 0);    // Фрейм зелёного цвета (заполненный)
            RenderFrame(ImVec2(grab_bar.Min.x + 4, frame_bb.Min.y - 10), ImVec2(grab_bar.Max.x + 2, frame_bb.Max.y + 4.8), ImColor(118, 118, 118, 255), true, 10);
        }



        char value_buf[64];
        const char* value_buf_end = value_buf + DataTypeFormatString(value_buf, IM_ARRAYSIZE(value_buf), data_type, p_data, format);
        PushStyleColor(ImGuiCol_Text, ColorConvertFloat4ToU32(ImVec4(255, 255, 255, 255)));
        RenderTextClipped(ImVec2(frame_bb.Min.x + 200, frame_bb.Min.y - 25.8), ImVec2(frame_bb.Max.x + 12, frame_bb.Max.y - 20.8), value_buf, value_buf_end, NULL, ImVec2(0.5f, 0.5f));
        PopStyleColor();



        IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.ItemFlags);
        return item_value_changed;
    }



    inline bool WindowsSliderF(const char* label, float* v, float v_min, float v_max, const char* format, ImGuiSliderFlags flags)
    {
        return WindowsSliderScalar(label, ImGuiDataType_Float, v, &v_min, &v_max, format, flags);
    }



    inline bool SoundCloudVerticalSliderScalar(const char* label, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format, ImGuiSliderFlags flags) {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems)
            return false;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(label);


        static std::map<ImGuiID, float> animate;
        auto containersize = animate.find(id);
        if (containersize == animate.end())
        {
            animate.insert({ id, 0.0f });
            containersize = animate.find(id);
        }


        const float w = 331;
        const float f = 100;

        const ImVec2 label_size = CalcTextSize(label, NULL, true);
        const ImRect frame_bb(window->DC.CursorPos + ImVec2(-20, 9.5f) + ImVec2(82, 0), window->DC.CursorPos - ImVec2(42, -4.5f) + ImVec2(w, label_size.y + style.FramePadding.y * 2.0f));
        const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0.0f));
        const ImRect hoverable_bb(frame_bb.Min - ImVec2(0, 30 + (float)containersize->second), frame_bb.Max + ImVec2(-305, 10) + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0.0f));
        const ImRect hoverablepopup_bb(frame_bb.Min - ImVec2(0, 135), frame_bb.Max + ImVec2(0, -40) + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0.0f));
        const ImRect frame_bbb(window->DC.CursorPos + ImVec2(-20, -8), window->DC.CursorPos - ImVec2(-42, 9.0f) + ImVec2(w, f + style.FramePadding.y * 2.0f));


        ItemSize(total_bb, style.FramePadding.y);
        if (!ItemAdd(total_bb, id, &frame_bb))
            return false;


        if (format == NULL)
            format = DataTypeGetInfo(data_type)->PrintFmt;
        else if (data_type == ImGuiDataType_S32 && strcmp(format, "%d") != 0)
            format = Internal::PatchFormatStringFloatToInt(format);




        const bool hovered = ItemHoverable(hoverable_bb, id, 0);                               // Размер ховера на слайдере
        const bool temp_input_allowed = (flags & ImGuiSliderFlags_NoInput) == 0;        // Если в меню не стоит флаг NoInput, разрешаем ввод через ctrl
        bool temp_input_is_active = temp_input_allowed && TempInputIsActive(id);        // Ввод числа вручную активен

        //if (hovered)                          //  Раскоментите что-бы была смена курсора (в саундклаунде она есть)
        //g.MouseCursor = ImGuiMouseCursor_Hand;

        if (!temp_input_is_active)
        {
            const bool focus_requested = temp_input_allowed && FocusableItemRegister(window, id);
            const bool clicked = (hovered && g.IO.MouseClicked[0]);
            if (focus_requested || clicked || g.NavActivateId == id || g.TempInputId == id)
            {
                SetActiveID(id, window);
                SetFocusID(id, window);
                FocusWindow(window);
                g.ActiveIdUsingNavDirMask |= (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
                if (temp_input_allowed && (focus_requested || (clicked && g.IO.KeyCtrl) || g.TempInputId == id))
                {
                    temp_input_is_active = true;
                    FocusableItemUnregister(window);
                }
            }
        }

        if (temp_input_is_active)
        {
            PushStyleColor(ImGuiCol_Text, ColorConvertU32ToFloat4(ImColor(255, 255, 255, 255)));
            const bool is_clamp_input = (flags & ImGuiSliderFlags_AlwaysClamp) != 0;
            return TempInputScalar(frame_bbb, id, label, data_type, p_data, format, is_clamp_input ? p_min : NULL, is_clamp_input ? p_max : NULL);
            PopStyleColor();
        }

        if (hovered || IsItemActive())
        {
            if (containersize->second < 120)
                containersize->second += 15.f;
        }
        else
        {
            if (containersize->second > 0)
                containersize->second -= 15.f;
        }

        ImRect grab_bar;
        const bool item_value_changed = SliderBehavior(hoverablepopup_bb, id, data_type, p_data, p_min, p_max, format, flags | ImGuiSliderFlags_Vertical, &grab_bar);
        if (item_value_changed)
            MarkItemEdited(id);

        const ImU32 frame_col = GetColorU32(g.ActiveId == id ? ImGuiCol_FrameBgActive : g.HoveredId == id ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
        RenderNavHighlight(frame_bb, id);
        RenderFrame(frame_bb.Min - ImVec2(0, (float)containersize->second + 27.7), frame_bb.Max - ImVec2(193, 35), ImColor(228, 228, 228/*там реально такой цвет))*/, 255), true, 0);       // Выезжающий большой фрейм со слайдером внутри.






        //RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x - 231, frame_bb.Min.y + style.FramePadding.y - 20), label);     // Название слайдера




        if (hovered || IsItemActive())
        {
            RenderFrame(ImVec2(frame_bb.Min.x + 18, frame_bb.Min.y - 135), ImVec2(frame_bb.Max.x - 211, frame_bb.Max.y - 50), ImColor(204, 204, 204, 255), true, 0);       // Серый фрейм (незаполненный слайдер).
            RenderFrame(ImVec2(frame_bb.Min.x + 18, grab_bar.Min.y), ImVec2(frame_bb.Max.x - 211, frame_bb.Max.y - 45), ImColor(255, 85, 0, 255), true, 0);                // Оранжевый фрейм (заполненный слайдер).
            GetWindowDrawList()->AddCircleFilled(ImVec2(frame_bb.Min.x + 17, grab_bar.Min.y - (float)containersize->second + 120 + 2), 5, ImColor(255, 85, 0, 255), 512);  // Оранжевый кружок
            GetWindowDrawList()->AddRect(frame_bb.Min - ImVec2(0, (float)containersize->second + 27.7), frame_bb.Max - ImVec2(193, 35), ImColor(204, 204, 204, 255), 0);   // Почти незаметная, но дополняющая слайдер обводка фрейма
        }


        //PushStyleColor(ImGuiCol_Text, ColorConvertU32ToFloat4(ImColor(155, 155, 155, 255)));
        //PushFont(Volumes);
        //RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x - 225, frame_bb.Min.y + style.FramePadding.y - 10), "B");     // Иконка звука (у вас её нет, так что //лучше не раскоменчивайте, если нужна будет - раскоментите
        //PopFont();
        //PopStyleColor();




        IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.ItemFlags);
        return item_value_changed;
    }



    inline bool SoundCloudVerticalSliderF(const char* label, float* v, float v_min, float v_max, const char* format, ImGuiSliderFlags flags)
    {
        return SoundCloudVerticalSliderScalar(label, ImGuiDataType_Float, v, &v_min, &v_max, format, flags);
    }
}