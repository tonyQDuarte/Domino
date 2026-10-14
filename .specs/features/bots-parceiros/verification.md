# Bots parceiros verification

**Verdict**: FAIL
**Profile**: standard
**Diff range**: 8419202..9241605
**Round**: 1 - full
**Verifier**: independent sub-agent (author != verifier)

Escopo: os 17 checks (C1-C17) de `checks.md`, o diff inteiro `8419202..HEAD` (8419202 = `v1.0.0`;
HEAD = 9241605). O produto mudou em `src/core/{bots.c,bots.h,controller.c,controller.h,cpu.c,domino.h,layout.c,layout.h,menu.c,menu.h}`,
`src/ui/{app.c,draw.c}` e `CMakeLists.txt`. Os testes novos são `tests/test_bots.c` e `tests/test_bots_app.c`.

Os 17 checks passam com evidência localizada. O FAIL vem de dois mutantes que sobreviveram à suíte inteira
(114/114 verde com cada um aplicado). Os dois deixam um membro de cobertura sem prova e uma linha de `Test policy`
não atendida.

## Binding sources

| Source | Opened | Contradiction | Uncovered |
| --- | --- | --- | --- |
| nenhuma fonte binding além das respostas do usuário no chat listadas em `plan.md` `## Sources` (não há arquivo de design). O passo 1 só roda no perfil `ui` e aqui não tem entrada | - | - | - |

## Checks

As provas rodaram no HEAD 9241605 em duas invocações sobre o mesmo build. A primeira foi
`ctest --test-dir build -L bots-parceiros -V`, com 17/17 aprovados. Cada teste aparece individualmente como
`Test #98 ... #114 ... Passed`. A segunda foi a suíte inteira, `ctest --test-dir build --output-on-failure`,
com `100% tests passed out of 114`. Os 16 nomes de `test_bots` estão registrados em `CMakeLists.txt` (laço
`foreach` do bloco `# bots-parceiros`), e `app_menu_partner_starts_game` está registrado logo abaixo. O harness
rejeita nome desconhecido: `./build/test_bots nonexistent_name` imprime `teste desconhecido` e sai com código 2.
Assim, nenhum filtro passa sem um teste por trás. Todas as provas ficam em arquivos que o diff criou.

