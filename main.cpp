#include <GL/glut.h>
#include <cmath>
#include <string>
#include <vector>
#include <utility>
#include <cstdlib>
#include <ctime>
#include <tuple>
#include <cstdlib>

using namespace std;

/* Variaveis e afins */

GLfloat massa1 = 20.0f;  // Massa do planeta maior
GLfloat massa2 = 5.0f;   // Massa do planeta menor
GLfloat raio1 = 0.8f;    // Raio do planeta maior
GLfloat raio2 = 0.2f;    // Raio do planeta menor
GLfloat G = 1.5f;        // Constante gravitacional
GLfloat x_planeta1 = 0.0f, y_planeta1 = 0.0f, z_planeta1 = 0.0f;  // Posição do planeta maior
GLfloat x_planeta2 = 5.0f, y_planeta2 = 0.0f, z_planeta2 = 0.0f;  // Posição inicial do planeta menor
GLfloat vx_planeta2 = 0.0f, vy_planeta2 = 2.0f, vz_planeta2 = 0.0f;  // Velocidade inicial do planeta menor
GLboolean vistaTopo = GL_FALSE;  // Flag para alternar entre as visões
GLboolean planeta2colidiu = GL_FALSE; // Flag para verificar se o planeta menor colidiu
GLfloat dt = 0.01; // da quantidade de frames
const int MAX_TRAIL_LENGTH = 500;  // Tamanho máximo do rastro
GLboolean particlesGenerated = GL_FALSE; // flag para saber se as particulas ja foram geradas
std::vector<std::pair<float, float>> planet2Trail;// Lista para armazenar as posições do trail
std::vector<std::tuple<float, float, float>> explosionParticles; // lista para as particulas de explosao
float explosionTimer = 8.0f; // Duração da explosão em segundos
std::tuple<float, float, float> pontoExplosao; // lista para guardar o ponto de colisao do planeta
std::vector<std::tuple<float,float,float>> estrelas; // tripla que guarda a posicao das estrelas
const GLint N_estrelas = 1000; //constante para representar o numero de estrelas
GLfloat cameraAngleX = 0.0f, cameraAngleY = 0.0f;  // Ângulos de rotação da câmera
GLfloat cameraDistance = 5.0f;                    // Distância da câmera ao centro
float cameraX = 0.0f, cameraY = 0.0f; // Posição da câmera no plano
bool isPanning = false;              // Controle do botão direito do mouse
int lastMouseX, lastMouseY;       // Última posição do mouse


//funcao para modificar a camera
void updateCamera() {

    glLoadIdentity();

    float radX = cameraAngleX * M_PI / 180.0f;
    float radY = cameraAngleY * M_PI / 180.0f;

    GLfloat cameraX = cameraDistance * cos(radX) * sin(radY);
    GLfloat cameraY = cameraDistance * sin(radX);
    GLfloat cameraZ = cameraDistance * cos(radX) * cos(radY);

    gluLookAt(cameraX, cameraY, cameraZ,  
              0.0f, 0.0f, 0.0f,          
              0.0f, 1.0f, 0.0f);         
              
}
//funcao para capturar o movimento
void motion(int x, int y) {

    if (isPanning) {
        float deltaX = (x - lastMouseX) * 0.01f;
        float deltaY = (y - lastMouseY) * 0.01f;
        cameraX += deltaX;
        cameraY -= deltaY;
        lastMouseX = x;
        lastMouseY = y;
        glutPostRedisplay();
    }
}

