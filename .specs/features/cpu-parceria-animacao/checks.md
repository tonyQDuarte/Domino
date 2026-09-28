# CPU em parceria e animação checks

Profile: standard
Plan: `.specs/features/cpu-parceria-animacao/plan.md`

23 checks in 6 slices · 2 one-way doors · 0 open, of which 0 block

Build e PATH como em `AGENTS.md`. Os testes novos ficam em `tests/test_parceria.c` (executável
`test_parceria`), para que os arquivos de teste de `partida-duplas` fiquem intocados (C21).

## Checks

### S1 - A CPU joga pela dupla · 4 files · ~20 KB · ~5k

- [x] **C1** - Em 200 seeds, em cada vez de CPU ao longo da mão, trocar entre si as peças das outras três mãos (mantendo as quantidades) não muda a peça nem a ponta escolhidas por `cpu_choose` (AC 1)
Proof: `ctest --test-dir build -R "^cpu_ignores_hidden_hands$" --output-on-failure`

- [x] **C2** - `cpu_decide` decide a partir de um `CpuView` montado à mão, sem nenhum `Game`, e `cpu_view_of` copia só a mão do próprio assento, as quantidades, as pontas e os passes (door 1, AC 1)
Proof: `ctest --test-dir build -R "^cpu_decide_from_view_only$" --output-on-failure`

- [x] **C3** - Um passe com pontas 2/5 marca `lacks[assento][2]` e `lacks[assento][5]`, nenhum outro valor, e a marca continua depois de jogadas seguintes (AC 2)
Proof: `ctest --test-dir build -R "^pass_records_lacks$" --output-on-failure`

- [x] **C4** - `game_next_hand` e `game_restart_match` começam a mão com todos os 28 `lacks` falsos (AC 3)
Proof: `ctest --test-dir build -R "^new_hand_clears_lacks$" --output-on-failure`

- [x] **C5** - Modo parceiro (Norte com 4, Sul com 2, Sul passou no 3 e no 6), pontas 3/1, mão `[1|6]`,`[3|0]`: a CPU joga `[3|0]` na esquerda, e não `[1|6]`, que deixaria 3/6; se `[1|6]` for a única jogada, ela joga `[1|6]` (AC 4)
Proof: `ctest --test-dir build -R "^cpu_partner_mode_avoids_blocking$" --output-on-failure`

- [x] **C6** - O mesmo cenário do C5 com o parceiro tendo 5 peças (mais) ou 4 (empate) fica em modo próprio, e a CPU joga `[1|6]` (AC 4, AC 6)
Proof: `ctest --test-dir build -R "^cpu_self_mode_when_partner_not_fewer$" --output-on-failure`

- [x] **C7** - O adversário seguinte passou no 4 e no 2, pontas 4/5, mão `[5|2]`,`[5|6]`: em modo próprio a CPU joga `[5|2]` (deixa 4/2); em modo parceiro, com o parceiro também tendo passado no 4 e no 2, joga `[5|6]` (AC 4 vem antes do AC 5) (AC 5)
Proof: `ctest --test-dir build -R "^cpu_prefers_opponent_pass$" --output-on-failure`

- [x] **C8** - Pontas 1/2, mão `[1|3]`,`[2|6]`,`[3|4]`, sem passes: em modo próprio a CPU joga `[1|3]` (sobram 2 peças que encaixam); em modo parceiro joga `[2|6]` (AC 6 só vale no modo próprio) (AC 6)
Proof: `ctest --test-dir build -R "^cpu_self_mode_keeps_own_moves$" --output-on-failure`

- [x] **C9** - O parceiro usado no modo parceiro é Norte→Sul, Leste→Oeste e Oeste→Leste; passes de um assento que não é o parceiro não ativam o AC 4 (AC 7)
Proof: `ctest --test-dir build -R "^cpu_partner_pairs$" --output-on-failure`

- [x] **C10** - Com as regras empatadas, `cpu_decide` escolhe a maior soma, depois a carroça, depois a primeira da mão, e a ponta esquerda para peça que encaixa nas duas (AC 8)
Proof: `ctest --test-dir build -R "^cpu_tiebreak_like_before$" --output-on-failure`

### S2 - A peça desliza até a mesa · 4 files · ~25 KB · ~6k

- [x] **C11** - Com animações ligadas, depois de uma jogada há exatamente uma peça em voo: aos 0 ms o centro dela é o centro da peça de origem na mão (Sul e CPU), aos 175 ms está estritamente entre origem e destino, aos 349 ms a menos de 1 px do centro do lugar na linha, e aos 350 ms não há peça em voo (AC 9)
Proof: `ctest --test-dir build -R "^slide_moves_hand_to_board$" --output-on-failure`

