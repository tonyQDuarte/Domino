# CPU em parceria e animação das peças

## Problem

Hoje cada CPU joga sozinha: sempre a peça válida de maior soma, sem olhar para o parceiro nem para os
adversários (AC 18 da feature `partida-duplas`). A CPU do Norte tranca o humano tão facilmente quanto
tranca um adversário, e uma CPU com 6 peças não abre caminho para o parceiro que está com 1. Quem
joga com a CPU de parceira sente que joga sozinho contra três.

Além disso, toda jogada aparece de uma vez na mesa. Quando duas CPUs jogam seguidas, fica difícil
saber de que mão saiu cada peça e o que mudou.

A fonte é o pedido do usuário no chat: a CPU deve jogar em parceria, e a prioridade da dupla é
sempre de quem está com menos peças. O usuário também pediu as animações das peças. Não há números
nem prazo.

Quando esta feature ficar pronta, a CPU parceira passa a abrir caminho para quem da dupla está mais
perto de bater, inclusive o humano. Cada peça jogada desliza da mão até a mesa, a mão começa com a
distribuição animada, as peças das CPUs viram no fim da mão e os destaques pulsam.

## Flow

Reaproveita o `domino_core` e a `domino_ui` da feature `partida-duplas`. A escolha da CPU continua
saindo de `cpu_choose` e o desenho continua saindo de `draw_frame`. Nada de novo na tela ou na janela.

1. `domino_core` `game_pass` (exists) - além de passar a vez, registra que o assento não tem os valores das duas pontas
2. vez de uma CPU -> controlador do `domino_core` (exists) - chama `cpu_choose`, que monta um `CpuView` (door 1) só com o que é visível e decide por ele
3. `domino_core` `game_play` (exists) - aplica a jogada; o controlador abre o deslize da peça na linha do tempo de animação (door 2)
4. `domino_ui` `app_run` (exists) - liga as animações com `ctl_set_animations`, repassa `dt` e cliques como antes
5. controlador - avança a linha do tempo (distribuição, deslize, virada, pulso); enquanto há animação que bloqueia, não conta o tempo da CPU nem aceita clique nas peças do Sul
6. saída: `ctl_view` entrega as peças em voo com o retângulo do momento, a escala da virada, a opacidade do pulso e se o placar do fim da mão já aparece; `draw_frame` (exists) desenha

## Impact

| Front | What changes |
| --- | --- |
| domain | novo termo: `CpuView` - o que uma CPU pode ver (própria mão, pontas, quantidades, passes), vive em `domino_core` |
| domain | novo termo: `lacks` - valores em que cada assento já passou na mão atual, vive em `Game` |
| domain | termo existente: modo da CPU. Antes havia um só (maior soma). Agora são dois: `modo parceiro` (o parceiro tem menos peças) e `modo próprio`. Quem depende disso hoje: só `controller.c`, por meio de `cpu_choose` |
| behaviour | o AC 18 de `partida-duplas` (maior soma, carroça, primeira, esquerda) deixa de ser a regra inteira e vira o último desempate (AC 8 aqui). Os casos do teste `cpu_picks_heaviest` continuam válidos, porque neles o parceiro tem 7 peças e ninguém passou |
| behaviour | `View` ganha campos de animação. Os campos atuais mantêm o significado, então os 40 testes de `partida-duplas` não mudam |
| stored data | nada a migrar - continua sem nada gravado |

## Relations

None - nenhum dado é gravado.

## Surface

None - `domino.exe` mantém a mesma linha de comando e os mesmos códigos de saída.

## Landing

| One-way door | Literal shape | Alternative rejected |
| --- | --- | --- |
| 1. O que a CPU enxerga | `typedef struct { Hand own; Seat seat; int counts[SEAT_COUNT]; bool lacks[SEAT_COUNT][7]; bool empty; uint8_t left, right; bool first_hand; } CpuView;` montado por `cpu_view_of(const Game*, Seat)`. A decisão `cpu_decide(const CpuView*, int*, End*)` não recebe `Game` | `cpu_choose` lendo `Game` direto: nada impediria uma regra futura de olhar a mão do parceiro, e o usuário pediu que a CPU visse só o visível |
| 2. Animação na linha do tempo do controlador | a linha do tempo fica em `domino_core` e avança só pelo `dt` de `ctl_update`. Liga com `ctl_set_animations(c, true)` e vem desligada no `ctl_init`. `View` traz `Flying flying[TILE_COUNT]` (retângulo interpolado + valores), `hand_shown[SEAT_COUNT]`, `board_hidden`, `flip_scale`, `overlay_visible`, `pulse` | Animar em `domino_ui` com `GetTime()` da raylib: impossível de testar sem janela, e quebraria o AD-003. Animação sempre ligada: mudaria o comportamento que os 40 testes de `partida-duplas` provam |

