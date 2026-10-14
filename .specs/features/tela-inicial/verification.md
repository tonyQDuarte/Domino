# Tela inicial verification

**Verdict**: FAIL
**Profile**: standard
**Diff range**: d702049..0c5a6f6
**Round**: 1 - full
**Verifier**: independent sub-agent (author != verifier)

## Binding sources

| Source | Opened | Contradiction | Uncovered |
| --- | --- | --- | --- |
| none beyond the user's chat answers in plan.md `## Sources` (no design file); step 1 is `ui`-only and has no input here | - | - | - |

## Checks

All proofs run at HEAD 0c5a6f6 in one invocation (`ctest --test-dir build --output-on-failure`, 96/96 passed), then `ctest -V -L tela-inicial`: every named test below appears individually as `Test #NN ... Passed`, is registered in `CMakeLists.txt` (foreach lists after `# tela-inicial`), and the harness (`tests/harness.h` `run_named`) exits 2 on an unknown name, so no filter can pass empty.

| Check | Claim | Proof run | Evidence | Result |
| --- | --- | --- | --- | --- |
| C1 | texts, 5 rects top-to-bottom, no overlap, in window; no game in 3 frames | `menu_layout_and_texts` #80, `app_menu_before_game` #90 exit 0 | `tests/test_menu.c:46` - `strcmp(MENU_TITLE, "Dominó") == 0` (and :47-49); `tests/test_menu.c:56` - `r[i].y >= r[i - 1].y + r[i - 1].h`; `:58` - `!rect_overlaps(r[i], r[j])`; `tests/test_menu_app.c:71-72` - `s.menu_frames == 3`, `!s.started` | PASS |
| C2 | accepts 32,65,126,160,231,255; refuses 31,127,159,256,128512; ç = C3 A7; 17th refused | `menu_type_filter` #81 exit 0 | `tests/test_menu.c:70` - `menu_type(&m, ok[i])`; `:76` - `!menu_type(&m, bad[i])`; `:83` - `m.name[0] == 0xC3 && m.name[1] == 0xA7`; `:89` - `!menu_type(&m, 'z')` | PASS |
| C3 | Backspace `Zé` -> `Z` -> empty -> unchanged | `menu_backspace_utf8` #82 exit 0 | `tests/test_menu.c:105` - `strcmp(m.name, "Z") == 0`; `:108`, `:110` - `m.name[0] == '\0' && m.chars == 0` | PASS |
| C4 | blank / `   ` -> disabled, click and Enter MENU_NONE | `menu_play_disabled_when_blank` #83 exit 0 | `tests/test_menu.c:121` - `!menu_play_enabled(&m)`; `:124` - `a == MENU_NONE`; `:125` - `menu_enter(&m) == MENU_NONE` | PASS |
| C5 | `  Ana  ` -> MENU_PLAY, name `Ana`; app_run starts game animated, dealing, name in controller | `menu_play_trims_name` #84, `app_menu_play_starts_game` #91 exit 0 | `tests/test_menu.c:138-139,142` - `a == MENU_PLAY`, `menu_enter(&m) == MENU_PLAY`, `strcmp(t, "Ana") == 0`; `tests/test_menu_app.c:83-86` - `s.started`, `s.animate`, `s.dealing`, `strcmp(s.player, "Ana") == 0` | PASS |
| C6 | click Sair -> MENU_QUIT (empty or filled); app_run returns 0 without game | `menu_quit_click` #85, `app_menu_quit_returns_0` #92 exit 0 | `tests/test_menu.c:155` - `a == MENU_QUIT`; `tests/test_menu_app.c:96-98` - `run(...) == 0`, `!s.started`, `s.menu_frames == 2` | PASS (claim as written; real mouse path unproven - see Coverage, Faults M4) |
| C7 | View carries `Ana`; name rect centred x=640, y>=560, bottom <=605, no overlap with message | `player_name_above_south_hand` #86 exit 0 | `tests/test_menu.c:169` - `strcmp(v.player_name, "Ana") == 0`; `:174-178` - centre in (639.5,640.5), `n.y >= 560`, `n.y + n.h <= hand[0].y`, `hand[0].y == 605`, `!rect_overlaps(n, msg)` | PASS |
| C8 | name_save creates missing dir, writes exactly `5A C3 A9`; app_run writes exactly `Ana` | `name_save_format` #87, `app_menu_saves_name` #93 exit 0 | `tests/test_menu.c:189` - `name_save("tmp_menu/nova/jogador.txt", "Zé")` after `_rmdir` at :188; `:192-193` - `n == 3` and bytes 0x5A,0xC3,0xA9; `tests/test_menu_app.c:119-120` - `n == 3`, `memcmp(buf, "Ana", 3) == 0` | PASS |
| C9 | file `Zé` -> name_load `Zé`; app_run field prefilled | `name_load_prefills` #88, `app_menu_prefills_name` #94 exit 0 | `tests/test_menu.c:202-203` - `name_load(...)`, `strcmp(out, "Zé") == 0`; `tests/test_menu_app.c:133` - `strcmp(s.seen_name, "Zé") == 0` | PASS |
| C10 | empty name for absent, empty, `Ana\n`, `A\tB`, `80`, `C4 80`; 20 letters -> first 16 | `name_load_rejects_invalid` #89 exit 0 | `tests/test_menu.c:217-218` - absent; `:227-229` - each of the 5 bad cases fails `CHECK(false)` if accepted; `:234-235` - `strcmp(out, "abcdefghijklmnop") == 0` | PASS (claim as written; precision gap - claim omits the 127-159 range, see Coverage, Faults M3) |
| C11 | dir-is-a-file -> name_save false; app_run still plays, returns 0 | `app_menu_save_failure_still_plays` #95 exit 0 | `tests/test_menu_app.c:143` - `!name_save("tmp_menu_app/arquivo/jogador.txt", "Ana")`; `:148-150` - `run(...) == 0`, `s.started`, `strcmp(s.player, "Ana") == 0` | PASS |
| C12 | whole suite passes; no pre-existing test file changed since d702049 | `ctest --test-dir build --output-on-failure` 96/96; `git diff --exit-code d702049 -- tests/test_core.c ... tests/core_has_no_raylib.cmake` exit 0 | ctest `100% tests passed out of 96`; diff exit 0; door 2 default: `AppOptions opt = {0}` leaves `show_menu` false (`src/ui/app.h` field `bool show_menu`), old tests unchanged | PASS |
| C13 | real domino.exe, temp APPDATA: `A`,`n`,`a`,Enter -> file `Ana`, exit 0 | `cli_menu_types_and_saves` #96 exit 0 | `tests/test_menu_cli.c:79` - `code == 0`; `:89-90` - `n == 3`, `memcmp(buf, "Ana", 3) == 0` (file is written only in the MENU_PLAY branch, `src/ui/app.c:79-83`, so the write implies the game started) | PASS |