//funcao para receber entradas do teclado
void teclado(unsigned char key, int x, int y) {

    if (key == 'v' || key == 'V') {
        vistaTopo = !vistaTopo;
    }
    if (key == 'r' || key == 'R'){
        planeta2colidiu = GL_FALSE;
        x_planeta2 = 5.0f, y_planeta2 = 0.0f, z_planeta2 = 0.0f;
        vx_planeta2 = 0.0f, vy_planeta2 = 2.0f, vz_planeta2 = 0.0f;
        massa1 = 20.0f;
        planeta2colidiu = GL_FALSE;
        particlesGenerated = GL_FALSE;
        explosionTimer = 8.0f;
        planet2Trail.clear();
        dt = 0.01f;
        cameraX = 0.0f, cameraY = 0.0f;
        cameraAngleY = 0.0f;
        isPanning = false;
        cameraDistance = 5.0f;
        glutPostRedisplay();
    }
    if (key == '1'){
        massa1 += 1.0f;
    }
    if (key == '2'){
        massa1 = std::max(0.1f, massa1 - 1.0f);
    }
    if (key == 'i'){
        dt += 0.01;
    }
    if (key == 'o'){
        dt = std::max(0.01f, dt - 0.01f);
    }
    if (key == 27) { 
        exit(0);  
    }
}
// funcao para receber entradas do mouse
void mouse(int button, int state, int x, int y) {

    if (button == GLUT_RIGHT_BUTTON) {
        if (state == GLUT_DOWN) {
            isPanning = true;
            lastMouseX = x;
            lastMouseY = y;
        } else if (state == GLUT_UP) {
            isPanning = false;
        }
    }

    // Scroll do mouse para aproximar/afastar
    if (button == 3) { // Scroll para cima
        cameraDistance = std::max(0.1f, cameraDistance - 0.5f);
        glutPostRedisplay();
    } else if (button == 4) { // Scroll para baixo
        cameraDistance = std::min(20.0f, cameraDistance + 0.5f);
        glutPostRedisplay();
    }

}

// funcao para mostrar o texto na tela
void desenhaTexto(float x, float y, const char* texto) {

    glDisable(GL_LIGHTING);
    glColor3f(1, 1, 1);
    glRasterPos2f(x, y);  // Posição do texto na tela
    for (int i = 0; texto[i] != '\0'; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, texto[i]);  // Fonte e caractere
    }
    glEnable(GL_LIGHTING);

}

// Função para calcular a força gravitacional
float forcaG(GLfloat massa1, GLfloat massa2, GLfloat distancia) {
    return (G * (massa1 * massa2)) / (distancia * distancia);  // Força gravitacional
}

// Função para desenhar um planeta
void desenhaPlaneta(GLfloat raio, GLfloat x_planeta, GLfloat y_planeta, GLfloat z_planeta, GLfloat r, GLfloat g, GLfloat b, bool iluminar) {
    
    glPushMatrix();
    
    if (iluminar) {
        // faz o planeta refletir luz
        GLfloat ambiente[] = {r * 0.2f, g * 0.2f, b * 0.2f, 1.0f}; 
        GLfloat difusa[] = {r, g, b, 1.0f}; 
        GLfloat especular[] = {0.8f, 0.8f, 0.8f, 1.0f};
        GLfloat brilho[] = {50.0f};

        glMaterialfv(GL_FRONT, GL_AMBIENT, ambiente);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, difusa);
        glMaterialfv(GL_FRONT, GL_SPECULAR, especular);
        glMaterialfv(GL_FRONT, GL_SHININESS, brilho);

    } else {

        glDisable(GL_LIGHTING);
        glColor3f(r, g, b);

    }

    glTranslatef(x_planeta, y_planeta, z_planeta);

    glutSolidSphere(raio, 60, 60);
    
    if (!iluminar) {
        glEnable(GL_LIGHTING);
    }

    glPopMatrix();

}


// Função para desenhar o rastro
void desenhaRastro() {
   
    glDisable(GL_LIGHTING);
    
    glColor4f(0.0f, 0.0f, 1.0f, 0.2f); 
    
    glBegin(GL_LINE_STRIP);
    for (const auto& pos : planet2Trail) {
        glVertex2f(pos.first, pos.second);
    }
    glEnd();
    

    glEnable(GL_LIGHTING);
}

// funcao que gera posicoes "aleatorias" das particulas da explosao
void gerarParticulasExplosao(const std::tuple<float, float, float>& pontoExplosao) {

    if (!particlesGenerated) {

        float centerX = std::get<0>(pontoExplosao);
        float centerY = std::get<1>(pontoExplosao);
        float centerZ = std::get<2>(pontoExplosao); 

        for (int i = 0; i < 10000; ++i) { // Gerar n particulas
            float theta = (rand() % 360) * (M_PI / 180.0f);
            float phi = (rand() % 180) * (M_PI / 180.0f) - M_PI / 2;

            float speed = (rand() % 50 + 10) / 100.0f;
            float particleX = centerX + cos(phi) * cos(theta) * speed;
            float particleY = centerY + cos(phi) * sin(theta) * speed;
            float particleZ = centerZ + sin(phi) * speed;

            explosionParticles.push_back(std::make_tuple(particleX, particleY, particleZ));
        }
        particlesGenerated = GL_TRUE;  // Impede a geração repetida de partículas
    }

}
// funcao que desenha as particulas basedada no vetor que foi inicializado com posicoes "aleatorias"
void desenhaParticulasExplosao() {

    if (explosionTimer <= 0.0f) return;

    glDisable(GL_LIGHTING);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_POINTS);
    for (const auto& particle : explosionParticles) {
        float distance = sqrt(std::pow(std::get<0>(particle), 2) + std::pow(std::get<1>(particle), 2));
        float intensity = 1.0f - (distance / 10.0f); 
        intensity = std::max(0.0f, intensity);

        float alpha = explosionTimer / 5.0f; 
        alpha = std::max(0.0f, std::min(1.0f, alpha));

        glColor4f(intensity, 0.5f * intensity, 0.0f, alpha);
        glVertex3f(std::get<0>(particle), std::get<1>(particle), std::get<2>(particle));
    }
    glEnd();

    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);

}


