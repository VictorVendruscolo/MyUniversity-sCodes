# Trabalho 1 (PP2) — Computação Gráfica — UEMS

**Aluno:** Victor Rech Vendruscolo (nº 12 da lista → **exercício 12: CONE**)
**Programa base:** `canvas.cpp` de Sumanta Guha (Chapter 3 / Canvas), obrigatório pelo enunciado.

Este documento explica **o que foi feito** e **como o programa funciona**, para leitura
posterior e para a avaliação escrita do conhecimento do código (PO2).

---

## 1. O que o enunciado pedia

O enunciado tem duas partes obrigatórias:

- **Item I (todos):** melhorar o `canvas.cpp` e — ponto marcado como *"IMPORTANTE NA
  AVALIAÇÃO"* — usar **monitoramento do movimento do mouse** para o usuário ver a
  primitiva mudando **em tempo real** enquanto move o mouse, **antes** do clique final
  que a salva (efeito *rubber-band* / pré-visualização).
- **Item II (por número do aluno):** o aluno nº *i* resolve o exercício *i*. Como sou o
  nº 12, o exercício é: **adicionar um cone de forma interativa, com as possibilidades de
  transformações geométricas, e criar um ícone adequado.**

Tudo foi feito **reaproveitando técnicas do `canvas.cpp` e das aulas** (pasta
`Computacao_Grafica` e o livro do Sumanta Guha):

| Recurso usado | Onde já aparecia na aula |
|---|---|
| Estrutura do canvas (classes, `vector`, caixas de seleção, `pickPrimitive`, menu) | `Cap4/Canvas.cpp` (versão que já tinha adicionado a primitiva CIRCLE em aula) |
| Callback de movimento do mouse com pré-visualização em tempo real | `Cap3/MouseMotion.cpp` |
| Desenho do cone com `glutWireCone(base, altura, slices, stacks)` | `Cap4/Objects.cpp`, `Cap4/Clown.cpp` |
| Transformações geométricas com `glTranslatef` / `glRotatef` / `glScalef` e teclas x/y/z | `Cap4/Objects.cpp` |
| Base elíptica do ícone desenhada com laço `cos/sin` | `Cap3/MouseMotion.cpp`, `Cap3/circulo.cpp` |
| `sqrt` (tamanho/distância) | `Cap3/MouseMotion.cpp` |
| `atan2` (direção do cone) | **trigonometria de transformações do Cap. 4 do Sumanta Guha** |

> **Dois recursos não estão no `canvas.cpp` base** e ficam explicitados no código:
> **`atan2`** (usado só para achar a direção do cone) vem da trigonometria do **Cap. 4**
> do Sumanta Guha; e **`glutPassiveMotionFunc`** é a versão "passiva" (mouse sem botão)
> do `glutMotionFunc` do `Cap3/MouseMotion.cpp`, exigida pelo Item I (entre os dois
> cliques o botão está solto).

---

## 2. Como usar (interação)

1. **Clique esquerdo** em uma das caixas da barra à esquerda para escolher a primitiva:
   ponto, linha, retângulo ou **cone** (a 4ª caixa, com o ícone de cone).
2. **Clique esquerdo na área de desenho:**
   - **Ponto:** um clique.
   - **Linha / Retângulo / Cone:** dois cliques. **Entre os dois cliques**, mova o mouse
     e veja a primitiva em **vermelho** mudando em tempo real (Item I). O 2º clique salva.
   - Para o cone: o 1º clique fixa o **vértice (ápice)**; ao mover o mouse o cone é
     **moldado a partir do vértice** — o vetor *1º ponto → mouse* define o **tamanho**
     (distância) **e a direção** para onde o cone aponta (como o retângulo muda de tamanho
     e de lado), com pré-visualização em tempo real. O 2º clique salva (em preto).
