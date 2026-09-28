# Peças de fora checks

Profile: standard
Plan: `.specs/features/pecas-de-fora/plan.md`

11 checks in 4 slices · 0 one-way doors · 0 open, of which 0 block

Build e PATH como em `AGENTS.md`. Os testes novos ficam em `tests/test_fora.c` (executável
`test_fora`).

## Checks

### S1 - Cada jogador recebe 6 e 4 ficam de fora · 6 files · ~30 KB · ~8k

- [x] **C1** - Em 1000 seeds, cada assento recebe 6 peças, `sleeping` tem 4, e as 28 peças de mãos mais `sleeping` são exatamente o conjunto `[a|b]` com 0 <= a <= b <= 6, cada uma uma vez (AC 1)
Proof: `ctest --test-dir build -R "^deal_6_each_4_out$" --output-on-failure`

- [x] **C2** - Durante a mão, `ctl_view` mostra 4 peças de fora de costas; os 4 retângulos de `layout_sleeping` ficam no canto superior direito (x >= 1000, y + h <= 110), dentro da janela, e não encostam na área da mesa, na fileira do Norte, na coluna do Leste nem no placar (AC 2)
Proof: `ctest --test-dir build -R "^sleeping_corner_face_down$" --output-on-failure`

- [x] **C3** - Em 200 seeds, em cada vez de CPU, trocar as 4 peças de fora com peças das mãos dos outros três assentos não muda a peça nem a ponta escolhidas por `cpu_choose` (AC 3)
Proof: `ctest --test-dir build -R "^cpu_ignores_sleeping$" --output-on-failure`

- [x] **C4** - Com animações, a peça k sai do centro da mesa aos 50·k ms e chega aos 50·k + 300 ms: para k < 24, na mão do assento k mod 4, posição k div 4; para k >= 24, na posição k − 24 do canto das peças de fora; aos 1650 ms não há peça em voo (AC 4)
Proof: `ctest --test-dir build -R "^deal_sends_four_to_corner$" --output-on-failure`

- [x] **C5** - De 0 a 1700 ms, a cada 25 ms, `sleeping_shown` é o número de peças k >= 24 que já chegaram (50·k + 300 <= t) (AC 5)
Proof: `ctest --test-dir build -R "^deal_corner_shows_arrived_only$" --output-on-failure`

### S2 - A primeira mão abre com a maior carroça distribuída · 3 files · ~15 KB · ~4k

- [x] **C6** - Em 1000 seeds, na primeira mão a vez é de quem tem a maior carroça distribuída, só essa carroça é aceita como abertura (as outras carroças e as peças comuns são recusadas), a CPU que abre escolhe essa carroça, e em pelo menos uma seed o `[6|6]` está entre as peças de fora (AC 6)
Proof: `ctest --test-dir build -R "^first_hand_highest_double$" --output-on-failure`

- [x] **C7** - Numa primeira mão em que o Sul abre, um clique em outra peça deixa a mesa vazia e mostra `Jogada inválida`; o clique na maior carroça abre a mão (AC 7)
Proof: `ctest --test-dir build -R "^south_first_open_invalid$" --output-on-failure`

### S3 - As peças de fora aparecem no fim · 2 files · ~10 KB · ~3k

- [x] **C8** - Numa batida com animações, as peças de fora estão de costas aos 350 ms e aos 549 ms e de face aos 550 ms, e `flip_scale` é o mesmo das CPUs (AC 8)
Proof: `ctest --test-dir build -R "^reveal_flips_sleeping$" --output-on-failure`

- [x] **C9** - Numa batida com animações desligadas, as 4 peças de fora estão de face logo depois do clique que bate (AC 9)
Proof: `ctest --test-dir build -R "^sleeping_face_up_without_animation$" --output-on-failure`

- [x] **C10** - Com o quadro de fim de mão ou de fim de partida visível, as peças de fora estão de face e as 4 aparecem (AC 10)
Proof: `ctest --test-dir build -R "^sleeping_shown_with_overlay$" --output-on-failure`

### S4 - O resto continua igual · 4 files · ~5 KB · ~1k

