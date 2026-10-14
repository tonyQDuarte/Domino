# Bots parceiros verification

**Verdict**: FAIL
**Profile**: standard
**Diff range**: 8419202..cd81394
**Round**: 2 - scoped
**Verifier**: independent sub-agent (author != verifier)

Escopo da rodada 2: o fix é `fe964d1..cd81394` (o `fe964d1` só acrescenta o relatório da rodada 1). Ele toca
`src/core/menu.c` (nova `menu_press`, `:93-97`), `src/core/menu.h` (declaração, `:41-42`), `src/ui/app.c` (o clique
de mouse da tela inicial agora chama `menu_press`, `:72`), `tests/test_bots.c` (`menu_card_click_selects` e
`bots_seating_by_seed` ganharam afirmações), `tests/test_bots_app.c` (o gancho passa a chamar `menu_press`, `:25`)
e `checks.md` (linhas `>` "Reforço da rodada 1" no C10 e no C12, e a nota de Coverage reescrita). A rodada 1
falhou por F1 e F2 sobreviventes, pelas linhas 1 e 2 de `Test policy`, por dois membros de cobertura sem prova
(clique real no cartão pela `app_run`; mãos seguintes com o RNG separado) e por uma nota de Coverage errada.

Resultado: F2 agora morre, e a linha 1 de `Test policy` e o membro "mãos seguintes" passam. A lacuna F1 continua
aberta com outra cara: trocar `menu_press` por `menu_click` em `src/ui/app.c:72` (o clique real deixa de escolher
o cartão) sobrevive. O clique real em `Jogar` não distingue as duas funções, porque as duas devolvem `MENU_PLAY`
ali. A cadeia que o `checks.md` afirma não fecha.

## Binding sources

carried from 9241605

| Source | Opened | Contradiction | Uncovered |
| --- | --- | --- | --- |
| nenhuma fonte binding além das respostas do usuário no chat listadas em `plan.md` `## Sources` (não há arquivo de design). O passo 1 só roda no perfil `ui` e aqui não tem entrada | - | - | - |

## Checks

