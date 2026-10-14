# Bots com nome e estilo de jogo

## Problem

Hoje as três CPUs jogam do mesmo jeito (a regra de `cpu-parceria-animacao`) e não têm nome: a mesa é
sempre "Parceiro", "Leste" e "Oeste". O jogador não escolhe com quem joga, e toda partida tem a mesma
cara. O usuário quer seis bots com nome e estilo próprio, uns mais colaborativos com o parceiro e
outros mais egoístas, para escolher o parceiro de jogo.

A fonte é o pedido do usuário no chat. Os nomes são dele: Audi, Manezinho, Seu Paulo, Seu Leo,
Guede e Neca. Os estilos ficaram a meu critério. Não há números.

Quando esta feature ficar pronta, a tela inicial mostra os seis bots com o estilo de cada um, o
jogador escolhe o parceiro, os dois adversários são sorteados entre os outros cinco, e a mesa mostra
o nome de cada bot.

## Flow

Reaproveita a cadeia de regras de `cpu_decide` (`cpu-parceria-animacao`), a tela inicial e o arquivo
do nome (`tela-inicial`) e o controlador. Nenhum bot ganha uma IA própria: cada estilo é uma forma de
ligar, desligar ou reordenar as regras que já existem.

1. tela inicial -> `menu.c` (exists) - além do nome, guarda qual dos 6 bots está selecionado; o clique num cartão troca a seleção
2. Jogar -> `app_run` (exists) - chama `ctl_set_bots` com o parceiro escolhido e a seed
3. `ctl_set_bots` (novo, `controller.c`) - põe o parceiro no Norte e sorteia dois adversários diferentes entre os outros 5, para Leste e Oeste, com um RNG separado do da distribuição
4. `Game.styles` (door 1) - guarda o estilo de cada assento; `cpu_view_of` (exists) copia o estilo do assento para o `CpuView`
5. vez de uma CPU -> `cpu_decide` (exists) - aplica as regras na ordem e com o modo que o estilo manda
6. saída: `ctl_view` traz o nome de cada assento; `draw_frame` desenha os nomes junto das fileiras, e as mensagens de passe usam o nome do bot

## Impact

| Front | What changes |
| --- | --- |
| domain | novo termo: `Bot` - nome + estilo, um dos 6 da tabela fixa em `bots.c` |
| domain | novo termo: `CpuStyle` - o estilo de jogo; o valor 0 é o estilo da Audi, que é exatamente a regra atual |
| domain | termo existente: `CpuView` ganha o campo `style` (door 1). Quem depende: `cpu_decide` e os testes que montam `CpuView` à mão, que zeram a estrutura e por isso continuam com o estilo atual |
| behaviour | sem bots configurados (padrão do controlador), a CPU joga exatamente como hoje e as mensagens continuam `Leste passou`, `Parceiro passou` e `Oeste passou`. Com bots, as mensagens usam o nome |
| behaviour | a tela inicial ganha a escolha do parceiro. O título, o rótulo e o campo do nome sobem; `Jogar` e `Sair` ficam exatamente onde estão hoje |
| tests | nenhum teste antigo muda. `menu_layout_and_texts` (C1 de `tela-inicial`) só exige a ordem de cima para baixo, que continua valendo; `cli_menu_backspace_and_clicks` clica em coordenadas fixas de `Jogar` e `Sair`, que não se movem |
| stored data | nada novo: o nome continua em `jogador.txt`; a escolha do parceiro não é gravada nesta feature |

## Relations

None - nada novo é gravado; a tabela de bots é fixa no código.

## Surface

None - a linha de comando, os códigos de saída e o arquivo do nome não mudam.

## Landing

| One-way door | Literal shape | Alternative rejected |
| --- | --- | --- |
| 1. Estilo dentro do `CpuView` (acrescenta à door 1 de `cpu-parceria-animacao`) | `typedef enum { STYLE_AUDI = 0, STYLE_SEU_PAULO, STYLE_NECA, STYLE_SEU_LEO, STYLE_GUEDE, STYLE_MANEZINHO } CpuStyle;`, campo `CpuStyle style` no fim do `CpuView`, e `CpuStyle styles[SEAT_COUNT]` no fim do `Game`. O zero é o estilo atual | Passar o estilo como argumento novo de `cpu_decide`: muda a assinatura que o C2 de `cpu-parceria-animacao` fixa (`bool (*)(const CpuView *, int *, End *)`) e obrigaria a reescrever testes cuja regra não mudou |

