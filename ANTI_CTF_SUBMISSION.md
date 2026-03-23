# Сдача anti-CTF

Автор: seat. Проект: midnightCrck.

## Тексты для игроков

По просьбе преподавателя: описание и маленькая подсказка по каждому таску — в `CTF_TASK_STATEMENTS.md`.  
Короткая выжимка: `ANTIctf.md`.  
Как проверять решение: `TEACHER_VERIFICATION.md` (не для участников).

## Что сделано в коде

**ImGui Vault** (`Source/ImGuiVaultFresh` → `vault_fresh.exe`): ImGui, GLFW, OpenGL.  
Сейф с PIN, бледные подсказки на прогресс-баре, левый и правый PIN, XOR, лимит попыток, после нуля — выход и запись в `vault.dat`, потом другой профиль.

**Timing Gate** (`Source/TimingGateFSM` → `timing_gate.exe`): то же по стеку.  
Смотрит на последовательность цифр и на паузы между нажатиями. Можно получить фейковый флаг, если ритм не тот.

**VOID CONFIG** (`Task-03-VOID-CONFIG` → `eclipse_config.exe`): DirectX 11 + Dear ImGui.  
Проверка параметров через хэш состояния, ложные ветки (в т.ч. ложный «успех»), настоящий флаг только при корректной комбинации.

Для удобства также есть упрощённый кроссплатформенный вариант: `Source/EclipseConfig`.

## Идея с аудио-спектрограммой

Только набросок: ключ в спектре wav, второй файл с шифром. В репозитории не делал, это отдельная тема.

## Что отправить преподавателю

1. Vault — готовый exe + условия в md.  
2. Timing Gate — готовый exe + те же условия.  
3. VOID CONFIG — готовый exe + условия/райтап/флаги в md.  
4. Спектрограмма — только задумка, без полного кода.

Папки с исходниками:
- `Source/ImGuiVaultFresh`
- `Source/TimingGateFSM`
- `Source/EclipseConfig` (упрощённый Task 3)
- `Task-03-VOID-CONFIG` (основной Task 3)

Deliverables:
- `Deliverables/ImGuiVaultFresh_windows.zip`
- `Deliverables/RhythmForensics_windows.zip`
- `Deliverables/VOID_CONFIG_windows.zip`
