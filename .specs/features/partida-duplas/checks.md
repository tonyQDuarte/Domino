# Partida em duplas checks

Profile: light
Plan: `.specs/features/partida-duplas/plan.md`

38 checks in 5 slices · 4 one-way doors · 0 open, of which 0 block

Todas as provas pressupõem o build feito (`cmake -S . -B build -G Ninja && cmake --build build`, com
`C:\msys64\ucrt64\bin` no PATH). Cada prova é um teste CTest de nome único, selecionado por
regex ancorada.

## Checks

### S1 - A mesa abre com as peças distribuídas · ~10 files · ~35 KB · ~9k

- [x] **C1** - `domino.exe --seed 1` abre uma janela de título `Dominó` com área cliente de 1280x720 (AC 1)
Proof: `ctest --test-dir build -R "^cli_window_title_and_size$" --output-on-failure`

- [x] **C2** - No primeiro quadro desenhado, o pixel (640, 360) da mesa vazia tem cor RGB(20, 100, 50) (AC 1)
Proof: `ctest --test-dir build -R "^window_table_color$" --output-on-failure`

- [x] **C3** - Para as seeds 0 a 999, a distribuição dá 7 peças a cada um dos 4 assentos, e as 28 peças são exatamente o conjunto `[a|b]` com 0 <= a <= b <= 6, sem repetição (AC 2)
Proof: `ctest --test-dir build -R "^deal_28_unique_7_each$" --output-on-failure`

- [x] **C4** - A mão do Sul aparece em ordem crescente de soma e todos os retângulos dela ficam na faixa inferior da janela (y >= 560) (AC 3)
Proof: `ctest --test-dir build -R "^south_hand_sorted_by_sum$" --output-on-failure`
Proof: `ctest --test-dir build -R "^south_hand_in_bottom_band$" --output-on-failure`

- [x] **C5** - Durante a mão, cada CPU (Norte, Leste, Oeste) tem uma fileira com exatamente tantas peças viradas para baixo quanto a mão dela tem, antes e depois de jogar (AC 4)
Proof: `ctest --test-dir build -R "^cpu_rows_match_hand_count$" --output-on-failure`

- [x] **C6** - A mesma seed produz as mesmas 4 mãos, e `--seed 42` chega ao jogo como a seed 42 (AC 5)
Proof: `ctest --test-dir build -R "^deal_same_seed_same_hands$" --output-on-failure`
Proof: `ctest --test-dir build -R "^cli_parse_seed$" --output-on-failure`

- [x] **C7** - `--seed x`, `--seed -1`, `--seed` sem valor e `--foo` fazem `domino.exe` escrever `uso: domino [--seed <n>]` no stderr e sair com código 2, sem abrir janela (AC 6)
Proof: `ctest --test-dir build -R "^cli_bad_args_exit_2$" --output-on-failure`

- [x] **C8** - Quando a janela não fica pronta, o programa escreve `erro: não foi possível abrir a janela` no stderr e retorna 1 (AC 7)
Proof: `ctest --test-dir build -R "^window_fail_exits_1$" --output-on-failure`

- [x] **C9** - Um `WM_CLOSE` enviado à janela de `domino.exe` encerra o processo com código 0 (AC 8)
Proof: `ctest --test-dir build -R "^cli_close_exits_0$" --output-on-failure`

### S2 - O humano joga as próprias peças · ~6 files · ~30 KB · ~8k

- [x] **C10** - Na primeira mão da partida, a vez começa com quem tem `[6|6]` e qualquer outra peça é recusada como primeira jogada (AC 9)
Proof: `ctest --test-dir build -R "^first_hand_double_six_starts$" --output-on-failure`
Proof: `ctest --test-dir build -R "^first_hand_rejects_non_double_six$" --output-on-failure`

- [x] **C11** - Depois de cada jogada ou passe, a vez vai Sul → Leste → Norte → Oeste → Sul (AC 10)
Proof: `ctest --test-dir build -R "^turn_order_counterclockwise$" --output-on-failure`

- [x] **C12** - Na vez do Sul, um clique numa peça que encaixa só numa ponta a coloca nessa ponta, com o valor igual encostado, e a ponta passa a ser o outro valor (AC 11)
Proof: `ctest --test-dir build -R "^click_single_end_places$" --output-on-failure`

- [x] **C13** - Na vez do Sul, com pontas 2 e 5, um clique em `[2|5]` não altera a mesa e marca as duas pontas como destacadas; o clique seguinte na ponta direita coloca a peça à direita (AC 12)
Proof: `ctest --test-dir build -R "^click_both_ends_waits_choice$" --output-on-failure`

- [x] **C14** - Na vez do Sul, com as duas pontas iguais a 3, um clique em `[3|1]` coloca a peça na ponta direita sem destacar ponta nenhuma (AC 13)
Proof: `ctest --test-dir build -R "^click_equal_ends_goes_right$" --output-on-failure`

- [x] **C15** - Um clique numa peça que não encaixa deixa mesa e mão iguais e mostra `Jogada inválida`, visível aos 1499 ms e ausente aos 1500 ms (AC 14)
Proof: `ctest --test-dir build -R "^click_invalid_shows_message$" --output-on-failure`

