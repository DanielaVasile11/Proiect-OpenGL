#include <freeglut.h>
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// ID-urile texturilor
GLuint iarbaRozTexture;
GLuint cerRozTexture;
GLuint munteTexture;
//încarc o imagine și o transform în textură
GLuint loadTexture(const char* path) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Setăm cum se va comporta textura când este mărită/micșorată
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

 
    int width, height, nrChannels;
    // Răsturnăm imaginea, deoarece OpenGL citește coordonatele de jos în sus
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);

    if (data) {
       
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    }
    else {
        printf("Eroare la incarcarea texturii: %s\n", path);
    }

    // Curățăm memoria după ce am trimis poza către placa video
    stbi_image_free(data);
    return textureID;
}
// Unghiul de rotație
float angle = 0.0f;
float angleUpDown = 0.0f;

void drawScene() {
      
        // 1. Iarba cu dealuri line
        
        glBindTexture(GL_TEXTURE_2D, iarbaRozTexture);
        glColor3f(1.0f, 1.0f, 1.0f);

        glBegin(GL_QUADS);
        // Împărțim podeaua (de la -50 la 50) în pătrățele de mărime 2.0
        for (float x = -50.0f; x < 50.0f; x += 2.0f) {
            for (float z = -50.0f; z < 50.0f; z += 2.0f) {

                // Efect de dealuri
                
                float y1 = sin(x / 5.0f) * cos(z / 5.0f) * 2.0f;
                float y2 = sin((x + 2.0f) / 5.0f) * cos(z / 5.0f) * 2.0f;
                float y3 = sin((x + 2.0f) / 5.0f) * cos((z + 2.0f) / 5.0f) * 2.0f;
                float y4 = sin(x / 5.0f) * cos((z + 2.0f) / 5.0f) * 2.0f;

                // Calculăm texturile ca să se repete frumos pe dealuri
                float u1 = (x + 50.0f) / 10.0f; float v1 = (z + 50.0f) / 10.0f;
                float u2 = (x + 52.0f) / 10.0f; float v2 = v1;
                float u3 = u2;                  float v3 = (z + 52.0f) / 10.0f;
                float u4 = u1;                  float v4 = v3;

                // Desenăm pătrățelul deformat
                glTexCoord2f(u1, v1); glVertex3f(x, y1 - 2.0f, z);
                glTexCoord2f(u2, v2); glVertex3f(x + 2.0f, y2 - 2.0f, z);
                glTexCoord2f(u3, v3); glVertex3f(x + 2.0f, y3 - 2.0f, z + 2.0f);
                glTexCoord2f(u4, v4); glVertex3f(x, y4 - 2.0f, z + 2.0f);
            }
        }
        glEnd();

       
      // 1.5. DESENĂM MUNTELE NATURAL TEXTURAT
      
        glBindTexture(GL_TEXTURE_2D, munteTexture); // Aplicăm textura de munte
        glColor3f(1.0f, 1.0f, 1.0f); 

        glBegin(GL_TRIANGLES);
        // --- VÂRFUL PRINCIPAL (Înălțime: 6.0f) ---
        // Fața din față
        glTexCoord2f(0.5f, 1.0f); glVertex3f(0.0f, 6.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-6.0f, -2.0f, 6.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(6.0f, -2.0f, 6.0f);

        // Fața din dreapta
        glTexCoord2f(0.5f, 1.0f); glVertex3f(0.0f, 6.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(6.0f, -2.0f, 6.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(6.0f, -2.0f, -6.0f);

        // Fața din spate
        glTexCoord2f(0.5f, 1.0f); glVertex3f(0.0f, 6.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(6.0f, -2.0f, -6.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-6.0f, -2.0f, -6.0f);

        // Fața din stânga
        glTexCoord2f(0.5f, 1.0f); glVertex3f(0.0f, 6.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-6.0f, -2.0f, -6.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-6.0f, -2.0f, 6.0f);

        // --- VÂRFUL SECUNDAR STÂNGA ---
        glTexCoord2f(0.5f, 1.0f); glVertex3f(-3.0f, 3.5f, 3.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-8.0f, -2.0f, 5.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, -2.0f, 7.0f);

        glTexCoord2f(0.5f, 1.0f); glVertex3f(-3.0f, 3.5f, 3.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, -2.0f, 7.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -2.0f, -1.0f);

        // --- VÂRFUL SECUNDAR DREAPTA ---
        glTexCoord2f(0.5f, 1.0f); glVertex3f(4.0f, 4.0f, 1.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, -2.0f, 6.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(8.0f, -2.0f, 3.0f);

        glTexCoord2f(0.5f, 1.0f); glVertex3f(4.0f, 4.0f, 1.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(8.0f, -2.0f, 3.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(5.0f, -2.0f, -4.0f);
        glEnd();


        // 2. DESENĂM CERUL (Skybox)
       
        glBindTexture(GL_TEXTURE_2D, cerRozTexture);
        glColor3f(1.0f, 1.0f, 1.0f);

        glBegin(GL_QUADS);
        // Peretele din SPATE 
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-50.0f, -10.0f, -50.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(50.0f, -10.0f, -50.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(50.0f, 20.0f, -50.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-50.0f, 20.0f, -50.0f);

        // Peretele din FAȚĂ 
        glTexCoord2f(0.0f, 0.0f); glVertex3f(50.0f, -10.0f, 50.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-50.0f, -10.0f, 50.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-50.0f, 20.0f, 50.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(50.0f, 20.0f, 50.0f);

        // Peretele din STÂNGA
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-50.0f, -10.0f, 50.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-50.0f, -10.0f, -50.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-50.0f, 20.0f, -50.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-50.0f, 20.0f, 50.0f);

        // Peretele din DREAPTA
        glTexCoord2f(0.0f, 0.0f); glVertex3f(50.0f, -10.0f, -50.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(50.0f, -10.0f, 50.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(50.0f, 20.0f, 50.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(50.0f, 20.0f, -50.0f);
        glEnd();
}

void init() {
    // Activăm 3D-ul și texturile
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    
   
    iarbaRozTexture = loadTexture("amestec-flori.jpg");
    cerRozTexture = loadTexture("cer_roz.jpg");
    munteTexture = loadTexture("munte.jpg");
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Poziționăm camera
    gluLookAt(0.0, 5.0, 20.0,  // Camera e la înălțimea 5 și distanța 20
        0.0, 0.0, 0.0,   // Se uită spre centrul scenei
        0.0, 1.0, 0.0);  // Axa Y este în sus


    // 1. Înclinarea Sus/Jos (rotație pe axa X: 1, 0, 0)
    glRotatef(angleUpDown, 1.0f, 0.0f, 0.0f);

    // 2. Întoarcerea Stânga/Dreapta (rotație pe axa Y: 0, 1, 0)
    glRotatef(angle, 0.0f, 1.0f, 0.0f);

    drawScene();
    glRotatef(angle, 0, 1, 0);

    glutSwapBuffers();
}

void reshape(int w, int h) {
    if (h == 0) h = 1;
    float aspect = (float)w / (float)h;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, aspect, 0.1f, 200.0f); // Unghi de vedere de 45 grade
    glMatrixMode(GL_MODELVIEW);
}

// Funcție  pentru a roti scena cu tastele săgeți
void specialKeys(int key, int x, int y) {
    // Navigare Stânga / Dreapta
    if (key == GLUT_KEY_RIGHT) angle += 5.0f;
    if (key == GLUT_KEY_LEFT) angle -= 5.0f;

    // Navigare Sus / Jos
    if (key == GLUT_KEY_UP) angleUpDown -= 5.0f;
    if (key == GLUT_KEY_DOWN) angleUpDown += 5.0f;

    // Blocăm rotația să nu ne dăm peste cap (limita la 80 de grade)
    if (angleUpDown > 80.0f) angleUpDown = 80.0f;
    if (angleUpDown < -80.0f) angleUpDown = -80.0f;

    glutPostRedisplay(); // Spunem ecranului să se redeseneze
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1000, 700);
    glutCreateWindow("Proiect P1 - Scena 3D");

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutSpecialFunc(specialKeys); // Să ne putem mișca

    glutMainLoop();
    return 0;
}