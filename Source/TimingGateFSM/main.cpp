#include "app.h"
#ifdef _WIN32
#include <windows.h>
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) { return AppMain(); }
#else
int main() { return AppMain(); }
#endif
