# Guia de Estudo — Trabalho 1 de Computação Gráfica (Cone)

> Objetivo: você conseguir **explicar cada linha** do `canvas.cpp` na avaliação escrita
> (PO2), com foco na sua parte (o **cone**, exercício 12). Estude na ordem das partes.
> No código, tudo que tem o marcador **`[T1]`** foi acrescentado/alterado por você; o
> resto é o `canvas.cpp` original do Sumanta Guha (só com os comentários traduzidos).

---

## Parte 0 — O que a prova cobra (para não errar o alvo)

- **Item I (obrigatório):** usar o **monitoramento do movimento do mouse** para o usuário
  ver a primitiva mudando **em tempo real**, **antes** do clique final. → Isso é feito por
  `mouseMotion` + `glutPassiveMotionFunc`/`glutMotionFunc` + `drawPreview`.
- **Item II (seu número = 12):** adicionar um **cone** interativo, com **transformações
  geométricas** e um **ícone** adequado. → Classe `Cone`, `glutWireCone`, teclado
  (rotação/escala/translação) e `drawConeSelectionBox`.
- **Base obrigatória:** o `canvas.cpp`. Tudo foi construído em cima dele.

---

## Parte 1 — Fundamentos de OpenGL/GLUT (a base de tudo)

Você precisa dominar estes conceitos, porque o cone depende deles.

### 1.1 Como um programa GLUT funciona (o `main`)
GLUT é uma biblioteca que cria a janela e chama funções suas (**callbacks**) quando algo
acontece. O `main` só **registra** os callbacks e entra num laço infinito:

- `glutInit`, `glutInitWindowSize`, `glutCreateWindow` → cria a janela.
- `glutDisplayFunc(drawScene)` → quem **desenha** a tela.
- `glutReshapeFunc(resize)` → chamada quando a janela muda de tamanho.
- `glutKeyboardFunc(keyInput)` → teclas comuns (letras, `+`, `-`, ESC).
- `glutSpecialFunc(specialKeyInput)` → teclas especiais (setas). *(igual ao Objects.cpp)*
- `glutMouseFunc(mouseControl)` → **cliques** do mouse.
- `glutPassiveMotionFunc(mouseMotion)` / `glutMotionFunc(mouseMotion)` → **movimento** do mouse.
- `glutMainLoop()` → laço infinito: fica esperando eventos e chamando os callbacks.

> **Pergunta típica:** "Quem chama `drawScene`?" → O GLUT, sempre que a tela precisa ser
> redesenhada, ou quando alguém pede com `glutPostRedisplay()`.

### 1.2 Buffer duplo
`glutInitDisplayMode(GLUT_SINGLE | GLUT_DOUBLE)` e `glutSwapBuffers()` no fim do
`drawScene`: desenha-se num buffer escondido e depois troca com o visível, evitando
"piscar". Por isso todo desenho termina com `glutSwapBuffers()`.

### 1.3 Sistema de coordenadas (MUITO importante)
No `resize`:
```c
glOrtho(0.0, w, 0.0, h, -maxDim, maxDim);
```
- **Projeção ortográfica**: sem perspectiva; 1 unidade = 1 pixel.
- Origem **(0,0)** no canto **inferior esquerdo**; `x` cresce para a direita, `y` para cima.
- O **mouse do sistema** tem origem no canto **superior** esquerdo (y para baixo). Por isso,
  no `mouseControl` e no `mouseMotion`, convertemos: **`y = height - y`**.
- `-maxDim..maxDim` é o intervalo de profundidade (z). **[T1]** No original era `-1..1`;
  ampliamos para o cone 3D inclinado (que tem z grande) não ser cortado. As formas 2D
  ficam em `z = 0`, então não mudam.

### 1.4 Modo imediato (como se desenha)
- `glBegin(MODO) ... glVertex3f(x,y,z) ... glEnd()` — lista de vértices. Modos usados:
  `GL_POINTS`, `GL_LINES`, `GL_LINE_STRIP`, `GL_LINE_LOOP`.
- `glColor3f(r,g,b)` — cor atual (0 a 1). `glPointSize`, `glLineStipple` (linha tracejada da grade).
- `glPolygonMode(GL_FRONT_AND_BACK, GL_LINE)` — desenha polígonos só com as **arestas**
  (aramado). `glRectf(x1,y1,x2,y2)` — retângulo entre dois cantos.

