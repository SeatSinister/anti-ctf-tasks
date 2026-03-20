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

#include <cmath>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>

// ===============================
// Русские строки в Unicode-escape (чтобы не зависеть от кодировки файла)
// ===============================
static constexpr const char* RU_WINDOW_TITLE = u8"\u0421\u0435\u0439\u0444 Vault";
static constexpr const char* RU_ATTEMPTS_FMT = u8"\u041e\u0441\u0442\u0430\u043b\u043e\u0441\u044c \u043f\u043e\u043f\u044b\u0442\u043e\u043a: %d";
static constexpr const char* RU_PIN_LABEL = u8"PIN-\u043a\u043e\u0434";
static constexpr const char* RU_PIN_HELP = u8"\u0412\u0432\u0435\u0434\u0438\u0442\u0435 4 \u0446\u0438\u0444\u0440\u044b";
static constexpr const char* RU_OPEN = u8"\u041e\u0442\u043a\u0440\u044b\u0442\u044c";
static constexpr const char* RU_LOCKED = u8"\U0001F512 \u0417\u0410\u041a\u0420\u042b\u0422\u041e";
static constexpr const char* RU_OPENED = u8"\U0001F513 \u041e\u0422\u041a\u0420\u042b\u0422\u041e";
static constexpr const char* RU_WRONG = u8"\u041d\u0435\u0432\u0435\u0440\u043d\u044b\u0439 \u043a\u043e\u0434!";
static constexpr const char* RU_FAKE_TITLE = u8"\u26A0\uFE0F \u0414\u0430\u043d\u043d\u044b\u0435 \u043f\u043e\u0432\u0440\u0435\u0436\u0434\u0435\u043d\u044b";
static constexpr const char* RU_FAKE_DATA = u8"\u0417\u0430\u0448\u0438\u0444\u0440\u043e\u0432\u0430\u043d\u043e: %s";
static constexpr const char* RU_FAKE_DESC = u8"\u041a\u043b\u044e\u0447 \u0434\u043b\u044f XOR \u0442\u044b \u0432\u0438\u0434\u0435\u043b \u043d\u0430 \u043f\u0440\u043e\u0433\u0440\u0435\u0441\u0441-\u0431\u0430\u0440\u0435. \u0420\u0430\u0441\u0448\u0438\u0444\u0440\u0443\u0439...";
static constexpr const char* RU_OPENED_TITLE = u8"\u2705 \u0414\u043e\u0441\u0442\u0443\u043f \u0440\u0430\u0437\u0440\u0435\u0448\u0435\u043d!";
static constexpr const char* RU_OPENED_DATA = u8"\u041d\u0430\u0441\u0442\u043e\u044f\u0449\u0438\u0439 \u0444\u043b\u0430\u0433 (\u0437\u0430\u0448\u0438\u0444\u0440\u043e\u0432\u0430\u043d): %s";
static constexpr const char* RU_OPENED_DESC_FMT = u8"\u0420\u0430\u0441\u0448\u0438\u0444\u0440\u0443\u0439 XOR-\u043e\u043c \u0441 \u043a\u043b\u044e\u0447\u043e\u043c 0x%02X";
static constexpr const char* RU_LOCK_TITLE = u8"\u26A0\uFE0F \u0421\u0418\u0421\u0422\u0415\u041c\u0410 \u0417\u0410\u0411\u041b\u041e\u041a\u0418\u0420\u041e\u0412\u0410\u041d\u0410";
static constexpr const char* RU_LOCK_DESC = u8"\u0410\u0432\u0430\u0440\u0438\u0439\u043d\u043e\u0435 \u0437\u0430\u0432\u0435\u0440\u0448\u0435\u043d\u0438\u0435 \u0447\u0435\u0440\u0435\u0437 %.1f \u0441...";
static constexpr const char* CTF_TASK_NAME = "# ImGui Vault";

