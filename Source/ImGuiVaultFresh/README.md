## ImGuiVaultFresh

CTF-приложение "ImGui Vault" на `Dear ImGui + GLFW + OpenGL3`.

Исходники: `main.cpp` (точка входа), `app.h` / `app.cpp` (логика).

**Текст задания для участников:** [`CTF_TASK_STATEMENTS.md`](../../CTF_TASK_STATEMENTS.md).  
**Проверка (спойлеры):** [`TEACHER_VERIFICATION.md`](../../TEACHER_VERIFICATION.md).

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

### Быстрая проверка задания (без конкретных PIN — см. `TEACHER_VERIFICATION.md`)

1. На прогресс-баре видны очень бледные подсказки (низкая непрозрачность).
2. Ложный PIN из «очевидной» подсказки ведёт к всплывающему окну с отвлекающими данными.
3. Настоящий PIN текущего профиля ведёт к окну с зашифрованными байтами флага (нужен XOR с ключом из UI).
4. Неверные попытки уменьшают лимит; при нуле — блокировка и выход; при следующем запуске возможен другой профиль (`vault.dat`).

### Проверка на `strings` (Linux/MinGW)

```bash
strings vault_fresh | rg "XOR key|format: AntiKpk|PIN: 1337"
```

Подсказки хранятся XOR-обфусцированно и декодируются в рантайме.