// funcao para mudar a posicao atual das particulas da explosao, efeito de "movimento"
void atualizaParticulas() {
    if (explosionTimer > 0.0f) {
        explosionTimer -= 0.1f;
    for (auto& particle : explosionParticles) {
        std::get<0>(particle) += (rand() % 20 - 10) / 1000.0f;  
        std::get<1>(particle) += (rand() % 20 - 10) / 1000.0f; 
        std::get<2>(particle) += (rand() % 20 - 10) / 1000.0f;
    }

}
}
// Função para verificar colisão
bool checarColisao() {

    GLfloat distancia = sqrt((x_planeta2 - x_planeta1)*(x_planeta2 - x_planeta1) +
                             (y_planeta2 - y_planeta1)*(y_planeta2 - y_planeta1) +
                             (z_planeta2 - z_planeta1)*(z_planeta2 - z_planeta1));

    if (distancia <= (raio1 + raio2) && particlesGenerated == GL_FALSE) {
        planeta2colidiu = GL_TRUE;
        pontoExplosao = std::make_tuple(x_planeta2, y_planeta2, z_planeta2);
        explosionParticles.clear();
        gerarParticulasExplosao(pontoExplosao);

        return GL_TRUE;
    }
    return GL_FALSE;

}  
// funcao para desenhar as estrelas
void desenhaEstrelas() {

    glDisable(GL_LIGHTING);  

    glPointSize(2.0f); 
    glBegin(GL_POINTS);
    for (const auto& estrela : estrelas) {
        glColor3f(1.0f, 1.0f, 1.0f);  
        glVertex3f(std::get<0>(estrela), std::get<1>(estrela), std::get<2>(estrela)); 
    }
    glEnd();

    glEnable(GL_LIGHTING); 

}

// funcao para carregar posicoes para estrelas
void gerarEstrelas() {

    for (int i = 0; i < N_estrelas; ++i) {
        float x = (rand() % 40000 - 20000) / 1000.0f; 
        float y = (rand() % 40000 - 20000) / 1000.0f;  
        float z = (rand() % 40000 - 20000) / 1000.0f;  
        estrelas.push_back(std::make_tuple(x, y, z));  
    }

}

// Função padrao para configurar a iluminacao
void setupLighting() {

    GLfloat light_position[] = {x_planeta1, y_planeta1, z_planeta1, 1.0f};  // Posição da luz
    GLfloat light_ambient[] = {0.1f, 0.1f, 0.1f, 1.0f};  // Luz ambiente
    GLfloat light_diffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};  // Luz difusa
    GLfloat light_specular[] = {1.0f, 1.0f, 1.0f, 1.0f};  // Luz especular

    glLightfv(GL_LIGHT0, GL_POSITION, light_position);  // Posição da luz
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);    // Luz ambiente
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);    // Luz difusa
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);  // Luz especular

    glEnable(GL_LIGHTING);  
    glEnable(GL_LIGHT0);   

}
// "circulo" em volta do planeta do meio, para dar sensacao de brilho
void desenhaHalo(GLfloat raio, GLfloat x_planeta, GLfloat y_planeta, GLfloat z_planeta) {

    glPushMatrix();
    glTranslatef(x_planeta, y_planeta, z_planeta);  // Posiciona o halo ao redor do planeta

    glDisable(GL_LIGHTING);

    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE); 
    glColor4f(1.0f, 0.8f, 0.0f, 0.5f);
    glutSolidSphere(raio, 60, 60);

    glDisable(GL_BLEND);

    glEnable(GL_LIGHTING);
    glPopMatrix();

}

