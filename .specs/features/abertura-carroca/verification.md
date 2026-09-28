# Abertura com carroça verification

**Verdict**: PASS
**Profile**: standard
**Diff range**: ef5fecf..ba64cee
**Round**: 1 - full
**Verifier**: independent sub-agent (author != verifier)

## Binding sources

No binding source beyond the user's chat answers recorded in `plan.md` `## Sources` (no design file); nothing to open, step 1 does not apply under `standard`.

## Checks

All proofs run at `ba64cee` in one invocation:
`ctest --test-dir build -R "^(open_requires_double|south_open_non_double_invalid|cpu_opens_with_double|first_hand_double_six_starts|first_hand_rejects_non_double_six|opener_order_after_batida|next_hand_uses_opener_order|tranque_winner_leads_opening|tranque_tie_leads_from_opener|next_hand_winner_starts|next_hand_after_tranque)$" --output-on-failure` - 11/11 listed individually as Passed (tests #7, #8, #27, #28, #63-#69), exit 0. Every name is registered in `CMakeLists.txt` (lines 51, 56-57 for `test_core`; 100-110 for `test_carroca`) and dispatched by name in the `cases[]` tables (`tests/test_core.c:936-957`, `tests/test_carroca.c:245-253`); `tests/harness.h:29` rejects unknown names, so no filter can match an empty test.

| Check | Claim | Proof run | Evidence | Result |
| --- | --- | --- | --- | --- |
| C1 | 7 carroças accepted on empty board (non-first hand); non-double refused on both ends, board stays empty | `-R "^open_requires_double$"` passed | `tests/test_carroca.c:44` loop `d = 0..6`; `:48-50` - `CHECK(!game_play(&g, 1, END_LEFT)); CHECK(!game_play(&g, 1, END_RIGHT)); CHECK(g.board.count == 0);`; `:52-54` - `CHECK(game_play(&g, 0, END_LEFT)); ... CHECK(g.board.line[0].left == d && g.board.line[0].right == d);` | PASS |
| C2 | South click on `[2|5]` leaves board empty + `Jogada inválida`; then `[3|3]` opens | `-R "^south_open_non_double_invalid$"` passed | `tests/test_carroca.c:69` - `CHECK(c.game.board.count == 0);`; `:73` - `CHECK(strcmp(v.message, "Jogada inválida") == 0);`; `:75-76` - `CHECK(c.game.board.count == 1); CHECK(c.game.board.line[0].left == 3 && ...right == 3);` | PASS |
| C3 | `cpu_decide` with `[6|5]`,`[2|2]`,`[1|4]` picks `[2|2]`; 200 seeds, every later opening is a double | `-R "^cpu_opens_with_double$"` passed | `tests/test_carroca.c:97` - `CHECK(idx == 1);`; `:110` - `CHECK(g.board.line[0].left == g.board.line[0].right);`; `:124` - `CHECK(openings > 200);` (sampling guard) | PASS |
| C4 | First hand still opens with `[6|6]` by its holder | `-R "^first_hand_double_six_starts$"`, `-R "^first_hand_rejects_non_double_six$"` passed | `tests/test_core.c:201` - `CHECK(g.turn == holder_of_double_six(&g));`; `:216-218` - `CHECK(!game_play(&g, i, END_LEFT)); CHECK(!game_play(&g, i, END_RIGHT)); CHECK(g.board.count == 0);` | PASS |
| C5 | `game_choose_opener`: lead, partner, next, next's partner, for all 4 leads | `-R "^opener_order_after_batida$"` passed | `tests/test_carroca.c:134` loop over 4 seats; `:143` `== lead`; `:146` `== partner`; `:149` `== next`; `:152` `CHECK(game_choose_opener(&g, lead) == next_partner);` | PASS |
| C6 | After East's batida, `game_next_hand` gives the turn per the order (200 seeds) | `-R "^next_hand_uses_opener_order$"` passed | `tests/test_carroca.c:174` - `CHECK(g.turn == want);` with `want = expected_opener(&g, SEAT_EAST)` (`:172`, oracle at `:19-26`); `:179` - `CHECK(not_batedor > 0);` | PASS |
| C7 | Tranque won by opener's team -> search starts at opener; won by other team -> at next seat | `-R "^tranque_winner_leads_opening$"` passed | `tests/test_carroca.c:205` `next_opener == SEAT_SOUTH` (S opens, Nós win); `:212` `== SEAT_NORTH` (E opens, Nós win); `:219` `== SEAT_WEST` (N opens, Eles win); `:224` `== SEAT_WEST` (W opens, Eles win); `:207`/`:214` `g.turn == expected_opener(...)` | PASS |
| C8 | Tied tranque -> search starts at opener | `-R "^tranque_tie_leads_from_opener$"` passed | `tests/test_carroca.c:235` - `CHECK(g.result == RESULT_TRANCADO && g.result_team < 0);`; `:236` - `CHECK(g.next_opener == openers[k]);` for all 4 openers | PASS |
| C9 | Rewritten `next_hand_winner_starts` / `next_hand_after_tranque` assert the new rule: turn per carroça order, double accepted, non-double refused | `-R "^next_hand_winner_starts$"`, `-R "^next_hand_after_tranque$"` passed | `tests/test_core.c:733` - `CHECK(g.turn == carroca_opener(&g, SEAT_EAST));`; `:747` - `CHECK(g.turn == carroca_opener(&g, SEAT_WEST));`; helper `:710-714` - `CHECK(!game_play(g, other, END_LEFT)); CHECK(g->board.count == 0); ... CHECK(game_play(g, dbl, END_LEFT));` | PASS |

Level and sampling: all claims are core-rule claims, proven at the core level, plus the controller path for C2 - no level gap. C3 and C6 use seeded sampling with explicit non-vacuity guards (`openings > 200`, `not_batedor > 0`); the 4th position of the order, which sampling may miss, is proven deterministically by C5.

## Coverage

| Set (size) | Recomputed from | Member -> proof | Unproven |
| --- | --- | --- | --- |
| carroças accepted at opening (7) | plan AC 1 (`[0|0]`..`[6|6]`), `rules.c:116` `tile_is_double` | C1 loop `test_carroca.c:44` over d=0..6, each asserted at `:52-54` | - |
| who tries to open with a non-double (2) | plan AC 2, AC 3; code paths `controller.c` -> `game_fits`, `cpu.c:35` | Sul C2 (`:69-73`) · CPU C3 (`:97`, `:110`) | - |
| opener order positions (4) | plan AC 5; `rules.c:84-88` | lead `:143` · partner `:146` · next `:149` · next's partner `:152` (C5) | - |
| batedor seat (4) | `Seat` enum `domino.h:16` | S/E/N/W via loop `test_carroca.c:134` (C5); end-to-end wiring `rules.c:193` via East C6 | - |
| search start (4 branches) | plan AC 5/6/7; `rules.c:193`, `rules.c:223-224` | batida C6 `:174` · tranque, opener in winning team C7 `:205`,`:224` · tranque, opener in other team C7 `:212`,`:219` · tie C8 `:236` | - |
| hand type (2) | plan AC 1, AC 4; `rules.c:116`, `cpu.c:35` (ternary on `first_hand`) | first hand `[6|6]` C4 · later hand double C1 (rules) and C3 (cpu) | - |
| plan acceptance criteria (7) | plan `## Criteria` AC 1-7 | AC1 C1 · AC2 C2 · AC3 C3 · AC4 C4 · AC5 C5, C6 · AC6 C7 · AC7 C8 | - |
| superseded old tests (2) | plan `Impact` rows; `git diff ef5fecf..HEAD -- tests/test_core.c` | `next_hand_winner_starts` C9 · `next_hand_after_tranque` C9 | - |
| Observable decisions with a landing (2) | plan `## Observable` | empty state AC 1 -> C1 · error state AC 2 -> C2 | - |

Sweep: `Relations`, `Surface`, `Landing` are `None`; no status codes, routes or stored fields. Out-of-scope rows and unconfirmed Assumptions (CPU picks the highest-sum double among several) are not criteria and owe no row.

## Test policy rows

| Row | Files it classifies | Required proof | Expectation met |
| --- | --- | --- | --- |
| Decide, sem fronteira | `src/core/rules.c` (`game_choose_opener`, `next_opener` in `game_pass`, `game_fits` empty board), `src/core/cpu.c` (`list_moves` empty-board filter) | own level: C5 (4 positions x 4 leads), C7+C8 (3 starts, both teams), C1+C4 (2 hand types), C3 direct `cpu_decide` | yes |
| Decide, alcançado pelo controlador | `src/core/controller.c` (unchanged; South click via `game_fits`) | controller: C2 refusal `:69-73` and acceptance `:75-76` · own level: C1 whole table | yes |

## Swept existing

- validation (C1, C2): constraint present at `rules.c:116` and reached by the controller - confirmed.
- state transitions (C5-C8): `next_opener` written at `rules.c:193` (batida) and `rules.c:223-224` (tranque), consumed only at `rules.c:64` - confirmed.
- data lifecycle (`next_opener` lives from one hand to the next): same three lines; no other reader (`grep next_opener src`) - confirmed.
- remaining rows are `n/a` policy.

## Supersession of partida-duplas AC 28/29

- (a) Rewritten tests assert the new rule, not a weakened one. Old: `CHECK(g.turn == SEAT_EAST)` / `== SEAT_WEST` and acceptance of a non-`[6|6]` tile. New: turn equals the carroça order from the batedor / tranque lead (`test_core.c:733`, `:747`), plus refusal of a non-double with empty board (`:710-712`) and acceptance of a double (`:714-715`). Fault M3 killed both; fault M5 killed neither (with seeds 9 and 1 the lead already holds a double), so these two tests only exercise position 1 of the order - the other positions are carried by C5/C6.
- (b) `git diff ef5fecf..HEAD` under `tests/` touches only `test_core.c` (two helpers added, two test bodies rewritten; nothing else) and the new `test_carroca.c`. Spec diffs to `partida-duplas/checks.md` and `cpu-parceria-animacao/checks.md` are 3 added lines, all starting with `> `, 0 deletions. `git diff 39873e8` over the C21 file list shows only `tests/test_core.c` changed, matching the note.
- (c) Full suite at HEAD: 69/69 passed (partida-duplas 40, cpu-parceria-animacao 22, abertura-carroca 7).

## Faults injected

Isolated worktree at HEAD under the session scratchpad, separate build dir `bmut`; each mutant applied, rebuilt, suite run (window/CLI tests excluded), reverted. Real tree `git status --porcelain` identical before and after; worktree removed with `git worktree remove --force`.

| Mutation | Location | Killed |
| --- | --- | --- |
| order swapped: next before partner | `src/core/rules.c:84` | yes - `opener_order_after_batida` (C5), also `next_hand_uses_opener_order` |
| tranque lead reverted to old rule (`next_opener = opener` always) | `src/core/rules.c:223` | yes - `tranque_winner_leads_opening` (C7) |
| empty board accepts any tile outside first hand (old rule) | `src/core/rules.c:116` | yes - `open_requires_double` (C1), `south_open_non_double_invalid` (C2), `next_hand_winner_starts`, `next_hand_after_tranque` (C9) |
| CPU opening filter reverted to any tile | `src/core/cpu.c:35` | yes - `cpu_opens_with_double` (C3) |
| `start_hand` uses `next_opener` directly, skipping `game_choose_opener` | `src/core/rules.c:64` | yes - `next_hand_uses_opener_order` (C6), `cpu_opens_with_double` |

Cap of 5 reached. The tie branch of `rules.c:223` (C8) was not faulted on its own; C8's assertion `test_carroca.c:236` targets `next_opener` directly. C4 covers unchanged code.

## Observations (non-blocking)

1. Plan `Impact` says the two rewritten tests "passam a montar mãos fixas"; they use seeded random deals plus a test-side oracle (`tests/test_core.c:688-696`). C9's claim does not require fixed hands, and the oracle is independent of `rules.c`, but the tests only reach position 1 of the order (see M5).
2. `next_hand_after_tranque` relies on a comment (`tests/test_core.c:746`) for "Eles ganharam" and does not assert `result_team` or `next_opener`; C7 carries that.
3. `opens_only_with_double` skips the refusal when the opener's hand is all doubles (`tests/test_core.c:710`); deterministic for the fixed seeds, refusal path confirmed reached by fault M3.

## Gate

`ctest --test-dir build --output-on-failure` - 69 passed, 0 failed. `validate_verification.py` could not run: Python is unavailable on this machine.