- Nothing else in this change is hard to reverse

## Os seis estilos

Do mais colaborativo para o mais egoísta. As regras são as de `cpu-parceria-animacao`: **P** = não
deixar as duas pontas em números em que o parceiro já passou; **A** = deixar as duas pontas em números
em que o adversário seguinte já passou; **J** = ficar com mais peças próprias que encaixam; **D** =
desempate (maior soma, carroça, primeira da mão, ponta esquerda).

| Bot | Estilo (texto do cartão) | Quando joga pelo parceiro | Ordem das regras |
| --- | --- | --- | --- |
| Seu Paulo | Fidelidade total | sempre | P, A, D |
| Neca | Colaboração | parceiro com tantas peças quanto ele ou menos | parceiro: P, A, D · próprio: A, J, D |
| Audi | Equilíbrio | parceiro com menos peças (a regra atual) | parceiro: P, A, D · próprio: A, J, D |
| Seu Leo | Bloqueio | parceiro com menos peças | parceiro: A, P, D · próprio: A, J, D |
| Guede | Jogo próprio | nunca | A, J, D |
| Manezinho | Descarrego | nunca | D |

## Criteria

### S1: Seis bots com estilos diferentes (P1)

Cada bot decide com o seu estilo, sempre só com o que é visível.

**Acceptance Criteria**

1. The system SHALL ter 6 bots, nesta ordem: Seu Paulo (`Fidelidade total`), Neca (`Colaboração`), Audi (`Equilíbrio`), Seu Leo (`Bloqueio`), Guede (`Jogo próprio`) e Manezinho (`Descarrego`)
2. WHILE a CPU tem o estilo de Seu Paulo, the system SHALL decidir em modo parceiro (P, A, D) qualquer que seja a quantidade de peças dela e do parceiro
3. WHILE a CPU tem o estilo de Neca, the system SHALL decidir em modo parceiro quando o parceiro tem tantas peças quanto ela ou menos, e em modo próprio (A, J, D) no resto
4. WHILE a CPU tem o estilo de Audi, the system SHALL decidir exatamente como a CPU de `cpu-parceria-animacao` (modo parceiro só quando o parceiro tem menos peças)
5. WHILE a CPU tem o estilo de Seu Leo, the system SHALL aplicar a regra A antes da regra P no modo parceiro, e decidir o modo como a Audi
6. WHILE a CPU tem o estilo de Guede, the system SHALL decidir sempre em modo próprio (A, J, D), sem olhar os passes do parceiro
7. WHILE a CPU tem o estilo de Manezinho, the system SHALL escolher só pelo desempate D, sem olhar passes nem a própria mão
8. The system SHALL, em qualquer estilo, decidir só pelo `CpuView`: trocar as peças das mãos dos outros assentos e as peças de fora SHALL não mudar a jogada escolhida

**Independent test:** pôr o mesmo cenário na frente dos 6 estilos e ver as jogadas diferentes que a tabela manda.

### S2: O jogador escolhe o parceiro na tela inicial (P1)

**Acceptance Criteria**

9. WHEN a tela inicial abre THEN the system SHALL mostrar o rótulo `Seu parceiro` e 6 cartões com o nome e o estilo de cada bot, na ordem do AC 1, com Audi selecionada
10. WHEN o jogador clica num cartão THEN the system SHALL selecionar esse bot e tirar a seleção do anterior
11. The system SHALL manter `Jogar` e `Sair` nos mesmos retângulos de `tela-inicial`, e o título, o rótulo do nome, o campo, o rótulo do parceiro, os cartões, `Jogar` e `Sair` SHALL ficar de cima para baixo sem se sobrepor
12. WHEN a partida começa pelo `Jogar` THEN the system SHALL pôr o bot escolhido no Norte e dois bots diferentes entre si, sorteados entre os outros 5, no Leste e no Oeste; a mesma seed SHALL dar os mesmos adversários
13. WHEN o jogador clica em `Nova partida` THEN the system SHALL manter o parceiro e sortear de novo os adversários

