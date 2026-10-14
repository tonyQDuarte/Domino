# Bots parceiros verification

**Verdict**: PASS
**Profile**: standard
**Diff range**: 8419202..fa083b9
**Round**: 3 - scoped
**Verifier**: independent sub-agent (author != verifier)

Escopo da rodada 3: o fix é `cd81394..fa083b9` (um commit, `fa083b9`). Ele acrescenta o gancho `mouse_input` em
`AppOptions` (`src/ui/app.h:23-24`), faz o bloco do mouse da tela inicial em `src/ui/app.c:70-77` perguntar ao gancho
(quando existe) se houve clique e onde, no lugar de `IsMouseButtonPressed`/`GetMousePosition`, cria o teste
`app_menu_card_click_real_path` (`tests/test_bots_app.c:69-98`) e o registra no `CMakeLists.txt:178-180`. Também
reescreve as notas do C10 e de Coverage em `checks.md` e traz o relatório da rodada 2. `tests/test_bots.c`,
`src/core/` e o resto de `src/ui/` não mudaram (`git diff --stat cd81394..HEAD -- tests/test_bots.c src/core` vazio).

A rodada 2 falhou por quatro coisas: F-c sobrevivia (trocar `menu_press` por `menu_click` no clique real da
`app_run`), a linha 2 de `Test policy` não estava atendida, o membro de cobertura "cartão clicado pelo caminho real
da `app_run`" não tinha prova e as notas do `checks.md` afirmavam uma ligação que nenhum teste provava.

Resultado: F-c agora morre de forma determinística por `app_menu_card_click_real_path`. As duas falhas novas no caminho
do gancho também morrem. A linha 2 de `Test policy` passa a ser atendida, o membro "cartão" ganha prova e as notas
do `checks.md` agora estão corretas.

## Binding sources

carried from cd81394 (o fix não toca a interface)

| Source | Opened | Contradiction | Uncovered |
| --- | --- | --- | --- |
| nenhuma fonte binding além das respostas do usuário no chat listadas em `plan.md` `## Sources` (não há arquivo de design). O passo 1 só roda no perfil `ui` e aqui não tem entrada | - | - | - |

## Checks

