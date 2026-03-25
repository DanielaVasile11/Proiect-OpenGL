#include <freeglut.h>
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// ID-urile texturilor
GLuint iarbaRozTexture;
GLuint cerRozTexture;
GLuint munteTexture;
GLuint drumTexture;
GLuint frunzeTexture;
GLuint tufisTexture;
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

// 1. FUNCȚIA PENTRU COPAC
void drawTree(float x, float z) {
    float y = sin(x / 5.0f) * cos(z / 5.0f) * 2.0f - 2.0f; //  înălțimea pământului
    glPushMatrix();
    glTranslatef(x, y, z); // Mutăm pensula la coordonatele dorite

    glDisable(GL_TEXTURE_2D);
    glColor3f(0.4f, 0.2f, 0.1f); // Maro pentru trunchi
    glBegin(GL_QUADS);
    glVertex3f(-0.5f, 0.0f, 0.0f); glVertex3f(0.5f, 0.0f, 0.0f);
    glVertex3f(0.5f, 3.0f, 0.0f); glVertex3f(-0.5f, 3.0f, 0.0f);

    glVertex3f(0.0f, 0.0f, -0.5f); glVertex3f(0.0f, 0.0f, 0.5f);
    glVertex3f(0.0f, 3.0f, 0.5f); glVertex3f(0.0f, 3.0f, -0.5f);
    glEnd();
    glEnable(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, frunzeTexture);
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_TRIANGLES); // Coroana (piramidă)
    glTexCoord2f(0.5f, 1.0f); glVertex3f(0.0f, 6.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-2.5f, 2.0f, 2.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(2.5f, 2.0f, 2.5f);

    glTexCoord2f(0.5f, 1.0f); glVertex3f(0.0f, 6.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(2.5f, 2.0f, 2.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(2.5f, 2.0f, -2.5f);

    glTexCoord2f(0.5f, 1.0f); glVertex3f(0.0f, 6.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(2.5f, 2.0f, -2.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-2.5f, 2.0f, -2.5f);

    glTexCoord2f(0.5f, 1.0f); glVertex3f(0.0f, 6.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-2.5f, 2.0f, -2.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-2.5f, 2.0f, 2.5f);
    glEnd();
    glPopMatrix();
}

// 2. FUNCȚIA PENTRU BĂNCUȚĂ
// Are și un unghi ca să o putem roti să fie cu fața catre drum
void drawBench(float x, float z, float angle) {
    float y = sin(x / 5.0f) * cos(z / 5.0f) * 2.0f - 2.0f;
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(angle, 0.0f, 1.0f, 0.0f); // Rotim băncuța

    glDisable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    // Șezutul (Lemn deschis)
    glColor3f(0.9f, 0.8f, 0.7f);
    glVertex3f(-1.5f, 0.5f, -0.5f); glVertex3f(1.5f, 0.5f, -0.5f);
    glVertex3f(1.5f, 0.5f, 0.5f); glVertex3f(-1.5f, 0.5f, 0.5f);
    // Spătarul
    glVertex3f(-1.5f, 0.5f, -0.5f); glVertex3f(1.5f, 0.5f, -0.5f);
    glVertex3f(1.5f, 1.2f, -0.6f); glVertex3f(-1.5f, 1.2f, -0.6f);
    // Picioarele (Puțin mai închise la culoare)
    glColor3f(0.7f, 0.6f, 0.5f);
    glVertex3f(-1.3f, 0.0f, -0.3f); glVertex3f(-1.1f, 0.0f, -0.3f);
    glVertex3f(-1.1f, 0.5f, -0.3f); glVertex3f(-1.3f, 0.5f, -0.3f);

    glVertex3f(1.1f, 0.0f, -0.3f); glVertex3f(1.3f, 0.0f, -0.3f);
    glVertex3f(1.3f, 0.5f, -0.3f); glVertex3f(1.1f, 0.5f, -0.3f);

    glVertex3f(-1.3f, 0.0f, 0.3f); glVertex3f(-1.1f, 0.0f, 0.3f);
    glVertex3f(-1.1f, 0.5f, 0.3f); glVertex3f(-1.3f, 0.5f, 0.3f);

    glVertex3f(1.1f, 0.0f, 0.3f); glVertex3f(1.3f, 0.0f, 0.3f);
    glVertex3f(1.3f, 0.5f, 0.3f); glVertex3f(1.1f, 0.5f, 0.3f);
    glEnd();
    glEnable(GL_TEXTURE_2D);
    glPopMatrix();
}

// 3. FUNCȚIA PENTRU STÂLPUL DE ILUMINAT 
void drawLamp(float x, float z) {
    float y = sin(x / 5.0f) * cos(z / 5.0f) * 2.0f - 2.0f;
    glPushMatrix();
    glTranslatef(x, y, z);
    glDisable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    // Stâlpul negru
    glColor3f(0.1f, 0.1f, 0.1f);
    glVertex3f(-0.1f, 0.0f, 0.0f); glVertex3f(0.1f, 0.0f, 0.0f);
    glVertex3f(0.1f, 3.5f, 0.0f); glVertex3f(-0.1f, 3.5f, 0.0f);

    glVertex3f(0.0f, 0.0f, -0.1f); glVertex3f(0.0f, 0.0f, 0.1f);
    glVertex3f(0.0f, 3.5f, 0.1f); glVertex3f(0.0f, 3.5f, -0.1f);

    // Becul/Lampa 
    glColor3f(1.0f, 0.9f, 0.5f);
    glVertex3f(-0.4f, 3.5f, 0.0f); glVertex3f(0.4f, 3.5f, 0.0f);
    glVertex3f(0.4f, 4.2f, 0.0f); glVertex3f(-0.4f, 4.2f, 0.0f);

    glVertex3f(0.0f, 3.5f, -0.4f); glVertex3f(0.0f, 3.5f, 0.4f);
    glVertex3f(0.0f, 4.2f, 0.4f); glVertex3f(0.0f, 4.2f, -0.4f);
    glEnd();
    glEnable(GL_TEXTURE_2D);
    glPopMatrix();
}

// 4. FUNCȚIA PENTRU TUFIȘ DECORATIV PE ROTUND (Metoda Cross-Quad)
void drawBush(float x, float z) {
    float y = sin(x / 5.0f) * cos(z / 5.0f) * 2.0f - 2.0f; // Aflăm înălțimea pământului
    glPushMatrix();
    glTranslatef(x, y, z); 

    glBindTexture(GL_TEXTURE_2D, tufisTexture);
    glColor3f(1.0f, 1.0f, 1.0f); // Alb pur pentru a păstra culorile imaginii

    // Setăm dimensiunile tufișului (Lățime: 2.0, Înălțime: 2.0)
    float size = 1.0f;
    float height = 2.0f;

    glBegin(GL_QUADS);
    // --- Panoul 1 (Orientat pe axa X: formăm \ în litera X) ---
    // Coordonate Textură (u,v)   ->   Coordonate 3D (x,y,z)
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-size, 0.0f, -size); // Stânga-jos
    glTexCoord2f(1.0f, 0.0f); glVertex3f(size, 0.0f, size); // Dreapta-jos
    glTexCoord2f(1.0f, 1.0f); glVertex3f(size, height, size); // Dreapta-sus
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-size, height, -size); // Stânga-sus

    // --- Panoul 2 (Orientat pe axa Z, perpendicular pe primul: formăm / în litera X) ---
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-size, 0.0f, size); // Stânga-jos
    glTexCoord2f(1.0f, 0.0f); glVertex3f(size, 0.0f, -size); // Dreapta-jos
    glTexCoord2f(1.0f, 1.0f); glVertex3f(size, height, -size); // Dreapta-sus
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-size, height, size); // Stânga-sus
    glEnd();

    glPopMatrix();
}

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


      
     // 2. DESENĂM CIRCUITUL STRADAL
    
        glBindTexture(GL_TEXTURE_2D, drumTexture);
        glColor3f(1.0f, 1.0f, 1.0f);

        glBegin(GL_QUADS);
        for (int unghi = 0; unghi < 360; unghi += 10) {
            float rad1 = unghi * 3.14159f / 180.0f;
            float rad2 = (unghi + 10) * 3.14159f / 180.0f;

            float x_in1 = 14.0f * cos(rad1); float z_in1 = 14.0f * sin(rad1);
            float x_out1 = 18.0f * cos(rad1); float z_out1 = 18.0f * sin(rad1);

            // AM RIDICAT DRUMUL LA -1.5f (în loc de -1.9f) sa nu se mai vada iarba
            float y_in1 = sin(x_in1 / 5.0f) * cos(z_in1 / 5.0f) * 2.0f - 1.5f;
            float y_out1 = sin(x_out1 / 5.0f) * cos(z_out1 / 5.0f) * 2.0f - 1.5f;

            float x_in2 = 14.0f * cos(rad2); float z_in2 = 14.0f * sin(rad2);
            float x_out2 = 18.0f * cos(rad2); float z_out2 = 18.0f * sin(rad2);

            float y_in2 = sin(x_in2 / 5.0f) * cos(z_in2 / 5.0f) * 2.0f - 1.5f;
            float y_out2 = sin(x_out2 / 5.0f) * cos(z_out2 / 5.0f) * 2.0f - 1.5f;

            // Desenăm dalele de asfalt
            glTexCoord2f(0.0f, unghi / 20.0f);        glVertex3f(x_in1, y_in1, z_in1);
            glTexCoord2f(1.0f, unghi / 20.0f);        glVertex3f(x_out1, y_out1, z_out1);
            glTexCoord2f(1.0f, (unghi + 10) / 20.0f); glVertex3f(x_out2, y_out2, z_out2);
            glTexCoord2f(0.0f, (unghi + 10) / 20.0f); glVertex3f(x_in2, y_in2, z_in2);
        }
        glEnd();

        
    // 3. ADĂUGĂM CELE 10 OBIECTE STATICE 
   

    // 4 COPACI
        drawTree(25.0f, -25.0f);
        drawTree(-30.0f, 15.0f);
        drawTree(-20.0f, -30.0f);
        drawTree(35.0f, 10.0f);

        //  2 BĂNCUȚE 
        drawBench(0.0f, 19.5f, 180.0f); // În față, orientată spre centru (unghi 180)
        drawBench(-19.5f, 0.0f, 90.0f); // Pe partea stângă, orientată spre centru (unghi 90)

        //  2 STÂLPI DE ILUMINAT 
        drawLamp(2.5f, 19.5f);
        drawLamp(-19.5f, 2.5f);

        //  2 TUFIȘURI 
        drawBush(-3.0f, 19.5f);
        drawBush(-19.5f, -3.0f);

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