**Independent test:** escolher Seu Paulo, jogar, e ver "Seu Paulo" no Norte e dois outros nomes nos lados.

### S3: Os nomes aparecem na mesa (P2)

**Acceptance Criteria**

14. WHILE uma partida com bots está na tela, the system SHALL mostrar o nome de cada bot junto da fileira dele (Norte, Leste e Oeste), sem encostar em outras fileiras, no placar nem na área da mesa
15. WHEN um bot passa THEN the system SHALL mostrar `<nome> passou` por 1500 ms
16. WHERE o controlador não tem bots (padrão), the system SHALL manter as mensagens `Leste passou`, `Parceiro passou` e `Oeste passou` e não mostrar rótulos de nome

**Independent test:** jogar até um bot passar e ver o nome dele na mensagem.

### S4: O resto continua igual (P1)

**Acceptance Criteria**

17. The system SHALL manter passando, sem alteração, todos os testes das features anteriores

**Independent test:** `ctest` inteiro verde, com os arquivos de teste antigos iguais aos de `v1.0.0`.

## Out of scope

| Excluded | Why |
| --- | --- |
| Escolher os adversários | O usuário escolheu escolher só o parceiro |
| Gravar o parceiro escolhido entre uma abertura e outra | Não foi pedido; volta para Audi a cada abertura. Pode usar `%APPDATA%\Domino\` (AD-007) numa próxima feature |
| Trocar `Você`, `Nós`, `Eles` e `Vocês venceram!` por nomes | Não foi pedido, e mudaria textos que os testes provam |
| Bots que contam as peças que já saíram | É uma IA nova; aqui os estilos só combinam as regras que já existem |
| Fotos ou avatares dos bots | Não foi pedido |

## Assumptions

| Assumption | Chosen default | Rationale | Confirmed? |
| --- | --- | --- | --- |
| Estilo de cada bot | a tabela "Os seis estilos" acima | O usuário deixou a meu critério; cobre do mais colaborativo ao mais egoísta e cada estilo muda pelo menos uma jogada em relação ao vizinho | n |
| Bot selecionado ao abrir | Audi | É o estilo atual, então quem não mexe joga como antes | n |
| Sorteio dos adversários | RNG próprio, semeado pela seed da partida, separado do RNG da distribuição | Os adversários mudam sem alterar as mãos que a seed já dá | n |
| Lugar dos nomes | Norte: abaixo da fileira; Leste e Oeste: acima da coluna | São os espaços livres perto de cada fileira | n |
| Texto do estilo nos cartões | substantivo neutro (`Colaboração`, não `Colaborativa`) | Não atribui gênero a nenhum bot | n |

**Open questions:** none - all resolved or logged above.

## Observable

| Surface | Decision | Landing |
| --- | --- | --- |
| screen `tela inicial` | empty state | AC 9 - Audi já vem selecionada; não existe "nenhum parceiro" |
| screen `tela inicial` | loading state | n/a - a tabela de bots é fixa no código |
| screen `tela inicial` | error state | n/a - não há entrada inválida para o cartão; o nome segue as regras de `tela-inicial` |
| screen `tela inicial` | unauthorised state | n/a - jogo local de um só usuário |
| screen `tela inicial` | density and ordering | AC 1, AC 9, AC 11 |
| screen `tela inicial` | destructive action confirms | n/a - trocar o parceiro não perde nada |
| screen `mesa` | nomes e mensagens | AC 14, AC 15, AC 16 |
| command `domino.exe` | flags, exit codes | n/a - linha de comando inalterada |

## Sources

- Pedido do usuário no chat (2026-10-13): bots com estilos diferentes para escolher como parceiro, com os nomes Audi, Manezinho, Seu Paulo, Seu Leo, Guede e Neca; estilos à escolha; branch separada, merge na `main` só depois de verificado
- Resposta do usuário no chat (2026-10-13): escolhe só o parceiro; adversários sorteados entre os outros 5
