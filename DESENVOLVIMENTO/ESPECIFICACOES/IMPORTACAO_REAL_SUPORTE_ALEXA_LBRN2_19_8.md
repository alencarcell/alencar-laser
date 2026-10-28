# Alencar Laser Qt 19.8 — compatibilidade com arquivo LightBurn real
Data 28/10/2026 · Estado: **não compilado/homologado no Windows nesta branch**.

## Arquivo fornecido
- Nome: SUPORTE PRA ALEXA_backup.lbrn2 (NÃO incluir a arte privada no repositório)
- SHA-256: `3336269e9bae2aa5cb2778123ff7ffea2ea1a8a515d2aca648fb44e22ebbad06`
- 42.185 bytes, XML válido, projeto LightBurn `AppVersion="2.1.00"`.
- Total de 20 elementos Shape: 13 Path + 4 Rect + 3 Group.
- 17 formas renderizáveis após ler as três estruturas Group; 3 grupos com seis objetos agrupados.
- 13 formas fechadas e 4 abertas.
- 1.331 nós vetoriais se B for amostrada em 24 segmentos/cúbica, mantendo vértices e retângulos originais.
- 7 Path com `PrimList=LineClosed` (forma compactada por linhas sequenciais); 6 com L/B explícitas.
- 4 Path sem texto local `PrimList`, referindo-se a `PrimID` de outra Shape (IDs 1, 6 e 8).
- Todos os CutIndex de Shapes presentes = 0. Parâmetros LightBurn de potência/velocidade NÃO são importados.

## Causas concretas do importador antigo
1. `decodePath()` só aceitava listas explícitas L/B; recusava `LineClosed`.
2. `readGeometry()` não resolvia geometria compartilhada por `PrimID`/`VertID`.
3. Havia regressão no teste: importador já desenhava elipse em 384 segmentos, mas o teste ainda exigia 96.
4. Grupos LightBurn eram achatados sem guardar agrupamento no `EditorState`.

## Alterações experimentais
- `src/lightburn_geometry_import.hpp`: reconhecer `LineClosed` e `LineOpen`; ler os IDs de referências; resolver geometria por dois passes em XML (também em grupos), com validação de duplicidade/ausência; preservar grupos no modelo de edição (um nível, grupos aninhados passam para grupo externo).
- `qt/qt6_test6.cpp`: regressões sintéticas de listas compactas, reutilização de VertID/PrimID, grupos, referências inválidas, rejeição atômica e elipse 384; novo modo `--lbrn2-file-19-8-test` para arquivo real.
- `SCRIPTS/TESTAR_19_8_WINDOWS.ps1`: `-ArquivoLbrn2` opcional para teste focado `-SomenteImportacao` e teste `-Abrir`.
- Para o SHA-256 conhecido, teste real exige 17 formas, 1.331 nós, 13 formas fechadas, 3 grupos com 6 membros. Grava apenas JSON de diagnóstico, sem caminho/arte e sem escrever no arquivo fonte.

## Executar no Windows
1. Verificar `git status --short` e preservar modificações locais antes de trocar branch.
2. `git fetch origin`
3. `git switch --track origin/fix/lbrn2-real-shared-primitives-19-8` (ou `git switch fix/lbrn2-real-shared-primitives-19-8` se já local).
4. `powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\TESTAR_19_8_WINDOWS.ps1" -SomenteImportacao -ArquivoLbrn2 "CAMINHO_DO_ARQUIVO_LBRN2"`
5. Somente se PASS, testar `-Abrir` com o mesmo `-ArquivoLbrn2` e selecionar o arquivo no aplicativo.
6. Checar JSON `RELATORIOS\AUDITORIAS\GITHUB_19_8\importacao\RESULTADO_LBRN2_REAL_19_8.json`.

## Limitações
- Leitura de geometria e grupos; fonte não é um clone completo do formato LightBurn. Texto editável, bitmap, potenciais recursos avançados de primitiva, compartilhamento de operações de gravação, hierarquia de grupos e configurações físicas exigem trabalho separado.
- SVG/curvas são amostrados como polilinhas: NÃO curvas Bézier nativas no arquivo .alaser resultante.
- Sem transmissão física, G-code, movimento, potência ou habilitação de Wi-Fi/TCP.
- Sem validação Qt/UCRT64 real desta branch; a auditoria externa em Python apenas confirma interpretação/contagem.
- `main` e `APLICATIVO` não alterados.
