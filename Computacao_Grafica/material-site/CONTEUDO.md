# Conteúdo ensinado — base para a PP4

Este arquivo resume o que foi ensinado nas aulas de Computação Gráfica (Profa. Mercedes, UEMS 2026).
O resumo cobre só os temas que a PP4 cobra. Ele foi escrito a partir dos slides, listas e códigos desta pasta.
Cada seção diz **o que foi ensinado**, **quais funções foram usadas** e **onde está o material**.

> Regra de ouro: se um recurso não aparece aqui nem nos arquivos citados, ele não foi visto em aula.
> A única exceção permitida pelo enunciado é a **textura** (seção 9).

---

## 0. O que a PP4 pede

Enunciado completo: `00-enunciados/PP4-CG-2026.pdf`.

**PL4 (2 pontos)** — proposta de projeto em slides, com o máximo de detalhes sobre cinco itens:

1. Cenário com modelos geométricos 3D (primitivas da GLUT, quádricas da GLU ou curvas/superfícies de Bézier).
2. Animação.
3. Uso de z-buffer para visualização realista da profundidade.
4. Iluminação.
5. Textura (não foi vista em aula; o aluno pesquisa uma técnica adequada).

A ideia inicial precisa ser autorizada pela professora antes.

**PP4 (4 pontos)** — implementar a proposta aprovada em OpenGL e C/C++.

**PO4 (4 pontos)** — perguntas da professora depois das apresentações.

**Apresentação** — 10 min de PL4 (apresentação 1 pt + domínio do assunto 1 pt).
Depois, 10 min de PP4 com as rotinas principais (domínio do código 2 pts + avaliação do código 2 pts).
Slides e código vão por e-mail no dia da apresentação, até as 8h, com assunto "PL4 e PP4 - nome do aluno".
Victor apresenta em **19/11/2026** (aluno nº 12).

**Modelo de prova anterior:** `00-enunciados/PP2-CG-2026.pdf`.
Ele mostra o estilo da professora: base de código obrigatória das aulas e avaliação do domínio do próprio código.

---

## Linha do tempo das aulas (temas da PP4)

| Data | Tema | Material |
|---|---|---|
| 09/04 – 23/04 | Modelagem: curvas, Bézier, superfícies | `01-modelagem-geometrica/`, lista `Lista Modelagem Geometrica.pdf` |
| 04/05 | Introdução ao OpenGL e GLUT | `02-opengl/CG-OpenGL1.pdf` |
| 07/05 | Projeção ortográfica, volume de visualização, recorte, cor | `CG-OpenGL1.pdf` + Experimenter cap. 2 |
| 11/05 – 21/05 | Primitivas, objetos curvos, buffer de profundidade, perspectiva, hemisfério | `02-opengl/CG-OpenGL2.pdf`, lista `L11.pdf` |
| 25/05 | Listas de exibição, fontes, mouse, menus, pontilhado, teclas não ASCII | `CG-OpenGL2.pdf` |
| 01/06 – 18/06 | Objetos GLUT/GLU, transformações, hierarquia, Bézier em OpenGL | `02-opengl/CG-OpenGL3.pdf`, `superficies.cpp`, `torus.c` |
| 22/06 – 06/07 | Animação e câmera (`gluLookAt`) | `03-animacao/CG-animacao.pdf`, lista `L22-CG-2026.pdf` |
| 24/09 | Cor (RGB, HSV) | `04-cor/CG-cor.pdf` |
| 01/10 | Visibilidade (back-face culling, z-buffer) | `05-visibilidade/` |
| 08/10 – 29/10 | Iluminação | `06-iluminacao/` |
| 05/11 | Definição de projetos | — |

---

## 1. Estrutura de um programa OpenGL/GLUT

Fonte: `02-opengl/CG-OpenGL1.pdf` (programa `square.cpp`, Experimenter cap. 2).

GLUT é orientada a eventos. O `main` inicializa a janela, registra callbacks e entra no laço de eventos.