3. **Transformações geométricas do cone** (agem sobre o **último cone desenhado**):
   - `x` / `X` — rotaciona no eixo X (+/−)
   - `y` / `Y` — rotaciona no eixo Y (+/−)
   - `z` / `Z` — rotaciona no eixo Z (+/−)
   - `+` / `-` — aumenta / diminui a escala
   - **setas** ← → ↑ ↓ — transladam o cone
4. **Clique direito** abre o menu: *Grid* (On/Off), *Clear* (limpa tudo), *Quit* (sair).
   **ESC** também sai.

---

## 3. Como funciona por dentro (visão geral do código)

### 3.1 Primitiva CONE (Item II)
Seguindo o mesmo molde das outras primitivas do `canvas.cpp`:

- `#define CONE 4` e `NUMBERPRIMITIVES 4` (antes eram 3 primitivas).
- Uma **classe `Cone`** que — igual a `Line`/`Rectangle` — guarda **dois pontos**:
  `(x1, y1)` = vértice/ápice e `(x2, y2)` = ponto do mouse; e o estado das transformações do
  próprio cone: `angleX, angleY, angleZ` (rotações) e `scale`. Do vetor entre os dois pontos
  saem o **tamanho** (distância, com `sqrt`, como no `MouseMotion.cpp`) e a **direção**
  (ângulo, com `atan2`, trigonometria do Cap. 4 do Sumanta Guha).
- Um `vector<Cone> cones` e a função `drawCones()` que percorre o vetor (igual a
  `drawPoints`, `drawLines`, `drawRectangles`).
- `drawConeSelectionBox()` desenha a 4ª caixa e o **ícone do cone** (duas laterais até o
  ápice + base elíptica feita com `cos/sin`).
- Em `pickPrimitive`, `mouseControl`, `clearAll` e `printInteraction` foi acrescentado o
  caso do cone, exatamente no mesmo padrão dos demais.

O desenho do cone (`Cone::drawCone`) usa a sequência clássica de `Cap4/Objects.cpp`:

```
// A partir dos dois pontos:
dx = x2 - x1;  dy = y2 - y1;
height   = sqrt(dx*dx + dy*dy);      // tamanho = distância vértice -> mouse (como MouseMotion.cpp)
radius   = 0.40 * height;            // raio proporcional (mantém forma de cone)
dirAngle = atan2(dy, dx) * 180/PI;   // direção do mouse, em graus (trig. do Cap. 4)

glPushMatrix();
  glTranslatef(x1, y1, 0);           // leva o VÉRTICE (ápice) ao ponto clicado
  glRotatef(angleZ, 0,0,1);          // transformações do usuário (rotação...)
  glRotatef(angleY, 0,1,0);
  glRotatef(angleX, 1,0,0);
  glScalef(scale, scale, scale);     // ...e escala
  glRotatef(dirAngle - 90, 0,0,1);   // aponta o cone na DIREÇÃO do mouse
  glRotatef(70, 1,0,0);              // inclinação fixa p/ ficar "3D" (base = elipse)
  glTranslatef(0, 0, -height);       // recua p/ o ápice ficar na origem (no clique)
  glutWireCone(radius, height, 20, 12);
glPopMatrix();
```

> **Ancoragem no vértice + direção:** o `glutWireCone` desenha a base em `z = 0` e o ápice
> em `z = altura`. O `glTranslatef(0, 0, -height)` recua o cone ao longo do eixo, levando o
> **ápice para a origem local** — ou seja, para o exato ponto clicado. O
> `glRotatef(dirAngle - 90, 0,0,1)` gira o cone para ele **apontar na direção do mouse**.
> Assim o cone **nasce do vértice** e é moldado (tamanho + direção) em tempo real conforme
> o mouse, como o retângulo usa os dois pontos.

### 3.2 Cone 3D dentro de um canvas 2D
O `canvas.cpp` usa **projeção ortográfica** (`glOrtho`), sem iluminação e sem
*depth-test*. Para um objeto 3D (o cone) aparecer com cara de cone:

- Ele é desenhado em **modo aramado** (`glutWireCone`) — não precisa de luz nem de teste
  de profundidade, mantendo o estilo plano do `canvas.cpp` original.
