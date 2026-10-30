# Alencar Laser — identidade aprovada Pégaso V1 (30/10/2026, continuidade da rodada 29/10)

## Escopo aprovado

O proprietário aprovou como símbolo da identidade visual do software a imagem enviada de um **pégaso com asas laranja, corpo preto, casco gerando ponto luminoso de laser**, sem letras sobre o símbolo. Paleta predominante: preto e laranja; assinatura “ALENCAR LASER” em metal prateado/laranja. É a identidade aprovada para uso na **candidata experimental 19.8**, sem qualquer mudança na base homologada.

**Atenção:** não reinterpretar/desenhar outro pégaso. O símbolo aprovado é a imagem enviada pelo operador no chat (arquivo local de origem `1000896590.png`, 1536×1536, conteúdo JPEG). A assinatura horizontal é arte aprovada no chat (`a_sleek_high_contrast_graphic_logo_branding_image.png`). O kit final é distribuído como **`Alencar_Laser_Pegaso_Kit_v1.zip`** na conversa; binários de imagem ainda NÃO foram inseridos no repositório remoto.

## Caminhos e formatos locais

O pacote contém `DADOS/LOGO/PEGASO_V1` com:
- `pegaso_aprovado_original.png`: cópia PNG sem redesenho do símbolo selecionado;
- `pegaso_icone_256.png` e outras resoluções (16/24/32/48/64/128/512);
- `alencar_laser_pegaso.ico`: ícone Windows multirresolução;
- `marca_horizontal_aprovada.png`, `marca_cabecalho_compacta.png`;
- `abertura_pegaso.png`: splash visual estático, com rótulo “EDITOR OFFLINE • TESTE 19.8 • EMISSÃO BLOQUEADA”;
- `manifesto.json` e hashes SHA-256.

PNG do pégaso mantém fundo escuro; NÃO é transparente e NÃO é arquivo SVG vetorial. Uma versão SVG deve ser feita e aprovada separadamente. Não distribuir arquivos de fontes.

## Integração C++ opcional (sem vincular o programa a um caminho antigo)

`DESENVOLVIMENTO/FONTE/qt/branding_pegaso_19_8.hpp` procura os recursos em `DADOS/LOGO/PEGASO_V1` a partir do diretório atual e de ancestrais do executável, sem assumir `C:\\ProjetoAlencarLaser` em C++.

`qt6_test6.cpp` usa:
1. `marca_cabecalho_compacta.png` no topo quando instalada;
2. `pegaso_icone_256.png` no ícone da janela/aplicativo;
3. `abertura_pegaso.png` apenas na abertura interativa (nunca nos testes automáticos);
4. **fallback imediato à logo anterior** `DADOS/LOGO/logo_alencar_laser.png` ou texto se o kit não estiver instalado.

A alteração visual não toca o formato `.alaser`, os documentos, o sistema de CAM, trajetos, os parâmetros, a conexão USB/TCP, G-code ou funções físicas. O banner `MÁQUINA DESCONECTADA | EMISSÃO BLOQUEADA` continua visível.

## Instalar o kit localmente — procedimento seguro

1. Confirmar que a branch no VS Code é `fix/validacao-real-19-8-20261029` e que `git status --short` não mostra mudanças locais conflitantes.
2. Atualizar com `git pull --ff-only origin fix/validacao-real-19-8-20261029`. Nunca fazer reset/clean automático.
3. Extrair `Alencar_Laser_Pegaso_Kit_v1.zip` fora de `C:\\ProjetoAlencarLaser`.
4. Rodar `powershell -NoProfile -ExecutionPolicy Bypass -File ".\\INSTALAR_IDENTIDADE_DEV.ps1"` **sem `-Aplicar`** para simular.
5. Conferir hashes e destino; só após revisão rodar o mesmo comando com `-Aplicar`. O instalador cria/atualiza EXCLUSIVAMENTE `DADOS/LOGO/PEGASO_V1` e faz backup em `BACKUP/IDENTIDADE_PEGASO` se um recurso de mesmo nome for diferente. Não toca no logo anterior, em `APLICATIVO` ou na base homologada.
6. No Windows, executar `powershell -NoProfile -ExecutionPolicy Bypass -File ".\\SCRIPTS\\TESTAR_19_8_WINDOWS.ps1" -SomenteMarca`. O relatório será `RELATORIOS/AUDITORIAS/GITHUB_19_8/marca_pegaso/RESULTADO_IDENTIDADE_PEGASO_19_8.json`. **PASS técnico não garante visual**, a seguir testar `-Abrir` após gates principais.
7. Inspecionar cabeçalho, Windows taskbar, 100% e escala 125%/150%, tela de abertura, legibilidade em fundo preto e modo claro; comparar janela sem kit (fallback). Não enviar comandos à máquina.

## Homologação e reversão

- O kit foi gerado e inspecionado localmente. A compilação Windows após a integração ainda precisa de confirmação.
- Antes de aprovar visualmente, checar se o pégaso é reconhecível em 16, 32 e 48 px, se o nome não fica minúsculo no cabeçalho e se os controles do editor continuam visíveis em monitor 1366×768.
- A reversão mais segura é **renomear** `DADOS/LOGO/PEGASO_V1` para `PEGASO_V1_DESATIVADO` com aplicativo fechado; o software volta a usar a logo anterior sem alterar projeto, versão ou executável.
- A PR #12 mantém estado rascunho e base experimental; sem merge em `main` nem substituição da versão homologada.


## Instalacao simplificada — ZIP em Downloads e um comando no VS Code

**Esta e a forma preferida para o operador.** Nao extrair manualmente o ZIP.

1. Baixe `Alencar_Laser_Pegaso_Kit_v1.zip` na pasta **Downloads** do Windows (pode conter `(1)` no nome se foi baixado novamente).
2. Atualize uma unica vez o repositorio experimental no VS Code com `git pull --ff-only origin fix/validacao-real-19-8-20261029`, estando nessa mesma branch e sem conflitos.
3. No terminal PowerShell localizado em `C:\ProjetoAlencarLaser`, execute **`.\04_INSTALAR_PEGASO.bat`**. Opcional para conferência sem copiar: `.\04_INSTALAR_PEGASO.bat -Simular`.

O arquivo `04_INSTALAR_PEGASO.bat` apenas inicia `SCRIPTS/INSTALAR_PEGASO_DOWNLOADS_19_8.ps1`. O script:
- verifica se o Git está na branch experimental e se o código de integração está presente;
- encontra o ZIP em Downloads (inclusive caminhos redirecionados pelo Windows);
- valida o SHA-256 fixado do ZIP aprovado `f1db7c250d72b9d702af3a1d493079d32e0fc3744465affee54edfa5bdaaecee` **antes de extrair**;
- extrai em pasta temporaria e confere os 13 itens/bytes/hashes de `manifesto.json`;
- prepara nova pasta e, se ja houver instalação diferente, move a anterior para `BACKUP/IDENTIDADE_PEGASO` antes de colocar `DADOS/LOGO/PEGASO_V1`;
- se houver falha durante substituição, tenta restaurar o backup. Nunca abre/executa scripts embutidos no ZIP.
- nao modifica `APLICATIVO`, EXE, `main`, logo anterior ou arquivos de trabalho `.alaser`.

**Limites:** instalação do kit visual apenas, nao é instalador do software. A compilação da integração Qt6, a abertura do editor e a avaliação visual ainda precisam passar no Windows. Esta rotina de instalação só foi verificada por revisão estática e validação do conteúdo do ZIP; nao houve execução de PowerShell no computador do operador.