- [x] **C16** - Fora da vez do Sul, um clique numa peça do Sul não muda mesa, mãos nem a vez (AC 15)
Proof: `ctest --test-dir build -R "^click_ignored_off_turn$" --output-on-failure`

- [x] **C17** - Quando chega a vez do Sul sem nenhuma peça que encaixe, a vez passa para Leste e aparece `Você passou` por 1500 ms (AC 16)
Proof: `ctest --test-dir build -R "^south_auto_pass$" --output-on-failure`

### S3 - As três CPUs jogam sozinhas · ~4 files · ~20 KB · ~5k

- [x] **C18** - Uma CPU com jogada válida não joga aos 799 ms da vez e jogou aos 800 ms (AC 17)
Proof: `ctest --test-dir build -R "^cpu_moves_at_800ms$" --output-on-failure`

- [x] **C19** - A CPU escolhe a peça válida de maior soma; no empate, a carroça; persistindo, a primeira na ordem da mão; e coloca na ponta esquerda a peça que encaixa nas duas (AC 18)
Proof: `ctest --test-dir build -R "^cpu_picks_heaviest$" --output-on-failure`

- [x] **C20** - CPU sem jogada passa a vez e mostra `Leste passou`, `Parceiro passou` ou `Oeste passou` conforme o assento, por 1500 ms (AC 19)
Proof: `ctest --test-dir build -R "^cpu_pass_message$" --output-on-failure`

- [x] **C21** - Enquanto a mão está em andamento, nenhuma peça de Norte, Leste ou Oeste é marcada para desenho de face (AC 20)
Proof: `ctest --test-dir build -R "^cpu_faces_hidden_in_play$" --output-on-failure`

### S4 - A mão termina, pontua e a partida vai até 6 · ~4 files · ~25 KB · ~7k

- [x] **C22** - Bater com uma peça que não é carroça e encaixa numa só ponta resulta em `SIMPLES` e +1 para a dupla de quem bateu (AC 21)
Proof: `ctest --test-dir build -R "^score_simples$" --output-on-failure`

- [x] **C23** - Bater com uma carroça que encaixa numa só ponta resulta em `CARROCA` e +2 (AC 22)
Proof: `ctest --test-dir build -R "^score_carroca$" --output-on-failure`

- [x] **C24** - Bater com uma peça que não é carroça e encaixa nas duas pontas resulta em `LA_E_LO` e +3, tanto com pontas diferentes (`[2|5]` em 2/5) quanto iguais (`[3|1]` em 3/3) (AC 23)
Proof: `ctest --test-dir build -R "^score_la_e_lo$" --output-on-failure`

- [x] **C25** - Bater com uma carroça que encaixa nas duas pontas (`[4|4]` em 4/4) resulta em `CRUZADA` e +4 (AC 24)
Proof: `ctest --test-dir build -R "^score_cruzada$" --output-on-failure`

- [x] **C26** - Quatro passes seguidos encerram a mão como `TRANCADO` e dão +1 à dupla com menor soma nas mãos (AC 25)
Proof: `ctest --test-dir build -R "^score_trancado_lower_sum$" --output-on-failure`

- [x] **C27** - Trancado com somas iguais entre as duplas termina a mão com +0 para as duas (AC 26)
Proof: `ctest --test-dir build -R "^score_trancado_tie$" --output-on-failure`

- [x] **C28** - No fim da mão as peças dos 4 assentos ficam marcadas para desenho de face, o overlay traz o tipo, os pontos e o placar, e só um clique ou Enter começa a próxima mão (AC 27)
Proof: `ctest --test-dir build -R "^hand_end_overlay$" --output-on-failure`

- [x] **C29** - Na mão seguinte a uma batida, a vez começa com quem bateu, e uma peça diferente de `[6|6]` é aceita como abertura (AC 28)
Proof: `ctest --test-dir build -R "^next_hand_winner_starts$" --output-on-failure`
> Substituído por `abertura-carroca` (AC 5 e C9 de lá), a pedido do usuário em 2026-09-28: a mão seguinte abre com carroça, e quem bateu só abre se tiver uma. O teste foi reescrito para a regra nova.

- [x] **C30** - Na mão seguinte a um tranque, a vez começa com quem abriu a mão trancada, e uma peça diferente de `[6|6]` é aceita como abertura (AC 29)
Proof: `ctest --test-dir build -R "^next_hand_after_tranque$" --output-on-failure`
> Substituído por `abertura-carroca` (AC 6, AC 7 e C9 de lá), a pedido do usuário em 2026-09-28: depois de um tranque, a abertura segue a ordem da carroça a partir da dupla que ganhou o ponto. O teste foi reescrito para a regra nova.

- [x] **C31** - O texto do placar é `Nós <x> × <y> Eles` (`Nós 0 × 0 Eles` no início, `Nós 3 × 1 Eles` depois de +3 e +1) e ancora no canto superior esquerdo (AC 30)
Proof: `ctest --test-dir build -R "^score_text_format$" --output-on-failure`

