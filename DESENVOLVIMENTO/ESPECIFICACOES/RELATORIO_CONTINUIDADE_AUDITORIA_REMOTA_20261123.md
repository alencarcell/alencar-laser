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


## 13. Auditoria de seleção, transformação, alinhamento, tesoura e histórico

**Escopo:** leitura estática dos fontes e testes remotos da branch. Nenhum executável foi compilado ou rodado nesta etapa; resultados abaixo descrevem o que os testes codificam, não uma execução confirmada.

### 13.1 Transformação de curvas Bézier nativas

Arquivos:
- `DESENVOLVIMENTO/FONTE/src/bezier_native.hpp`
- `DESENVOLVIMENTO/FONTE/src/bezier_interaction.hpp`
- `DESENVOLVIMENTO/FONTE/tests/native_transform_19_8_standalone.cpp`

O teste standalone declara cenários para:
- escala não uniforme em torno de um pivô;
- transformação conjunta de âncoras e handles de entrada/saída;
- preservação de IDs, quantidade de nós e `edgeKind`;
- rotação, deslocamento e preservação da seleção;
- Undo/Redo exato e rejeição de escala zero sem acrescentar estado ao histórico.

**Limite:** o teste apresentado transforma um contorno nativo selecionado; não prova todas as combinações de seleção múltipla, objetos bloqueados/ocultos, grupos mistos, texto editável e objetos legados na interface Qt. A validação é de núcleo standalone, não de mouse/GUI.

### 13.2 Alinhamento, centralização e espelhamento

Arquivos:
- `DESENVOLVIMENTO/FONTE/src/editor_tools.hpp`
- `DESENVOLVIMENTO/FONTE/tests/alignment_mirror_19_8_standalone.cpp`
- `DESENVOLVIMENTO/FONTE/tests/text_alignment_19_8_standalone.cpp`

A cobertura standalone declara:
- alinhamento de centros de dois objetos;
- centralização do conjunto selecionado no centro da área de trabalho;
- preservação de desenho não selecionado;
- espelhamento em torno de eixo calculado a partir dos limites atuais;
- Undo em uma transação para espelhamento;
- alinhamento por centro X/Y e bordas esquerda/direita;
- alinhamento de grupo como unidade, preservando deslocamentos relativos;
- metadados de texto editável e compatibilidade com metadados legados sem a chave `editableTexts`.

**Limites:** a suíte não equivale a validação visual de alças, indicadores, caixas delimitadoras e comandos de alinhamento na janela Qt. O teste de centralização usa coordenadas sintéticas e centro fixo (200,200); não comprova que o valor venha corretamente da dimensão real da máquina ou do workspace configurado. A área de trabalho e o alinhamento permanecem operações editoriais; não representam movimento físico.

### 13.3 Tesoura em vetores lineares e curvas nativas

Arquivos:
- `DESENVOLVIMENTO/FONTE/src/scissors_span.hpp`
- `DESENVOLVIMENTO/FONTE/src/bezier_interaction.hpp`
- `DESENVOLVIMENTO/FONTE/tests/native_scissors_realcut_19_8_standalone.cpp`
- `DESENVOLVIMENTO/FONTE/tests/test19_7_scissors_layers.cpp`
- `DESENVOLVIMENTO/FONTE/tests/test19_8_scissors_span.cpp`

A tesoura linear usa planejamento de trechos delimitados por cruzamentos, índice espacial BVH e cache por contorno. O código explicita recusas para arestas degeneradas, sobreposição colinear e contatos ambíguos; também trata costura duplicada de contorno legado como fechamento lógico sem reescrever os dados originais. Há regras específicas para cruzamentos transversais em vértices e pontas de caminhos abertos.

A tesoura nativa Bézier:
- aproxima as cúbicas adaptativamente para localizar candidatos;
- refina parâmetros de interseção com iterações numéricas;
- calcula um trecho entre cruzamentos;
- remove efetivamente a geometria selecionada, mantendo remanescentes independentes;
- contempla Undo/Redo, contorno aberto/fechado e remoção de aresta cúbica única.

O teste standalone inclui fixtures para corte de uma cúbica, corte interno de caminho aberto, abertura de anel fechado, interseção entre duas cúbicas, trecho delimitado por dois cruzamentos e recusa de cenário com apenas um cruzamento.

