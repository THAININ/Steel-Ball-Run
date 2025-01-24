#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <GL/freeglut.h>  

bool planet2Removed = false;  // Variável para verificar se o planeta 2 foi removido
bool planet2Exploding = false; // Flag para verificar se o planeta 2 está explodindo
float explosionTimer = 0.0f;   // Timer para controlar a duração da explosão
std::vector<std::pair<float, float>> explosionParticles; // Lista de partículas para o efeito de explosão
bool particlesGenerated = false; 



/* Configurações iniciais dos planetas */
float m1 = 1.0f; // Planeta 1 (estático)
float m2 = 0.5f; // Planeta 2 (móvel)
const float G = 0.8f; // Constante gravitacional (ajustada)

float planet1X = 0.0f;
float planet1Y = 0.0f;
float planet2X = 1.5f; // Planeta azul mais próximo
float planet2Y = 0.0f;

float planet2Vx = 0.0f; // Velocidade inicial em X
float planet2Vy = 0.7f; // Velocidade inicial em Y

const float dt = 0.01f; // Passo de tempo

// Armazenar as posições passadas do planeta 2 (rastro)
std::vector<std::pair<float, float>> planet2Trail;  // Lista de pares (x, y)
// armazenar as posicoes das estrelas 
std::vector<std::pair<float, float>> stars;

// Limitar o número de pontos no rastro
const int trailMaxSize = 500;

// Parâmetros da rotação (ângulo de inclinação)
float rotationAnglex = 0.0f;
float rotationAngley = 0.0f;
float rotationAnglez = 0.15f;  // Ângulo de inclinação em graus (ajustável)

// Raios dos planetas
float planet1Radius = 0.2f;  // Raio do planeta 1
float planet2Radius = 0.1f;  // Raio do planeta 2

void init() {
    glClearColor(0.0, 0.009, 0.09, 1.0);
}

void processInput(GLFWwindow* window) {
    /* Fecha o programa com a tecla ESC */
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
        m1 += 0.1f;
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
        m1 = std::max(0.1f, m1 - 0.1f);
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        m2 += 0.1f;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        m2 = std::max(0.1f, m2 - 0.1f);
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        rotationAnglex += 1.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        rotationAnglex -=1.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        rotationAngley += 1.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        rotationAngley -=1.0f;
    }
    /* reinicializa o simulador */
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        m1 = 1.0f;
        m2 = 0.1f;
        planet1X = 0.0f;
        planet1Y = 0.0f;
        planet2X = 1.5f;
        planet2Y = 0.0f;
        planet2Vx = 0.0f;
        planet2Vy = 0.5f;
        planet2Removed = false;
        planet2Exploding = false;
        particlesGenerated = false;
        explosionParticles.clear();
    }
}

// Função para calcular a intensidade da luz com base na distância
float calculateLightIntensity(float targetX, float targetY, float lightX, float lightY) {
    // Distância entre a fonte de luz e o ponto
    float dx = targetX - lightX;
    float dy = targetY - lightY;
    float distance = sqrt(dx * dx + dy * dy);

    // Atenuação da luz: quanto mais distante, mais fraca a luz
    // A intensidade diminui com a distância ao quadrado
    float intensity = 1.0f / (distance * distance + 1.0f);  // +1.0f para evitar divisão por zero

    return intensity;
}
// Função para calcular a direção da luz (do planeta 1 para o planeta 2)
void calculateLightDirection(float& lx, float& ly, float planet1X, float planet1Y, float planet2X, float planet2Y) {
    lx = planet1X - planet2X;
    ly = planet1Y - planet2Y;
    float length = sqrt(lx * lx + ly * ly);
    lx /= length;  // Normalizando o vetor de direção
    ly /= length;
}

// Função para calcular o fator de iluminação baseado no ângulo de incidência
float calculateLightingFactor(float normalX, float normalY, float lx, float ly) {
    // Produto escalar entre o vetor normal da superfície e a direção da luz
    float dotProduct = normalX * lx + normalY * ly;
    return std::max(dotProduct, 0.0f);  // Garantir que o valor não seja negativo (parte sombreada)
}

