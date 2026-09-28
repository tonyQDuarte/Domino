# Partida de dominó em duplas

## Problem

Hoje quem quer jogar uma partida de dominó brasileiro em duplas no computador precisa de mais três
pessoas na mesa. Não existe neste diretório nada que jogue, pontue ou desenhe uma partida, e a pasta
`D:\Dev\Git\Domino` está vazia.

O pedido não traz números nem prazo. A única fonte é a conversa: um jogo em C, com mesa verde e
4 jogadores, sendo 3 CPUs e 1 humano. Os requisitos vão sendo alinhados ao longo do projeto.

Quando esta feature ficar pronta, o jogador abre `domino.exe` e joga uma partida completa
em dupla com a CPU da frente contra as duas CPUs dos lados, do primeiro `[6|6]` até uma dupla
chegar a 6 pontos.

## Flow

Projeto novo: não há nada para reaproveitar. A única reutilização é a raylib, que cuida de janela,
entrada e desenho, sem código próprio de plataforma.

1. linha de comando -> `main` (door 3) - lê `--seed` e chama `app_run` da `domino_ui` (biblioteca `domino_app`, também usada pelo teste da janela), que abre a janela pela raylib (door 1)
2. `domino_core` (door 3) - embaralha e distribui as 28 peças e define quem começa (`[6|6]` ou quem bateu)
3. clique do mouse e tempo do quadro -> `domino_ui` (door 3) - repassa posição do clique e `dt` ao controlador do `domino_core`, que acerta a peça/ponta pelo layout e decide a jogada
4. `domino_core` - valida e aplica a jogada (ou o passe), avança a vez no sentido anti-horário
5. vez de uma CPU -> o controlador do `domino_core` conta 800 ms pelos `dt` recebidos, escolhe e aplica a jogada
6. `domino_core` - detecta batida ou tranque, classifica o resultado, soma os pontos e detecta fim de partida
7. saída: `domino_ui` desenha o quadro (mesa, mãos, placar, mensagens). Nada fica gravado.

## Impact

| Front | What changes |
| --- | --- |
| domain | novo termo: `Tile` (peça) - par de valores 0..6, 28 no conjunto duplo-seis, vive em `domino_core` |
| domain | novo termo: `Board` (mesa/linha) - sequência de peças com duas pontas abertas, vive em `domino_core` |
| domain | novo termo: `Team` (dupla) - assentos Sul+Norte (`Nós`) contra Leste+Oeste (`Eles`), vive em `domino_core` |
| domain | novo termo: `HandResult` - `SIMPLES`, `CARROCA`, `LA_E_LO`, `CRUZADA`, `TRANCADO`, vive em `domino_core` |
| stored data | nada a migrar - não existe dado persistido e esta feature não grava nada |
| toolchain | o `gcc` atual (MinGW.org 6.3, 32 bits) deixa de ser usado neste projeto e entra o MSYS2 UCRT64 (door 4). Os outros projetos que usam `C:\MinGW` continuam como estão |

## Relations

None - nenhum dado é gravado. Partida, mão e mesa só existem em memória durante a execução.

## Surface

| Route | In | Out | Status |
| --- | --- | --- | --- |
| `domino.exe [--seed <n>]` | `--seed` inteiro sem sinal (opcional) | janela do jogo; mensagem de erro no stderr | `0` saída normal, `1` falha ao abrir janela, `2` argumento inválido |

## Landing

| One-way door | Literal shape | Alternative rejected |
| --- | --- | --- |
| 1. Dependência gráfica | raylib do pacote MSYS2 `mingw-w64-ucrt-x86_64-raylib`, encontrada com `find_package(raylib REQUIRED)` | SDL2: precisa de SDL_ttf e SDL_image para texto e imagem (o usuário escolheu raylib). Release zip vendorado: alguém precisa atualizar à mão |
| 2. Build e testes | `CMakeLists.txt` (mínimo 3.21), executáveis de teste em C puro registrados com `add_test`, rodados por `ctest --test-dir build` | Makefile escrito à mão: não se integra a um test runner. Framework Unity/cmocka: uma dependência a mais para asserts simples |
| 3. Separação regras / tela | biblioteca estática `domino_core` (regras, CPU, layout da mesa) **sem nenhum `#include <raylib.h>`**, e o executável `domino` (`main` + `domino_ui`) que linka `domino_core` e `raylib` | Um executável só, misturando regras e desenho: as regras só poderiam ser testadas com uma janela aberta |
| 4. Toolchain | MSYS2 UCRT64 (`winget install MSYS2.MSYS2`, depois `pacman -S mingw-w64-ucrt-x86_64-{gcc,cmake,ninja,raylib}`), gerador `Ninja` | MinGW.org 6.3 já instalado: é 32 bits e a ABI não é compatível com os binários da raylib. w64devkit: daria para usar, mas a raylib teria que ser vendorada |

