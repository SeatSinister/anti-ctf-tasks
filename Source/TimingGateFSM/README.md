## Timing Gate FSM (сложный anti-CTF таск #2)

GUI-версия на Dear ImGui (единый стиль с `ImGuiVaultFresh`), без проблем кодировки консоли.

Проверяется:
- корректная последовательность PIN (`2718`),
- корректный ритм (тайминги между 4 нажатиями).

Если PIN верный, но ритм неверный, показывается фейковый флаг.

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
