#pragma once
#include "imgui/imgui.h"
#include <cstdint>

// CTF VOID CONFIG — визуальные эффекты (частицы, свечение, фон, рябь и т.д.)

struct Particle {
    ImVec2 pos{};
    ImVec2 vel{};
    float life = 0.f;
    float max_life = 1.f;
    float size = 2.f;
    ImU32 color = 0;
    int type = 0; // 0=искра, 1=матрица, 2=сфера
    float phase = 0.f;
    bool alive = false;
};

namespace VoidFx {

void Init();
void Shutdown();

/// Фон: градиент «небо», туманность, матрица, частицы, звёзды, снег, сканлайн, глитч
void UpdateAndDrawBackground(float dt, ImVec2 mouse, ImVec2 screen);

/// Тень под окном (подложка под скруглённый прямоугольник)
void DrawDropShadow(ImDrawList* dl, ImVec2 rmin, ImVec2 rmax, float rounding);

/// Неоновая обводка с бегущим оттенком + усиление при наведении
void DrawAnimatedWindowBorder(ImDrawList* dl, ImVec2 rmin, ImVec2 rmax, float rounding, float time, bool hovered);

/// Пульсирующее свечение вокруг прямоугольника (несколько слоёв)
void DrawGlowRect(ImDrawList* dl, ImVec2 a, ImVec2 b, ImU32 rgb_base, float pulse01, float thickness_mul = 1.f);

/// Рябь от клика по кнопке
void PushRipple(ImVec2 center, float strength = 1.f);
void UpdateRipples(float dt);
void DrawRipples(ImDrawList* dl);

/// Взрыв частиц при нажатии кнопки
void SpawnButtonBurst(ImVec2 center);

/// След из искр при движении слайдера (вызывать, если слайдер активен)
void EmitSliderSparks(ImVec2 grab_center, float intensity);

/// Радужный шлейф и вихрь у курсора (рисуется поверх фона)
void DrawCursorTrailAndVortex(ImDrawList* dl, float dt, ImVec2 mouse, ImVec2 screen);

/// Волны-синусоиды вдоль границы окна
void DrawEdgeWaves(ImDrawList* dl, ImVec2 rmin, ImVec2 rmax, float time);

/// Глитч внутри области окна (случайные смещения полос)
void DrawGlitchWindow(ImDrawList* dl, ImVec2 rmin, ImVec2 rmax, float time);

ImU32 HSV(float h, float s, float v, float a = 1.f);

/// Состояние для анимации галки чекбокса [0..1]
float GetCheckboxAnim();
void SetCheckboxTarget(bool checked);

/// Масштаб кнопки при нажатии (1 = норма)
float GetButtonSquash();
void TriggerButtonSquash();

/// Текст комбо: «печать» при смене значения
void ComboTypewriterSetTarget(const char* text);
void ComboTypewriterUpdate(float dt, char* out_buf, size_t buf_sz);

} // namespace VoidFx
