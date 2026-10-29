# Alencar Laser 19.8 — Hotfix de seleção, edição, histórico e Tesoura de curvas nativas

**Data:** 29/10/2026  
**Branch:** `feat/refino-curvas-rastreio-diagnosticos-19-8`  
**Origem:** teste manual informado pelo usuário: ferramenta **Desenhar curva** gera a geometria corretamente e o snap magnético funciona, mas a curva depois de concluída não podia ser selecionada, editada, recortada nem desfeita.

## Causa raiz inspecionada na fonte

O aplicativo trabalha com **dois documentos de geometria diferentes** nesta candidata:

1. `Editor` / `Document`: polilinhas, formas, seleção e Tesoura comum;
2. `CurveDragSession` / `CurveDocument`: curvas Bézier v5 nativas, alças e histórico próprio.

O botão **Seleção** desabilitava `nativeMode_`, enquanto o hit-test de seleção usava exclusivamente `picker_.query(editor,...)`, invisível às curvas de `CurveDocument`. Ao desfazer uma curva, `nativeHistory` executava apenas `updateNativePaths(true)`, que não removia `QGraphicsPathItem` de contorno eliminado do documento: risco de **curva fantasma**. Os atalhos Ctrl+Z/Delete dependiam de `bezierToolActive_` — flag que volta a falso ao trocar de ferramenta — e a Tesoura só operava sobre polilinhas.

## Correções publicadas no ramo experimental

### Seleção e edição
- A ferramenta **Seleção** primeiro pesquisa os contornos Bézier nativos por geometria e tolerância em pixels. Se clicar em um, seleciona e destaca a curva, sem converter para segmentos de reta.
- Uma curva selecionada com **Seleção** pode ser arrastada normalmente para mover seu contorno completo (uma operação de histórico). Se clicar fora, o seletor nativo é liberado e o hit-test de polilinhas volta a operar.
- A ferramenta **Bézier** (edição de nós) continua independente: ao acioná-la, `nativeEditHandles_=true`, permitindo selecionar/mover âncoras e alças. Os marcadores de alças **não ficam sobrepostos na Seleção comum**.
- A ferramenta **Desenhar curva** conserva cliques/arraste já validados pelo usuário; não altera a construção de curvas.

### Desfazer/Refazer e exclusão
- `CurveDragSession::undo/redo` reconcilia a seleção: uma curva excluída pelo histórico não deixa uma chave de seleção pendurada.
- `QtEditorLab::nativeHistory` recria a coleção de `QGraphicsPathItem` depois de Undo/Redo. Assim uma forma desfeita desaparece e uma refeita volta a aparecer.
- `lastNativeAction_` + comparação com `Editor::generation` encaminham **Ctrl+Z, Ctrl+Y, Ctrl+Shift+Z e botões Desfazer/Refazer** ao histórico nativo quando ele representa a última alteração, mesmo após clicar em **Seleção**. Se uma alteração subsequente for feita no editor de polilinhas, a preferência passa para esse documento.
- **Delete** e o botão **Excluir formas selecionadas** removem o contorno Bézier quando selecionado no modo Seleção. Em **Bézier / edição de alças**, Delete continua removendo o nó escolhido, e Ctrl+Delete remove o contorno.
- **Limite de histórico nesta arquitetura:** ainda são duas pilhas de Undo distintas, não há cronologia global unificada após operações alternadas dos dois tipos; a opção de Undo prioriza o último tipo de edição observado.

### Tesoura de curvas
- Em modo Tesoura, o clique sobre uma aresta Bézier nativa usa hit-test da curva real (amostragem para localizar posição `t`) e subdivisão **de Casteljau** no documento nativo.
- Curva aberta: passa a **dois contornos abertos editáveis**, com IDs diferentes e uma única transação Undo/Redo; preserva as duas seções da cúbica matematicamente, sem rasterizar nem achatar.
- Curva fechada: abre o contorno no ponto clicado, preservando a sequência de segmentos e as alças, com um nó duplicado em nova ID na coordenada do corte.
- **Limitação:** a Tesoura Bézier desta rodada **separa no ponto**, não elimina automaticamente um intervalo entre dois cruzamentos como a Tesoura mais avançada do Editor. A escolha de um trecho completo para exclusão será fase posterior e precisa de testes geométricos próprios.

## Regressões incluídas

- `DESENVOLVIMENTO/FONTE/tests/preview_draw_join_19_8_standalone.cpp`: seleção/reseleção, subdivisão da cúbica e comparação da curva original em `t=0,25` e `t=0,75`, corte do contorno aberto, abertura do fechado, exclusão/undo e prevenção de seleção inválida.
- `qt/qt6_test6.cpp --draw-join-19-8-test`: gesto Qt criando curva, troca para Seleção, clique na própria curva, checagem de seleção, Undo removendo o item gráfico, Redo restaurando, editor de alças e clique da Tesoura dividindo o caminho. Relatório em `RESULTADO_DESENHO_UNIAO_19_8.json`.
- `SCRIPTS/TESTAR_19_8_WINDOWS.ps1 -SomenteDesenho`: o gate de desenho **já existente** executa os testes C++20 e Qt6 alterados. Não precisa de novo comando.
- `.github/workflows/preview-19-8-offline.yml`: inclui o teste C++ portátil; execução do CI e Qt6 Windows desta correção ainda **não confirmadas**.

## Estado e roteiro de teste

**Publicado no GitHub somente na branch experimental; ainda sem PASS no Windows desta correção.** Testes anteriores do usuário aprovaram desenho e snap, mas evidenciaram falha após concluir Bézier. A correção atual exige validação real antes de ser considerada aprovada.

```powershell
cd C:\ProjetoAlencarLaser
git status --short
git pull --ff-only origin feat/refino-curvas-rastreio-diagnosticos-19-8
if ($LASTEXITCODE -ne 0) { throw 'Git interrompido; preservar arquivos locais.' }
powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\TESTAR_19_8_WINDOWS.ps1" -SomenteDesenho
```

Se a suíte passar, com a máquina desconectada, conferir em um **novo projeto de teste**, sem sobrescrever arquivos importantes:

1. Crie uma Bézier, pressione **Seleção**, clique na curva: deve destacar; arraste para mover o contorno.
2. Ctrl+Z deve remover o desenho após a criação, e Ctrl+Y deve restaurá-lo sem curvas fantasmas.
3. Clique em **Bézier** para visualizar e editar alças; depois em **Seleção** para mover normalmente.
4. Use **Tesoura** no meio de uma Bézier aberta: devem surgir dois caminhos editáveis; Ctrl+Z reúne novamente. Em uma curva fechada, deve abrir apenas no ponto escolhido.
5. Delete com curva selecionada deve excluir a forma; Ctrl+Z deve restaurar.

Não habilitar G-code, CAM, USB/TCP, comando M3/M4, motores ou emissão física. `main` e `APLICATIVO` homologado preservados.

## Próxima arquitetura recomendada

Consolidar um **documento e histórico geométrico comuns** entre as curvas nativas e as polilinhas, com seleção, ferramentas, layer metadata e Undo global; evitar conversão destrutiva das curvas. Isso é necessário para equivalência mais completa ao LightBurn, mas não é pré-requisito para testar este hotfix.