- [x] **C12** - Enquanto a peça desliza, `board_hidden` é o índice dela na linha; aos 350 ms volta a -1 (AC 10)
Proof: `ctest --test-dir build -R "^slide_hides_board_slot$" --output-on-failure`

- [x] **C13** - Depois de o Sul jogar, o Leste não joga aos 350 + 799 ms e jogou aos 350 + 800 ms; durante o deslize de uma peça do Oeste, com a vez já do Sul, um clique numa peça do Sul não muda nada, e depois do deslize o mesmo clique joga (AC 11)
Proof: `ctest --test-dir build -R "^slide_blocks_timer_and_clicks$" --output-on-failure`

### S3 - A distribuição é animada · 2 files · ~15 KB · ~4k

- [x] **C14** - Ao começar a mão com animações, a peça k (0..27) sai do centro da área da mesa aos 50·k ms para o assento k mod 4 (Sul, Leste, Norte, Oeste), chega ao centro do lugar dela na mão aos 50·k + 300 ms, e não há peça em voo aos 1650 ms (AC 12)
Proof: `ctest --test-dir build -R "^deal_animation_timing$" --output-on-failure`

- [x] **C15** - De 0 a 1700 ms, a cada 25 ms, `hand_shown[s]` é igual ao número de peças k com k mod 4 = s que já chegaram (50·k + 300 <= t) (AC 13)
Proof: `ctest --test-dir build -R "^deal_shows_arrived_only$" --output-on-failure`

- [x] **C16** - Durante a distribuição, um clique numa peça do Sul não joga, o Sul sem jogada não passa, e a CPU que abre não joga antes de 1650 + 800 ms; aos 1650 + 800 ms ela joga (AC 14)
Proof: `ctest --test-dir build -R "^deal_blocks_play$" --output-on-failure`

### S4 - As mãos viram no fim · 2 files · ~15 KB · ~4k

- [x] **C17** - Numa batida do Sul, a virada começa aos 350 ms (fim do deslize): `flip_scale` vale 1 aos 350 ms, menos de 0,05 aos 550 ms, e 1 aos 750 ms; as CPUs aparecem de costas até os 550 ms e de face a partir daí. Num tranque, a virada começa na hora, sem deslize (AC 15)
Proof: `ctest --test-dir build -R "^reveal_flips_cpu_hands$" --output-on-failure`

- [x] **C18** - `overlay_visible` é falso durante o deslize e a virada, tanto no fim de mão quanto no fim de partida, e passa a verdadeiro aos 750 ms (AC 16)
Proof: `ctest --test-dir build -R "^reveal_hides_overlay$" --output-on-failure`

- [x] **C19** - Um clique, ou um Enter, durante a virada deixa `overlay_visible` verdadeiro, `flip_scale` 1 e a fase ainda `PHASE_HAND_OVER`; o clique ou Enter seguinte começa a próxima mão (AC 17)
Proof: `ctest --test-dir build -R "^reveal_skip_on_input$" --output-on-failure`

### S5 - Os destaques pulsam · 2 files · ~10 KB · ~3k

- [x] **C20** - Com pontas destacadas e animações ligadas, `pulse` amostrado a cada 1 ms em 800 ms tem mínimo em [0,35, 0,36] e máximo em [0,84, 0,85], e `pulse(t) == pulse(t + 800)` (AC 18)
Proof: `ctest --test-dir build -R "^pulse_period_800$" --output-on-failure`

### S6 - O que já existe continua igual · 3 files · ~10 KB · ~3k

- [x] **C21** - Os 40 testes de `partida-duplas` (label `partida-duplas`) passam, e os arquivos `tests/test_core.c`, `tests/test_process.c`, `tests/test_window.c`, `tests/harness.h`, `tests/fixtures.h` e `tests/core_has_no_raylib.cmake` não mudaram desde `39873e8` (AC 19)
Proof: `ctest --test-dir build -L partida-duplas --output-on-failure`
Proof: `git diff --exit-code 39873e8 -- tests/test_core.c tests/test_process.c tests/test_window.c tests/harness.h tests/fixtures.h tests/core_has_no_raylib.cmake`

- [x] **C22** - No primeiro quadro de `app_run`, o controlador está com as animações ligadas e a distribuição em andamento (AC 20, door 2)
Proof: `ctest --test-dir build -R "^app_animations_on$" --output-on-failure`

- [x] **C23** - Depois de `ctl_init`, as animações estão desligadas: nenhuma peça em voo, `hand_shown` igual à quantidade na mão, e numa batida o quadro de resultado aparece na hora (door 2, AC 19)
Proof: `ctest --test-dir build -R "^animations_off_by_default$" --output-on-failure`

