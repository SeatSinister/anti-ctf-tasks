#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <lunasvg.h>
#ifdef _WIN32
#ifdef APIENTRY
#undef APIENTRY
#endif
#include <windows.h>
#endif
#include <GL/gl.h>

#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <string>
#include <vector>

static constexpr const char* WIN_TITLE = "Timing Gate FSM";
static constexpr const char* UI_TITLE = "# Rhythm Forensics";
static constexpr const char* RU_ATTEMPTS = u8"\u041e\u0441\u0442\u0430\u043b\u043e\u0441\u044c \u043f\u043e\u043f\u044b\u0442\u043e\u043a: %d";
static constexpr const char* RU_INPUT = u8"\u0412\u0432\u0435\u0434\u0438\u0442\u0435 \u043a\u043e\u0434";
static constexpr const char* RU_DESC = u8"\u0420\u0435\u0448\u0430\u0435\u0442 \u043d\u0435 \u0441\u0442\u0440\u043e\u043a\u0430, \u0430 \u0440\u0438\u0442\u043c \u043f\u0435\u0440\u0435\u0445\u043e\u0434\u043e\u0432.";
static constexpr const char* RU_START = u8"\u0421\u0442\u0430\u0440\u0442 \u0441\u0435\u0430\u043d\u0441\u0430";
static constexpr const char* RU_CLEAR = u8"\u0421\u0431\u0440\u043e\u0441 \u0432\u0432\u043e\u0434\u0430";
static constexpr const char* RU_CHECK = u8"\u041f\u0440\u043e\u0432\u0435\u0440\u0438\u0442\u044c";
static constexpr const char* RU_WRONG = u8"\u274c \u041d\u0435\u0432\u0435\u0440\u043d\u044b\u0439 \u043f\u0435\u0440\u0435\u0445\u043e\u0434 FSM";
static constexpr const char* RU_LOCK = u8"\u26A0\uFE0F FSM \u0437\u0430\u0431\u043b\u043e\u043a\u0438\u0440\u043e\u0432\u0430\u043d";
static constexpr const char* RU_NEAR = u8"\u041f\u043e\u0447\u0442\u0438 \u0432\u0435\u0440\u043d\u043e, \u043d\u043e \u0440\u0438\u0442\u043c \u043d\u0435 \u043f\u043e\u043f\u0430\u043b";
static constexpr const char* RU_OK = u8"\u0414\u043e\u0441\u0442\u0443\u043f \u0440\u0430\u0437\u0440\u0435\u0448\u0435\u043d";
static constexpr const char* RU_HINT = u8"\u041f\u043e\u0434\u0441\u043a\u0430\u0437\u043a\u0430: \u0447\u0438\u0441\u043b\u043e e \u0438 \u0442\u0440\u0438 \u0444\u0430\u0437\u044b \u0440\u0438\u0442\u043c\u0430.";

static constexpr uint8_t kObfKey = 0x5A;
static std::string Dec(const uint8_t* arr, size_t n)
{
    std::string s;
    s.resize(n);
    for (size_t i = 0; i < n; ++i) s[i] = static_cast<char>(arr[i] ^ kObfKey);
    return s;
}

static GLuint MakeTextureRGBA(const unsigned char* data, int w, int h)
{
    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);
    return tex;
}

static bool LoadSvgToTexture(const char* path, int w, int h, GLuint& outTex)
{
    auto doc = lunasvg::Document::loadFromFile(path);
    if (!doc) return false;
    const auto bmp = doc->renderToBitmap(w, h);
    if (!bmp.valid() || !bmp.data()) return false;
    outTex = MakeTextureRGBA(bmp.data(), static_cast<int>(bmp.width()), static_cast<int>(bmp.height()));
    return outTex != 0;
}

static std::string ExeDir()
{
#ifdef _WIN32
    char buf[1024]{};
    const DWORD n = GetModuleFileNameA(nullptr, buf, static_cast<DWORD>(sizeof(buf)));
    if (n == 0 || n >= sizeof(buf)) return ".";
    return std::filesystem::path(buf).parent_path().string();
#else
    return ".";
#endif
}

static std::string ResolveAsset(const std::string& fileName, const char* fallbackAbs)
{
    const std::filesystem::path p1 = std::filesystem::path(ExeDir()) / fileName;
    if (std::filesystem::exists(p1)) return p1.string();
    const std::filesystem::path p2 = std::filesystem::path(fileName);
    if (std::filesystem::exists(p2)) return p2.string();
    return fallbackAbs ? std::string(fallbackAbs) : std::string();
}