### 1.5 A pilha de matrizes e a ORDEM das transformações (o conceito-chave do cone)
- `glTranslatef`, `glRotatef`, `glScalef` **multiplicam** a matriz atual (ModelView).
- `glPushMatrix()` salva a matriz; `glPopMatrix()` restaura. Servem para uma transformação
  não "vazar" para o resto da cena.
- **Regra de ouro:** a transformação escrita **por último** (mais perto do desenho) é a
  **primeira** aplicada ao objeto. Ou seja, leia de baixo para cima.

Exemplo com o cone (código de cima para baixo):
```
glTranslatef(x1,y1,0)      (6º a agir no vértice: leva pro clique)
glRotatef(angleZ/Y/X)      (5º: rotações do usuário)
glScalef(scale)            (4º: escala do usuário)
glRotatef(dirAngle-90, Z)  (3º: aponta o cone na direção do mouse)
glRotatef(70, X)           (2º: inclinação 3D)
glTranslatef(0,0,-height)  (1º a agir: encosta o ápice na origem)
glutWireCone(...)          (vértices "crus" do cone)
```
Isto é, o cone é: **construído → recuado (ápice na origem) → inclinado → apontado ao mouse → escalado/rotacionado → posicionado no clique.**

---

## Parte 2 — Arquitetura do `canvas.cpp` (visão geral)

O programa repete um **mesmo padrão** para cada primitiva (ponto, linha, retângulo, cone):

1. Uma **classe** (`Point`, `Line`, `Rectangle`, `Cone`) que guarda os dados e tem um
   método `draw...()`.
2. Um **`vector`** com todas as instâncias já criadas (`points`, `lines`, ...).
3. Um **iterador** e uma função `drawXxx()` que percorre o vetor desenhando cada item.
4. Uma **caixa de seleção** na barra esquerda (`drawXxxSelectionBox`) com um **ícone**.

Fluxo de uso:
- `mouseControl` decide: clicou na **barra** → `pickPrimitive` escolhe a primitiva;
  clicou no **canvas** → cria/point/começa a primitiva.
- `pointCount` é uma **máquina de estados** de 2 cliques: `0` = esperando 1º ponto,
  `1` = já tem o 1º ponto e espera o 2º (é nesse estado que aparece a pré-visualização).
- `drawScene` desenha tudo, na ordem certa.

---

## Parte 3 — SUA PARTE: o CONE, destrinchado

### 3.1 A classe `Cone` (dois pontos, como `Line`/`Rectangle`)
```c
class Cone {
  Cone(int x1,y1,x2,y2);         // (x1,y1)=vértice; (x2,y2)=mouse
  void drawCone();
  void rotateX/Y/Z(float a);     // acumulam ângulos
  void changeScale(float f);     // multiplica a escala
  void translate(int dx,int dy); // move os DOIS pontos juntos
private:
  int x1,y1,x2,y2;
  float angleX,angleY,angleZ,scale;
};
```
- Guarda **dois pontos** exatamente como `Line`/`Rectangle` (mesma "cara" do original).
- Guarda também o **estado de transformação** próprio: 3 ângulos + escala. Cada cone
  lembra como foi girado/escalado.
- `translate` move `(x1,y1)` **e** `(x2,y2)` juntos → o cone inteiro anda sem deformar.
- Do vetor entre os dois pontos saem **duas coisas**: o **tamanho** (distância, `sqrt`) e a
  **direção** para onde o cone aponta (ângulo, `atan2`).

> **Pergunta típica:** "Por que dois pontos e não centro+raio?" → Para seguir o mesmo
> modelo de `Line`/`Rectangle` do canvas; do vetor entre os dois pontos saem o tamanho
> (distância) e a direção (ângulo), como o retângulo usa dois cantos.

### 3.2 `drawCone()` — o coração (linha a linha)
```c
float dx = x2 - x1;
float dy = y2 - y1;
float height   = sqrt(dx*dx + dy*dy);      // (A)
float radius   = 0.40 * height;            // (B)
float dirAngle = atan2(dy, dx) * 180/PI;   // (C)
glPushMatrix();
  glTranslatef(x1, y1, 0);           // (D)
  glRotatef(angleZ,0,0,1);           // (E)
  glRotatef(angleY,0,1,0);
  glRotatef(angleX,1,0,0);
  glScalef(scale,scale,scale);       // (F)
  glRotatef(dirAngle - 90, 0,0,1);   // (G)
  glRotatef(70, 1,0,0);              // (H)
  glTranslatef(0,0,-height);         // (I)
  glutWireCone(radius, height, 20, 12); // (J)
glPopMatrix();
```
- **(A) `height`** = distância do vértice ao mouse, pela **fórmula da distância**
  `√(dx²+dy²)`. É a **mesma ideia** do `MouseMotion.cpp`, que calcula o raio do círculo
  com `sqrt`. Quanto mais longe o mouse, maior o cone.
