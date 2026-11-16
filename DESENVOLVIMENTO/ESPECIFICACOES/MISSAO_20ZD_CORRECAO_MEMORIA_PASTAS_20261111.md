# ALENCAR LASER — Correção 20zd | Memória persistente de pastas

Data: 11/11/2026. Branch mantida para o operador: `feat/memoria-pastas-exportacao-svg-20zc-20261111`. PR: #32.

## Resultado de homologação manual 20zc
- Memória de pastas após fechar/reabrir e usar Salvar como: **FALHOU**.
- Exportação SVG de engrenagem com furo: **PASSOU**.
- Importação no LightBurn e dimensões em milímetros: **PASSOU**.

O JSON de diagnóstico de desempenho não registra o estado da caixa de diálogo nem o resultado de QSettings. Não permite afirmar a causa raiz da falha de persistência.

## Lacuna identificada na validação anterior
O teste 20zc chamava `rememberSaveDirectory` diretamente com um arquivo INI temporário, reabria um novo objeto QSettings e conferia a chave. Ele não exercitava o fluxo real `Salvar como → gravar projeto → fechar janela → nova janela → Salvar como`. Portanto, o PASS anterior não comprovava a experiência do operador.

## Correção 20zd
- Preferência de diretório separada da recuperação de sessão e gravada em `QSettings::IniFormat + QSettings::UserScope`, em `AlencarLaser/DesktopBeta.ini` no perfil do usuário do Windows.
- A antiga chave nativa `files/lastSaveDirectory` é lida para migração quando o novo INI ainda não tem pasta válida.
- Após salvar com sucesso, o código faz `sync()`, verifica `QSettings::status()` e abre uma **nova instância** de QSettings para confirmar a leitura do mesmo diretório.
- O valor é espelhado na antiga chave nativa somente quando o store usado é o verdadeiro INI de usuário. Testes com arquivos temporários nunca escrevem no Registro do usuário.
- A preferência de pasta não depende de `uiPersistenceEnabled_`/recuperação de projetos; a persistência é habilitada para execução interativa. A configuração de teste é explicitamente isolada.
- Se o salvamento do projeto funcionar mas a preferência não for gravada, a barra de status mostra um aviso em vez de afirmar que a pasta foi memorizada.
- A mesma preferência é usada em Salvar, Salvar como, exportar SVG e exportar diagnóstico.

## Regressão que faltava: diálogo real e reinício
Novo modo `--save-folder-20zd-test`, exposto no script como `-SomentePastas`:
1. Usa um INI temporário isolado via variável de ambiente de teste.
2. Abre a janela Qt e chama o método real `saveAs()`.
3. Intercepta apenas a caixa de diálogo de teste (não nativa), escolhe uma pasta diferente e salva um projeto `.alaser`.
4. Verifica o arquivo no destino e lê a preferência em um novo QSettings.
5. Fecha a primeira janela e cria uma nova instância de `LabWindow`.
6. Abre novamente Salvar como e verifica o diretório inicial real do diálogo, depois cancela.
7. Restaura as variáveis de ambiente e não altera as preferências reais do operador.

O modo de teste não abre comunicação com máquina, não emite G-code e não grava no Registro real.

## Validar no Windows
Na raiz `C:\ProjetoAlencarLaser`, preservar primeiro o `git status --short`, depois:

```powershell
git fetch origin
git switch feat/memoria-pastas-exportacao-svg-20zc-20261111
git pull --ff-only origin feat/memoria-pastas-exportacao-svg-20zc-20261111
powershell -NoProfile -ExecutionPolicy Bypass -File '.\SCRIPTS\TESTAR_19_8_WINDOWS.ps1' -SomentePastas
```

O teste deve terminar com `Memoria persistente real do dialogo Salvar como 20zd PASS`. Se falhar, enviar:
- `RELATORIOS\AUDITORIAS\GITHUB_19_8\memoria_pastas_20zd\RESULTADO_MEMORIA_PASTAS_20ZD.json`;
- `RELATORIOS\COMPILACAO\BUILD_QT_19_8_STDERR.txt` se a compilação falhar.

Se passar, executar a suíte completa com `-Abrir` e repetir manualmente o cenário de Save As após reiniciar o aplicativo. A exportação SVG/LightBurn que já passou não precisa ser refeita, exceto se houver regressão.

## Critério de conclusão
- [ ] Compilação Qt Windows PASS.
- [ ] Teste de diálogo real + nova janela PASS.
- [ ] Teste manual do operador após fechar e reabrir PASS.
- [ ] Preferência não é alterada ao cancelar diálogo.
- [ ] Exportação SVG e dimensões LightBurn continuam PASS.
- [ ] `main` e `APLICATIVO` não alterados; laser desconectado.

**Status atual:** correção e regressão automatizada publicadas no branch/PR; ainda sem compilação Windows nem execução local do novo teste. Não declarar memória aprovada antes do operador validar novamente.