```c
glutInit(&argc, argv);
glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); // DOUBLE: animação; DEPTH: z-buffer
glutInitWindowSize(500, 500);
glutInitWindowPosition(100, 100);
glutCreateWindow("nome");
setup();                        // estados iniciais: cor de fundo, luzes, texturas
glutDisplayFunc(drawScene);     // desenho
glutReshapeFunc(resize);        // janela redimensionada
glutKeyboardFunc(keyInput);     // teclado
glutMainLoop();
```

Callbacks vistos: `glutDisplayFunc`, `glutReshapeFunc`, `glutKeyboardFunc`, `glutSpecialFunc` (teclas não ASCII),
`glutMouseFunc`, `glutMotionFunc`, `glutPassiveMotionFunc`, `glutIdleFunc`, `glutTimerFunc`.
`glutPostRedisplay()` pede um novo desenho.

**OpenGL é uma máquina de estados.** Cor, espessura de linha e material ficam valendo até serem trocados.
Objetos são desenhados na ordem do código.

**Compilação no Linux** (slide): `g++ -o prog prog.cpp -lglut -lGL -lGLU -lm`.

---

## 2. Primitivas, objetos curvos e projeções

Fonte: `02-opengl/CG-OpenGL2.pdf`, lista `02-opengl/listas/L11.pdf`.

- **Primitivas:** `GL_POINTS`, `GL_LINES`, `GL_LINE_STRIP`, `GL_LINE_LOOP`, `GL_TRIANGLES`,
  `GL_TRIANGLE_STRIP`, `GL_TRIANGLE_FAN`, `GL_QUADS`, `GL_QUAD_STRIP`, `GL_POLYGON`.
- **Modo de desenho:** `glPolygonMode(face, GL_FILL | GL_LINE | GL_POINT)`. A face da frente depende da orientação dos vértices.
- **Objetos curvos aproximados:** amostrar a equação paramétrica e ligar os pontos.
  Círculo: `x = X + R cos t`, `y = Y + R sin t`, com `t = 2πi/N` (`circle.cpp`).
  Hélice: `x = R cos t`, `y = R sin t`, `z = t − 60` (`helix.cpp`).
- **Hemisfério** (`hemisphere.cpp`, Guha cap. 3): malha de `(p+1)(q+1)` pontos.
  A longitude é `θ = 2πi/p` e a latitude é `φ = (π/2)·j/q`.
  O ponto é `x = R cos φ cos θ`, `y = R sin φ`, `z = R cos φ sin θ`, desenhado com `GL_TRIANGLE_STRIP`.
- **Projeção ortográfica:** `glOrtho(left, right, bottom, top, near, far)`.
- **Projeção perspectiva:** `glFrustum(left, right, bottom, top, near, far)`, com `0 < near < far`.
  Os códigos de iluminação usam a forma equivalente `gluPerspective(fovy, aspect, near, far)`.
  Perspectiva é mais realista porque objetos distantes ficam menores.
- **Listas de exibição:** `glGenLists`, `glNewList(id, GL_COMPILE)`, `glEndList`, `glCallList`.
  Exemplos: `torus.c`, `helixList.cpp`, `multipleLists.cpp`.
- **Interação:** fontes (`fonts.cpp`), mouse (`mouse.cpp`, `mouseMotion.cpp`), menus (`menus.cpp`),
  pontilhado (`lineStipple.cpp`) e o integrador `canvas.cpp` (base da PP2). Todos estão no Guha cap. 3.

---

## 3. Modelagem 3D (item 1 da PP4)

Fontes: `02-opengl/CG-OpenGL3.pdf`, `01-modelagem-geometrica/`, Guha cap. 10.

### 3.1 Objetos prontos da GLUT
Todos existem em versão `Wire` (aramado) e `Solid`. Para iluminação, use `Solid`, que já gera normais.