// ===============================
// Обфускация строк (чтобы их нельзя было просто вытащить через strings)
// ===============================
static constexpr uint8_t kKey = 0x5A;
static std::string Dec(const uint8_t* e, size_t n)
{
    std::string s;
    s.resize(n);
    for (size_t i = 0; i < n; ++i) s[i] = static_cast<char>(e[i] ^ kKey);
    return s;
}

static constexpr uint8_t kSWindow[] = { 0x09, 0x3F, 0x33, 0x3C, 0x7A, 0x0C, 0x3B, 0x2F, 0x36, 0x2E };
static constexpr uint8_t kSAttempts[] = { 0x15, 0x29, 0x2E, 0x3B, 0x36, 0x35, 0x29, 0x7D, 0x7A, 0x2A, 0x35, 0x2A, 0x23, 0x2E, 0x35, 0x31, 0x60, 0x7A };
static constexpr uint8_t kSOpen[] = { 0x15, 0x2E, 0x31, 0x28, 0x23, 0x2E, 0x7D };
static constexpr uint8_t kSHintL[] = { 0x02, 0x15, 0x08, 0x7A, 0x31, 0x3F, 0x23, 0x7A, 0x67, 0x7A, 0x6A, 0x22, 0x6E, 0x68 };
static constexpr uint8_t kSHintM[] = { 0x3C, 0x35, 0x28, 0x37, 0x3B, 0x2E, 0x60, 0x7A, 0x1B, 0x34, 0x2E, 0x33, 0x11, 0x2A, 0x31 };
static constexpr uint8_t kSHintR[] = { 0x0A, 0x13, 0x14, 0x60, 0x7A, 0x6B, 0x69, 0x69, 0x6D };
static constexpr uint8_t kSHintLAlt[] = { 0x02, 0x15, 0x08, 0x7A, 0x31, 0x3F, 0x23, 0x7A, 0x67, 0x7A, 0x6A, 0x22, 0x68, 0x6E };
static constexpr uint8_t kSHintMAlt[] = { 0x3C, 0x35, 0x28, 0x37, 0x3B, 0x2E, 0x60, 0x7A, 0x08, 0x3F, 0x2E, 0x28, 0x23 };
static constexpr uint8_t kSHintRAlt[] = { 0x0A, 0x13, 0x14, 0x60, 0x7A, 0x6E, 0x68, 0x6E, 0x68 };

