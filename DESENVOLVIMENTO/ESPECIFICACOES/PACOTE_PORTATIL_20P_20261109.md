# ALENCAR LASER — PACOTE PORTÁTIL DE HOMOLOGAÇÃO 20P

**Data:** 09/11/2026  
**Branch de empacotamento:** `release/20p-portable-homologation-20261109`  
**Fonte do executável:** `candidate/offline-20o-20261107`.

## O que o procedimento faz
O Windows já confirmou os 14 gates da candidata 20o (captura fornecida pelo operador), mas **esse fato não substitui verificar o próprio executável, sua revisão e seu SHA-256**. O empacotador faz essas verificações novamente.

1. Confere se existe um JSON de 20o com status `AUTOMATED_PASS_MANUAL_PENDING`, 14 gates PASS, SHA-256 íntegro do executável e revisão **exatamente igual** ao HEAD da branch fonte 20o.
2. Copia o executável de laboratório para uma **nova pasta com nome único**, sem sobrescrever arquivos do programa instalado.
3. Executa `windeployqt` do MSYS2 UCRT64 e inclui runtimes de compilador disponíveis da mesma origem; exige `platforms/qwindows.dll`.
4. Inclui a fixture sintética (não seus projetos reais), um iniciador de laboratório e manifesto SHA-256 de todos os arquivos distribuídos.
5. Gera um ZIP e imprime seu SHA-256. O ZIP é um artefato local produzido no seu PC, **não está incluído no GitHub**.
6. O iniciador valida todos os hashes, usa o modo `MinimalViewportUpdate` experimental e abre somente a fixture fornecida, com diretório de sessão separado em `%LOCALAPPDATA%\AlencarLaser\Homologacao20P`.

## Como gerar no PowerShell
Mantenha a máquina **fisicamente desconectada**. Feche os executáveis em uso.

```powershell
cd C:\ProjetoAlencarLaser
git fetch origin
if ($LASTEXITCODE -ne 0) { throw 'Falha no fetch' }
git switch release/20p-portable-homologation-20261109
if ($LASTEXITCODE -ne 0) { throw 'Falha ao mudar branch: NAO use force, reset nem clean' }
git pull --ff-only origin release/20p-portable-homologation-20261109
if ($LASTEXITCODE -ne 0) { throw 'Falha no pull' }
powershell -NoProfile -ExecutionPolicy Bypass -File '.\SCRIPTS\GERAR_PORTATIL_HOMOLOGACAO_20P_WINDOWS.ps1'
```

**Local padrão:** `C:\ProjetoAlencarLaser\HOMOLOGACAO_PORTATIL\ALENCAR_LASER_20P_HOMOLOGACAO_<data>_<identificador>`, mais `.zip` ao lado.

## Teste manual do pacote
Extraia o ZIP para uma pasta temporária fora de `APLICATIVO`. Na pasta extraída, execute:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File '.\EXECUTAR_ALENCAR_LASER_20P.ps1'
```

Na GUI: selecione 1/20/575 formas, Ctrl+D, um Undo/Redo; teste Tesoura, zoom e seleção. **Nunca use hardware conectado.** Não copie trabalhos originais para o pacote. Se um arquivo real for necessário na fase seguinte, use somente cópia de homologação.

## Reversão e backup
- **Reversão da homologação portátil:** feche o editor e mantenha o ZIP como backup se desejar; apague apenas a pasta da extração portátil. Isso não desinstala nem altera o programa atual.
- **Sessões e relatórios** são mantidos em `%LOCALAPPDATA%\AlencarLaser\Homologacao20P`. Não apague automaticamente esses registros; arquive antes se desejar limpá-los.
- **Backup de produção:** nenhuma pasta de produção é alterada ou salva automaticamente por este empacotador. Faça um backup independente antes de qualquer instalação de substituição no futuro.
- **Atenção:** isto é uma distribuição **portátil de homologação**, não um instalador definitivo com rollback automático.

## Situação
Scripts e documentação publicados; **ZIP ainda não gerado nem inspecionado no Windows nesta etapa**. A verificação dos runtimes do Qt e os testes de abertura precisam passar antes de aprovar o pacote como transportável. A versão main e o diretório APLICATIVO não foram modificados. `Ctrl+D` normal e proteção de carga extrema têm testes automatizados aprovados, mas a correção de um `command_error` eventual ainda requer investigação se reaparecer.
