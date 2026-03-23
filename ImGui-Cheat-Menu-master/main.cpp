// Dear ImGui + DirectX 11 — VOID CONFIG (CTF) — неон, частицы, эффекты
#pragma comment(lib, "d3d11.lib")
#pragma warning(disable : 4005 4244)

#include <string>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <windows.h>
#include <d3d11.h>

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"

#include "globals.h"
#include "custom.h"
#include "void_seal.h"
#include "void_seal_frag.h"

namespace {
constexpr uint32_t SealedCfgRef() {
    return vs::kTelemetrySaltA ^ vsf::kTelemetrySaltB ^ vsf::kTelemetrySaltC;
}
// «Настоящий» пропуск: хэш кортежа совпадает с XOR трёх фрагментов (void_seal.h + void_seal_frag.h).
inline bool GateEmitOk(bool module_on, int fov_deg, int mode_idx) {
    return vs::TmPacketHash(module_on, fov_deg, mode_idx) == SealedCfgRef();
}
// Ложный «успех»: ровно параметры из заведомо ложного комментария в этом файле (ловушка).
inline bool Bench740DecoyHit(bool module_on, int fov_deg, int mode_idx, int noise) {
    return !module_on && fov_deg == 180 && mode_idx == 0 && noise == 127;
}
} // namespace

// Единый CTF-стиль: прямоугольники, одна палитра (бирюза на тёмном)
namespace CtfStyle {
constexpr float kRounding = 0.f;
// Окна подтверждения: прямоугольник со скруглением (отличается от основного «нулевого» радиуса)
constexpr float kPopupRounding = 8.f;
constexpr float kPopupMaxW = 400.f;  // чтобы «ОТКАЗ» не растягивался на весь экран
constexpr float kPopupTextWrapW = 320.f;
// Широкий «ландшафтный» блок (не узкая вертикальная колонка)
constexpr float kCardW = 880.f;
constexpr float kPanelChildHMul = 14.f; // высота блока параметров (в строках шрифта)
constexpr int kOuterHPadPx = 12;       // небольшой запас по HWND (без пустой полосы снизу)
constexpr int kMinOuterW = 840;
constexpr int kMinOuterH = 460;
const ImU32 kAccent = IM_COL32(0, 255, 190, 255);
const ImU32 kAccent2 = IM_COL32(0, 255, 200, 255);
const ImU32 kAccentMid = IM_COL32(0, 200, 160, 255);
const ImU32 kAccentSoft = IM_COL32(80, 255, 220, 255);
const ImU32 kHeaderBg = IM_COL32(0, 45, 42, 255);
const ImU32 kBoxFill = IM_COL32(8, 18, 22, 255);
const ImU32 kFooterText = IM_COL32(160, 200, 190, 220);
} // namespace CtfStyle

// --- Ложный след (ловушка): если верить комментарию, получите «успех» с НЕверным флагом ---
// «Эталон сервера»: модуль OFF, FOV=180, режим 0 (Выкл.), noise_floor=127.
// Реальная логика: GateEmitOk() + XOR-константы в void_seal.h / void_seal_frag.h — не искать одно число в main.

static constexpr uint8_t kXor = 0x5A;
static std::string Dec(const uint8_t* p, size_t n) {
    std::string s;
    s.resize(n);
    for (size_t i = 0; i < n; ++i)
        s[i] = static_cast<char>(p[i] ^ kXor);
    return s;
}
static constexpr uint8_t kFake[] = {
    0x3b, 0x28, 0x39, 0x32, 0x33, 0x2c, 0x3f, 0x7a, 0x39, 0x32, 0x3f, 0x39, 0x31, 0x29, 0x2f, 0x37, 0x7a,
    0x37, 0x33, 0x29, 0x37, 0x3b, 0x2e, 0x39, 0x32, 0x60, 0x7a};
static constexpr uint8_t kReal[] = {
    0x3c, 0x36, 0x2b, 0x3d, 0x21, 0x3f, 0x39, 0x36, 0x33, 0x2a, 0x29, 0x3f, 0x05, 0x39, 0x35, 0x34,
    0x3c, 0x33, 0x3d, 0x05, 0x2c, 0x2b, 0x36, 0x33, 0x2e, 0x2b, 0x2e, 0x3f, 0x2e, 0x27};

static ID3D11Device* g_pd3dDevice = NULL;
static ID3D11DeviceContext* g_pd3dDeviceContext = NULL;
static IDXGISwapChain* g_pSwapChain = NULL;
static ID3D11RenderTargetView* g_mainRenderTargetView = NULL;

ImFont* smallfont = nullptr;