`glutSolidSphere(r, slices, stacks)`, `glutSolidCube(size)`, `glutSolidCone(r, h, slices, stacks)`,
`glutSolidTorus(rInterno, rExterno, nsides, rings)`, `glutSolidDodecahedron()`, `glutSolidOctahedron()`,
`glutSolidTetrahedron()`, `glutSolidIcosahedron()`, `glutSolidTeapot(size)`.
Exemplo: `glutObjects.cpp` (Guha cap. 3/10).

### 3.2 Quádricas da GLU
```c
GLUquadricObj *q = gluNewQuadric();
gluSphere(q, raio, slices, stacks);
gluCylinder(q, raioBase, raioTopo, altura, slices, stacks); // raioTopo = 0 gera cone
gluDisk(q, raioInterno, raioExterno, slices, loops);        // raioInterno != 0 gera furo
gluPartialDisk(...);                                        // usado nas barbatanas do torpedo
```
Exemplos: `gluQuadrics.cpp`, `torpedo.cpp` (Guha cap. 10).

### 3.3 Malhas a partir de equações
Superfícies são amostradas por dois parâmetros `(u, v)` e desenhadas com faixas de triângulos.
Exemplos do Guha cap. 10: `cylinder.cpp`, `torus.cpp`, `helicalPipe.cpp`, `doublyCurledCone.cpp`,
`hyperboloid1sheet.cpp`, `bilinearPatch.cpp`, `extrudedHelix.cpp`, `table.cpp` (superfície de revolução).

**Varredura (sweeping)** — slides de modelagem:
- *Extrusão:* uma curva geratriz desliza ao longo de um caminho.
- *Revolução:* a curva gira em torno de um eixo. Teoria em `01-modelagem-geometrica/SuperficiesRevolucao.pdf`.

### 3.4 Curvas de Bézier
- Definição: `P(t) = Σ Bᵢ · Jₙ,ᵢ(t)`, com `0 ≤ t ≤ 1` e `n + 1` pontos de controle.
- Pesos de Bernstein: `Jₙ,ᵢ(t) = C(n,i) · (1−t)ⁿ⁻ⁱ · tⁱ`. Eles somam 1.
- A curva passa pelo primeiro e pelo último ponto e fica dentro do fecho convexo.
- Cúbica (n = 3): `P(t) = (1−t)³B₀ + 3t(1−t)²B₁ + 3t²(1−t)B₂ + t³B₃`.
- Algoritmo de **de Casteljau**: interpolações lineares sucessivas entre pontos de controle.
- Problemas: falta de controle local e grau que cresce com o número de pontos.
- Implementação "na mão": `02-opengl/codigos/superficies.cpp` (pontos por clique, Bernstein calculado no código).

### 3.5 Superfícies de Bézier
- Definição: `Q(u,v) = Σᵢ Σⱼ Bᵢⱼ · Jₙᵢ(u) · Kₘⱼ(v)` no domínio `[0,1]×[0,1]`.
- Em OpenGL (avaliadores):
  ```c
  glMap2f(GL_MAP2_VERTEX_3, 0, 1, 3, 4, 0, 1, 12, 4, &pontosControle[0][0][0]);
  glEnable(GL_MAP2_VERTEX_3);
  glMapGrid2f(20, 0.0, 1.0, 20, 0.0, 1.0);
  glEvalMesh2(GL_FILL, 0, 20, 0, 20);
  ```
  Para curvas: `glMap1f`, `glMapGrid1f`, `glEvalMesh1`, `glEvalCoord1f`.
- Com iluminação, use `glEnable(GL_AUTO_NORMAL)` para as normais (`litBezierCanoe.cpp`, Guha cap. 11).
- Exemplos: `bezierCurves.cpp`, `bezierCurveWithEvalMesh.cpp`, `bezierSurface.cpp`, `bezierCanoe.cpp` (cap. 10).
- Modelo combinado sugerido em aula: `torpedo.cpp` (cilindro, hemisfério, discos e pás de Bézier).
  O exercício seguinte pedia um **avião** com superfícies de Bézier e quádricas.