static void SetProceduralIcon(GLFWwindow* window)
{
    // Иконка в стиле синего круглого эмблемного значка.
    const int w = 64, h = 64;
    std::vector<unsigned char> px(static_cast<size_t>(w * h * 4), 0);
    auto put = [&](int x, int y, unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
        if (x < 0 || y < 0 || x >= w || y >= h) return;
        const size_t i = static_cast<size_t>((y * w + x) * 4);
        px[i + 0] = r; px[i + 1] = g; px[i + 2] = b; px[i + 3] = a;
    };

    const float cx = 32.0f, cy = 32.0f;
    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            const float dx = x - cx, dy = y - cy;
            const float d = std::sqrt(dx * dx + dy * dy);
            if (d < 30.0f) put(x, y, 10, 20, 60, 255);
            if (d > 22.0f && d < 24.5f) put(x, y, 40, 230, 255, 255);
            if (d > 17.0f && d < 18.8f) put(x, y, 70, 120, 255, 255);
            if (d > 12.0f && d < 13.4f) put(x, y, 120, 70, 255, 255);
        }
    }
    // Центральный ромб.
    for (int y = 18; y <= 46; ++y)
    {
        const int t = std::abs(32 - y);
        const int half = 10 - (t * 10 / 14);
        for (int x = 32 - half; x <= 32 + half; ++x)
            put(x, y, 30, 220, 255, 255);
    }
    // Оранжевая V-форма.
    for (int i = 0; i < 18; ++i)
    {
        put(22 + i, 22 + i, 255, 150, 30, 255);
        put(42 - i, 22 + i, 255, 150, 30, 255);
        put(22 + i, 23 + i, 255, 120, 10, 255);
        put(42 - i, 23 + i, 255, 120, 10, 255);
    }

    GLFWimage icon{};
    icon.width = w;
    icon.height = h;
    icon.pixels = px.data();
    glfwSetWindowIcon(window, 1, &icon);
}

// "archive checksum mismatch: 9f2a7b13"
static constexpr uint8_t kFakeFlag[] = {
    0x3b,0x28,0x39,0x32,0x33,0x2c,0x3f,0x7a,0x39,0x32,0x3f,0x39,0x31,0x29,0x2f,0x37,0x7a,
    0x37,0x33,0x29,0x37,0x3b,0x2e,0x39,0x32,0x60,0x7a,0x63,0x3c,0x68,0x3b,0x6d,0x38,0x6b,0x69
};
static constexpr uint8_t kRealFlag[] = {
    0x3c,0x36,0x3b,0x3d,0x21,0x2e,0x33,0x37,0x33,0x34,0x3d,0x05,0x29,0x2e,0x3b,0x2e,
    0x3f,0x05,0x37,0x3b,0x39,0x32,0x33,0x34,0x3f,0x05,0x37,0x3b,0x29,0x2e,0x3f,0x28,
    0x27
};