**Riscos e limites identificados por leitura:**
- A interseção Bézier é numérica e baseada em amostragem adaptativa seguida de refinamento. Não é um solucionador analítico exato; tangências, contatos quase paralelos, interseções múltiplas muito próximas e segmentos degenerados merecem fixtures adversariais próprias.
- O teste apresentado confirma uma interseção transversal simples e um trecho reto delimitado por duas linhas. Não demonstra cobertura exaustiva de tangências, cúbicas quase coincidentes, múltiplas raízes na mesma dupla de curvas ou tolerâncias extremas.
- A política de recusa de contatos ambíguos é uma proteção deliberada contra corte destrutivo, mas precisa ser conferida na GUI para que o usuário receba indicação clara da recusa e não interprete ausência de corte como falha silenciosa.
- Não há evidência de teste físico ou emissão de comandos; a tesoura é edição geométrica offline.

### 13.4 Histórico e custo de memória

Arquivo:
- `DESENVOLVIMENTO/FONTE/tests/history_memory_19_9_standalone.cpp`

O teste standalone cria um contorno com 4.000 nós e declara verificações para:
- contabilização de nós correntes, Undo e Redo;
- deslocamento, Undo e Redo;
- limpeza da pilha Redo após nova edição;
- limite de 40 estados de Undo;
- estimativa de nós retidos no histórico.

O próprio `editor_tools.hpp` documenta `HistoryNodeFootprint` como estimativa do payload dos arrays de nós, não como medição de RSS ou memória total do processo. Portanto, `undoNodes` e `undoNodeBytes` não devem ser apresentados como consumo total de RAM. O limite de 40 estados reduz retenção, mas um documento muito grande pode continuar gerando histórico substancial.

**Lacuna:** não foi encontrada nesta amostra uma medição de RSS/heap sob edição repetida de documentos grandes com geometrias mistas, curvas Bézier, metadados, imagens e projetos persistidos. O teste de 4.000 nós é uma regressão sintética, não benchmark de produção.

### 13.5 Seleção e integração Qt

Arquivos:
- `DESENVOLVIMENTO/FONTE/qt/qt6_test15_integrated.cpp`
- `DESENVOLVIMENTO/FONTE/qt/qt6_test16_persistence.cpp`
- `DESENVOLVIMENTO/FONTE/qt/qt6_test17_mission.cpp`
- `DESENVOLVIMENTO/FONTE/qt/qt6_test6.cpp`

O teste 15 declara eventos Qt para inserção por Shift+clique, Undo/Redo, arraste de handle, preview sem mutação do estado confirmado, cancelamento por Escape, Delete e salvamento/reabertura v5.

O teste 17 é a cobertura mais ampla de interação encontrada nesta etapa: declara cliques reais Qt em subpaths Bézier sobrepostos, ciclo por Alt, tolerância de seleção em zooms 1/6/24/200, inserção no subpath selecionado, arraste de âncora e handles, Escape, Delete, Undo/Redo, cliques fora da geometria e persistência com objetos legados e curvas nativas. Também compara estado antes/depois de salvar e reabrir e verifica que arquivos inválidos/schema futuro não danificam o projeto aberto.

**Lacuna central:** o cenário de interação do teste 17 constrói dois contornos Bézier nativos sobrepostos para testar seleção de subpath. A etapa de persistência também mantém geometrias legadas e nativas no mesmo projeto, mas isso não prova que uma única operação de seleção/transformação da GUI consiga selecionar e transformar simultaneamente os dois tipos com comportamento coerente. Essa capacidade deve permanecer como **NÃO COMPROVADA** até existir fixture explícita de seleção mista e eventos Qt sobre objetos legados + Bézier na mesma ação.

Também não foi comprovado nesta auditoria:
- comportamento visual em diferentes DPRs/monitores Windows;
- seleção por janela/laço em objetos mistos;
- seleção e transformação de grupos contendo geometrias legadas e nativas;
- interação com camadas ocultas/bloqueadas durante seleção mista;
- uso com documentos grandes e resposta de interface sob carga.

### 13.6 Matriz de evidência desta etapa

