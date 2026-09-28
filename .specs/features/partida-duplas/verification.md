# Partida em duplas verification

**Verdict**: PASS
**Profile**: light
**Diff range**: c4e22fc..0223dbd (HEAD)
**Round**: 1 - full
**Verifier**: independent sub-agent (author != verifier)

## Binding sources

| Source | Opened | Contradiction | Uncovered |
| --- | --- | --- | --- |
| none - no design file; plan `## Sources` lists only the user's chat answers (2026-09-27), already folded into `plan.md` | n/a | none | - |

The diff to `checks.md` in the range is only the 38 `[ ]` -> `[x]` ticks (checked with `git diff --word-diff`); no claim text or proof was edited during the build.

## Checks

All CTest proofs were run in one invocation at HEAD `0223dbd`: `ctest --test-dir build --output-on-failure -V` -> exit 0, `100% tests passed out of 40`. Each of the 39 named tests below appears individually as `Passed` in that output, is registered in `CMakeLists.txt:47-71`, and resolves to a function in `tests/*.c` dispatched by name (`tests/harness.h:20-31`, unknown name returns 2, so a misnamed test cannot pass silently).

| Check | Claim | Proof run | Evidence | Result |
| --- | --- | --- | --- | --- |
| C1 | window title `Dominó`, client area 1280x720 | `cli_window_title_and_size` Passed (0.41 s), real `domino.exe --seed 1` | `tests/test_process.c:121-123` - `CHECK(wcscmp(title, L"Dominó") == 0); CHECK(r.right - r.left == 1280); CHECK(r.bottom - r.top == 720)` | PASS |
| C2 | pixel (640,360) of first frame is RGB(20,100,50) | `window_table_color` Passed (0.43 s) | `tests/test_window.c:32` - `if (sampled.r != 20 \|\| sampled.g != 100 \|\| sampled.b != 50) ... CHECK(false)`; sample taken at frame 0 (`test_window.c:16-18`) inside `BeginDrawing`/`EndDrawing` (`src/ui/app.c:52-56`) | PASS |
| C3 | seeds 0..999: 7 each, exactly the 28 `[a\|b]`, no repeats | `deal_28_unique_7_each` Passed | `tests/test_core.c:65` - `CHECK(g.hands[s].count == 7)`; `:74` `CHECK(total == 28)`; `:77` `CHECK(seen[a][b] == 1)` over `0<=a<=b<=6`, loop `seed < 1000` at `:59` | PASS |
| C4 | South hand ascending by sum, all rects y >= 560 | `south_hand_sorted_by_sum`, `south_hand_in_bottom_band` Passed | `tests/test_core.c:135` - `CHECK(tile_sum(h->tiles[i - 1]) <= tile_sum(h->tiles[i]))`; `:146` - `CHECK(r[i].y >= 560)` for n=1..7 | PASS |
| C5 | each CPU row has exactly hand-count face-down tiles, before and after playing | `cpu_rows_match_hand_count` Passed | `tests/test_core.c:163-164` - `CHECK(v.hand_count[s] == n); CHECK(!v.face_up[s])`; `:188-189` `CHECK(after < before); CHECK(check_cpu_rows(&c) == 0)` | PASS |
| C6 | same seed same 4 hands; `--seed 42` reaches the game as 42 | `deal_same_seed_same_hands`, `cli_parse_seed` Passed | `tests/test_core.c:91-92` - `CHECK(g1.hands[s].tiles[i].a == g2.hands[s].tiles[i].a)` (and `.b`), seeds 0..49; `:114-116` - `CHECK(cli_parse(3, a1, &seed, &has) == 0); CHECK(has); CHECK(seed == 42)` | PASS |
| C7 | `--seed x`, `--seed -1`, `--seed`, `--foo` -> usage on stderr, exit 2, no window | `cli_bad_args_exit_2` Passed (real exe) | `tests/test_process.c:94` - `cases[] = {"--seed x", "--seed -1", "--seed", "--foo"}`; `:101` - `if (code != 2 \|\| strstr(err, "uso: domino [--seed <n>]") == NULL) ... CHECK(false)`. "no window" is not asserted (see finding 1); holds by structure `src/ui/main.c:11-13` returns 2 before `app_run` | PASS |
| C8 | window not ready -> `erro: não foi possível abrir a janela` on stderr, return 1 | `window_fail_exits_1` Passed | `tests/test_window.c:58` - `if (code != 1 \|\| strstr(buf, "erro: não foi possível abrir a janela") == NULL) ... return 1`; via seam `opt.force_window_fail = true` (`:47`) - declared level gap, see finding 2 | PASS |
| C9 | `WM_CLOSE` to the window -> exit 0 | `cli_close_exits_0` Passed (real exe) | `tests/test_process.c:134-136` - `PostMessageW(w, WM_CLOSE, 0, 0); ... CHECK(code == 0)` | PASS |
| C10 | first hand: `[6\|6]` holder starts, any other tile rejected | `first_hand_double_six_starts`, `first_hand_rejects_non_double_six` Passed | `tests/test_core.c:201` - `CHECK(g.turn == holder_of_double_six(&g))` (200 seeds); `:217-219` - `CHECK(!game_play(&g, i, END_LEFT)); CHECK(!game_play(&g, i, END_RIGHT)); CHECK(g.board.count == 0)` | PASS |
| C11 | turn S -> E -> N -> W -> S | `turn_order_counterclockwise` Passed | `tests/test_core.c:240,242,244,248` - `CHECK(g.turn == SEAT_EAST)`, `== SEAT_NORTH`, `== SEAT_WEST`, `== SEAT_SOUTH` (last hop by a play, others by pass) | PASS |
| C12 | single-end fit placed there, equal value touching, end becomes other value | `click_single_end_places` Passed | `tests/test_core.c:260-261` - `CHECK(c.game.board.line[1].left == 3); CHECK(c.game.board.line[1].right == 5)` (`[5\|3]` on 1/3); left side `:272-273` `line[0].left == 4`, `line[0].right == 1` | PASS |
| C13 | ends 2/5, click `[2\|5]` -> board unchanged, both ends highlighted; next click on right end places right | `click_both_ends_waits_choice` Passed | `tests/test_core.c:284` `CHECK(c.game.board.count == 1)`; `:288-289` `CHECK(v.highlight_end[END_LEFT]); CHECK(v.highlight_end[END_RIGHT])`; `:291-293` `board.count == 2`, `line[1].left == 5`, `line[1].right == 2` | PASS |
| C14 | ends 3/3, click `[3\|1]` -> right end, no highlight | `click_equal_ends_goes_right` Passed | `tests/test_core.c:308-312` - `CHECK(!v.highlight_end[END_LEFT] && !v.highlight_end[END_RIGHT]); CHECK(c.game.board.count == 2); ... CHECK(c.game.board.line[1].right == 1)` | PASS |
| C15 | non-fitting click: board and hand unchanged, `Jogada inválida` visible at 1499 ms, gone at 1500 ms | `click_invalid_shows_message` Passed | `tests/test_core.c:323-325` board/hand unchanged; `:330` `CHECK(strcmp(v.message, "Jogada inválida") == 0)`; `:331-333` `ctl_update(&c, 1499) ... CHECK(v.message_visible)`; `:334-336` `ctl_update(&c, 1) ... CHECK(!v.message_visible)` | PASS |
| C16 | off-turn click on South tile changes nothing | `click_ignored_off_turn` Passed | `tests/test_core.c:349-352` - `CHECK(c.game.board.count == 1); CHECK(c.game.hands[SEAT_SOUTH].count == 2); CHECK(c.game.hands[SEAT_EAST].count == east); CHECK(c.game.turn == SEAT_EAST)` | PASS |
| C17 | South with no fit -> turn to East, `Você passou` for 1500 ms | `south_auto_pass` Passed | `tests/test_core.c:367` `CHECK(c.game.turn == SEAT_EAST)`; `:372` `CHECK(strcmp(v.message, "Você passou") == 0)`; `:373-379` visible at 1499, `!v.message_visible` after +1 | PASS |
| C18 | CPU not moved at 799 ms, moved at 800 ms | `cpu_moves_at_800ms` Passed | `tests/test_core.c:391-393` - `ctl_update(&c, 799); CHECK(c.game.board.count == 1); CHECK(c.game.turn == SEAT_EAST)`; `:394-397` - `ctl_update(&c, 1); CHECK(c.game.board.count == 2) ... CHECK(c.game.turn == SEAT_NORTH)` | PASS |
| C19 | heaviest; tie -> double; tie -> first in hand; both-ends -> left | `cpu_picks_heaviest` Passed | `tests/test_core.c:414` `idx == 1` (heaviest); `:422-423` `idx == 1`, `end == END_RIGHT` (double wins tie); `:430` `idx == 0` (first in hand); `:438-439` `idx == 0`, `end == END_LEFT` (both ends) | PASS |
| C20 | CPU pass -> `Leste passou` / `Parceiro passou` / `Oeste passou` for 1500 ms | `cpu_pass_message` Passed | `tests/test_core.c:452` - `msgs[] = {"Leste passou", "Parceiro passou", "Oeste passou"}`; `:467` `CHECK(strcmp(v.message, msgs[k]) == 0)`; `:468-474` visible at 1499, gone at 1500 | PASS |
| C21 | no N/E/W tile marked face-up while hand in play | `cpu_faces_hidden_in_play` Passed | `tests/test_core.c:488-490` - `CHECK(!v.face_up[SEAT_EAST]); CHECK(!v.face_up[SEAT_NORTH]); CHECK(!v.face_up[SEAT_WEST])` over 20 seeds x up to 40 steps; consumed by `src/ui/draw.c:107-110` | PASS |
| C22 | non-double, one end -> `SIMPLES`, +1 | `score_simples` Passed | `tests/test_core.c:516-518` - `CHECK(g.result == RESULT_SIMPLES); CHECK(g.score[TEAM_THEM] == 1); CHECK(g.score[TEAM_US] == 0)` | PASS |
| C23 | double, one end -> `CARROCA`, +2 | `score_carroca` Passed | `tests/test_core.c:532-534` - `CHECK(g.result == RESULT_CARROCA); CHECK(g.score[TEAM_US] == 2); CHECK(g.score[TEAM_THEM] == 0)` | PASS |
| C24 | non-double both ends -> `LA_E_LO`, +3, on 2/5 and 3/3 | `score_la_e_lo` Passed | `tests/test_core.c:548-549` - `RESULT_LA_E_LO`, `score[TEAM_US] == 3` (`[2\|5]` on 2/5); `:558-559` - `RESULT_LA_E_LO`, `score[TEAM_THEM] == 3` (`[3\|1]` on 3/3) | PASS |
| C25 | `[4\|4]` on 4/4 -> `CRUZADA`, +4 | `score_cruzada` Passed | `tests/test_core.c:574-575` - `CHECK(g.result == RESULT_CRUZADA); CHECK(g.score[TEAM_US] == 4)` | PASS |
| C26 | 4 passes -> `TRANCADO`, +1 to lower sum | `score_trancado_lower_sum` Passed | `tests/test_core.c:603` still `PHASE_PLAYING` after 3 passes; `:607-609` `RESULT_TRANCADO`, `score[TEAM_THEM] == 1`, `score[TEAM_US] == 0` (10 < 14); reverse `:616-617` | PASS |
| C27 | tranque with equal sums -> +0 both | `score_trancado_tie` Passed | `tests/test_core.c:629-631` - `CHECK(g.result == RESULT_TRANCADO); CHECK(g.score[TEAM_US] == 0); CHECK(g.score[TEAM_THEM] == 0)` | PASS |
| C28 | hand end: 4 hands face-up, overlay type/points/score, only click or Enter continues | `hand_end_overlay` Passed | `tests/test_core.c:648` `CHECK(v.face_up[k])` for all 4; `:649-651` `"Batida simples"`, `"+1 para Nós"`, `"Nós 1 × 0 Eles"`; `:652-653` still `PHASE_HAND_OVER` after 60000 ms; `:658` `PHASE_PLAYING` after click and after Enter | PASS |
| C29 | after a batida, winner opens, non-`[6\|6]` accepted | `next_hand_winner_starts` Passed | `tests/test_core.c:700` `CHECK(g.turn == SEAT_EAST)`; `:709-710` `CHECK(game_play(&g, idx, END_LEFT)); CHECK(g.board.count == 1)` with non-`[6\|6]` idx | PASS |
| C30 | after tranque, opener of that hand opens, non-`[6\|6]` accepted | `next_hand_after_tranque` Passed | `tests/test_core.c:723-724` `CHECK(g.turn == SEAT_WEST); CHECK(!g.first_hand)`; `:733` `CHECK(game_play(&g, idx, END_LEFT))` | PASS |
| C31 | `Nós 0 × 0 Eles`, then `Nós 3 × 1 Eles`, top-left anchor | `score_text_format` Passed | `tests/test_core.c:743` `strcmp(v.score_text, "Nós 0 × 0 Eles") == 0`; `:744` `CHECK(v.score_x <= 20 && v.score_y <= 20)`; `:748` `"Nós 3 × 1 Eles"`; drawn each frame at those coords `src/ui/draw.c:143` | PASS |
| C32 | >= 6 ends match with `Vocês venceram!` / `Eles venceram!` and `Nova partida` button | `match_end_message` Passed | `tests/test_core.c:765-766` `"Vocês venceram!"`, `show_new_match_button`; `:778-780` `"Eles venceram!"`, button, `"Nós 1 × 8 Eles"` (>6); `:788-790` 5 points stays `PHASE_HAND_OVER`, no button | PASS |
| C33 | `Nova partida` resets to 0 × 0 and `[6\|6]` rule | `new_match_resets` Passed | `tests/test_core.c:806-811` - `PHASE_PLAYING`, `score[TEAM_US] == 0`, `score[TEAM_THEM] == 0`, `first_hand`, `board.count == 0`, `turn == holder_of_double_six(...)`; `:814` `"Nós 0 × 0 Eles"` | PASS |
| C34 | first line tile centred in table area | `layout_first_tile_centered` Passed | `tests/test_core.c:850-851` - `CHECK(cx > t.x + t.w / 2 - 0.5f && cx < t.x + t.w / 2 + 0.5f)` (and `cy`), for `[6\|6]` and `[2\|5]` | PASS |
| C35 | 1..28 tiles, left/right/alternating growth: all inside table, no overlap | `layout_within_table` Passed | `tests/test_core.c:869-872` - containment in `layout_table_area()`; `:875-878` `rect_overlaps` -> `CHECK(false)`; `n = 1..TILE_COUNT`, anchors `{0, n-1, n/2}`, with and without doubles (`:859-862`) | PASS |
| C36 | every double perpendicular to its segment | `layout_doubles_perpendicular` Passed | `tests/test_core.c:901` - `CHECK(s[i].vertical == s[i].horizontal_segment)` for every double; `:906-907` `doubles_seen > 0`, `vertical_segments > 0` | PASS |
| C37 | no `domino_core` file includes `raylib.h` | `core_has_no_raylib` Passed | `tests/core_has_no_raylib.cmake:9-11` - `string(FIND "${content}" "raylib.h" pos)` -> `FATAL_ERROR`; `:4` empty glob fails. `CMakeLists.txt:12-18` `domino_core` sources are all under `src/core`, no raylib link | PASS |
| C38 | configure finds raylib by `find_package`, build exits 0 | `cmake -S . -B build -G Ninja && cmake --build build` exit 0 (incremental); plus fresh configure+build in a scratch dir exit 0, 17/17 steps | `CMakeLists.txt:20` - `find_package(raylib REQUIRED)`; fresh cache `raylib_DIR:PATH=C:/msys64/ucrt64/lib/cmake/raylib`, config found at `C:/msys64/ucrt64/lib/cmake/raylib/raylib-config.cmake` | PASS |

