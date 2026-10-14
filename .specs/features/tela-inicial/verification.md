# Tela inicial verification

**Verdict**: PASS
**Profile**: standard
**Diff range**: d702049..5b9a390
**Round**: 2 - scoped
**Verifier**: independent sub-agent (author != verifier)

Round 2 scope: fix commit `0c5a6f6..5b9a390` touches only `tests/test_menu.c`, `tests/test_menu_cli.c`,
`CMakeLists.txt` (one new test registration), `checks.md` (`>` note lines under C3, C5, C6, C10) and this
report. No product code changed (`git diff 0c5a6f6..5b9a390 -- src/` is empty). Round 1 verdict was FAIL
on surviving mutants M3, M4, the unproven Backspace mapping, and Test policy rows 1 and 2.

## Binding sources

carried from 0c5a6f6

| Source | Opened | Contradiction | Uncovered |
| --- | --- | --- | --- |
| none beyond the user's chat answers in plan.md `## Sources` (no design file); step 1 is `ui`-only and has no input here | - | - | - |

## Checks

Proofs verified at 5b9a390: one invocation, `ctest --test-dir build --output-on-failure -V`, 97/97 passed. Every
named test appears individually as `Test #NN ... Passed` (#80-#97 are the 18 tela-inicial tests). The harness
(`tests/harness.h` `run_named`) exits 2 on an unknown name. C12 `git diff --exit-code d702049 -- <pre-existing
test files>` exit 0. Citations refreshed for `tests/test_menu.c` and `tests/test_menu_cli.c`. All other citations
are carried from 0c5a6f6 (those files are untouched by the fix).

| Check | Claim | Proof run | Evidence | Result |
| --- | --- | --- | --- | --- |
| C1 | texts, 5 rects top-to-bottom, no overlap, in window; no game in 3 frames | `menu_layout_and_texts` #80, `app_menu_before_game` #90 exit 0 | `tests/test_menu.c:46` - `strcmp(MENU_TITLE, "Dominó") == 0` (and :47-49); `:56` - `r[i].y >= r[i - 1].y + r[i - 1].h`; `:58` - `!rect_overlaps(r[i], r[j])`; `tests/test_menu_app.c:71-72` - `s.menu_frames == 3`, `!s.started` | PASS |
| C2 | accepts 32,65,126,160,231,255; refuses 31,127,159,256,128512; ç = C3 A7; 17th refused | `menu_type_filter` #81 exit 0 | `tests/test_menu.c:70` - `menu_type(&m, ok[i])`; `:76` - `!menu_type(&m, bad[i])`; `:83` - `m.name[0] == 0xC3 && m.name[1] == 0xA7`; `:89` - `!menu_type(&m, 'z')` | PASS |
| C3 | Backspace `Zé` -> `Z` -> empty -> unchanged | `menu_backspace_utf8` #82, extra `cli_menu_backspace_and_clicks` #97 exit 0 | `tests/test_menu.c:105` - `strcmp(m.name, "Z") == 0`; `:108`, `:110` - `m.name[0] == '\0' && m.chars == 0`; real key: `tests/test_menu_cli.c:175` sends `VK_BACK` after `Anax`, `:180-183` fails unless file is exactly `Ana` | PASS |
| C4 | blank / `   ` -> disabled, click and Enter MENU_NONE | `menu_play_disabled_when_blank` #83 exit 0 | `tests/test_menu.c:121` - `!menu_play_enabled(&m)`; `:124` - `a == MENU_NONE`; `:125` - `menu_enter(&m) == MENU_NONE` | PASS |
| C5 | `  Ana  ` -> MENU_PLAY, name `Ana`; app_run starts game animated, dealing, name in controller | `menu_play_trims_name` #84, `app_menu_play_starts_game` #91, extra #97 exit 0 | `tests/test_menu.c:138-139,142` - `a == MENU_PLAY`, `menu_enter(&m) == MENU_PLAY`, `strcmp(t, "Ana") == 0`; `tests/test_menu_app.c:83-86` - `s.started`, `s.animate`, `s.dealing`, `strcmp(s.player, "Ana") == 0`; real click on Jogar `tests/test_menu_cli.c:176`, file `Ana` at `:180-183` | PASS |
| C6 | click Sair -> MENU_QUIT (empty or filled); app_run returns 0 without game | `menu_quit_click` #85, `app_menu_quit_returns_0` #92, extra #97 exit 0 | `tests/test_menu.c:155` - `a == MENU_QUIT`; `tests/test_menu_app.c:96-98` - `run(...) == 0`, `!s.started`, `s.menu_frames == 2`; real click on Sair `tests/test_menu_cli.c:188`, `:189` - `finish(&pi, 5000) == 0` with no WM_CLOSE (timeout -> TerminateProcess 99) | PASS |
| C7 | View carries `Ana`; name rect centred x=640, y>=560, bottom <=605, no overlap with message | `player_name_above_south_hand` #86 exit 0 | `tests/test_menu.c:169` - `strcmp(v.player_name, "Ana") == 0`; `:174-178` - centre in (639.5,640.5), `n.y >= 560`, `n.y + n.h <= hand[0].y`, `hand[0].y == 605`, `!rect_overlaps(n, msg)` | PASS |
| C8 | name_save creates missing dir, writes exactly `5A C3 A9`; app_run writes exactly `Ana` | `name_save_format` #87, `app_menu_saves_name` #93 exit 0 | `tests/test_menu.c:189` - `name_save("tmp_menu/nova/jogador.txt", "Zé")` after `_rmdir` at :188; `:192-193` - `n == 3` and bytes 0x5A,0xC3,0xA9; `tests/test_menu_app.c:119-120` - `n == 3`, `memcmp(buf, "Ana", 3) == 0` | PASS |
| C9 | file `Zé` -> name_load `Zé`; app_run field prefilled | `name_load_prefills` #88, `app_menu_prefills_name` #94 exit 0 | `tests/test_menu.c:202-203` - `name_load(...)`, `strcmp(out, "Zé") == 0`; `tests/test_menu_app.c:133` - `strcmp(s.seen_name, "Zé") == 0` | PASS |
| C10 | empty name for absent, empty, `Ana\n`, `A\tB`, `80`, `C4 80` (+ `7F`, `C2 80`, `C2 9F` per note); 20 letters -> first 16 | `name_load_rejects_invalid` #89 exit 0 | `tests/test_menu.c:217-218` - absent; `:223-226` table of 8 bad files, `:227-231` - `CHECK(false)` if any accepted; `:236-237` - `strcmp(out, "abcdefghijklmnop") == 0` | PASS |
| C11 | dir-is-a-file -> name_save false; app_run still plays, returns 0 | `app_menu_save_failure_still_plays` #95 exit 0 | `tests/test_menu_app.c:143` - `!name_save("tmp_menu_app/arquivo/jogador.txt", "Ana")`; `:148-150` - `run(...) == 0`, `s.started`, `strcmp(s.player, "Ana") == 0` | PASS |
| C12 | whole suite passes; no pre-existing test file changed since d702049 | `ctest --test-dir build --output-on-failure` 97/97; `git diff --exit-code d702049 -- tests/test_core.c ... tests/core_has_no_raylib.cmake` exit 0 | `100% tests passed out of 97`; diff exit 0 | PASS |
| C13 | real domino.exe, temp APPDATA: `A`,`n`,`a`,Enter -> file `Ana`, exit 0 | `cli_menu_types_and_saves` #96 exit 0 | `tests/test_menu_cli.c:79` - `code == 0`; `:89-90` - `n == 3`, `memcmp(buf, "Ana", 3) == 0` (file written only in the MENU_PLAY branch, `src/ui/app.c:79-83`) | PASS |

## Coverage

Rows the fix touched are verified at 5b9a390; the rest are carried from 0c5a6f6 (no product code changed, so no
member was added). Recomputed from plan AC 1-12, Flow step 3, Surface, Landing door 1/door 2 and the code.

| Set (size) | Recomputed from | Member -> proof | Unproven |
| --- | --- | --- | --- |
| char code ranges typed (5) - carried from 0c5a6f6 | AC 2 | <32 (31) C2 · 32-126 (32,65,126) C2 · 127-159 (127,159) C2 · 160-255 (160,231,255) C2 · >255 (256,128512) C2 | - |
| char ranges rejected in the file (4) - verified at 5b9a390 | AC 10 + door 1 UTF-8 | <32 (`\n`,`\t`) C10 · 127-159 (`7F`, `C2 80`, `C2 9F`) C10 `tests/test_menu.c:224-225` · >255 (`C4 80`) C10 · invalid UTF-8 (`80`) C10 | - (M3 now killed) |
| ignored-file reasons (AC 10) - verified at 5b9a390 | AC 10 | absent C10 · empty C10 · out-of-range chars (row above) · >16 valid -> first 16 C10 | - |
| menu actions (3) - carried from 0c5a6f6 | `MenuAction` in `src/core/menu.h` | MENU_NONE C4 · MENU_PLAY C5 · MENU_QUIT C6 | - |
| menu inputs, decision level (4) - carried from 0c5a6f6 | Flow step 3 | char C2 · Backspace C3 · Enter C4,C5 · click C4,C5,C6 | - |
| menu inputs, real-input mapping in app_run (4) - verified at 5b9a390 | Flow step 3 + `src/ui/app.c:63-76` | char `GetCharPressed` C13, #97 · Enter C13 · Backspace `app.c:66-67` #97 (`test_menu_cli.c:175`) · mouse click `app.c:70-75` #97 Jogar `:176` and Sair `:188-189` | - (M4 and Backspace mutant killed) |
| start-screen elements (AC 1) - carried from 0c5a6f6 | AC 1 | title, label, field, Jogar, Sair C1 (constants + rects; drawing is instrumentation per Test policy row 3) | - (note: the green background on the menu frame has no pixel proof; accepted under row 3) |
| file statuses (4) - carried from 0c5a6f6 | plan Surface | `lido` C9 · `ignorado` C10 · `gravado` C8 · `falha ao gravar` C11 | - |
| door 1 format attributes (6) - carried from 0c5a6f6 | Landing door 1 | path C13 · UTF-8 no BOM C8 · no trailing newline C8 · 1-16 chars C2,C10 · no edge spaces on write C8 · Domino dir created C8 | - |
| door 2 (2) - carried from 0c5a6f6 | Landing door 2 | default false C12 · `main` sets true C13 (killed M5) | - |
| startup config: menu (2 assemblies) - carried from 0c5a6f6 | read directly | `src/ui/main.c:23` `opt.show_menu = true;` -> C13, #97 · tests' assembly `tests/test_menu_app.c:57-59` -> C5,C8,C9,C11 | - |

## Test policy rows

Rows 1 and 2 re-judged at 5b9a390; row 3 carried from 0c5a6f6.

| Row | Files it classifies | Required proof | Expectation met |
| --- | --- | --- | --- |
| Decide, sem fronteira | `src/core/namefile.c` | own level: C8, C9, C10, C11 | yes - every AC 10 rejection range now has a case (`tests/test_menu.c:223-225`); M3 killed |
| Decide, alcançado pela `domino.exe` | `src/core/menu.c`, `src/ui/app.c` (menu loop) | own level C1-C6, C8, C9, C11 · process C13 + `cli_menu_backspace_and_clicks` | yes - process level now drives typing, Enter, Backspace, click Jogar, click Sair and the save; the input-mapping branches the `menu_input` hook skips (`app.c:66-67`, `app.c:70-75`) are proven at the process level; M4 and the Backspace mutant killed |
| Instrumentação (desenho da tela inicial) | `src/ui/draw.c` | none own; layout C1, View C7, enabled flag C4 | yes (carried from 0c5a6f6) |

Swept: carried from 0c5a6f6 - no row resolves to existing code.

Non-blocking finding in `checks.md` (verified at 5b9a390): the three `>` note lines under C3, C5 and C6
(`checks.md:28`, `:36`, `:41`) write the proof as `ctest --test-dir build -R "^cli_menu_backspace_and_clicks  --output-on-failure`.
The closing `$"` is missing, so the quote is unterminated and the command cannot be pasted and run as written.
The test is registered and ran (#97). This does not change any check's `Proof:` line, but the author should fix the text.

## Faults injected

Round 2 faults verified at 5b9a390. They ran in an isolated worktree (scratchpad `wt2`, HEAD 5b9a390) with a separate
build dir `bld`. The real tree's `git status --porcelain` was empty before and after (byte-identical). The worktree was
removed with `git worktree remove --force`, and `git stash` was not used. Round 1 rows M1, M2 and M5 are carried from 0c5a6f6
(their surfaces and proofs are untouched by the fix).

| Mutation | Location | Killed |
| --- | --- | --- |
| M1 `cp <= 126` -> `cp <= 127` in `menu_accepts` (carried from 0c5a6f6) | `src/core/menu.c:21` | yes - `menu_type_filter` failed at `tests/test_menu.c:76` |
| M2 Backspace continuation-byte loop -> `while (0)` (carried from 0c5a6f6) | `src/core/menu.c:46` | yes - `menu_backspace_utf8` failed at `tests/test_menu.c:105` |
| M3 `name_load` uses `cp >= 32 && cp <= 255` instead of `menu_accepts` (re-injected) | `src/core/namefile.c:32` | yes - `name_load_rejects_invalid` failed at `tests/test_menu.c:231` |
| M4 menu-loop mouse block `if (IsMouseButtonPressed(...))` -> `if (0)` (re-injected) | `src/ui/app.c:70` | yes - `cli_menu_backspace_and_clicks` failed at `tests/test_menu_cli.c:183` (file absent) |
| M6 real Backspace handling `if (IsKeyPressed(KEY_BACKSPACE) \|\| IsKeyPressedRepeat(KEY_BACKSPACE))` -> `if (0)` (new) | `src/ui/app.c:66` | yes - `cli_menu_backspace_and_clicks` failed at `tests/test_menu_cli.c:183` (file held `Anax`) |
| M5 `opt.show_menu = true` -> `false` (carried from 0c5a6f6) | `src/ui/main.c:23` | yes - `cli_menu_types_and_saves` failed at `tests/test_menu_cli.c:84` |

M4 was killed by the Jogar half of #97. The Sair half (`tests/test_menu_cli.c:188-189`) was not made to fail on its
own: a broken click mapping is caught first by the Jogar half, and app-level MENU_QUIT handling is covered by #92.

## Gate

`ctest --test-dir build --output-on-failure` at 5b9a390 gave 97 passed, 0 failed (18 tela-inicial tests individually listed
with `-V`). `git diff --exit-code d702049 -- <pre-existing test files>` exited 0.
`validate_verification.py` could not run because Python is not installed on this machine (`python3`/`python` resolve to
the Microsoft Store stub). The script did not gate this verdict.