### 3.6 Lista de modelagem
Arquivo: `01-modelagem-geometrica/listas/Lista Modelagem Geometrica.pdf`.
Cobra representação paramétrica × não paramétrica e implícita × explícita.
Cobra também o papel dos pontos de controle, dos polinômios de Bernstein e dos coeficientes binomiais.
Pede o algoritmo de de Casteljau e uma curva de Bézier com 4 pontos dados.
Pede gomos e cortes do hemisfério, o toro como superfície de revolução
e superfícies de revolução com geratriz qualquer ou de Bézier.

---

## 4. Transformações, hierarquia e câmera

Fontes: `02-opengl/CG-OpenGL3.pdf`, `03-animacao/CG-animacao.pdf`.

- `glTranslatef(x, y, z)`, `glScalef(sx, sy, sz)` (escala negativa espelha), `glRotatef(ângulo, x, y, z)`.
- A **ordem importa**. A última transformação escrita é a primeira aplicada ao objeto.
- **Pilha de matrizes:** `glPushMatrix()` / `glPopMatrix()` isolam as transformações de cada peça.
  Exemplo da aula: carro → rodas → 5 parafusos por roda.
- **Modos de matriz:** `glMatrixMode(GL_PROJECTION)` para a projeção e `glMatrixMode(GL_MODELVIEW)` para objetos e câmera.
- **Câmera:** `gluLookAt(eyeX, eyeY, eyeZ, centroX, centroY, centroZ, upX, upY, upZ)`.
  A alternativa é mover a cena com `glTranslatef` e `glRotatef`.
- Exercícios da aula: mesa com cubos e vistas diferentes, `planet.c` com mais planetas e luas, `robot.c` com mão e dedos.
- Exemplos do Guha cap. 4 (`referencias/sumanta-guha/ExperimenterSource/Chapter4/`): `box.cpp`,
  `composeTransformations.cpp`, `boxWithLookAt.cpp` e os `experiment*.cpp` de escala, rotação, translação e `gluLookAt`.

---

## 5. Animação (item 2 da PP4)

Fonte: `03-animacao/CG-animacao.pdf`, lista `03-animacao/listas/L22-CG-2026.pdf`.

Três formas de controlar a animação:

| Forma | Como | Exemplo |
|---|---|---|
| Interativa | tecla ou mouse altera variável e chama `glutPostRedisplay()` | `rotatingHelix1.cpp` |
| Automática (idle) | `glutIdleFunc(funcao)`; passar `NULL` pausa | `rotatingHelix2.cpp` |
| Automática (timer) | `glutTimerFunc(ms, funcao, valor)`, re-agendada dentro da própria função | `rotatingHelix3.cpp` |

Padrão do timer usado nos slides e em `spots.c`:
```c
void animate(int value) {
    if (isAnimate) { t += 0.01; angle += 5; }
    glutTimerFunc(animationPeriod, animate, 1);
    glutPostRedisplay();
}
```

**Buffer duplo:** `GLUT_DOUBLE` no `glutInitDisplayMode` e `glutSwapBuffers()` no fim do desenho, no lugar de `glFlush()`.
Ele evita o efeito de "fantasma" entre quadros.

**Física vista:** lançamento oblíquo em `throwBall.cpp`, com `x(t) = h·t` e `y(t) = v·t − (g/2)·t²`.

**Exemplos de animação da aula** (Guha cap. 4, em `referencias/sumanta-guha/ExperimenterSource/Chapter4/`):
`rotatingHelix1/2/3.cpp`, `ballAndTorus.cpp`, `throwBall.cpp`, `clown.cpp`, `floweringPlant.cpp`
e `spaceTravel.cpp` (duas viewports, câmera na nave, detecção de colisão).
Os slides citam `clown1/2/3.cpp`, mas a pasta tem só `clown.cpp`, que é o passo final.
O capítulo traz também `animateMan1/2.cpp` (boneco com partes móveis; o `animateMan2` lê `animateManDataIn.txt`).
Versões feitas pelo Victor estão em `../Cap4/` (`BolaRolando`, `Clown`, `FlowerPlan`, `Carro`, `TorusBall`) e em `../SpaceTravel.cpp`, `../Planet.cpp` e `../Robot.cpp`.