### Level and sampling judgments (light)

1. **C7 - sub-claim "sem abrir janela" has no assertion.** `tests/test_process.c:92-107` asserts exit code and stderr only; it never enumerates windows for the child pid, although `wait_window` (`:80`) exists in the same file. It holds today only by structure: `src/ui/main.c:11-13` returns 2 before `app_run`, the only `InitWindow` caller (`src/ui/app.c:30`). Precision gap in the proof, not a defect in the code.
2. **C8 - injection seam, declared.** `force_window_fail` (`src/ui/app.c:28-32`) skips `InitWindow` entirely, so the real `!IsWindowReady()` branch is never executed and the test runs `app_run` in-process. It does not go through `main.c:17`, so the process exit code 1 is not observed either. checks.md Coverage notes declare this. The level matches the declaration, but the claim ("o programa ... retorna 1") is proven at function level, not process level.
3. **C6 - the `main` -> `app_run` -> `ctl_init` hop is unasserted.** `cli_parse_seed` proves parsing only. The wiring `src/ui/main.c:11-17` -> `src/ui/app.c:40` (`ctl_init(&c, opt->seed)`) has no test. Low risk, since it is a 3-line pass-through.
4. **Screen checks are proven on the `View`, declared.** C4, C5, C15, C21, C28, C31 and C32 assert `ctl_view` fields. I read `src/ui/draw.c` to confirm the drawing consumes exactly those fields: `face_up` -> `draw_back` (`:107-110`), `score_x/score_y` (`:143`), `message_visible` (`:144-145`), overlay strings (`:121-133`). C32's `show_new_match_button` is **not** read by `draw.c`; the button is drawn on `phase == PHASE_MATCH_OVER` (`:121-127`), which is equivalent today because `src/core/controller.c:171-174` sets the flag under the same condition. C36 asserts the `vertical` flag rather than the rect's aspect. `draw.c:79` draws by that flag, and `src/core/layout.c:35,42,50,53` builds rect dimensions consistent with it.
5. **Sampling, all minor.**
   - C9 "em qualquer momento" is sampled at one moment, about 300 ms after the window appears (`test_process.c:133`). The loop `app.c:41` handles close uniformly.
   - C34 is tested at `count == 1` only. `layout.c:73` centres the anchor for every count, so it holds.
   - C29 is tested with a single winner seat (East), and C30 with a single opener (West).
   - C4's ordering is tested on the fresh deal only. Order is kept after plays by the shift removal at `src/core/rules.c:163-165`.