static int AppMain()
{
    if (!glfwInit()) return 1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(920, 650, WIN_TITLE, nullptr, nullptr);
    if (!window) return 2;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    SetProceduralIcon(window);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;

    io.Fonts->Clear();
#ifdef _WIN32
    const std::string meltaPath = ResolveAsset("Meltaface regular.ttf", u8"C:\\Users\\seat\\Downloads\\Новая папка\\Meltaface\\Meltaface regular.ttf");
    if (io.Fonts->AddFontFromFileTTF(meltaPath.c_str(), 24.0f, nullptr, io.Fonts->GetGlyphRangesCyrillic()) == nullptr)
        io.Fonts->AddFontDefault();
#else
    io.Fonts->AddFontDefault();
#endif
    ImGui::StyleColorsDark();
    ImGuiStyle& st = ImGui::GetStyle();
    st.Colors[ImGuiCol_WindowBg] = ImVec4(0, 0, 0, 1);
    st.Colors[ImGuiCol_TitleBg] = ImVec4(0.20f, 0.08f, 0.30f, 1.0f);
    st.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.28f, 0.10f, 0.42f, 1.0f);
    st.Colors[ImGuiCol_FrameBg] = ImVec4(0.12f, 0.07f, 0.19f, 1.0f);
    st.Colors[ImGuiCol_Button] = ImVec4(0.44f, 0.18f, 0.62f, 1.0f);
    st.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.55f, 0.24f, 0.74f, 1.0f);
    st.Colors[ImGuiCol_ButtonActive] = ImVec4(0.62f, 0.30f, 0.82f, 1.0f);
    st.FrameRounding = 8.0f;
    st.WindowRounding = 10.0f;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(nullptr);

    const std::string bgPath = ResolveAsset("background_2.svg", "C:\\Users\\seat\\Downloads\\background_2.svg");
    GLuint bgTex = 0;
    int fbw = 0, fbh = 0;
    glfwGetFramebufferSize(window, &fbw, &fbh);
    (void)LoadSvgToTexture(bgPath.c_str(), fbw > 0 ? fbw : 920, fbh > 0 ? fbh : 650, bgTex);

    const std::array<int, 4> expected = { ((0x2B ^ 0x29) & 0xF), ((0x71 ^ 0x76) & 0xF), ((0x10 ^ 0x11) & 0xF), ((0x38 ^ 0x30) & 0xF) };
    // Проверяем только интервалы МЕЖДУ цифрами: 2->7, 7->1, 1->8.
    const std::array<int, 4> minMs = { 0, 260, 120, 340 };
    const std::array<int, 4> maxMs = { 9999, 520, 280, 700 };

    int attempts = 3;
    bool started = false;
    bool solved = false;
    double prevPress = 0.0;
    std::array<int, 4> typed = { 0, 0, 0, 0 };
    std::array<int, 4> deltas = { 0, 0, 0, 0 };
    int pos = 0;

    bool popWrong = false, popNear = false, popOk = false, popLock = false;
    const std::string fakeFlag = Dec(kFakeFlag, sizeof(kFakeFlag));
    const std::string realFlag = Dec(kRealFlag, sizeof(kRealFlag));

    auto resetRun = [&]()
    {
        started = (attempts > 0);
        solved = false;
        prevPress = glfwGetTime();
        typed = { 0, 0, 0, 0 };
        deltas = { 0, 0, 0, 0 };
        pos = 0;
    };

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (bgTex != 0)
        {
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(io.DisplaySize);
            ImGui::Begin("##bg", nullptr,
                ImGuiWindowFlags_NoDecoration |
                ImGuiWindowFlags_NoInputs |
                ImGuiWindowFlags_NoBringToFrontOnFocus |
                ImGuiWindowFlags_NoSavedSettings);
            ImDrawList* bg = ImGui::GetWindowDrawList();
            const ImVec2 p0 = ImGui::GetWindowPos();
            const ImVec2 p1 = ImVec2(p0.x + io.DisplaySize.x, p0.y + io.DisplaySize.y);
            bg->AddImage((ImTextureID)(intptr_t)bgTex, p0, p1);
            bg->AddRectFilled(p0, p1, IM_COL32(34, 10, 58, 145));
            ImGui::End();
        }

        ImGui::SetNextWindowPos(ImVec2(60, 40), ImGuiCond_Once);
        ImGui::SetNextWindowSize(ImVec2(860, 600), ImGuiCond_Once);
        ImGui::Begin("##timing_main", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
        ImGui::TextUnformatted(UI_TITLE);
        ImGui::Separator();
        ImGui::Text(RU_ATTEMPTS, attempts);
        ImGui::TextUnformatted(RU_DESC);
        ImGui::TextUnformatted(RU_HINT);
        ImGui::Spacing();

        if (ImGui::Button(RU_START, ImVec2(190, 0))) resetRun();
        ImGui::SameLine();
        if (ImGui::Button(RU_CLEAR, ImVec2(190, 0)) && started && !solved)
        {
            typed = { 0, 0, 0, 0 };
            deltas = { 0, 0, 0, 0 };
            pos = 0;
            prevPress = glfwGetTime();
        }
        ImGui::SameLine();
        ImGui::TextUnformatted(started ? u8"\u0421\u0435\u0430\u043d\u0441 \u0430\u043a\u0442\u0438\u0432\u0435\u043d" : u8"\u041d\u0430\u0436\u043c\u0438\u0442\u0435 \u0441\u0442\u0430\u0440\u0442");
        ImGui::Spacing();

        ImGui::TextUnformatted(RU_INPUT);
        const ImVec2 keySize(60, 36);
        ImGui::BeginDisabled(attempts <= 0 || !started || solved);
        for (int i = 1; i <= 9; ++i)
        {
            char label[4]{};
            std::snprintf(label, sizeof(label), "%d", i);
            if (ImGui::Button(label, keySize) && started && !solved && attempts > 0 && pos < 4)
            {
                const double now = glfwGetTime();
                typed[pos] = i;
                deltas[pos] = (pos == 0) ? 0 : static_cast<int>((now - prevPress) * 1000.0);
                prevPress = now;
                pos++;
            }
            if ((i % 3) != 0) ImGui::SameLine();
        }
        if (ImGui::Button("0", keySize) && started && !solved && attempts > 0 && pos < 4)
        {
            const double now = glfwGetTime();
            typed[pos] = 0;
            deltas[pos] = (pos == 0) ? 0 : static_cast<int>((now - prevPress) * 1000.0);
            prevPress = now;
            pos++;
        }
        ImGui::SameLine();
        if (ImGui::Button("C", keySize) && started && pos > 0)
        {
            pos--;
            typed[pos] = 0;
            deltas[pos] = 0;
            prevPress = glfwGetTime();
        }

        if (ImGui::Button(RU_CHECK, ImVec2(180, 0)) && started && !solved && pos == 4)
        {
            bool seqOk = true;
            bool timeOk = true;
            for (int i = 0; i < 4; ++i)
            {
                if (typed[i] != expected[i]) seqOk = false;
                if (i > 0 && (deltas[i] < minMs[i] || deltas[i] > maxMs[i])) timeOk = false;
            }

            if (seqOk && timeOk)
            {
                solved = true;
                popOk = true;
            }
            else if (seqOk && !timeOk)
            {
                popNear = true;
                attempts--;
            }
            else
            {
                popWrong = true;
                attempts--;
            }
            if (attempts <= 0 && !solved) { popLock = true; started = false; }
            typed = { 0, 0, 0, 0 };
            deltas = { 0, 0, 0, 0 };
            pos = 0;
            prevPress = glfwGetTime();
        }
        ImGui::EndDisabled();

        ImGui::Spacing();
        ImGui::Text("Code: %d%d%d%d", typed[0], typed[1], typed[2], typed[3]);
        ImGui::Text("dT: 2->7=%d  7->1=%d  1->8=%d ms", deltas[1], deltas[2], deltas[3]);
        ImGui::TextUnformatted(u8"\u0424\u0430\u0437\u044b \u0440\u0438\u0442\u043c\u0430: \u043c\u0435\u0434\u043b\u0435\u043d\u043d\u043e -> \u0431\u044b\u0441\u0442\u0440\u043e -> \u0434\u043b\u0438\u043d\u043d\u0430\u044f \u043f\u0430\u0443\u0437\u0430");

        if (popWrong) { ImGui::OpenPopup("##wrong"); popWrong = false; }
        if (popNear) { ImGui::OpenPopup("##near"); popNear = false; }
        if (popOk) { ImGui::OpenPopup("##ok"); popOk = false; }
        if (popLock) { ImGui::OpenPopup("##lock"); popLock = false; }

        ImGui::SetNextWindowSize(ImVec2(560, 0), ImGuiCond_Appearing);
        if (ImGui::BeginPopupModal("##wrong", nullptr, ImGuiWindowFlags_NoResize))
        {
            ImGui::TextWrapped("%s", RU_WRONG);
            if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }
        ImGui::SetNextWindowSize(ImVec2(560, 0), ImGuiCond_Appearing);
        if (ImGui::BeginPopupModal("##near", nullptr, ImGuiWindowFlags_NoResize))
        {
            ImGui::TextWrapped("%s", RU_NEAR);
            ImGui::TextWrapped(u8"\u041b\u043e\u0436\u043d\u044b\u0439 \u0430\u0440\u0442\u0435\u0444\u0430\u043a\u0442: %s", fakeFlag.c_str());
            if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }
        ImGui::SetNextWindowSize(ImVec2(560, 0), ImGuiCond_Appearing);
        if (ImGui::BeginPopupModal("##ok", nullptr, ImGuiWindowFlags_NoResize))
        {
            ImGui::TextWrapped("%s", RU_OK);
            ImGui::TextWrapped("flag: %s", realFlag.c_str());
            if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }
        ImGui::SetNextWindowSize(ImVec2(560, 0), ImGuiCond_Appearing);
        if (ImGui::BeginPopupModal("##lock", nullptr, ImGuiWindowFlags_NoResize))
        {
            ImGui::TextWrapped("%s", RU_LOCK);
            if (ImGui::Button("Exit")) glfwSetWindowShouldClose(window, GLFW_TRUE);
            ImGui::EndPopup();
        }

        ImGui::End();

        ImGui::Render();
        int w = 0, h = 0;
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    if (bgTex) glDeleteTextures(1, &bgTex);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) { return AppMain(); }
#else
int main() { return AppMain(); }
#endif