- **(B) `radius = 0.40*height`** — o raio da base é 40% da altura, só para o cone ficar
  com proporção agradável (nem "gordo", nem "agulha"). É uma escolha estética.
- **(C) `dirAngle = atan2(dy,dx)`** — o **ângulo** (em graus) da direção do vetor
  vértice→mouse. `atan2(dy,dx)` transforma um vetor `(dx,dy)` num ângulo; é a
  **trigonometria de transformações vista no Cap. 4 do Sumanta Guha**. Multiplicamos por
  `180/PI` porque `atan2` devolve **radianos** e o `glRotatef` quer **graus**.
- **(D)** leva a origem local para o ponto do **clique** (onde ficará o vértice).
- **(E)** rotações do **usuário** (teclas x/y/z) — giram o cone em torno do vértice.
- **(F)** escala do **usuário** (teclas +/-).
- **(G) `glRotatef(dirAngle - 90, 0,0,1)`** — gira o cone em torno de Z para ele **apontar
  na direção do mouse**. O `-90` é porque a referência "para cima" está a 90°; assim, se o
  mouse está a `dirAngle`, o cone gira `dirAngle - 90` e passa a apontar para lá. É isso que
  faz o cone mudar de **direção** conforme o mouse (como o retângulo muda de lado).
- **(H) `glRotatef(70, 1,0,0)`** — inclina 70° em torno de X. Isso faz a base circular
  aparecer como **elipse** (dá o efeito 3D na tela 2D).
- **(I) `glTranslatef(0,0,-height)`** — o `glutWireCone` desenha a **base em z=0** e o
  **ápice em z=altura**. Recuando o cone em `-height`, o **ápice vai para a origem**
  (o clique). Sem isso, quem ficaria no clique seria o centro da base.
- **(J) `glutWireCone(raio, altura, fatias, aneis)`** — desenha o cone **aramado**.
  `20` fatias (divisões ao redor) e `12` anéis (divisões na altura). É a **mesma função**
  usada em `Cap4/Objects.cpp` (`glutWireCone(3,8,30,30)`) e `Cap4/Clown.cpp`.

> **Atenção à ordem:** lembre da regra "a última transformação escrita é a primeira aplicada
> ao vértice". Lendo de baixo p/ cima: o cone é **recuado (I)** → **inclinado (H)** →
> **apontado ao mouse (G)** → **escalado/rotacionado pelo usuário (F,E)** → **posto no
> clique (D)**.

> **[Procedência p/ a prova]** O `atan2` **não está no `canvas.cpp` base**, mas é a
> **trigonometria de transformações vista no Cap. 4 do Sumanta Guha** (o livro da
> disciplina). Ele é usado só para converter o vetor `(dx,dy)` em um ângulo e orientar o
> cone. Guarde essa origem para justificar seu uso, do mesmo modo que o
> `glutPassiveMotionFunc` (ver 3.4).

> **Pergunta típica:** "Por que aramado e não sólido?" → O canvas é 2D, **sem iluminação
> e sem depth-test**. Um `glutSolidCone` sem luz sairia como uma mancha preta. O aramado
> mostra a forma do cone sem precisar de luz — fiel ao estilo do canvas.

### 3.3 O ícone do cone (`drawConeSelectionBox`)
- Desenha a **4ª caixa** (faixa `0.6h`..`0.7h`) e, dentro, um **cone visto de lado**:
  - **Laterais**: um `GL_LINE_STRIP` de 3 pontos (base-esq → ápice → base-dir).
  - **Base elíptica**: um `GL_LINE_LOOP` com `cos/sin` (mesma técnica do círculo em
    `circulo.cpp`/`MouseMotion.cpp`), achatado no eixo y para parecer elipse.
- O `if (primitive == CONE)` pinta a caixa de **branco** quando o cone está selecionado
  (destaque), senão cinza — igual às outras caixas.

