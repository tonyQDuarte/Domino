# Peças de fora verification

**Verdict**: PASS
**Profile**: standard
**Diff range**: 83cfbc6..8784304
**Round**: 1 - full
**Verifier**: independent sub-agent (author != verifier)

## Binding sources

| Source | Opened | Contradiction | Uncovered |
| --- | --- | --- | --- |
| none - the plan's `Sources` are only the user's chat answers (2026-09-28) recorded in plan.md; there is no design file | n/a | none | - |

## Checks

Proofs run at HEAD 8784304 in one invocation: `ctest --test-dir build --output-on-failure` gave 79/79 passed. Each name below appears on its own line in that output (Test #70-#79 for the new tests, and #1, #2, #7, #8, #26, #31, #54-#56 for the rewritten ones). All 10 new tests are registered in `CMakeLists.txt:116-123` (`add_test(NAME ${t} COMMAND test_fora ${t})`, label `pecas-de-fora`) and defined in `tests/test_fora.c:390-401`.

| Check | Claim | Proof run | Evidence | Result |
| --- | --- | --- | --- | --- |
| C1 | 1000 seeds: 6 per seat, 4 sleeping, 28 = double-six set exactly once | `deal_6_each_4_out` (#70) Passed | `tests/test_fora.c:71` `CHECK(g.hands[s].count == 6)`; `:84` `CHECK(total == 28)`; `:87` `CHECK(seen[a][b] == 1)` | PASS |
| C2 | 4 sleeping tiles face down during the hand; corner rects x>=1000, y+h<=110, in window, no overlap with table/North/East/score | `sleeping_corner_face_down` (#71) Passed | `tests/test_fora.c:108-109` `CHECK(v.sleeping_shown == 4)`, `CHECK(!v.sleeping_face_up)`; `:118-125` `r[i].x >= 1000`, `r[i].y + r[i].h <= 110`, `!rect_overlaps(r[i], table/score/north[k]/east[k])` | PASS |
| C3 | 200 seeds, swapping the sleeping tiles with other seats' tiles does not change `cpu_choose` | `cpu_ignores_sleeping` (#72) Passed | `tests/test_fora.c:157-160` `CHECK(ha == hb)`, `CHECK(ia == ib)`, `CHECK(ea == eb)`; `:171` `CHECK(compared > 1000)` | PASS |
| C4 | tile k leaves the center at 50k, lands at 50k+300; k<24 to hand slot, k>=24 to corner slot k-24; nothing flying at 1650 | `deal_sends_four_to_corner` (#73) Passed | `tests/test_fora.c:213` center at start; `:227` `dist(... cx(slot), cy(slot)) >= 1.0f` fails, with `deal_slot` returning `z[k - 24]` for k>=24 (`:178-180`); `:234` gone at landing; `:236-237` `now == 1650`, `v.flying_count == 0` | PASS |
| C5 | 0..1700 ms every 25 ms, `sleeping_shown` = number of k>=24 that have landed | `deal_corner_shows_arrived_only` (#74) Passed | `tests/test_fora.c:252-257` expected count over `k = 24..27`, `if (v.sleeping_shown != arrived) CHECK(false)` | PASS |
| C6 | first hand: turn goes to the holder of the highest dealt double; only that double is accepted; the CPU opener picks it; at least one seed has [6\|6] out | `first_hand_highest_double` (#75) Passed | `tests/test_fora.c:274` `CHECK(g.turn == holder)`; `:284-285` `!game_play(&g, i, END_LEFT)`, `board.count == 0`; `:290` `CHECK(ci == idx)`; `:292` board holds `d\|d`; `:294` `CHECK(six_out > 0)` | PASS |
| C7 | South clicks another tile on the first hand: empty table and `Jogada inválida`; a click on the highest double opens | `south_first_open_invalid` (#76) Passed | `tests/test_fora.c:318` `CHECK(c.game.board.count == 0)`; `:322` `strcmp(v.message, "Jogada inválida") == 0`; `:324-325` `board.count == 1`, `line[0].left == d` | PASS |
| C8 | with animations, sleeping tiles face down at 350 and 549 ms, face up at 550; same `flip_scale` as the CPUs | `reveal_flips_sleeping` (#77) Passed | `tests/test_fora.c:342` `!v.sleeping_face_up` at 350; `:346-347` still down at 549, `v.flip_scale < 0.01f`; `:350-351` `v.sleeping_face_up` and `== v.face_up[SEAT_EAST]` at 550. "Same scale" holds by construction: one `View.flip_scale` field (`src/core/controller.h:61`), which `src/ui/draw.c:128-129` applies to the corner row | PASS |
| C9 | animations off: face up right after the winning click | `sleeping_face_up_without_animation` (#78) Passed | `tests/test_fora.c:363-364` `CHECK(v.sleeping_face_up)`, `CHECK(v.sleeping_shown == 4)` | PASS |
| C10 | hand-end and match-end overlays: face up, all 4 shown | `sleeping_shown_with_overlay` (#79) Passed | `tests/test_fora.c:373` phase `HAND_OVER` / `MATCH_OVER`; `:378-380` `overlay_visible`, `sleeping_face_up`, `sleeping_shown == 4`; `:383` still true after 5000 ms | PASS |
| C11 | full suite green, including the rewritten old tests; no other old test changed | `ctest --test-dir build --output-on-failure` 79/79; `git diff --stat 83cfbc6 -- tests/test_process.c tests/test_window.c tests/test_app_anim.c tests/test_carroca.c tests/harness.h tests/fixtures.h tests/core_has_no_raylib.cmake` produced no output, exit 0 | rewritten tests assert the new rule: `tests/test_core.c:78` `count == 6`, `:96` `seen[a][b] == 1` (with sleeping counted, `:87-92`); `:220` `g.turn == holder_of_opening_double(&g)`; `:240` `h->count == 6`; `:680` `count == 6`; `:847` `turn == holder_of_opening_double`; `tests/test_parceria.c:524` slot check with `hand_slot` sending k>=24 to the corner (`:471-475`); `:580` `hand_shown` excludes k>=24; `:624` South opens with his highest double | PASS |

Supersession judgment (C11):

- (a) **Not weakened.** Each rewritten test now asserts the new rule. `deal_28_unique_7_each` also folds the 4 sleeping tiles into the uniqueness count. `first_hand_*` and `new_match_resets` use a helper (`tests/test_core.c:13-31`) that computes the highest double in the hands. `deal_animation_timing` now also asserts the corner destination. `deal_shows_arrived_only` asserts that hands do not count k>=24. `deal_blocks_play` opens with South's highest double on a seed where South opens. `deal_same_seed_same_hands` only changed its loop bound from 7 to `HAND_SIZE` (`:119`), and its assertion `:124` `CHECK(differ)` is unchanged.
- (b) **Only those 9 changed.** `git diff 83cfbc6..HEAD -- tests` touches only `test_core.c`, `test_parceria.c` and the new `test_fora.c`. The changed helpers are used only by tests in the list: `holder_of_opening_double` at `test_core.c:220,847`, `highest_double_value` at `:231`, and `hand_slot` only at `test_parceria.c:523`. The three old `checks.md` files gained only `> ` lines: filtering the diff's `+`/`-` lines for anything that is not `+> ` printed nothing. The `CMakeLists.txt` change is purely additive (`:111-123`).
- (c) **Every other old test passes.** All 69 pre-existing tests passed at HEAD.

## Coverage

| Set (size) | Recomputed from | Member -> proof | Unproven |
| --- | --- | --- | --- |
| deal destination by k (28 = 24 hand slots + 4 corner slots) | plan AC 4 + `src/core/controller.c:255-260` (`k < dealt` branch) | all 28 k iterated in C4 (`test_fora.c:219-235`); hand slots: k<24 · corner: k>=24 | - |
| arrival count shown (hand vs corner) | plan AC 5 + `controller.c:255-258` | corner C5 · hands: rewritten `deal_shows_arrived_only` (`test_parceria.c:579-580`) | - |
| highest dealt double, reachable values (4: 6, 5, 4, 3) | plan AC 6 + `src/core/rules.c:66-72`. At most 4 doubles can be out, so at least 3 stay in hands and the highest is >= 3. The author's "7 values" includes 3 unreachable ones | C6 over seeds 0-999. A verifier probe linked against `libdomino_core.a` counted d=6: 860, d=5: 123, d=4: 15, d=3: 2, so every reachable value is exercised. The `d > 4` mutant below, which only differs at d=3, was killed | - |
| first-hand opening acceptance (2: the opening double vs every other tile) | `rules.c:127-128` `game_fits` | accepted C6 `:291`, C7 `:324` · refused C6 `:284`, C7 `:317-318` | - |
| CPU first-hand filter (2: top double vs other) | `src/core/cpu.c:32-41` | C6 `:290` `ci == idx`, including seeds where [6\|6] is out (`:294`) | - |
| who tries a wrong opening (2) | plan AC 7 + `game_play` / `ctl_click` | South C7 · turn holder via `game_play` C6 | - |
| sleeping-tile state (4: hidden in play, flipping, up without animation, up with overlay) | `controller.c:310-311` + plan AC 2/8/9/10 | C2 · C8 · C9 · C10 | - |
| overlay kind (2) | plan AC 10, `GamePhase` HAND_OVER / MATCH_OVER | C10 loop `match = 0, 1` (`test_fora.c:370-373`) | - |
| corner neighbours (4: table, North row, East column, score) | `src/core/layout.c:20,125,131,141-148` + plan AC 2 | C2 `test_fora.c:121-125` | - |
| superseded old tests (9) | plan `Impact` row (8 named + `deal_same_seed_same_hands`) | all 9 ran and passed (#1, #2, #7, #8, #26, #31, #54, #55, #56) | - |
| seat order S, E, N, W x6 | plan AC 4 | C4 via `deal_slot` `k % 4`, `k / 4` (`test_fora.c:182-187`) | - |

Sweep notes, none of them a gap:

- `Surface`, `Relations` and `Landing` are `None`, so there are no statuses or routes to enumerate.
- `checks.md` C11 and the Coverage row say 8 superseded tests. `plan.md` Impact records the 9th (`deal_same_seed_same_hands`). The C11 text "nenhum outro teste antigo muda" is stale against the plan. This is a precision note on the checks: the 9th change is recorded and approved in the plan and in the `partida-duplas` note.
- The "ordem das peças de fora" Assumption (shuffle order) has no assertion. It is an unconfirmed assumption, not an acceptance criterion.

## Test policy rows

| Row | Files it classifies | Required proof | Expectation met |
| --- | --- | --- | --- |
| Decide, sem fronteira | `src/core/rules.c` (`start_hand` 24+4 split, highest-double search, first-hand `game_fits`), `src/core/cpu.c` (`list_moves` first-hand filter) | own level: C1 (`game_new_match`), C6 (`game_new_match`, `game_play`, `cpu_choose`) | yes - each decision-table row is asserted: split, every reachable double value (6/5/4/3, sampled and confirmed), accepted vs refused opening, CPU's top-double pick; mutants on split, search bound and CPU filter all killed |
| Decide, alcançado pelo controlador | `src/core/controller.c` (`view_deal` destination and count, `ctl_view` `sleeping_shown` / `sleeping_face_up`); `rules.c` first-hand rule reached through `ctl_click` | own level: C4, C5, C8, C9, C10 (controller is the own level) · through the controller: C7 for the first-hand rule as the player sees it | yes - the controller path (South click -> `Jogada inválida`) is asserted, and the full rules sit at their own level in C6; destination and face mutants killed |
| Instrumentação | `src/core/layout.c` (`layout_sleeping`), `src/ui/draw.c` (corner row) | none of its own; covered by `View` tests | yes - C2 asserts `layout_sleeping` geometry. `draw.c:123-136` reads only `v->sleeping_shown`, `v->sleeping_face_up`, `v->flip_scale` and `g->sleeping` |

## Swept existing

- validation -> C7: the refusal exists at `rules.c:127-128` (`game_fits`). Present.
- data lifecycle -> C1: `start_hand` refills `g->sleeping` (`rules.c:63-64`) and is called from both `game_new_match` (`:107`) and `game_next_hand` (`:120`), so the constraint is in the code. Precision note: C1 asserts only the next hand's counts (`test_fora.c:93,96`), not that its sleeping tiles form a valid new partition.
- state transitions -> C8, C10: `controller.c:311` `sleeping_face_up = g->phase != PHASE_PLAYING && !hidden_reveal`. Present.
- The other rows are `n/a` (user-approved policy).

## Faults injected

Isolated worktree at `<scratchpad>/wt` (`git worktree add ... HEAD`), with its own build dir `wt/bld`. The real tree's `git status --porcelain` was empty before and identical (empty) after. The worktree was removed with `git worktree remove --force` and pruned. `git stash` was not used.

| Mutation | Location | Narrowest proof | Killed |
| --- | --- | --- | --- |
| sleeping tiles taken from `set[i]` (overlapping South's hand) instead of `set[24 + i]` | `src/core/rules.c:64` | `deal_6_each_4_out` Failed | yes |
| highest-double search stops at 4 (`d > 0` -> `d > 4`): wrong opener only when [6\|6], [5\|5] and [4\|4] are all out | `src/core/rules.c:69` | `first_hand_highest_double` Failed | yes |
| CPU first-hand filter reverted to `[6\|6]` only | `src/core/cpu.c:41` | `first_hand_highest_double` Failed | yes |
| deal destination for k>=24 always `corner[0]` | `src/core/controller.c:260` | `deal_sends_four_to_corner` Failed | yes |
| `sleeping_face_up` ignores `hidden_reveal` (face up during the first half of the flip) | `src/core/controller.c:311` | `reveal_flips_sleeping` Failed | yes |

Not mutated because of the cap of 5: `sleeping_shown` counting (`controller.c:255-258`, proof C5) and `game_fits` first-hand (`rules.c:127`, proofs C6 and C7).

## Gate

`ctest --test-dir build --output-on-failure` - 79 passed, 0 failed (HEAD 8784304). In the worktree, `ctest -L pecas-de-fora` also passed 10/10 after all mutants were reverted. `validate_verification.py` was not run: Python is not available on this machine, so the gate script result is absent.