- Nothing else in this change is hard to reverse

## Criteria

### S1: A CPU joga pela dupla (P1)

A CPU ajuda quem da dupla tem menos peças e tenta fazer o adversário seguinte passar, sem saber as peças dos outros.

**Acceptance Criteria**

1. The system SHALL decidir a jogada da CPU só a partir da própria mão, das pontas da mesa, da quantidade de peças de cada assento e dos valores em que cada assento já passou; trocar as peças das outras três mãos, mantendo as quantidades, SHALL não mudar a jogada escolhida
2. WHEN um assento passa com as pontas `a` e `b` na mesa THEN the system SHALL registrar que esse assento não tem `a` nem `b` até o fim da mão
3. WHEN uma mão começa THEN the system SHALL apagar todos os registros de passe da mão anterior
4. WHILE o parceiro da CPU tem menos peças que ela (modo parceiro), the system SHALL descartar as jogadas que deixam as duas pontas em valores em que o parceiro já passou, desde que exista outra jogada válida
5. WHEN a CPU escolhe entre as jogadas que restam (depois do AC 4 no modo parceiro, ou de saída no modo próprio) THEN the system SHALL preferir as que deixam as duas pontas em valores em que o próximo adversário (o assento seguinte) já passou
6. WHILE a CPU tem tantas peças quanto o parceiro ou menos (modo próprio), the system SHALL preferir, entre as que restam depois do AC 5, a jogada depois da qual mais peças da própria mão encaixam nas pontas
7. The system SHALL tratar o Sul (humano) como parceiro do Norte e o Leste como parceiro do Oeste nas regras dos AC 4 a 6
8. WHEN as regras dos AC 4 a 6 empatam THEN the system SHALL desempatar como em `partida-duplas`: maior soma, depois carroça, depois a primeira na ordem da mão, e ponta esquerda quando a peça encaixa nas duas

**Independent test:** numa mão em que o Sul tem 2 peças, ver o Norte evitar deixar as pontas em números em que o Sul já passou.

### S2: A peça desliza até a mesa (P1)

Cada peça jogada sai da mão de quem jogou e chega ao seu lugar na linha.

**Acceptance Criteria**

9. WHEN um assento joga uma peça com as animações ligadas THEN the system SHALL deslizar a peça em 350 ms: aos 0 ms o centro dela está no centro da peça de origem na mão, aos 350 ms está no centro do lugar dela na linha, e entre os dois fica no meio do caminho
10. WHILE a peça desliza, the system SHALL não desenhar a peça no lugar final da linha
11. WHILE uma peça desliza, the system SHALL não contar os 800 ms da CPU seguinte e SHALL ignorar cliques nas peças do Sul

**Independent test:** jogar uma peça e ver a peça sair da mão e chegar à linha antes de o Leste começar a jogar.

### S3: A distribuição é animada (P2)

No começo de cada mão, as peças saem do centro da mesa para as mãos.

**Acceptance Criteria**

12. WHEN uma mão começa com as animações ligadas THEN the system SHALL mandar as 28 peças do centro da mesa para o lugar delas nas mãos, uma a cada 50 ms, cada uma voando 300 ms, na ordem Sul, Leste, Norte, Oeste repetida 7 vezes, terminando aos 1650 ms
13. WHILE a distribuição acontece, the system SHALL mostrar em cada mão só as peças que já chegaram
14. WHILE a distribuição acontece, the system SHALL ignorar cliques nas peças do Sul, não contar o tempo da CPU e não passar a vez do Sul automaticamente

**Independent test:** apertar `Nova partida` ou começar uma mão nova e ver as peças voarem para as quatro mãos.

### S4: As mãos viram no fim (P2)

No fim da mão, as peças das CPUs viram antes de o placar aparecer.

**Acceptance Criteria**

15. WHEN uma mão termina com as animações ligadas THEN the system SHALL, depois que a última peça termina de deslizar, virar as peças das três CPUs em 400 ms: verso até os 200 ms, face dos 200 ms aos 400 ms, com a escala horizontal indo de 1 a 0 e voltando a 1
16. WHILE as peças viram, the system SHALL não mostrar o quadro de resultado do fim da mão nem o do fim da partida
17. IF o jogador clica ou aperta Enter antes de a virada terminar, no fim de uma mão THEN the system SHALL encerrar as animações na hora e mostrar o quadro de resultado, sem começar a próxima mão

