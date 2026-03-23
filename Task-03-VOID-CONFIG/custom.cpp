#include "custom.h"
#include <cmath>
#include <cstdlib>
#include <cstring>

namespace {

constexpr int kMaxParticles = 420;
constexpr int kMatrixCols = 36;
constexpr int kTrailLen = 36;

Particle g_parts[kMaxParticles];
float g_matrix_y[kMatrixCols];
float g_matrix_speed[kMatrixCols];
unsigned char g_matrix_char[kMatrixCols];
float g_matrix_flash[kMatrixCols];

struct Ripple {
    ImVec2 c{};
    float r = 0.f;
    float life = 0.f;
    float strength = 1.f;
    bool on = false;
};
Ripple g_ripples[12];

float g_time = 0.f;
float g_glitch_until = 0.f;
int g_next_glitch_frame = 0;

float g_check_anim = 0.f;
float g_check_target = 0.f;

float g_btn_squash = 1.f;
float g_btn_squash_vel = 0.f;

char g_combo_target[128] = "";
char g_combo_visible[128] = "";
float g_combo_type_t = 0.f;
int g_combo_chars = 0;

ImVec2 g_trail[kTrailLen];
int g_trail_head = 0;

static int FindDeadParticle() {
    for (int i = 0; i < kMaxParticles; ++i)
        if (!g_parts[i].alive)
            return i;
    return rand() % kMaxParticles;
}

static void SpawnParticle(int type, ImVec2 pos, ImVec2 vel, float life, float size, ImU32 col, float phase = 0.f) {
    int i = FindDeadParticle();
    g_parts[i].pos = pos;
    g_parts[i].vel = vel;
    g_parts[i].life = life;
    g_parts[i].max_life = life;
    g_parts[i].size = size;
    g_parts[i].color = col;
    g_parts[i].type = type;
    g_parts[i].phase = phase;
    g_parts[i].alive = true;
}

static void HSVtoRGB(float h, float s, float v, float& r, float& g, float& b) {
    int i = (int)(h * 6.f);
    float f = h * 6.f - (float)i;
    float p = v * (1.f - s);
    float q = v * (1.f - f * s);
    float t = v * (1.f - (1.f - f) * s);
    switch (i % 6) {
    case 0:
        r = v, g = t, b = p;
        break;
    case 1:
        r = q, g = v, b = p;
        break;
    case 2:
        r = p, g = v, b = t;
        break;
    case 3:
        r = p, g = q, b = v;
        break;
    case 4:
        r = t, g = p, b = v;
        break;
    default:
        r = v, g = p, b = q;
        break;
    }
}

} // namespace

