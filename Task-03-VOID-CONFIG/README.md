# VOID CONFIG (CTF Task)

Новый интерфейс: минималистичное окно конфигуратора. Тема: тёмный фон, бирюзово-бирюзовые акценты.

## Сборка

1. Открыть `imgui dx11 3.sln` в Visual Studio
2. Собрать (требуется DirectX SDK / Windows SDK)

## CTF-логика

Один экран: Enable, FOV, Mode, кнопка **Apply Config**.

- **Секретная комбинация (VOID CONFIG v2.1):** Enable **ON**, FOV **271**, Mode **Полный**
- Верно → попап с `flqg{eclipse_config_vqlitqtet}`
- Неверно → «Invalid config» + фейковый флаг

---

Project uses default C++ 14 and Unicode.