verified at cd81394. As provas rodaram em uma invocação, `ctest --test-dir build --output-on-failure -V`, com
`100% tests passed out of 114`. Cada teste aparece individualmente como `Test #NN ... Passed`. Os 17 de
bots-parceiros são #98-#114, e `cli_menu_backspace_and_clicks` é o #97. O build da árvore real estava em dia com o
HEAD (`ninja: no work to do`). O harness rejeita nome desconhecido: `./build/test_bots nonexistent_name` imprime
`teste desconhecido` e sai com 2. A prova do C17,
`git diff --exit-code v1.0.0 -- tests/test_core.c ... tests/core_has_no_raylib.cmake`, saiu com 0 (`v1.0.0^{commit}` =
8419202). `git diff --stat v1.0.0 HEAD -- tests` lista só `test_bots.c` e `test_bots_app.c`. As citações de
`tests/test_bots.c` a partir da linha 266 e de `tests/test_bots_app.c` foram renovadas. As linhas 1-265 de
`test_bots.c` não mudaram, e as citações de C1-C9 são as da rodada 1, conferidas no arquivo atual.

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
| C10 | clique no centro do cartão i seleciona e devolve `MENU_NONE`; ordem 5,0,3; fora não muda (+ reforço: `menu_press` idem, e `MENU_PLAY` em `Jogar`) | `menu_card_click_selects` #107 exit 0 | `tests/test_bots.c:261-265` - `menu_click(...) == MENU_NONE`, `menu_pick_partner(...)`, `m.partner == i`; `:269-270` - `menu_press(&p, cx(r), cy(r)) == MENU_NONE`, `p.partner == i` (F-b morto aqui); `:275-276` - `m.partner == order[k]`; `:279-286` - fora (título e (20,700)) com `menu_pick_partner`, `menu_click` e `menu_press`, `m.partner == 3`; `:288-289` - `menu_press(&m, cx(play), cy(play)) == MENU_PLAY`, `m.partner == 3` (F-d morto aqui). O claim do check, que é de nível de menu, está provado. A última oração do reforço (`checks.md:60`, "o clique real em `Jogar` ... prova que a `app_run` chama `menu_press`") é falsa, porque F-c sobrevive. Veja Coverage e Faults | PASS |
| C11 | `Jogar`/`Sair` nos retângulos de `tela-inicial`; 12 retângulos na janela, de cima para baixo, sem sobreposição | `menu_partner_layout` #108 exit 0 | `tests/test_bots.c:300-301` - `rect_eq(menu_play_rect(), 540, 360, 200, 55)`, `rect_eq(menu_quit_rect(), 540, 440, 200, 55)`; `:314` - dentro da janela; `:316` - `!rect_overlaps(r[i], r[j])`; `:320` - ordem vertical; `:323-327` - faixa única, abaixo do rótulo, acima de `Jogar`, da esquerda para a direita; `:329` - `Sair` abaixo de `Jogar` | PASS |
| C12 | `ctl_set_bots` p=0..5 x seeds 0..99: Norte=p, E/O distintos e != p, `styles`, mesma seed = mesmos adversários, 1ª **e** 2ª mão iguais às sem bots, cada um dos 5 aparece; `app_run` com Seu Paulo | `bots_seating_by_seed` #109, `app_menu_partner_starts_game` #114 exit 0 | `tests/test_bots.c:341` - `bots_ok(&a, p)`; `:344` - mesmos E/O com a mesma seed; `:346-347` - 1ª mão, `memcmp(a.game.hands, plain.game.hands, ...) == 0` e `sleeping`; `:351-352` - mesmas afirmações depois de `game_next_hand` nos dois (F2 morto em `:351`); `:356` - `seen[i] == (i != p)`; `tests/test_bots_app.c:58` - `strcmp(BOTS[a.bot[SEAT_NORTH]].name, "Seu Paulo") == 0`; `:61` - `e != w && e != 0 && w != 0`; `:62-63` - estilos; `:64` - 2ª execução, mesmos adversários. A seleção na `app_run` continua sendo feita pelo gancho (`tests/test_bots_app.c:25`, agora com `menu_press`), que pula `src/ui/app.c:64-76` | PASS |
| C13 | Nova partida mantém o Norte, E/O válidos e `styles`; >= 2 pares em 20 cliques | `new_match_keeps_partner_redraws` #110 exit 0 | `tests/test_bots.c:372` - `c.game.phase == PHASE_PLAYING`; `:373` - `bots_ok(&c, 1)`; `:384` - `distinct >= 2` | PASS |
| C14 | `seat_name` N/L/O e Sul vazio; rótulos na janela, sem tocar fileiras, mesa, placar, fora, mensagem, outros rótulos; Norte `y+h <= 40`, x 640; L/O acima da coluna de 6 | `bot_names_on_table` #111 exit 0 | `tests/test_bots.c:397` - `v.seat_name[SEAT_SOUTH][0] == '\0'`; `:400` - `strcmp(v.seat_name[cpus[k]], BOTS[c.bot[cpus[k]]].name) == 0`; `:411-419` - janela, `layout_table_area()`, `score`, `layout_message()`, `sleeping[i]`, outros rótulos; `:428` - fileiras; `:434-436` - `n.y + n.h <= 40`, `<= row[0].y`, `cx(n) == 640`; `:441-442` - `r.y + r.h <= row[0].y`, `cx(r) == cx(row[0])` | PASS |
| C15 | Seu Leo no Leste: `Seu Leo passou` aos 800 ms, visível +1499, some aos 1500 | `bot_pass_message` #112 exit 0 | `tests/test_bots.c:473` - `seed < 1000` (achou a seed com Seu Leo no Leste); `:479-480` - `v.message_visible`, `strcmp(v.message, "Seu Leo passou") == 0`; `:483-484` - ainda visível; `:487` - `!v.message_visible` | PASS |
| C16 | sem `ctl_set_bots`: `Leste passou`, `Parceiro passou`, `Oeste passou`; `seat_name` vazio nos 4 | `no_bots_keeps_messages` #113 exit 0 | `tests/test_bots.c:502-503` - `v.message_visible`, `strcmp(v.message, texts[k]) == 0`; `:505` - `v.seat_name[s][0] == '\0'` | PASS |
| C17 | suíte inteira passa; nenhum arquivo de teste antigo mudou desde `v1.0.0` | `ctest --test-dir build --output-on-failure` 114/114; `git diff --exit-code v1.0.0 -- <13 arquivos>` exit 0 | `100% tests passed out of 114`; diff com saída 0 | PASS |