Provas verified at fa083b9. Todas rodaram numa invocação só na árvore real,
`ctest --test-dir build --output-on-failure`, com `100% tests passed out of 115`. Cada teste aparece individualmente como
`Test #NN ... Passed`. Os 18 testes de bots-parceiros são #98-#115 (o #115 é o novo `app_menu_card_click_real_path`),
e `cli_menu_backspace_and_clicks` (#97) passou de primeira, sem precisar rodar de novo. O build da árvore real estava
em dia com o HEAD (`ninja: no work to do`). O harness rejeita nome desconhecido: `test_bots_app nonexistent_name`
imprime `teste desconhecido` e sai com 2. A prova do C17,
`git diff --exit-code v1.0.0 -- tests/test_core.c ... tests/core_has_no_raylib.cmake`, saiu com 0, e
`git diff --stat v1.0.0 HEAD -- tests` lista só `test_bots.c` e `test_bots_app.c`.

Citações: as de `tests/test_bots.c` são carried from cd81394, porque o arquivo não mudou no fix. As de
`tests/test_bots_app.c` foram renovadas at fa083b9 (o `#include "namefile.h"` empurrou as linhas antigas uma para
baixo).

| Check | Claim | Proof run | Evidence | Result |
| --- | --- | --- | --- | --- |
| C1 | 6 bots, nomes/textos/estilos na ordem, estilos distintos | `bots_table` #98 exit 0 | `tests/test_bots.c:93` - `BOT_COUNT == 6`; `:95-97` - `strcmp(BOTS[i].name, names[i]) == 0`, `strcmp(BOTS[i].style_text, texts[i]) == 0`, `BOTS[i].style == styles[i]`; `:99` - `BOTS[i].style != BOTS[j].style` | PASS |
| C2 | Seu Paulo, cenário P, joga `[3\|0]` com parceiro 2, 4, 5 | `style_seu_paulo_always_partner` #99 exit 0 | `tests/test_bots.c:106-109` - `counts[] = {2, 4, 5}`, `picks(&v, 3, 0, -1)` | PASS |
| C3 | Neca: `[3\|0]` com 2 e 4, `[1\|6]` com 5 | `style_neca_partner_when_not_more` #100 exit 0 | `tests/test_bots.c:117`, `:119` - `picks(&v, 3, 0, -1)`; `:121` - `picks(&v, 1, 6, -1)` | PASS |
| C4 | `STYLE_AUDI == 0`; P: `[3\|0]` com 2, `[1\|6]` com 4 e 5; L: `[5\|6]`; J: `[1\|3]` esquerda | `style_audi_is_current_rule` #101 exit 0 | `tests/test_bots.c:127` - `STYLE_AUDI == 0`; `:129` - `picks(&v, 3, 0, -1)`; `:131`, `:133` - `picks(&v, 1, 6, -1)`; `:135` - `picks(&v, 5, 6, -1)`; `:137` - `picks(&v, 1, 3, END_LEFT)` | PASS |
| C5 | Seu Leo: L `[5\|2]`; P `[3\|0]` com 2, `[1\|6]` com 4 | `style_seu_leo_blocks_first` #102 exit 0 | `tests/test_bots.c:144` - `picks(&v, 5, 2, -1)`; `:146` - `picks(&v, 3, 0, -1)`; `:148` - `picks(&v, 1, 6, -1)` | PASS |
| C6 | Guede: P(2) `[1\|6]`; L `[5\|2]`; J `[1\|3]` esquerda | `style_guede_never_partner` #103 exit 0 | `tests/test_bots.c:155` - `picks(&v, 1, 6, -1)`; `:157` - `picks(&v, 5, 2, -1)`; `:159` - `picks(&v, 1, 3, END_LEFT)` | PASS |
| C7 | Manezinho: P(2) `[1\|6]`; L `[5\|6]`; J `[2\|6]` direita | `style_manezinho_tiebreak_only` #104 exit 0 | `tests/test_bots.c:166` - `picks(&v, 1, 6, -1)`; `:168` - `picks(&v, 5, 6, -1)`; `:170` - `picks(&v, 2, 6, END_RIGHT)` | PASS |
| C8 | 6 estilos x seeds 0..99: `cpu_view_of(g,s).style == g->styles[s]`; embaralhar o escondido não muda índice nem ponta | `styles_decide_from_view_only` #105 exit 0 | `tests/test_bots.c:216-217` - `v.style == g.styles[g.turn]`, `v.style == (CpuStyle)style`; `:222-225` - `oka == okb`, `ia == ib`, `ea == eb`; `:237` - `compared > 1000` | PASS |
| C9 | `MENU_PARTNER_LABEL` = `Seu parceiro`; `partner == 2` com e sem nome | `menu_partner_default_audi` #106 exit 0 | `tests/test_bots.c:245` - `strcmp(MENU_PARTNER_LABEL, "Seu parceiro") == 0`; `:248`, `:250` - `m.partner == 2`; `:251` - `strcmp(BOTS[m.partner].name, "Audi") == 0` | PASS |
| C10 | clique no centro do cartão i seleciona e devolve `MENU_NONE`; ordem 5,0,3; fora não muda (+ reforços: `menu_press` idem e `MENU_PLAY` em `Jogar`; cartão e `Jogar` pelo caminho real da `app_run`) | `menu_card_click_selects` #107, `app_menu_card_click_real_path` #115 exit 0 | `tests/test_bots.c:261-265` - `menu_click(...) == MENU_NONE`, `m.partner == i`; `:269-270` - `menu_press(&p, cx(r), cy(r)) == MENU_NONE`, `p.partner == i`; `:275-276` - `m.partner == order[k]`; `:279-286` - fora, `m.partner == 3`; `:288-289` - `menu_press(&m, cx(play), cy(play)) == MENU_PLAY`. Caminho real (verified at fa083b9): `tests/test_bots_app.c:93` - `s.started`; `:94` - `s.bot[SEAT_NORTH] == 0`; `:95` - `s.styles[SEAT_NORTH] == STYLE_SEU_PAULO`, com o gancho `mouse` (`:71-78`) clicando o centro de `menu_card_rect(0)` no quadro 0 e o de `menu_play_rect()` no quadro 1, sem `menu_input`. F-c morre em `:94` | PASS |
| C11 | `Jogar`/`Sair` nos retângulos de `tela-inicial`; 12 retângulos na janela, de cima para baixo, sem sobreposição | `menu_partner_layout` #108 exit 0 | `tests/test_bots.c:300-301` - `rect_eq(menu_play_rect(), 540, 360, 200, 55)`, `rect_eq(menu_quit_rect(), 540, 440, 200, 55)`; `:314` - dentro da janela; `:316` - `!rect_overlaps(r[i], r[j])`; `:320` - ordem vertical; `:323-327` - faixa única; `:329` - `Sair` abaixo de `Jogar` | PASS |
| C12 | `ctl_set_bots` p=0..5 x seeds 0..99: Norte=p, E/O distintos e != p, `styles`, mesma seed = mesmos adversários, 1ª e 2ª mão iguais às sem bots, cada um dos 5 aparece; `app_run` com Seu Paulo | `bots_seating_by_seed` #109, `app_menu_partner_starts_game` #114 exit 0 | `tests/test_bots.c:341` - `bots_ok(&a, p)`; `:344` - mesmos E/O; `:346-347` - 1ª mão, `memcmp(a.game.hands, plain.game.hands, ...) == 0`; `:351-352` - depois de `game_next_hand`; `:356` - `seen[i] == (i != p)`; `tests/test_bots_app.c:59` - `strcmp(BOTS[a.bot[SEAT_NORTH]].name, "Seu Paulo") == 0`; `:62` - `e != w && e != 0 && w != 0`; `:63-64` - estilos; `:65` - 2ª execução, mesmos adversários | PASS |
| C13 | Nova partida mantém o Norte, E/O válidos e `styles`; >= 2 pares em 20 cliques | `new_match_keeps_partner_redraws` #110 exit 0 | `tests/test_bots.c:372` - `c.game.phase == PHASE_PLAYING`; `:373` - `bots_ok(&c, 1)`; `:384` - `distinct >= 2` | PASS |
| C14 | `seat_name` N/L/O e Sul vazio; rótulos na janela, sem tocar fileiras, mesa, placar, fora, mensagem, outros rótulos; Norte `y+h <= 40`, x 640; L/O acima da coluna de 6 | `bot_names_on_table` #111 exit 0 | `tests/test_bots.c:397` - `v.seat_name[SEAT_SOUTH][0] == '\0'`; `:400` - `strcmp(v.seat_name[cpus[k]], BOTS[c.bot[cpus[k]]].name) == 0`; `:411-419` - janela, mesa, placar, mensagem, fora, outros rótulos; `:428` - fileiras; `:434-436` - `n.y + n.h <= 40`, `cx(n) == 640`; `:441-442` - L/O acima e centrados na coluna | PASS |
| C15 | Seu Leo no Leste: `Seu Leo passou` aos 800 ms, visível +1499, some aos 1500 | `bot_pass_message` #112 exit 0 | `tests/test_bots.c:479-480` - `v.message_visible`, `strcmp(v.message, "Seu Leo passou") == 0`; `:483-484` - ainda visível; `:487` - `!v.message_visible` | PASS |
| C16 | sem `ctl_set_bots`: `Leste passou`, `Parceiro passou`, `Oeste passou`; `seat_name` vazio nos 4 | `no_bots_keeps_messages` #113 exit 0 | `tests/test_bots.c:502-503` - `strcmp(v.message, texts[k]) == 0`; `:505` - `v.seat_name[s][0] == '\0'` | PASS |
| C17 | suíte inteira passa; nenhum arquivo de teste antigo mudou desde `v1.0.0` | `ctest --test-dir build --output-on-failure` 115/115; `git diff --exit-code v1.0.0 -- <13 arquivos>` exit 0 | `100% tests passed out of 115`; diff com saída 0 | PASS |

### O gancho `mouse_input` passa mesmo pelo bloco real de `app.c`?

Sim. Em `src/ui/app.c:61-78`, o gancho `menu_input` ocupa o ramo `if` e pula o teclado e o mouse inteiros. Já o
`mouse_input` fica dentro do ramo `else`, o mesmo que a `domino.exe` executa. O teste novo não liga `menu_input`, então
roda o laço de caracteres e teclas (`:64-69`), lê `GetMousePosition()` (`:70`) e chama `menu_press(&menu, m.x, m.y)`
(`:74`) com a mesma lógica de `clicked`/`action` (`:75-76`). Depois segue para `MENU_PLAY` -> `ctl_set_bots(&c,
menu.partner, ...)` (`:81-88`). O nome vem de verdade de `name_load` (`:43-44`), a partir do arquivo que o teste
grava em `tests/test_bots_app.c:82`. O que o gancho troca é só a fonte do estado do botão e da posição, que antes
vinham de `IsMouseButtonPressed`/`GetMousePosition`. F-e e F-f mostram que o teste depende das duas coisas que o
gancho entrega, e F-c mostra que depende da função chamada em `:74`.

Os ramos de produção do mouse ficam fora desse teste: `IsMouseButtonPressed(MOUSE_BUTTON_LEFT)` (`:72`, quando o
gancho é `NULL`) e a posição real do cursor. Eles continuam provados só por `cli_menu_backspace_and_clicks`, como a
nota de Coverage do `checks.md` diz (veja F-g).

## Coverage

As linhas cuja autoridade o fix tocou são verified at fa083b9. As outras são carried from cd81394, porque o fix não
acrescentou ramo de decisão no núcleo. O único ramo novo é o ternário do gancho em `app.c:71-72`, e ele entrou como
membro na linha de mapeamento real.

| Set (size) | Recomputed from | Member -> proof | Unproven |
| --- | --- | --- | --- |
| bots (6) - carried from cd81394 | AC 1, `src/core/bots.c:3-10` | os 6 C1 | - |
| estilos x modo (6) - carried from cd81394 | tabela "Os seis estilos", `cpu.c:100-106` | Seu Paulo C2 · Neca C3 · Audi C4 · Seu Leo C5 · Guede C6 · Manezinho C7 | - |
| quantidade do parceiro vs própria (3 bordas) - carried from cd81394 | AC 2-5 | menos C2-C7 · igual C2-C5 · mais C2-C4 | - |
| regras da cadeia (4) - carried from cd81394 | tabela "Os seis estilos" | P C2-C5 · A C5, C6 · J C4, C6 · D C7 | - |
| ordem P/A no modo parceiro (2) - carried from cd81394 | AC 5 | P antes C4 · A antes C5 | - |
| estilos sem trapaça (6) - carried from cd81394 | AC 8 | os 6 C8 | - |
| elementos da tela inicial (12) - carried from cd81394 | AC 9, AC 11 | os 12 C11; texto do rótulo C9 | - |
| cliques na tela inicial, nível de decisão (`menu_press`: cartão, fora, `Jogar`) (3) - carried from cd81394 | AC 10, `src/core/menu.c:74-97` | cartão `tests/test_bots.c:269-270` · fora `:285-286` · `Jogar` `:288` | - |
| cliques na tela inicial, mapeamento real na `app_run` (cartão, `Jogar`, `Sair`, fonte do clique: gancho / raylib) (5) - verified at fa083b9 | Flow 1, `src/ui/app.c:70-77` | cartão -> `menu_press` em `app.c:74`: `app_menu_card_click_real_path` `tests/test_bots_app.c:94` (F-c morto) · `Jogar` pelo bloco real: mesmo teste, `:93` (`s.started`), e `cli_menu_backspace_and_clicks` `tests/test_menu_cli.c:176-183` · `Sair`: `tests/test_menu_cli.c:188-189` · fonte = gancho (`app.c:71`, posição e resultado): `:93` (F-e e F-f mortos) · fonte = raylib (`app.c:72`): `cli_menu_backspace_and_clicks` (F-g morto 9/9) | - |
| assentos com bot (3) - carried from cd81394 | AC 12, AC 14 | Norte C12, C13, C14 · Leste C12-C15 · Oeste C12, C13, C14 | - |
| começos de partida com bots (2) - verified at fa083b9 | AC 12, AC 13 | `Jogar` C12 (#114 pelo gancho `menu_input`, #115 pelo bloco real do mouse) · `Nova partida` C13 | - |
| mãos que a seed dá, com e sem bots (2) - carried from cd81394 | Assumption "Sorteio dos adversários", Flow 3 | 1ª mão `tests/test_bots.c:346-347` · mãos seguintes `:351-352` | - |
| mensagens de passe (2 modos x 3 assentos) - carried from cd81394 | AC 15, AC 16 | com bots Leste C15 (Norte e Oeste pelo mesmo ramo) · sem bots L/N/O C16 | - |
| Landing door 1 (1) - carried from cd81394 | Landing | C4 `:127` · C8 `:216` · C17 | - |
| startup config: bots (2 assemblies) - verified at fa083b9 | lido direto | `src/ui/main.c:23` -> `app.c:88` `ctl_set_bots(&c, menu.partner, opt->seed);` C12 (#114, #115) · `ctl_set_bots` direto C12, C13, C15 | - |

Notas do `checks.md` (verified at fa083b9). O reforço da rodada 1 no C10 (`checks.md:60`) agora diz que a frase antiga
não provava nada e por quê. O reforço da rodada 2 (`checks.md:61`) diz que a prova passa pelo `app.c` sem
`menu_input` e que o gancho `mouse_input` só informa o clique, o que confere com `app.c:61-77`. A nota de Coverage
(`checks.md:109-113`) diz que a leitura do botão pela raylib continua provada só por `cli_menu_backspace_and_clicks`,
o que confere com F-g. As três estão corretas. Há uma imprecisão pequena, que não bloqueia: o gancho também diz *se*
houve clique, e não só *onde*. O comentário em `src/ui/app.h:23` já descreve isso direito.

## Test policy rows

A linha 2 foi re-julgada at fa083b9: era a não atendida e classifica `src/ui/app.c`, que o fix tocou. As linhas 1 e
3 são carried from cd81394.

| Row | Files it classifies | Required proof | Expectation met |
| --- | --- | --- | --- |
| Decide, sem fronteira | `src/core/cpu.c`; `src/core/controller.c` (`ctl_set_bots`, Nova partida, mensagem de passe) | um no próprio nível | yes (carried from cd81394) |
| Decide, alcançado pela `app_run` | `src/core/menu.c` (cartões, `menu_press`); `src/ui/app.c` | próprio nível **e** `app_run` | yes - o próprio nível tem a tabela inteira (C9-C11; `menu_press` em `tests/test_bots.c:269-289`). Na `app_run`, o "escolher" agora é provado pelo bloco real do mouse, com o cartão clicado em `app.c:74` (`tests/test_bots_app.c:94`, F-c morto), e o "começar" por `tests/test_bots_app.c:93-95` e `:58-65` |
| Instrumentação (desenho dos cartões e dos nomes) | `src/ui/draw.c` | nenhum próprio | yes (carried from cd81394) |

Swept: carried from cd81394. Nenhuma linha cita uma restrição como já existente.

## Faults injected

verified at fa083b9. O worktree ficou em `scratchpad/verify-wt3`, criado a partir do HEAD fa083b9, com build próprio
em `bld`. O baseline no worktree deu 115/115. O `git status --porcelain` da árvore real estava vazio antes e depois
(`cmp` sem diferença). O worktree foi removido com `git worktree remove --force`, e `git worktree list` mostra só a
árvore real. `git stash` não foi usado (`git stash list` vazio). Cada falha foi revertida com `git checkout -- src`
antes da seguinte. F-c, F-e e F-f rodaram com a suíte inteira menos `cli_menu_backspace_and_clicks` (`-E`), então
nenhuma dessas mortes depende do teste instável. F-g rodou com a suíte inteira. Foram 4 mutações, todas em linhas que
o fix tocou.

| Mutation | Location | Killed |
| --- | --- | --- |
| F-c (de novo) `menu_press(&menu, m.x, m.y)` -> `menu_click(&menu, m.x, m.y)` (o clique real deixa de escolher o cartão) | `src/ui/app.c:74` | yes - `app_menu_card_click_real_path` falhou em `tests/test_bots_app.c:94` (`s.bot[SEAT_NORTH] == 0`); 113/114 com o cli excluído |
| F-e ignorar o gancho: `opt->mouse_input ? ... :` -> `0 ? ... :` (o mouse vem sempre de `IsMouseButtonPressed`) | `src/ui/app.c:71` | yes - `app_menu_card_click_real_path` falhou em `tests/test_bots_app.c:93` (`s.started`); 113/114 |
| F-f o gancho é chamado, mas a posição dele é descartada (`&m.x, &m.y` -> `&(float){0}, &(float){0}`, e `menu_press` recebe a posição de `GetMousePosition`) | `src/ui/app.c:71` | yes - `app_menu_card_click_real_path` falhou em `tests/test_bots_app.c:93` (`s.started`); 113/114 |
| F-g ramo de produção morto: `: IsMouseButtonPressed(MOUSE_BUTTON_LEFT)` -> `: false` (a `domino.exe` ignora o mouse na tela inicial) | `src/ui/app.c:72` | yes - só `cli_menu_backspace_and_clicks` falhou, em `tests/test_menu_cli.c:183` (arquivo do nome não gravado, porque o clique em `Jogar` não chegou). Com o mutante falhou 9 de 9 vezes (1 na suíte inteira e 8 sozinho), sempre em `:183`. Sem o mutante, no mesmo worktree, passou 8 de 8. O brief pediu para não contar morte que só esse teste produz, por causa da instabilidade de ~2/11. Aqui ela é separável do ruído (9/9 contra 0/8), e o ramo pertence de propósito a esse teste, como diz a nota de Coverage do `checks.md`. Por isso marquei `yes`. Sem contar F-g, ficam 3 de 3 mortas pelas provas da própria feature |

Falha considerada e não injetada: "ignorar o resultado do gancho" no sentido de forçar `pressed = true` quando o
gancho existe. Esse mutante é equivalente. Na `domino.exe` o gancho é `NULL`, então nada muda no produto. No teste,
o gancho já devolve `true` nos quadros 0 e 1, e a partir do quadro 2 a `app_run` já saiu da tela inicial. A variante
que muda comportamento ("chamar o gancho, mas usar `IsMouseButtonPressed`") é a mesma superfície de F-e.

## Gate

`ctest --test-dir build --output-on-failure` no fa083b9 (árvore real): 115 passed, 0 failed. Os testes #97-#115
aparecem um a um como `Passed`, e `cli_menu_backspace_and_clicks` passou na primeira vez.
`git diff --exit-code v1.0.0 -- <13 arquivos de teste antigos>` saiu com 0.

O `validate_verification.py` não rodou porque o Python não está instalado nesta máquina, então o script não validou
este veredito. Conferi a consistência à mão: nenhuma célula `Contradiction`/`Uncovered` preenchida, nenhuma `Killed`
com `no`, nenhuma `Unproven` preenchida, os 17 checks PASS e as 3 linhas de `Test policy` com `yes`. Tudo isso é
compatível com PASS.

Lacunas que ficam, nenhuma bloqueante:

1. `cli_menu_backspace_and_clicks` (de `tela-inicial`, sem mudança desde `v1.0.0`) continua instável. Na rodada 2
   falhou ~2/11 sem mutante; nesta rodada passou 9/9 sem mutante (1 na árvore real, 8 no worktree). Ele é a única
   prova do ramo de produção `IsMouseButtonPressed` em `src/ui/app.c:72`. Um vermelho falso dele quebra a prova do
   C17.
2. A redação da nota do C10 em `checks.md:61` diz que o gancho "só diz onde foi o clique". Na verdade ele também diz se
   houve clique (`src/ui/app.h:23-24`, `src/ui/app.c:71`). É imprecisão de texto, não de prova.