// Função para desenhar o planeta 2 com sombreamento parcial, incluindo a atenuação pela distância
void drawShadedPlanet(float planet2X, float planet2Y, float planet2Radius, float planet1X, float planet1Y) {
    // Calcular a direção da luz
    float lx, ly;
    calculateLightDirection(lx, ly, planet1X, planet1Y, planet2X, planet2Y);

    // Desenhando o planeta
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(planet2X, planet2Y);  // Centro do planeta
    for (int i = 0; i <= 32; ++i) {
        // Ângulo para cada ponto na circunferência do círculo
        float theta = 2.0f * M_PI * float(i) / float(32);
        float x = planet2X + planet2Radius * cosf(theta);
        float y = planet2Y + planet2Radius * sinf(theta);

        // Vetor normal (direção do ponto na circunferência do círculo)
        float normalX = x - planet2X;
        float normalY = y - planet2Y;
        float length = sqrt(normalX * normalX + normalY * normalY);
        normalX /= length;
        normalY /= length;

        // Calcular o fator de iluminação baseado no ângulo entre a normal e a direção da luz
        float lightingFactor = calculateLightingFactor(normalX, normalY, lx, ly);

        // Calcular a intensidade de luz com base na distância
        float distanceFactor = calculateLightIntensity(x, y, planet1X, planet1Y);

        // Ajustar a cor do planeta com o fator de iluminação e a atenuação de distância
        glColor3f(0.2f * lightingFactor * distanceFactor, 0.95f * lightingFactor * distanceFactor, 0.6f * lightingFactor * distanceFactor);
        glVertex2f(x, y);
    }
    glEnd();
}


// Função para gerar partículas da explosão
void generateExplosionParticles(float centerX, float centerY) {
    if(!particlesGenerated){
    for (int i = 0; i < 10000; ++i) { // Gerar 50 partículas
        float angle = (rand() % 360) * (M_PI / 180.0f); // ângulo aleatório para cada partícula
        float speed = (rand() % 50 + 10) / 100.0f; // Velocidade aleatória para a partícula
        float particleX = centerX + cos(angle) * speed;
        float particleY = centerY + sin(angle) * speed;
        explosionParticles.push_back({particleX, particleY});
    }
    }
}


// Função para desenhar as partículas da explosão
void drawExplosionParticles() {
    glColor3f(1.0f, 0.5f, 0.0f); // Cor laranja para as partículas

    glBegin(GL_POINTS);
    for (const auto& particle : explosionParticles) {
        glVertex2f(particle.first, particle.second);
    }
    glEnd();
}

// Função para atualizar as partículas da explosão
void updateExplosionParticles() {
    for (auto& particle : explosionParticles) {
        particle.first += (rand() % 20 - 10) / 1000.0f;  // Movimentação aleatória em X
        particle.second += (rand() % 20 - 10) / 1000.0f; // Movimentação aleatória em Y
    }
}


// Função para ajustar a projeção com base no tamanho da janela
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);  // Ajusta o viewport para o novo tamanho da janela
    
    // Ajustar a projeção ortográfica para a nova proporção da janela
    float aspectRatio = float(width) / float(height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-2.0f * aspectRatio, 2.0f * aspectRatio, -2.0f, 2.0f, -1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);
}


void initializeStars(int numStars) {
    stars.clear();  // Limpa qualquer estrela anterior

    for (int i = 0; i < numStars; ++i) {
        float x = (rand() % 10000-5000) / 1000.0f;  
        float y = (rand() % 10000-5000) / 1000.0f;  
        stars.push_back({x, y});  // Adiciona a posição ao vetor
    }
}


 //auto explicativo, desenha estrelas LOL
void drawStars() {
    glColor3f(1.0f, 1.0f, 1.0f); // Cor branca para as estrelas

    glBegin(GL_POINTS);
    for (const auto& star : stars) {
        glVertex2f(star.first, star.second);  // Desenha cada estrela
    }
    glEnd();
}



// Função para renderizar texto na tela usando GLUT
void renderText(float x, float y, const std::string& text) {
    glRasterPos2f(x, y);  // Define a posição do texto na tela
    for (char c : text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);  // Renderiza cada caractere
    }
}

// Função para calcular a força restauradora (como uma mola gravitacional)
float calculateRestoringForce(float m1, float m2, float distance) {
    if (distance < 0.1f) {
        distance = 0.1f; // Evitar divisão por zero
    }
    return -((m1 * m2) / (distance * distance)) * G;
}

// Função para desenhar o círculo (planeta)
void drawCircle(float x, float y, float radius, int segments = 32) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y);  // Centro do círculo
    for (int i = 0; i <= segments; ++i) {
        float theta = 2.0f * M_PI * float(i) / float(segments);
        float dx = radius * cosf(theta);
        float dy = radius * sinf(theta);
        glVertex2f(x + dx, y + dy);
    }
    glEnd();
}

// Função para desenhar o rastro
void drawTrail() {
    glColor4f(0.0f, 0.0f, 1.0f, 0.2f); // Azul claro e transparente

    glBegin(GL_POINTS);
    for (const auto& point : planet2Trail) {
        glVertex2f(point.first, point.second);  // Desenhar cada ponto no rastro
    }
    glEnd();
}

bool checkCollision() {
    float dx = planet2X - planet1X;
    float dy = planet2Y - planet1Y;
    float distance = sqrt(dx * dx + dy * dy);

    if (distance < (planet1Radius + planet2Radius)) {
        if (!planet2Exploding) {  // Se ainda não estiver explodindo
            planet2Exploding = true;  // Ativar explosão
            explosionTimer = 1.5f;    // Definir o tempo de duração da explosão
            generateExplosionParticles(planet2X, planet2Y);  // Gerar partículas
            particlesGenerated = true;
        }
        planet2Removed = true;  // Marcar o planeta 2 como removido após a colisão
        return true;
    }
    return false;
}