- Uma **inclinação fixa** (`glRotatef(70, 1,0,0)`) faz a base circular virar uma **elipse**
  e as laterais formarem o triângulo característico do cone.
- A única mudança na projeção foi **ampliar os planos near/far** do `glOrtho` (de `-1..1`
  para `-maxDim..maxDim`), para o cone inclinado não ser cortado. Isso **não afeta** as
  primitivas 2D, que ficam todas em `z = 0`.

### 3.3 Pré-visualização em tempo real (Item I)
- Uma variável global `(curX, curY)` guarda a **posição atual do mouse**.
- A função `mouseMotion(x, y)` é registrada nos callbacks
  **`glutPassiveMotionFunc`** (mouse movido **sem** botão pressionado — o caso entre os
  dois cliques) e **`glutMotionFunc`** (mouse movido **com** botão pressionado).
  Ela atualiza `(curX, curY)` e chama `glutPostRedisplay()` sempre que ainda falta o 2º
  clique (`pointCount == 1`). Essa é a mesma técnica de `Cap3/MouseMotion.cpp`, usando a
  variante *passive* porque o canvas trabalha com **cliques**, não com arraste.
- Em `drawScene`, quando `pointCount == 1`, a função `drawPreview()` desenha em **vermelho**
  a linha, o retângulo ou o cone do 1º ponto `(tempX, tempY)` até o mouse `(curX, curY)` —
  é a primitiva "prévia" que o usuário vê mudar em tempo real antes de salvar.

### 3.4 Transformações geométricas (teclado)
- `keyInput` trata `x X y Y z Z` (rotações) e `+ -` (escala), chamando os métodos do
  **último cone** (`cones.back()`), que é o "cone ativo".
- `specialKeyInput` trata as **setas** para transladar o cone ativo.
- Cada tecla altera o estado do cone e chama `glutPostRedisplay()` para redesenhar.

---

## 4. Como compilar e executar

O projeto usa OpenGL + GLUT (freeglut), como todos os códigos da disciplina.

```bash
# na pasta Trabalho1_CG
g++ canvas.cpp -o canvas -lglut -lGL -lGLU -lm
./canvas
```

Ou usar a task do VS Code já existente no repositório
(`Computacao_Grafica/.vscode/tasks.json`), que compila com
`-lGL -lGLU -lglut` e executa.

---

## 5. Resumo das alterações em relação ao `canvas.cpp` original

- **+** `#include <cmath>` e `#define CONE 4`, `NUMBERPRIMITIVES 4`, `PI`.
- **+** variáveis globais `curX, curY` (posição do mouse para a pré-visualização).
- **+** classe `Cone`, `vector<Cone> cones`, `drawCones()`. O `drawCone` usa `sqrt` (tamanho)
  e `atan2` (direção, trig. do Cap. 4) para moldar o cone conforme o mouse.
- **+** `drawConeSelectionBox()` (4ª caixa + ícone de cone).
- **+** `drawPreview()` (pré-visualização em tempo real de linha/retângulo/cone).
- **+** `mouseMotion()` e registro de `glutPassiveMotionFunc` / `glutMotionFunc`.
- **+** `specialKeyInput()` e registro de `glutSpecialFunc` (setas → translação).
- **~** `keyInput` agora rotaciona/escala o cone ativo.
- **~** `resize` amplia os planos near/far do `glOrtho` (para o cone 3D não ser cortado).
- **~** `drawScene`, `pickPrimitive`, `mouseControl`, `clearAll`, `printInteraction`
  atualizados para incluir o cone.
- **~** `drawScene` desenha a **barra de seleção por último**: as caixas preenchidas
  cobrem qualquer parte do cone que invada a área da aba, então o cone gerado **nunca
  ultrapassa** o limite da aba de seleção de formas.
- As primitivas originais (ponto, linha, retângulo, grade e menu) foram **mantidas**.
