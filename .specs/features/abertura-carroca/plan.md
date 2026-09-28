# Abertura com carroça

## Problem

Hoje, da segunda mão em diante, quem abre pode jogar qualquer peça (AC 28 e AC 29 de
`partida-duplas`). Na mesa do usuário a regra é outra: toda mão abre com uma carroça. Quem tem o
direito de abrir e não tem carroça passa esse direito adiante, e a vez só vai para a outra dupla
quando nenhum dos dois da dupla vencedora tem carroça. Do jeito atual, o jogo não segue a regra
de quem está jogando.

A fonte é o pedido do usuário no chat, depois de testar o jogo. Não há números.

Quando esta feature ficar pronta, toda mão abre com uma carroça, e a vez de abrir respeita a dupla
que ganhou a mão anterior.

## Flow

Reaproveita a regra de abertura que já existe em `domino_core` (o `[6|6]` da primeira mão) e a
amplia: quem pode abrir e com que peça continuam sendo decididos em `rules.c`, e a CPU continua
enxergando a regra só pelo `CpuView`.

1. fim da mão -> `game_play` / `game_pass` (exists) - guardam quem bateu ou quem abriu a mão trancada e qual dupla ganhou
2. `game_next_hand` (exists) - distribui e escolhe quem abre: o primeiro, na ordem da regra, que tem carroça
3. mesa vazia -> `game_fits` (exists) - na primeira mão aceita só `[6|6]`; nas outras, só carroças
4. vez de uma CPU com a mesa vazia -> `cpu_decide` (exists) - considera só as carroças como jogadas válidas
5. saída: o Sul vê `Jogada inválida` se clicar numa peça que não é carroça (caminho atual do `ctl_click`)

## Impact

| Front | What changes |
| --- | --- |
| behaviour | **AC 28 e AC 29 de `partida-duplas` são substituídos** pelos AC 3 a 5 daqui. A mão seguinte deixa de abrir com "qualquer peça", e quem bateu, ou quem abriu a mão trancada, só abre se tiver carroça |
| tests | os testes `next_hand_winner_starts` (C29) e `next_hand_after_tranque` (C30) de `partida-duplas` afirmam "aceita qualquer peça" e "abre quem bateu/abriu" a partir de uma distribuição aleatória. Eles passam a montar mãos fixas e a aceitar só carroça. É uma mudança em `tests/test_core.c`, feita porque a regra mudou, e fica registrada como nota em `partida-duplas/checks.md` (C29, C30) |
| tests | a segunda prova do C21 de `cpu-parceria-animacao` (`tests/test_core.c` igual a `39873e8`) deixa de valer pelo mesmo motivo. Ganha uma nota de substituição, e os outros arquivos de teste antigos continuam intocados |
| domain | termo existente: `next_opener` deixa de ser "quem abre" e vira "por onde começa a procura por uma carroça". Quem depende disso: só `start_hand` em `rules.c` |
| stored data | nada a migrar - continua sem nada gravado |

## Relations

None - nenhum dado é gravado.

## Surface

None - a linha de comando e os códigos de saída não mudam.

## Landing

None - a regra vive em `rules.c` e `cpu.c` e se desfaz trocando código; não cria contrato, dado
gravado, dependência nem padrão novo.

## Criteria

### S1: Toda mão abre com carroça (P1)

A primeira peça de qualquer mão é uma carroça.

**Acceptance Criteria**

1. WHEN começa uma mão que não é a primeira da partida THEN the system SHALL aceitar como primeira jogada somente uma carroça (`[0|0]` a `[6|6]`), em qualquer das que o jogador da vez tiver
2. IF o Sul clica numa peça que não é carroça para abrir uma mão que não é a primeira THEN the system SHALL deixar a mesa vazia e mostrar `Jogada inválida`
3. WHEN uma CPU abre uma mão que não é a primeira THEN the system SHALL jogar uma carroça
4. The system SHALL manter a primeira mão da partida abrindo com `[6|6]` por quem o tem (AC 9 de `partida-duplas`)

**Independent test:** ganhar uma mão e ver a mão seguinte abrir com uma carroça.

### S2: A vez de abrir respeita a dupla vencedora (P1)

Quem tem o direito de abrir e não tem carroça passa a vez de abrir adiante.

**Acceptance Criteria**

5. WHEN a mão anterior terminou em batida THEN the system SHALL dar a abertura ao primeiro, nesta ordem, que tiver ao menos uma carroça: quem bateu, o parceiro de quem bateu, o jogador seguinte a quem bateu, o parceiro desse jogador
6. WHEN a mão anterior terminou trancada e uma dupla ganhou o ponto THEN the system SHALL usar a mesma ordem do AC 5, começando pelo jogador da dupla vencedora que abriu a mão trancada ou, se quem abriu é da outra dupla, pelo jogador seguinte a quem abriu
7. WHEN a mão anterior terminou trancada sem ninguém pontuar THEN the system SHALL usar a mesma ordem do AC 5, começando por quem abriu a mão trancada

**Independent test:** forçar uma mão em que quem bateu não tem carroça e ver o parceiro abrir.

## Out of scope

| Excluded | Why |
| --- | --- |
| Obrigar a abrir com a maior carroça | O usuário pediu só que a abertura seja uma carroça |
| Redistribuir quando ninguém tem carroça | Não acontece: as 7 carroças estão sempre nas mãos, então alguém sempre tem |
| Mudar a primeira mão da partida | Continua com `[6|6]`, que já é uma carroça |

## Assumptions

| Assumption | Chosen default | Rationale | Confirmed? |
| --- | --- | --- | --- |
| Qual carroça a CPU usa para abrir | a que as regras de `cpu-parceria-animacao` escolherem entre as carroças (no fim, a de maior soma) | Não foi pedida uma carroça específica, e reaproveita a decisão que já existe | n |
| Qual carroça o Sul usa para abrir | qualquer uma das que tiver, à escolha dele | O usuário pediu só que fosse carroça | n |
| Aviso de quem abre | nenhum texto novo; a vez aparece como hoje | Não foi pedido; o indicador de vez continua como melhoria futura | n |

**Open questions:** none - all resolved or logged above.

## Observable

| Surface | Decision | Landing |
| --- | --- | --- |
| screen `mesa` | empty state | AC 1 - a mesa vazia agora só aceita carroça na abertura |
| screen `mesa` | loading state | n/a - nada é carregado |
| screen `mesa` | error state | AC 2 - `Jogada inválida` ao abrir com peça que não é carroça |
| screen `mesa` | unauthorised state | n/a - jogo local de um só usuário |
| screen `mesa` | density and ordering | n/a - a tela não muda |
| screen `mesa` | destructive action confirms | n/a - nenhuma ação destrutiva nova |
| command `domino.exe` | flags, exit codes, output | n/a - linha de comando inalterada |

## Sources

- Pedido do usuário no chat (2026-09-28): a pedra inicial deve ser carroça; se ninguém da dupla vencedora tiver, a outra dupla começa
- Respostas do usuário no chat (2026-09-28): quem bateu, senão o parceiro, senão a outra dupla; depois de tranque, a dupla que ganhou o ponto