| Área | Cobertura declarada nos fontes/testes | Estado desta auditoria |
|---|---|---|
| Transformação Bézier nativa | Escala, rotação, deslocamento, handles, Undo/Redo | Fonte/teste revisados; execução não verificada |
| Alinhamento/espelhamento | Centro, bordas, grupo, eixo dinâmico, Undo | Fonte/teste revisados; GUI não comprovada |
| Tesoura linear | Cruzamentos, trechos, casos ambíguos e costura legada | Fonte/teste revisados; execução não verificada |
| Tesoura Bézier | Interseção, corte real, remanescentes, Undo/Redo | Fonte/teste revisados; casos adversariais incompletos |
| Histórico | Limite de 40 estados e contagem de payload de nós | Fonte/teste revisados; RSS total não medido |
| Interação Qt | Zoom, subpaths sobrepostos, handles, Escape, Delete | Teste codificado; execução Windows não verificada |
| Seleção mista legado + Bézier | Persistência conjunta existe; seleção/transformação simultânea não demonstrada | **NÃO COMPROVADA** |

### 13.7 Próximas verificações prioritárias

1. Criar fixture Qt com pelo menos um objeto legado e uma curva Bézier nativa, selecionar ambos na mesma ação e verificar movimento, alinhamento, escala, rotação, Undo/Redo e persistência v5.
2. Acrescentar casos adversariais da tesoura Bézier: tangência, quase paralelismo, duas ou mais interseções próximas, cúbicas coincidentes/parcialmente sobrepostas, cruzamento em âncora e tolerâncias pequenas/grandes.
3. Validar no Qt a apresentação de recusas da tesoura e seleção de itens ocultos/bloqueados.
4. Medir RSS/heap em execução Windows com projeto misto de grande porte e histórico no limite; separar payload estimado de memória real.
5. Executar os alvos standalone e Qt no ambiente Windows do projeto, registrar comando, SHA do executável, logs completos e fixtures; não conectar máquina laser.

**Conclusão da seção:** a branch contém uma base de testes substancial para edição geométrica, tesoura e curvas nativas, incluindo eventos Qt codificados. O principal bloqueio funcional ainda sem evidência é a seleção/transformação simultânea de geometria legada e Bézier nativa. A auditoria permanece estática: nenhum PASS de execução, build ou homologação é atribuído nesta seção.


### 13.8 Complemento após inspeção aprofundada: seleção mista já possui implementação e fixture Qt

A leitura adicional de `qt6_test6.cpp` revelou uma cobertura que não estava visível nos testes 15–17 revisados inicialmente e que precisa qualificar a conclusão das seções 13.5–13.7:

- `combinedSelectionBounds()` une os limites dos objetos convencionais e das curvas nativas selecionadas;
- `selectAllMixed()` seleciona objetos convencionais editáveis e todos os contornos Bézier nativos, mantendo os dois modelos selecionados simultaneamente;
- `beginSelectionTransform()` reúne os itens gráficos dos dois modelos em um mesmo quadro/preview;
- `finishSelectionTransform()` confirma a transformação nos dois modelos; se a operação convencional falhar após o commit nativo, tenta desfazer a alteração nativa;
- `history()` possui caminho específico para Undo/Redo coordenado, verificando as gerações/contagens de histórico e revertendo o primeiro lado se o segundo lado não puder acompanhar;
- o atalho Ctrl+A chama `selectAllMixed()`.

Foi localizada uma fixture Qt no próprio `qt6_test6.cpp` que:
1. importa duas curvas nativas LightBurn;
2. acrescenta um retângulo convencional ao mesmo documento;
3. seleciona contornos nativos individualmente e em conjunto;
4. chama `selectAllMixed()` e também envia Ctrl+A como evento Qt;
5. verifica que o quadro de seleção contém o retângulo e uma curva nativa;
6. arrasta um handle compartilhado e confirma o deslocamento de ambos os modelos;
7. usa um único Ctrl+Z e Ctrl+Y para restaurar/refazer os dois lados;
8. verifica roundtrip v5 e preservação de metadados de camada não verificados.

**Conclusão corrigida:** seleção mista e transformação translacional coordenada não são apenas intenção de código; há implementação explícita e uma fixture de integração Qt que codifica esse cenário. A classificação correta é **IMPLEMENTADO + TESTE Qt CODIFICADO; EXECUÇÃO NÃO VERIFICADA NESTA AUDITORIA**. A lacuna remanescente não é ausência total de cobertura mista, mas cobertura incompleta: a fixture encontrada demonstra Ctrl+A, quadro combinado, arraste translacional e Undo/Redo; não demonstra claramente escala e rotação mistas, seleção aditiva manual de formas convencionais e Bézier, alinhamento misto por comandos, nem grupos mistos em todas as combinações.

