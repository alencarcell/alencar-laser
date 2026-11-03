# Alencar Laser 19.9 — Importador LightBurn de Bézier nativa (03/11/2026)

## Objetivo e estado
Evitar transformar cada aresta Bézier `B` da fonte LightBurn em 24 segmentos retos, preservando âncoras, tangentes de entrada/saída, tipos de aresta, fechamento, camadas e transformações `XForm` no formato `alaser::CurveDocument`.

**Estado:** módulo novo **somente leitura e independente da GUI** `src/lightburn_native_import_19_9.hpp`, testes novos `tests/lightburn_native_19_9.cpp`, alvo CMake `alencar_test19_9_lbrn2_native`, gate `SCRIPTS/TESTAR_19_8_WINDOWS.ps1 -SomenteBezierNativo`. Ainda não foi compilado com Qt no Windows do operador nesta revisão, nem foi conectado ao menu Arquivo. Sem merge, sem uso de hardware.

## Evidência independente do arquivo real
Fonte: arquivo `ampulheita do tempo(1).lbrn2`, lido localmente, SHA-256 `65638a6fd7425fa69e1af47d02182eee57e372f42fd46e1608b7de0cdffbe86d`, LightBurn XML com 2.134 formas: 2.133 `Path` e 1 `Ellipse`. Sem grupos. Há reutilização de `VertID`/`PrimID` e o parser resolve referências só após ler todo o XML, preservando referências futuras. Os campos `c0x/y` e `c1x/y` de cada vértice representam alças que são aplicadas à aresta `B`; `L` fica reta.

| Geometria | Conversão poligonal legada | Conversão nativa planejada |
|---|---:|---:|
| Quantidade de formas da fonte | 2.134 | 2.134 |
| Contornos editáveis (esperado) | não homologado | 2.134 |
| Nós/âncoras resultantes | ~397 mil (estimativa) | **17.947** |
| Arestas cúbicas nativas | 0 após tesselação | **16.417**, incluindo 16 da elipse |
| Arestas retas | segmentos tessellados misturados | **1.530** |
| Curvas Bézier originais da fonte | 16.401 | 16.401 preservadas como cúbicas |

A contagem acima foi obtida por uma leitura independente do XML com Python, **não por execução do novo código C++ Qt no Windows**. Foi convertida em asserção precisa no teste do arquivo original: em caso de divergência, teste FAIL.

## Regras de segurança
- `readNativeGeometry199()` não altera o editor, não seleciona camadas nem gera G-code. Valida `CurveDocument` inteiro antes do retorno; erro impede qualquer importação parcial.
- Preserva transformações `XForm` tanto de âncoras quanto das alças, também aceita curvas mistas `B` e `L` sem costurar subcaminhos desconexos.
- Respeita limites: arquivo até 32 MiB, `readShape` limita formas e profundidade, até 30 mil contornos e 250 mil âncoras nativas, posições finitas e dentro de alcance seguro.
- Rejeita `Group` até existir persistência fiel de agrupamento no formato nativo; recusar é mais seguro que descartar hierarquia.
- Elipse é uma cônica exata na origem, mas `CurveDocument` contém cúbicas: **somente a elipse** é aproximada com 16 arcos cúbicos curtos. Registrar esta limitação e comparar tolerância visual antes da integração à GUI.
- `MirrorX/MirrorY` ativos são rejeitados por não haver homologação de referência/origem de máquina.
- Sem envio de fonte do usuário ao GitHub, GitHub Actions, terceiro ou máquina laser; a fixture sintética é suficiente para teste mínimo. Arquivo real usado no computador do operador com `-ArquivoLbrn2`.

## GATE Windows
```powershell
cd C:\ProjetoAlencarLaser
git status --short
# Somente após preservar modificações rastreadas e fechar Alencar Laser:
git pull --ff-only origin fix/curvas-tesoura-preflight-19-8-20261030
powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\TESTAR_19_8_WINDOWS.ps1" -SomenteBezierNativo
# Auditoria do arquivo original, quando caminho local estiver disponível:
powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\TESTAR_19_8_WINDOWS.ps1" -SomenteBezierNativo -ArquivoLbrn2 "C:\CAMINHO\ampulheita do tempo.lbrn2"
```

A aprovação exige `PASS native LBRN2 19.9` e `REAL LBRN2: sourceShapes=2134 nativeContours=2134 nativeAnchors=17947 cubicEdges=16417 straightEdges=1530`. Se falhar, corrigir parser antes de integrar à UI.

## Próximos gates — AINDA NÃO REALIZADOS
1. Compilar/rodar no Windows e testar arquivo real, comparar 3–5 amostras de cúbicas por avaliação matemática e verificar contornos fechados.
2. Integrar como fluxo **experimental opt-in e transacional** ao editor, sem apagar desenho atual antes da validação/decisão de salvar ou descartar; persistir `CurveDocument` integral em `.alaser` v5 e reabrir com hashes corretos.
3. Medir RAM, tempo de carregar, `paint_p95_ms`, seleção de objetos, Undo/Redo e estabilidade com o arquivo original.
4. Somente depois decidir se importador nativo pode substituir conversão legada, com rollback e revisão manual.

Versão homologada, `main`, `APLICATIVO`, instalador e máquina não foram alterados.