### 3.4 Pré-visualização em tempo real — **ITEM I** (a parte mais cobrada)
Três peças trabalham juntas:
1. **`curX, curY`** (globais `[T1]`): guardam a posição atual do mouse.
2. **`mouseMotion(x,y)`**: callback chamado a cada movimento. Se `pointCount == 1`
   (já dei o 1º clique, falta o 2º), atualiza `curX,curY` (convertendo `y = height - y`)
   e chama `glutPostRedisplay()` → a tela redesenha.
3. **`drawPreview()`** dentro do `drawScene`: se `pointCount == 1`, desenha em **vermelho**
   a primitiva do 1º ponto até o mouse. Para o cone, cria um `Cone temp(tempX,tempY,curX,curY)`
   e chama `temp.drawCone()`.

**Por que `glutPassiveMotionFunc` E `glutMotionFunc`?**
- `glutMotionFunc` só dispara com o **botão pressionado**. Mas entre os dois cliques o
  botão está **solto**. Por isso é preciso o `glutPassiveMotionFunc` (movimento **sem**
  botão), que é o que garante o preview aparecer. Registramos os dois por robustez.
- **[Honestidade p/ a prova]** `glutPassiveMotionFunc` é a única função que **não aparece
  literalmente** nos códigos da aula (mas o `glutMotionFunc` aparece no `MouseMotion.cpp`).
  Ela é a "irmã passiva" dele e é **exigida pelo enunciado** (ver a mudança antes do clique
  final, com o botão solto). Essa é a justificativa se a professora perguntar.

### 3.5 Transformações geométricas (teclado) — **ITEM II**
- **`keyInput`** trata `x X y Y z Z` (rotações, ±5°) e `+ -` (escala, ×1.1 / ×0.9).
  Cada tecla chama o método no **último cone**: `cones.back()`.
- **`specialKeyInput`** (setas) chama `translate` no último cone.
- **`cones.back()`** = o cone **ativo** = o último desenhado. O `if (!cones.empty())`
  evita erro quando ainda não há nenhum cone.
- Internamente as transformações mexem em `angleX/Y/Z` e `scale`, que o `drawCone` aplica
  com `glRotatef`/`glScalef` — **exatamente como em `Cap4/Objects.cpp`** (que gira objetos
  com x/y/z).

> **Pergunta típica:** "Se eu apertar 'x' e não houver cone, o que acontece?" → Nada, por
> causa do `if (!cones.empty())`. "Qual cone gira?" → Sempre o último desenhado.

### 3.6 Colocação do cone (dois cliques) — dentro do `mouseControl`
- 1º clique (`pointCount == 0`): guarda `tempX,tempY` (vértice) e inicia `curX,curY`;
  `pointCount++`.
- Move o mouse → `mouseMotion` atualiza o preview.
- 2º clique (`pointCount == 1`): `cones.push_back(Cone(tempX,tempY,x,y))` e zera `pointCount`.
- É o **mesmo esquema** de linha e retângulo (dois cliques).

### 3.7 Por que o cone não invade a barra de seleção (ordem de desenho) `[T1]`
No `drawScene`, a **barra de seleção é desenhada por ÚLTIMO**. Como as caixas são
retângulos **preenchidos** (`glRectf` com `GL_FILL`), elas **cobrem** qualquer parte do
cone que tenha vazado para a área da aba. Resultado: o cone é recortado no limite da barra
e nunca desenha por cima dos ícones. *(No original a barra vinha primeiro.)* É o
**algoritmo do pintor**: quem é desenhado por último fica por cima.

### 3.8 `resize` — o ajuste do near/far `[T1]`
Trocamos `glOrtho(...,-1,1)` por `glOrtho(...,-maxDim,maxDim)`. Motivo: o cone inclinado
ocupa profundidade (z) de várias dezenas/centenas de pixels; com o intervalo `-1..1` ele
seria **cortado**. As primitivas 2D estão em `z=0`, então **não são afetadas**.

---

## Parte 4 — Perguntas prováveis da prova (com respostas prontas)

1. **Como um cone 3D aparece numa tela 2D ortográfica?** Desenhado em **aramado**, com uma
   **inclinação fixa** (`glRotatef(70,1,0,0)`) que transforma a base circular em elipse.
   Não precisa de luz nem depth-test.
2. **Onde fica o vértice do cone?** No **ponto do 1º clique**. Isso é garantido pelo
   `glTranslatef(0,0,-height)`, que leva o ápice à origem local.
