# Shared assets (Windows embed)

Файлы в этой папке линкуются в `.exe` как `RCDATA` через `embed.rc.in` (см. CMake в `ImGuiVaultFresh` и `TimingGateFSM`).

- `Meltaface regular.ttf`
- `background_2.svg`

Без них конфигурация CMake завершится с ошибкой — положите копии сюда перед сборкой.