- Nothing else in this change is hard to reverse

## Criteria

### S1: A mesa abre com as peças distribuídas (P1)

O jogador abre o programa e vê a mesa verde com a própria mão e as mãos fechadas das três CPUs.

**Acceptance Criteria**

1. WHEN o programa inicia THEN the system SHALL abrir uma janela de 1280x720 com título `Dominó` e fundo da mesa na cor RGB(20, 100, 50)
2. WHEN uma mão começa THEN the system SHALL distribuir as 28 peças do duplo-seis, 7 para cada um dos 4 assentos, sem peça repetida e sem sobra
3. WHILE uma mão está em andamento the system SHALL mostrar as peças do assento Sul (humano) viradas para cima, ordenadas pela soma em ordem crescente, na faixa inferior da janela
4. WHILE uma mão está em andamento the system SHALL mostrar as peças de cada CPU viradas para baixo, uma fileira por assento (Norte, Leste, Oeste), na mesma quantidade que a mão daquela CPU tem
5. WHEN o programa recebe `--seed <n>` THEN the system SHALL produzir a mesma distribuição em toda execução com o mesmo `<n>`
6. IF o argumento de `--seed` não é um inteiro sem sinal, ou aparece qualquer outro argumento THEN the system SHALL escrever `uso: domino [--seed <n>]` no stderr e sair com código 2 sem abrir janela
7. IF a janela não puder ser criada THEN the system SHALL escrever `erro: não foi possível abrir a janela` no stderr e sair com código 1
8. WHEN o usuário fecha a janela, em qualquer momento THEN the system SHALL encerrar com código 0

**Independent test:** rodar `domino.exe --seed 42` duas vezes e ver a mesma mão no Sul. Rodar `domino.exe --seed x` e ver o código 2.

### S2: O humano joga as próprias peças (P1)

Na sua vez, o jogador clica numa peça e ela vai para a mesa, ou é recusada com um motivo visível.

**Acceptance Criteria**

9. WHEN começa a primeira mão de uma partida THEN the system SHALL dar a vez ao assento que tem `[6|6]` e aceitar somente `[6|6]` como primeira jogada
10. The system SHALL passar a vez no sentido anti-horário: Sul → Leste → Norte → Oeste → Sul
11. WHILE é a vez do Sul, WHEN o jogador clica numa peça que encaixa em uma só ponta THEN the system SHALL colocá-la nessa ponta, com o valor igual encostado na ponta
12. WHILE é a vez do Sul, WHEN o jogador clica numa peça que encaixa nas duas pontas e as pontas têm valores diferentes THEN the system SHALL destacar as duas pontas e colocar a peça na ponta que o jogador clicar em seguida
13. WHILE é a vez do Sul, WHEN o jogador clica numa peça que encaixa nas duas pontas e as pontas têm o mesmo valor THEN the system SHALL colocá-la na ponta direita sem pedir escolha
14. IF o jogador clica numa peça que não encaixa em nenhuma ponta THEN the system SHALL deixar a mesa e a mão como estão e mostrar `Jogada inválida` por 1,5 s
15. WHILE não é a vez do Sul the system SHALL ignorar cliques nas peças do Sul
16. WHEN chega a vez do Sul e nenhuma peça dele encaixa THEN the system SHALL passar a vez automaticamente e mostrar `Você passou` por 1,5 s

**Independent test:** com uma seed em que o Sul tem `[6|6]`, jogar a primeira peça. Depois clicar numa peça que não encaixa e ver `Jogada inválida`.