A fixture pertence ao arquivo monolítico `qt6_test6.cpp`; o modo exato de execução e sua associação a um alvo dedicado devem ser conferidos no CMake e no dispatcher antes de afirmar que ela roda automaticamente. A existência do código de teste não comprova que tenha sido compilado ou executado na máquina Windows.

**Prioridade atualizada:** executar o cenário misto existente com o alvo/modo correto; depois acrescentar escala, rotação e alinhamento mistos, mais casos de falha/rollback entre os dois históricos. Não duplicar uma fixture que já existe.


## 14. Auditoria complementar: transformações mistas, alinhamento GUI e casos-limite da tesoura

**Escopo:** inspeção estática adicional de `qt6_test6.cpp`, `editor_tools.hpp` e testes 19.8. Sem build ou execução nesta etapa.

### 14.1 Transformação mista: implementação mais ampla que a fixture

A implementação de `finishSelectionTransform()` usa os mesmos parâmetros de transformação para os dois modelos:
- deslocamento: `editor.moveObjects()` + `nativeSession_->transformSelectedContours()`;
- rotação: `editor.rotateObjects()` + transformação das curvas nativas;
- escala/afim: `editor.affineObjects()` + transformação das curvas nativas.

O cálculo do preview também trabalha sobre um quadro comum. O caminho de rollback tenta desfazer o lado nativo se o modelo convencional rejeitar a operação. O histórico misto verifica geração e contagens Undo/Redo dos dois modelos antes de aplicar a operação coordenada.

**Cobertura comprovada por leitura:** a fixture Qt encontrada na seção 13.8 verifica arraste translacional e Undo/Redo conjunto. A implementação contempla escala e rotação, mas não encontrei na fixture revisada uma verificação equivalente de escala/rotação mistas, nem testes explícitos que provoquem uma falha real de commit no segundo modelo para validar o rollback.

**Risco residual:** os dois modelos mantêm históricos próprios, coordenados por estado/contagens e uma flag de última ação mista. Isso é uma estratégia deliberada, mas exige testes de sequência: edição externa em um dos modelos após a ação mista, múltiplos Undo/Redo, nova edição após Undo, falha parcial e troca de seleção/documento. A lógica de invalidação reduz risco de desfazer a pilha errada, mas não substitui essas sequências de integração.

### 14.2 Alinhamento GUI e centralização na mesa

O núcleo `Editor::alignObjects()` opera sobre objetos convencionais do `EditorState`, expande grupos como unidades indivisíveis e cria uma única transação de Undo. Modos documentados: esquerda, direita, topo, base, centro X, centro Y e ambos os centros. A referência é o último objeto selecionado.

A barra de ações rápidas expõe os seis alinhamentos direcionais/centros e o alinhamento dos dois centros. Já a ação **Centralizar na mesa física** contém uma guarda explícita que recusa a operação quando `bezierToolActive_` está ativo, informando ao usuário que, nesta beta, deve selecionar polilinhas em vez de curvas Bézier nativas.

**Classificação correta:**
- Alinhamento de objetos convencionais/grupos: implementado no núcleo, com teste standalone; GUI/execução ainda não homologadas nesta auditoria.
- Alinhamento de seleção mista convencional + Bézier: **não demonstrado**; a API `alignObjects()` recebe IDs do modelo convencional e não inclui chaves do documento nativo.
- Centralização na mesa para Bézier nativa: **bloqueada intencionalmente pela interface beta**, não deve ser anunciada como suportada.
- Centralização na mesa para seleção mista: não suportada/demonstrada.

Não se deve remover a guarda de segurança apenas para uniformizar a interface. A evolução correta é implementar uma operação transacional equivalente nos dois modelos, com Undo/Redo coordenado e testes de preservação do centro da mesa configurada, sem alterar dimensões nem enviar comandos ao hardware.

### 14.3 Tesoura: cobertura adversarial existente

A revisão de `tests/test19_8_scissors_span.cpp` encontrou cenários específicos já codificados:
- cruzamento transversal em vértice de caminho estrangeiro;
- cruzamento transversal no próprio vértice do caminho-alvo;
- tangência em vértice recusada com motivo `vertex_or_tangent`;
- sobreposição colinear recusada com motivo `collinear_overlap`;
- contato em T tratado como limite terminal;
- quase contato (near miss) não promovido a interseção;
- cortes sucessivos nos remanescentes, preservando linhas-guia;
- Undo em cadeia restaurando a geometria original;
- soldagem opcional de extremidades com uma única etapa de Undo.

