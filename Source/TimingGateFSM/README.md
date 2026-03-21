## Timing Gate FSM (сложный anti-CTF таск #2)

GUI-версия на Dear ImGui (единый стиль с `ImGuiVaultFresh`), без проблем кодировки консоли.

Исходники: `main.cpp` (точка входа), `app.h` / `app.cpp` (логика).

Проверяется:
- корректная **последовательность** цифр (см. условие в [`CTF_TASK_STATEMENTS.md`](../../CTF_TASK_STATEMENTS.md)),
- корректный **ритм** (тайминги между четырьмя нажатиями после старта сессии).

Если последовательность верная, но ритм неверный, возможен сценарий «почти успех» с **фейковым** флагом.

Эталон проверки для преподавателя — [`TEACHER_VERIFICATION.md`](../../TEACHER_VERIFICATION.md) (спойлеры).

### Сборка (Windows)

```bat
cmake -S "Source\TimingGateFSM" -B "Source\TimingGateFSM\build"
cmake --build "Source\TimingGateFSM\build" --config Release --target timing_gate
```

### Запуск (Windows)

```bat
.\Source\TimingGateFSM\build\Release\timing_gate.exe
```

### Сборка/запуск (Linux)

```bash
cmake -S Source/TimingGateFSM -B Source/TimingGateFSM/build -DCMAKE_BUILD_TYPE=Release
cmake --build Source/TimingGateFSM/build --target timing_gate
./Source/TimingGateFSM/build/timing_gate
```

### Почему это anti-LLM

- Нельзя решить только статическим чтением PIN.
- Нужен правильный ритм реальных нажатий.
- Есть обманка: “почти успех” с фейковым флагом.