### S3: As três CPUs jogam sozinhas (P1)

Depois do humano, as CPUs jogam no ritmo que dá para acompanhar, sem mostrar as peças delas.

**Acceptance Criteria**

17. WHEN chega a vez de uma CPU que tem jogada válida THEN the system SHALL aplicar a jogada 800 ms depois do início da vez
18. WHEN uma CPU escolhe a jogada THEN the system SHALL escolher, entre as peças válidas, a de maior soma; no empate, a carroça; persistindo o empate, a primeira na ordem da mão; e SHALL colocá-la na ponta esquerda quando ela encaixar nas duas
19. WHEN chega a vez de uma CPU sem jogada válida THEN the system SHALL passar a vez e mostrar `<Assento> passou` (`Leste passou`, `Parceiro passou` ou `Oeste passou`) por 1,5 s
20. WHILE uma mão está em andamento the system SHALL não desenhar a face de nenhuma peça das CPUs

**Independent test:** jogar uma peça e ver Leste, Norte e Oeste jogarem ou passarem em sequência, uns 800 ms cada.

### S4: A mão termina, pontua e a partida vai até 6 (P1)

Cada batida ou tranque vira pontos no placar, e a partida acaba quando uma dupla chega a 6.

**Acceptance Criteria**

21. WHEN um jogador bate com uma peça que não é carroça e encaixa em uma só ponta THEN the system SHALL classificar como `SIMPLES` e dar 1 ponto à dupla dele
22. WHEN um jogador bate com uma carroça que encaixa em uma só ponta THEN the system SHALL classificar como `CARROCA` e dar 2 pontos à dupla dele
23. WHEN um jogador bate com uma peça que não é carroça e encaixa nas duas pontas THEN the system SHALL classificar como `LA_E_LO` e dar 3 pontos à dupla dele
24. WHEN um jogador bate com uma carroça que encaixa nas duas pontas THEN the system SHALL classificar como `CRUZADA` e dar 4 pontos à dupla dele
25. WHEN os 4 assentos passam em sequência THEN the system SHALL encerrar a mão como `TRANCADO` e dar 1 ponto à dupla com a menor soma de pontos nas mãos
26. IF a mão tranca e as duas duplas têm a mesma soma THEN the system SHALL encerrar a mão sem dar pontos a ninguém
27. WHEN uma mão termina THEN the system SHALL revelar as mãos das 4 posições e mostrar o tipo do resultado, os pontos ganhos e o placar, até o jogador clicar ou apertar Enter
28. WHEN começa uma mão que não é a primeira da partida THEN the system SHALL dar a vez a quem bateu na mão anterior, que pode abrir com qualquer peça
29. WHEN começa uma mão depois de um tranque THEN the system SHALL dar a vez a quem abriu a mão trancada, que pode abrir com qualquer peça
30. The system SHALL mostrar o placar `Nós <x> × <y> Eles` no canto superior esquerdo durante toda a partida
31. WHEN uma dupla chega a 6 pontos ou mais THEN the system SHALL mostrar `Vocês venceram!` (Sul+Norte) ou `Eles venceram!` (Leste+Oeste), o placar final e o botão `Nova partida`
32. WHEN o jogador clica em `Nova partida` THEN the system SHALL zerar o placar e começar uma primeira mão (regra do `[6|6]`)

**Independent test:** com uma seed fixa, jogar até o fim de uma mão e ver o placar ganhar os pontos que correspondem ao tipo mostrado.

### S5: A linha de peças cabe na mesa (P2)

A linha cresce para os dois lados e faz a curva antes de sair da área da mesa.

**Acceptance Criteria**

33. The system SHALL desenhar a primeira peça da mão no centro da área da mesa e crescer a linha para a esquerda e para a direita a partir dela
34. The system SHALL manter todas as peças da linha dentro da área da mesa (a janela menos as faixas das mãos e do placar) para qualquer linha de 1 a 28 peças, fazendo a curva da linha na borda
35. The system SHALL desenhar as carroças da linha atravessadas, perpendiculares ao sentido da linha naquele trecho

