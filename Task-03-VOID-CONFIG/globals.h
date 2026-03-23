

static bool checkbox = false;
static bool checkbox2 = false;
static int slider = 0;

static bool i[5] = {false, false, false, false, false};

static int item = 0;

// Декой: отображается в UI, на проверку конфига не влияет (анти-AI: лишние степени свободы)
static int noise_floor = 0;

static const char* items[5] = {"Выкл.", "Фаза 1", "Фаза 2", "Фаза 3", "Полный"};

static char text[256] = "";

static int key;

static int key2;

static float color[4] = {0.0f, 0.0f, 0.0f, 1.0f};
DWORD picker_flags = ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaPreview;