3. **O que o mouse define no cone?** As **duas coisas**, a partir do vetor 1º ponto→mouse:
   o **tamanho** (distância, `sqrt(dx²+dy²)`, como no `MouseMotion.cpp`) e a **direção**
   para onde ele aponta (ângulo, `atan2(dy,dx)`, trigonometria do Cap. 4 do Sumanta Guha).
   Igual ao retângulo, que muda de tamanho e de lado conforme o mouse.
4. **Por que a barra de seleção é desenhada por último?** Para **cobrir** (recortar)
   qualquer parte do cone que invada a área da barra (algoritmo do pintor).
5. **Diferença entre `glutWireCone` e `glutSolidCone`?** Wire = só arestas (aramado);
   Solid = preenchido (precisa de iluminação para ficar bom). Usamos wire.
6. **Por que dois callbacks de movimento do mouse?** `glutMotionFunc` só dispara com botão
   pressionado; entre os dois cliques o botão está solto, então é preciso o
   `glutPassiveMotionFunc`.
7. **O que faz `glPushMatrix`/`glPopMatrix` no `drawCone`?** Isolam as transformações do
   cone para não afetarem o resto da cena (cada cone desenha e "desfaz" sua matriz).
8. **Se eu inverter a ordem de duas transformações, muda?** Sim — transformações não são
   comutativas. A última escrita é a primeira aplicada ao objeto.
9. **Por que `radius = 0.40*height`?** Só proporção estética para manter forma de cone.
10. **O que é `cones.back()`?** O último cone do vetor = o "cone ativo" que o teclado
    transforma.
11. **Para que serve o `atan2` no cone e de onde vem?** Serve para achar a **direção** do
    cone: converte o vetor `(dx,dy)` (vértice→mouse) num **ângulo** que orienta o cone com
    `glRotatef`. É trigonometria de transformações vista no **Cap. 4 do Sumanta Guha**.
    (Multiplica-se por `180/PI` porque `atan2` devolve radianos e `glRotatef` usa graus.)

---

## Parte 5 — Glossário rápido (função → o que faz → onde aparece na aula)

| Função | O que faz | Aparece em (aula) |
|---|---|---|
| `glutWireCone(r,h,fatias,aneis)` | desenha cone aramado | Objects.cpp, Clown.cpp, SpaceTravel.cpp |
| `glPushMatrix`/`glPopMatrix` | salva/restaura matriz | Objects.cpp, Robot.cpp, ... |
| `glTranslatef`/`glRotatef`/`glScalef` | translada/gira/escala | Objects.cpp, Rotacao*.cpp, ... |
| `glutMotionFunc` | mouse movendo (botão apertado) | MouseMotion.cpp |
| `glutPassiveMotionFunc` | mouse movendo (botão solto) | **não está na aula** (irmã do acima; exigida p/ Item I) |
| `glutSpecialFunc` / `GLUT_KEY_*` | teclas de seta | Objects.cpp, TorusBall.cpp, ... |
| `sqrt` | raiz (distância = tamanho) | MouseMotion.cpp |
| `atan2(dy,dx)` | vetor → ângulo (direção) | trigonometria do **Cap. 4 do Sumanta Guha** |
| `cos`/`sin` | círculo/elipse | circulo.cpp, MouseMotion.cpp |
| `glOrtho` | projeção 2D | canvas.cpp (original), Mouse.cpp, ... |
| `glRectf`, `glBegin/glVertex/glEnd` | desenho imediato | canvas.cpp (original) |

---

## Parte 6 — Roteiro de estudo (como treinar)

1. **Leia a Parte 1** e saiba explicar em voz alta: o laço do GLUT, a conversão de `y`, e a
   **ordem das transformações**. Sem isso, o `drawCone` não faz sentido.
2. **Cubra o `drawCone` com a mão** e tente reescrever de memória as etapas (A)–(H).
3. **Explique o Item I** apontando as 3 peças: `curX/curY`, `mouseMotion`, `drawPreview`.
4. **Simule perguntas** da Parte 4 respondendo sem olhar.
5. **Rode e brinque**: desenhe cones, aperte x/y/z/+/-/setas, veja o preview em vermelho,
   desenhe um cone grande na borda esquerda e confirme que ele é recortado na barra.
6. **Compile** para fixar o comando:
   ```bash
   g++ canvas.cpp -o canvas -lglut -lGL -lGLU -lm
   ./canvas
   ```

> Dica: se você consegue explicar **por que** cada uma das etapas (A)–(H) do `drawCone`
> existe, e as **3 peças** do Item I, você domina a sua parte do trabalho.