// Набор PIN-профилей: после каждого сбоя выбирается новый профиль.
static constexpr int kFakePins[] = { 1337, 4242, 8080, 2718, 5150, 6060, 7331, 9901 };
static constexpr int kRealPins[] = { 1957, 1492, 1789, 1066, 1917, 2049, 1221, 1605 };
static constexpr uint8_t kCrashKeys[] = { 0x24, 0x2A, 0x31, 0x17, 0x24, 0x2A, 0x31, 0x17 };
static constexpr uint8_t kSLocked[] = { 0x00, 0x1B, 0x11, 0x08, 0x03, 0x0E, 0x15 };
static constexpr uint8_t kSWrong[] = { 0x14, 0x3F, 0x2C, 0x3F, 0x28, 0x34, 0x23, 0x33, 0x7A, 0x31, 0x35, 0x3E, 0x7B };
static constexpr uint8_t kSFakeTitle[] = { 0x1E, 0x35, 0x29, 0x2E, 0x2F, 0x2A, 0x7A, 0x2A, 0x35, 0x36, 0x2F, 0x39, 0x32, 0x3F, 0x34, 0x76, 0x7A, 0x34, 0x35, 0x7A, 0x3E, 0x3B, 0x34, 0x34, 0x23, 0x3F, 0x7A, 0x2A, 0x35, 0x2C, 0x28, 0x3F, 0x20, 0x32, 0x3E, 0x3F, 0x34, 0x23 };
static constexpr uint8_t kSFakeData[] = { 0x00, 0x3B, 0x29, 0x32, 0x33, 0x3C, 0x28, 0x35, 0x2C, 0x3B, 0x34, 0x34, 0x23, 0x3F, 0x7A, 0x3E, 0x3B, 0x34, 0x34, 0x23, 0x3F, 0x60, 0x7A };
static constexpr uint8_t kSFakeDesc[] = { 0x11, 0x36, 0x23, 0x2F, 0x39, 0x32, 0x7A, 0x3E, 0x36, 0x23, 0x3B, 0x7A, 0x02, 0x15, 0x08, 0x7A, 0x2E, 0x23, 0x7A, 0x2F, 0x20, 0x32, 0x3F, 0x7A, 0x2C, 0x33, 0x3E, 0x3F, 0x36, 0x74, 0x7A, 0x08, 0x3B, 0x29, 0x29, 0x32, 0x33, 0x3C, 0x28, 0x2F, 0x33, 0x7A, 0x33, 0x7A, 0x2A, 0x35, 0x36, 0x2F, 0x39, 0x32, 0x33, 0x29, 0x32, 0x74, 0x74, 0x74, 0x7A, 0x39, 0x32, 0x2E, 0x35, 0x77, 0x2E, 0x35 };
static constexpr uint8_t kSOpenTitle[] = { 0x09, 0x1F, 0x13, 0x1C, 0x7A, 0x15, 0x0E, 0x11, 0x08, 0x03, 0x0E, 0x7B };
static constexpr uint8_t kSOpenData[] = { 0x14, 0x3B, 0x29, 0x2E, 0x35, 0x23, 0x3B, 0x29, 0x32, 0x39, 0x32, 0x33, 0x33, 0x7A, 0x3C, 0x36, 0x3B, 0x3D, 0x7A, 0x72, 0x20, 0x3B, 0x29, 0x32, 0x33, 0x3C, 0x28, 0x35, 0x2C, 0x3B, 0x34, 0x73, 0x60, 0x7A };
static constexpr uint8_t kSOpenDesc[] = { 0x08, 0x3B, 0x29, 0x29, 0x32, 0x33, 0x3C, 0x28, 0x2F, 0x33, 0x7A, 0x02, 0x15, 0x08, 0x77, 0x35, 0x37, 0x7A, 0x29, 0x7A, 0x31, 0x36, 0x23, 0x2F, 0x39, 0x32, 0x35, 0x37, 0x7A, 0x6A, 0x22, 0x6E, 0x68 };
static constexpr uint8_t kSOK[] = { 0x15, 0x11 };

static int FilterDigits(ImGuiInputTextCallbackData* d)
{
    if (d->EventFlag == ImGuiInputTextFlags_CallbackCharFilter)
        return (d->EventChar >= '0' && d->EventChar <= '9') ? 0 : 1;
    return 0;
}

static std::string HexLine(const unsigned char* bytes, size_t n)
{
    std::string out;
    out.reserve(n * 3);
    char b[8]{};
    for (size_t i = 0; i < n; ++i)
    {
        if (i) out += ' ';
        std::snprintf(b, sizeof(b), "%02x", static_cast<unsigned>(bytes[i]));
        out += b;
    }
    return out;
}

static std::string HexBlock(const unsigned char* bytes, size_t n, size_t perLine)
{
    std::string out;
    char b[8]{};
    for (size_t i = 0; i < n; ++i)
    {
        if (i)
            out += ((i % perLine) == 0) ? '\n' : ' ';
        std::snprintf(b, sizeof(b), "%02x", static_cast<unsigned>(bytes[i]));
        out += b;
    }
    return out;
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

    // Рендерим SVG в RGBA bitmap.
    const auto bmp = doc->renderToBitmap(w, h);
    if (!bmp.valid() || !bmp.data()) return false;
    outTex = MakeTextureRGBA(bmp.data(), static_cast<int>(bmp.width()), static_cast<int>(bmp.height()));
    return outTex != 0;
}

