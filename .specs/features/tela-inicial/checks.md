# Tela inicial checks

Profile: standard
Plan: `.specs/features/tela-inicial/plan.md`

13 checks in 3 slices · 2 one-way doors · 0 open, of which 0 block

Build e PATH como em `AGENTS.md`. O estado da tela inicial fica em `src/core/menu.c` e a leitura e a
gravação do nome em `src/core/namefile.c`, os dois sem raylib (AD-003). Os testes ficam em
`tests/test_menu.c` (regras, executável `test_menu`) e `tests/test_menu_app.c` (`app_run` com o menu,
executável `test_menu_app`). Para testar a `app_run` sem teclado e mouse de verdade, as `AppOptions`
ganham um gancho de teste, `menu_input`, que diz a ação do quadro; o C13 usa teclas reais enviadas ao
`domino.exe`.

## Checks

### S1 - A tela inicial recebe o nome e começa a partida · 8 files · ~40 KB · ~10k

- [x] **C1** - Os textos da tela inicial são `Dominó`, `Seu nome`, `Jogar` e `Sair`; os retângulos do título, do rótulo, do campo, de `Jogar` e de `Sair` ficam nessa ordem de cima para baixo, sem se sobrepor e dentro da janela; e com `show_menu` ligado nenhuma partida começa nos 3 primeiros quadros sem ação (AC 1)
Proof: `ctest --test-dir build -R "^menu_layout_and_texts$" --output-on-failure`
Proof: `ctest --test-dir build -R "^app_menu_before_game$" --output-on-failure`

- [x] **C2** - `menu_type` aceita os códigos 32, 65, 126, 160, 231 (`ç`) e 255 e recusa 31, 127, 159, 256 e 128512; `ç` é gravado como os bytes `C3 A7`; com 16 caracteres no nome, o 17º é recusado (AC 2)
Proof: `ctest --test-dir build -R "^menu_type_filter$" --output-on-failure`

- [x] **C3** - Backspace em `Zé` deixa `Z` (apaga os 2 bytes do `é`); outro Backspace deixa o nome vazio; Backspace com o nome vazio não muda nada (AC 3)
Proof: `ctest --test-dir build -R "^menu_backspace_utf8$" --output-on-failure`

- [x] **C4** - Com o nome vazio ou `   `, `menu_play_enabled` é falso, e tanto o clique no centro de `Jogar` quanto o Enter devolvem `MENU_NONE` (AC 4)
Proof: `ctest --test-dir build -R "^menu_play_disabled_when_blank$" --output-on-failure`

- [x] **C5** - Com o nome `  Ana  `, o clique em `Jogar` e o Enter devolvem `MENU_PLAY` e o nome usado é `Ana`; na `app_run`, a ação `MENU_PLAY` começa a partida com as animações ligadas, a distribuição em andamento e o nome `Ana` no controlador (AC 5)
Proof: `ctest --test-dir build -R "^menu_play_trims_name$" --output-on-failure`
Proof: `ctest --test-dir build -R "^app_menu_play_starts_game$" --output-on-failure`

- [x] **C6** - O clique no centro de `Sair` devolve `MENU_QUIT`, com nome vazio ou não; na `app_run`, a ação `MENU_QUIT` termina o laço e retorna 0 sem começar a partida (AC 6)
Proof: `ctest --test-dir build -R "^menu_quit_click$" --output-on-failure`
Proof: `ctest --test-dir build -R "^app_menu_quit_returns_0$" --output-on-failure`

- [x] **C7** - Depois de `ctl_set_player_name(c, "Ana")`, a `View` traz `Ana`; o retângulo de `layout_player_name` fica centrado em x = 640, entre a área da mesa (y >= 560) e a mão do Sul (y + h <= 605), e não se sobrepõe ao retângulo de `layout_message` (AC 7)
Proof: `ctest --test-dir build -R "^player_name_above_south_hand$" --output-on-failure`

### S2 - O nome é lembrado · 4 files · ~15 KB · ~4k

- [x] **C8** - `name_save` num caminho cuja pasta ainda não existe cria a pasta e grava exatamente os bytes `5A C3 A9` para `Zé` (UTF-8 sem BOM, sem quebra de linha); na `app_run`, depois do `MENU_PLAY` com `  Ana  `, o arquivo contém exatamente `Ana` (AC 8, door 1)
Proof: `ctest --test-dir build -R "^name_save_format$" --output-on-failure`
Proof: `ctest --test-dir build -R "^app_menu_saves_name$" --output-on-failure`

- [x] **C9** - Com o arquivo contendo `Zé`, `name_load` devolve `Zé` e a tela inicial da `app_run` abre com o campo `Zé` (AC 9)
Proof: `ctest --test-dir build -R "^name_load_prefills$" --output-on-failure`
Proof: `ctest --test-dir build -R "^app_menu_prefills_name$" --output-on-failure`

- [x] **C10** - `name_load` devolve nome vazio para arquivo ausente, arquivo vazio, `Ana\n`, `A\tB`, o byte solto `80` e `Ā` (`C4 80`, código 256); e para 20 letras devolve as 16 primeiras (AC 10, door 1)
Proof: `ctest --test-dir build -R "^name_load_rejects_invalid$" --output-on-failure`