- [x] **C11** - A suíte inteira passa, incluindo os 8 testes antigos reescritos para a regra nova (`deal_28_unique_7_each`, `first_hand_double_six_starts`, `first_hand_rejects_non_double_six`, `hand_end_overlay`, `new_match_resets`, `deal_animation_timing`, `deal_shows_arrived_only`, `deal_blocks_play`); nenhum outro teste antigo muda (AC 11)
Proof: `ctest --test-dir build --output-on-failure`
Proof: `git diff --stat 83cfbc6 -- tests/test_process.c tests/test_window.c tests/test_app_anim.c tests/test_carroca.c tests/harness.h tests/fixtures.h tests/core_has_no_raylib.cmake`

## Coverage

| Set (size) | Member -> proof | Unproven |
| --- | --- | --- |
| destino da peça na distribuição (2) | mão C4 · canto C4 | - |
| maior carroça distribuída (7 valores) | C6, amostrado em 1000 seeds, com pelo menos um caso sem `[6\|6]` | - |
| quem abre a primeira mão com peça errada (2) | Sul C7 · CPU C6 | - |
| estado das peças de fora (4) | de costas durante a mão C2 · virando C8 · de face sem animação C9 · de face com o quadro C10 | - |
| quadro visível (2) | fim de mão C10 · fim de partida C10 | - |
| vizinhos do canto (4) | mesa C2 · fileira do Norte C2 · coluna do Leste C2 · placar C2 | - |
| testes antigos substituídos (8) | os 8 nomes de C11, cada um rodando na suíte | - |

- Nenhum check cita código de saída; `Surface` e `Landing` são `None`
- O C6 é amostral: a maior carroça é sorteada; um valor que nunca aparece nas 1000 seeds fica sem caso, e o teste falha se nenhuma seed deixar o `[6|6]` de fora

## Test policy

| Code | Required proofs | Coverage expectation |
| --- | --- | --- |
| Decide, sem fronteira | um no próprio nível | um caso afirmado por linha da tabela de decisão |
| Decide, alcançado pelo controlador | um no próprio nível **e** um pelo controlador | no controlador: o caminho que o jogador vê; no próprio nível: a regra inteira |
| Instrumentação (desenho que só lê a `View`) | nenhum próprio | coberto pelos testes de `View` |

Evidence:

- `src/core/rules.c` (`start_hand`, `game_fits` na primeira mão): divisão 24 + 4, escolha da maior carroça, recusa das outras peças -> decide, sem fronteira (C1, C6)
- `src/core/cpu.c` (`list_moves` na primeira mão): 1 filtro -> decide, sem fronteira (C6)
- `src/core/controller.c` (distribuição, virada, `ctl_view`): destino por k, contagem por tempo, face por fase -> decide, alcançado pelo controlador (C4, C5, C7 a C10)
- `src/core/layout.c` (`layout_sleeping`): posição fixa, sem condição -> instrumentação provada por C2
- `src/ui/draw.c`: só lê a `View` -> instrumentação
- análogo no repo: `tests/test_parceria.c` prova a distribuição e a virada pelo controlador

Cost: 10 provas novas em 1 arquivo de teste, mais as 8 reescritas.

## Swept

- validation: C7 - abertura com peça errada é recusada
- failure modes: n/a - nenhuma operação que possa falhar no meio
- idempotency: n/a - nenhuma operação repetível nova
- authorization: n/a - jogo local de um só usuário
- concurrency: n/a - laço único, sem threads
- data lifecycle: C1 - as peças de fora valem só para a mão, e a mão seguinte sorteia outras
- dependency failure: n/a - nenhuma dependência nova
- state transitions: C8, C10 - de costas durante a mão, de face no fim
- observability: n/a - nenhum requisito de log

## Handoff

- Arquivos tocados: `domino.h` (3 KB), `rules.c` (7 KB), `cpu.c` (4 KB), `controller.c`/`.h` (14 KB),
  `layout.c`/`.h` (5 KB), `draw.c` (7 KB), `tests/test_core.c` e `tests/test_parceria.c` (só os 8 testes
  substituídos), `tests/test_fora.c` (novo, ~12 KB), `CMakeLists.txt`, artefatos (~20 KB). Total
  ~95 KB / 4 = ~24k, abaixo do orçamento de 150k - one builder
