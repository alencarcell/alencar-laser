# ALENCAR LASER — RELATÓRIO DE CONTINUIDADE E AUDITORIA REMOTA

**Data da auditoria:** 23/11/2026  
**Repositório:** `alencarcell/alencar-laser`  
**Branch auditada:** `feat/selecao-mista-alinhamento-svg-bezier-20ze-20261112`  
**HEAD remoto confirmado:** `be7f46820d515d4c231aaf62340cf3ee862671d5`  
**Commit:** `feat: validação e geração de engrenagens 19.9`

## 1. Escopo e limites desta auditoria

Esta revisão foi feita sobre o conteúdo remoto do GitHub. Não houve acesso ao disco `C:\ProjetoAlencarLaser`, ao terminal PowerShell do operador ou ao hardware. Portanto, este documento não declara que a compilação local, a suíte Qt ou a homologação visual/física passaram.

O checkpoint de 16/11 foi atualizado em termos de estado: as cinco alterações rastreadas que nele apareciam como pendentes foram incluídas no commit `be7f468`.

## 2. Sincronização confirmada

Comparação remota entre `c7c8e9fa05afe344934c6218ff99acfeb4d1436e` e `be7f46820d515d4c231aaf62340cf3ee862671d5`:

- 1 commit à frente;
- 0 commits atrás;
- 7 arquivos alterados;
- 248 inserções e 17 exclusões.

Arquivos incluídos:
- `AUDITORIA_GITHUB/INDICE.json`
- `AUDITORIA_GITHUB/REGISTROS/2026-11-05/interaction_session_be7e09471706d603.json`
- `DESENVOLVIMENTO/ESPECIFICACOES/CHECKPOINT_CONTINUIDADE_GROK_20261116.md`
- `DESENVOLVIMENTO/FONTE/qt/gear_dialog_19_9.hpp`
- `DESENVOLVIMENTO/FONTE/qt/qt6_test6.cpp`
- `DESENVOLVIMENTO/FONTE/src/gear_generator_19_9.hpp`
- `DESENVOLVIMENTO/FONTE/tests/gear_generator_19_9_standalone.cpp`

## 3. Alterações de engrenagens presentes no HEAD

O código remoto confirma:
- limite geral de dentes alterado para 4–120;
- perfil Angular/Sawtooth aceita base angular de 10–98%;
- posição da ponta aceita 0–100%;
- teste standalone inclui engrenagem Angular de 4 dentes, base 98%, ponta em 100%, além da posição 0%;
- perfil Involute continua como padrão compatível.

### Limites técnicos que precisam permanecer explícitos

O próprio código descreve os perfis Angular/Sawtooth como formas editoriais de corte, não como perfil certificado para engrenamento. O gerador de par nominal e a cremalheira mantêm `meshingValidated=false`. A documentação e a interface devem continuar distinguindo geometria vetorial gerada de validação mecânica, tolerância de kerf, resistência e funcionamento real.

A aceitação de quatro dentes pelo gerador e pelo teste comprova apenas que o algoritmo produz a geometria esperada segundo os testes existentes. Não comprova resistência, ausência universal de auto-interseções ou funcionamento mecânico.

## 4. Registro de auditoria de interação

O registro `be7e09471706d603` está marcado como `MANUAL_REVIEW_PENDING`. Nele:
- seleção, arraste, zoom/pan, tesoura e undo/redo estão como `NOT_TESTED`;
- casos testados: 0;
- casos com falha: 0;
- amostras de memória: 174;
- RSS P95: 115,109 MB;
- RSS máximo: 115,488 MB;
- CPU P95: 25%.

Essas métricas não substituem teste funcional. Zero falhas com zero casos testados não é evidência de aprovação.

## 5. Evidência local comunicada pelo operador (não reproduzida nesta auditoria)

