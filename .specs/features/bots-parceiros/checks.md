# Bots parceiros checks

Profile: standard
Plan: `.specs/features/bots-parceiros/plan.md`

17 checks in 4 slices · 1 one-way door · 0 open, of which 0 block

Build e PATH como em `AGENTS.md`. A tabela de bots fica em `src/core/bots.c` (sem raylib, AD-003).
Os testes de regra, menu, controlador e layout ficam em `tests/test_bots.c` (executável `test_bots`,
só `domino_core`); os da `app_run` com a tela inicial ficam em `tests/test_bots_app.c` (executável
`test_bots_app`, com `domino_app`), usando os ganchos `menu_input` e `on_game_start` que já existem.
Nenhum arquivo de teste antigo é editado (AC 17).

Cenários usados nas provas de S1 (CPU no Norte, parceiro Sul, seguinte Oeste):

- **Cenário P**: mão `[1|6] [3|0] [4|4] [5|5]`, pontas 3 e 1, Sul passou no 3 e no 6. Modo parceiro
  joga `[3|0]` (não tranca o parceiro); modo próprio e o desempate puro jogam `[1|6]`.
- **Cenário L**: mão `[5|2] [5|6] [0|0]`, 3 peças, parceiro com 2, pontas 4 e 5, Sul e Oeste passaram
  no 4 e no 2. P antes de A joga `[5|6]`; A antes de P joga `[5|2]` (faz o Oeste passar, mesmo
  trancando o parceiro); o desempate puro joga `[5|6]`.
- **Cenário J**: CPU no Leste, mão `[1|3] [2|6] [3|4]`, pontas 1 e 2, parceiro com 7, ninguém passou.
  A regra J joga `[1|3]` na esquerda; o desempate puro joga `[2|6]` na direita.

## Checks

### S1 - Seis bots com estilos diferentes · 5 files · ~16 KB · ~4k

- [x] **C1** - `BOT_COUNT` é 6 e `BOTS[0..5]` tem, nesta ordem, os nomes `Seu Paulo`, `Neca`, `Audi`, `Seu Leo`, `Guede`, `Manezinho`, os textos `Fidelidade total`, `Colaboração`, `Equilíbrio`, `Bloqueio`, `Jogo próprio`, `Descarrego` e os estilos `STYLE_SEU_PAULO`, `STYLE_NECA`, `STYLE_AUDI`, `STYLE_SEU_LEO`, `STYLE_GUEDE`, `STYLE_MANEZINHO`; os 6 estilos são diferentes entre si (AC 1)
Proof: `ctest --test-dir build -R "^bots_table$" --output-on-failure`

- [x] **C2** - Com `STYLE_SEU_PAULO`, no cenário P, a CPU joga `[3|0]` com o parceiro tendo 2, 4 e 5 peças (ela tem 4) (AC 2)
Proof: `ctest --test-dir build -R "^style_seu_paulo_always_partner$" --output-on-failure`

- [x] **C3** - Com `STYLE_NECA`, no cenário P, a CPU joga `[3|0]` com o parceiro tendo 2 e 4 peças e `[1|6]` com 5 (AC 3)
Proof: `ctest --test-dir build -R "^style_neca_partner_when_not_more$" --output-on-failure`

- [x] **C4** - `STYLE_AUDI` vale 0; com ele, no cenário P, a CPU joga `[3|0]` com o parceiro tendo 2 peças e `[1|6]` com 4 e 5; no cenário L joga `[5|6]`; no cenário J joga `[1|3]` na esquerda (AC 4, door 1)
Proof: `ctest --test-dir build -R "^style_audi_is_current_rule$" --output-on-failure`

- [x] **C5** - Com `STYLE_SEU_LEO`, no cenário L a CPU joga `[5|2]`; no cenário P joga `[3|0]` com o parceiro tendo 2 peças e `[1|6]` com 4 (AC 5)
Proof: `ctest --test-dir build -R "^style_seu_leo_blocks_first$" --output-on-failure`