| Check | Claim | Proof run | Evidence | Result |
| --- | --- | --- | --- | --- |
| C1 | 6 bots, nomes/textos/estilos na ordem, estilos distintos | `bots_table` #98 exit 0 | `tests/test_bots.c:93` - `BOT_COUNT == 6`; `:95-97` - `strcmp(BOTS[i].name, names[i]) == 0`, `strcmp(BOTS[i].style_text, texts[i]) == 0`, `BOTS[i].style == styles[i]` com as tabelas literais de `:88-92`; `:99` - `BOTS[i].style != BOTS[j].style` | PASS |
| C2 | Seu Paulo, cenário P, joga `[3\|0]` com parceiro 2, 4, 5 | `style_seu_paulo_always_partner` #99 exit 0 | `tests/test_bots.c:106-109` - `counts[] = {2, 4, 5}`, `picks(&v, 3, 0, -1)` | PASS |
| C3 | Neca: `[3\|0]` com 2 e 4, `[1\|6]` com 5 | `style_neca_partner_when_not_more` #100 exit 0 | `tests/test_bots.c:117`, `:119` - `picks(&v, 3, 0, -1)` (2 e 4); `:121` - `picks(&v, 1, 6, -1)` (5). A borda `igual` (4) separa `<=` de `<` (F3 morto) | PASS |
| C4 | `STYLE_AUDI == 0`; P: `[3\|0]` com 2, `[1\|6]` com 4 e 5; L: `[5\|6]`; J: `[1\|3]` esquerda | `style_audi_is_current_rule` #101 exit 0 | `tests/test_bots.c:127` - `STYLE_AUDI == 0`; `:129` - `picks(&v, 3, 0, -1)`; `:131`, `:133` - `picks(&v, 1, 6, -1)`; `:135` - `picks(&v, 5, 6, -1)`; `:137` - `picks(&v, 1, 3, END_LEFT)` | PASS |
| C5 | Seu Leo: L `[5\|2]`; P `[3\|0]` com 2, `[1\|6]` com 4 | `style_seu_leo_blocks_first` #102 exit 0 | `tests/test_bots.c:144` - `picks(&v, 5, 2, -1)`; `:146` - `picks(&v, 3, 0, -1)`; `:148` - `picks(&v, 1, 6, -1)`. O cenário L separa a ordem A,P da ordem P,A da Audi. Conferi à mão: P,A dá `[5\|6]` e A,P dá `[5\|2]`. A borda 4 separa o modo da Audi do da Neca | PASS |
| C6 | Guede: P(2) `[1\|6]`; L `[5\|2]`; J `[1\|3]` esquerda | `style_guede_never_partner` #103 exit 0 | `tests/test_bots.c:155` - `picks(&v, 1, 6, -1)`; `:157` - `picks(&v, 5, 2, -1)`; `:159` - `picks(&v, 1, 3, END_LEFT)`. Conferi à mão: sem A, o L daria `[5\|6]`; sem J, o J daria `[2\|6]` | PASS |
| C7 | Manezinho: P(2) `[1\|6]`; L `[5\|6]`; J `[2\|6]` direita | `style_manezinho_tiebreak_only` #104 exit 0 | `tests/test_bots.c:166` - `picks(&v, 1, 6, -1)`; `:168` - `picks(&v, 5, 6, -1)`; `:170` - `picks(&v, 2, 6, END_RIGHT)`. A regra A mudaria o L para `[5\|2]`, e a regra J mudaria o J para `[1\|3]` | PASS |
| C8 | 6 estilos x seeds 0..99: `cpu_view_of(g,s).style == g->styles[s]`; embaralhar o escondido não muda índice nem ponta de `cpu_choose` | `styles_decide_from_view_only` #105 exit 0 | `tests/test_bots.c:216-217` - `v.style == g.styles[g.turn]`, `v.style == (CpuStyle)style`; `:222-225` - `oka == okb`, `ia == ib`, `ea == eb` sobre `cpu_choose` (que é `cpu_view_of` -> `cpu_decide`, `src/core/cpu.c:141-145`); `:237` - `compared > 1000`. Exercita `Game.styles -> cpu_view_of -> cpu_decide` (F4 morto em `:216`) | PASS |
| C9 | `MENU_PARTNER_LABEL` = `Seu parceiro`; `partner == 2` com e sem nome | `menu_partner_default_audi` #106 exit 0 | `tests/test_bots.c:245` - `strcmp(MENU_PARTNER_LABEL, "Seu parceiro") == 0`; `:248`, `:250` - `m.partner == 2`; `:251` - `strcmp(BOTS[m.partner].name, "Audi") == 0` | PASS |
| C10 | clique no centro do cartão i seleciona, `menu_click` devolve `MENU_NONE`; ordem 5,0,3; fora não muda | `menu_card_click_selects` #107 exit 0 | `tests/test_bots.c:261` - `menu_click(&m, cx(r), cy(r)) == MENU_NONE`; `:262-265` - `menu_pick_partner(...)`, `m.partner == i` (inclusive o segundo clique); `:270-271` - `m.partner == order[k]`; `:274-279` - `!menu_pick_partner(...)` no título e em (20,700), `m.partner == 3` | PASS |
| C11 | `Jogar`/`Sair` nos retângulos de `tela-inicial`; 12 retângulos na janela, de cima para baixo, sem sobreposição | `menu_partner_layout` #108 exit 0 | `tests/test_bots.c:290-291` - `rect_eq(menu_play_rect(), 540, 360, 200, 55)`, `rect_eq(menu_quit_rect(), 540, 440, 200, 55)`; `:304` - dentro de `SCREEN_W`/`SCREEN_H`; `:306` - `!rect_overlaps(r[i], r[j])`; `:310` - ordem título/rótulo/campo/rótulo do parceiro; `:313-317` - faixa única, abaixo do rótulo, acima de `Jogar`, esquerda->direita; `:319` - `Sair` abaixo de `Jogar` | PASS |
| C12 | `ctl_set_bots` p=0..5 x seeds 0..99: Norte=p, E/O distintos e != p, `styles` certos, mesma seed = mesmos adversários, mãos iguais às sem bots, cada um dos 5 aparece; `app_run` com Seu Paulo | `bots_seating_by_seed` #109, `app_menu_partner_starts_game` #114 exit 0 | `tests/test_bots.c:331` - `bots_ok(&a, p)` (definido em `:73-82`: Norte, distintos, `game.styles` = `BOTS[..].style`); `:334` - mesma seed, mesmos E/O; `:336-337` - `memcmp(a.game.hands, plain.game.hands, ...) == 0`, idem `sleeping`; `:341` - `seen[i] == (i != p)`; `tests/test_bots_app.c:58` - `strcmp(BOTS[a.bot[SEAT_NORTH]].name, "Seu Paulo") == 0`; `:61` - `e != w && e != 0 && w != 0`; `:62-63` - estilos; `:64` - segunda execução, mesmos adversários. Passa como está escrito, mas tem dois furos, ambos com mutante sobrevivente (veja Faults): (a) as mãos só são comparadas na 1ª mão; (b) na `app_run`, quem escolhe o cartão é o gancho do teste, não a `app_run` | PASS |
| C13 | Nova partida mantém o Norte, E/O válidos e `styles` certos; >= 2 pares em 20 cliques | `new_match_keeps_partner_redraws` #110 exit 0 | `tests/test_bots.c:357` - `c.game.phase == PHASE_PLAYING`; `:358` - `bots_ok(&c, 1)`; `:369` - `distinct >= 2`. Uma Nova partida que mantém os mesmos adversários é pega (F5 morto em `:369`) | PASS |
| C14 | `seat_name` N/L/O com o nome e Sul vazio; rótulos na janela, sem tocar fileiras (1..6), mesa, placar, peças de fora, mensagem e outros rótulos; Norte com `y+h <= 40` e centro x 640; L/O acima da coluna de 6 e centrados | `bot_names_on_table` #111 exit 0 | `tests/test_bots.c:382` - `v.seat_name[SEAT_SOUTH][0] == '\0'`; `:385` - `strcmp(v.seat_name[..], BOTS[c.bot[..]].name) == 0`; `:396-404` - janela, `layout_table_area()`, `score`, `layout_message()`, `sleeping[i]`, outros rótulos; `:413` - fileiras dos 4 assentos com 1..`HAND_SIZE` (=6, `src/core/domino.h:8`); `:419-421` - `n.y + n.h <= 40`, `<= row[0].y`, `cx(n) == 640`; `:426-427` - `r.y + r.h <= row[0].y`, `cx(r) == cx(row[0])` | PASS |
| C15 | Seu Leo no Leste sem jogada: `Seu Leo passou` aos 800 ms, visível 1499 ms depois, some aos 1500 | `bot_pass_message` #112 exit 0 | `tests/test_bots.c:456` - escolhe a seed com `c.bot[SEAT_EAST] == 3`; `:464-465` - `v.message_visible`, `strcmp(v.message, "Seu Leo passou") == 0`; `:468-469` - ainda visível após +1499; `:472` - `!v.message_visible` após +1 | PASS |
| C16 | sem `ctl_set_bots`: `Leste passou`, `Parceiro passou`, `Oeste passou`; `seat_name` vazio nos 4 | `no_bots_keeps_messages` #113 exit 0 | `tests/test_bots.c:479` - os três textos; `:487-488` - `v.message_visible`, `strcmp(v.message, texts[k]) == 0`; `:490` - `v.seat_name[s][0] == '\0'` | PASS |
| C17 | suíte inteira passa; nenhum arquivo de teste antigo mudou desde `v1.0.0` | `ctest --test-dir build --output-on-failure` 114/114; `git diff --exit-code v1.0.0 -- tests/test_core.c ... tests/core_has_no_raylib.cmake` exit 0 | `100% tests passed out of 114`; o diff saiu com 0. `git diff --stat v1.0.0 HEAD -- tests` lista só `test_bots.c` e `test_bots_app.c`, e a lista de `tests/` em `v1.0.0` (13 arquivos) é exatamente a do comando de prova. `CMakeLists.txt` só ganhou linhas no fim; nenhum registro antigo mudou | PASS |