Nas capturas compartilhadas em 18/11:
- o script `SCRIPTS/ABRIR_19_9_WINDOWS.ps1` foi executado e informou runtime UCRT64 encontrado, executável `alencar_test7_qt6.exe` e abertura do editor experimental;
- a janela do Gerador de Engrenagens foi exibida com pré-visualização;
- tentativas anteriores de build falharam por invocação incorreta/localização do CMake e, depois, o build via Make/Ninja mostrou falhas de compilação em objetos Qt 6/15/17. O trecho visível não expôs as mensagens completas dos erros C++;
- não foi apresentada nesta auditoria uma saída final inequívoca de compilação limpa da branch `be7f468`, nem relatório completo da suíte.

Assim, a abertura do executável e a visualização do diálogo são evidências de execução de um binário, mas não provam que esse binário foi compilado a partir do HEAD `be7f468`.

## 6. Arquivos locais não rastreados vistos nas capturas

O `git status --short` exibido pelo operador listou arquivos não rastreados, incluindo:
- `.vscode/`;
- `DELTA_LOCAL.patch` e `PATCH_PRE_SYNC_ALENCAR.patch`;
- backups `.backup-*` de fontes e testes;
- `HOMOLOGACAO_PORTATIL/`;
- relatórios e logs de compilação;
- scripts locais de desempenho e reparação.

O operador mostrou apenas os primeiros 80 registros dos dois patches. Portanto, o conteúdo integral e a relação entre ambos não foram auditados. Nenhum desses arquivos foi removido, publicado ou alterado por esta auditoria.

A orientação anterior permanece válida: preservar backups; não executar `git add .`; não publicar `.vscode/settings.json`, binários, DLLs, pacotes portáteis ou relatórios com caminhos/dados locais sem revisão individual.

## 7. Próximas ações técnicas recomendadas

1. No Windows, confirmar branch e SHA com:
   ```powershell
   git status --short
   git branch --show-current
   git rev-parse --short HEAD
   ```
2. Se a intenção for testar exatamente o código remoto, confirmar que o SHA local é `be7f468` antes de compilar. Não trocar de branch com alterações locais pendentes.
3. Reproduzir build em diretório isolado e capturar o primeiro erro completo, sem ocultar a saída do compilador.
4. Executar primeiro o teste standalone do gerador e registrar comando, código de saída e SHA.
5. Executar regressões Qt focadas e a suíte pertinente; separar PASS automatizado de inspeção manual.
6. Validar separadamente perfis de 4 dentes, topologia, segmentos muito curtos e casos de contato/colinearidade. Não inferir segurança de fabricação.
7. Só depois avaliar os patches locais e decidir quais arquivos devem ser versionados.

## 8. Restrições de segurança e integração

- Não mesclar PRs experimentais em `main` com base apenas neste relatório.
- Não atualizar `APLICATIVO` nem instalador sem homologação específica.
- Não executar GRBL/G-code, movimento ou emissão de laser.
- Manter hardware desconectado durante testes de software.
- Não apagar backups, não usar `git clean` e não executar `git reset` para tentar limpar o estado local.
- Não declarar homologação mecânica ou de produção a partir de testes de geometria offline.

**Conclusão:** o commit `be7f468` está confirmado no GitHub e incorpora as sete alterações listadas. A revisão remota confirma o conteúdo dos arquivos e os limites declarados pelo código; compilação limpa do HEAD, regressões Qt e homologação manual permanecem pendentes de evidência local.


## 9. Verificação adicional de integração (23/11/2026)

- A branch de trabalho foi confirmada no HEAD `35427776376c141ae9b0d3a3f4927cfb9562dfff`.
- A branch `main` permanece em `c14008afd0705cc91449b683162e6c1430566cf0`; não foi alterada.
- Comparação entre `main` e a branch de trabalho retornou `diverged`, com merge-base em `397341d139d974e0f3d90b79ed28604e697a81b1`, 1.019 commits à frente e 2 atrás segundo o endpoint de comparação. O volume elevado reflete a história divergente e **não deve ser interpretado como 1.019 funcionalidades novas**.
- A busca por PR com head exatamente `feat/selecao-mista-alinhamento-svg-bezier-20ze-20261112` não retornou PR aberto ou fechado. Portanto, o checkpoint/branch não está vinculado a uma proposta de integração identificada nesta consulta.
- A consulta de Actions para a branch retornou zero execuções. O status do commit de documentação `3542777` retornou `pending`, sem status individuais. Isso não equivale a falha de testes; significa ausência de evidência de CI para esse commit.
- O índice `AUDITORIA_GITHUB/INDICE.json` contém três registros e aponta o relatório mais recente para 05/11/2026. O relatório desta auditoria é posterior, mas ainda não foi incluído nesse índice. Não alterei o índice para não misturar um relatório de auditoria documental com os registros técnicos sanitizados de sessão sem definir o esquema apropriado.