- [x] **C11** - Com o caminho do arquivo dentro de uma "pasta" que na verdade é um arquivo, `name_save` retorna falso e a `app_run` começa a partida mesmo assim depois do `MENU_PLAY`, retornando 0 (AC 11)
Proof: `ctest --test-dir build -R "^app_menu_save_failure_still_plays$" --output-on-failure`

### S3 - O resto continua igual e a `domino.exe` liga o menu · 4 files · ~10 KB · ~3k

- [x] **C12** - A suíte inteira passa, e nenhum arquivo de teste existente mudou desde `d702049` (AC 12, door 2)
Proof: `ctest --test-dir build --output-on-failure`
Proof: `git diff --exit-code d702049 -- tests/test_core.c tests/test_parceria.c tests/test_carroca.c tests/test_fora.c tests/test_window.c tests/test_app_anim.c tests/test_process.c tests/harness.h tests/fixtures.h tests/core_has_no_raylib.cmake`

- [x] **C13** - O `domino.exe` real, com `APPDATA` numa pasta temporária vazia, abre a tela inicial: as teclas `A`, `n`, `a` e Enter enviadas à janela começam a partida, e ao fechar a janela o arquivo `<APPDATA>\Domino\jogador.txt` contém exatamente `Ana` e o processo sai com 0 (AC 1, AC 5, AC 8, door 2)
Proof: `ctest --test-dir build -R "^cli_menu_types_and_saves$" --output-on-failure`

## Coverage

| Set (size) | Member -> proof | Unproven |
| --- | --- | --- |
| códigos de caractere (5 faixas) | < 32 C2 · 32–126 C2 · 127–159 C2 · 160–255 C2 · > 255 C2 | - |
| ações do menu (3) | `MENU_NONE` C4 · `MENU_PLAY` C5 · `MENU_QUIT` C6 | - |
| entradas da tela inicial (4) | caractere C2 · Backspace C3 · Enter C4, C5, C13 · clique C4, C5, C6 | - |
| elementos da tela inicial (5) | título C1 · rótulo C1 · campo C1 · `Jogar` C1 · `Sair` C1 | - |
| arquivo `jogador.txt` (4 status) | `lido` C9 · `ignorado` C10 · `gravado` C8 · `falha ao gravar` C11 | - |
| arquivos ignorados (6) | ausente C10 · vazio C10 · quebra de linha C10 · tab C10 · UTF-8 inválido C10 · código 256 C10 | - |
| `Landing` doors (2) | door 1 C8, C10 · door 2 C12, C13 | - |
| startup config: menu (2 assemblies) | `domino.exe` via `main` C13 · `app_run` dos testes C5, C12 | - |

- O C13 é o único que passa pelo `main` e pelo teclado de verdade; os outros testes da `app_run` usam
  o gancho `menu_input` no lugar do teclado e do mouse, e por isso não provam a leitura das teclas
  pela raylib

## Test policy

| Code | Required proofs | Coverage expectation |
| --- | --- | --- |
| Decide, sem fronteira | um no próprio nível | um caso afirmado por linha da tabela de decisão |
| Decide, alcançado pela `domino.exe` | um no próprio nível **e** um pelo processo real | no processo: digitar, Enter e gravar; no próprio nível: a tabela inteira |
| Instrumentação (desenho da tela inicial) | nenhum próprio | coberto pelos testes de layout e de `View` |

Evidence:

- `src/core/menu.c`: filtro de 5 faixas de código, limite de 16, Backspace em UTF-8, validação com espaços, 3 ações -> decide; chega à `domino.exe` pela `app_run`
- `src/core/namefile.c`: 6 motivos para ignorar o arquivo, truncamento em 16, criação de pasta, falha de gravação -> decide, sem fronteira própria
- `src/ui/app.c` (laço do menu): troca da tela inicial para a partida, saída com 0 -> decide, alcançado pela `domino.exe` (C5, C6, C13)
- `src/ui/draw.c` (tela inicial): só desenha a partir do `Menu` e do layout -> instrumentação
- análogo no repo: `tests/test_process.c` prova o `domino.exe` real; `tests/test_app_anim.c` prova a `app_run` por gancho

Cost: 13 checks, 18 provas em 3 executáveis de teste.

## Swept

- validation: C2, C4, C10 - caracteres recusados, nome em branco, arquivo inválido
- failure modes: C11 - gravação que falha não impede a partida
- idempotency: n/a - gravar o mesmo nome de novo só reescreve o arquivo com os mesmos bytes
- authorization: n/a - arquivo na pasta do próprio usuário, jogo local
- concurrency: n/a - laço único; duas janelas abertas ao mesmo tempo é caso que o jogo não trata (o último a gravar vence)
- data lifecycle: C8, C9 - o arquivo é criado no primeiro Jogar e reescrito a cada Jogar; nunca é apagado pelo jogo
- dependency failure: C11 - sistema de arquivos recusando a gravação
- state transitions: C5, C6, C13 - tela inicial para partida, tela inicial para saída
- observability: n/a - nenhum requisito de log

## Handoff

- Arquivos tocados: `menu.c`/`.h` e `namefile.c`/`.h` (novos, ~8 KB), `controller.c`/`.h` (15 KB),
  `layout.c`/`.h` (6 KB), `app.c`/`.h` (4 KB), `main.c` (1 KB), `draw.c` (8 KB), `CMakeLists.txt` (4 KB),
  testes novos (~20 KB), artefatos (~20 KB). Total ~86 KB / 4 = ~22k, abaixo do orçamento de 150k -
  one builder