## Coverage

Recomputed from plan AC 1-12, Flow step 3, Surface row, Landing door 1/door 2, and the code (`src/core/menu.c`, `src/core/namefile.c`, `src/ui/app.c`, `src/ui/main.c`), not from the author's table.

| Set (size) | Recomputed from | Member -> proof | Unproven |
| --- | --- | --- | --- |
| char code ranges typed (5) | AC 2 | <32 (31) C2 · 32-126 (32,65,126) C2 · 127-159 (127,159) C2 · 160-255 (160,231,255) C2 · >255 (256,128512) C2 | - |
| char ranges rejected in the file (4) | AC 10 "fora dos códigos aceitos pelo AC 2" + door 1 UTF-8 | <32 (`\n`,`\t`) C10 · 127-159 none · >255 (`C4 80`) C10 · invalid UTF-8 (`80`) C10 | 127-159 in file (`7F` or `C2 80`..`C2 9F`): no case; mutant M3 survived at `src/core/namefile.c:32` |
| ignored-file reasons (AC 10) | AC 10 | absent C10 · empty C10 · out-of-range chars (see row above) · >16 valid -> first 16 C10 | 127-159 member (same as above) |
| menu actions (3) | `MenuAction` in `src/core/menu.h` | MENU_NONE C4 · MENU_PLAY C5 · MENU_QUIT C6 | - |
| menu inputs, decision level (4) | Flow step 3 | char C2 · Backspace C3 · Enter C4,C5 · click C4,C5,C6 | - |
| menu inputs, real-input mapping in app_run (4) | Flow step 3 + `src/ui/app.c:63-76` (bypassed whenever `menu_input` is set) | char `GetCharPressed` C13 · Enter `KEY_ENTER` C13 · Backspace `src/ui/app.c:66-67` none · mouse click `src/ui/app.c:70-75` none | Backspace mapping; mouse-click mapping (Jogar and Sair, AC 5/AC 6) - mutant M4 disabling it survived all 96 tests |
| start-screen elements (AC 1) | AC 1 | title C1 · label C1 · field C1 · Jogar C1 · Sair C1 (constants + rects; drawing is instrumentation per Test policy row 3) | - (note: "sobre a mesa verde" in draw_menu `ClearBackground(TABLE_COLOR)` has no menu-frame pixel proof; accepted under row 3, see Test policy) |
| file statuses (4) | plan Surface | `lido` C9 · `ignorado` C10 · `gravado` C8 · `falha ao gravar` C11 | - |
| door 1 format attributes (6) | Landing door 1 | path `<APPDATA>\Domino\jogador.txt` C13 (`test_menu_cli.c:48,89`) · UTF-8 no BOM C8 (`test_menu.c:192-193`) · no trailing newline C8 (n==3) · 1-16 chars C2,C10 · no edge spaces on write C8 (`test_menu_app.c:119-120`) · Domino dir created C8 (`test_menu.c:188-189`) | - |
| door 2 (2) | Landing door 2 | default false C12 · `main` sets true C13 (killed M5) | - |
| startup config: menu (2 assemblies) | read directly | `src/ui/main.c:23` `opt.show_menu = true;` + `:24-27` name_path from APPDATA -> C13 · tests' assembly `tests/test_menu_app.c:57-59` -> C5,C8,C9,C11 | - |