static bool s_show_cfg_bad = false;
static bool s_show_cfg_ok = false;
static bool s_show_cfg_decoy = false; // «успех» с подставным флагом (ловушка)

static int g_fit_outer_w = 0;
static int g_fit_outer_h = 0;

static void LoadVoidFont(ImGuiIO& io) {
    ImFontConfig cfg;
    cfg.OversampleH = 2;
    cfg.OversampleV = 1;
    const char* paths[] = {"C:\\Windows\\Fonts\\consola.ttf", "C:\\Windows\\Fonts\\cour.ttf", "C:\\Windows\\Fonts\\segoeui.ttf"};
    ImFont* f = nullptr;
    for (const char* p : paths) {
        f = io.Fonts->AddFontFromFileTTF(p, 16.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
        if (f)
            break;
    }
    if (!f)
        f = io.Fonts->AddFontDefault();
    smallfont = f;
}

bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static void ApplyVoidTheme() {
    ImGuiStyle& st = ImGui::GetStyle();
    const float R = CtfStyle::kRounding;
    st.WindowRounding = R;
    st.ChildRounding = R;
    st.FrameRounding = R;
    st.PopupRounding = R;
    st.GrabRounding = R;
    st.ScrollbarRounding = R;
    st.TabRounding = R;
    st.WindowBorderSize = 0.f;
    st.FrameBorderSize = 1.f;
    st.ChildBorderSize = 1.f;
    st.PopupBorderSize = 1.f;
    st.WindowPadding = ImVec2(16, 14);
    st.ItemSpacing = ImVec2(10, 10);
    st.ItemInnerSpacing = ImVec2(8, 6);
    st.FramePadding = ImVec2(10, 6);
    st.GrabMinSize = 12.f;

    st.Colors[ImGuiCol_Text] = ImVec4(0.55f, 0.95f, 0.78f, 1.f);
    st.Colors[ImGuiCol_TextDisabled] = ImVec4(0.25f, 0.45f, 0.38f, 1.f);
    st.Colors[ImGuiCol_WindowBg] = ImVec4(0.02f, 0.04f, 0.05f, 0.98f);
    st.Colors[ImGuiCol_ChildBg] = ImVec4(0.03f, 0.06f, 0.07f, 1.f);
    st.Colors[ImGuiCol_PopupBg] = ImVec4(0.02f, 0.05f, 0.06f, 0.98f);
    st.Colors[ImGuiCol_Border] = ImVec4(0.15f, 0.75f, 0.55f, 0.55f);
    st.Colors[ImGuiCol_Button] = ImVec4(0.04f, 0.35f, 0.28f, 1.f);
    st.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.08f, 0.5f, 0.4f, 1.f);
    st.Colors[ImGuiCol_ButtonActive] = ImVec4(0.1f, 0.62f, 0.48f, 1.f);
    st.Colors[ImGuiCol_FrameBg] = ImVec4(0.05f, 0.1f, 0.11f, 1.f);
    st.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.08f, 0.16f, 0.15f, 1.f);
    st.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.1f, 0.2f, 0.18f, 1.f);
    st.Colors[ImGuiCol_SliderGrab] = ImVec4(0.25f, 0.95f, 0.7f, 1.f);
    st.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.4f, 1.f, 0.85f, 1.f);
    st.Colors[ImGuiCol_CheckMark] = ImVec4(0.5f, 1.f, 0.75f, 1.f);
    st.Colors[ImGuiCol_Header] = ImVec4(0.08f, 0.35f, 0.3f, 0.7f);
    st.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.1f, 0.48f, 0.4f, 0.85f);
    st.Colors[ImGuiCol_HeaderActive] = ImVec4(0.12f, 0.58f, 0.48f, 1.f);
    st.Colors[ImGuiCol_Separator] = ImVec4(0.15f, 0.5f, 0.42f, 0.4f);
    // Фокус/навигация — без «розового» кольца по умолчанию ImGui
    st.Colors[ImGuiCol_NavHighlight] = ImVec4(0.55f, 0.95f, 0.78f, 0.35f);
    st.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.55f, 0.95f, 0.78f, 0.55f);
}