## Coverage

| Set (size) | Member -> proof | Unproven |
| --- | --- | --- |
| regras da CPU em ordem (7) | não trancar o parceiro C5 · adversário passa C7 · mais jogo na própria mão C8 · maior soma C10 · carroça C10 · primeira da mão C10 · ponta esquerda C10 | - |
| escolha do modo (3) | parceiro com menos C5 · empate C6 · parceiro com mais C6 | - |
| pares de parceiros (3) | Norte→Sul C9 · Leste→Oeste C9 · Oeste→Leste C9 | - |
| animações (4) | distribuição C14 · deslize C11 · virada C17 · pulso C20 | - |
| transições da linha do tempo (8) | nada→distribuição C14 · distribuição→nada C14 · nada→deslize C11 · deslize→nada C11 · deslize→virada C17 · nada→virada (tranque) C17 · virada→nada C17 · virada→nada por clique/Enter C19 | - |
| entradas bloqueadas por animação (7) | clique na distribuição C16 · tempo da CPU na distribuição C16 · passe automático na distribuição C16 · clique no deslize C13 · tempo da CPU no deslize C13 · clique na virada C19 · Enter na virada C19 | - |
| quadro de resultado escondido (2) | fim de mão C18 · fim de partida C18 | - |
| origem do deslize (2) | mão do Sul C11 · fileira de CPU C11 | - |
| `Landing` doors (2) | door 1 C2 · door 2 C22, C23 | - |
| startup config: animações (2 assemblies) | `domino.exe` via `app_run` C22 · controlador de teste via `ctl_init` C23 | - |

- Nenhum check cita código de saída nem a linha de comando; `Surface` é `None`
- C1 é amostral (200 seeds); as regras uma a uma estão nos cenários fixos de C5 a C10

## Test policy

O repositório não diz em que nível provar cada código nem quanto do espaço de entrada cobrir.

| Code | Required proofs | Coverage expectation |
| --- | --- | --- |
| Decide, alcançado através de uma fronteira (`app_run`) | um na fronteira **e** um no próprio nível | na fronteira: as animações ligadas; no próprio nível: um caso por linha da tabela de decisão |
| Decide, sem fronteira | um no próprio nível | um caso afirmado por linha da tabela de decisão |
| Instrumentação (desenho que só lê a `View`) | nenhum próprio | coberto pelos testes de `View` de quem a produz |

Evidence:

- `src/core/cpu.c` (`cpu_decide`): 3 filtros em sequência mais 4 desempates e a escolha de modo, cerca de 12 pontos de decisão -> decide, sem fronteira
- `src/core/rules.c` (`game_pass`, início de mão): registra e limpa `lacks`, 2 pontos -> decide, sem fronteira
- `src/core/controller.c` (linha do tempo): 4 estados, 8 transições, 7 entradas bloqueadas -> decide, alcançado através de `app_run`
- `src/ui/draw.c`: só lê a `View` e desenha, nenhuma condição que mude o resultado do jogo -> instrumentação
- análogo no repo: `tests/test_core.c` prova controlador e regras no próprio nível, um caso por ramo (por exemplo `click_*`, `score_*`)

Cost: 22 provas no próprio nível em 3 arquivos, mais 1 na fronteira. Sem estas linhas, a tabela da CPU
seria provada só pelo C1, que percorre jogos inteiros e não falha quando uma regra está fora de ordem.

## Swept

- validation: n/a - a feature não recebe entrada nova do usuário
- failure modes: n/a - nenhuma operação que possa falhar no meio (sem disco, rede ou janela nova)
- idempotency: C19 - o clique que pula a virada não começa a próxima mão
- authorization: n/a - jogo local de um só usuário
- concurrency: n/a - laço único; animação e tempo da CPU avançam pelo mesmo `dt` em sequência (C13, C16)
- data lifecycle: C4 - os passes valem só dentro da mão
- dependency failure: n/a - nenhuma dependência nova
- state transitions: C11, C14, C17, C19
- observability: n/a - nenhum requisito de log

## Handoff

- Arquivos tocados: `cpu.c` (~1 KB -> ~5 KB), `rules.c` (6 KB), `domino.h` (3 KB), `controller.c` (6 KB -> ~12 KB),
  `controller.h` (1,5 KB), `draw.c` (6 KB), `app.c`/`app.h` (2,5 KB), `CMakeLists.txt` (3 KB), testes novos
  (~25 KB), artefatos (~25 KB). Total ~90 KB / 4 = ~23k, abaixo do orçamento de 150k - one builder