## Test policy rows

| Row | Files it classifies | Required proof | Expectation met |
| --- | --- | --- | --- |
| Decide, sem fronteira | `src/core/namefile.c` | own level: C8, C9, C10, C11 (name_save false at `test_menu_app.c:143`) | no - the AC 10 decision "char outside AC 2 codes" has a 127-159 row with no case; M3 survived |
| Decide, alcançado pela `domino.exe` | `src/core/menu.c`, `src/ui/app.c` (menu loop) | own level C1-C6, C8, C9, C11 · process C13 (digitar, Enter, gravar) | no - process part met (C13), but "no próprio nível: a tabela inteira" is not met for `app.c`: its input-mapping branches (Backspace `app.c:66-67`, mouse click `app.c:70-75`) are skipped by the `menu_input` hook in every app test and absent from C13; M4 survived |
| Instrumentação (desenho da tela inicial) | `src/ui/draw.c` (`draw_menu`, name label in `draw_frame`) | none own; layout C1, View C7, enabled flag C4 | yes - rects, View name and enabled state are proven; the green background reuses `TABLE_COLOR` (pixel-proven for the game frame by pre-existing `window_table_color`, not for the menu frame) |

Swept: no row resolves to existing code (validation, failure modes, dependency failure, data lifecycle and state transitions cite checks C2-C13; the rest are `n/a` policy). Nothing to re-read.

Hook judgment: the level split is honest for what C13 covers (typing via `GetCharPressed` and Enter go through the real `main` and raylib), and the hook tests faithfully exercise the menu->game transition, save, prefill and quit given an action. But `menu_input` replaces the entire real-input block (`src/ui/app.c:61-76`), so a bug in the Backspace or mouse mapping is invisible to every test - demonstrated by M4. The checks.md note under Coverage acknowledges the hook does not prove raylib input reading, but the Coverage table still lists "clique C4, C5, C6", which hides the gap.

## Faults injected

Isolated worktree at scratchpad `wt` (HEAD 0c5a6f6), separate build dir `bld`. Real tree `git status --porcelain` empty before and after (byte-identical); worktree removed with `git worktree remove --force`. No `git stash`.

| Mutation | Location | Killed |
| --- | --- | --- |
| M1 `cp <= 126` -> `cp <= 127` in `menu_accepts` | `src/core/menu.c:21` | yes - `menu_type_filter` failed at `tests/test_menu.c:76` |
| M2 Backspace continuation-byte loop -> `while (0)` | `src/core/menu.c:46` | yes - `menu_backspace_utf8` failed at `tests/test_menu.c:105` |
| M3 `name_load` uses `cp >= 32 && cp <= 255` instead of `menu_accepts` (forgets 127-159) | `src/core/namefile.c:32` | no - `name_load_rejects_invalid` passed |
| M4 menu mouse block `if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))` -> `if (0)` (menu loop only) | `src/ui/app.c:70` | no - full suite 96/96 passed |
| M5 `opt.show_menu = true` -> `false` | `src/ui/main.c:23` | yes - `cli_menu_types_and_saves` failed at `tests/test_menu_cli.c:84` |

Not mutated (cap of 5): `menu_trimmed_name`, `name_save` directory creation, the app_run MENU_PLAY transition/save call.

## Gate

`ctest --test-dir build --output-on-failure` - 96 passed, 0 failed (17 tela-inicial tests run individually with `-V -L tela-inicial`). `git diff --exit-code d702049 -- <pre-existing test files>` - exit 0.
`validate_verification.py` could not run: Python is not installed on this machine (`python3`/`python` resolve to the Microsoft Store stub). The verdict above is not gated by the script.
