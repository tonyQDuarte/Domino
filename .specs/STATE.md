# Project state

## Decisions

| ID | Decision | Rationale | Status | Date |
| --- | --- | --- | --- | --- |
| AD-001 | Toolchain é MSYS2 UCRT64 (gcc, cmake, ninja) | O MinGW.org 6.3 instalado é 32 bits e não é compatível com os binários da raylib | active | 2026-09-27 |
| AD-002 | Gráficos e entrada pela raylib (pacote MSYS2), via `find_package(raylib)` | Escolha do usuário; C puro, desenha texto sem bibliotecas extras | active | 2026-09-27 |
| AD-003 | Regras ficam em `domino_core`, que nunca inclui `raylib.h`; tela em `domino_ui` | Mantém as regras testáveis sem janela, e toda feature nova segue essa separação | active | 2026-09-27 |
| AD-004 | Testes são executáveis C puros registrados no CTest | Não traz dependência; `ctest` dá o código de saída que serve de prova | active | 2026-09-27 |
| AD-005 | A CPU decide só por `CpuView` (própria mão, pontas, quantidades, passes); nunca por `Game` | O usuário escolheu CPU sem trapaça; a estrutura garante isso para regras futuras | active | 2026-09-27 |
| AD-006 | Animações vivem na linha do tempo do controlador em `domino_core`, desligadas no `ctl_init` e ligadas pela `app_run` | Testáveis sem janela, e os testes de regra continuam sem animação | active | 2026-09-27 |

## Handoff

**Feature**: cpu-parceria-animacao
**Where**: plan aprovado (standard), checks escritos; construindo
**In progress**: nada
**Next step**: testes de C1-C23, depois implementação
**Blockers**: nenhum
**Uncommitted**: nada
**Branch**: feat/partida-duplas
