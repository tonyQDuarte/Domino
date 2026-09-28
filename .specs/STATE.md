# Project state

## Decisions

| ID | Decision | Rationale | Status | Date |
| --- | --- | --- | --- | --- |
| AD-001 | Toolchain é MSYS2 UCRT64 (gcc, cmake, ninja) | O MinGW.org 6.3 instalado é 32 bits e não é compatível com os binários da raylib | active | 2026-09-27 |
| AD-002 | Gráficos e entrada pela raylib (pacote MSYS2), via `find_package(raylib)` | Escolha do usuário; C puro, desenha texto sem bibliotecas extras | active | 2026-09-27 |
| AD-003 | Regras ficam em `domino_core`, que nunca inclui `raylib.h`; tela em `domino_ui` | Mantém as regras testáveis sem janela, e toda feature nova segue essa separação | active | 2026-09-27 |
| AD-004 | Testes são executáveis C puros registrados no CTest | Não traz dependência; `ctest` dá o código de saída que serve de prova | active | 2026-09-27 |

## Handoff

**Feature**: partida-duplas
**Where**: plan.md escrito, aguardando revisão do usuário
**In progress**: nada
**Next step**: com o plano aprovado, escrever `checks.md`
**Blockers**: toolchain MSYS2 ainda não instalado
**Uncommitted**: `.specs/` (o diretório ainda não é repositório git)
**Branch**: nenhum
