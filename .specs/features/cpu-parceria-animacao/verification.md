# CPU em parceria e animação verification

**Verdict**: PASS
**Profile**: standard
**Diff range**: 7ab288b..14eef12
**Round**: 2 - scoped
**Verifier**: independent sub-agent (author != verifier)

Round 1 (at 297ad4b) was FAIL: surviving mutant F1, three unproven coverage members, three unmet Test policy rows. The fix is commit 14eef12 (`git diff 297ad4b..14eef12`). It touches only `tests/test_parceria.c` (new assertions inside the existing C7, C11 and C14 tests; no check text changed) and the "Peça em voo" Assumptions row of `plan.md`. It also commits the round-1 report. No product code changed: `git diff 297ad4b..14eef12 -- src CMakeLists.txt` is empty.

## Binding sources

carried from 297ad4b. The fix did not touch the interface.

| Source | Opened | Contradiction | Uncovered |
| --- | --- | --- | --- |
| none (chat answers in plan.md `Sources` only) | n/a | none | - |

## Checks

verified at 14eef12. Every proof was re-run at the new HEAD in one invocation: `ctest --test-dir build --output-on-failure -V` gave 62/62 passed, with each of the 22 new tests listed individually as `Passed` (#41-#62). `build/test_parceria.exe` (23:43:53) is newer than `tests/test_parceria.c` (23:43:52), so the binary includes the fix. Citations were refreshed for `tests/test_parceria.c`, the only test file the fix touched. All other files are unchanged.

| Check | Claim | Proof run | Evidence | Result |
| --- | --- | --- | --- | --- |
| C1 | swapping the other three hands never changes the CPU move, 200 seeds | `-R "^cpu_ignores_hidden_hands$"` #41 Passed | `tests/test_parceria.c:97-100` - `CHECK(ha == hb); CHECK(ia == ib); CHECK(ea == eb);`; `:110` - `CHECK(compared > 1000)` | PASS |
| C2 | `cpu_decide` takes only `CpuView`; `cpu_view_of` copies own hand, counts, ends, lacks | `-R "^cpu_decide_from_view_only$"` #42 Passed | `tests/test_parceria.c:116` - `bool (*decide)(const CpuView *, int *, End *) = cpu_decide;`; `:134-141` own tiles, counts, `w.left == 2 && w.right == 5`, `w.lacks[SEAT_WEST][3]` | PASS |
| C3 | pass on 2/5 marks exactly lacks[2],[5]; persists | `-R "^pass_records_lacks$"` #43 Passed | `tests/test_parceria.c:157` - `CHECK(g.lacks[SEAT_EAST][v] == (v == 2 \|\| v == 5))`; `:166` persists after North plays | PASS |
| C4 | next hand and restart clear all 28 lacks | `-R "^new_hand_clears_lacks$"` #44 Passed | `tests/test_parceria.c:182`, `:189` - `CHECK(!g.lacks[s][v])` | PASS |
| C5 | partner mode avoids leaving 3/6; [1\|6] when it is the only move | `-R "^cpu_partner_mode_avoids_blocking$"` #45 Passed | `tests/test_parceria.c:201-202` - `[3\|0]`, `END_LEFT`; `:209-210` - `idx == 0`, `END_RIGHT` | PASS |
| C6 | partner with 5 or 4 (tie) -> self mode -> [1\|6] | `-R "^cpu_self_mode_when_partner_not_fewer$"` #46 Passed | `tests/test_parceria.c:219,224` - `partner_counts[k]` in {5,4} ... `CHECK(v.own.tiles[idx].a == 1 && v.own.tiles[idx].b == 6)` | PASS |
| C7 | opponent-pass preference; AC4 before AC5 in partner mode; (new) AC5 before AC6 in self mode | `-R "^cpu_prefers_opponent_pass$"` #47 Passed | `tests/test_parceria.c:240-241` self `idx == 0`, `END_RIGHT`; **new** `:255-256` - hand [5\|2],[5\|6],[6\|1], ends 4/5, West lacks 4/2, partner 7 -> `CHECK(idx == 0); CHECK(end == END_RIGHT);` (kills F1); `:263` partner mode `idx == 1` | PASS |
| C8 | self -> [1\|3]; partner -> [2\|6] | `-R "^cpu_self_mode_keeps_own_moves$"` #48 Passed | `tests/test_parceria.c:274-275`, `:279-280` | PASS |
| C9 | partner N->S, E->W, W->E; non-partner passes inert | `-R "^cpu_partner_pairs$"` #49 Passed | `tests/test_parceria.c:298` (`CHECK(v.own.tiles[idx].a == 3)`), `:304` - `CHECK(w.own.tiles[idx].a == 1 && w.own.tiles[idx].b == 6)`. Line `:291` still checks the test's own `partner_of` helper, which is weak but not load-bearing | PASS |
| C10 | tiebreak sum, double, first, left | `-R "^cpu_tiebreak_like_before$"` #50 Passed | `tests/test_parceria.c:316`, `:321-322`, `:327`, `:332-333` | PASS |
| C11 | slide 0/175/349/350 ms, South and CPU origin; (new) ease-out at 175 ms, face-up with destination values | `-R "^slide_moves_hand_to_board$"` #51 Passed | `tests/test_parceria.c:363` strictly between; **new** `:365-366` - centre at `src + 0.875f * (dst - src)` within 0.5 px (kills F10); **new** `:368` `CHECK(v.flying[0].face_up)`, `:369` `vertical == slots[1].vertical`, `:370` `CHECK(v.flying[0].first == 3 && v.flying[0].second == 5)`; CPU **new** `:391-392` face-up, `first == 4 && second == 3`; 349 ms `< 1.0f` and 350 ms `flying_count == 0` follow | PASS |
| C12 | `board_hidden` = slot during slide, -1 at 350 | `-R "^slide_hides_board_slot$"` #52 Passed | `tests/test_parceria.c:416` `== 0` at 349; `:419` `== -1`; `:426` `== 1` | PASS |
| C13 | East not at 350+799, plays at 350+800; South click ignored during slide | `-R "^slide_blocks_timer_and_clicks$"` #53 Passed | `tests/test_parceria.c:442`, `:444`, `:457`, `:461` | PASS |
| C14 | deal 50·k start at centre, 50·k+300 at slot, seat k mod 4, none at 1650; (new) face-down; deal also on next hand and Nova partida | `-R "^deal_animation_timing$"` #54 Passed | `tests/test_parceria.c:505` centre; `:519` slot `< 1.0f`; `:523` gone; `:525` `now == 1650`; **new** `:536` - `CHECK(!v.flying[i].face_up)` (kills F9); **new** `:549-556` - after hand end + reveal, click (next hand) and click on `layout_new_match_button()` (Nova partida) -> `CHECK(m.anim == ANIM_DEAL)`, `flying_count == 1`, id 0 at table centre, `hand_shown[s] == 0` (kills F6) | PASS |
| C15 | `hand_shown` = arrived count every 25 ms | `-R "^deal_shows_arrived_only$"` #55 Passed | `tests/test_parceria.c:575` - `if (v.hand_shown[s] != arrived)` ... `CHECK(false)` | PASS |
| C16 | deal blocks click, auto-pass, CPU until 1650+800 | `-R "^deal_blocks_play$"` #56 Passed | `tests/test_parceria.c:602`, `:604`, `:614`, `:617`, `:628` | PASS |
| C17 | flip 1 / <0.05 / 1 at 350/550/750; face switch at 550; tranque immediate | `-R "^reveal_flips_cpu_hands$"` #57 Passed | `tests/test_parceria.c:663`, `:670`, `:674`; tranque `:686` `CHECK(c.anim == ANIM_REVEAL)`, `:688` `CHECK(v.flying_count == 0)` | PASS |
| C18 | overlay hidden during slide+reveal, hand end and match end; visible at 750 | `-R "^reveal_hides_overlay$"` #58 Passed | `tests/test_parceria.c:706` - `if (v.overlay_visible) ... CHECK(false)`; `:713` - `CHECK(v.overlay_visible)` | PASS |
| C19 | click/Enter during reveal skips to result, stays HAND_OVER; next input starts next hand | `-R "^reveal_skip_on_input$"` #59 Passed | `tests/test_parceria.c:732-734`; `:739-740` | PASS |
| C20 | pulse in [0.35,0.36]..[0.84,0.85], period 800 | `-R "^pulse_period_800$"` #60 Passed | `tests/test_parceria.c:766-767`; `:770` - `fabsf(v.pulse - samples[t]) < 1e-4f` | PASS |
| C21 | 40 `partida-duplas` tests pass; old test files unchanged since 7ab288b | `ctest -L partida-duplas` gave 100% of 40 passed; `git diff --exit-code 7ab288b -- tests/test_core.c tests/test_process.c tests/test_window.c tests/harness.h tests/fixtures.h tests/core_has_no_raylib.cmake` exit 0 | labels in `CMakeLists.txt` on all 4 old groups; diff exit 0 | PASS |
| C22 | first frame of `app_run`: animations on, deal running | `-R "^app_animations_on$"` #62 Passed | `tests/test_app_anim.c:29-31` (unchanged); `src/ui/app.c:41` `ctl_set_animations(&c, true);`, `src/ui/main.c:17` | PASS |
| C23 | animations off after `ctl_init` | `-R "^animations_off_by_default$"` #61 Passed | `tests/test_parceria.c:782` `!c.animate`; `:788` `board_hidden == -1`; `:798` `overlay_visible` | PASS |

Level/sampling: carried from 297ad4b. The fix did not change them.

Precision gaps from round 1:
- The ease-out gap is closed at `:365-366` (F10 killed).
- The C20 1% amplitude tolerance remains. It is a note about the check, not a failing row.

## Coverage

Recomputed at 14eef12 for the rows the round-1 gaps made unproven. Other rows are carried from 297ad4b, because product code is unchanged.

| Set (size) | Recomputed from | Member -> proof | Unproven |
| --- | --- | --- | --- |
| CPU rule chain and its order (9) - verified at 14eef12 | plan AC 4, 5, 6, 8 + Assumptions "Ordem das regras"; `src/core/cpu.c:94-115` | AC4 filter C5 · AC4 fallback C5 · AC5 filter C7 · AC5 fallback C7 `u` · AC4 before AC5 C7 `w` (`:263`) · **AC5 before AC6 (self mode) C7 `x` (`:255-256`, F1 killed)** · AC6 self C8 · AC6 not in partner mode C8 · tiebreaks C10 | - |
| entries into the deal animation (3) - verified at 14eef12 | AC 12 + S3 independent test; `src/core/controller.c:45`, `:58` (via `:109`, `:115`, `:165`) | first hand via `ctl_set_animations` C14/C22 · next hand C14 (`:549-556`, click path) · Nova partida C14 (`:549-556`) - F6 killed | - |
| door 2 `View` fields (Landing literal shape) - verified at 14eef12 | plan Landing door 2 + revised Assumption "Peça em voo"; `controller.h:37-64`, `controller.c:257-261`, `:276-279` | rect C11/C14 · **slide `face_up` C11 `:368`, `:391` (F7 killed) · slide `first/second` C11 `:370`, `:392` (F8 killed) · slide `vertical` C11 `:369` · deal `face_up = false` C14 `:536` (F9 killed)** · hand_shown C15/C23 · board_hidden C12 · flip_scale C17 · overlay_visible C18 · pulse C20 · on/off C22/C23 | - |
| mode choice (3) | carried from 297ad4b | fewer C5 · tie C6 · more C6 | - |
| partner pairs (3) | carried from 297ad4b | N->S, E->W, W->E C9 | - |
| lacks lifecycle (3) | carried from 297ad4b | record C3 · persist C3 · clear C4 | - |
| animation kinds (4) | carried from 297ad4b | deal C14 · slide C11 · reveal C17 · pulse C20 | - |
| timeline transitions other than deal entry (6) | carried from 297ad4b | C11, C14, C17, C19 | - |
| inputs blocked by animation (7) | carried from 297ad4b | C13, C16, C19 | - |
| overlay hidden (2) | carried from 297ad4b | C18 | - |
| slide origin (2) | carried from 297ad4b | South C11 · CPU row C11 | - |
| door 1 (1) | carried from 297ad4b | C2 | - |
| startup config: animations (2 assemblies) | carried from 297ad4b | `app_run` C22 · `ctl_init` C23 | - |

Note:
- The next-hand deal is asserted only through the click path. The Enter path (`controller.c:165`) calls the same `new_hand_started` helper, and F6 removed the deal from that shared helper and was killed. A separate Enter-path deal case would add no new information about the helper.
- The revised plan Assumption (slide face-up with destination values, deal face-down) now matches the code and is asserted.

Swept rows: carried from 297ad4b (idempotency `controller.c:104-107`, data lifecycle `rules.c:71`, state transitions present).

Deal start size note: carried from 297ad4b. It is consistent with AC 12.

## Test policy rows

verified at 14eef12 (the three rows unmet in round 1 were re-judged).

| Row | Files it classifies | Required proof | Expectation met |
| --- | --- | --- | --- |
| Decide, reached across a boundary (`app_run`) | `src/core/controller.c` (timeline) | boundary C22 · own level C11-C20, C23 | yes - boundary C22; own level now covers deal entry on next hand and Nova partida (C14 `:549-556`) in addition to the transitions and blocked inputs carried from 297ad4b |
| Decide, no boundary | `src/core/cpu.c`, `src/core/rules.c` | own level C2-C10, C3-C4 | yes - the self-mode AC5-before-AC6 row now has its case (C7 `:255-256`); `rules.c` carried from 297ad4b |
| Instrumentation (drawing that only reads `View`) | `src/ui/draw.c` | none of its own; covered by producer's `View` tests | yes - every `View`/`Flying` field `draw.c` reads is now asserted by a controller test (rect, `face_up`, `first/second`, `vertical`, `hand_shown`, `board_hidden`, `flip_scale`, `overlay_visible`, `pulse`) |

## Faults injected

verified at 14eef12 for F1 and F6-F10. F2-F5 are carried from 297ad4b, since their product code and covering tests are unchanged apart from line shifts.

All faults were applied in an isolated worktree `scratchpad/wt2` at HEAD 14eef12, with a separate build dir `bwt`. The real tree's `git status --porcelain` was empty before and identical (empty) after. The worktree was removed with `git worktree remove --force` (`git worktree list` shows only the main tree).

| Mutation | Location | Killed |
| --- | --- | --- |
| F1 (re-injected): self mode applies own-fit filter (AC6) before opponent-pass filter (AC5) | `src/core/cpu.c:96-106` | yes - `cpu_prefers_opponent_pass` failed at `tests/test_parceria.c:255` (`idx == 0`) |
| F2: mode boundary `<` -> `<=` (carried from 297ad4b) | `src/core/cpu.c:92` | yes - `cpu_self_mode_when_partner_not_fewer` failed (now `:224`) |
| F3: `game_pass` does not record the right end (carried from 297ad4b) | `src/core/rules.c:197` | yes - `pass_records_lacks` failed (now `:157`) |
| F4: CPU timer counts during animation (carried from 297ad4b) | `src/core/controller.c:187` | yes - `slide_blocks_timer_and_clicks` failed (now `:442`) |
| F5: pulse period 800 -> 1000 ms (carried from 297ad4b) | `src/core/controller.c:322` | yes - `pulse_period_800` failed (now `:770`) |
| F6: `new_hand_started` no longer starts the deal | `src/core/controller.c:58` | yes - `deal_animation_timing` failed at `tests/test_parceria.c:550` (`m.anim == ANIM_DEAL`) |
| F7: slide flying tile `face_up = false` | `src/core/controller.c:279` | yes - `slide_moves_hand_to_board` failed at `:368` (`v.flying[0].face_up`) |
| F8: slide flying tile `first`/`second` swapped | `src/core/controller.c:276-277` | yes - `slide_moves_hand_to_board` failed at `:370` (`first == 3 && second == 5`) |
| F9: deal flying tile `face_up = true` | `src/core/controller.c:261` | yes - `deal_animation_timing` failed at `:536` (`!v.flying[i].face_up`) |
| F10: ease-out cubic -> linear | `src/core/controller.c:233` | yes - `slide_moves_hand_to_board` failed at `:365` (87.5% position) |

## Gate

`ctest --test-dir build --output-on-failure -V` at 14eef12 gave 62 passed, 0 failed. `ctest -L partida-duplas` gave 40 passed. The C21 `git diff --exit-code 7ab288b -- <old test files>` exited 0.

`validate_verification.py` could not be run because Python is unavailable on this machine. The PASS verdict is consistent with this report's own rows: every `Killed` is yes, every `Unproven` is `-`, every Test policy row is met, and every check has a located `file:line` and a PASS result.