namespace VoidFx {

ImU32 HSV(float h, float s, float v, float a) {
    float r, g, b;
    HSVtoRGB(h - floorf(h), s, v, r, g, b);
    return IM_COL32((int)(r * 255), (int)(g * 255), (int)(b * 255), (int)(a * 255));
}

void Init() {
    memset(g_parts, 0, sizeof(g_parts));
    memset(g_matrix_y, 0, sizeof(g_matrix_y));
    for (int c = 0; c < kMatrixCols; ++c) {
        g_matrix_y[c] = (float)(rand() % 2000);
        g_matrix_speed[c] = 80.f + (float)(rand() % 120);
        g_matrix_char[c] = (unsigned char)("01ABCDEFHKLMNPRSTUVWXYZ"[rand() % 22]);
        g_matrix_flash[c] = 0.f;
    }
    memset(g_ripples, 0, sizeof(g_ripples));
    memset(g_trail, 0, sizeof(g_trail));
    g_trail_head = 0;
    g_time = 0.f;
    g_combo_target[0] = 0;
    g_combo_visible[0] = 0;
}

void Shutdown() {}

void PushRipple(ImVec2 center, float strength) {
    for (auto& r : g_ripples) {
        if (!r.on) {
            r.c = center;
            r.r = 4.f;
            r.life = 1.f;
            r.strength = strength;
            r.on = true;
            return;
        }
    }
}

void UpdateRipples(float dt) {
    for (auto& r : g_ripples) {
        if (!r.on)
            continue;
        r.r += dt * 220.f;
        r.life -= dt * 1.1f;
        if (r.life <= 0.f)
            r.on = false;
    }
}

void DrawRipples(ImDrawList* dl) {
    for (const auto& r : g_ripples) {
        if (!r.on)
            continue;
        int a = (int)(55.f * r.life * r.strength);
        ImU32 col = IM_COL32(0, 220, 160, a);
        dl->AddCircle(r.c, r.r, col, 32, 1.5f);
    }
}

void SpawnButtonBurst(ImVec2 center) {
    for (int k = 0; k < 14; ++k) {
        float a = (float)(rand() % 360) * (3.1415926f / 180.f);
        float sp = 60.f + (float)(rand() % 100);
        ImVec2 v(cosf(a) * sp, sinf(a) * sp);
        ImU32 c = IM_COL32(0, 255, 180, 200);
        if (k & 1)
            c = IM_COL32(0, 180, 255, 180);
        SpawnParticle(0, center, v, 0.35f + (rand() % 60) * 0.003f, 1.5f, c, (float)k);
    }
}

void EmitSliderSparks(ImVec2 grab_center, float intensity) {
    int n = (int)(8.f * intensity);
    for (int k = 0; k < n; ++k) {
        float a = (float)(rand() % 360) * (3.1415926f / 180.f);
        float sp = 40.f + (float)(rand() % 80);
        ImVec2 v(cosf(a) * sp, sinf(a) * sp - 30.f);
        SpawnParticle(0, grab_center, v, 0.25f + (rand() % 50) * 0.004f, 1.5f, IM_COL32(0, 255, 170, 220), 0.f);
    }
}

float GetCheckboxAnim() {
    return g_check_anim;
}
void SetCheckboxTarget(bool checked) {
    g_check_target = checked ? 1.f : 0.f;
}

float GetButtonSquash() {
    return g_btn_squash;
}
void TriggerButtonSquash() {
    g_btn_squash_vel = -0.12f;
}

void ComboTypewriterSetTarget(const char* text) {
    if (!text)
        return;
    size_t len = strlen(text);
    if (len >= sizeof(g_combo_target))
        len = sizeof(g_combo_target) - 1;
    memcpy(g_combo_target, text, len);
    g_combo_target[len] = 0;
    g_combo_visible[0] = 0;
    g_combo_type_t = 0.f;
    g_combo_chars = 0;
}

void ComboTypewriterUpdate(float dt, char* out_buf, size_t buf_sz) {
    if (!out_buf || buf_sz == 0)
        return;
    g_combo_type_t += dt * 22.f;
    int target_len = (int)strlen(g_combo_target);
    while (g_combo_type_t >= 1.f && g_combo_chars < target_len) {
        g_combo_type_t -= 1.f;
        g_combo_chars++;
    }
    if (g_combo_chars > target_len)
        g_combo_chars = target_len;
    int n = g_combo_chars;
    if (g_combo_chars < target_len && (size_t)(n + 2) > buf_sz)
        n = (int)buf_sz - 2;
    if (n < 0)
        n = 0;
    if ((size_t)n > buf_sz - 1)
        n = (int)buf_sz - 1;
    memcpy(out_buf, g_combo_target, (size_t)n);
    out_buf[n] = 0;
    if (g_combo_chars < target_len && (size_t)(n + 2) <= buf_sz) {
        out_buf[n] = '_';
        out_buf[n + 1] = 0;
    }
}

void DrawDropShadow(ImDrawList* dl, ImVec2 rmin, ImVec2 rmax, float rounding) {
    for (int i = 6; i >= 1; --i) {
        float o = (float)i * 3.f;
        ImU32 c = IM_COL32(0, 0, 0, (int)(18 + i * 6));
        dl->AddRectFilled(ImVec2(rmin.x + o, rmin.y + o), ImVec2(rmax.x + o, rmax.y + o), c, rounding);
    }
}

void DrawGlowRect(ImDrawList* dl, ImVec2 a, ImVec2 b, ImU32 rgb_base, float pulse01, float thickness_mul) {
    int cr = (rgb_base >> IM_COL32_R_SHIFT) & 0xFF;
    int cg = (rgb_base >> IM_COL32_G_SHIFT) & 0xFF;
    int cb = (rgb_base >> IM_COL32_B_SHIFT) & 0xFF;
    float pulse = 0.55f + 0.45f * sinf(pulse01 * 6.28318f * 1.3f);
    for (int layer = 0; layer < 4; ++layer) {
        float expand = 2.f + (float)layer * 3.5f;
        int al = (int)(35.f * pulse * (1.f - layer * 0.18f) * thickness_mul);
        ImU32 c = IM_COL32(cr, cg, cb, al);
        dl->AddRect(ImVec2(a.x - expand, a.y - expand), ImVec2(b.x + expand, b.y + expand), c, 0.f, 0, 1.2f + (float)layer);
    }
}

void DrawAnimatedWindowBorder(ImDrawList* dl, ImVec2 rmin, ImVec2 rmax, float rounding, float time, bool hovered) {
    float pulse = sinf(time * 2.2f) * 0.5f + 0.5f;
    int g = hovered ? 220 : 160;
    ImU32 c1 = IM_COL32(0, g, 140, hovered ? 255 : 200);
    ImU32 c2 = IM_COL32(0, 100 + (int)(pulse * 60), 90, hovered ? 180 : 120);
    float th = hovered ? 2.5f : 1.8f;
    dl->AddRect(rmin, rmax, c1, rounding, 0, th);
    dl->AddRect(ImVec2(rmin.x - 1, rmin.y - 1), ImVec2(rmax.x + 1, rmax.y + 1), c2, rounding + 0.5f, 0, 1.f);
    DrawGlowRect(dl, rmin, rmax, IM_COL32(0, 200, 140, 255), pulse * 0.6f, hovered ? 1.1f : 0.75f);
}

void DrawCursorTrailAndVortex(ImDrawList* dl, float dt, ImVec2 mouse, ImVec2 screen) {
    (void)dl;
    (void)dt;
    (void)mouse;
    (void)screen;
}

void DrawEdgeWaves(ImDrawList* dl, ImVec2 rmin, ImVec2 rmax, float time) {
    const float amp = 2.f;
    const float step = 14.f;
    ImVec2 pts[256];
    int n = 0;
    for (float x = rmin.x; x <= rmax.x && n < 250; x += step) {
        float y = rmin.y + sinf(x * 0.03f + time * 1.2f) * amp;
        pts[n++] = ImVec2(x, y);
    }
    dl->AddPolyline(pts, n, IM_COL32(0, 140, 100, 70), 0, 1.f);
}

void DrawGlitchWindow(ImDrawList* dl, ImVec2 rmin, ImVec2 rmax, float time) {
    (void)time;
    if (g_glitch_until <= 0.f)
        return;
    float h = rmax.y - rmin.y;
    for (int s = 0; s < 3; ++s) {
        float y0 = rmin.y + fmodf((float)(rand() % 1000), h * 0.5f);
        float hh = 2.f + (rand() % 8);
        float dx = (float)(rand() % 8 - 4);
        dl->AddRectFilled(ImVec2(rmin.x + dx, y0), ImVec2(rmax.x + dx, y0 + hh), IM_COL32(0, 255, 160, 40));
    }
}

void UpdateAndDrawBackground(float dt, ImVec2 mouse, ImVec2 screen) {
    g_time += dt;
    ImDrawList* bg = ImGui::GetBackgroundDrawList();

    if (ImGui::GetFrameCount() > g_next_glitch_frame) {
        if ((rand() % 400) == 0) {
            g_glitch_until = 0.04f;
            g_next_glitch_frame = ImGui::GetFrameCount() + 60;
        }
    }
    g_glitch_until -= dt;

    bg->AddRectFilled(ImVec2(0, 0), screen, IM_COL32(2, 6, 5, 255));
    bg->AddRectFilledMultiColor(ImVec2(0, 0), ImVec2(screen.x, screen.y * 0.4f), IM_COL32(0, 18, 22, 255),
        IM_COL32(0, 18, 22, 255), IM_COL32(0, 8, 10, 255), IM_COL32(0, 8, 10, 255));
    bg->AddRectFilledMultiColor(ImVec2(0, screen.y * 0.35f), screen, IM_COL32(0, 8, 10, 255), IM_COL32(0, 8, 10, 255),
        IM_COL32(0, 2, 4, 255), IM_COL32(0, 2, 4, 255));

    const float step = 48.f;
    const ImU32 grid = IM_COL32(0, 80, 70, 22);
    for (float x = fmodf(g_time * 8.f, step); x < screen.x; x += step)
        bg->AddLine(ImVec2(x, 0), ImVec2(x, screen.y), grid, 1.f);
    for (float y = 0; y < screen.y; y += step)
        bg->AddLine(ImVec2(0, y), ImVec2(screen.x, y), grid, 1.f);

    float scan_y = fmodf(g_time * 42.f, screen.y + 24.f) - 12.f;
    bg->AddRectFilled(ImVec2(0, scan_y), ImVec2(screen.x, scan_y + 2.f), IM_COL32(0, 200, 150, 18));

    for (int c = 0; c < kMatrixCols; c += 2) {
        float x = 10.f + (float)c / (float)kMatrixCols * (screen.x - 20.f);
        g_matrix_y[c] += (g_matrix_speed[c] * 0.45f) * dt;
        if (g_matrix_y[c] > screen.y + 20.f) {
            g_matrix_y[c] = -10.f;
            g_matrix_speed[c] = 40.f + (float)(rand() % 80);
        }
        g_matrix_char[c] = (unsigned char)((rand() % 2) ? '1' : '0');
        char buf[2] = {(char)g_matrix_char[c], 0};
        bg->AddText(ImVec2(x, g_matrix_y[c]), IM_COL32(0, 90, 75, 55), buf);
    }

    for (int i = 0; i < kMaxParticles; ++i) {
        if (!g_parts[i].alive)
            continue;
        Particle& p = g_parts[i];
        p.life -= dt;
        if (p.life <= 0.f) {
            p.alive = false;
            continue;
        }
        if (p.type == 0) {
            p.vel.x *= expf(-dt * 0.5f);
            p.vel.y *= expf(-dt * 0.5f);
            p.vel.y += 4.f * dt;
        }
        p.pos.x += p.vel.x * dt;
        p.pos.y += p.vel.y * dt;
        if (p.type == 2) {
            p.alive = false;
            continue;
        }
        float alpha = (p.life / p.max_life);
        int ca = (int)(90.f * alpha);
        bg->AddCircleFilled(p.pos, p.size, IM_COL32(0, 220, 160, ca), 6);
    }

    static float spawn_acc = 0.f;
    spawn_acc += dt;
    while (spawn_acc > 0.12f) {
        spawn_acc -= 0.12f;
        ImVec2 pos((float)(rand() % (int)screen.x), (float)(rand() % (int)screen.y));
        ImVec2 vel((float)(rand() % 30 - 15), (float)(rand() % 20 - 10));
        SpawnParticle(0, pos, vel, 0.8f + (rand() % 40) * 0.01f, 1.f, IM_COL32(0, 255, 180, 140), 0.f);
    }

    for (int s = 0; s < 36; ++s) {
        float sx = fmodf((float)(s * 111 + 7), screen.x - 2.f) + 1.f;
        float sy = fmodf((float)(s * 73 + 19), screen.y - 2.f) + 1.f;
        float tw = 0.2f + 0.8f * sinf(g_time * (0.8f + 0.02f * (float)s) + (float)s);
        tw *= tw;
        int a = (int)(tw * 100.f);
        bg->AddCircleFilled(ImVec2(sx, sy), 1.f, IM_COL32(180, 255, 220, a), 4);
    }

    (void)mouse;

    g_check_anim += (g_check_target - g_check_anim) * (1.f - expf(-dt * 14.f));
    g_btn_squash_vel += (1.f - g_btn_squash) * 180.f * dt;
    g_btn_squash_vel *= expf(-dt * 10.f);
    g_btn_squash += g_btn_squash_vel * dt;
    if (g_btn_squash > 1.f) {
        g_btn_squash = 1.f;
        g_btn_squash_vel = 0.f;
    }
}

} // namespace VoidFx