- [x] **C6** - Com `STYLE_GUEDE`, no cenário P a CPU joga `[1|6]` com o parceiro tendo 2 peças; no cenário L joga `[5|2]`; no cenário J joga `[1|3]` na esquerda (AC 6)
Proof: `ctest --test-dir build -R "^style_guede_never_partner$" --output-on-failure`

- [x] **C7** - Com `STYLE_MANEZINHO`, no cenário P a CPU joga `[1|6]` com o parceiro tendo 2 peças; no cenário L joga `[5|6]`; no cenário J joga `[2|6]` na direita (AC 7)
Proof: `ctest --test-dir build -R "^style_manezinho_tiebreak_only$" --output-on-failure`

- [x] **C8** - Para cada um dos 6 estilos, em 100 partidas (seeds 0..99) com `Game.styles` de Leste, Norte e Oeste naquele estilo: `cpu_view_of(g, s).style == g->styles[s]`, e em cada vez de CPU, embaralhar as peças das mãos dos outros 3 assentos e as de fora (mantendo as quantidades) não muda o índice nem a ponta escolhidos por `cpu_choose` (AC 8, door 1)
Proof: `ctest --test-dir build -R "^styles_decide_from_view_only$" --output-on-failure`

### S2 - O jogador escolhe o parceiro na tela inicial · 7 files · ~25 KB · ~6k

- [x] **C9** - `MENU_PARTNER_LABEL` é `Seu parceiro`; depois de `menu_init`, `partner` vale 2 (Audi), com e sem nome inicial (AC 9)
Proof: `ctest --test-dir build -R "^menu_partner_default_audi$" --output-on-failure`

- [x] **C10** - Clicar no centro de `menu_card_rect(i)` deixa `partner == i` e devolve `MENU_NONE`, para i = 0..5 e na ordem 5, 0, 3; clicar fora dos cartões (no título e em (20, 700)) não muda `partner` (AC 10)
Proof: `ctest --test-dir build -R "^menu_card_click_selects$" --output-on-failure`
> Nota do build: a seleção ficou em `menu_pick_partner(Menu *, x, y)`, chamada pela `app_run` antes de `menu_click`; `menu_click` continua com `const Menu *` porque `tests/test_menu.c` (que não muda, AC 17) passa um ponteiro `const`. O teste afirma as duas coisas: o cartão seleciona e `menu_click` devolve `MENU_NONE`.

- [x] **C11** - `menu_play_rect()` é {540, 360, 200, 55} e `menu_quit_rect()` é {540, 440, 200, 55}; título, rótulo do nome, campo, rótulo do parceiro, os 6 cartões, `Jogar` e `Sair` ficam dentro da janela, nessa ordem de cima para baixo (os 6 cartões numa faixa só, cada um abaixo do rótulo do parceiro e acima de `Jogar`) e nenhum par desses 12 retângulos se sobrepõe (AC 11)
Proof: `ctest --test-dir build -R "^menu_partner_layout$" --output-on-failure`

- [x] **C12** - `ctl_set_bots(c, p, seed)` para p = 0..5 e seeds 0..99: o Norte recebe o bot p, Leste e Oeste recebem bots diferentes entre si e de p, e `game.styles` de cada assento é o estilo do bot dele; a mesma seed dá os mesmos adversários; as mãos distribuídas são iguais às de `ctl_init` sem bots com a mesma seed; e, para cada p, cada um dos outros 5 bots aparece como adversário em alguma seed. Na `app_run`, com o parceiro trocado para Seu Paulo antes do `MENU_PLAY`, a partida começa com Seu Paulo no Norte e dois adversários diferentes dele e entre si, e duas execuções com a mesma seed dão os mesmos adversários (AC 12)
Proof: `ctest --test-dir build -R "^bots_seating_by_seed$" --output-on-failure`
Proof: `ctest --test-dir build -R "^app_menu_partner_starts_game$" --output-on-failure`

