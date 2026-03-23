#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <cstdio>
#include <cstring>

static constexpr const char* WIN_TITLE = "Eclipse Config";
static constexpr const char* RU_RAGE = "Rage Aim";
static constexpr const char* RU_LEGIT = "Legit";
static constexpr const char* RU_MISC = "Misc";
static constexpr const char* RU_APPLY = "Apply Config";
static constexpr const char* RU_BAD = "Invalid config";
static constexpr const char* RU_HINT = "Config hash mismatch";

static constexpr uint8_t kXor = 0x5A;
static std::string Dec(const uint8_t* p, size_t n) {
    std::string s;
    s.resize(n);
    for (size_t i = 0; i < n; ++i) s[i] = static_cast<char>(p[i] ^ kXor);
    return s;
}

static constexpr uint8_t kFake[] = {
    0x3b,0x28,0x39,0x32,0x33,0x2c,0x3f,0x7a,0x39,0x32,0x3f,0x39,0x31,0x29,0x2f,0x37,0x7a,
    0x37,0x33,0x29,0x37,0x3b,0x2e,0x39,0x32,0x60,0x7a
};
// XOR 0x5A: "flqg{eclipse_config_vqlitqtet}"
static constexpr uint8_t kReal[] = {
    0x3c,0x36,0x2b,0x3d,0x21,0x3f,0x39,0x36,0x33,0x2a,0x29,0x3f,0x05,0x39,0x35,0x34,
    0x3c,0x33,0x3d,0x05,0x2c,0x2b,0x36,0x33,0x2e,0x2b,0x2e,0x3f,0x2e,0x27
};

static int AppMain() {
    if (!glfwInit()) return 1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* w = glfwCreateWindow(700, 500, WIN_TITLE, nullptr, nullptr);
    if (!w) return 2;
    glfwMakeContextCurrent(w);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.Fonts->AddFontDefault();

    ImGui::StyleColorsDark();
    ImGuiStyle& st = ImGui::GetStyle();
    st.Colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.08f, 0.10f, 1.0f);
    st.Colors[ImGuiCol_ChildBg] = ImVec4(0.10f, 0.10f, 0.12f, 1.0f);
    st.Colors[ImGuiCol_Button] = ImVec4(0.96f, 0.16f, 0.57f, 0.8f);
    st.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.98f, 0.25f, 0.65f, 1.0f);
    st.Colors[ImGuiCol_ButtonActive] = ImVec4(0.90f, 0.10f, 0.50f, 1.0f);
    st.Colors[ImGuiCol_Header] = ImVec4(0.96f, 0.16f, 0.57f, 0.4f);
    st.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.96f, 0.16f, 0.57f, 0.6f);
    st.Colors[ImGuiCol_HeaderActive] = ImVec4(0.96f, 0.16f, 0.57f, 0.8f);
    st.WindowRounding = 12.0f;
    st.ChildRounding = 8.0f;
    st.FrameRounding = 6.0f;

    ImGui_ImplGlfw_InitForOpenGL(w, true);
    ImGui_ImplOpenGL3_Init(nullptr);

    enum Tab { Rage, Legit, Misc };
    Tab tab = Rage;
    bool enable = false;
    int sliderVal = 0;
    int comboIdx = 0;
    const char* items[] = { "Off", "Phase 1", "Phase 2", "Phase 3", "Full" };
    bool popBad = false, popOk = false;
    const std::string fakeStr = Dec(kFake, sizeof(kFake));
    const std::string realStr = Dec(kReal, sizeof(kReal));

    const bool needEnable = true;
    const int needSlider = 271;
    const int needCombo = 3;
    const int needTab = Rage;

    while (!glfwWindowShouldClose(w)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(50, 30), ImGuiCond_Once);
        ImGui::SetNextWindowSize(ImVec2(600, 440), ImGuiCond_Once);
        ImGui::Begin("Eclipse", nullptr, ImGuiWindowFlags_NoCollapse);

        ImGui::TextColored(ImVec4(0.96f, 0.16f, 0.57f, 1.0f), "ECLIPSE");
        ImGui::SameLine(500);
        ImGui::TextDisabled("flux1337");
        ImGui::Separator();

        if (ImGui::Button(RU_RAGE, ImVec2(120, 0))) tab = Rage;
        ImGui::SameLine();
        if (ImGui::Button(RU_LEGIT, ImVec2(120, 0))) tab = Legit;
        ImGui::SameLine();
        if (ImGui::Button(RU_MISC, ImVec2(120, 0))) tab = Misc;
        ImGui::Spacing();

        ImGui::BeginChild("cfg", ImVec2(0, 300), true);

        if (tab == Rage) {
            ImGui::Checkbox("Enable", &enable);
            ImGui::SliderInt("FOV", &sliderVal, 0, 360, "%d");
            ImGui::Combo("Mode", &comboIdx, items, IM_ARRAYSIZE(items));
        } else if (tab == Legit) {
            ImGui::Text("(empty)");
        } else {
            ImGui::Text("(empty)");
        }

        ImGui::EndChild();
        ImGui::Spacing();

        if (ImGui::Button(RU_APPLY, ImVec2(200, 35))) {
            bool ok = (tab == needTab && enable == needEnable && sliderVal == needSlider && comboIdx == needCombo);
            if (ok)
                popOk = true;
            else
                popBad = true;
        }

        if (popBad) { ImGui::OpenPopup("##bad"); popBad = false; }
        if (popOk) { ImGui::OpenPopup("##ok"); popOk = false; }

        if (ImGui::BeginPopupModal("##bad", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("%s", RU_BAD);
            ImGui::TextWrapped("%s: %s", RU_HINT, fakeStr.c_str());
            if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }
        if (ImGui::BeginPopupModal("##ok", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.4f, 1.0f), "Config OK");
            ImGui::Text("flag: %s", realStr.c_str());
            if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }

        ImGui::End();

        ImGui::Render();
        int cw = 0, ch = 0;
        glfwGetFramebufferSize(w, &cw, &ch);
        glViewport(0, 0, cw, ch);
        glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(w);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(w);
    glfwTerminate();
    return 0;
}

#ifdef _WIN32
#include <windows.h>
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) { return AppMain(); }
#else
int main() { return AppMain(); }
#endif
