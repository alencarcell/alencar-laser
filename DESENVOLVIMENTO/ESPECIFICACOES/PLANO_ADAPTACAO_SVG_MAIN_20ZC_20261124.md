# Plano de adaptação SVG para a arquitetura main — 20zc/20ze

Data: 24/11/2026

## 1. Objetivo

Portar a importação e exportação SVG aprovadas nos testes Windows da branch de desenvolvimento para a arquitetura atualmente existente em `main`, sem trazer a branch de memória de pastas, sem alterar o formato de persistência sem migração explícita e sem habilitar fabricação/GRBL.

Este documento é um plano de implementação e validação. Não declara código portado nem homologação da main.

## 2. Base e evidências

- Base da branch: `main` em `c14008afd0705cc91449b683162e6c1430566cf0`.
- Branch isolada: `feat/svg-main-adaptation-20zc-20261124`.
- Testes executados pelo usuário em Windows na branch mais nova: compilação do alvo `alencar_test7_qt6`, `--svg-export-20zc-test` e `--svg-import-20ze-test` retornaram sucesso.
- Esses resultados não representam build ou teste desta branch baseada em main.

Arquivos de arquitetura inspecionados em main:
- `src/bezier_native.hpp`: `CurveContour` contém `key`, `nodes`, `edgeKind`, `closed`; não contém `layer`.
- `src/editor_tools.hpp`: `EditorState.objects` associa ID de objeto a `Meta`; `Meta.layer` aponta para `EditorState.layers`.
- `src/bezier_package.hpp`: codec de curvas serializa chave do objeto/subpath, nós, arestas e fechamento; não serializa camada na curva.
- `src/integrated_project_edit.hpp`: preserva metadados convencionais e mantém `CurveDocument` em campo separado.
- `qt/qt6_test6.cpp`: renderização de geometrias convencionais consulta camada pelo metadata do objeto; curvas nativas são mantidas separadamente em `CurveDragSession`.

## 3. Contrato proposto — não implementar sem testes

### 3.1 Associação de camada

Não adicionar `CurveContour.layer` nesta adaptação inicial.

Usar `CurveContour.key.object` como identidade do objeto e consultar:
`EditorState.objects[objectId].layer` → `EditorState.layers[layerId]`.

Regras:
- Toda curva importada precisa ter objeto registrado em `EditorState.objects`.
- A camada destino inicial deve ser escolhida explicitamente pela interface/ação de importação; fallback silencioso para camada 0 não é permitido quando a camada pedida não existe, está oculta ou bloqueada.
- Exportação deve falhar com erro explícito se uma curva não tiver metadata ou apontar para camada inexistente.
- Camada invisível deve ser tratada de modo consistente com o comportamento convencional; camada bloqueada não deve impedir exportação, pois bloqueio de edição não é invisibilidade.
- A importação não deve sobrescrever metadata de objetos já existentes.

### 3.2 Identificadores e colisões

- O importador deve receber o estado/documento existente ou um alocador de IDs fornecido pela janela; não pode iniciar IDs de objeto em zero sem verificar colisão.
- Reservar IDs de objeto de forma determinística acima do maior ID existente, verificando overflow e chaves repetidas.
- Preservar a distinção entre `Key.object` e `Key.sub`; cada subpath precisa de chave única no documento combinado.
- Importação repetida do mesmo SVG deve criar novos objetos, salvo se a operação explicitamente substituir uma seleção/documento.
- Falha em qualquer contorno deve ser transacional: nenhum fragmento de importação pode ser publicado.

### 3.3 Undo/Redo e histórico

- Importação de múltiplos contornos deve constituir uma única operação atômica de histórico, ou uma operação composta equivalente.
- Undo deve remover geometria e metadata dos objetos importados em conjunto.
- Redo deve restaurar os mesmos IDs/chaves e metadata.
- Não pode deixar metadata órfã nem geometria sem metadata.
- A integração com histórico convencional e `CurveHistory` precisa ser explicitamente testada; não assumir que os dois históricos já são coordenados nesta base.

### 3.4 Persistência

- Antes de alterar o codec, decidir se a camada nativa será derivada de `key.object` e persistida no metadata de objetos, ou se exigirá novo campo/schema versionado.
- A estratégia derivada só é válida se o pacote salvar e reabrir os registros de objeto para cada curva nativa, sem colisão com objetos poligonais.
- Se o formato atual não conseguir preservar a associação de maneira inequívoca, não implementar fallback; propor migração versionada com compatibilidade v4/v5 e fixtures de roundtrip.
- Não sobrescrever projetos reais durante testes. Usar diretório temporário e fixtures descartáveis.
- Manter `fabricationEnabled=false`; SVG não contém potência, velocidade, operações CAM ou G-code.

