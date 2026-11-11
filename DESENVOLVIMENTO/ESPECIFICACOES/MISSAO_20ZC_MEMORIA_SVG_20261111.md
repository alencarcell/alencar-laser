# ALENCAR LASER — Fase 20zc | Memória das pastas e exportação SVG

Data: 11/11/2026. Base: `perf/repaint-local-20zb-20261111`. Candidata anterior J 20za preservada. Não alterar `main` ou `APLICATIVO`.

## Requisitos da operação
1. Ao salvar projetos ou exportar arquivos, a caixa de diálogo deve partir da **última pasta escolhida e utilizada com sucesso**, mesmo depois de fechar o programa.
2. A exportação de desenho vetorial para `.svg` deve funcionar inclusive com **engrenagens**, furos, pares e cremalheiras inseridos pelo próprio gerador.

## Implementação 20zc
- Helper persistente em `src/svg_export_20zc.hpp` usa `QSettings("AlencarLaser","DesktopBeta")`, chave `files/lastSaveDirectory`. A pasta só é gravada **após salvar o arquivo**, nunca ao cancelar. Na ausência de uma pasta válida, usa pasta do projeto anterior ou Documentos.
- A memória vale para `Salvar`, `Salvar como`, `Exportar diagnóstico JSON` e `Exportar SVG`. `files/lastProject` continua independente e preservado.
- Menus: `Arquivo > Exportar desenho vetorial em SVG...` e `Gerar > Engrenagens > Exportar engrenagens/desenho atual em SVG...`; atalho `Ctrl+Shift+E`.
- O SVG é **do desenho vetorial visível inteiro**, não somente do objeto selecionado. As engrenagens inseridas no editor são contornos canônicos; por isso são exportadas diretamente, com seus furos. O arquivo inclui polilinhas, formas e texto já vetorizado, além de curvas Bézier nativas usando comando `C` SVG.
- SVG 1.1 com `width`/`height` em **mm** e `viewBox` no mesmo sistema de unidades; sem conversão para pixels. Usa `QSaveFile` atômico, valida coordenadas e recusa exportação vazia/arquivo com extensão incorreta.
- Todas as formas são caminhos de traço preto, sem preenchimento. **Bitmaps, efeitos Fill/hatch, imagens não vetorizadas, potência, velocidade e CAM não são exportados.** Se há raster no projeto, o aplicativo informa a omissão. Para vetorizar bitmap, usar a ferramenta de traçado primeiro.
- A operação **não** altera o arquivo `.alaser v5`, o histórico, as camadas originais, a geometria nem a comunicação da máquina.

## Testes automáticos incorporados
Novo modo Qt `--svg-export-20zc-test` e parâmetro do PowerShell `-SomenteSvg`:
- Insere engrenagem gerada pelo motor de produção com contorno exterior e furo;
- Acrescenta uma Bézier nativa e verifica `<path>` SVG, comando `C`, fechamento `Z`, dimensões em mm e integridade XML;
- Verifica invariância do estado serializado e do número de Undo após exportar;
- Simula reinício de `QSettings` em INI isolado para verificar memória de pasta;
- Rejeita extensão incorreta sem criar arquivo;
- Produz `GEAR_SVG_20ZC_TEST.svg` para inspeção manual e JSON `RESULTADO_EXPORTACAO_SVG_20ZC.json`.
Também incluído no gate integral `-Abrir`.

## Validação no Windows
1. Conferir mudanças locais com `git status --short`, sem descartar arquivos.
2. Atualizar a branch 20zc com `git fetch origin` e `git switch --track origin/feat/memoria-pastas-exportacao-svg-20zc-20261111` (ou `git switch` sem `--track` se já houver branch local).
3. Rodar `powershell -NoProfile -ExecutionPolicy Bypass -File '.\SCRIPTS\TESTAR_19_8_WINDOWS.ps1' -SomenteSvg`. Em caso de erro, usar o primeiro `error:` do compilador ou o JSON gerado.
4. Se PASS, rodar a suíte completa com `-Abrir`, laser desconectado.
5. Criar engrenagem, par e cremalheira, exportar SVG pelo menu e abrir no LightBurn. **Medir largura/altura e furo com régua virtual** antes de usar em MDF.
6. Salvar SVG em outra pasta, fechar e abrir o aplicativo, depois acessar Salvar como: o diálogo deve lembrar a pasta. Cancelar não deve modificá-la.
7. Comparar o SVG com o `.alaser` (desenho, agrupamento visual e dimensões), sem alterar a fonte original.

## Estado real
Implementação publicada somente na branch experimental. **Compilação Windows, regressões executadas, importação SVG real no LightBurn e homologação ainda PENDENTES.** Não mesclar em `main`. Laser desconectado.
