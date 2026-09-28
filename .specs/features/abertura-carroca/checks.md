# Abertura com carroça checks

Profile: standard
Plan: `.specs/features/abertura-carroca/plan.md`

9 checks in 2 slices · 0 one-way doors · 0 open, of which 0 block

Build e PATH como em `AGENTS.md`. Os testes novos ficam em `tests/test_carroca.c` (executável
`test_carroca`). A ordem de quem abre fica numa função pura,
`Seat game_choose_opener(const Game *g, Seat lead)`, para que cada posição da ordem seja provada
com mãos fixas.

## Checks

### S1 - Toda mão abre com carroça · 4 files · ~15 KB · ~4k

- [ ] **C1** - Numa mão que não é a primeira, com a mesa vazia, cada uma das 7 carroças `[0|0]` a `[6|6]` é aceita por `game_play` como abertura, e uma peça que não é carroça é recusada nas duas pontas, com a mesa continuando vazia (AC 1)
Proof: `ctest --test-dir build -R "^open_requires_double$" --output-on-failure`

- [ ] **C2** - Na abertura de uma mão que não é a primeira, um clique do Sul em `[2|5]` deixa a mesa vazia e mostra `Jogada inválida`; o clique seguinte em `[3|3]` abre a mão (AC 2)
Proof: `ctest --test-dir build -R "^south_open_non_double_invalid$" --output-on-failure`

- [ ] **C3** - Com a mesa vazia fora da primeira mão, `cpu_decide` com `[6|5]`, `[2|2]`, `[1|4]` escolhe `[2|2]`; e em 200 seeds, jogando mãos inteiras, a primeira peça de toda mão depois da primeira é uma carroça (AC 3)
Proof: `ctest --test-dir build -R "^cpu_opens_with_double$" --output-on-failure`

- [ ] **C4** - A primeira mão da partida continua abrindo com `[6|6]` por quem o tem (AC 4)
Proof: `ctest --test-dir build -R "^first_hand_double_six_starts$" --output-on-failure`
Proof: `ctest --test-dir build -R "^first_hand_rejects_non_double_six$" --output-on-failure`

### S2 - A vez de abrir respeita a dupla vencedora · 3 files · ~25 KB · ~6k

- [ ] **C5** - `game_choose_opener` a partir de quem bateu devolve: quem bateu, se ele tem carroça; senão o parceiro, se tem; senão o jogador seguinte, se tem; senão o parceiro do seguinte. Isso vale para cada um dos 4 assentos como quem bateu (AC 5)
Proof: `ctest --test-dir build -R "^opener_order_after_batida$" --output-on-failure`

- [ ] **C6** - Depois de uma batida do Leste, `game_next_hand` dá a vez a `game_choose_opener(g, Leste)`, conferido em 200 seeds contra a ordem calculada pelo teste (AC 5)
Proof: `ctest --test-dir build -R "^next_hand_uses_opener_order$" --output-on-failure`

- [ ] **C7** - Num tranque ganho pela dupla de quem abriu a mão, a procura começa em quem abriu; num tranque ganho pela outra dupla, começa no jogador seguinte a quem abriu (AC 6)
Proof: `ctest --test-dir build -R "^tranque_winner_leads_opening$" --output-on-failure`

- [ ] **C8** - Num tranque empatado, a procura começa em quem abriu a mão trancada (AC 7)
Proof: `ctest --test-dir build -R "^tranque_tie_leads_from_opener$" --output-on-failure`

- [ ] **C9** - Os testes `next_hand_winner_starts` e `next_hand_after_tranque` de `partida-duplas` (C29, C30), reescritos para a regra nova, passam: a vez vai para quem a ordem da carroça indica, uma carroça é aceita e uma peça que não é carroça é recusada (substitui AC 28 e AC 29 de `partida-duplas`)
Proof: `ctest --test-dir build -R "^next_hand_winner_starts$" --output-on-failure`
Proof: `ctest --test-dir build -R "^next_hand_after_tranque$" --output-on-failure`

## Coverage

| Set (size) | Member -> proof | Unproven |
| --- | --- | --- |
| carroças aceitas na abertura (7) | C1, table-driven over `[0|0]`..`[6|6]` | - |
| quem tenta abrir com peça errada (2) | Sul C2 · CPU C3 | - |
| ordem de quem abre (4) | quem bateu C5 · parceiro C5 · seguinte C5 · parceiro do seguinte C5 | - |
| quem bateu (4 assentos) | Sul C5 · Leste C5 · Norte C5 · Oeste C5 | - |
| início da procura (4) | batida C5, C6 · tranque, quem abriu na dupla vencedora C7 · tranque, quem abriu na outra dupla C7 · tranque empatado C8 | - |
| tipo de mão (2) | primeira mão `[6|6]` C4 · mão seguinte carroça C1 | - |
| testes antigos substituídos (2) | `next_hand_winner_starts` C9 · `next_hand_after_tranque` C9 | - |

- Nenhum check cita código de saída; `Surface` e `Landing` são `None`

## Test policy

| Code | Required proofs | Coverage expectation |
| --- | --- | --- |
| Decide, sem fronteira | um no próprio nível | um caso afirmado por linha da tabela de decisão |
| Decide, alcançado pelo controlador (clique do Sul) | um no próprio nível **e** um pelo controlador | no controlador: a recusa e o aceite; no próprio nível: a tabela inteira |

Evidence:

- `src/core/rules.c` (`game_choose_opener`, escolha de `next_opener`, `game_fits` com mesa vazia): ordem de 4 posições, 3 inícios de procura e 2 tipos de mão, cerca de 9 pontos de decisão -> decide, sem fronteira
- `src/core/cpu.c` (`list_moves` com mesa vazia): 1 filtro novo -> decide, sem fronteira
- `src/core/controller.c`: não muda; a recusa do Sul passa pelo `game_fits` -> provada pelo controlador em C2
- análogo no repo: `tests/test_parceria.c` prova a tabela da CPU com um cenário fixo por regra (C5 a C10 de `cpu-parceria-animacao`)

Cost: 9 provas em 2 arquivos de teste. Sem estas linhas, a ordem de 4 posições seria provada só pelo
C6, que usa mãos sorteadas e pode nunca cair na quarta posição.

## Swept

- validation: C1, C2 - peça que não é carroça é recusada
- failure modes: n/a - nenhuma operação que possa falhar no meio
- idempotency: n/a - nenhuma operação repetível nova
- authorization: n/a - jogo local de um só usuário
- concurrency: n/a - laço único, sem threads
- data lifecycle: n/a - nada é gravado; `next_opener` vale só da mão atual para a seguinte (C6)
- dependency failure: n/a - nenhuma dependência nova
- state transitions: C5, C6, C7, C8 - passagem de uma mão para a seguinte
- observability: n/a - nenhum requisito de log

## Handoff

- Arquivos tocados: `rules.c` (6 KB), `domino.h` (3 KB), `cpu.c` (4 KB), `tests/test_core.c` (só os 2
  testes substituídos, ~1 KB de mudança num arquivo de 30 KB), `tests/test_carroca.c` (novo, ~10 KB),
  `CMakeLists.txt` (3 KB), artefatos (~15 KB). Total ~70 KB / 4 = ~18k, abaixo do orçamento de 150k -
  one builder