**Lista L22:** bola rolando no plano inclinado, bola quicando entre caixas, bola em hélice,
planeta com duas luas e bola de bilhar com física.

---

## 6. Visibilidade e z-buffer (item 3 da PP4)

Fonte: `05-visibilidade/CG-visibilidade.pdf`, `CG-OpenGL2.pdf` (buffer de profundidade), lista `L41-2025.pdf`.

### 6.1 Conceitos
- Algoritmos de visibilidade removem o que não deve ser visto.
- Eles podem ser do **espaço da imagem** (resolução de pixel) ou do **espaço da cena** (resolução da geometria).

### 6.2 Back-face culling (espaço da cena)
- Uma face é da frente se `E · N ≥ 0`, onde `E` é o vetor de visão e `N` é a normal. Se `E · N < 0`, ela é removida.
- Normal da face com pontos P1, P2, P3: `n = u × v`, com `u = P2P3` e `v = P2P1`.
- O método funciona para **um único poliedro convexo fechado**. Ele falha em poliedros côncavos e em cenas com vários objetos.
- Implementação manual: `05-visibilidade/codigos/visibilidade.c` (cubo com rotação e teste `N·L > 0`).

### 6.3 Z-buffer (espaço da imagem)
- Usa duas memórias: o buffer de cor (frame buffer) e o buffer de profundidade (z-buffer).
- Algoritmo dos slides:
  1. Inicializar o z-buffer com a profundidade máxima e o buffer de cor com a cor de fundo.
  2. Rasterizar cada polígono.
  3. Para cada pixel: se a profundidade for **menor** que a guardada, pintar o pixel e atualizar o z-buffer. Senão, ignorar.
- A lista L41 usa outra convenção: z-buffer começa em −∞ e vence o **maior** z.
- Em OpenGL, são três passos obrigatórios:
  ```c
  glutInitDisplayMode(... | GLUT_DEPTH);                 // alocar o buffer
  glEnable(GL_DEPTH_TEST);                               // ligar o teste
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);    // limpar a cada quadro
  ```
  `glDepthFunc(GL_LESS)` muda a regra do teste (padrão: `GL_LESS`).
- Exemplos: `triangles.c` (z-buffer básico), `planes.c` (`glDepthFunc`), `hidplanes.c` (câmera girando com espaço),
  `circularAnnuluses.cpp` (Experimenter 2.22).

---

## 7. Cor

Fonte: `04-cor/CG-cor.pdf`, lista `L41-2025.pdf` (parte de cor).

- Cor é radiação eletromagnética visível, de cerca de 400 a 700 nm.
- O olho tem **cones** (cor, muita luz) e **bastonetes** (tons de cinza, pouca luz).
  Há três tipos de cone, sensíveis a vermelho, verde e azul. Por isso se usam três cores primárias.
- **RGB:** 0 a 255 por componente (em OpenGL, 0.0 a 1.0). R = G = B gera escala de cinza.
- **HSV:** cone com H (matiz, 0° a 360°, vermelho em 0°), S (saturação, 0 a 1; S = 0 é cinza)
  e V (valor, 0 no vértice/preto a 1 na base).
- OpenGL usa RGB: `glColor3f(r, g, b)`, `glClearColor(r, g, b, a)`.

---

## 8. Iluminação (item 4 da PP4)

Fonte: `06-iluminacao/CG-iluminacao.pdf`, lista `L42-2025.pdf`, códigos em `06-iluminacao/codigos/`.

### 8.1 Teoria
- **Fontes de luz:** ambiente (sem direção, ilumina tudo igual), direcional (raios paralelos, como o Sol)
  e pontual (emite em todas as direções a partir de um ponto).
- **Reflexão difusa (Lambert):** depende só do cosseno entre a normal `N` e a direção da luz `L`.
  Não depende do observador.
