# Correção do Ctrl+D — duplicação em lote 20m (07/11/2026)

## Falha reproduzível pela leitura do código
Na GUI Qt TESTE 19.8, o atalho `Ctrl+D` acionava `duplicate_obj` e exigia `ids.size()==1`, lançando `Duplicar: selecione um objeto por vez.`. O coletor armazenava apenas `command_error`, sem a causa. O relato do operador de 23 erros no diagnóstico e falhas ao duplicar centenas de contornos é compatível com essa restrição, mas não permite atribuir TODOS os eventos antigos à mesma ação.

## Correção aplicada (branch experimental)
- `src/editor_tools.hpp`: `Editor::duplicateObjects(ids,offset)`, única chamada a `change()`, criando novos IDs de objetos e nós, offset +3/+3 mm, mantendo camadas e agrupamentos com IDs de grupo renovados, e clonando metadados de textos editáveis completos. Expande seleções parciais de grupos/textos. Verifica bloqueio de camada, IDs, finitude, limite de 1000 objetos e 150000 nós antes de persistir. Erros não produzem Undo, novos contornos ou alterações parciais.
- `qt/qt6_test6.cpp`: duplicar pelo botão e pelo Ctrl+D agora aceita seleção múltipla e seleciona as cópias. O menu de contexto utiliza a mesma operação.
- `src/diagnostics_collector.hpp`: `shapes_duplicated` adicionado à allowlist de eventos agregados locais. Não inclui caminhos, nomes, coordenadas nem arquivos.
- Testes C++ offline `alencar_test20m_batch_duplicate`: 575 clones, Undo/Redo atômico, grupos, texto editável, camadas bloqueadas e limite.
- Teste GUI Qt `--duplicate-batch-575-20m-test`: Ctrl+D real em janela Qt, 575 selecionados, 1150 contornos ao final, renderização, Undo/Redo e seleção dos novos.
- `SCRIPTS/TESTAR_DUPLICACAO_575_20M_WINDOWS.ps1`: compila e valida ambos os alvos, gerando JSON local.

## Comando para Windows
Feche o editor de desenvolvimento e mantenha a máquina laser fisicamente desconectada.

```powershell
cd C:\ProjetoAlencarLaser
git fetch origin
if ($LASTEXITCODE -ne 0) { throw 'Falha fetch' }
git switch fix/duplicate-batch-20m-20261107
if ($LASTEXITCODE -ne 0) { throw 'Falha na troca de branch; nao use force/reset/clean' }
git pull --ff-only origin fix/duplicate-batch-20m-20261107
if ($LASTEXITCODE -ne 0) { throw 'Falha pull' }
powershell -NoProfile -ExecutionPolicy Bypass -File '.\SCRIPTS\TESTAR_DUPLICACAO_575_20M_WINDOWS.ps1'
```

## Critérios de conclusão
**Código implementado e publicado; ainda não compilado nem executado no Windows depois dos commits**. Só homologar Ctrl+D em lote após:
1. PASS dos testes C++ e GUI em Windows.
2. Abrir cópia de projeto real, selecionar 575 objetos e Ctrl+D uma vez, conferir duplicatas +3 mm, IDs, camadas, agrupamentos, seleção, Undo/Redo, salvar/reabrir.
3. Verificar que `command_error` não cresce na tentativa bem-sucedida e `shapes_duplicated` registra a ação, com coleta ativada.
4. Executar novamente sete gates da suíte de pré-lançamento antes de produzir instalador.

Escopo atual: **polilinhas e texto vetorizado do editor canônico**, não imagens raster independentes ou curvas Bézier em sessão nativa; devem ser avaliadas separadamente. Sem modificação em main, APLICATIVO, instalador ou comandos físicos.