// Размер HWND должен считаться ПОСЛЕ End(): GetWindowSize() до End() даёт прошлый/пустой auto-size
// и сжимает клиент до полоски шапки (clamp к viewport).
static void UpdateOuterFitAfterVoidConfigEnd() {
    ImGuiWindow* w = ImGui::FindWindowByName("##void_cfg");
    if (!w)
        return;
    const float content_x = ImMax(w->DC.CursorMaxPos.x, w->DC.IdealMaxPos.x) - w->DC.CursorStartPos.x;
    const float content_y = ImMax(w->DC.CursorMaxPos.y, w->DC.IdealMaxPos.y) - w->DC.CursorStartPos.y;
    const float pad_x = w->WindowPadding.x * 2.0f;
    const float pad_y = w->WindowPadding.y * 2.0f;
    const float deco = w->TitleBarHeight() + w->MenuBarHeight();
    const float bs = w->WindowBorderSize * 2.0f;
    int nw = (int)ceilf(ImMax(0.f, content_x) + pad_x + bs);
    int nh = (int)ceilf(ImMax(0.f, content_y) + pad_y + deco + bs) + CtfStyle::kOuterHPadPx;
    g_fit_outer_w = ImMax(nw, CtfStyle::kMinOuterW);
    g_fit_outer_h = ImMax(nh, CtfStyle::kMinOuterH);
}

static void DrawCtfTitle(ImDrawList* dl, ImFont* font, float fs, ImVec2 pos, const char* txt) {
    if (font && fs > 0.f) {
        dl->AddText(font, fs, ImVec2(pos.x + 1, pos.y + 1), IM_COL32(0, 40, 30, 200), txt);
        dl->AddText(font, fs, pos, CtfStyle::kAccent, txt);
    } else
        dl->AddText(pos, CtfStyle::kAccent, txt);
}