static void SetProceduralIcon(GLFWwindow* window)
{
    // Иконка в стиле второго скрина (белый символ на чёрном фоне).
    const int w = 64, h = 64;
    std::vector<unsigned char> px(static_cast<size_t>(w * h * 4), 0);
    auto put = [&](int x, int y, unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
        if (x < 0 || y < 0 || x >= w || y >= h) return;
        size_t i = static_cast<size_t>((y * w + x) * 4);
        px[i + 0] = r; px[i + 1] = g; px[i + 2] = b; px[i + 3] = a;
    };
    const float cx = 32.0f, cy = 32.0f;
    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            const float dx = x - cx, dy = y - cy;
            const float d = std::sqrt(dx * dx + dy * dy);
            const float m = std::abs(dx) + std::abs(dy);
            if ((d > 24.0f && d < 25.6f) || (d > 18.5f && d < 19.8f) || (m > 9.2f && m < 10.6f))
                put(x, y, 245, 245, 245, 255);
        }
    }
    for (int y = 3; y <= 11; ++y)
    {
        int t = y - 3;
        put(32 - t, y, 245, 245, 245, 255);
        put(32 + t, y, 245, 245, 245, 255);
    }
    for (int y = 53; y <= 61; ++y)
    {
        int t = 61 - y;
        put(32 - t, y, 245, 245, 245, 255);
        put(32 + t, y, 245, 245, 245, 255);
    }
    GLFWimage icon{};
    icon.width = w;
    icon.height = h;
    icon.pixels = px.data();
    glfwSetWindowIcon(window, 1, &icon);
}

static int AppMain()
{
    if (!glfwInit()) return 1;
    int crashCount = 0;
    {
        std::ifstream markerIn("vault.dat");
        if (!markerIn.good())
        {
            std::ofstream markerOut("vault.dat", std::ios::trunc);
            markerOut << 0;
        }
        else
        {
            markerIn >> crashCount;
            if (markerIn.fail() || crashCount < 0) crashCount = 0;
        }
    }
    const bool wasCrashed = (crashCount > 0);
    const int profileCount = static_cast<int>(sizeof(kFakePins) / sizeof(kFakePins[0]));
    int profileIndex = 0;
    if (wasCrashed) profileIndex = 1 + ((crashCount - 1) % (profileCount - 1));

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(920, 650, RU_WINDOW_TITLE, nullptr, nullptr);
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
    // Единый шрифт для ВСЕГО интерфейса.
    if (io.Fonts->AddFontFromFileTTF(u8"C:\\Users\\seat\\Downloads\\Новая папка\\Meltaface\\Meltaface regular.ttf", 24.0f, nullptr, io.Fonts->GetGlyphRangesCyrillic()) == nullptr)
    {
        // Fallback на системный шрифт, если пользовательский путь недоступен.
        if (io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesCyrillic()) == nullptr)
            io.Fonts->AddFontDefault();
    }
    {
        // Для совместимости с 16-битным ImWchar используем базовый диапазон символов.
        static const ImWchar ranges[] = { 0x2600, 0x27BF, 0 };
        ImFontConfig cfg{};
        cfg.MergeMode = true;
        (void)io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\seguiemj.ttf", 18.0f, &cfg, ranges);
    }
#else
    io.Fonts->AddFontDefault();
