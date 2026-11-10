#define _USE_MATH_DEFINES  // Garante a inclusão de M_PI em compiladores baseados em MSVC/Windows
#include <GL/glut.h>
#include <math.h>

// Garante que M_PI existirá mesmo se o compilador omitir a macro acima
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Ângulo de rotação para movimentar as luzes
float angulo = 0.0f;


// Configurações globais de iluminação e materiais
void init() {
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f); // Fundo quase totalmente escuro
    glEnable(GL_DEPTH_TEST);                 // Habilita o buffer de profundidade
    glEnable(GL_LIGHTING);                   // Habilita o sistema de iluminação do OpenGL
    
    // Suaviza a tonalidade das faces iluminadas
    glShadeModel(GL_SMOOTH);                 //
    
    // Habilita o rastreamento de cor para que os materiais aceitem glColor3f
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
}

// Configura e posiciona um Spot Light
void configurarSpot(GLenum luz, float r, float g, float b, float posX, float posZ, float dirX, float dirZ) {
    // Cores da luz (Difusa e Especular com a cor do Spot)
    float corDifusa[] = { r, g, b, 1.0f };
    float corEspecular[] = { r, g, b, 1.0f };
    float corAmbiente[] = { 0.05f, 0.05f, 0.05f, 1.0f };

    // Posição da lâmpada no "teto" (Y = 5.0)
    float posicao[] = { posX, 5.0f, posZ, 1.0f }; // O quarto argumento '1.0' define como luz posicional
    
    // Direção para onde o cone aponta (aponta ligeiramente para baixo e para os lados)
    float direcao[] = { dirX, -1.0f, dirZ };

    // Configurando as propriedades da luz no OpenGL
    glLightfv(luz, GL_AMBIENT, corAmbiente);
    glLightfv(luz, GL_DIFFUSE, corDifusa);
    glLightfv(luz, GL_SPECULAR, corEspecular);
    glLightfv(luz, GL_POSITION, posicao);

    // Transformando a luz em SPOT (Cone de iluminação)
    glLightf(luz, GL_SPOT_CUTOFF, 25.0f);      // Ângulo do cone (abertura de 25 graus)
    glLightfv(luz, GL_SPOT_DIRECTION, direcao); // Direção do feixe
    glLightf(luz, GL_SPOT_EXPONENT, 15.0f);    // Concentração da luz no centro do cone

    glEnable(luz); // Ativa esta luz específica
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Posiciona a câmera olhando de cima e um pouco mais afastada para ver o "teto"
    gluLookAt(0.0, 7.0, 12.0,  0.0, 1.0, 0.0,  0.0, 1.0, 0.0);

    // --- CÁLCULO E CONFIGURAÇÃO DOS SPOTS (ILUMINAÇÃO) ---

    // Luz 1: Vermelha
    float x1 = 3.0f * cosf(angulo);
    float z1 = 3.0f * sinf(angulo);
    configurarSpot(GL_LIGHT0, 1.0f, 0.0f, 0.0f, x1, z1, -x1*0.5f, -z1*0.5f);

    // Luz 2: Verde
    float x2 = 3.0f * cosf(angulo + 2.0f);
    float z2 = 3.0f * sinf(angulo + 2.0f);
    configurarSpot(GL_LIGHT1, 0.0f, 1.0f, 0.0f, x2, z2, -x2*0.5f, -z2*0.5f);

    // Luz 3: Azul
    float x3 = 3.0f * cosf(-angulo + 4.0f);
    float z3 = 3.0f * sinf(-angulo + 4.0f);
    configurarSpot(GL_LIGHT2, 0.0f, 0.0f, 1.0f, x3, z3, -x3*0.5f, -z3*0.5f);


    // --- DESENHO CORPOLÓGICO DAS LÂMPADAS (VISÍVEIS NO TETO) ---
    // Desativamos a iluminação para que as lâmpadas tenham cor sólida (pareçam emitir luz)
    glDisable(GL_LIGHTING);

    // Desenha Lâmpada Vermelha
    glColor3f(1.0f, 0.0f, 0.0f);
    glPushMatrix();
        glTranslatef(x1, 5.0f, z1); // Mesma posição da LIGHT0
        glutSolidSphere(0.2, 16, 16);
    glPopMatrix();

    // Desenha Lâmpada Verde
    glColor3f(0.0f, 1.0f, 0.0f);
    glPushMatrix();
        glTranslatef(x2, 5.0f, z2); // Mesma posição da LIGHT1
        glutSolidSphere(0.2, 16, 16);
    glPopMatrix();

    // Desenha Lâmpada Azul
    glColor3f(0.0f, 0.0f, 1.0f);
    glPushMatrix();
        glTranslatef(x3, 5.0f, z3); // Mesma posição da LIGHT2
        glutSolidSphere(0.2, 16, 16);
    glPopMatrix();

    // Reativamos a iluminação para renderizar o restante da pista de dança corretamente
    glEnable(GL_LIGHTING);


    // --- DESENHO DA CENA (Pista e Objetos) ---
    
    // Desenha o piso refletor
    glColor3f(0.4f, 0.4f, 0.4f);
    glBegin(GL_QUADS);
        glNormal3f(0.0f, 1.0f, 0.0f);
        glVertex3f(-5.0f, -1.0f, -5.0f);
        glVertex3f(-5.0f, -1.0f,  5.0f);
        glVertex3f( 5.0f, -1.0f,  5.0f);
        glVertex3f( 5.0f, -1.0f, -5.0f);
    glEnd();

    // Desenha os objetos no centro
    glColor3f(0.7f, 0.7f, 0.7f);
    
    glPushMatrix();
        glTranslatef(-1.5f, -0.2f, 0.0f);
        glutSolidSphere(0.8, 40, 40);
    glPopMatrix();

    glPushMatrix();
        glTranslatef(1.5f, -0.2f, 0.0f);
        glRotatef(-90, 1, 0, 0);
        glutSolidCone(0.7, 1.5, 40, 40);
    glPopMatrix();

    glPushMatrix();
        glTranslatef(0.0f, 0.1f, -1.5f);
        glutSolidTeapot(0.7);
    glPopMatrix();

    glutSwapBuffers();
}

// Atualiza o ângulo da animação de forma contínua
void atualizar(int valor) {
    angulo += 0.03f; // Velocidade da rotação das lâmpadas discoteca
    if (angulo > 2 * M_PI) angulo -= 2 * M_PI;

    glutPostRedisplay(); // Avisa o GLUT que a tela precisa ser redesenhada
    glutTimerFunc(16, atualizar, 0); // Mantém a animação travada a ~60 FPS
}

// Redimensionamento de tela padrão
void reshape(int w, int h) {
    glViewport(0, 0, w, h); //
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)w / (double)h, 1.0, 20.0); //
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv); //
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); // Usa Double Buffer para animações fluidas
    glutInitWindowSize(800, 600); //
    glutCreateWindow("Pista de Discoteca Retro - OpenGL Spots"); //

    init();

    glutDisplayFunc(display); //
    glutReshapeFunc(reshape);
    glutTimerFunc(0, atualizar, 0); // Inicia o timer de animação

    glutMainLoop(); //
    return 0;
}

