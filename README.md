# Anti-CTF tasks

Два практических задания (Windows): **ImGui Vault** и **Rhythm Forensics (Timing Gate FSM)**.

- **Условия для участников:** [`CTF_TASK_STATEMENTS.md`](CTF_TASK_STATEMENTS.md) · кратко для Word: [`ANTIctf.md`](ANTIctf.md)  
- **Проверка (спойлеры):** [`TEACHER_VERIFICATION.md`](TEACHER_VERIFICATION.md)  
- **Сдача / методология:** [`ANTI_CTF_SUBMISSION.md`](ANTI_CTF_SUBMISSION.md)  
- **Готовые архивы:** каталог [`Deliverables/`](Deliverables/)  
- **Сборка:** `Source/ImGuiVaultFresh`, `Source/TimingGateFSM` (CMake). Ассеты для embed: `Source/SharedAssets/`.

Репозиторий: <https://github.com/SeatSinister/anti-ctf-tasks>

---

### Где исходники и что не заливать на GitHub

**Исходный код** лежит только здесь:

- `Source/ImGuiVaultFresh/` — `main.cpp`, `app.cpp`, `app.h`, `CMakeLists.txt`
- `Source/TimingGateFSM/` — то же
- `Source/SharedAssets/` — ресурсы для вшивания в exe (шрифт, SVG, `embed.rc.in`, `resource_ids.h`)

Папка, где после CMake/Visual Studio появляются `.vcxproj`, `CMakeCache.txt`, `_deps`, `.vs`, `Debug`/`Release` — это **каталог сборки**. Его **не коммитят**: он создаётся у каждого на своём ПК заново (`cmake -B build` → `cmake --build build`). В `.gitignore` уже указано `**/build/` и служебные файлы IDE.

**Скачать исходники с GitHub:** зелёная кнопка **Code → Download ZIP** — в архиве будет репозиторий **без** локальной папки `build` (если ты её не закоммитил). Дальше у себя: распаковать → открыть CMakeLists в VS или собрать из консоли по инструкции в `README` каждого проекта.