// função para atualizar a posição do planeta
void atualizarPosicao() {

    // calculor de distancia, forca gravitacional, aceleracao e posicao 
    GLfloat distancia = sqrt((x_planeta2 - x_planeta1)*(x_planeta2 - x_planeta1) +
                             (y_planeta2 - y_planeta1)*(y_planeta2 - y_planeta1) +
                             (z_planeta2 - z_planeta1)*(z_planeta2 - z_planeta1));
    GLfloat F = forcaG(massa1, massa2, distancia);
    GLfloat ax = F * (x_planeta1 - x_planeta2) / (massa2 * distancia);
    GLfloat ay = F * (y_planeta1 - y_planeta2) / (massa2 * distancia);

    // atualizando a velocidade do planeta menor
    vx_planeta2 += ax * dt;
    vy_planeta2 += ay * dt;

    // atualizando a posicao do planeta menor
    x_planeta2 += vx_planeta2 * dt;
    y_planeta2 += vy_planeta2 * dt;

    // adicionando a nova posição ao trail
    planet2Trail.push_back(std::make_pair(x_planeta2, y_planeta2));

    // limitar o tamanho do trail
    if (planet2Trail.size() > MAX_TRAIL_LENGTH) {
        planet2Trail.erase(planet2Trail.begin()); 
    } 
}

// Função de exibição
void display() {

    //variaveis utilizadas nas funcoes do display
    GLfloat distancia = sqrt((x_planeta2 - x_planeta1)*(x_planeta2 - x_planeta1) +
                             (y_planeta2 - y_planeta1)*(y_planeta2 - y_planeta1) +
                             (z_planeta2 - z_planeta1)*(z_planeta2 - z_planeta1));
    GLfloat F = F = forcaG(massa1, massa2, distancia);
     char textoMassa[100];
    snprintf(textoMassa, sizeof(textoMassa), "Massa do Planeta Maior: %.2f", massa1);  
    char textoMassa2[100];
    snprintf(textoMassa2, sizeof(textoMassa2), "Massa do Planeta Menor: %.2f", massa2);  
    char textoDistancia[100];
    snprintf(textoDistancia, sizeof(textoDistancia), "Distancia entre os corpos: %.2f", distancia);  
    char textoForca[100];
    snprintf(textoForca,sizeof(textoForca),"Forca gravitacional atual: %.2f", F);

    

    // verificar colisao dos planetas
    checarColisao();

    //funcao principal de atualizacao dos planetas
    atualizarPosicao();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();

    updateCamera();

    // configuracao da camera
    if (vistaTopo) {
        gluLookAt(0.0f, 4.0f, 0.0f, 
                  0.0f, 0.0f, 0.0f,  
                  0.0f, 0.0f, -1.0f);  
    } else {
        gluLookAt(cameraX, cameraY, cameraDistance, // Posição da câmera
              cameraX, cameraY, 0.0f,          // Centro da cena
              0.0f, 1.0f, 0.0f);
    }

    // desenha as estrelas
    desenhaEstrelas();

    // configura a luz
    setupLighting();

    // desenha o planeta maior
    desenhaPlaneta(raio1, x_planeta1, y_planeta1, z_planeta1,0.95f, 0.95f, 0.60f,false);
    desenhaHalo(raio1*1.1,x_planeta1,y_planeta1,z_planeta1);

    //desenha o planeta menor com o rastro
    if(!planeta2colidiu){
    desenhaPlaneta(raio2, x_planeta2, y_planeta2, z_planeta2,0.20f,0.95f,0.60f,true);
    desenhaRastro();

    // se o planeta 2 colidir gerar a "explosao"
    }else{
    gerarParticulasExplosao(pontoExplosao);
    desenhaParticulasExplosao();
    atualizaParticulas();
    }

    //exibir informacoes na tela
    desenhaTexto(-6.0f, 3.0f, textoMassa);     
    desenhaTexto(-6.0f, 2.0f, textoMassa2); 
    desenhaTexto(-6.0f, 1.0f, textoDistancia);
    desenhaTexto(-6.0f,4.0f, textoForca);

    glutSwapBuffers();
    glutPostRedisplay();
}

// Função padrao de inicializacao
void init() {
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); 
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, 800.0f / 600.0f, 0.1f, 80.0f); 
    glMatrixMode(GL_MODELVIEW);
    gerarEstrelas();
}

// Funcao main padrao
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1280,720);
    glutCreateWindow("Orbita");
    init();
    glutDisplayFunc(display);  
    glutKeyboardFunc(teclado);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutMainLoop();  
    return 0;
}