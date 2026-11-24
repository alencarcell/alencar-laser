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
