# Alencar Laser — Editor 19.8: texto direto, alças e curvas
Data: 28/10/2026
Estado: **implementação experimental no GitHub; Windows/Qt e mouse físico NÃO testados**.

## Referência de comportamento: LightBurn
- Texto: https://docs.lightburnsoftware.com/2.0/Reference/Text/
  - Clique na área, digite; Esc conclui. A edição de objetos de texto é preservada como propriedade no LightBurn.
- Controles de transformação: https://docs.lightburnsoftware.com/latest/Reference/TransformControls/
  - 8 alças de tamanho; quatro de rotação. Canto preserva proporção, Shift libera, Ctrl usa centro; a transformação é reversível por Esc antes da conclusão.
- Tesoura: https://docs.lightburnsoftware.com/2.0/Reference/TrimShapes/
  - Exige intervalo entre interseções ou remove forma toda sem cruzamentos; não opera em grupos. Pode unir novos caminhos quando apropriado.
  
## Implementação nesta branch
1. Texto: substitui caixas modais por QLineEdit transparente sobre QWidget viewport; uma linha, até 100 caracteres, Arial/12 mm inicialmente; Enter/Esc conclui, campo vazio cancela; a vetorização cria contornos numa única transação Ctrl+Z. **Texto confirmado ainda NÃO é texto semanticamente editável**; não há suporte a múltiplas linhas, kerning ou texto curvo neste bloco.
2. Seleção: contorno-limite com oito alças de tamanho e quatro pontos de rotação. Arraste mostra prévia visual com QGraphicsItem::setTransform, sem modificar geometria canônica antes do release. Os comandos Editor::scaleObjects/rotateObjects registram o commit em uma operação atômica; Esc cancela. Canto proporcional; Shift sem bloqueio; Ctrl ancora no centro. Camadas bloqueadas/invisíveis não são transformadas.
3. Elipses recém-criadas: aumento de 64 para 512 pontos na geometria poligonal para reduzir facetas. **Não são círculos/arcos matematicamente exatos**; polígonos existentes e compatibilidade v5 não são reescritos.
4. Tesoura: nenhum afrouxamento cego em tolerância ou topologia. O planner atual recusa tangências, interseções exatamente nos vértices e trechos com colinearidade, além de grupos/Bézier nativa. Registrar projetos reais que falham, preservar os arquivos originais, comparar geometria antes/depois e criar testes por caso.

## Segurança e validação
- `main` e `APLICATIVO` não alterados.
- Wi-Fi/TCP, laser físico, G-code e comandos de movimento continuam bloqueados.
- Teste focado: `powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\TESTAR_19_8_WINDOWS.ps1" -SomenteInteracao`.
- Se focado PASS: suíte `-Abrir`, mouse físico, realce de alças, Esc durante escala, seleção múltipla, texto com vazados, salvar/reabrir cópia .alaser v5, Tesoura em curva e importação LBRN2 real.
- JSON do teste: `RELATORIOS/AUDITORIAS/GITHUB_19_8/interacao/RESULTADO_TEXTO_TRANSFORMACOES_19_8.json`.
- Falha do gate: não abrir a candidata automaticamente; investigar JSON/compilação, não publicar nem mesclar.

## Pendências posteriores
- Modelo de texto editável (família, tamanho, string, alinhamento, linhas) e edição diretamente sobre o objeto.
- Modelo geométrico de elipse/arco/Bézier nativo; preview Qt com QPainterPath de precisão e persistência segura.
- Transformação de curvas Bézier nativas (seleção atual se limita a objetos de polilinha), tesoura com interseções de curvas e união automática.
- Instrumentação de desempenho em projetos com 13 mil+ nós; não prometer paridade completa LightBurn antes dos testes.
