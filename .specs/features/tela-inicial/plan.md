# Tela inicial com nome do jogador

## Problem

Hoje `domino.exe` abre direto numa partida já distribuída: não há como se identificar, nem como sair
sem fechar a janela, e o jogo nunca sabe quem está jogando. O usuário pediu uma tela inicial com um
menu onde o jogador digita o próprio nome.

A fonte é o pedido do usuário no chat. Não há números.

Quando esta feature ficar pronta, o jogo abre numa tela inicial com o campo do nome (já preenchido
com o nome da última vez), "Jogar" e "Sair", e durante a partida o nome aparece junto da mão do Sul.

## Flow

Reaproveita a `app_run`, o controlador e o desenho atuais. A partida só começa depois da tela inicial,
e o menu segue o mesmo padrão das animações (AD-006): fica desligado por padrão e a `domino.exe` liga.

1. `main` (exists) - liga o menu nas `AppOptions` (door 2) e passa o caminho do arquivo do nome
2. `app_run` (exists) - lê o nome salvo (door 1) e abre a tela inicial em vez da partida
3. teclado e mouse -> `app_run` - repassam caracteres, Backspace, Enter e cliques para o menu do `domino_core`, que filtra o texto e decide entre Jogar e Sair
4. Jogar -> `app_run` - grava o nome (door 1), chama `ctl_init` e liga as animações como hoje; o nome vai para a `View`
5. Sair -> `app_run` - sai do laço e retorna 0
6. saída: `draw_frame` (exists) desenha a partida com o nome acima da mão do Sul; a tela inicial tem um desenho próprio em `domino_ui`

## Impact

| Front | What changes |
| --- | --- |
| domain | novo termo: `Menu` - o estado da tela inicial (texto do nome e a ação escolhida), vive em `domino_core` sem raylib (AD-003) |
| domain | novo termo: nome do jogador - texto de 1 a 16 caracteres, guardado no `Controller` e mostrado na `View` |
| behaviour | `domino.exe` deixa de abrir direto na partida. A distribuição começa só depois do "Jogar" |
| behaviour | os textos atuais (`Você passou`, `Nós 0 × 0 Eles`, `Vocês venceram!`) continuam iguais; o nome aparece só como rótulo da mão do Sul |
| tests | nenhum teste antigo muda: `app_run` chamado sem `show_menu` (como fazem `window_table_color`, `window_fail_exits_1` e `app_animations_on`) continua abrindo direto na partida. Os testes de processo (`cli_*`) já rodam o `domino.exe` real e só olham título, tamanho, saída e argumentos, que não mudam |
| stored data | **novo arquivo** `%APPDATA%\Domino\jogador.txt`. Não existe hoje em nenhuma máquina, então não há nada a migrar |

## Relations

None - um único valor de texto gravado em arquivo, sem entidades nem relações. O formato do arquivo é
a door 1.

## Surface

| Route | In | Out | Status |
| --- | --- | --- | --- |
| arquivo `%APPDATA%\Domino\jogador.txt` | lido ao abrir a tela inicial | gravado ao clicar em Jogar | `lido` (nome válido), `ignorado` (ausente, vazio ou inválido), `gravado`, `falha ao gravar` (a partida começa mesmo assim) |

## Landing

| One-way door | Literal shape | Alternative rejected |
| --- | --- | --- |
| 1. Onde e como o nome fica gravado | `%APPDATA%\Domino\jogador.txt`, texto UTF-8 sem BOM, uma linha só com o nome (1 a 16 caracteres, sem espaços nas pontas), sem quebra de linha no fim. A pasta `Domino` é criada se não existir | Registro do Windows: só funciona com a API do Windows e não dá para testar lendo um arquivo. Arquivo ao lado do `domino.exe`: a pasta de instalação pode não permitir escrita |
| 2. Menu ligado só pela `domino.exe` | campo `bool show_menu` nas `AppOptions`, falso por padrão; `main` liga, junto com `const char *name_path` | Menu sempre ligado em `app_run`: os testes que olham o primeiro quadro (`window_table_color`, `app_animations_on`) passariam a ver o menu, e teriam de ser reescritos sem que a regra deles tenha mudado |

- Nothing else in this change is hard to reverse

## Criteria

### S1: A tela inicial recebe o nome e começa a partida (P1)

**Acceptance Criteria**

