## Eclipse Config (Task 3, simplified)

Конфигуратор «чит-меню» в стиле ImGui. Флаг появляется только при **скрытой комбинации** настроек. Anti‑LLM: нужно реально покликать в интерфейсе и подобрать значения, а не только статически анализировать код.

Условие таска: [CTF_TASK_STATEMENTS.md](../../CTF_TASK_STATEMENTS.md)

### Сборка (Windows)

```bat
cmake -S "Source\EclipseConfig" -B "Source\EclipseConfig\build"
cmake --build "Source\EclipseConfig\build" --config Release --target eclipse_config
```

Запуск:

```bat
.\Source\EclipseConfig\build\Release\eclipse_config.exe
```

### Логика

- Вкладка **Rage Aim**: чекбокс Enable, слайдер FOV (0–360), комбо Mode (Off, Phase 1–3, Full).
- Кнопка **Apply Config** — проверяет комбинацию.
- Неверная комбинация → попап «Invalid config» с **фейковым** флагом.
- Верная комбинация → попап с **настоящим** флагом `flqg{eclipse_config_vqlitqtet}`.

**Секретная комбинация (для проверки):**
- Tab: **Rage Aim**
- Enable: **ON**
- FOV: **271**
- Mode: **Full**

Флаги в коде замаскированы XOR 0x5A, `strings` даёт только обфусцированные байты.