### Consequência operacional

Não abrir integração automática contra `main` nem tentar rebase/merge remoto. Antes de propor PR, é necessário identificar a branch-base correta e entender a divergência histórica, confirmar se a branch é de continuidade local do operador e reconciliar o estado Windows. O trabalho permanece isolado e a branch principal intacta.


## 10. Auditoria de integração de testes e CI (24/11/2026)

### CMake / CTest

Foi lido integralmente `DESENVOLVIMENTO/FONTE/CMakeLists.txt` no HEAD da branch de trabalho (`c7308d3c70907f465361187f1d003aa22d9d3421); blob do CMake `ff3f3634cdfa7eb6c85845624c66bc4926c96693`).

- O arquivo declara múltiplos executáveis standalone para recuperação, tesoura, Bézier, distribuição, engrenagens, mecanismos, histórico e importação LightBurn.
- A configuração Qt usa `ALENCAR_REQUIRE_QT` para permitir exigir o SDK em builds que não podem aceitar um falso sucesso sem GUI.
- O arquivo termina após as declarações dos alvos 19.9. Na versão examinada, não há chamadas a `enable_testing()`, `include(CTest)` ou `add_test()`.
- Portanto, os alvos existem para compilação, mas não há registro de descoberta/execução automática deles via CTest. Não afirmar que CTest executa a suíte.
- Alguns alvos dependem de Qt6 e outros de Boost.JSON, minizip e ZLIB; a disponibilidade dessas dependências precisa ser considerada por qualquer integração futura.

### Workflows

Foram lidos os cinco YAMLs presentes na branch:

- `.github/workflows/portable-curves-19-8.yml`
- `.github/workflows/preview-19-8-offline.yml`
- `.github/workflows/qt-windows-gestures-19-8.yml`
- `.github/workflows/powershell-parser-19-8.yml`
- `.github/workflows/central-auditoria-19-9.yml`

Constatações:

1. Os workflows `portable-curves`, `powershell-parser`, `qt-windows-gestures` e `central-auditoria` têm gatilhos `push` concentrados na branch antiga `fix/curvas-tesoura-preflight-19-8-20261030`.
2. Os gatilhos `pull_request` também apontam para branches-base específicas e antigas (`fix/validacao-real-19-8-20261029` ou `feat/refino-curvas-rastreio-diagnosticos-19-8`), não para a branch atual de auditoria.
3. O workflow `preview-19-8-offline` tem gatilhos para duas branches antigas e um gatilho manual `workflow_dispatch`; seu job Windows de build Qt é explicitamente ignorado em eventos `pull_request`.
4. O workflow `qt-windows-gestures` tenta compilar e executar um conjunto amplo de testes offline no Windows, mas seus filtros de branch também não incluem a branch de auditoria.
5. A consulta de Actions para `feat/selecao-mista-alinhamento-svg-bezier-20ze-20261112` retornou zero execuções; o status do commit `c7308d3c70907f465361187f1d003aa22d9d3421` retornou `pending` e zero status individuais.

### Interpretação e ações seguras

- A ausência de execuções na branch atual é coerente com os filtros de branch dos workflows; não é evidência de falha de compilação.
- Os workflows não devem ser simplesmente ampliados para todas as branches sem uma política de integração: isso poderia executar jobs duplicados ou testar branches com dependências e bases incompatíveis.
- Recomenda-se criar uma matriz explícita de alvos offline, Qt/Windows e validação manual; depois integrar os testes portáveis ao CTest com condições de dependência claras.
- Antes de modificar CMake ou workflows, escolher uma branch de integração adequada e confirmar sua relação com a PR e com o estado local do operador.

**Nenhum CMake, YAML, código funcional, branch principal, PR ou configuração de hardware foi alterado nesta etapa.** Esta seção é auditoria documental do conteúdo remoto; nenhum workflow foi executado por esta revisão.

## 11. Auditoria de dependências Bézier, GUI e persistência v5 (24/11/2026)

Foram inspecionados os fontes qt6_test11_seams.cpp até qt6_test17_mission.cpp na branch feat/selecao-mista-alinhamento-svg-bezier-20ze-20261112. Esta é uma revisão estática do código remoto; os executáveis não foram compilados nem executados nesta etapa.

### Mapa de dependências observado

- **Geometria e renderização:** os testes 11–12 usam geometry.hpp, Document, Contour e renderização Qt (QPainter/QGraphicsScene). São ensaios de rasterização/atualização de cena, não testes de interação física com mouse.
- **Bézier nativo:** os testes 13–14 usam bezier_native.hpp e bezier_interaction.hpp; trabalham com CurveDocument, CurveContour, CurveDragSession, edgeAt, nativeCurvePath e hit-testing.
- **Integração com a janela:** os testes 15–17 incluem diretamente qt6_test6.cpp (renomeando o main) e exercitam LabWindow, a viewport, eventos Qt, ferramentas e salvamento. Portanto, são acoplados ao código da janela/monólito de teste 6, não apenas às bibliotecas geométricas.
- **Persistência:** os testes 15–17 dependem de readIntegratedProject, writeIntegratedProject/rotinas de save, encodeCurves, nativeCurvePath e do formato integrado versão 5. O teste 16 também verifica metadados de editor, operações e proveniência de amostras.

### Cobertura real descrita pelos fontes

| Teste | Cobertura pretendida no código | Limite importante |
|---|---|---|
| 11 | Compara contorno completo e dividido, SquareCap/FlatCap, zoom/DPR e subpixel; gera CSV, PNG e JSON | Mede diferenças; não declara aprovação de produção nem homologa Windows |
| 12 | Atualização incremental de QGraphicsScene versus reconstrução completa, 96 frames e cancelamento sem pixels fantasmas | Offscreen; sem mouse real, Windows ou máquina |
| 13 | Curvas cúbicas nativas na QGraphicsView, zooms, atualização e cancelamento | Compara janelas/cenas Qt; não é fluxo completo do editor de produção |
| 14 | Hit-test de traçado, seleção, arraste, preview, commit, Undo/Redo, Escape e contornos coincidentes | Implementa uma classe CurveEditor14 própria; não instancia LabWindow. Assim, não prova por si só a integração do editor de produção |
| 15 | Janela LabWindow: seleção, inserção Shift+clique, Undo/Redo, arraste de alça, Escape, Delete, gravação e reabertura v5 em outro processo | Forte teste de integração, mas precisa ser compilado/executado; o modo --verify deve receber o mesmo arquivo de projeto gravado pelo --write |
| 16 | Janela LabWindow: importação v5, curvas nativas, metadados, operações e proveniência; gravação e verificação separada | Depende de fixture de entrada v5 e de duas execuções; no modo --verify, o argumento de origem continua necessário porque o código lê origin antes de ramificar |
| 17 | Janela LabWindow: zoom, seleção de subpaths coincidentes, inserção, Undo/Redo, arraste de âncoras/alças, Escape, Delete, persistência, arquivos inválidos e schema futuro | Mais abrangente, mas é o teste mais acoplado e exige diretório temporário isolado, fixtures e execução em processos/etapas corretos |

### Riscos e observações estáticas

1. **Teste 14 não equivale à homologação da janela real.** Ele cria seu próprio CurveEditor14 e seu próprio QGraphicsScene. Serve como teste de contrato da sessão/hit-test, mas o teste 15/17 é que alcança LabWindow.
2. **Teste 15 e 17 compartilham o monólito qt6_test6.cpp.** Uma mudança de API, estado ou comportamento no arquivo central pode afetar vários testes ao mesmo tempo. Isso reforça a necessidade de build completo e execução ordenada; não justifica refatorar agora sem ambiente.
3. **Persistência v5 exige teste de ida e volta em processo independente.** Um único teste de serialização em memória não cobre a restauração da GUI, identidade dos nós, metadados, operações e proveniência. Os testes 15–17 já tentam cobrir partes diferentes desse contrato.
4. **Teste 16 --verify lê a origem antes da ramificação.** A execução precisa manter disponível a fixture de origem usada no --write; não basta fornecer apenas o arquivo de destino. Isso deve ser refletido no comando/roteiro de CI.
5. **Teste 17 tem saídas auxiliares deliberadas.** Além do projeto de destino, grava .expected, .png, .corrupt e .unknown. Executá-lo diretamente sobre arquivos do usuário seria inadequado; usar somente pasta temporária descartável e fixture copiada.
6. **Teste 17 inclui um caminho de teste de singleton e cenários sintéticos.** Esses cenários ajudam a encontrar regressões de seleção/identidade, mas não substituem a fixture real de projetos do usuário nem o teste manual no Windows.
7. Os testes 11–13 fazem comparações de pixels/rasterização; diferenças entre plataformas, Qt, DPR e backend podem exigir evidências e tolerâncias justificadas. Não relaxar igualdade estrita sem diagnóstico.
8. Os comentários e JSONs dos próprios testes explicitam machine_connected=false e/ou windows_homologated=false. Nenhum desses testes autoriza conectar ou movimentar a máquina laser.

### Ordem de validação recomendada

1. Build dos alvos standalone de geometria e Bézier, com dependências explícitas e sem alterar a produção.
2. Executar testes 11–13 em ambiente Qt offscreen e guardar artefatos por SHA/versão de Qt.
3. Executar teste 14 para contrato de interação/hit-test isolado.
4. Executar teste 15 em pasta temporária: --write e depois --verify no arquivo produzido.
5. Executar teste 16 com fixture v5 original preservada e arquivo de saída separado; manter a origem disponível também no modo --verify.
6. Executar teste 17 em diretório temporário exclusivo, com as etapas --interaction, --write e --verify conforme seus argumentos; verificar os sidecars antes de limpar a pasta temporária.
7. Repetir build e testes no Windows/Qt SDK real e realizar homologação visual/manual do editor, sem conectar ou emitir comandos para a máquina laser.

**Conclusão desta etapa:** há uma cadeia de testes mais completa do que a auditoria inicial sugeria, especialmente nos testes 15–17. O principal risco não é ausência total de cobertura, mas a diferença entre testes isolados e a janela integrada, o acoplamento ao qt6_test6.cpp, a orquestração de fixtures/arquivos temporários e a falta de evidência de execução no HEAD atual. Nenhuma alteração funcional é recomendada antes de executar a matriz acima em ambiente controlado.

**Nenhum código funcional, CMake, workflow, branch principal, PR, arquivo local ou configuração de hardware foi alterado nesta etapa.**

## 12. Auditoria de importação/exportação SVG e LightBurn (24/11/2026)

Escopo: revisão estática de svg_import_20ze.hpp, svg_export_20zc.hpp, lightburn_geometry_import.hpp, lightburn_native_import_19_9.hpp, testes LightBurn e trechos de integração em qt6_test6.cpp. Nenhum executável foi compilado ou executado nesta etapa.

### SVG — capacidades verificadas no código

- O importador 20ze lê XML offline e converte path, line, polyline, polygon, rect, circle e ellipse em CurveDocument; preserva segmentos cúbicos como Bézier nativa.
- O parser de path contempla M/L/H/V/C/S/Q/T/A/Z, incluindo comandos relativos, curvas quadráticas convertidas em cúbicas e arcos convertidos em segmentos cúbicos.
- Há suporte a grupos SVG (g), transformações matrix/translate/scale/rotate/skewX/skewY, viewBox, dimensões físicas e unidades mm/cm/in/pt/pc/px/q.
- Há limites declarados para tamanho do arquivo, número de contornos, âncoras e coordenadas. O fluxo de importação na janela adiciona geometria ao desenho existente, pede confirmação e tem teste de Undo/Redo e drag-and-drop.
- O exportador 20zc grava SVG 1.1 com dimensões físicas em mm, viewBox em mm, caminhos vetoriais, curvas Bézier C, fechamento Z e gravação atômica QSaveFile. Exporta os vetores visíveis, não CAM, potência, velocidade, bitmap ou G-code.
- O teste SVG está embutido em qt6_test6.cpp nos modos --svg-export-20zc-test e --svg-import-20ze-test; o alvo Qt alencar_test7_qt6 compila esse arquivo. Não há alvo CMake separado e dedicado exclusivamente a SVG.

### Lacunas e riscos identificados

1. **Efeitos no elemento raiz:** checkNoClip() é aplicado aos elementos filhos durante readChildren(), mas não é chamado sobre os atributos do elemento raiz svg. Um clip-path, mask ou filter aplicado diretamente ao SVG raiz pode não ser refletido na geometria importada. Antes de aceitar esse tipo de arquivo, o importador deve rejeitar explicitamente o efeito no root ou implementá-lo.
2. **Leitura permissiva de listas numéricas:** numbers() extrai substrings numéricas por regex sem validar integralmente os caracteres entre elas. Isso é usado em viewBox, transformações e listas de pontos. Dados malformados com texto intercalado podem ser interpretados como se fossem uma lista válida. Recomenda-se validar separadores e consumir integralmente a string, com testes negativos.
3. **Dimensão de exportação Bézier:** o exportador calcula bounds incluindo âncoras e pontos de controle, em vez dos extremos analíticos da curva cúbica. Isso gera um retângulo conservador (possivelmente maior que o desenho visível), não altera as coordenadas da curva, mas pode fazer width/height informados no SVG excederem a extensão visual real. Para cotas físicas confiáveis, calcular os extremos da Bézier e testar com curvas cujos controles ficam fora da caixa visual.
4. **Semântica de visibilidade entre vetores legados e nativos:** polilinhas legadas exigem metadata de objeto e camada; curvas nativas tratam camada ausente como visível. Essa diferença está comentada no código, mas precisa de teste explícito para garantir que ocultar/mostrar camadas tenha comportamento previsível em desenhos mistos.
5. **Estilos SVG:** o importador deliberadamente importa geometria, não estilos, preenchimentos, espessuras, cores, texto, imagens ou referências use. Elementos text/image/use/foreignObject são rejeitados; clip-path/mask/filter em filhos são rejeitados. Gradientes, patterns, markers e style são ignorados como definições, não convertidos. Isso precisa permanecer claro na interface para evitar expectativa de fidelidade visual integral.
6. **Escala e alinhamento:** preserveAspectRatio=slice é recusado; meet/none são tratados. Faltam evidências executadas com arquivos reais de Inkscape/Illustrator/LightBurn contendo viewBox, unidades, grupos aninhados, rotação e skew. Os testes sintéticos existentes não equivalem a essa matriz de compatibilidade.
7. **Precisão de exportação:** coordenadas são serializadas com 9 casas decimais e stroke-width fixo de 0.1 mm; o arquivo representa caminhos de traço sem preencher áreas. Isso é uma escolha de representação vetorial, não exportação de parâmetros de fabricação nem de preenchimento/corte.
8. **Importação LBRN2 conservadora antiga:** lightburn_geometry_import.hpp aproxima cada curva B por 24 segmentos lineares e impõe limite de 250 mil nós; grupos são achatados com um nível de grupo e formas incompatíveis são recusadas. É útil como caminho legado de geometria, mas não preserva Bézier nativa.
9. **Importação LBRN2 nativa experimental:** lightburn_native_import_19_9.hpp preserva comandos B como Bézier nativa, resolve VertID/PrimID compartilhados após ler o XML completo e recusa grupos/hierarquia, espelhamento MirrorX/MirrorY, tipos não suportados e arquivos acima de 32 MB. Elipses são aproximadas por 16 arcos cúbicos; isso não é representação cônica analítica exata.
10. **Camadas e agrupamento LBRN2:** o parser nativo carrega CutIndex em CurveContour.layer, mas o CurveDocument não preserva hierarquia de grupos. O parser recusa grupos para não achatá-los silenciosamente. A importação nativa ainda é exploratória e não substitui o caminho ativo da GUI sem validação de camada, agrupamento, IDs e persistência.
11. **Segurança de importação:** ambos os importadores recusam arquivos com MirrorX/MirrorY ativo por falta de homologação da origem/orientação. A importação nativa ignora deliberadamente configurações de corte do LightBurn; nenhuma informação de potência/velocidade deve ser tratada como importada.

### Matriz de validação que falta

- SVG: root clip/mask/filter; listas numéricas com caracteres inválidos; unidades físicas e CSS px; viewBox com meet/none/slice; transforms aninhados e skew; comandos S/T/A e paths múltiplos; geometria Bézier com bounds extremos; arquivo com 32 MB/limites; camadas ocultas e desenho misto.
- LBRN2: fixture real com grupos, subpaths desconectados, referências compartilhadas, curvas fechadas/abertas, elipses, transforms, camadas e MirrorX/Y; comparar contagem e coordenadas contra o LightBurn sem salvar por cima do original.
- Roundtrip: importar SVG/LBRN2 para cópia .alaser v5, salvar, fechar, reabrir em processo independente, exportar SVG e comparar coordenadas, IDs, número de contornos, fechamento, handles e dimensões. Não exigir preservação de estilo/bitmap quando o formato/código explicitamente não os suporta.
- GUI: testar diálogo de importação, confirmação, cancelamento, drag-and-drop, Undo/Redo e falha no meio do parse sem substituir o desenho existente.

### Decisão técnica

- **SVG export:** implementação tem uma base coerente e teste integrado escrito, mas falta executar gate Qt/Windows e validar cotas reais; o cálculo conservador de bounds deve ser corrigido ou documentado antes de usar dimensões do SVG como cota final.
- **SVG import:** cobertura de comandos vetoriais é ampla, mas a validação de listas numéricas e os efeitos no root são bloqueios de confiabilidade a tratar antes de classificar como importador robusto.
- **LBRN2:** manter o importador poligonal legado separado do importador nativo experimental. Não fazer substituição automática até homologar grupos, camadas, orientação, fixture real e roundtrip v5.
- Não alterar os parsers sem uma branch de correção isolada, testes de regressão reproduzíveis e build Qt. Não integrar à main nesta etapa.

**Nenhum código funcional, CMake, workflow, PR, branch principal, arquivo local ou configuração de hardware foi alterado.**

### 12.1. Conferência dos testes SVG embutidos na janela Qt

A leitura dos blocos svgExport20zcTest() e svgImport20zeTest() em qt6_test6.cpp confirma que há testes executáveis, não apenas documentação:

- Exportação: cria desenho com engrenagem exterior/furo e uma Bézier aberta; verifica namespace XML, mm, path, C/Z, integridade XML, memória de pasta em QSettings isolado, extensão incorreta e invariância de estado/histórico.
- Importação: exporta e reimporta um SVG gerado pelo próprio aplicativo; verifica três contornos, anéis da engrenagem, Bézier e handles, dimensões físicas, drag-and-drop, append sem substituir desenho existente, Undo/Redo em lote e hash do arquivo-fonte inalterado.
- Fixture adicional cobre grupo com translate, retângulo, círculo e arco A; fixture negativa cobre rejeição de texto SVG.
- Os testes gravam JSON de resultado e usam QTemporaryDir e variáveis de ambiente isoladas para evitar alterar as preferências reais.

Limites confirmados no código dos testes:
- O roundtrip principal é gerado pelo próprio exportador; não é fixture independente de terceiros.
- A fixture adicional não cobre matrix, rotate, skew, preserveAspectRatio não padrão, comandos S/T, clip/mask/filter no root, listas numéricas malformadas, unidades q/pc ou SVGs grandes.
- O teste negativo cobre text, mas não todos os elementos não suportados nem os casos de parse parcial.
- O teste não verifica cotas de uma curva Bézier cujo ponto extremo real seja diferente do envelope dos pontos de controle.

A existência desses testes melhora a confiança na integração básica, mas não elimina as lacunas da matriz anterior. A execução Qt/Windows permanece pendente até haver log/artefato do SHA exato da branch.
