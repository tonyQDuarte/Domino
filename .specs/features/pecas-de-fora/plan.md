# Peças de fora (6 por jogador)

## Problem

Hoje as 28 peças vão todas para as mãos, 7 para cada jogador (AC 2 de `partida-duplas`). Com tudo
distribuído, quem conta as peças sabe exatamente o que cada um ainda tem, e o fim de cada mão fica
previsível. O usuário quer que 4 peças fiquem de fora, sem ninguém saber quais, e que elas sejam
mostradas no fim da mão.

A fonte é o pedido do usuário no chat, depois de testar o jogo. Não há números.

Quando esta feature ficar pronta, cada jogador recebe 6 peças, 4 ficam de costas num canto da mesa,
e essas 4 viram junto com as mãos das CPUs no fim da mão.

## Flow

Reaproveita a distribuição de `start_hand`, a regra de abertura de `abertura-carroca`, a linha do
tempo de animação de `cpu-parceria-animacao` (distribuição e virada) e o desenho das fileiras de
costas da `domino_ui`.

1. `start_hand` (exists) - embaralha as 28 peças, dá 6 a cada assento e guarda as 4 restantes em `Game.sleeping`
2. `start_hand` (exists) - primeira mão da partida: a vez vai para quem tem a maior carroça distribuída, e essa carroça vira a única abertura aceita; nas outras mãos, regra de `abertura-carroca`
3. `cpu_view_of` (exists) - continua sem as peças de fora, então a CPU não as enxerga
4. controlador (exists) - a distribuição animada manda 24 peças para as mãos e as 4 de fora para o canto; a virada do fim da mão vira também as 4 de fora
5. saída: `ctl_view` diz quantas peças de fora já aparecem e se estão de face; `draw_frame` (exists) desenha a fileira no canto superior direito

## Impact

| Front | What changes |
| --- | --- |
| domain | novo termo: `sleeping` (peças de fora, as que "dormem") - 4 peças por mão, vive em `Game` |
| domain | `HAND_SIZE` passa de 7 para 6. Quem depende: `Hand`, a distribuição, os layouts das mãos e a distribuição animada |
| behaviour | **substitui o AC 2 de `partida-duplas`** (7 por jogador, sem sobra) pelos AC 1 e 2 daqui |
| behaviour | **substitui o AC 9 de `partida-duplas`** (abre quem tem `[6\|6]`, só com ela) pelo AC 6 daqui (abre quem tem a maior carroça distribuída, só com ela). Quando o `[6\|6]` está numa mão, o resultado é o mesmo de antes |
| behaviour | **substitui os AC 12 e 13 de `cpu-parceria-animacao`** (as 28 peças voam para as mãos) pelo AC 4 daqui (24 vão para as mãos e 4 para o canto, no mesmo tempo total de 1650 ms) |
| behaviour | `abertura-carroca` dizia que as 7 carroças estão sempre nas mãos. Agora pelo menos 3 estão, e a regra de abertura continua sempre achando alguém com carroça |
| tests | testes antigos que afirmam a regra substituída e são reescritos para a regra nova, com nota de substituição no `checks.md` de origem: `deal_28_unique_7_each` (C3), `first_hand_double_six_starts` e `first_hand_rejects_non_double_six` (C10), `hand_end_overlay` (C28: a mão nova tem 6 peças), `new_match_resets` (C33: abre a maior carroça) em `partida-duplas`; `deal_animation_timing` (C14), `deal_shows_arrived_only` (C15) e `deal_blocks_play` (C16: procura a carroça de abertura) em `cpu-parceria-animacao`. Durante o build apareceu mais um: `deal_same_seed_same_hands` (C6 de `partida-duplas`) percorria a mão com um `7` fixo e passou a ler fora do vetor; o limite virou `HAND_SIZE`, sem mudar o que o teste afirma O C4 de `abertura-carroca` usa como prova os testes do C10 e segue a mesma nota |
| stored data | nada a migrar - continua sem nada gravado |

## Relations

None - nenhum dado é gravado.

## Surface

None - a linha de comando e os códigos de saída não mudam.

## Landing

None - o tamanho da mão e as peças de fora vivem em `rules.c`, e a fileira do canto vive no layout e
no desenho; tudo se desfaz trocando código, sem dado gravado, contrato ou dependência nova.

## Criteria

### S1: Cada jogador recebe 6 e 4 ficam de fora (P1)

A mão começa com 24 peças nas mãos e 4 escondidas.

**Acceptance Criteria**

