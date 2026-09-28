# CPU em parceria e animação verification

**Verdict**: FAIL
**Profile**: standard
**Diff range**: 7ab288b..297ad4b
**Round**: 1 - full
**Verifier**: independent sub-agent (author != verifier)

## Binding sources

No binding source beyond the user's chat answers recorded in plan.md `## Sources` (no design file, no contract); step 1 is a `ui`-profile step and does not apply under `standard`.

| Source | Opened | Contradiction | Uncovered |
| --- | --- | --- | --- |
| none (chat answers in plan.md `Sources` only) | n/a | none | - |

## Checks

All proofs run at HEAD 297ad4b in one invocation: `ctest --test-dir build --output-on-failure -V` - 62/62 passed, each of the 22 new tests listed individually as `Passed` (#41-#62). Each test name is registered in `CMakeLists.txt` (foreach at lines 82-91 for `test_parceria`, `add_test(NAME app_animations_on ...)` for `test_app_anim`) and dispatched by name from `tests/test_parceria.c:755-777` / `tests/test_app_anim.c:37-39`; `harness.h` `run_named` returns 2 on an unknown name, so no name can pass empty.

| Check | Claim | Proof run | Evidence | Result |
| --- | --- | --- | --- | --- |
| C1 | swapping the other three hands (same counts) never changes the CPU move, 200 seeds | `-R "^cpu_ignores_hidden_hands$"` #41 Passed | `tests/test_parceria.c:95-98` - `CHECK(ha == hb); CHECK(ia == ib); CHECK(ea == eb);` after pooling the other three hands and redistributing them reversed (`:82-93`); `:108` - `CHECK(compared > 1000)` | PASS |
| C2 | `cpu_decide` takes only `CpuView`; `cpu_view_of` copies own hand, counts, ends, lacks | `-R "^cpu_decide_from_view_only$"` #42 Passed | `tests/test_parceria.c:114` - `bool (*decide)(const CpuView *, int *, End *) = cpu_decide;` (compile-time signature); `:132-139` - own tiles, `w.counts[s] == g.hands[s].count`, `w.left == 2 && w.right == 5`, `w.lacks[SEAT_WEST][3]` | PASS |
| C3 | pass on 2/5 marks exactly lacks[2],[5]; persists after later plays | `-R "^pass_records_lacks$"` #43 Passed | `tests/test_parceria.c:155` - `CHECK(g.lacks[SEAT_EAST][v] == (v == 2 \|\| v == 5))`; `:164` - `CHECK(g.lacks[SEAT_EAST][2] && g.lacks[SEAT_EAST][5])` after North plays | PASS |
| C4 | next hand and restart clear all 28 lacks | `-R "^new_hand_clears_lacks$"` #44 Passed | `tests/test_parceria.c:180` and `:187` - `CHECK(!g.lacks[s][v])` after `game_next_hand` and `game_restart_match` | PASS |
| C5 | partner mode avoids leaving 3/6; plays [1\|6] only when forced | `-R "^cpu_partner_mode_avoids_blocking$"` #45 Passed | `tests/test_parceria.c:199-200` - `CHECK(v.own.tiles[idx].a == 3 && v.own.tiles[idx].b == 0); CHECK(end == END_LEFT)`; `:207-208` - `idx == 0`, `END_RIGHT` | PASS |
| C6 | partner with 5 (more) or 4 (tie) -> self mode -> [1\|6] | `-R "^cpu_self_mode_when_partner_not_fewer$"` #46 Passed | `tests/test_parceria.c:215,222` - `partner_counts[] = {5, 4}` ... `CHECK(v.own.tiles[idx].a == 1 && v.own.tiles[idx].b == 6)` (tie boundary killed F2) | PASS |
| C7 | self: [5\|2]; partner mode with partner also lacking 4/2: [5\|6] (AC4 before AC5) | `-R "^cpu_prefers_opponent_pass$"` #47 Passed | `tests/test_parceria.c:238-239` - `idx == 0`, `END_RIGHT`; `:252` - `CHECK(idx == 1)` in partner mode. Swapping AC4/AC5 yields idx 0 (reasoned: forces_pass keeps only [5\|2], then not_blocking empties and falls back) | PASS |
| C8 | ends 1/2, [1\|3],[2\|6],[3\|4]: self -> [1\|3]; partner -> [2\|6] | `-R "^cpu_self_mode_keeps_own_moves$"` #48 Passed | `tests/test_parceria.c:263-264` - `idx == 0`, `END_LEFT`; `:268-269` - `idx == 1`, `END_RIGHT` | PASS |
| C9 | partner N->S, E->W, W->E; non-partner passes do not trigger AC4 | `-R "^cpu_partner_pairs$"` #49 Passed | `tests/test_parceria.c:287` - `CHECK(v.own.tiles[idx].a == 3)` with lacks on the expected partner; `:293` - `CHECK(w.own.tiles[idx].a == 1 && w.own.tiles[idx].b == 6)` with lacks on the previous seat. Note `:280` `CHECK(partner_of(s) == expected[k])` checks the test's own helper (`:32`), not product code - the behavioural lines settle it | PASS |
| C10 | tiebreak: sum, double, first in hand, left end | `-R "^cpu_tiebreak_like_before$"` #50 Passed | `tests/test_parceria.c:305` sum `idx == 1`; `:310-311` double `idx == 1`; `:316` first `idx == 0`; `:321-322` left `idx == 0`, `END_LEFT` | PASS |
| C11 | one flying tile; 0 ms at hand origin (South and CPU), 175 ms strictly between, 349 ms <1 px of slot, 350 ms none | `-R "^slide_moves_hand_to_board$"` #51 Passed | `tests/test_parceria.c:345` origin `< 0.5f`; `:352` `to_dst > 0.5f && to_src > 0.5f && to_dst < total && to_src < total`; `:356` `< 1.0f` at 349; `:359` `v.flying_count == 0` at 350; CPU row `:372`, `:375`, `:378` | PASS |
| C12 | `board_hidden` = slot index during slide, -1 at 350 | `-R "^slide_hides_board_slot$"` #52 Passed | `tests/test_parceria.c:396` - `v.board_hidden == 0` at 349; `:399` - `== -1` at 350; `:406` right end `== 1` | PASS |
| C13 | East not at 350+799, played at 350+800; South click ignored during West slide, works after | `-R "^slide_blocks_timer_and_clicks$"` #53 Passed | `tests/test_parceria.c:422` - `hands[SEAT_EAST].count == 2`; `:424` - `== 1`; `:437-438` click ignored; `:441-442` click plays after slide (killed F4) | PASS |
| C14 | tile k leaves table centre at 50k, lands on hand slot of seat k mod 4 at 50k+300, nothing flying at 1650 | `-R "^deal_animation_timing$"` #54 Passed | `tests/test_parceria.c:485` - centre `< 0.5f` at `start = DEAL_STEP_MS * k`; `:499` - slot `< 1.0f` at arrive-1; `:503` - `flying_with_id(&v, k) == NULL` at arrive; `:505-507` - `now == 1650`, `flying_count == 0`. Seat order via `hand_slot` `k % 4` (`:451`) with `Seat` enum S,E,N,W (`src/core/domino.h:16`) | PASS |
| C15 | `hand_shown[s]` equals arrived count every 25 ms 0..1700 | `-R "^deal_shows_arrived_only$"` #55 Passed | `tests/test_parceria.c:523-527` - `DEAL_STEP_MS * k + DEAL_FLIGHT_MS <= t` ... `if (v.hand_shown[s] != arrived) CHECK(false)` | PASS |
| C16 | during deal: click ignored, no auto-pass, opener CPU not before 1650+800, plays at 1650+800 | `-R "^deal_blocks_play$"` #56 Passed | `tests/test_parceria.c:552` `board.count == 0` at 2449; `:554` `== 1` at 2450; `:564` click ignored at 1000; `:577-578` `turn == SEAT_SOUTH`, `passes == 0` | PASS |
| C17 | flip 1 at 350, <0.05 at 550, 1 at 750; backs until 550, faces from 550; tranque starts reveal immediately | `-R "^reveal_flips_cpu_hands$"` #57 Passed | `tests/test_parceria.c:613` `fabsf(v.flip_scale - 1.0f) < 1e-4f`; `:617` backs at 549; `:620-621` `flip_scale < 0.05f`, faces at 550; `:624` scale 1 at 750; `:636-638` tranque `ANIM_REVEAL`, `flying_count == 0` | PASS |
| C18 | overlay hidden during slide+reveal at hand end and match end; visible at 750 | `-R "^reveal_hides_overlay$"` #58 Passed | `tests/test_parceria.c:652` both phases; `:656-658` `if (v.overlay_visible) CHECK(false)` for t<750; `:663` `CHECK(v.overlay_visible)` | PASS |
| C19 | click/Enter during reveal -> overlay visible, scale 1, still HAND_OVER; next input starts next hand | `-R "^reveal_skip_on_input$"` #59 Passed | `tests/test_parceria.c:682-684` - `v.overlay_visible`, `flip_scale` 1, `phase == PHASE_HAND_OVER`; `:689-690` - `PHASE_PLAYING`, `board.count == 0` | PASS |
| C20 | pulse min in [0.35,0.36], max in [0.84,0.85], period 800 | `-R "^pulse_period_800$"` #60 Passed | `tests/test_parceria.c:716-717` - `lo >= 0.35f - 1e-6f && lo <= 0.36f`, `hi >= 0.84f && hi <= 0.85f + 1e-6f`; `:720` - `fabsf(v.pulse - samples[t]) < 1e-4f` (killed F5) | PASS |
| C21 | 40 `partida-duplas` tests pass; old test files unchanged since 7ab288b | `ctest -L partida-duplas` - 40 tests, 0 failed; `git diff --exit-code 7ab288b -- tests/test_core.c ... tests/core_has_no_raylib.cmake` exit 0 | label assigned in `CMakeLists.txt` (`set_tests_properties(${t} PROPERTIES LABELS partida-duplas)` on all 4 old groups); diff exit code 0 | PASS |
| C22 | first frame of `app_run`: animations on, deal running | `-R "^app_animations_on$"` #62 Passed | `tests/test_app_anim.c:29-31` - `CHECK(animate); CHECK(dealing); CHECK(flying >= 1);`; assembly read directly: `src/ui/app.c:41` `ctl_set_animations(&c, true);`, `src/ui/main.c:17` `return app_run(&opt);` | PASS |
| C23 | after `ctl_init` animations off: nothing flying, hand_shown = count, overlay immediate on win | `-R "^animations_off_by_default$"` #61 Passed | `tests/test_parceria.c:732` `!c.animate`; `:735-737` `flying_count == 0`, `hand_shown[s] == count`; `:748-749` `v.overlay_visible`, `flying_count == 0` | PASS |

Level/sampling: C1 samples one permutation (reversal of the pooled tiles) per CPU turn over 200 seeds and >1000 comparisons; the door-1 signature (C2) makes reading `Game` structurally impossible, so one perturbation per state is adequate. All other checks are fixed scenarios at the controller/core level, which is the level the Test policy assigns.

Precision gaps (findings about the checks, not failing on their own):
- AC 9 "entre os dois fica no meio do caminho" became "strictly between" in C11, and the plan's ease-out cubic curve (Assumptions) has no assertion: linear interpolation would pass `tests/test_parceria.c:352`.
- C20 bounds tolerate a 1% amplitude error (0.36..0.84 passes `:716-717`).

## Coverage

| Set (size) | Recomputed from | Member -> proof | Unproven |
| --- | --- | --- | --- |
| CPU rule chain and its order (9) | plan AC 4, 5, 6, 8 + Assumptions "Ordem das regras" ; `src/core/cpu.c:94-115` | AC4 filter C5 · AC4 fallback when only blocking move C5 · AC5 filter C7 · AC5 fallback (no forcing move) C7 `u` · AC4 before AC5 (partner mode) C7 `w` · AC6 in self mode C8 · AC6 not in partner mode C8 · tiebreaks sum/double/first/left C10 · **AC5 before AC6 (self mode)** none | AC5-before-AC6 in self mode: no test distinguishes it; mutant F1 (own-fit filter applied before opponent-pass filter, `src/core/cpu.c:96-106`) survived all 62 tests. Counterexample: North, ends 4/5, hand [5\|2],[5\|6],[6\|1], West lacks 4 and 2, partner 7 -> spec says [5\|2], mutant plays [5\|6] |
| mode choice (3) | AC 4/6 + Assumption tie -> self; `cpu.c:92` | fewer C5 · tie C6 · more C6 | - |
| partner pairs (3) | AC 7; `cpu.c:90` | N->S, E->W, W->E C9 (behavioural lines `:287`, `:293`) | - |
| lacks lifecycle (3) | AC 2, 3; `src/core/rules.c:71,196-197` | record both ends C3 · persist across plays C3 · clear on next hand / restart C4 | - |
| entries into the deal animation (3) | AC 12 "WHEN uma mão começa" + S3 independent test ("Nova partida ou começar uma mão nova"); `src/core/controller.c:45,58` (via `:109`, `:115`, `:165`) | first hand via `ctl_set_animations` C14/C22 · **next hand (click/Enter)** none · **Nova partida** none | next hand and new match: `rg -n "ANIM_DEAL" tests/` hits only `tests/test_app_anim.c:17`; `reveal_skip_on_input` (`:689-690`) asserts only phase/board after the next hand, never that the deal runs. Removing `start_anim(c, ANIM_DEAL)` at `controller.c:58` would pass every test (reasoned, not injected - cap of 5 reached) |
| animation kinds (4) | AC 9, 12, 15, 18 | deal C14 · slide C11 · reveal C17 · pulse C20 | - |
| timeline transitions (other than deal entry) (6) | `controller.c:184-195`, `:72-78`, `:83-89`, `:103-107`, `:158-162` | deal->none C14 · none->slide C11 · slide->none C11 · slide->reveal C17 · none->reveal on tranque C17 · reveal->none C17, by click/Enter C19 | - |
| inputs blocked by animation (7) | AC 11, 14, 17; `controller.c:121`, `:187-195` | deal click/CPU timer/auto-pass C16 · slide click/CPU timer C13 · reveal click/Enter C19 | - |
| overlay hidden (2) | AC 16 | hand end C18 · match end C18 | - |
| slide origin (2) | AC 9 | South hand C11 · CPU row C11 (East) | - |
| door 2 `View` fields (Landing literal shape) (6+) | plan Landing door 2: `Flying flying[TILE_COUNT]` "(retângulo interpolado + valores)", `hand_shown`, `board_hidden`, `flip_scale`, `overlay_visible`, `pulse`; `controller.h:37-64` | flying rect C11/C14 · hand_shown C15/C23 · board_hidden C12 · flip_scale C17 · overlay_visible C18 · pulse C20 · on/off C22/C23 · **Flying values (`first`, `second`, `vertical`, `face_up`)** none | Flying values: `rg -n "\.first\b\|\.second\|vertical\|face_up" tests/test_parceria.c tests/test_app_anim.c` finds only `v->face_up[...]` (hand faces) at `:599`; `controller.c:257-261,276-279` are unasserted. The plan's "Peça em voo: face para cima, orientação do destino" assumption is unproven, and the deal deliberately deviates (`face_up = false`, `controller.c:261`) without any check recording that decision |
| door 1 (1) | plan Landing door 1 literal shape; `src/core/domino.h:87-101` | C2 (shape matches literal; signature checked at compile time `:114`) | - |
| startup config: animations (2 assemblies) | `src/ui/app.c:41`, `src/ui/main.c:17`, `controller.c:32-36` | `domino.exe` via `app_run` C22 · `ctl_init` C23 | - |

Swept rows resolving to existing code, re-read: idempotency C19 - `controller.c:104-107` returns after clearing the animation without calling `game_next_hand` (present); data lifecycle C4 - `rules.c:71` `memset(g->lacks, 0, ...)` in `start_hand` (present); state transitions C11/C14/C17/C19 present as above. `Surface` and `Relations` are `None`; no exit-code/route set owed.

Note on the deal start size (asked by the orchestrator): tiles start at 40% size centred on the table (`controller.c:252-254`). AC 12 fixes only the start point (table centre), which C14 asserts; size is unspecified, so this is consistent with AC 12. It is also what keeps the untouched `window_table_color` test green: table centre is (640,335), the sampled pixel is (640,360) (`tests/test_window.c:18`); a full-size 50x100 tile would cover it on frame 0, the 40% tile (y 315..355) does not. The coupling is undocumented in the checks and fragile, but not a contradiction.

## Test policy rows

| Row | Files it classifies | Required proof | Expectation met |
| --- | --- | --- | --- |
| Decide, reached across a boundary (`app_run`) | `src/core/controller.c` (timeline) | boundary C22 · own level C11-C20, C23 | no - boundary met (C22), but the own-level decision table has unasserted rows: deal entry on next hand / Nova partida (`controller.c:58`) |
| Decide, no boundary | `src/core/cpu.c`, `src/core/rules.c` | own level C2-C10 (cpu), C3-C4 (rules) | no - `rules.c` met; `cpu.c` has no case for the AC5-before-AC6 row in self mode (F1 survived) |
| Instrumentation (drawing that only reads `View`) | `src/ui/draw.c` | none of its own; covered by producer's `View` tests | no - `draw.c` reads `Flying.first/second/vertical/face_up` (`draw.c` flying loop), which no `View` test asserts |

## Faults injected

Isolated worktree at `scratchpad/wt` (HEAD 297ad4b), separate build dir `bwt`; real tree `git status --porcelain` empty before and identical (empty) after; worktree removed with `git worktree remove --force`.

| Mutation | Location | Killed |
| --- | --- | --- |
| F1: self mode applies own-fit filter (AC6) before opponent-pass filter (AC5) | `src/core/cpu.c:96-106` | no - `cpu_prefers_opponent_pass` passed, and the full suite (62/62) passed |
| F2: mode boundary `<` -> `<=` (tie goes to partner mode) | `src/core/cpu.c:92` | yes - `cpu_self_mode_when_partner_not_fewer` failed at `tests/test_parceria.c:222` |
| F3: `game_pass` no longer records the right end in `lacks` | `src/core/rules.c:197` | yes - `pass_records_lacks` failed at `tests/test_parceria.c:155` |
| F4: CPU timer also counts while an animation runs (slide no longer blocks it) | `src/core/controller.c:187` | yes - `slide_blocks_timer_and_clicks` failed at `tests/test_parceria.c:422` |
| F5: pulse period 800 -> 1000 ms | `src/core/controller.c:322` | yes - `pulse_period_800` failed at `tests/test_parceria.c:720` |

## Gate

`ctest --test-dir build --output-on-failure -V` - 62 passed, 0 failed (40 `partida-duplas` + 22 `cpu-parceria-animacao`); `ctest -L partida-duplas` - 40 passed; C21 `git diff --exit-code` exit 0.

`validate_verification.py` could not be run: Python is unavailable on this machine. The verdict above is FAIL on its own rows (one `Killed: no`, three non-empty `Unproven` cells, three unmet Test policy rows).