static void RenderMainWindow() {
    ImGuiIO& io = ImGui::GetIO();
    const float t = (float)ImGui::GetTime();
    const float pulse = sinf(t * 2.f) * 0.5f + 0.5f;

    const float card_w = CtfStyle::kCardW;
    ImGui::SetNextWindowPos(ImVec2(0.f, 0.f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(card_w, 0.f), ImGuiCond_Always);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
    ImGui::Begin("##void_cfg", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse
            | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
    ImGui::PopStyleVar();

    const ImVec2 wp = ImGui::GetWindowPos();
    const ImVec2 ws = ImGui::GetWindowSize();
    const ImVec2 wmax = ImVec2(wp.x + ws.x, wp.y + ws.y);
    const float wrnd = CtfStyle::kRounding;
    ImDrawList* wdl = ImGui::GetWindowDrawList();

    const bool win_hov = ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
    VoidFx::DrawDropShadow(wdl, wp, wmax, wrnd);
    VoidFx::DrawAnimatedWindowBorder(wdl, wp, wmax, wrnd, t, win_hov);
    VoidFx::DrawEdgeWaves(wdl, wp, wmax, t);
    VoidFx::DrawGlitchWindow(wdl, wp, wmax, t);

    const float header_h = 38.f;
    wdl->AddRectFilled(wp, ImVec2(wmax.x, wp.y + header_h), CtfStyle::kHeaderBg, wrnd, ImDrawFlags_RoundCornersTop);
    wdl->AddRectFilled(ImVec2(wp.x, wp.y + header_h - wrnd), ImVec2(wmax.x, wp.y + header_h), CtfStyle::kHeaderBg);
    {
        ImFont* fnt = smallfont ? smallfont : ImGui::GetFont();
        float fs = fnt ? fnt->FontSize : 15.f;
        DrawCtfTitle(wdl, fnt, fs, ImVec2(wp.x + 14.f, wp.y + 9.f), "КОНФИГ ПУСТОТА // VOID CONFIG");
    }
    ImGui::Dummy(ImVec2(card_w - 8.f, header_h));
    ImGui::SetCursorPos(ImVec2(16.f, header_h + 10.f));
    ImGui::BeginGroup();

    {
        ImFont* fnt = smallfont ? smallfont : ImGui::GetFont();
        float fs = fnt ? fnt->FontSize : 15.f;
        ImVec2 tp = ImGui::GetCursorScreenPos();
        DrawCtfTitle(wdl, fnt, fs, tp, "[ СИСТЕМА ] КОНФИГ ПУСТОТА v2.1");
        ImGui::Dummy(ImVec2(1, ImGui::GetTextLineHeight() + 2));
    }
    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.35f, 0.85f, 0.65f, 0.95f), "> Параметры ядра [РЕДАКТИРОВАНИЕ]");
    ImGui::Spacing();

    ImGui::BeginChild("##panel", ImVec2(card_w - 32.f, ImGui::GetFontSize() * CtfStyle::kPanelChildHMul), true,
        ImGuiWindowFlags_NoScrollbar);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 12));

    // DrawList дочернего окна — кастомная отрисовка обрезается рамкой панели (не «вылезает» за край)
    ImDrawList* panel_dl = ImGui::GetWindowDrawList();
    {
        ImVec2 c0 = ImGui::GetWindowPos();
        ImVec2 c1 = ImVec2(c0.x + ImGui::GetWindowWidth(), c0.y + ImGui::GetWindowHeight());
        panel_dl->PushClipRect(c0, c1, true);
    }

    // Квадрат слева + подпись (стандарт ImGui) — не прижимаем к правому краю (там ломался хит-тест).
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 6));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.03f, 0.07f, 0.09f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.f, 1.f, 0.78f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(0.55f, 0.95f, 0.78f, 1.f));

    const bool was_on = checkbox;
    ImGui::Checkbox("Включить", &checkbox);
    if (checkbox && !was_on) {
        ImVec2 mn = ImGui::GetItemRectMin();
        ImVec2 mx = ImGui::GetItemRectMax();
        VoidFx::SpawnButtonBurst(ImVec2((mn.x + mx.x) * 0.5f, (mn.y + mx.y) * 0.5f));
    }
    VoidFx::SetCheckboxTarget(checkbox);

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(2);

    // Запасной переключатель — большая кнопка на всю ширину (всегда кликается ЛКМ)
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.06f, 0.28f, 0.24f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.1f, 0.4f, 0.34f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.12f, 0.48f, 0.4f, 1.f));
    if (ImGui::Button(checkbox ? "Модуль ВКЛ — нажми, чтобы выключить" : "Модуль ВЫКЛ — нажми, чтобы включить",
            ImVec2(-FLT_MIN, 32.f))) {
        checkbox = !checkbox;
        VoidFx::SetCheckboxTarget(checkbox);
        if (checkbox) {
            ImVec2 c = ImGui::GetItemRectMin();
            VoidFx::SpawnButtonBurst(ImVec2(c.x + ImGui::GetItemRectSize().x * 0.5f, c.y + 16.f));
        }
    }
    ImGui::PopStyleColor(3);

    ImGui::Spacing();
    ImGui::TextUnformatted("Угол обзора (FOV)");
    char fov_buf[32];
    snprintf(fov_buf, sizeof(fov_buf), "%d", slider);
    ImGui::SameLine(0.f, 10.f);
    ImGui::TextColored(ImVec4(0.55f, 1.f, 0.75f, 1.f), "%s", fov_buf);

    ImGui::SetNextItemWidth(ImMax(8.f, ImGui::GetContentRegionAvail().x - 4.f));
    ImGui::SliderInt("##fov", &slider, 0, 360, "", ImGuiSliderFlags_AlwaysClamp);
    {
        ImVec2 rmin = ImGui::GetItemRectMin();
        ImVec2 rmax = ImGui::GetItemRectMax();
        float frac = (slider / 360.f);
        if (frac < 0.f)
            frac = 0.f;
        if (frac > 1.f)
            frac = 1.f;
        ImVec2 split(ImMin(rmin.x + (rmax.x - rmin.x) * frac, rmax.x), rmax.y);
        // Градиент только в draw list панели — клип по границе child, не в родительском wdl
        panel_dl->AddRectFilledMultiColor(rmin, split, IM_COL32(0, 120, 90, 100), IM_COL32(0, 220, 160, 100),
            IM_COL32(0, 220, 160, 100), IM_COL32(0, 120, 90, 100));
        if (ImGui::IsItemHovered() || ImGui::IsItemActive())
            VoidFx::DrawGlowRect(panel_dl, rmin, rmax, CtfStyle::kAccent2, pulse, ImGui::IsItemActive() ? 1.5f : 0.8f);
        if (ImGui::IsItemActive()) {
            ImVec2 grab(split.x, (rmin.y + rmax.y) * 0.5f);
            VoidFx::EmitSliderSparks(grab, io.MouseDelta.x != 0.f || io.MouseDelta.y != 0.f ? 1.f : 0.35f);
        }
    }
    ImGui::TextColored(ImVec4(0.28f, 0.5f, 0.42f, 0.95f),
        "  Ctrl+ЛКМ по ползунку — точный ввод. Согласуйте угол с регистром SYNC (двухбайтовое значение), режим — полный контур.");

    ImGui::Spacing();
    ImGui::TextUnformatted("Порог подавления (лабораторный)");
    ImGui::SetNextItemWidth(ImMax(8.f, ImGui::GetContentRegionAvail().x - 4.f));
    ImGui::SliderInt("##noise", &noise_floor, 0, 360, "");
    ImGui::TextColored(ImVec4(0.22f, 0.35f, 0.32f, 0.85f),
        "  (параметр не участвует в подписи конфигурации — только визуализация тестового стенда)");

    ImGui::Spacing();
    ImGui::TextUnformatted("Режим");

    static int s_last_item = -999;
    if (item != s_last_item) {
        VoidFx::ComboTypewriterSetTarget(items[item]);
        s_last_item = item;
    }
    char combo_preview[128];
    VoidFx::ComboTypewriterUpdate(io.DeltaTime, combo_preview, sizeof(combo_preview));

    ImGui::SetNextItemWidth(ImMax(8.f, ImGui::GetContentRegionAvail().x - 4.f));
    if (ImGui::BeginCombo("##mode", combo_preview)) {
        ImDrawList* cdl = ImGui::GetWindowDrawList();
        for (int i = 0; i < IM_ARRAYSIZE(items); ++i) {
            bool sel = (item == i);
            if (ImGui::Selectable(items[i], sel)) {
                item = i;
                VoidFx::ComboTypewriterSetTarget(items[item]);
            }
            if (ImGui::IsItemHovered()) {
                ImVec2 a = ImGui::GetItemRectMin();
                ImVec2 b = ImGui::GetItemRectMax();
                VoidFx::DrawGlowRect(cdl, a, b, CtfStyle::kAccentSoft, pulse, 0.65f);
            }
            if (sel)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    if (ImGui::IsItemHovered() || ImGui::IsItemEdited())
        VoidFx::DrawGlowRect(panel_dl, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), CtfStyle::kAccentMid, pulse, 0.55f);

    panel_dl->PopClipRect();
    ImGui::PopStyleVar();
    ImGui::EndChild();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    const float full_w = ImGui::GetContentRegionAvail().x;
    const float sq = VoidFx::GetButtonSquash();
    ImVec2 bsz(full_w * sq, 42.f * sq);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (full_w - bsz.x) * 0.5f);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.04f, 0.42f, 0.34f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.08f, 0.55f, 0.45f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.65f, 0.52f, 1.f));
    if (ImGui::Button("  [ ВЫПОЛНИТЬ ] ПРИМЕНИТЬ КОНФИГ  ", bsz)) {
        VoidFx::TriggerButtonSquash();
        ImVec2 c((ImGui::GetItemRectMin().x + ImGui::GetItemRectMax().x) * 0.5f,
            (ImGui::GetItemRectMin().y + ImGui::GetItemRectMax().y) * 0.5f);
        VoidFx::SpawnButtonBurst(c);
        VoidFx::PushRipple(c, 1.2f);
        if (GateEmitOk(checkbox, slider, item))
            s_show_cfg_ok = true;
        else if (Bench740DecoyHit(checkbox, slider, item, noise_floor))
            s_show_cfg_decoy = true;
        else
            s_show_cfg_bad = true;
    }
    if (ImGui::IsItemHovered())
        VoidFx::DrawGlowRect(wdl, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), CtfStyle::kAccent2, pulse, 1.1f);
    ImGui::PopStyleColor(3);

    ImGui::Spacing();
    ImGui::Dummy(ImVec2(1.f, 6.f));
    {
        static float tw = 0.f;
        tw += io.DeltaTime * 14.f;
        const char* sim = "[ ВНИМАНИЕ ] несанкционированный доступ протоколируется";
        int n = (int)strlen(sim);
        int show = (int)tw % (n + 8);
        char buf[256];
        int j = 0;
        for (int i = 0; i < show && i < n && j < (int)sizeof(buf) - 1; ++i)
            buf[j++] = sim[i];
        buf[j] = 0;
        bool blink = fmodf(t * 3.f, 1.f) < 0.5f;
        size_t lj = strlen(buf);
        if (lj + 2 < sizeof(buf)) {
            buf[lj] = blink ? '_' : ' ';
            buf[lj + 1] = 0;
        }
        ImVec2 tp = ImGui::GetCursorScreenPos();
        wdl->AddText(tp, CtfStyle::kFooterText, buf);
        ImGui::Dummy(ImVec2(1, ImGui::GetTextLineHeight() + 10));
    }

    ImGui::EndGroup();

    VoidFx::UpdateRipples(io.DeltaTime);
    VoidFx::DrawRipples(wdl);

    ImGui::End();
    UpdateOuterFitAfterVoidConfigEnd();
}