- [x] **C13** - Com bots e a partida em `PHASE_MATCH_OVER`, o clique no centro de `Nova partida` mantém o bot do Norte, deixa Leste e Oeste diferentes entre si e do Norte, com `game.styles` de acordo; em 20 cliques seguidos aparecem pelo menos 2 pares de adversários diferentes (AC 13)
Proof: `ctest --test-dir build -R "^new_match_keeps_partner_redraws$" --output-on-failure`

### S3 - Os nomes aparecem na mesa · 5 files · ~25 KB · ~6k

- [x] **C14** - Com bots, a `View` traz em `seat_name` o nome do bot de Norte, Leste e Oeste e vazio no Sul; `layout_bot_name(s)` para Norte, Leste e Oeste fica dentro da janela, não se sobrepõe à fileira de nenhum assento com 1 a 6 peças, à área da mesa, ao retângulo do placar {0, 0, 300, 50}, às peças de fora, à mensagem nem aos outros dois rótulos; o do Norte fica acima da fileira do Norte (y + h <= 40) e centrado em x = 640, e os de Leste e Oeste ficam acima da coluna de 6 peças e centrados nela (AC 14)
Proof: `ctest --test-dir build -R "^bot_names_on_table$" --output-on-failure`

- [x] **C15** - Com bots e Seu Leo no Leste sem jogada, depois de 800 ms na vez dele a mensagem visível é `Seu Leo passou`; continua visível 1499 ms depois e some aos 1500 ms (AC 15)
Proof: `ctest --test-dir build -R "^bot_pass_message$" --output-on-failure`

- [x] **C16** - Sem `ctl_set_bots`, os passes de Leste, Norte e Oeste mostram `Leste passou`, `Parceiro passou` e `Oeste passou`, e `seat_name` dos 4 assentos é vazio (AC 16)
Proof: `ctest --test-dir build -R "^no_bots_keeps_messages$" --output-on-failure`

### S4 - O resto continua igual · 2 files · ~10 KB · ~3k

- [x] **C17** - A suíte inteira passa, e nenhum arquivo de teste existente mudou desde `v1.0.0` (AC 17)
Proof: `ctest --test-dir build --output-on-failure`
Proof: `git diff --exit-code v1.0.0 -- tests/test_core.c tests/test_parceria.c tests/test_carroca.c tests/test_fora.c tests/test_menu.c tests/test_menu_app.c tests/test_menu_cli.c tests/test_window.c tests/test_app_anim.c tests/test_process.c tests/harness.h tests/fixtures.h tests/core_has_no_raylib.cmake`

## Coverage

| Set (size) | Member -> proof | Unproven |
| --- | --- | --- |
| bots (6) | C1, table-driven over all 6 | - |
| estilos x modo de decisão (6) | `STYLE_SEU_PAULO` C2 · `STYLE_NECA` C3 · `STYLE_AUDI` C4 · `STYLE_SEU_LEO` C5 · `STYLE_GUEDE` C6 · `STYLE_MANEZINHO` C7 | - |
| quantidade do parceiro vs própria (3 bordas) | menos C2, C3, C4, C5, C6 · igual C2, C3, C4, C5 · mais C2, C3, C4 | - |
| regras da cadeia (4) | P C2, C3, C4, C5 · A C5, C6 · J C4, C6 · D C7 | - |
| ordem P/A no modo parceiro (2) | P antes C4 · A antes C5 | - |
| estilos sem trapaça (6) | C8, table-driven over all 6 | - |
| elementos da tela inicial (12) | título C11 · rótulo do nome C11 · campo C11 · rótulo do parceiro C9, C11 · cartões 0..5 C10, C11 · `Jogar` C11 · `Sair` C11 | - |
| cliques na tela inicial (3) | cartão C10 · fora dos cartões C10 · `Jogar`/`Sair` existentes `menu_play_trims_name`, `menu_quit_click` | - |
| assentos com bot (3) | Norte C12, C13, C14 · Leste C12, C13, C14, C15 · Oeste C12, C13, C14 | - |
| começos de partida com bots (2) | `Jogar` C12 · `Nova partida` C13 | - |
| mensagens de passe (2 modos x 3 assentos) | com bots C15 (Leste; o texto vem do mesmo nome para os 3) · sem bots Leste C16 · Norte C16 · Oeste C16 | - |
| `Landing` doors (1) | door 1 C4, C8 | - |
| startup config: bots (2 assemblies) | `app_run` com o menu C12 · `ctl_set_bots` direto nos testes C12, C13, C15 | - |