## Coverage

Recalculado a partir de `plan.md` (AC 1-17, a tabela "Os seis estilos", Flow 1-6, Landing door 1, Assumptions) e
do código (`src/core/cpu.c:99-126`, `src/core/controller.c:42-71,133-139,160-161`, `src/ui/app.c:70-87`,
`src/ui/main.c:23`).

| Set (size) | Recomputed from | Member -> proof | Unproven |
| --- | --- | --- | --- |
| bots (6) | AC 1, `src/core/bots.c:3-10` | os 6 C1 (`tests/test_bots.c:94-97`, tabela literal) | - |
| estilos x modo (6) | tabela "Os seis estilos", `cpu.c:100-106` | Seu Paulo C2 · Neca C3 · Audi C4 · Seu Leo C5 · Guede C6 · Manezinho C7 | - |
| quantidade do parceiro vs própria (3 bordas) | AC 2-5, `cpu.c:101-105` | menos C2-C7 · igual C2, C3, C4, C5 · mais C2, C3, C4. Guede e Manezinho são constantes e a borda `menos` já os separa de `true`. Seu Leo usa o ramo `default` da Audi, e a borda `igual` o separa da Neca | - |
| regras da cadeia (4) | tabela "Os seis estilos" | P C2-C5 · A C5, C6 (L) · J C4, C6 (J) · D C7 (P, L, J) | - |
| ordem P/A no modo parceiro (2) | AC 5, `cpu.c:110-115` | P antes C4 `:135` · A antes C5 `:144` | - |
| estilos sem trapaça (6) | AC 8 | os 6 C8 (`tests/test_bots.c:204` percorre 0..5) | - |
| elementos da tela inicial (12) | AC 9, AC 11 | título, rótulo do nome, campo, rótulo do parceiro, 6 cartões, `Jogar`, `Sair` C11 (`:294-301`); o texto do rótulo C9 | - |
| cliques no cartão, nível de decisão (2) | AC 10, `menu.c:74-82` | dentro C10 · fora C10 | - |
| cliques na tela inicial, mapeamento real na `app_run` (3) | Flow 1, `src/ui/app.c:70-76` | `Jogar` `cli_menu_backspace_and_clicks` (`tests/test_menu_cli.c:176`) · `Sair` mesmo teste (`:188`) · cartão -> `menu_pick_partner(&menu, m.x, m.y)` (`app.c:72`): **nenhuma prova**. O gancho `menu_input` de `tests/test_bots_app.c:25` chama `menu_pick_partner` por conta própria e pula `app.c:64-76`; o teste de processo só clica em `Jogar` e `Sair`. F1 (apagar `app.c:72`) sobreviveu com 114/114 | cartão (`app.c:72`) |
| assentos com bot (3) | AC 12, AC 14 | Norte C12, C13, C14 · Leste C12-C15 · Oeste C12, C13, C14 | - |
| começos de partida com bots (2) | AC 12, AC 13 | `Jogar` C12 · `Nova partida` C13 | - |
| mãos que a seed dá, com e sem bots (2) | plan Assumption "Sorteio dos adversários" (RNG próprio, "os adversários mudam sem alterar as mãos que a seed já dá"), Flow 3 | 1ª mão C12 `tests/test_bots.c:336-337` · mãos seguintes da partida: **nenhuma prova**. F2 (sortear os adversários com `c->game.rng`, o RNG da distribuição) sobreviveu com 114/114. Esse mutante muda a 2ª mão em diante (`src/core/rules.c:52` usa o mesmo estado), mas a 1ª mão já foi distribuída em `ctl_init` | mãos seguintes |
| mensagens de passe (2 modos x 3 assentos) | AC 15, AC 16, `controller.c:133-139` | com bots: Leste C15; Norte e Oeste vêm do mesmo ramo `c->bot[s] >= 0` (`controller.c:133`), aceito como no `checks.md` · sem bots: Leste, Norte, Oeste C16 | - |
| Landing door 1 (1) | Landing | `STYLE_AUDI == 0` C4 `:127` · estilo passa pelo `CpuView` C8 `:216` · os `CpuView` antigos zerados continuam Audi: C17 (`test_parceria` sem mudança e verde) | - |
| startup config: bots (2 assemblies) | lido direto | `src/ui/main.c:23` `opt.show_menu = true;` -> `app.c:87` `ctl_set_bots(&c, menu.partner, opt->seed);` C12 (#114, montagem `tests/test_bots_app.c:42-48`) · `ctl_set_bots` direto C12, C13, C15 · caminho sem menu (`app.c:50-54`, sem bots) C16 no nível do controlador | - |

Observação que não bloqueia: `Game.styles` sobrevive a `game_next_hand` e a `game_restart_match` porque
`src/core/rules.c:103-120` não zera o `Game` (só `game_new_match` faz `memset`, em `:112`). Nenhum teste joga até a
2ª mão com bots para afirmar isso, mas nenhum artefato enumera esse membro e o código é anterior à feature.

## Test policy rows

| Row | Files it classifies | Required proof | Expectation met |
| --- | --- | --- | --- |
| Decide, sem fronteira | `src/core/cpu.c`; `src/core/controller.c` (`ctl_set_bots`, Nova partida, mensagem de passe) | um no próprio nível | no - `cpu.c` tem a tabela inteira (C2-C8; F3 e F4 mortos). Em `controller.c`, porém, a decisão "RNG separado do da distribuição" (Flow 3) só é afirmada na 1ª mão (`tests/test_bots.c:336-337`), e F2 sobreviveu |
| Decide, alcançado pela `app_run` | `src/core/menu.c` (cartões); `src/ui/app.c` | próprio nível **e** `app_run` | no - o próprio nível está completo (C9-C11) e a `app_run` prova o "começar" (`tests/test_bots_app.c:58-64`). O "escolher" na `app_run` não tem prova: quem escolhe é o gancho do teste (`tests/test_bots_app.c:25`), e o desvio do clique real para `menu_pick_partner` (`app.c:72`) está sem prova (F1 sobreviveu). A nota de Coverage do `checks.md` diz que "o clique de mouse de verdade passa pela mesma `menu_click`", e isso está errado: a seleção do cartão passa por `menu_pick_partner`, que `menu_click` não chama (`src/core/menu.c:74-91`) |
| Instrumentação (desenho dos cartões e dos nomes) | `src/ui/draw.c` | nenhum próprio | yes - os cartões vêm de `BOTS[i]` e `menu_card_rect(i)` (C1, C11), o destaque vem de `m->partner` (C9, C10), e os nomes vêm de `v.seat_name` e `layout_bot_name` (C14, C16) |

Swept: nenhuma linha de `## Swept` cita uma restrição como já existente. As que citam checks (C10, C12, C13, C15,
C16) foram conferidas acima, e as linhas `n/a` são política aprovada.

## Faults injected

Isolamento: worktree em `scratchpad/verify-wt` a partir do HEAD 9241605, com build próprio em `bld` (os mesmos
comandos cmake). Antes da primeira falha, o baseline no worktree deu 17/17. O `git status --porcelain` da árvore
real estava vazio antes e depois, e os dois arquivos são idênticos byte a byte (`cmp`). O worktree foi removido com
`git worktree remove --force`, `git worktree list` mostra só a árvore real, e `git stash` não foi usado (a lista
está vazia). Cada falha foi revertida com `git checkout --` no próprio worktree antes da seguinte. Foram 5
mutações, o teto.

| Mutation | Location | Killed |
| --- | --- | --- |
| F1 apagar `menu_pick_partner(&menu, m.x, m.y);` do clique de mouse da tela inicial | `src/ui/app.c:72` | no - suíte inteira 114/114. Na `domino.exe`, clicar num cartão não troca mais o parceiro |
| F2 `bot_next(&c->bot_rng)` -> `bot_next(&c->game.rng)` nas duas chamadas de `draw_opponents` (adversários sorteados pelo RNG da distribuição) | `src/core/controller.c:58`, `:61` | no - suíte inteira 114/114 |
| F3 Neca `counts[partner] <= own.count` -> `<` | `src/core/cpu.c:102` | yes - `style_neca_partner_when_not_more` falhou em `tests/test_bots.c:119` (`picks(&v, 3, 0, -1)`) |
| F4 apagar `v.style = g->styles[s];` em `cpu_view_of` | `src/core/cpu.c:26` | yes - `styles_decide_from_view_only` falhou em `tests/test_bots.c:216` (`v.style == g.styles[g.turn]`) |
| F5 Nova partida sem novo sorteio: `if (c->bot[SEAT_NORTH] >= 0)` -> `if (0)` | `src/core/controller.c:160` | yes - `new_match_keeps_partner_redraws` falhou em `tests/test_bots.c:369` (`distinct >= 2`) |

As superfícies de C1, C5-C7 e C9-C16 não receberam uma falha própria por causa do teto de 5. Para os estilos, fiz
uma análise à mão no lugar da mutação (colunas de C5, C6 e C7): Seu Leo com a ordem da Audi, Guede sem A ou sem J,
e Manezinho com A ou com J mudam cada um pelo menos uma jogada afirmada.

## Gate

`ctest --test-dir build --output-on-failure` no 9241605: 114 passed, 0 failed (os 17 testes de bots-parceiros
aparecem um a um com `-L bots-parceiros -V`, #98-#114). `git diff --exit-code v1.0.0 -- <arquivos de teste antigos>`
saiu com 0. O `validate_verification.py` não rodou porque o Python não está instalado nesta máquina. O script não
validou este veredito. A consistência foi conferida à mão: há duas células `Killed` com `no`, duas células
`Unproven` preenchidas e duas linhas de `Test policy` com `no`, todas compatíveis com o veredito FAIL.

Lacunas, da mais grave para a menos grave:

1. A escolha do parceiro por clique real na `domino.exe` não tem prova. F1 sobreviveu. Afeta o C12 e o AC 10/12 na
   fronteira da `app_run`, em `src/ui/app.c:72`. A prova do C12 (`tests/test_bots_app.c:25`) faz a seleção dentro
   do gancho. Faltam um clique num cartão pelo processo real (como `tests/test_menu_cli.c:176`) e uma afirmação do
   bot do Norte resultante, ou um gancho de mouse que passe por `app.c:70-76`.
2. "RNG dos adversários separado do da distribuição" só está provado na 1ª mão. F2 sobreviveu. Afeta o C12, em
   `tests/test_bots.c:336-337` e `src/core/controller.c:58,61`. Falta comparar as mãos seguintes (por exemplo, a
   2ª mão com `game_next_hand`, ou a mão depois de uma Nova partida) entre um controlador com bots e um sem bots
   com a mesma seed. Isso também é uma lacuna de precisão do check: o texto "as mãos distribuídas" não diz quais
   mãos.
3. A nota de Coverage do `checks.md` ("o clique de mouse de verdade passa pela mesma `menu_click`") está errada
   depois da mudança registrada na nota do build do C10. Não bloqueia sozinha, mas foi ela que escondeu a lacuna 1.