1. WHEN uma mão começa THEN the system SHALL dar 6 peças a cada um dos 4 assentos e deixar 4 de fora, e as 28 peças, somando mãos e peças de fora, SHALL ser exatamente o duplo-seis, sem repetição
2. WHILE uma mão está em andamento the system SHALL mostrar as 4 peças de fora de costas no canto superior direito da janela, fora da área da mesa e sem encostar nas fileiras das mãos
3. The system SHALL decidir a jogada da CPU sem as peças de fora: trocar as peças de fora com peças das mãos dos outros três assentos SHALL não mudar a jogada escolhida
4. WHEN uma mão começa com as animações ligadas THEN the system SHALL mandar as peças uma a cada 50 ms, cada uma voando 300 ms: as 24 primeiras para as mãos, na ordem Sul, Leste, Norte, Oeste repetida 6 vezes, e as 4 últimas para o canto das peças de fora, terminando aos 1650 ms
5. WHILE a distribuição acontece, the system SHALL mostrar no canto só as peças de fora que já chegaram

**Independent test:** abrir o jogo e ver 6 peças em cada mão e 4 de costas no canto.

### S2: A primeira mão abre com a maior carroça distribuída (P1)

O `[6|6]` pode ter ficado de fora, então a abertura passa a ser a maior carroça que está nas mãos.

**Acceptance Criteria**

6. WHEN começa a primeira mão de uma partida THEN the system SHALL dar a vez a quem tem a maior carroça distribuída (`[6|6]`, senão `[5|5]`, e assim por diante) e aceitar somente essa carroça como primeira jogada
7. IF o Sul clica em outra peça para abrir a primeira mão THEN the system SHALL deixar a mesa vazia e mostrar `Jogada inválida`

**Independent test:** com uma seed em que o `[6|6]` fica de fora, ver quem tem o `[5|5]` (ou a maior carroça distribuída) abrir a partida.

### S3: As peças de fora aparecem no fim (P1)

**Acceptance Criteria**

8. WHEN uma mão termina com as animações ligadas THEN the system SHALL virar as 4 peças de fora junto com as mãos das CPUs: de costas até os 200 ms da virada e de face a partir daí, com a mesma escala horizontal
9. WHEN uma mão termina com as animações desligadas THEN the system SHALL mostrar as 4 peças de fora de face na hora
10. WHILE o quadro de fim de mão ou de fim de partida está na tela, the system SHALL manter as 4 peças de fora de face

**Independent test:** bater e ver as 4 peças do canto virarem junto com as das CPUs.

### S4: O resto continua igual (P1)

**Acceptance Criteria**

11. The system SHALL manter passando, sem alteração, todos os testes das features anteriores que não estão listados como substituídos em `Impact`

**Independent test:** `ctest` inteiro verde.

## Out of scope

| Excluded | Why |
| --- | --- |
| Comprar das peças de fora | O usuário pediu peças fora do jogo, não um monte de compra |
| Contar as peças de fora no tranque | Não estão na mão de ninguém, então ficam fora da soma, como manda a regra do tranque |
| CPU que deduz quais peças ficaram de fora | É um avanço de IA à parte; a CPU atual só usa os passes |
| Redistribuir quando o `[6\|6]` fica de fora | O usuário escolheu a maior carroça distribuída |

## Assumptions

| Assumption | Chosen default | Rationale | Confirmed? |
| --- | --- | --- | --- |
| Lugar exato da fileira de fora | 4 peças de 30x60 em pé, lado a lado, no canto superior direito (à direita da fileira do Norte, acima da coluna do Leste) | É o canto livre da janela e tem o mesmo tamanho das peças de costas das CPUs | n |
| Ordem das peças de fora | a ordem em que saíram do embaralhamento | Não importa para o jogo e não dá pista nenhuma | n |
| Texto junto da fileira | nenhum | Não foi pedido; as peças de costas no canto já se explicam | n |

**Open questions:** none - all resolved or logged above.

## Observable

| Surface | Decision | Landing |
| --- | --- | --- |
| screen `mesa` | empty state | AC 5 - o canto começa vazio e enche durante a distribuição |
| screen `mesa` | loading state | n/a - nada é carregado |
| screen `mesa` | error state | AC 7 - `Jogada inválida` ao abrir a primeira mão com outra peça |
| screen `mesa` | unauthorised state | n/a - jogo local de um só usuário |
| screen `mesa` | density and ordering | AC 2 - lugar da fileira; ordem em Assumptions |
| screen `mesa` | destructive action confirms | n/a - nenhuma ação destrutiva nova |
| screen `fim de mão` | structure | AC 8, AC 10 |
| command `domino.exe` | flags, exit codes, output | n/a - linha de comando inalterada |

## Sources

- Pedido do usuário no chat (2026-09-28): 6 peças por jogador, 4 de fora, e mostrar as de fora ao revelar no fim
- Respostas do usuário no chat (2026-09-28): sem o `[6|6]`, abre a maior carroça distribuída; as 4 ficam de costas num canto e viram no fim