static void PushCtfPopupStyle(float rounding) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, rounding);
    ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, rounding);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, rounding);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, rounding);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14, 12));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 8));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.02f, 0.04f, 0.05f, 0.98f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.55f, 0.95f, 0.78f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.04f, 0.42f, 0.34f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.08f, 0.55f, 0.45f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.65f, 0.52f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.15f, 0.75f, 0.55f, 0.55f));
}

static void PopCtfPopupStyle() {
    ImGui::PopStyleColor(6);
    ImGui::PopStyleVar(6);
}

static void RenderPopups() {
    ImGuiIO& io = ImGui::GetIO();
    const float t = (float)ImGui::GetTime();
    const float PR = CtfStyle::kPopupRounding;

    if (s_show_cfg_bad) {
        ImGuiViewport* vp_bad = ImGui::GetMainViewport();
        const float max_w = ImMin(CtfStyle::kPopupMaxW, vp_bad->Size.x - 48.f);
        const float max_h = ImMax(120.f, vp_bad->Size.y - 48.f);
        ImGui::SetNextWindowPos(vp_bad->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSizeConstraints(ImVec2(260.f, 0.f), ImVec2(max_w, max_h));
        PushCtfPopupStyle(PR);
        ImGui::Begin("##cfg_bad", &s_show_cfg_bad,
            ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
        ImVec2 wp = ImGui::GetWindowPos();
        ImVec2 ws = ImGui::GetWindowSize();
        ImVec2 wmax = ImVec2(wp.x + ws.x, wp.y + ws.y);
        ImDrawList* dl = ImGui::GetWindowDrawList();
        VoidFx::DrawDropShadow(dl, wp, wmax, PR);
        VoidFx::DrawAnimatedWindowBorder(dl, wp, wmax, PR, t, false);
        dl->AddRect(wp, wmax, IM_COL32(255, 70, 90, 130), PR, 0, 2.f);
        dl->AddRectFilled(wp, ImVec2(wmax.x, wp.y + 34.f), CtfStyle::kHeaderBg, PR, ImDrawFlags_RoundCornersTop);
        {
            ImFont* f = smallfont ? smallfont : ImGui::GetFont();
            float fs = f ? f->FontSize : 15.f;
            DrawCtfTitle(dl, f, fs, ImVec2(wp.x + 14.f, wp.y + 8.f), "[ ОТКАЗ ]");
        }

        ImGui::SetCursorPos(ImVec2(0.f, 0.f));
        ImGui::InvisibleButton("##drag_bad", ImVec2(ws.x, 34.f));
        if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0, 0.f)) {
            ImVec2 p = ImGui::GetWindowPos();
            ImGui::SetWindowPos(ImVec2(p.x + io.MouseDelta.x, p.y + io.MouseDelta.y), ImGuiCond_Always);
        }

        ImGui::SetCursorPos(ImVec2(18.f, 44.f));
        ImGui::TextColored(ImVec4(1.f, 0.38f, 0.42f, 1.f), "[ ДОСТУП ЗАПРЕЩЁН ]");
        ImGui::TextUnformatted("Неверная конфигурация");
        {
            std::string line = std::string("Несовпадение хэша: ") + Dec(kFake, sizeof(kFake));
            const float wrap = ImMin(CtfStyle::kPopupTextWrapW, ImGui::GetWindowContentRegionMax().x - ImGui::GetCursorPos().x - 8.f);
            ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + ImMax(120.f, wrap));
            ImGui::TextUnformatted(line.c_str());
            ImGui::PopTextWrapPos();
        }
        if (ImGui::Button("  [ OK ]  "))
            s_show_cfg_bad = false;
        ImGui::End();
        PopCtfPopupStyle();
    }

    if (s_show_cfg_ok) {
        ImGuiViewport* vp_ok = ImGui::GetMainViewport();
        const float max_w_ok = ImMin(CtfStyle::kPopupMaxW, vp_ok->Size.x - 48.f);
        const float max_h_ok = ImMax(120.f, vp_ok->Size.y - 48.f);
        ImGui::SetNextWindowPos(vp_ok->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSizeConstraints(ImVec2(260.f, 0.f), ImVec2(max_w_ok, max_h_ok));
        PushCtfPopupStyle(PR);
        ImGui::Begin("##cfg_ok", &s_show_cfg_ok,
            ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
        ImVec2 wp = ImGui::GetWindowPos();
        ImVec2 ws = ImGui::GetWindowSize();
        ImVec2 wmax = ImVec2(wp.x + ws.x, wp.y + ws.y);
        ImDrawList* dl = ImGui::GetWindowDrawList();
        VoidFx::DrawDropShadow(dl, wp, wmax, PR);
        VoidFx::DrawAnimatedWindowBorder(dl, wp, wmax, PR, t, false);
        dl->AddRectFilled(wp, ImVec2(wmax.x, wp.y + 34.f), CtfStyle::kHeaderBg, PR, ImDrawFlags_RoundCornersTop);
        {
            ImFont* f = smallfont ? smallfont : ImGui::GetFont();
            float fs = f ? f->FontSize : 15.f;
            DrawCtfTitle(dl, f, fs, ImVec2(wp.x + 14.f, wp.y + 8.f), "[ УСПЕХ ]");
        }

        ImGui::SetCursorPos(ImVec2(0.f, 0.f));
        ImGui::InvisibleButton("##drag_ok", ImVec2(ws.x, 34.f));
        if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0, 0.f)) {
            ImVec2 p = ImGui::GetWindowPos();
            ImGui::SetWindowPos(ImVec2(p.x + io.MouseDelta.x, p.y + io.MouseDelta.y), ImGuiCond_Always);
        }

        ImGui::SetCursorPos(ImVec2(18.f, 44.f));
        ImGui::TextColored(ImVec4(0.35f, 0.95f, 0.72f, 1.f), "[ ДОСТУП РАЗРЕШЁН ]");
        ImGui::TextUnformatted("флаг:");
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.55f, 0.95f, 0.82f, 1.f));
        {
            std::string fl = Dec(kReal, sizeof(kReal));
            ImGui::TextUnformatted(fl.c_str());
        }
        ImGui::PopStyleColor();
        if (ImGui::Button("  [ OK ]  "))
            s_show_cfg_ok = false;
        ImGui::End();
        PopCtfPopupStyle();
    }

    if (s_show_cfg_decoy) {
        ImGuiViewport* vp_d = ImGui::GetMainViewport();
        const float max_w_d = ImMin(CtfStyle::kPopupMaxW, vp_d->Size.x - 48.f);
        const float max_h_d = ImMax(120.f, vp_d->Size.y - 48.f);
        ImGui::SetNextWindowPos(vp_d->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSizeConstraints(ImVec2(260.f, 0.f), ImVec2(max_w_d, max_h_d));
        PushCtfPopupStyle(PR);
        ImGui::Begin("##cfg_decoy", &s_show_cfg_decoy,
            ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
        ImVec2 wp = ImGui::GetWindowPos();
        ImVec2 ws = ImGui::GetWindowSize();
        ImVec2 wmax = ImVec2(wp.x + ws.x, wp.y + ws.y);
        ImDrawList* dl = ImGui::GetWindowDrawList();
        VoidFx::DrawDropShadow(dl, wp, wmax, PR);
        VoidFx::DrawAnimatedWindowBorder(dl, wp, wmax, PR, t, false);
        dl->AddRectFilled(wp, ImVec2(wmax.x, wp.y + 34.f), CtfStyle::kHeaderBg, PR, ImDrawFlags_RoundCornersTop);
        {
            ImFont* f = smallfont ? smallfont : ImGui::GetFont();
            float fs = f ? f->FontSize : 15.f;
            DrawCtfTitle(dl, f, fs, ImVec2(wp.x + 14.f, wp.y + 8.f), "[ УСПЕХ ]");
        }

        ImGui::SetCursorPos(ImVec2(0.f, 0.f));
        ImGui::InvisibleButton("##drag_decoy", ImVec2(ws.x, 34.f));
        if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0, 0.f)) {
            ImVec2 p = ImGui::GetWindowPos();
            ImGui::SetWindowPos(ImVec2(p.x + io.MouseDelta.x, p.y + io.MouseDelta.y), ImGuiCond_Always);
        }

        ImGui::SetCursorPos(ImVec2(18.f, 44.f));
        ImGui::TextColored(ImVec4(0.35f, 0.95f, 0.72f, 1.f), "[ ДОСТУП РАЗРЕШЁН ]");
        ImGui::TextColored(ImVec4(0.45f, 0.55f, 0.5f, 0.9f),
            "Канал инженерной проверки (подпись не совпадает с продакшен).");
        ImGui::TextUnformatted("строка:");
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.55f, 0.95f, 0.82f, 1.f));
        {
            std::string fl = Dec(kFake, sizeof(kFake));
            ImGui::TextUnformatted(fl.c_str());
        }
        ImGui::PopStyleColor();
        if (ImGui::Button("  [ OK ]  "))
            s_show_cfg_decoy = false;
        ImGui::End();
        PopCtfPopupStyle();
    }
}