- [x] **C32** - Chegar a 6 ou mais encerra a partida com `Vocês venceram!` para Sul+Norte ou `Eles venceram!` para Leste+Oeste, e o botão `Nova partida` aparece (AC 31)
Proof: `ctest --test-dir build -R "^match_end_message$" --output-on-failure`

- [x] **C33** - Clicar em `Nova partida` zera o placar para 0 × 0 e a mão nova volta à regra do `[6|6]` (AC 32)
Proof: `ctest --test-dir build -R "^new_match_resets$" --output-on-failure`

### S5 - A linha de peças cabe na mesa · ~3 files · ~12 KB · ~3k

- [x] **C34** - A primeira peça da linha tem o centro no centro da área da mesa (AC 33)
Proof: `ctest --test-dir build -R "^layout_first_tile_centered$" --output-on-failure`

- [x] **C35** - Para linhas de 1 a 28 peças, crescendo só à esquerda, só à direita e alternando, todos os retângulos ficam dentro da área da mesa e nenhum se sobrepõe a outro (AC 34)
Proof: `ctest --test-dir build -R "^layout_within_table$" --output-on-failure`

- [x] **C36** - Toda carroça na linha tem orientação perpendicular ao sentido do trecho em que está (AC 35)
Proof: `ctest --test-dir build -R "^layout_doubles_perpendicular$" --output-on-failure`

### Doors

- [x] **C37** - Nenhum arquivo de `domino_core` inclui `raylib.h` (door 3)
Proof: `ctest --test-dir build -R "^core_has_no_raylib$" --output-on-failure`

- [x] **C38** - Com o MSYS2 UCRT64 no PATH, `cmake -S . -B build -G Ninja` encontra a raylib por `find_package` e `cmake --build build` termina com código 0 (doors 1, 2, 4)
Proof: `cmake -S . -B build -G Ninja && cmake --build build`

## Coverage

| Set (size) | Member -> proof | Unproven |
| --- | --- | --- |
| `domino.exe` statuses (3) | `0` C9 · `1` C8 · `2` C7 | - |
| argumentos inválidos (4) | `--seed x` C7 · `--seed -1` C7 · `--seed` sem valor C7 · `--foo` C7 | - |
| `HandResult` (5) | `SIMPLES` C22 · `CARROCA` C23 · `LA_E_LO` C24 · `CRUZADA` C25 · `TRANCADO` C26, C27 | - |
| passagem da vez (4) | Sul→Leste C11 · Leste→Norte C11 · Norte→Oeste C11 · Oeste→Sul C11 | - |
| quem abre a mão (3) | primeira mão `[6|6]` C10 · depois de batida C29 · depois de tranque C30 | - |
| desempate da CPU (4) | maior soma C19 · carroça C19 · primeira na mão C19 · ponta esquerda C19 | - |
| clique do Sul (5) | uma ponta C12 · duas pontas diferentes C13 · duas pontas iguais C14 · não encaixa C15 · fora da vez C16 | - |
| mensagens de passe (4) | `Você passou` C17 · `Leste passou` C20 · `Parceiro passou` C20 · `Oeste passou` C20 | - |
| vencedor da partida (2) | `Vocês venceram!` C32 · `Eles venceram!` C32 | - |
| tamanho da linha (28) | C35, table-driven over 1..28 in 3 growth patterns | - |
| `Landing` doors (4) | door 1 C38 · door 2 C38 · door 3 C37 · door 4 C38 | - |
| startup config: janela (2 assemblies) | `domino.exe` C1 · smoke em processo C2 - os dois passam pela mesma `app_run` | - |

- Checks que nomeiam código de saída ou texto do stderr: C7, C8, C9. C7 e C9 executam o
  `domino.exe` de verdade. C8 usa um ponto de injeção em `app_run`, porque não dá para forçar o
  driver gráfico a falhar
- Os checks de tela (C4, C5, C15, C21, C28, C31, C32) verificam o estado de visualização que o
  `domino_core` entrega à `domino_ui`. O desenho em si é só instrumentação e não tem prova própria

## Swept

- validation: C7 (argumentos), C15 (jogada que não encaixa)
- failure modes: C8
- idempotency: C16 - cliques repetidos depois da jogada caem fora da vez e são ignorados
- authorization: n/a - jogo local de um só usuário, sem conta nem permissão
- concurrency: n/a - laço de quadro único, sem threads; os temporizadores avançam pelo `dt` em sequência (C18)
- data lifecycle: n/a - nada é gravado
- dependency failure: C8 (raylib não consegue abrir a janela)
- state transitions: C10, C11, C28, C29, C30, C32, C33
- observability: n/a - nenhum requisito de log; erros vão para o stderr (C7, C8)

## Handoff

- O projeto começa vazio. Estimativa a partir do que será escrito: S1 ~35 KB, S2 ~30 KB, S3 ~20 KB,
  S4 ~25 KB, S5 ~12 KB, e os artefatos em `.specs/` ~30 KB. Total ~152 KB / 4 = ~38k, abaixo do
  orçamento de 150k - one builder
