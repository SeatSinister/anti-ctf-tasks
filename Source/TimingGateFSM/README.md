## Timing Gate FSM

Та же связка ImGui + GLFW + OpenGL, что и у Vault.

Файлы: `main.cpp`, `app.cpp`, `app.h`.

Нужны правильные цифры и правильные паузы между нажатиями после старта. Подробности в [CTF_TASK_STATEMENTS.md](../../CTF_TASK_STATEMENTS.md).  
Ответы: [TEACHER_VERIFICATION.md](../../TEACHER_VERIFICATION.md).

### Windows

```bat
cmake -S "Source\TimingGateFSM" -B "Source\TimingGateFSM\build"
cmake --build "Source\TimingGateFSM\build" --config Release --target timing_gate
```

```bat
.\Source\TimingGateFSM\build\Release\timing_gate.exe
```

### Linux

```bash
cmake -S Source/TimingGateFSM -B Source/TimingGateFSM/build -DCMAKE_BUILD_TYPE=Release
cmake --build Source/TimingGateFSM/build --target timing_gate
./Source/TimingGateFSM/build/timing_gate
```

Без выдерживания пауз один только реверс по коду не спасёт — нужны реальные нажатия с таймингом.