1. WHEN `domino.exe` abre THEN the system SHALL mostrar, sobre a mesa verde e antes de qualquer distribuição, o título `Dominó`, o rótulo `Seu nome`, o campo de texto e os botões `Jogar` e `Sair`
2. WHEN o jogador digita um caractere de código 32 a 126 ou 160 a 255 THEN the system SHALL acrescentá-lo ao fim do nome, até 16 caracteres; IF o caractere tem outro código ou o nome já tem 16 THEN the system SHALL ignorá-lo
3. WHEN o jogador aperta Backspace THEN the system SHALL apagar o último caractere do nome, inteiro, mesmo que seja acentuado
4. WHILE o nome está vazio ou só tem espaços, the system SHALL desenhar `Jogar` desabilitado, e o clique nele ou o Enter SHALL não começar a partida
5. WHEN o jogador clica em `Jogar` ou aperta Enter com um nome válido THEN the system SHALL começar a partida, com a distribuição animada, usando o nome sem os espaços das pontas
6. WHEN o jogador clica em `Sair` THEN the system SHALL fechar a janela e terminar com código 0
7. WHILE a partida está na tela, the system SHALL mostrar o nome do jogador logo acima da mão do Sul

**Independent test:** abrir o jogo, digitar um nome, clicar em Jogar e ver o nome acima da mão.

### S2: O nome é lembrado (P1)

**Acceptance Criteria**

8. WHEN a partida começa pelo `Jogar` THEN the system SHALL gravar o nome em `%APPDATA%\Domino\jogador.txt` no formato da door 1, criando a pasta se ela não existir
9. WHEN a tela inicial abre e o arquivo tem um nome válido THEN the system SHALL mostrar o campo já preenchido com esse nome
10. IF o arquivo não existe, está vazio ou tem caracteres fora dos códigos aceitos pelo AC 2 THEN the system SHALL abrir com o campo vazio, sem mostrar erro; IF tem mais de 16 caracteres válidos THEN the system SHALL usar os 16 primeiros
11. IF o arquivo não pode ser gravado THEN the system SHALL começar a partida mesmo assim, sem mostrar erro

**Independent test:** jogar uma vez com um nome, fechar, abrir de novo e ver o nome preenchido.

### S3: O resto continua igual (P1)

**Acceptance Criteria**

12. WHERE `show_menu` está desligado (padrão das `AppOptions`), the system SHALL abrir direto na partida como antes, e todos os testes das features anteriores SHALL passar sem alteração

**Independent test:** `ctest` inteiro verde.

## Out of scope

| Excluded | Why |
| --- | --- |
| Tela "Como jogar" e tela de opções | O usuário escolheu só Jogar e Sair |
| Voltar ao menu no meio ou no fim da partida | Não foi pedido; o fim da partida continua com `Nova partida` |
| Trocar `Você`, `Nós` e `Vocês` pelo nome nos textos do jogo | Não foi pedido, e mudaria textos que os testes atuais provam |
| Nomes para as CPUs | Não foi pedido |
| Placar ou histórico salvos | Só o nome é gravado |

## Assumptions

| Assumption | Chosen default | Rationale | Confirmed? |
| --- | --- | --- | --- |
| Limite do nome | 16 caracteres | Cabe acima da mão do Sul sem encostar nas outras fileiras | n |
| Caracteres aceitos | códigos 32–126 e 160–255 (letras, números, pontuação e acentos do português) | É o que a fonte carregada desenha; caractere sem desenho apareceria como um quadrado | n |
| Esc na tela inicial | fecha o jogo com código 0, como hoje durante a partida | É o comportamento padrão da janela e o mesmo do `Sair` | n |
| `--seed` | continua valendo para a partida que começa depois do `Jogar` | Mantém a partida reproduzível | n |
| Cursor do campo | barra piscando a cada 500 ms no fim do texto | Mostra onde o texto entra; o nome só cresce ou diminui pelo fim | n |

**Open questions:** none - all resolved or logged above.

## Observable

| Surface | Decision | Landing |
| --- | --- | --- |
| screen `tela inicial` | empty state | AC 4 - campo vazio deixa `Jogar` desabilitado; AC 10 - arquivo ausente abre vazio |
| screen `tela inicial` | loading state | n/a - o nome é lido de um arquivo pequeno antes do primeiro quadro |
| screen `tela inicial` | error state | AC 10, AC 11 - arquivo ruim ou que não grava não mostra erro e não impede o jogo |
| screen `tela inicial` | unauthorised state | n/a - jogo local de um só usuário |
| screen `tela inicial` | density and ordering | AC 1 - título, rótulo, campo, Jogar, Sair, de cima para baixo |
| screen `tela inicial` | destructive action confirms | n/a - `Sair` só fecha, não há nada em andamento para perder |
| screen `mesa` | nome do jogador | AC 7 |
| command `domino.exe` | flags, exit codes | AC 6 - `Sair` termina com 0; o resto da linha de comando não muda |
| file `jogador.txt` | formato, leitura, gravação, falhas | door 1, AC 8 a AC 11 |

## Sources

- Pedido do usuário no chat (2026-10-13): tela inicial com menu onde o jogador possa inserir o nome
- Respostas do usuário no chat (2026-10-13): menu só com Jogar e Sair; lembrar o nome em arquivo
