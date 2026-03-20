## ImGuiVaultFresh

CTF-приложение "ImGui Vault" на `Dear ImGui + GLFW + OpenGL3`.

### Сборка на Windows

```bat
cmake -S "Source\ImGuiVaultFresh" -B "Source\ImGuiVaultFresh\build"
cmake --build "Source\ImGuiVaultFresh\build" --config Release --target vault_fresh
```

### Запуск на Windows

```bat
.\Source\ImGuiVaultFresh\build\Release\vault_fresh.exe
```

### Сборка/запуск на Linux

```bash
cmake -S Source/ImGuiVaultFresh -B Source/ImGuiVaultFresh/build -DCMAKE_BUILD_TYPE=Release
cmake --build Source/ImGuiVaultFresh/build --config Release --target vault_fresh
./Source/ImGuiVaultFresh/build/vault_fresh
```

### Быстрая проверка задания

1. На прогресс-баре видны очень бледные подсказки (alpha = 25).
2. `1337` -> всплывает окно с ложными данными.
3. `1957` -> всплывает окно с настоящими зашифрованными байтами флага.
4. Любой другой PIN уменьшает попытки; при нуле приложение закрывается после подтверждения popup.

### Проверка на `strings` (Linux/MinGW)

```bash
strings vault_fresh | rg "XOR key|format: AntiKpk|PIN: 1337"
```

Подсказки хранятся XOR-обфусцированно и декодируются в рантайме.

