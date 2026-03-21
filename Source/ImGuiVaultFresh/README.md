## ImGui Vault

Окно-сейф на ImGui + GLFW + OpenGL.

Файлы: `main.cpp` (вход), `app.cpp` / `app.h` (остальное).

Условие таска: [CTF_TASK_STATEMENTS.md](../../CTF_TASK_STATEMENTS.md)  
Как проверять: [TEACHER_VERIFICATION.md](../../TEACHER_VERIFICATION.md)

### Сборка (Windows)

```bat
cmake -S "Source\ImGuiVaultFresh" -B "Source\ImGuiVaultFresh\build"
cmake --build "Source\ImGuiVaultFresh\build" --config Release --target vault_fresh
```

Запуск:

```bat
.\Source\ImGuiVaultFresh\build\Release\vault_fresh.exe
```

### Linux

```bash
cmake -S Source/ImGuiVaultFresh -B Source/ImGuiVaultFresh/build -DCMAKE_BUILD_TYPE=Release
cmake --build Source/ImGuiVaultFresh/build --config Release --target vault_fresh
./Source/ImGuiVaultFresh/build/vault_fresh
```

### Проверка глазами (без конкретных PIN — см. teacher md)

1. На прогресс-баре почти не видно текст.  
2. Один PIN ведёт в левый попап.  
3. Другой — hex флага, XOR ключ из интерфейса.  
4. Нет попыток — блокировка, потом при новом запуске может смениться профиль (`vault.dat`).

Строки в бинарнике замазаны XOR’ом, просто strings мало что даст.