A integração Qt da tesoura também:
- não recorre a apagar uma aresta Bézier inteira quando não encontra trecho seguro;
- exige um intervalo delimitado por dois cruzamentos para o corte K nativo;
- mantém a geometria se o intervalo ficar inválido antes do commit;
- apresenta mensagens distintas para sobreposição, tangência/vértice, aresta degenerada, contorno fechado com uma única interseção e cursor perto do limite;
- informa que nada foi removido quando não existe intervalo seguro.

O teste standalone `native_scissors_realcut_19_8_standalone.cpp` confirma no código de teste interseção cúbica refinada, corte do intervalo e Undo/Redo. Entretanto, o arquivo é curto e não contém uma bateria adversarial Bézier comparável à bateria linear de `test19_8_scissors_span.cpp`.

**Lacuna prioritária da tesoura Bézier:** acrescentar fixtures para tangência entre cúbicas, quase tangência, cúbicas coincidentes/parcialmente sobrepostas, múltiplas interseções próximas na mesma dupla de curvas, interseção em âncora compartilhada e estabilidade sob diferentes tolerâncias/escala. Não afirmar que esses casos falham; eles simplesmente não estão comprovados pela amostra de teste revisada.

### 14.4 Matriz de estado revisada

| Recurso | Implementação encontrada | Teste codificado | Estado de execução |
|---|---|---|---|
| Transformação mista por deslocamento | Sim | Fixture Qt com ambos os modelos | Não executado nesta auditoria |
| Transformação mista por escala/rotação | Sim, no mesmo commit da transformação | Não localizado teste específico equivalente | Não comprovado |
| Undo/Redo misto | Sim, coordenação dos dois históricos | Fixture Qt para deslocamento | Não executado nesta auditoria |
| Alinhamento de objetos convencionais/grupos | Sim | Standalone | Não executado nesta auditoria |
| Alinhamento misto convencional + Bézier | Não demonstrado | Não localizado | Não comprovado |
| Centralizar Bézier na mesa | Bloqueado explicitamente na beta | Guarda de interface | Restrição intencional |
| Tesoura linear: vértice, tangência, T, overlap, near miss | Sim | Bateria adversarial standalone | Não executado nesta auditoria |
| Tesoura Bézier: corte/Undo básico | Sim | Standalone e integração Qt codificados | Não executado nesta auditoria |
| Tesoura Bézier: tangência/overlap/múltiplas raízes | Não comprovado na suíte revisada | Lacuna de fixtures | Não comprovado |

### 14.5 Próxima sequência recomendada