**Independent test:** bater e ver as peças das CPUs virarem antes do quadro de resultado.

### S5: Os destaques pulsam (P3)

As pontas destacadas e a peça selecionada pulsam.

**Acceptance Criteria**

18. WHILE há pontas destacadas para a escolha de ponta, the system SHALL variar a opacidade do destaque das pontas e da borda da peça selecionada entre 0,35 e 0,85, com período de 800 ms

**Independent test:** clicar numa peça que encaixa nas duas pontas e ver as pontas pulsarem.

### S6: O que já existe continua igual (P1)

**Acceptance Criteria**

19. WHERE as animações estão desligadas (padrão do controlador), the system SHALL se comportar como em `partida-duplas`, e os 40 testes daquela feature SHALL passar sem nenhuma alteração
20. WHEN `domino.exe` abre a janela THEN the system SHALL ligar as animações

**Independent test:** `ctest` com os 40 testes antigos verdes, e o jogo animado ao abrir.

## Out of scope

| Excluded | Why |
| --- | --- |
| CPU que vê a mão do parceiro | O usuário escolheu que a CPU só vê o visível |
| Contar as peças que já saíram para deduzir mãos (além dos passes) | Fica para uma próxima rodada da IA. Os passes já dão a informação mais forte |
| Níveis de dificuldade | Não foi pedido |
| Sons das animações | Não foi pedido |
| Botão ou opção para desligar as animações no jogo | Não foi pedido. Desligar fica só no controlador, para os testes |
| Indicador de "sua vez" | Não foi pedido nesta feature. Continua como melhoria futura |

## Assumptions

| Assumption | Chosen default | Rationale | Confirmed? |
| --- | --- | --- | --- |
| Empate de peças entre CPU e parceiro | Modo próprio: a CPU joga para si | "Quem tem menos peças" não decide o empate, e jogar para si é o comportamento mais seguro | n |
| Ordem das regras no modo parceiro | não trancar o parceiro (AC 4) > fazer o adversário passar (AC 5) > desempate | A prioridade pedida é o parceiro, então a regra dele vem primeiro | n |
| Ordem das regras no modo próprio | fazer o adversário passar (AC 5) > manter jogo na própria mão (AC 6) > desempate | Fazer o adversário passar também adianta a vez do parceiro | n |
| Tempos | deslize 350 ms, distribuição 50 ms entre peças e 300 ms de voo, virada 400 ms, pulso de 800 ms | Rápidos o bastante para não atrasar o jogo e lentos o bastante para dar para acompanhar | n |
| Curva do movimento | ease-out cúbica no deslize e na distribuição | Começa rápido e chega suave, que é o costume em jogos de cartas | n |
| Clique durante a distribuição | é ignorado, não pula a animação | São 1,65 s, e pular deixaria a mão aparecer pela metade | n |
| Botão `Nova partida` antes de terminar a virada | já funciona se clicado na área dele | É o comportamento atual (AC 32 de `partida-duplas`), e o AC 19 exige mantê-lo | n |
| Peça em voo | desenhada com a face para cima e com a orientação do lugar de destino | A peça vai ficar visível na mesa de qualquer jeito | n |

**Open questions:** none - all resolved or logged above.

## Observable

| Surface | Decision | Landing |
| --- | --- | --- |
| screen `mesa` | empty state | AC 13 - durante a distribuição as mãos começam vazias e enchem |
| screen `mesa` | loading state | n/a - nada é carregado; a distribuição é uma animação, não uma espera |
| screen `mesa` | error state | n/a - a feature não cria nenhum caminho de erro novo |
| screen `mesa` | unauthorised state | n/a - jogo local de um só usuário |
| screen `mesa` | density and ordering | AC 12 - ordem da distribuição; o resto continua como em `partida-duplas` |
| screen `mesa` | destructive action confirms | n/a - nenhuma ação destrutiva nova |
| screen `fim de mão` | structure and next action | AC 16, AC 17 |
| command `domino.exe` | flags, exit codes, output | n/a - linha de comando inalterada (AC 20 só liga as animações) |

## Sources

- Pedido do usuário no chat (2026-09-27): CPU que joga em parceria com prioridade para quem tem menos peças, e animação das peças
- Respostas do usuário no chat (2026-09-27): a CPU só vê o visível; as quatro animações (deslize, distribuição, virada, pulso)