void updateExplosion() {
    if (planet2Exploding) {
        explosionTimer -= dt; // Reduzir o timer a cada quadro
        if (explosionTimer <= 0.0f) {
            planet2Exploding = false; // Desligar o efeito de explosão quando o timer acabar
            explosionParticles.clear(); // Limpar as partículas
        }
        updateExplosionParticles(); // Atualizar a posição das partículas
    }
}

void updateOscillation() {
    // Calcular a distância entre os planetas
    float dx = planet2X - planet1X;
    float dy = planet2Y - planet1Y;
    float distance = sqrt(dx * dx + dy * dy);

    // Calcular a força restauradora
    float force = calculateRestoringForce(m1, m2, distance);

    // Calcular a aceleração do planeta azul
    float accelerationX = (force / m2) * (dx / distance);
    float accelerationY = (force / m2) * (dy / distance);

    // Atualizar a velocidade do planeta azul
    planet2Vx += accelerationX * dt;
    planet2Vy += accelerationY * dt;

    // Atualizar a posição do planeta azul
    planet2X += planet2Vx * dt;
    planet2Y += planet2Vy * dt;

    // Adicionar a nova posição ao rastro
    planet2Trail.push_back({planet2X, planet2Y});

    // Limitar o número de pontos no rastro
    if (planet2Trail.size() > trailMaxSize) {
        planet2Trail.erase(planet2Trail.begin());  // Remover o ponto mais antigo
    }

    // Verificar colisão entre os planetas
    if (checkCollision()) {
        
    }

}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    drawStars();
    updateOscillation();
    updateExplosion();
   

    if(!planet2Removed){

    // Calcular a intensidade da luz com base na posição do planeta-luz (planeta 1)
    float lightIntensity2 = calculateLightIntensity(planet1X, planet1Y, planet2X, planet2Y); // Luz do planeta 1 iluminando o planeta 2

    // Aplicar rotação (inclinação) em todo o cenário
    glPushMatrix();
    glRotatef(rotationAnglex, 1.0f, 0.0f, 0.0f); // Rotaciona em torno do eixo x
    glRotatef(rotationAngley, 0.0f, 1.0f, 0.0f);   // Rotaciona em torno do eixo y
    glRotatef(rotationAnglez, 0.0f, 0.0f, 1.0f);     // Rotaciona em torno do eixo Z

    // Desenhar o rastro
    drawTrail();
    
    // Desenhar o planeta 2
    glColor3f(0.20f * lightIntensity2, 0.95f * lightIntensity2, 0.60f * lightIntensity2); // Efeito de luz no planeta 2
    drawShadedPlanet(planet2X, planet2Y, planet2Radius, planet1X, planet1Y);
    }
    // Desenhar o planeta 1
    // Alterar a cor do planeta 1 com base na intensidade da luz (sempre "brilhante" pois é a fonte de luz)
    glColor3f(0.95f, 0.95f, 0.60f); // O planeta 1 é verde
    drawCircle(planet1X, planet1Y, planet1Radius);


    glPopMatrix();  // Restaura a transformação (deixa o restante do código sem rotação)

    // Exibir os valores de m1, m2, e da distancia na tela
    glColor3f(1.0f, 1.0f, 1.0f);
    float dx = planet2X - planet1X;
    float dy = planet2Y - planet1Y;
    float distance = sqrt(dx * dx + dy * dy);
    std::string m1Text = "m1: " + std::to_string(m1);
    std::string m2Text = "m2: " + std::to_string(m2);
    std::string dText = "Distancia: " + std::to_string(distance);
    renderText(-1.8f, 1.8f, m1Text);  // Exibe m1 no canto superior esquerdo
    renderText(-1.8f, 1.6f, m2Text);  // Exibe m2 logo abaixo
    if(!planet2Removed){
    renderText(-1.8f,1.4f,dText);
    }

    
    if (planet2Exploding) {
        drawExplosionParticles();
    }
    
    
}

int main(int argc, char** argv) {
    /* Inicializar a biblioteca */
    if (!glfwInit()) {
        return -1;
    }
    glutInit(&argc, argv);

    /* Criar uma nova janela */
    GLFWwindow* window = glfwCreateWindow(1280, 720, "🎀Orbita🎀", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    /* Define o contexto da janela como atual */
    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    initializeStars(700);
    init();

    /* Loop até o usuário fechar a janela */
    while (!glfwWindowShouldClose(window)) {
        processInput(window);
        display();

        /* Troca os buffers frontais e traseiros */
        glfwSwapBuffers(window);

        /* Processa eventos */
        glfwPollEvents();
    }

    /* Fecha o programa */
    glfwTerminate();
    return 0;
}