// Funcție  pentru a elimina fundalul alb dintr-un JPG
GLuint loadTextureTransparent(const char* path) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);

    
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 4);

    if (data) {
        //  Parcurgem fiecare pixel din poză
        for (int i = 0; i < width * height * 4; i += 4) {
            // Verificăm dacă culoarea este albă sau foarte aproape de alb (ex: > 240)
            if (data[i] > 240 && data[i + 1] > 240 && data[i + 2] > 240) {
                data[i + 3] = 0;   // Facem pixelul complet INVIZIBIL
            }
            else {
                data[i + 3] = 255; // Lăsăm pixelul vizibil (opac)
            }
        }

        // Trimitem poza modificată către placa video 
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    }
    else {
        printf("Eroare la incarcarea texturii transparente: %s\n", path);
    }

    stbi_image_free(data);
    return textureID;
}

void init() {
    // Activăm 3D-ul și texturile
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);



    iarbaRozTexture = loadTexture("amestec-flori.jpg");
    cerRozTexture = loadTexture("cer_roz.jpg");
    munteTexture = loadTexture("munte7.jpg");
    drumTexture = loadTexture("drum8.jpg");
    frunzeTexture = loadTexture("frunze1.jpg");
    tufisTexture = loadTextureTransparent("tufis2.jpg");

    // 2. ACTIVĂM TESTUL DE TRANSPARENȚĂ
 
        glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.1f);
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