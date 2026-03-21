# Задания anti-CTF

Два таска под Windows: сейф **ImGui Vault** и **Timing Gate** (ритм + FSM).

Ссылки:
- Условия для людей, которые решают: [CTF_TASK_STATEMENTS.md](CTF_TASK_STATEMENTS.md)
- Короткая версия под Word: [ANTIctf.md](ANTIctf.md)
- Ответы для препода (не раздавать участникам): [TEACHER_VERIFICATION.md](TEACHER_VERIFICATION.md)
- Что сдавал и зачем: [ANTI_CTF_SUBMISSION.md](ANTI_CTF_SUBMISSION.md)
- Готовые zip с exe: папка [Deliverables/](Deliverables/)

Код:
- `Source/ImGuiVaultFresh/` — сейф
- `Source/TimingGateFSM/` — тайминг-таск
- `Source/SharedAssets/` — шрифт, фон, rc для вшивания в exe

Репо: https://github.com/SeatSinister/anti-ctf-tasks

---

### Откуда исходники

Нужны только папки `Source/...` выше. То, что CMake и Visual Studio накидывают (vcxproj, CMakeCache, `_deps`, Debug/Release) — это сборка, её в git не кладут, у каждого своя появится после cmake.

Скачать проект: на GitHub **Code → Download ZIP**.