## Coverage

As linhas que o fix tocou são verified at cd81394. As outras são carried from 9241605, porque o fix não acrescentou
nenhum ramo de decisão: `menu_press` só compõe `menu_pick_partner` e `menu_click`, e `controller.c`/`cpu.c` não
mudaram (`git diff fe964d1..HEAD -- src/core/controller.c src/core/cpu.c` está vazio).

| Set (size) | Recomputed from | Member -> proof | Unproven |
| --- | --- | --- | --- |
| bots (6) - carried from 9241605 | AC 1, `src/core/bots.c:3-10` | os 6 C1 | - |
| estilos x modo (6) - carried from 9241605 | tabela "Os seis estilos", `cpu.c:100-106` | Seu Paulo C2 · Neca C3 · Audi C4 · Seu Leo C5 · Guede C6 · Manezinho C7 | - |
| quantidade do parceiro vs própria (3 bordas) - carried from 9241605 | AC 2-5 | menos C2-C7 · igual C2-C5 · mais C2-C4 | - |
| regras da cadeia (4) - carried from 9241605 | tabela "Os seis estilos" | P C2-C5 · A C5, C6 · J C4, C6 · D C7 | - |
| ordem P/A no modo parceiro (2) - carried from 9241605 | AC 5 | P antes C4 · A antes C5 | - |
| estilos sem trapaça (6) - carried from 9241605 | AC 8 | os 6 C8 | - |
| elementos da tela inicial (12) - carried from 9241605 | AC 9, AC 11 | os 12 C11; texto do rótulo C9 | - |
| cliques na tela inicial, nível de decisão (`menu_press`: cartão, fora, `Jogar`) (3) - verified at cd81394 | AC 10, `src/core/menu.c:74-97` | cartão `tests/test_bots.c:269-270` · fora `:285-286` · `Jogar` `:288` (F-b e F-d mortos) | - |
| cliques na tela inicial, mapeamento real na `app_run` (3) - verified at cd81394 | Flow 1, `src/ui/app.c:70-76` | `Jogar` `cli_menu_backspace_and_clicks` (`tests/test_menu_cli.c:176`) · `Sair` mesmo teste (`:188-189`) · cartão -> `menu_press(&menu, m.x, m.y)` (`app.c:72`): **nenhuma prova**. O gancho de `tests/test_bots_app.c:25` chama `menu_press` por conta própria e pula `app.c:64-76`; o teste de processo nunca clica num cartão. Os cliques em `Jogar` e `Sair` são respondidos do mesmo jeito por `menu_click`, e F-c (trocar `menu_press` por `menu_click` em `app.c:72`) sobreviveu. A cadeia do `checks.md` ("`menu_press` provada no núcleo + clique real em `Jogar` prova que a `app_run` chama `menu_press`") não se sustenta: o clique em `Jogar` não distingue `menu_press` de `menu_click` | cartão (`app.c:72`) |
| assentos com bot (3) - carried from 9241605 | AC 12, AC 14 | Norte C12, C13, C14 · Leste C12-C15 · Oeste C12, C13, C14 | - |
| começos de partida com bots (2) - carried from 9241605 | AC 12, AC 13 | `Jogar` C12 · `Nova partida` C13 | - |
| mãos que a seed dá, com e sem bots (2) - verified at cd81394 | Assumption "Sorteio dos adversários", Flow 3 | 1ª mão C12 `tests/test_bots.c:346-347` · mãos seguintes C12 `:351-352` (depois de `game_next_hand`; F2 morto em `:351`). Como o estado do RNG da distribuição é contínuo, a 2ª mão igual prova que `ctl_set_bots` não o consome | - |
| mensagens de passe (2 modos x 3 assentos) - carried from 9241605 | AC 15, AC 16 | com bots Leste C15 (Norte e Oeste pelo mesmo ramo) · sem bots L/N/O C16 | - |
| Landing door 1 (1) - carried from 9241605 | Landing | C4 `:127` · C8 `:216` · C17 | - |
| startup config: bots (2 assemblies) - carried from 9241605 | lido direto | `src/ui/main.c:23` -> `app.c:86` `ctl_set_bots(&c, menu.partner, opt->seed);` C12 (#114) · `ctl_set_bots` direto C12, C13, C15 | - |

Nota de Coverage do `checks.md` (verified at cd81394): o texto novo (`checks.md:108-111`) ainda afirma uma prova que
não existe. Ele diz que a `menu_press` está "ligada à `domino.exe` pelo clique real em `Jogar`", e F-c mostra que
essa ligação não é observada por nenhum teste. O reforço do C10 (`checks.md:60`) repete a mesma afirmação.

## Test policy rows

Linhas 1 e 2 re-julgadas at cd81394 (eram as não atendidas e classificam arquivos tocados). Linha 3 carried from
9241605.

| Row | Files it classifies | Required proof | Expectation met |
| --- | --- | --- | --- |
| Decide, sem fronteira | `src/core/cpu.c`; `src/core/controller.c` (`ctl_set_bots`, Nova partida, mensagem de passe) | um no próprio nível | yes - `cpu.c` tem a tabela inteira (C2-C8). Em `controller.c`, o "RNG separado" agora é afirmado na 1ª e na 2ª mão (`tests/test_bots.c:346-352`), e F2 morre em `:351` |
| Decide, alcançado pela `app_run` | `src/core/menu.c` (cartões, `menu_press`); `src/ui/app.c` | próprio nível **e** `app_run` | no - o próprio nível está completo (C9-C11; `menu_press` em `tests/test_bots.c:269-289`, F-b e F-d mortos). Na `app_run`, o "começar" está provado (`tests/test_bots_app.c:58-64`), mas o "escolher" ainda é feito pelo gancho do teste (`tests/test_bots_app.c:25`), e o desvio do clique real para `menu_press` (`src/ui/app.c:72`) continua sem prova: F-c sobreviveu |
| Instrumentação (desenho dos cartões e dos nomes) | `src/ui/draw.c` | nenhum próprio | yes (carried from 9241605) |

Swept: carried from 9241605. Nenhuma linha cita uma restrição como já existente.

## Faults injected

verified at cd81394. Isolamento: worktree em `scratchpad/verify-wt2` a partir do HEAD cd81394, com build próprio em
`bld`. O baseline no worktree deu 114/114. O `git status --porcelain` da árvore real estava vazio antes e depois
(`cmp` sem diferença). O worktree foi removido com `git worktree remove --force`, `git worktree list` mostra só a
árvore real, e `git stash` não foi usado (`git stash list` vazio). Cada falha foi revertida com `git checkout -- src`
no worktree antes da seguinte, e a suíte inteira rodou com cada uma. Foram 4 mutações.

| Mutation | Location | Killed |
| --- | --- | --- |
| F-a (F2 de novo) `bot_next(&c->bot_rng)` -> `bot_next(&c->game.rng)` nas duas chamadas de `draw_opponents` | `src/core/controller.c:58`, `:61` | yes - `bots_seating_by_seed` falhou em `tests/test_bots.c:351` (`memcmp(a.game.hands, plain.game.hands, ...) == 0`, 2ª mão); 113/114 |
| F-b apagar `menu_pick_partner(m, x, y);` dentro de `menu_press` | `src/core/menu.c:95` | yes - `menu_card_click_selects` falhou em `tests/test_bots.c:270` (`p.partner == i`) e `app_menu_partner_starts_game` em `tests/test_bots_app.c:58` (Seu Paulo no Norte); 112/114 |
| F-c `menu_press(&menu, m.x, m.y)` -> `menu_click(&menu, m.x, m.y)` no clique de mouse da tela inicial (o clique real deixa de escolher o cartão) | `src/ui/app.c:72` | no - nenhuma afirmação distingue. Na passada da suíte inteira, `cli_menu_backspace_and_clicks` falhou uma vez em `tests/test_menu_cli.c:183`. Rodando esse teste 12 vezes com o mutante, ele falhou 2 vezes (`:189` e `:183`) e passou 10. O teste sem mutante, rodado 11 vezes, falhou 2 (`:183` e `:189`). As falhas são instabilidade de tempo e não uma morte: em (640,387) e (640,467) não há cartão (os retângulos não se sobrepõem, C11), então `menu_press` e `menu_click` devolvem a mesma ação ali. Na `domino.exe`, clicar num cartão não troca mais o parceiro |
| F-d `menu_press` devolve sempre `MENU_NONE` (perde a ação do clique) | `src/core/menu.c:96` | yes - `menu_card_click_selects` falhou em `tests/test_bots.c:288` (`menu_press(&m, cx(play), cy(play)) == MENU_PLAY`). `cli_menu_backspace_and_clicks` também falhou em `tests/test_menu_cli.c:183`, mas esse teste é instável e não conta como prova |

F-d mostra o que o clique real em `Jogar` prova de fato: a `app_run` chama uma função que devolve `MENU_PLAY` no
botão. Isso vale tanto para `menu_press` quanto para `menu_click`, e por isso F-c sobrevive. Fechar a lacuna exige um
clique real num cartão seguido de uma afirmação do parceiro escolhido (por exemplo, no processo, clicar no centro de
`menu_card_rect(0)` e depois em `Jogar`, com uma saída observável do bot do Norte), ou um gancho que injete a posição
do mouse e passe por `app.c:70-75`.

## Gate

`ctest --test-dir build --output-on-failure -V` no cd81394 (árvore real): 114 passed, 0 failed (#98-#114 e #97
aparecem um a um como `Passed`). `git diff --exit-code v1.0.0 -- <13 arquivos de teste antigos>` saiu com 0. O
`validate_verification.py` não rodou porque o Python não está instalado nesta máquina, e o script não validou este
veredito. A consistência foi conferida à mão: uma célula `Killed` com `no` (F-c), uma célula `Unproven` preenchida
(cartão em `app.c:72`) e uma linha de `Test policy` com `no` (linha 2), todas compatíveis com FAIL. Os 17 checks
estão PASS.

Lacunas, da mais grave para a menos grave:

1. A escolha do parceiro por clique real na `domino.exe` continua sem prova. F-c sobreviveu: trocar `menu_press` por
   `menu_click` em `src/ui/app.c:72` não derruba nenhum teste de forma confiável. Isso afeta o AC 10/12 na fronteira
   da `app_run` e a linha 2 de `Test policy`. O gancho de `tests/test_bots_app.c:25` faz a seleção por conta própria,
   e `tests/test_menu_cli.c:176`/`:188` só clicam em botões, onde as duas funções se comportam igual.
2. O `checks.md` afirma uma ligação que não está provada. O reforço do C10 (`checks.md:60`) e a nota de Coverage
   (`checks.md:108-111`) dizem que o clique real em `Jogar` prova que a `app_run` chama `menu_press`. F-c refuta isso.
3. Não bloqueia: `cli_menu_backspace_and_clicks` (de `tela-inicial`, sem mudança desde `v1.0.0`) é instável. No
   worktree, sem mutante, falhou 2 de 11 vezes, em `tests/test_menu_cli.c:183` (arquivo diferente de `Ana`) e `:189`
   (`finish(&pi, 5000)`). Na árvore real passou. Um teste instável na prova do C17 e das linhas de mapeamento real pode
   dar falso vermelho, e também falso "morto" numa injeção de falha.
