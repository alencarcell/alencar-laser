# Fase 20S — correção de compilação Qt: `findChild` e `Q_OBJECT`

Registro: 09/11/2026. Evidência: captura do terminal Windows com `qobject.h:163:62: error: static assertion failed: No Q_OBJECT in the class passed to QObject::findChild`, a partir de `qt6_test6.cpp:9194`.

## Causa
A classe `alaser::escape20s::Preview` estende `QWidget`, sem declaração própria `Q_OBJECT`. O teste de GUI chamava:
```cpp
dialog.findChild<alaser::escape20s::Preview*>("escape_preview_20s");
```
No Qt 6.12, o `findChild<T>()` desse tipo demanda `Q_OBJECT` no tipo pesquisado.

## Correção
Commit `59a5352b8e0524bd276a8af2cd792d058310917b`: usar `findChild<QWidget*>()` (tipo com metadados Qt) e converter o ponteiro para a classe concreta conhecida por esse nome:
```cpp
auto* previewWidget=dialog.findChild<QWidget*>("escape_preview_20s");
auto* preview=static_cast<alaser::escape20s::Preview*>(previewWidget);
```
O teste já valida o ponteiro antes de usá-lo. Não foi necessário adicionar `Q_OBJECT` nem alterar moc, widgets em produção, algoritmo de roda de escape ou arquivos de projetos.

## Situação
Correção publicada na mesma branch `feat/20s-escape-wheel-study-20261109`. **Compilação e oito gates ainda não foram reexecutados no Windows após o commit.** Avisos de depreciação e indentação anteriores não são a causa da falha fatal apresentada.

## Comando Windows
```powershell
cd C:\ProjetoAlencarLaser
git pull --ff-only origin feat/20s-escape-wheel-study-20261109
if ($LASTEXITCODE -ne 0) { throw 'Pull não realizado; verifique mudanças locais sem force/reset/clean.' }
powershell -NoProfile -ExecutionPolicy Bypass -File '.\SCRIPTS\TESTAR_RODA_ESCAPE_20S_WINDOWS.ps1' -Abrir
```

Máquina laser permanece fisicamente desconectada. Não fazer merge na main nem promoção para produção com base apenas no patch de compilação.
