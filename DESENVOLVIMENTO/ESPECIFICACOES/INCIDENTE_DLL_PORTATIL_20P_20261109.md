# Alencar Laser — Hotfix 20p: DLL ausente ao abrir ZIP portátil (09/11/2026)

## Incidente observado
A distribuição 20p foi gerada e o ZIP extraído, mas o iniciador abortou na linha de execução do executável com **exit code -1073741515**. Esse código corresponde a **0xC0000135 / STATUS_DLL_NOT_FOUND**, um erro de carregamento de dependência (DLL) no Windows. O log NÃO informa o nome da DLL ausente, portanto não é correto afirmar que uma biblioteca específica está faltando.

O empacotador anterior executou `windeployqt`, copiou três runtimes MSYS2, registrou hashes e produziu o ZIP, mas **não fez um smoke test real com PATH isolado**. O ambiente do projeto geralmente contém `C:\msys64\ucrt64\bin`, que pode mascarar dependências ausentes.

## Correção publicada
Branch `fix/20p-portable-dll-smoke-20261109` derivada de `release/20p-portable-homologation-20261109`.

- Novo `SCRIPTS/VALIDAR_DLL_PORTATIL_20P_WINDOWS.ps1`: usa o `objdump` de binutils MSYS2 para examinar os imports de todas as DLLs e EXEs empacotadas, incluindo plugins Qt, buscando dependências recursivas da mesma instalação UCRT64. Dependências não encontradas bloqueiam o processo. Não baixa DLLs de terceiros.
- Executa `--grouped-center-transform-19-9-test` em um processo cujo PATH **não inclui** `C:\msys64`, verifica exit 0, JSON PASS, histórico Undo/Redo íntegro e ausência de comandos físicos.
- Se o teste falhar, **nenhum ZIP novo será criado**. O diagnóstico conserva a pasta tentativa para inspeção.
- Se o teste passar, grava um relatório `*_SMOKE_RESULTADO.json` com hash do executável e DLLs adicionais copiadas; exclui logs temporários do pacote antes do manifesto final.
- O empacotador continua exigindo relatório anterior de 14 gates PASS, revisão exata e SHA-256 do executável candidato 20o.
- O iniciador do pacote continua validando o manifesto SHA-256, usa fixture sintética e nunca substitui a instalação principal.

## Comando para gerar pacote corrigido

```powershell
cd C:\ProjetoAlencarLaser
git fetch origin
if ($LASTEXITCODE -ne 0) { throw 'Falha no fetch' }
git switch fix/20p-portable-dll-smoke-20261109
if ($LASTEXITCODE -ne 0) { throw 'Falha ao mudar de branch; nao force' }
git pull --ff-only origin fix/20p-portable-dll-smoke-20261109
if ($LASTEXITCODE -ne 0) { throw 'Falha no pull' }
powershell -NoProfile -ExecutionPolicy Bypass -File '.\SCRIPTS\GERAR_PORTATIL_HOMOLOGACAO_20P_WINDOWS.ps1'
```

Após a mensagem verde `PACOTE PORTATIL 20p CRIADO APOS SMOKE ISOLADO`, extrair apenas o **ZIP novo**, não reutilizar o antigo. Executar `EXECUTAR_ALENCAR_LASER_20P.ps1` no diretório extraído com laser fisicamente desconectado.

## Importante
- **Implementado no GitHub, ainda não validado no Windows do usuário.**
- O smoke verifica ausência de dependências em uma sessão sintética, mas não substitui testes com mouse real, DPI, outros computadores ou todos os plugins Qt carregados dinamicamente.
- A falha pode revelar outras bibliotecas de carregamento dinâmico não capturadas pelos imports estáticos; se reaparecer, coletar código/JSON e rever dependências específicas em vez de baixar DLLs de sites aleatórios.
- Nenhuma atualização no aplicativo instalado e nenhum comando físico à máquina laser.