## 4. Plano de testes obrigatório

1. Importar SVG simples em documento vazio e verificar IDs, metadata e camada escolhida.
2. Importar SVG em documento com polilinhas e curvas nativas; verificar ausência de colisões de objeto/subpath.
3. Importar SVG duas vezes; confirmar IDs distintos e contagens esperadas.
4. Testar camada válida, inexistente, oculta e bloqueada.
5. Forçar SVG inválido após um elemento válido; confirmar rollback integral.
6. Undo/Redo da importação em lote; comparar geometria e metadata antes/depois.
7. Exportar com camada visível e invisível; confirmar filtragem correta.
8. Rejeitar curva sem metadata ou com referência a camada inexistente.
9. Salvar/reabrir projeto v5 com curvas importadas em camada não padrão; comparar IDs, camada e geometria.
10. Testar compatibilidade de abertura de fixtures v4 e v5 existentes.
11. Roundtrip SVG→documento→SVG; comparar geometria e tolerância numérica, não bytes do arquivo.
12. Executar os testes standalone e Qt offscreen; depois build Windows desta branch. Não conectar hardware.

## 5. Critérios de aceite

- Compila em build isolado desta branch baseada em main.
- Todos os testes obrigatórios passam no Windows.
- Nenhuma alteração nos projetos reais, builds preexistentes ou `main`.
- Persistência mantém a associação de camada após save/open.
- Histórico não deixa estados parciais ou metadata órfã.
- Exportação não depende de campo inexistente em `CurveContour`.
- PR pequena, com diff restrito a módulos SVG, integração estritamente necessária, testes e documentação.
- Nenhuma operação de laser/GRBL/G-code é executada.

## 6. Estado

| Item | Estado |
|---|---|
| Branch isolada criada a partir de main | PASS |
| Contrato arquitetural documentado | PASS |
| Código SVG adaptado | NOT STARTED |
| Testes de associação/IDs/histórico | NOT STARTED |
| Persistência roundtrip | NOT STARTED |
| Build/testes Windows nesta branch | NOT TESTED |
| PR ou merge para main | NÃO REALIZADO |

## 7. Regra de segurança

Não copiar `qt6_test6.cpp` da branch maior para esta branch. Não adicionar `layer` a `CurveContour` sem migração e roundtrip. Não alterar `main`, não mesclar a branch de memória de pastas e não usar arquivos locais não rastreados como fonte sem inspeção explícita.


## 8. Caracterização do contrato de metadados — 24/11/2026

Foi adicionado `tests/svg_main_layer_contract.cpp` e o alvo CMake `alencar_test_svg_main_layer_contract`.

O teste constrói um `EditorState` válido com um objeto poligonal na camada 0 e um `CurveDocument` separado com uma curva Bézier nativa. Em seguida:

1. Registra em `EditorState.objects` um metadata para um ID usado somente pela curva Bézier. A validação atual rejeita o estado porque existe metadata sem objeto poligonal correspondente.
2. Reutiliza o ID do objeto poligonal para a curva nativa. Os dois modelos aceitam o mesmo número, mas isso representa alias da metadata poligonal, não uma associação independente para a curva.

Isso refuta a hipótese simplista de registrar um objeto Bézier-only em `EditorState.objects` sem alterar o contrato atual de `validate(EditorState)`. Também demonstra que compartilhar um ID numérico com uma polilinha não cria identidade independente.

**Resultado:** teste de caracterização codificado; ainda não compilado nem executado nesta branch. Não é teste de importação SVG e não altera o formato de persistência.

### Decisão arquitetural atualizada

Não implementar a associação via `EditorState.objects` como se fosse direta. Avaliar um registro explícito de vínculo para curvas nativas, com identidade e camada próprias, persistido em campo versionado do pacote ou estrutura canônica equivalente. Se isso exigir evolução de schema, documentar migração e compatibilidade antes de tocar no codec. Não usar `curveSource`/CSV de amostras como armazenamento incidental de metadata.

Estado do teste:
- Fonte adicionada: PASS (publicada na branch).
- Alvo CMake adicionado com include explícito de Boost.JSON: PASS (publicado na branch).
- Compilação: NOT TESTED.
- Execução: NOT TESTED.
- Alteração em `main`: nenhuma.