6. **Observation outside the checks (not a check failure).** `src/core/cli.c:7-22` accepts a repeated `--seed` (`--seed 1 --seed 2` -> exit path 0, last wins). AC 6 says "ou aparece qualquer outro argumento", and whether a second `--seed` counts as "another argument" is ambiguous. No check covers it. Precision gap in the plan/checks, not proof of a defect.

### Swept rows re-read

- validation: C7 -> `src/core/cli.c:7-22`; C15 -> `src/core/controller.c:75-76`. Both present.
- failure modes / dependency failure: C8 -> `src/ui/app.c:32-34`. Present.
- idempotency: C16 -> `src/core/controller.c:55-56` (`if (g->turn != SEAT_SOUTH) return;`). Present; the test clicks twice (`test_core.c:347-348`).
- concurrency (n/a, timers by `dt`): `src/core/controller.c:117-119`. Consistent with the stated policy.
- state transitions: C10, C11, C28, C29, C30, C32 and C33 all exist and pass (above).
- authorization, data lifecycle, observability: `n/a` - user-approved policy, nothing to re-read.

## Coverage

Not recomputed under `light`. I spot-read the checks.md Coverage rows against the tests and found no contradiction:
- statuses 0/1/2 -> C9/C8/C7
- the 4 invalid-arg members are literally the `cases[]` at `test_process.c:94`
- all 5 `HandResult` values are asserted
- all 4 turn hops, 3 openers, 4 CPU tiebreak members, 5 click kinds, 4 pass messages and 2 winners are asserted
- line sizes 1..28 are table-driven
- doors map to C37/C38

## Gate

- `cmake -S . -B build -G Ninja && cmake --build build` exited 0. A fresh scratch-dir configure and build also exited 0.
- `ctest --test-dir build --output-on-failure -V`: 40 passed, 0 failed (100%).
- `validate_verification.py` was not run: Python is not available on this machine (`python3` is the Microsoft Store alias, recorded in plan Assumptions). I checked the report's columns by hand: no `Result` other than PASS, no `Contradiction`/`Uncovered` entries, and no `Unproven` cells, because Coverage was not recomputed under light.