1. Rodar o executável Qt correto com a fixture de seleção mista já existente e registrar SHA, comando, Qt, ambiente e resultado JSON.
2. Estender a fixture existente para escala e rotação mistas, Undo/Redo em sequência e rollback provocado por falha controlada.
3. Implementar alinhamento misto somente após definir uma transação coordenada entre Editor e CurveDocument; manter a guarda atual até então.
4. Completar a bateria adversarial da tesoura Bézier antes de alterar o solucionador de interseção.
5. Separar o alvo de teste Qt de integração do monólito `qt6_test6.cpp) quando houver branch de integração apropriada; não alterar CMake/workflows nesta auditoria sem build Windows reproduzível.

**Conclusão:** a tesoura linear tem uma suíte adversarial mais madura que a tesoura Bézier. A transformação mista está implementada para deslocamento, escala e rotação, mas sua prova Qt encontrada é principalmente translacional. Alinhamento misto não está demonstrado e centralização de Bézier na mesa é uma restrição beta explícita. Nenhum resultado de execução ou homologação foi inferido.


## 15. Revisão numérica do detector de interseções Bézier usado pela tesoura K

**Arquivo:** `DESENVOLVIMENTO/FONTE/src/bezier_interaction.hpp`  
**Escopo:** análise estática do algoritmo entre as linhas de amostragem, interseção e planejamento de intervalo. Não houve execução numérica nesta etapa.

### 15.1 Pipeline observado

1. Cada cúbica é subdividida recursivamente por De Casteljau (`sampleCubicRec`) até que a distância dos controles à corda seja menor que a tolerância, ou até profundidade 24.
2. `findCubicIntersections()` rejeita primeiro pares cujas caixas dos quatro pontos de controle não se sobrepõem.
3. Cada curva é amostrada com tolerância `intersectionToleranceMm * 0.25` (padrão de interseção: 0,01 mm; amostragem: 0,0025 mm).
4. Os segmentos aproximados são cruzados em pares; segmentos quase paralelos com determinante absoluto abaixo de `1e-14` não geram candidato.
5. Um candidato é refinado por até 10 iterações de Newton em parâmetros `ta/tb`, com derivadas das cúbicas.
6. O par é aceito se a distância entre os pontos refinados for menor ou igual à tolerância; duplicatas próximas em parâmetros são removidas.
7. O planejador ignora interseções muito próximas dos extremos das arestas (`endpointEpsilon=1e-5`), ordena os cruzamentos ao longo do contorno e escolhe os dois limites em torno do cursor, respeitando a tolerância de proximidade de 0,25 mm.

### 15.2 Proteções e decisões conservadoras

- O corte não é executado se não houver pelo menos dois cruzamentos utilizáveis.
- O cursor próximo de um limite é recusado, evitando selecionar um intervalo instável.
- Interseções em extremos de arestas não são usadas como limites Bézier nativos neste planejador.
- A geometria cortada é reconstruída por subdivisão cúbica exata, não por exportação de segmentos lineares.
- As arestas retas são convertidas para representação cúbica linear antes do cálculo, para manter parâmetro consistente com `splitEdge()`.

Essas decisões favorecem preservar a geometria em casos ambíguos, mesmo que isso signifique não oferecer corte em alguns contatos.

### 15.3 Riscos numéricos a validar, sem declarar defeito

- **Tangência e quase tangência:** o cruzamento inicial depende de segmentos amostrados com determinante não quase nulo. Tangências exatas ou quase paralelas podem não produzir candidato inicial, antes mesmo do refinamento Newton. A política segura é não cortar sem limites confiáveis, mas o usuário pode perceber um corte ausente.
- **Newton sem convergência explícita:** há limite de 10 iterações e teste final de distância, mas não há critério separado de convergência em `ta/tb` nem retorno de diagnóstico da quantidade de iterações. Casos mal condicionados precisam de fixtures.
- **Múltiplas raízes próximas:** a deduplicação usa distância em parâmetro `1e-4`; testar raízes distintas mais próximas que esse limiar para verificar se não são fundidas indevidamente.
- **Parâmetros próximos às pontas:** `endpointEpsilon=1e-5` e a deduplicação podem interagir com curvas curtas ou raízes quase coincidentes com âncoras.
- **Custo:** para cada par de arestas cujas caixas globais se sobrepõem, o algoritmo compara segmentos amostrados em laço duplo. Em desenhos com muitas cúbicas espacialmente próximas, o custo pode crescer rapidamente. O BVH mencionado na tesoura linear não deve ser presumido como aplicado a este caminho nativo sem evidência específica.
- **Escala física:** as tolerâncias estão em milímetros; testar a mesma geometria em tamanhos muito pequenos/grandes e em zooms diferentes. O zoom afeta a seleção do cursor, mas não altera diretamente a tolerância geométrica de interseção.

### 15.4 Fixtures propostas antes de qualquer refatoração

1. Cruzamento transversal simples com solução conhecida e erro máximo medido.
2. Tangência exata entre duas cúbicas: confirmar recusa segura e motivo observável.
3. Quase tangência com separações graduais (por exemplo 0,1; 0,01; 0,001 mm): medir falso contato e falso negativo.
4. Duas raízes próximas em uma mesma dupla de curvas, incluindo distância paramétrica menor e maior que `1e-4`.
5. Cúbicas coincidentes e parcialmente sobrepostas: garantir recusa, nunca corte arbitrário.
6. Interseção em âncora/limite entre arestas adjacentes: verificar política de endpoint e ausência de duplicatas.
7. Mesmo conjunto geométrico escalado por fatores 0,01, 1 e 100, mantendo resultado topológico esperado.
8. Documento com 10, 100, 500 e 1.000 arestas Bézier próximas: registrar tempo, número de amostras e pares de segmentos avaliados.

**Conclusão:** o detector possui uma cadeia clara de aproximação adaptativa, refinamento Newton e validação residual, com política conservadora para não realizar cortes sem dois limites. A principal lacuna é evidência adversarial e de desempenho para casos Bézier mal condicionados. Não se recomenda alterar tolerâncias ou o algoritmo antes de adicionar fixtures mensuráveis e executar a suíte offline.