## 9. Contrato independente de camada para curvas nativas — 24/11/2026

Criado `src/curve_layer_bindings.hpp` com `CurveLayerBindings`, indexado por `Key{object,subpath}`, sem inserir IDs de curvas em `EditorState.objects`. A camada é resolvida contra `EditorState.layers`, enquanto a identidade do contorno continua pertencendo a `CurveDocument`.

O contrato implementado exige:
- exatamente um vínculo por contorno Bézier;
- nenhuma referência a contorno inexistente;
- toda camada referenciada deve existir no editor;
- chaves duplicadas no JSON são recusadas;
- serialização JSON explícita de `objectId`, `subpathId` e `layerId`.

Criado `tests/curve_layer_bindings_test.cpp` e alvo `alencar_test_curve_layer_bindings`, incluindo roundtrip JSON e casos negativos para vínculo ausente, camada desconhecida, vínculo órfão e chave duplicada.

**Limite importante:** isto ainda é um contrato isolado, não integração completa de produto. O pacote v5 atual não lê nem grava esse campo; `editorStateForIntegratedProject()` também não carrega os vínculos. A próxima etapa deve incorporar o array como campo adicional preservável em `legacy.layers` (schema `editor-meta-1`) e conectar o ciclo de abrir/salvar sem descartar vínculos, mantendo compatibilidade v4/v5. A função `preserveEditorMetadata()` já conserva campos adicionais que não são sobrescritos por `metadata(edited)`, mas isso, sozinho, não materializa nem valida os vínculos na abertura.

### Estado
- Header do contrato: publicado.
- Teste de roundtrip e rejeições: publicado.
- Alvo CMake: publicado, vincula `boost_json_impl.cpp`.
- Compilação/execução: NOT TESTED nesta sessão.
- GUI/importação SVG/persistência `.alaser`: NOT TESTED.
- `main`: sem alterações.


## 10. Persistência dos vínculos Bézier no pacote v5 — 24/11/2026

Implementada uma primeira integração do contrato de camadas com os metadados do projeto:

- `integrated_project_edit.hpp` agora oferece `preserveCurveLayerBindings()`, `curveLayerBindingsForIntegratedProject()` e `buildEditedIntegratedProjectWithCurveLayers()`.
- Os vínculos são armazenados no objeto `legacy.layers` sob a chave `nativeCurveLayerBindings`, mantendo `schema=editor-meta-1`.
- A identidade é a chave composta `objectId/subpathId`; `layerId` é validado contra as camadas do `EditorState`.
- Projeto v5 com curvas nativas mas sem a chave de vínculos é rejeitado explicitamente. Projeto v4 não ganha vínculos implicitamente.
- O codec de ZIP não foi alterado: o campo está no membro `design/layers.json`, coberto pelo digest existente do pacote v5.

Adicionado `tests/curve_layer_persistence_test.cpp` e alvo CMake `alencar_test_curve_layer_persistence`. O teste cria estado e curva em memória, grava pacote `.alaser` em arquivo temporário exclusivo, reabre, valida a camada e verifica rejeição de vínculo ausente. O teste remove apenas o arquivo temporário que ele próprio cria.

### Limitações e riscos restantes
- O teste ainda não foi compilado/executado; revisão remota não confirma compatibilidade de compilação.
- A GUI ainda não carrega/atualiza `CurveLayerBindings` durante importação, edição, exclusão ou Undo/Redo.
- A chamada existente `buildEditedIntegratedProject()` permanece disponível e não injeta vínculos; a integração de produto deve migrar para `buildEditedIntegratedProjectWithCurveLayers()` quando possuir estado válido de vínculos.
- A abertura de projeto v5 com curvas existentes e sem vínculos passa a exigir política explícita de migração/default antes de ser usada por esse novo caminho. Não atribuir camada 0 silenciosamente.
- O teste não verifica substituição de arquivo existente no Windows, já identificada como risco separado de `std::filesystem::rename`.

### Estado
| Item | Estado |
|---|---|
| Metadado de vínculo no pacote v5 | Implementado na branch |
| Validação de camada e chave composta | Implementado na branch |
| Teste de roundtrip `.alaser` | Codificado |
| CMake do teste | Adicionado |
| Build/execução Windows | NOT TESTED |
| Integração GUI/Undo/Redo | NOT IMPLEMENTED |
| Compatibilidade com projetos v5 antigos com curvas | Requer política de migração explícita |
| `main` | Intacta |