**Independent test:** jogar uma mão até o fim e ver as duas pontas da linha fazendo curva antes de chegar nas faixas das mãos.

## Out of scope

| Excluded | Why |
| --- | --- |
| Compra do monte | O usuário escolheu duplas sem compra. Com 4 jogadores e 7 peças cada não sobra monte |
| Pontuação pela soma da mão / partida até 100 | O usuário escolheu batidas até 6 |
| Salvar e carregar partida | Não foi pedido. A partida é curta |
| Níveis de dificuldade e CPU que joga pela dupla (não trancar o parceiro, contar peças) | A CPU de S3 é a base. A estratégia fica para uma feature própria |
| Som e música | Não foi pedido |
| Animações além do destaque de pontas e das mensagens temporárias | Não foi pedido. Pode entrar depois sem mexer nas regras |
| Janela redimensionável e tela cheia | O layout é fixo em 1280x720 nesta feature |
| Multijogador local ou em rede | O pedido é 1 humano e 3 CPUs |
| Regras regionais (galo, buchada, pontas valendo pontos) | O usuário escolheu a pontuação por tipo de batida |

## Assumptions

| Assumption | Chosen default | Rationale | Confirmed? |
| --- | --- | --- | --- |
| Lá-e-lô quando as duas pontas têm o mesmo valor | Uma peça que não é carroça e encaixa nas duas pontas vale `LA_E_LO`, mesmo se as pontas forem iguais | Uma regra só, sem caso especial. É a leitura mais comum de "serve dos dois lados" | n |
| Nomes no código | Identificadores em inglês (`Tile`, `Board`, `Team`), textos da tela em pt-BR | É o costume em C e as mensagens continuam no idioma do jogador | n |
| Fechar a janela no meio da partida | Fecha sem confirmar e a partida se perde | Não há nada gravado para proteger, e um diálogo de confirmação seria uma tela a mais | n |
| Seed padrão sem `--seed` | `time(NULL)` | Cada partida sai diferente. A flag existe para reproduzir uma partida | n |
| Interface de quem passou | O assento Norte aparece como `Parceiro` nas mensagens; Leste e Oeste pelo próprio nome | O jogador reconhece o parceiro na hora | n |
| Python indisponível | Os validadores `validate_*.py` não rodam. As mesmas verificações são feitas lendo os artefatos (modo degradado) | `python3` é só o alias da Microsoft Store nesta máquina | n |

**Open questions:** none - all resolved or logged above.

## Observable

| Surface | Decision | Landing |
| --- | --- | --- |
| screen `mesa` | empty state | AC 33 - a mesa começa vazia e a primeira peça vai para o centro |
| screen `mesa` | loading state | n/a - nada é carregado de disco, tudo é desenhado por código na abertura |
| screen `mesa` | error state | AC 7 - falha ao abrir a janela; AC 14 - jogada inválida |
| screen `mesa` | unauthorised state | n/a - jogo local de um só usuário, sem conta |
| screen `mesa` | density and ordering | AC 3 - mão do Sul em ordem crescente de soma; AC 4 - fileiras das CPUs |
| screen `mesa` | destructive action confirms | n/a - `Nova partida` só aparece com a partida encerrada; fechar a janela sem confirmar está em Assumptions |
| screen `fim de mão` | structure and next action | AC 27 - revela mãos, tipo, pontos e placar; clique ou Enter continua |
| screen `fim de partida` | structure and next action | AC 31, AC 32 |
| command `domino.exe` | output format and verbosity | n/a - em sucesso não escreve nada; só erros vão para o stderr (AC 6, AC 7) |
| command `domino.exe` | every flag and its default | AC 5, AC 6 - só `--seed`, padrão em Assumptions |
| command `domino.exe` | exit codes | AC 6 (2), AC 7 (1), AC 8 (0) |
| command `domino.exe` | fails halfway | n/a - nada é gravado, então não existe estado parcial para recuperar |

## Sources

- Pedido do usuário no chat (2026-09-27): jogo em C, mesa verde, 4 jogadores (3 CPU + 1 humano)
- Respostas do usuário no chat (2026-09-27): raylib, duplas sem compra, MSYS2 UCRT64, batidas até 6