#endif
    ImGui::StyleColorsDark();

    // Чёрный стиль + КРАСНЫЕ акценты (по запросу).
    ImGuiStyle& st = ImGui::GetStyle();
    st.Colors[ImGuiCol_WindowBg] = wasCrashed ? ImVec4(0.05f, 0.0f, 0.0f, 1) : ImVec4(0, 0, 0, 1);
    st.Colors[ImGuiCol_TitleBg] = wasCrashed ? ImVec4(0.28f, 0.06f, 0.07f, 1.0f) : ImVec4(0.22f, 0.05f, 0.06f, 1.0f);
    st.Colors[ImGuiCol_TitleBgActive] = wasCrashed ? ImVec4(0.36f, 0.08f, 0.09f, 1.0f) : ImVec4(0.30f, 0.06f, 0.07f, 1.0f);
    st.Colors[ImGuiCol_FrameBg] = wasCrashed ? ImVec4(0.18f, 0.04f, 0.05f, 1.0f) : ImVec4(0.14f, 0.04f, 0.05f, 1.0f);
    st.Colors[ImGuiCol_FrameBgHovered] = wasCrashed ? ImVec4(0.26f, 0.07f, 0.08f, 1.0f) : ImVec4(0.22f, 0.06f, 0.07f, 1.0f);
    st.Colors[ImGuiCol_Button] = ImVec4(0.55f, 0.10f, 0.12f, 1.0f);
    st.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.70f, 0.12f, 0.14f, 1.0f);
    st.Colors[ImGuiCol_ButtonActive] = ImVec4(0.78f, 0.15f, 0.17f, 1.0f);
    st.Colors[ImGuiCol_Border] = ImVec4(0.65f, 0.16f, 0.18f, 0.60f);
    st.Colors[ImGuiCol_Separator] = ImVec4(0.60f, 0.14f, 0.16f, 0.55f);
    st.WindowRounding = 10.0f;
    st.FrameRounding = 8.0f;
    st.ChildRounding = 8.0f;
    st.GrabRounding = 8.0f;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(nullptr);

    // Фон из SVG (если файл доступен).
    const char* bgPath = "C:\\Users\\seat\\Downloads\\background_2.svg";
    GLuint bgTex = 0;
    int fbw = 0, fbh = 0;
    glfwGetFramebufferSize(window, &fbw, &fbh);
    (void)LoadSvgToTexture(bgPath, fbw > 0 ? fbw : 920, fbh > 0 ? fbh : 650, bgTex);

    int attempts = 3;
    char pin[5] = {};
    bool popWrong = false, popFake = false, popOpen = false;
    bool popBlocked = false;
    bool lockoutActive = false;
    double lockoutStart = 0.0;
    bool safeOpened = false;
    const int fakePin = kFakePins[profileIndex];
    const int realPin = kRealPins[profileIndex];
    const uint8_t activeXorKey = wasCrashed ? kCrashKeys[profileIndex] : 0x42;
    std::string hintL = wasCrashed ? Dec(kSHintLAlt, sizeof(kSHintLAlt)) : Dec(kSHintL, sizeof(kSHintL));
    if (hintL.size() >= 2)
    {
        char keyHex[3]{};
        std::snprintf(keyHex, sizeof(keyHex), "%02X", activeXorKey);
        hintL[hintL.size() - 2] = keyHex[0];
        hintL[hintL.size() - 1] = keyHex[1];
    }
    const std::string hintM = wasCrashed ? Dec(kSHintMAlt, sizeof(kSHintMAlt)) : Dec(kSHintM, sizeof(kSHintM));
    char hintRBuf[24]{};
    std::snprintf(hintRBuf, sizeof(hintRBuf), "PIN: %04d", fakePin);
    const std::string hintR = hintRBuf;

    static constexpr unsigned char fakeData[] = {
        0x7d, 0x20, 0x2b, 0x3a, 0x38, 0x2e, 0x3f, 0x2d, 0x3b, 0x7d, 0x2a, 0x3f, 0x34, 0x3b, 0x3a,
        0x2b, 0x2e, 0x2c, 0x30, 0x3d, 0x20, 0x3f, 0x2c, 0x3d, 0x28, 0x2b, 0x7d
    };
    static constexpr unsigned char fakeDataAlt[] = {
        0x6a, 0x71, 0x55, 0x5e, 0x40, 0x63, 0x5a, 0x7d, 0x44, 0x58, 0x73, 0x6e, 0x59, 0x5b
    };
    static constexpr unsigned char fakeDataAlt2[] = {
        0x54, 0x4f, 0x67, 0x72, 0x61, 0x5e, 0x49, 0x70, 0x5b, 0x76, 0x43, 0x68, 0x52, 0x4d
    };
    static constexpr unsigned char fakeDataAlt3[] = {
        0x45, 0x63, 0x7a, 0x58, 0x74, 0x4b, 0x5d, 0x6f, 0x41, 0x73, 0x5c, 0x4e, 0x69, 0x57
    };
    static constexpr unsigned char fakeDataAlt4[] = {
        0x78, 0x5a, 0x70, 0x4d, 0x67, 0x59, 0x7c, 0x42, 0x6e, 0x55, 0x49, 0x73, 0x5f, 0x64
    };
    static constexpr unsigned char realFlag[] = {
        0x03, 0x2c, 0x36, 0x2b, 0x09, 0x32, 0x29, 0x39, 0x2b, 0x2f, 0x25, 0x37, 0x2b, 0x1d, 0x34,
        0x2b, 0x31, 0x37, 0x23, 0x2e, 0x1d, 0x31, 0x36, 0x27, 0x23, 0x2e, 0x36, 0x2a, 0x3f
    };
    static constexpr unsigned char realFlagAlt[] = {
        0x65, 0x4a, 0x50, 0x4d, 0x6f, 0x54, 0x4f, 0x5f, 0x50, 0x56, 0x5d, 0x7b, 0x45, 0x43, 0x45,
        0x4d, 0x4a, 0x7b, 0x4c, 0x45, 0x47, 0x4f, 0x41, 0x56, 0x59
    };
    static constexpr unsigned char realFlagAlt2[] = {
        0x6b, 0x44, 0x5e, 0x43, 0x61, 0x5a, 0x41, 0x51, 0x5a, 0x58, 0x45, 0x4c, 0x43, 0x46, 0x4f,
        0x75, 0x59, 0x42, 0x43, 0x4c, 0x5e, 0x75, 0x4f, 0x49, 0x42, 0x45, 0x57
    };
    static constexpr unsigned char realFlagAlt3[] = {
        0x70, 0x5f, 0x45, 0x58, 0x7a, 0x41, 0x5a, 0x4a, 0x43, 0x54, 0x55, 0x6e, 0x45, 0x59, 0x54,
        0x5c, 0x54, 0x6e, 0x52, 0x58, 0x41, 0x59, 0x54, 0x43, 0x4c
    };
    static constexpr unsigned char realFlagAlt4[] = {
        0x56, 0x79, 0x63, 0x7e, 0x5c, 0x67, 0x7c, 0x6c, 0x65, 0x78, 0x63, 0x76, 0x63, 0x7e, 0x79,
        0x70, 0x48, 0x74, 0x78, 0x73, 0x72, 0x64, 0x48, 0x60, 0x7e, 0x79, 0x6a
    };

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Кастомный фон: рисуем SVG как полноэкранную текстуру.
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
            // Лёгкое затемнение поверх, чтобы интерфейс читался.
            bg->AddRectFilled(p0, p1, IM_COL32(0, 0, 0, 125));
            ImGui::End();
        }

        ImGui::SetNextWindowPos(ImVec2(60, 40), ImGuiCond_Once);
        ImGui::SetNextWindowSize(ImVec2(860, 600), ImGuiCond_Once);
        ImGui::Begin("##vault_main", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

        ImGui::TextUnformatted(CTF_TASK_NAME);
        ImGui::Separator();

        ImGui::Text(RU_ATTEMPTS_FMT, attempts);
        ImGui::TextUnformatted(RU_PIN_LABEL);
        ImGui::BeginDisabled(lockoutActive);
        ImGui::PushItemWidth(260);
        ImGui::InputText("##pin", pin, sizeof(pin),
            ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_CallbackCharFilter,
            FilterDigits);
        ImGui::PopItemWidth();
        ImGui::TextUnformatted(RU_PIN_HELP);
        if (ImGui::IsWindowAppearing())
            ImGui::SetKeyboardFocusHere(-1);

        // Резервный ввод через экранную клавиатуру (если не работает фокус/раскладка).
        auto appendDigit = [&](char d)
        {
            const size_t len = std::strlen(pin);
            if (len < 4)
            {
                pin[len] = d;
                pin[len + 1] = '\0';
            }
        };
        auto backspaceDigit = [&]()
        {
            const size_t len = std::strlen(pin);
            if (len > 0)
                pin[len - 1] = '\0';
        };

        const ImVec2 keySize(54, 34);
        const char* row1 = "123";
        const char* row2 = "456";
        const char* row3 = "789";
        for (int i = 0; i < 3; ++i)
        {
            char lbl[2] = { row1[i], 0 };
            if (ImGui::Button(lbl, keySize)) appendDigit(lbl[0]);
            if (i < 2) ImGui::SameLine();
        }
        for (int i = 0; i < 3; ++i)
        {
            char lbl[2] = { row2[i], 0 };
            if (ImGui::Button(lbl, keySize)) appendDigit(lbl[0]);
            if (i < 2) ImGui::SameLine();
        }
        for (int i = 0; i < 3; ++i)
        {
            char lbl[2] = { row3[i], 0 };
            if (ImGui::Button(lbl, keySize)) appendDigit(lbl[0]);
            if (i < 2) ImGui::SameLine();
        }
        if (ImGui::Button("C", keySize)) pin[0] = '\0';
        ImGui::SameLine();
        if (ImGui::Button("0", keySize)) appendDigit('0');
        ImGui::SameLine();
        if (ImGui::Button(u8"←", keySize)) backspaceDigit();

        if (ImGui::Button(RU_OPEN, ImVec2(150, 0)))
        {
            const int v = std::strlen(pin) ? std::atoi(pin) : -1;
            if (v == fakePin) popFake = true;
            else if (v == realPin) { popOpen = true; safeOpened = true; }
            else
            {
                attempts--;
                if (attempts <= 0)
                {
                    attempts = 0;
                    lockoutActive = true;
                    popBlocked = true;
                    lockoutStart = glfwGetTime();
                    std::ofstream markerOut("vault.dat", std::ios::trunc);
                    markerOut << (crashCount + 1);
                }
                else
                {
                    popWrong = true;
                }
            }
            pin[0] = '\0';
        }
        ImGui::EndDisabled();

        const float p = static_cast<float>(std::strlen(pin)) / 4.0f;
        char pbuf[16]{};
        std::snprintf(pbuf, sizeof(pbuf), "%d%%", static_cast<int>(p * 100.0f + 0.5f));
        ImGui::ProgressBar(p, ImVec2(520, 0), pbuf);

        ImDrawList* dl = ImGui::GetWindowDrawList();
        const ImVec2 bmin = ImGui::GetItemRectMin();
        const ImVec2 bmax = ImGui::GetItemRectMax();
        const float bw = bmax.x - bmin.x;
        const ImU32 pale = IM_COL32(255, 255, 255, 25);
        dl->AddText(ImVec2(bmin.x + 8, bmin.y + 2), pale, hintL.c_str());
        dl->AddText(ImVec2(bmin.x + bw * 0.5f - 95, bmin.y + 2), pale, hintM.c_str());
        dl->AddText(ImVec2(bmax.x - 120, bmin.y + 2), pale, hintR.c_str());

        ImGui::Spacing();
        ImGui::Dummy(ImVec2(300, 110));
        {
            ImVec2 r0 = ImGui::GetItemRectMin();
            ImVec2 r1 = ImGui::GetItemRectMax();
            dl->AddRect(r0, r1, IM_COL32(230, 230, 230, 255), 10, 0, 2);
            const char* status = safeOpened ? RU_OPENED : RU_LOCKED;
            const ImVec2 ts = ImGui::CalcTextSize(status);
            dl->AddText(ImVec2((r0.x + r1.x - ts.x) * 0.5f, (r0.y + r1.y - ts.y) * 0.5f), IM_COL32(255, 255, 255, 240), status);
        }

        if (popWrong) { ImGui::OpenPopup("##wrong"); popWrong = false; }
        if (popFake) { ImGui::OpenPopup("##fake"); popFake = false; }
        if (popOpen) { ImGui::OpenPopup("##open"); popOpen = false; }
        if (popBlocked) { ImGui::OpenPopup("##blocked"); popBlocked = false; }

        ImGui::SetNextWindowSize(ImVec2(560, 0), ImGuiCond_Appearing);
        if (ImGui::BeginPopupModal("##wrong", nullptr, ImGuiWindowFlags_NoResize))
        {
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + 520.0f);
            ImGui::TextWrapped("%s", RU_WRONG);
            ImGui::PopTextWrapPos();
            if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }
        ImGui::SetNextWindowSize(ImVec2(560, 0), ImGuiCond_Appearing);
        if (ImGui::BeginPopupModal("##fake", nullptr, ImGuiWindowFlags_NoResize))
        {
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + 520.0f);
            ImGui::TextWrapped("%s", RU_FAKE_TITLE);
            ImGui::TextUnformatted(u8"\u0417\u0430\u0448\u0438\u0444\u0440\u043e\u0432\u0430\u043d\u043e:");
            const unsigned char* fakePtr = fakeData;
            size_t fakeSize = sizeof(fakeData);
            if (wasCrashed)
            {
                const int rot = profileIndex % 4;
                if (rot == 0) { fakePtr = fakeDataAlt; fakeSize = sizeof(fakeDataAlt); }
                else if (rot == 1) { fakePtr = fakeDataAlt2; fakeSize = sizeof(fakeDataAlt2); }
                else if (rot == 2) { fakePtr = fakeDataAlt3; fakeSize = sizeof(fakeDataAlt3); }
                else { fakePtr = fakeDataAlt4; fakeSize = sizeof(fakeDataAlt4); }
            }
            const std::string fakeHex = HexBlock(fakePtr, fakeSize, 10);
            ImGui::TextUnformatted(fakeHex.c_str());
            ImGui::TextWrapped("%s", RU_FAKE_DESC);
            ImGui::PopTextWrapPos();
            if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }
        ImGui::SetNextWindowSize(ImVec2(560, 0), ImGuiCond_Appearing);
        if (ImGui::BeginPopupModal("##open", nullptr, ImGuiWindowFlags_NoResize))
        {
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + 520.0f);
            ImGui::TextWrapped("%s", RU_OPENED_TITLE);
            ImGui::TextUnformatted(u8"\u041d\u0430\u0441\u0442\u043e\u044f\u0449\u0438\u0439 \u0444\u043b\u0430\u0433 (\u0437\u0430\u0448\u0438\u0444\u0440\u043e\u0432\u0430\u043d):");
            const unsigned char* realPtr = realFlag;
            size_t realSize = sizeof(realFlag);
            if (wasCrashed)
            {
                const int rot = profileIndex % 4;
                if (rot == 0) { realPtr = realFlagAlt; realSize = sizeof(realFlagAlt); }
                else if (rot == 1) { realPtr = realFlagAlt2; realSize = sizeof(realFlagAlt2); }
                else if (rot == 2) { realPtr = realFlagAlt3; realSize = sizeof(realFlagAlt3); }
                else { realPtr = realFlagAlt4; realSize = sizeof(realFlagAlt4); }
            }
            const std::string realHex = HexBlock(realPtr, realSize, 10);
            ImGui::TextUnformatted(realHex.c_str());
            ImGui::TextWrapped(RU_OPENED_DESC_FMT, activeXorKey);
            ImGui::PopTextWrapPos();
            if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }
        ImGui::SetNextWindowSize(ImVec2(560, 0), ImGuiCond_Appearing);
        if (ImGui::BeginPopupModal("##blocked", nullptr, ImGuiWindowFlags_NoResize))
        {
            const float remain = 2.0f - static_cast<float>(glfwGetTime() - lockoutStart);
            ImGui::TextWrapped("%s", RU_LOCK_TITLE);
            ImGui::Separator();
            ImGui::TextWrapped(RU_LOCK_DESC, remain > 0.0f ? remain : 0.0f);
            ImGui::EndPopup();
        }

        if (lockoutActive && (glfwGetTime() - lockoutStart) >= 2.0)
            std::abort();

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
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    return AppMain();
}
#else
int main()
{
    return AppMain();
}
#endif