- O C12 prova a `app_run` pelo gancho `menu_input`; o clique de mouse de verdade passa pela mesma
  `menu_click`, que já é chamada pela `domino.exe` e exercitada por `cli_menu_backspace_and_clicks`
- O C15 prova o texto com bot em um assento; os outros dois usam o mesmo caminho (`seat_name` do assento)

## Test policy

| Code | Required proofs | Coverage expectation |
| --- | --- | --- |
| Decide, sem fronteira | um no próprio nível | um caso afirmado por linha da tabela de decisão |
| Decide, alcançado pela `app_run` | um no próprio nível **e** um pela `app_run` | na `app_run`: escolher e começar; no próprio nível: a tabela inteira |
| Instrumentação (desenho dos cartões e dos nomes) | nenhum próprio | coberto pelos testes de layout e de `View` |

Evidence:

- `src/core/cpu.c`: 6 estilos x 3 modos de quantidade, 2 ordens de regra, modo só-desempate -> decide, sem fronteira
- `src/core/controller.c` (`ctl_set_bots`, Nova partida, mensagem de passe): sorteio de 2 entre 5, troca do texto da mensagem -> decide, sem fronteira
- `src/core/menu.c` (cartões): seleção por clique em 6 retângulos -> decide, alcançado pela `app_run`
- `src/ui/app.c`: passa `menu.partner` para `ctl_set_bots` -> decide pouco, alcançado pela `app_run` (C12)
- `src/ui/draw.c`: só desenha a partir da `View` e do layout -> instrumentação
- análogo no repo: `tests/test_parceria.c` prova `cpu_decide` por cenários de `CpuView`; `tests/test_menu_app.c` prova a `app_run` por gancho

Cost: 17 checks, 19 provas em 2 executáveis de teste novos.

## Swept

- validation: C10 - clique fora dos cartões não muda a seleção; `ctl_set_bots` só recebe índices da tabela, vindos do menu
- failure modes: n/a - nada é lido nem gravado; a tabela de bots é fixa
- idempotency: C10 - clicar de novo no cartão selecionado mantém a seleção
- authorization: n/a - jogo local de um só usuário
- concurrency: n/a - laço único, sem threads
- data lifecycle: n/a - a escolha do parceiro não é gravada (fora do escopo no plano)
- dependency failure: n/a - não há dependência nova
- state transitions: C12, C13 - menu para partida com bots; fim de partida para nova partida com novos adversários
- observability: C15, C16 - mensagens de passe são o único retorno textual; nenhum requisito de log

## Handoff

- Arquivos tocados: `domino.h` (4 KB), `cpu.c` (4 KB), `bots.c`/`.h` (novos, ~2 KB), `controller.c`/`.h`
  (13 KB), `menu.c`/`.h` (4 KB), `layout.c`/`.h` (7 KB), `app.c` (4 KB), `draw.c` (8 KB),
  `CMakeLists.txt` (7 KB), testes novos (~25 KB), artefatos (~25 KB). Total ~103 KB / 4 = ~26k, abaixo
  do orçamento de 150k - one builder

- **Boundary:** C1-C17 fechados no commit `feat(bots)` deste branch; suíte 114/114, sem avisos de compilação
- **Settled mid-build:** nome do Norte acima da fileira, não abaixo (resposta do usuário; premissa do plano atualizada)
- **Abandoned:** `menu_click(Menu *)` selecionando o cartão - gerava aviso `discards const` em `tests/test_menu.c`