- **Reflexão especular:** gera o brilho (highlight). Depende da posição do observador.
- **Modelo completo** (somado para cada uma das m luzes):
  `I = Ia·Ka + Id·Kd·cos θ + Ie·Ke·cosⁿ φ`.
  θ é o ângulo entre N e L. φ é o ângulo entre a reflexão R e o observador E. n controla o brilho.
- **Atenuação:** `fator = 1 / (kc + kl·d + kq·d²)`. Vale só para luz posicional.

### 8.2 OpenGL — passos da aula
1. Definir **normais** em cada vértice (`glNormal3f`). Objetos `glutSolid*` e quádricas já geram normais.
2. Criar e posicionar as luzes.
3. Definir o **material** dos objetos.
4. Definir a luz ambiente global e o modelo de iluminação (`glLightModelfv`).

```c
glEnable(GL_LIGHTING);
glEnable(GL_LIGHT0);                         // GL_LIGHT0 ... GL_LIGHT7
glLightfv(GL_LIGHT0, GL_AMBIENT,  amb);
glLightfv(GL_LIGHT0, GL_DIFFUSE,  dif);      // "a cor da luz"
glLightfv(GL_LIGHT0, GL_SPECULAR, spe);      // cor do brilho
glLightfv(GL_LIGHT0, GL_POSITION, pos);      // w = 0: direcional; w = 1: posicional
// Spot:
glLightf (GL_LIGHT1, GL_SPOT_CUTOFF, 25.0);  // abertura do cone (padrão 180 = não é spot)
glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, dir);
glLightf (GL_LIGHT1, GL_SPOT_EXPONENT, 15.0);// concentração no centro
// Atenuação:
glLightf (GL_LIGHT1, GL_CONSTANT_ATTENUATION | GL_LINEAR_ATTENUATION | GL_QUADRATIC_ATTENUATION, valor);
// Material:
glMaterialfv(GL_FRONT, GL_AMBIENT | GL_DIFFUSE | GL_SPECULAR | GL_EMISSION, cor);
glMaterialf (GL_FRONT, GL_SHININESS, 50.0);
// Atalho: glColor3f vira material
glEnable(GL_COLOR_MATERIAL);
glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
glShadeModel(GL_SMOOTH);                     // ou GL_FLAT (sceneflat.c)
```

**Valores padrão das luzes:** ambient (0,0,0,1). Diffuse e specular são (1,1,1,1) só na `GL_LIGHT0`; nas outras, (0,0,0,1).
Position (0,0,1,0). Spot direction (0,0,−1). Exponent 0. Cutoff 180. Atenuação constante 1, linear 0, quadrática 0.

**Valores padrão do material:** ambient (0.2,0.2,0.2,1), diffuse (0.8,0.8,0.8,1), specular (0,0,0,1), shininess 0, emission (0,0,0,1).

### 8.3 Códigos da aula
| Arquivo | O que mostra |
|---|---|
| `light.c` | esfera com material difuso e especular, luz direcional |
| `movelight.c` | luz que gira com o mouse; cubo sem luz marca a fonte |
| `plane.c` | luz posicional (local) × direcional (infinito) sobre um plano |
| `material.c` | 12 materiais diferentes; inclui material **emissivo** |
| `colormat.c` | modo `GL_COLOR_MATERIAL` |
| `lightlab.c` | laboratório com menus para ligar luzes e trocar cores |
| `sccolorlight.c` / `scene.c` | luz colorida sobre cone, esfera e toro (os dois arquivos são iguais) |
| `sceneflat.c` | mesma cena com `GL_FLAT` |
| `teaambient.c`, `teapots.c` | luz ambiente e muitos materiais em chaleiras |
| `spots.c` | 3 spots coloridos animados (ver seção 10) |

Guha cap. 11 traz mais exemplos: `lightAndMaterial1/2.cpp`, `spotlight.cpp`, `litTriangle.cpp`,
`litCylinder.cpp`, `litBezierCanoe.cpp`, `checkeredFloor.cpp` e `shipMovie.cpp`.