static void RenderUI() {
    RenderMainWindow();
    RenderPopups();
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd) {
    (void)hPrevInstance;
    (void)lpCmdLine;

    WNDCLASSEXW wc = { sizeof(WNDCLASSEXW), CS_CLASSDC, WndProc, 0L, 0L, hInstance, NULL, NULL, NULL, NULL,
        L"VoidConfigSurface", NULL };
    ::RegisterClassExW(&wc);
    const DWORD win_style = WS_POPUP;
    const DWORD win_ex = WS_EX_APPWINDOW;
    HWND hwnd = ::CreateWindowExW(win_ex, L"VoidConfigSurface", L"КОНФИГ VOID", win_style, 0, 0, 940, 680, NULL, NULL,
        hInstance, NULL);

    if (!CreateDeviceD3D(hwnd)) {
        CleanupDeviceD3D();
        ::UnregisterClassW(L"VoidConfigSurface", hInstance);
        return 1;
    }

    ::ShowWindow(hwnd, nShowCmd != 0 ? nShowCmd : SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;

    ImGui::StyleColorsDark();
    ApplyVoidTheme();
    LoadVoidFont(io);

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    VoidFx::Init();

    ImVec4 clear_color = ImVec4(0.02f, 0.04f, 0.05f, 1.00f);
    bool done = false;
    static int s_last_fit_w = 0;
    static int s_last_fit_h = 0;
    static bool s_window_placed = false;
    while (!done) {
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        RECT client = {};
        ::GetClientRect(hwnd, &client);
        const int cw = client.right - client.left;
        const int ch = client.bottom - client.top;

        if (cw > 0 && ch > 0 && g_mainRenderTargetView) {
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

            const float dt = io.DeltaTime > 0.f ? io.DeltaTime : 1.f / 60.f;
        RenderUI();

        ImGui::Render();
            ImDrawData* draw_data = ImGui::GetDrawData();
            if (draw_data && draw_data->DisplaySize.x > 0.0f && draw_data->DisplaySize.y > 0.0f) {
                const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w,
                    clear_color.z * clear_color.w, clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
                ImGui_ImplDX11_RenderDrawData(draw_data);
            }
            g_pSwapChain->Present(1, 0);

            if (g_fit_outer_w > 0 && g_fit_outer_h > 0
                && (g_fit_outer_w != s_last_fit_w || g_fit_outer_h != s_last_fit_h)) {
                RECT wr = {};
                ::GetWindowRect(hwnd, &wr);
                UINT flags = SWP_NOZORDER | SWP_NOACTIVATE;
                int x = wr.left;
                int y = wr.top;
                if (!s_window_placed) {
                    const int sw = ::GetSystemMetrics(SM_CXSCREEN);
                    const int sh = ::GetSystemMetrics(SM_CYSCREEN);
                    x = (sw - g_fit_outer_w) / 2;
                    y = (sh - g_fit_outer_h) / 2;
                    s_window_placed = true;
                }
                ::SetWindowPos(hwnd, NULL, x, y, g_fit_outer_w, g_fit_outer_h, flags);
                s_last_fit_w = g_fit_outer_w;
                s_last_fit_h = g_fit_outer_h;
            }
        }
    }

    VoidFx::Shutdown();
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(L"VoidConfigSurface", hInstance);

    return 0;
}

bool CreateDeviceD3D(HWND hWnd) {
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2,
            D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D() {
    CleanupRenderTarget();
    if (g_pSwapChain) {
        g_pSwapChain->Release();
        g_pSwapChain = NULL;
    }
    if (g_pd3dDeviceContext) {
        g_pd3dDeviceContext->Release();
        g_pd3dDeviceContext = NULL;
    }
    if (g_pd3dDevice) {
        g_pd3dDevice->Release();
        g_pd3dDevice = NULL;
    }
}

void CreateRenderTarget() {
    ID3D11Texture2D* pBackBuffer = NULL;
    if (FAILED(g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer))))
        return;
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget() {
    if (g_mainRenderTargetView) {
        g_mainRenderTargetView->Release();
        g_mainRenderTargetView = NULL;
    }
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg) {
    case WM_NCHITTEST: {
        LRESULT hit = ::DefWindowProc(hWnd, msg, wParam, lParam);
        if (hit == HTCLIENT) {
            POINT pt = { (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam) };
            ::ScreenToClient(hWnd, &pt);
            // Перетаскивание за верхнюю полосу (~как шапка ImGui). Контент панели ниже ~48 px.
            if (pt.y < 38)
                return HTCAPTION;
        }
        return hit;
    }
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE)
            ::PostQuitMessage(0);
        break;
    case WM_GETMINMAXINFO: {
        LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
        lpMMI->ptMinTrackSize.x = 820;
        lpMMI->ptMinTrackSize.y = 420;
        return 0;
    }
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED) {
            UINT w = (UINT)LOWORD(lParam);
            UINT h = (UINT)HIWORD(lParam);
            if (w > 0 && h > 0) {
            CleanupRenderTarget();
                g_pSwapChain->ResizeBuffers(0, w, h, DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
            }
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}
