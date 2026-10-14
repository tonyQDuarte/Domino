# Dominó - panorama do projeto

Leia este arquivo primeiro ao continuar o projeto num chat novo. Depois leia `.specs/STATE.md`
(decisões e onde parou) e o `plan.md` / `checks.md` da feature em andamento.

## O que é

Jogo de dominó brasileiro em duplas, em C com raylib, para Windows. Mesa verde, 1 jogador humano
(Sul) e 3 CPUs. Partida até 6 pontos.

- Repositório: https://github.com/tonyQDuarte/Domino (público, licença Apache-2.0)
- Pasta local: `D:\Dev\Git\Domino`
- Versão publicada: tag `v1.1.0` no `main` (bots-parceiros); anterior `v1.0.0` (`8419202`)

## Como compilar e testar

Toolchain MSYS2 UCRT64 (gcc, cmake, ninja, raylib 6.0 do pacote `mingw-w64-ucrt-x86_64-raylib`),
com `C:\msys64\ucrt64\bin` no PATH:

```bash
cmake -S . -B build -G Ninja
cmake --build build
ctest --test-dir build --output-on-failure
```

O jogo fica em `build\domino.exe` (as DLLs da raylib são copiadas para `build\`). `--seed <n>`
repete a mesma partida. Python não está instalado, então os scripts `validate_*.py` da skill não
rodam: as checagens deles são feitas lendo os artefatos.

## Arquitetura (decisões em `.specs/STATE.md`)

- `src/core/` = `domino_core`: regras (`rules.c`), CPU (`cpu.c`), layout (`layout.c`), controlador
  com animações (`controller.c`), tela inicial (`menu.c`), arquivo do nome (`namefile.c`), CLI
  (`cli.c`). **Nunca inclui `raylib.h`** (AD-003), e o teste `core_has_no_raylib` garante isso.
- `src/ui/` = `domino_app` + `domino.exe`: janela, laço (`app.c`), desenho (`draw.c`), `main.c`.
- A CPU decide só por `CpuView` (o que é visível), nunca por `Game` (AD-005).
- Animações e tela inicial ficam desligadas por padrão e são ligadas pela `app_run`/`main`
  (AD-006, AD-007), para os testes de regra rodarem sem janela.
- Dados do jogador em `%APPDATA%\Domino\` (AD-007): hoje só `jogador.txt` com o nome.

## Regras implementadas

- 28 peças, **6 por jogador e 4 de fora** (de costas no canto superior direito, viradas no fim da mão).
- Primeira mão: abre quem tem a **maior carroça distribuída**, só com ela.
- Mãos seguintes: abre com **carroça**; a ordem é quem bateu, parceiro, jogador seguinte, parceiro dele.
  Depois de um tranque, começa pela dupla que ganhou o ponto (empate: por quem abriu).
- Vez no sentido anti-horário: Sul -> Leste -> Norte -> Oeste. Sem compra; quem não tem jogo passa.
- Pontos: batida simples 1, carroça 2, lá-e-lô 3, cruzada 4; tranque 1 para a dupla com menos pontos
  na mão (empate: ninguém pontua). Partida até 6.
- CPU em parceria (estilo atual = "Audi"): joga pelo parceiro quando ele tem menos peças.

## Features (`.specs/features/`)

| Feature | O que entregou | Status |
| --- | --- | --- |
| `partida-duplas` | partida jogável em duplas, mesa verde, pontuação | verificada (light), no `main` |
| `cpu-parceria-animacao` | CPU em parceria, animações | verificada (standard), no `main` |
| `abertura-carroca` | abertura com carroça e ordem de abertura | verificada (standard), no `main` |
| `pecas-de-fora` | 6 por jogador, 4 de fora | verificada (standard), no `main` |
| `tela-inicial` | tela inicial com nome salvo | verificada (standard), no `main`, tag `v1.0.0` |
| `bots-parceiros` | 6 bots com nome e estilo, escolha do parceiro | verificada (standard), no `main`, tag `v1.1.0` |

Algumas regras antigas foram substituídas por features novas a pedido do usuário; os `checks.md`
antigos têm notas `> Substituído por ...` nos pontos afetados, em vez de linhas apagadas.

## Como trabalhamos (skill `tlc-spec-lean`)

1. `plan.md` da feature -> o usuário aprova e escolhe o perfil (tem escolhido `standard`).
2. `checks.md` com uma prova CTest por check -> testes escritos a partir dos checks -> código.
3. Um Verifier independente (sub-agente novo) roda tudo, injeta falhas e escreve `verification.md`.
   Se reprovar, os testes são reforçados e há uma rodada 2 restrita ao que mudou.
4. Cada feature vai num branch `feat/<nome>`. **Merge no `main` só depois do PASS do Verifier e da
   confirmação do usuário.**

## Git e GitHub

- Identidade: Antonio Duarte <tony.qduarte@gmail.com> (global). Nunca usar o e-mail `strada.log.br`.
- Remoto `origin` = `https://tonyQDuarte@github.com/tonyQDuarte/Domino.git`, conta `tonyQDuarte`.
- **O push é feito pelo usuário no terminal dele**: o gerenciador de credenciais desta máquina é
  antigo e não abre login no terminal do agente. Depois do push, o agente confere com `git fetch`
  e com a API pública do GitHub.
- Mensagens de commit no padrão Conventional Commits, sem linha de co-autoria.