**Lista L42:** luz posicional colorida, spot colorido, atenuação, lanterna spot na mão do robô e semáforo com material emissivo.

---

## 9. Textura (item 5 da PP4 — pesquisa própria)

**Não foi vista em aula.** O enunciado pede que o aluno pesquise uma técnica.
A fonte mais próxima do material da disciplina é o **Guha cap. 12**: `referencias/sumanta-guha/ExperimenterSource/Chapter12/`.
Os experimentos 12.1 a 12.18 estão no `Experimenter.pdf`, páginas 87 a 94.

### 9.1 Passos (de `loadTextures.cpp`)
```c
static unsigned int texture[2];
glGenTextures(2, texture);                         // criar ids
glBindTexture(GL_TEXTURE_2D, texture[0]);          // ativar
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);       // ou GL_CLAMP
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);  // ou GL_LINEAR
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, largura, altura, 0, GL_RGB, GL_UNSIGNED_BYTE, dados);
glEnable(GL_TEXTURE_2D);
glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);  // com iluminação: GL_MODULATE
// no desenho:
glBindTexture(GL_TEXTURE_2D, texture[0]);
glBegin(GL_POLYGON);
  glTexCoord2f(0, 0); glVertex3f(-10, -10, 0);
  glTexCoord2f(1, 0); glVertex3f( 10, -10, 0);
  ...
glEnd();
```

### 9.2 De onde vem a imagem
- **Externa (BMP):** a função `getBMPData()` lê BMP **24 bits sem compressão** e troca BGR por RGB.
  Os BMPs do Guha **não estão no servidor da disciplina**. Para usar essa opção, é preciso uma imagem própria.
- **Procedural:** a imagem é gerada no código. Exemplo: tabuleiro 64×64 em `createChessboard()`.
  Não depende de arquivo externo.

### 9.3 Técnicas por exemplo
| Arquivo | Técnica |
|---|---|
| `experimentTexture*.cpp` | coordenadas de textura, repetição (tiling), clamp |
| `fieldAndSky.cpp` | grama repetida no chão e céu ao fundo |
| `fieldAndSkyFiltered.cpp`, `compareFilters.cpp`, `mipmapLevels.cpp` | filtros e mipmaps |
| `fieldAndSkyLit.cpp` | textura + luz direcional (`GL_MODULATE`) |
| `litTexturedCylinder.cpp` | rótulo em cilindro iluminado |
| `texturedTorus.cpp` | textura procedural num toro, **animada** pelas coordenadas |
| `texturedTorpedo.cpp` | textura em quádrica (`gluQuadricTexture(q, GL_TRUE)`) e em Bézier (`GL_MAP2_TEXTURE_COORD_2`) |

---

## 10. Exemplo integrador: `spots.c`

`06-iluminacao/codigos/spots.c` é o único código de iluminação publicado em 2026 e o mais próximo do formato da PP4.
Ele já tem **quatro dos cinco itens**:

| Item da PP4 | Como aparece em `spots.c` |
|---|---|
| 1. Modelos 3D | piso com `GL_QUADS`, `glutSolidSphere`, `glutSolidCone`, `glutSolidTeapot` |
| 2. Animação | `glutTimerFunc(16, atualizar, 0)` gira as luzes (cerca de 60 FPS) |
| 3. Z-buffer | `GLUT_DEPTH`, `glEnable(GL_DEPTH_TEST)`, limpeza do `GL_DEPTH_BUFFER_BIT` |
| 4. Iluminação | 3 spots coloridos (`GL_LIGHT0..2`), `GL_COLOR_MATERIAL`, `GL_SMOOTH` |
| 5. Textura | **ausente** |

Detalhe útil: as lâmpadas são esferas desenhadas com `GL_LIGHTING` desligado, para parecerem acesas.

---

## 11. Lacunas do material neste repositório

- **Texturas BMP do Guha** não existem no servidor.
- Temas fora da PP4 não foram copiados: introdução, transformações geométricas teóricas,
  transformações projetivas, recorte e amostragem.
