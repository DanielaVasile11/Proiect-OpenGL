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

GLuint loadTexture(const char* path) {
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
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    }
    else {
        printf("Eroare la incarcarea texturii: %s\n", path);
    }
    stbi_image_free(data);
    return textureID;
}

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
        for (int i = 0; i < width * height * 4; i += 4) {
            if (data[i] > 240 && data[i + 1] > 240 && data[i + 2] > 240) data[i + 3] = 0;
            else data[i + 3] = 255;
        }
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    }
    else {
        printf("Eroare la incarcarea texturii transparente: %s\n", path);
    }
    stbi_image_free(data);
    return textureID;
}

// MATRICEA PENTRU UMBRE PLANE
void shadowMatrix(GLfloat shadowMat[16], GLfloat groundplane[4], GLfloat lightpos[4]) {
    GLfloat dot = groundplane[0] * lightpos[0] + groundplane[1] * lightpos[1] +
        groundplane[2] * lightpos[2] + groundplane[3] * lightpos[3];

    shadowMat[0] = dot - lightpos[0] * groundplane[0];
    shadowMat[4] = 0.f - lightpos[0] * groundplane[1];
    shadowMat[8] = 0.f - lightpos[0] * groundplane[2];
    shadowMat[12] = 0.f - lightpos[0] * groundplane[3];

    shadowMat[1] = 0.f - lightpos[1] * groundplane[0];
    shadowMat[5] = dot - lightpos[1] * groundplane[1];
    shadowMat[9] = 0.f - lightpos[1] * groundplane[2];
    shadowMat[13] = 0.f - lightpos[1] * groundplane[3];

    shadowMat[2] = 0.f - lightpos[2] * groundplane[0];
    shadowMat[6] = 0.f - lightpos[2] * groundplane[1];
    shadowMat[10] = dot - lightpos[2] * groundplane[2];
    shadowMat[14] = 0.f - lightpos[2] * groundplane[3];

    shadowMat[3] = 0.f - lightpos[3] * groundplane[0];
    shadowMat[7] = 0.f - lightpos[3] * groundplane[1];
    shadowMat[11] = 0.f - lightpos[3] * groundplane[2];
    shadowMat[15] = dot - lightpos[3] * groundplane[3];
}

// VARIABILE GLOBALE 
float camX = 0.0f; float camY = 5.0f; float camZ = 20.0f;
float angle = 0.0f; float angleUpDown = 0.0f; float rotatieBancuta = 0.0f;

// Pozitiile Luminilor
GLfloat light_position[] = { 10.0f, 30.0f, 10.0f, 0.0f }; // Soarele (Directional)

// Pozitie pentru munte ca umbra sa fie destul de lunga sa iasa de sub el (Soare mai coborat pe cer)
GLfloat mountainSun_pos[] = { 30.0f, 10.0f, 30.0f, 0.0f };

// Pozitia  becului de pe stalp 
GLfloat lightPole1_pos[] = { -19.5f, 3.8f, 2.5f, 1.0f };
GLfloat lightPole2_pos[] = { 2.5f, 3.8f, 19.5f, 1.0f };

// Pozitii mai inalte  doar pentru copaci (ca sa nu faca o dunga infinita)
GLfloat treePole1_pos[] = { -19.5f, 12.0f, 2.5f, 1.0f };
GLfloat treePole2_pos[] = { 2.5f, 12.0f, 19.5f, 1.0f };

// ==========================================
// FUNCȚIILE DE DESENARE
// ==========================================
void drawTree(float x, float z, bool isShadow = false) {
    float y = -2.0f;
    glPushMatrix();
    glTranslatef(x, y, z);

    glDisable(GL_TEXTURE_2D);
    if (!isShadow) {
        glColor3f(0.4f, 0.2f, 0.1f);
        glNormal3f(0.0f, 1.0f, 0.0f);
    }
    glBegin(GL_QUADS);
    glVertex3f(-0.5f, 0.0f, 0.0f); glVertex3f(0.5f, 0.0f, 0.0f);
    glVertex3f(0.5f, 3.0f, 0.0f); glVertex3f(-0.5f, 3.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, -0.5f); glVertex3f(0.0f, 0.0f, 0.5f);
    glVertex3f(0.0f, 3.0f, 0.5f); glVertex3f(0.0f, 3.0f, -0.5f);
    glEnd();

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, frunzeTexture);
    if (!isShadow) {
        glColor3f(1.0f, 1.0f, 1.0f);
    }

    glBegin(GL_TRIANGLES);
    if (!isShadow) glNormal3f(0.0f, 0.5f, 1.0f);
    glTexCoord2f(0.5f, 1.0f); glVertex3f(0.0f, 6.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-2.5f, 2.0f, 2.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(2.5f, 2.0f, 2.5f);

    if (!isShadow) glNormal3f(1.0f, 0.5f, 0.0f);
    glTexCoord2f(0.5f, 1.0f); glVertex3f(0.0f, 6.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(2.5f, 2.0f, 2.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(2.5f, 2.0f, -2.5f);

    if (!isShadow) glNormal3f(0.0f, 0.5f, -1.0f);
    glTexCoord2f(0.5f, 1.0f); glVertex3f(0.0f, 6.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(2.5f, 2.0f, -2.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-2.5f, 2.0f, -2.5f);

    if (!isShadow) glNormal3f(-1.0f, 0.5f, 0.0f);
    glTexCoord2f(0.5f, 1.0f); glVertex3f(0.0f, 6.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-2.5f, 2.0f, -2.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-2.5f, 2.0f, 2.5f);
    glEnd();
    glPopMatrix();
}

void drawBench(float x, float z, float angle, bool isShadow = false) {
    float y = -2.0f;
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(angle, 0.0f, 1.0f, 0.0f);

    glDisable(GL_TEXTURE_2D);
    if (!isShadow) {
        glNormal3f(0.0f, 1.0f, 0.0f);
        glColor3f(0.9f, 0.8f, 0.7f);
    }
    glBegin(GL_QUADS);
    glVertex3f(-1.5f, 0.5f, -0.5f); glVertex3f(1.5f, 0.5f, -0.5f);
    glVertex3f(1.5f, 0.5f, 0.5f); glVertex3f(-1.5f, 0.5f, 0.5f);
    glEnd();

    if (!isShadow) { glNormal3f(0.0f, 0.5f, 1.0f); }
    glBegin(GL_QUADS);
    glVertex3f(-1.5f, 0.5f, -0.5f); glVertex3f(1.5f, 0.5f, -0.5f);
    glVertex3f(1.5f, 1.2f, -0.6f); glVertex3f(-1.5f, 1.2f, -0.6f);
    glEnd();

    if (!isShadow) { glNormal3f(0.0f, 0.0f, 1.0f); glColor3f(0.7f, 0.6f, 0.5f); }
    glBegin(GL_QUADS);
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

void drawBush(float x, float z, bool isShadow = false) {
    float y = -2.0f;
    glPushMatrix();
    glTranslatef(x, y, z);

    // ACTIVĂM TEXTURA ȘI DECUPAJUL ALPHA PENTRU UMBRĂ DETALIATĂ
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tufisTexture);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.1f);

    if (!isShadow) {
        glColor3f(1.0f, 1.0f, 1.0f);
        glNormal3f(0.0f, 1.0f, 0.0f);
    }

    float size = 1.0f;
    float height = 2.0f;

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-size, 0.0f, -size);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(size, 0.0f, size);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(size, height, size);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-size, height, -size);

    glTexCoord2f(0.0f, 0.0f); glVertex3f(-size, 0.0f, size);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(size, 0.0f, -size);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(size, height, -size);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-size, height, size);
    glEnd();

    glPopMatrix();
}

void drawMountain(bool isShadow = false) {
    if (!isShadow) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, munteTexture);
        glColor3f(1.0f, 1.0f, 1.0f);
    }
    else {
        glDisable(GL_TEXTURE_2D);
    }

    glBegin(GL_TRIANGLES);
    if (!isShadow) glNormal3f(0.0f, 0.5f, 1.0f);
    glTexCoord2f(0.5f, 1.0f); glVertex3f(0.0f, 6.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-6.0f, -2.0f, 6.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(6.0f, -2.0f, 6.0f);

    if (!isShadow) glNormal3f(1.0f, 0.5f, 0.0f);
    glTexCoord2f(0.5f, 1.0f); glVertex3f(0.0f, 6.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(6.0f, -2.0f, 6.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(6.0f, -2.0f, -6.0f);

    if (!isShadow) glNormal3f(0.0f, 0.5f, -1.0f);
    glTexCoord2f(0.5f, 1.0f); glVertex3f(0.0f, 6.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(6.0f, -2.0f, -6.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-6.0f, -2.0f, -6.0f);

    if (!isShadow) glNormal3f(-1.0f, 0.5f, 0.0f);
    glTexCoord2f(0.5f, 1.0f); glVertex3f(0.0f, 6.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-6.0f, -2.0f, -6.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-6.0f, -2.0f, 6.0f);

    if (!isShadow) glNormal3f(0.0f, 1.0f, 0.0f);
    glTexCoord2f(0.5f, 1.0f); glVertex3f(-3.0f, 3.5f, 3.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-8.0f, -2.0f, 5.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, -2.0f, 7.0f);

    glTexCoord2f(0.5f, 1.0f); glVertex3f(-3.0f, 3.5f, 3.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, -2.0f, 7.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -2.0f, -1.0f);

    glTexCoord2f(0.5f, 1.0f); glVertex3f(4.0f, 4.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, -2.0f, 6.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(8.0f, -2.0f, 3.0f);

    glTexCoord2f(0.5f, 1.0f); glVertex3f(4.0f, 4.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(8.0f, -2.0f, 3.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(5.0f, -2.0f, -4.0f);
    glEnd();

    glEnable(GL_TEXTURE_2D);
}

void drawLamp(float x, float z) {
    float y = -2.0f;
    glPushMatrix();
    glTranslatef(x, y, z);
    glDisable(GL_TEXTURE_2D);
    glNormal3f(0.0f, 1.0f, 0.0f);

    glBegin(GL_QUADS);
    glColor3f(0.1f, 0.1f, 0.1f);
    glVertex3f(-0.1f, 0.0f, 0.0f); glVertex3f(0.1f, 0.0f, 0.0f);
    glVertex3f(0.1f, 3.5f, 0.0f); glVertex3f(-0.1f, 3.5f, 0.0f);

    glVertex3f(0.0f, 0.0f, -0.1f); glVertex3f(0.0f, 0.0f, 0.1f);
    glVertex3f(0.0f, 3.5f, 0.1f); glVertex3f(0.0f, 3.5f, -0.1f);

    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 0.9f, 0.5f);
    glVertex3f(-0.4f, 3.5f, 0.0f); glVertex3f(0.4f, 3.5f, 0.0f);
    glVertex3f(0.4f, 4.2f, 0.0f); glVertex3f(-0.4f, 4.2f, 0.0f);
    glVertex3f(0.0f, 3.5f, -0.4f); glVertex3f(0.0f, 3.5f, 0.4f);
    glVertex3f(0.0f, 4.2f, 0.4f); glVertex3f(0.0f, 4.2f, -0.4f);
    glEnable(GL_LIGHTING);
    glEnd();

    glEnable(GL_TEXTURE_2D);
    glPopMatrix();
}

// FUNCȚIA CARE PROIECTEAZĂ UMBRA
void castShadow(int type, float x, float z, float angle, GLfloat lightPos[]) {
    GLfloat groundPlane[4] = { 0.0f, 1.0f, 0.0f, 1.90f };
    GLfloat shadowMat[16];
    shadowMatrix(shadowMat, groundPlane, lightPos);

    glPushMatrix();
    glMultMatrixf(shadowMat);

    if (type == 1) drawTree(x, z, true);
    else if (type == 2) drawBench(x, z, angle, true);
    else if (type == 3) drawBush(x, z, true);
    else if (type == 4) drawMountain(true);

    glPopMatrix();
}

// ==========================================
// FUNCȚIA SCENEI PRINCIPALE
// ==========================================
void drawScene() {
    glLightfv(GL_LIGHT1, GL_POSITION, lightPole1_pos);
    glLightfv(GL_LIGHT2, GL_POSITION, lightPole2_pos);

    // 1. Iarba 
    glBindTexture(GL_TEXTURE_2D, iarbaRozTexture);
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);
    for (float x = -50.0f; x < 50.0f; x += 2.0f) {
        for (float z = -50.0f; z < 50.0f; z += 2.0f) {
            float u1 = (x + 50.0f) / 10.0f; float v1 = (z + 50.0f) / 10.0f;
            float u2 = (x + 52.0f) / 10.0f; float v2 = v1;
            float u3 = u2;                  float v3 = (z + 52.0f) / 10.0f;
            float u4 = u1;                  float v4 = v3;

            glTexCoord2f(u1, v1); glVertex3f(x, -2.0f, z);
            glTexCoord2f(u2, v2); glVertex3f(x + 2.0f, -2.0f, z);
            glTexCoord2f(u3, v3); glVertex3f(x + 2.0f, -2.0f, z + 2.0f);
            glTexCoord2f(u4, v4); glVertex3f(x, -2.0f, z + 2.0f);
        }
    }
    glEnd();

    // 2. Drumul 
    glBindTexture(GL_TEXTURE_2D, drumTexture);
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);
    for (int unghi = 0; unghi < 360; unghi += 10) {
        float rad1 = unghi * 3.14159f / 180.0f;
        float rad2 = (unghi + 10) * 3.14159f / 180.0f;

        float x_in1 = 14.0f * cos(rad1); float z_in1 = 14.0f * sin(rad1);
        float x_out1 = 18.0f * cos(rad1); float z_out1 = 18.0f * sin(rad1);
        float x_in2 = 14.0f * cos(rad2); float z_in2 = 14.0f * sin(rad2);
        float x_out2 = 18.0f * cos(rad2); float z_out2 = 18.0f * sin(rad2);

        float y_road = -1.95f;

        glTexCoord2f(0.0f, unghi / 20.0f);        glVertex3f(x_in1, y_road, z_in1);
        glTexCoord2f(1.0f, unghi / 20.0f);        glVertex3f(x_out1, y_road, z_out1);
        glTexCoord2f(1.0f, (unghi + 10) / 20.0f); glVertex3f(x_out2, y_road, z_out2);
        glTexCoord2f(0.0f, (unghi + 10) / 20.0f); glVertex3f(x_in2, y_road, z_in2);
    }
    glEnd();

   
    // 3. UMBRELE 
    
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);

    // -- UMBRA PRINCIPALĂ DE LA SOARE (GRI DESCHIS) --
    glColor4f(0.1f, 0.1f, 0.1f, 0.4f);
    castShadow(1, 25.0f, -25.0f, 0.0f, light_position); // Copaci
    castShadow(1, -30.0f, 15.0f, 0.0f, light_position);
    castShadow(1, -20.0f, -30.0f, 0.0f, light_position);
    castShadow(1, 35.0f, 10.0f, 0.0f, light_position);
    castShadow(2, 0.0f, 19.5f, 180.0f + rotatieBancuta, light_position); // Banci
    castShadow(2, -19.5f, 0.0f, 90.0f, light_position);
    castShadow(3, -3.0f, 19.5f, 0.0f, light_position);  // Tufisuri
    castShadow(3, -19.5f, -3.0f, 0.0f, light_position);
    castShadow(4, 0.0f, 0.0f, 0.0f, mountainSun_pos);   // UMBRA LA MUNTE ALUNGITĂ

    // -- UMBRA A DOUA (ALUNGITĂ) DE LA STÂLPI (GRI ÎNCHIS) --
    glColor4f(0.1f, 0.1f, 0.1f, 0.6f);

    // Stâlpul 1: Umbrește obiectele scunde folosind înălțimea lui normală (lightPole1_pos)
    castShadow(2, -19.5f, 0.0f, 90.0f, lightPole1_pos); // Banca
    castShadow(3, -19.5f, -3.0f, 0.0f, lightPole1_pos); // Tufis 

    // Copacii iau umbra din lumina înaltă, ca să nu fie infiniți
    castShadow(1, -30.0f, 15.0f, 0.0f, treePole1_pos); // Copac
    castShadow(1, -20.0f, -30.0f, 0.0f, treePole1_pos); // Copac

    // Stâlpul 2: Umbrește obiectele scunde folosind înălțimea lui normală 
    castShadow(2, 0.0f, 19.5f, 180.0f + rotatieBancuta, lightPole2_pos); // Banca
    castShadow(3, -3.0f, 19.5f, 0.0f, lightPole2_pos); // Tufis 

    // Copacii iau umbra din lumina înaltă
    castShadow(1, 25.0f, -25.0f, 0.0f, treePole2_pos); 
    castShadow(1, 35.0f, 10.0f, 0.0f, treePole2_pos);  

    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    // 4. Muntele 
    drawMountain(false);

    // 5. Obiectele Principale 3D
    drawTree(25.0f, -25.0f);
    drawTree(-30.0f, 15.0f);
    drawTree(-20.0f, -30.0f);
    drawTree(35.0f, 10.0f);
    drawBench(0.0f, 19.5f, 180.0f + rotatieBancuta);
    drawBench(-19.5f, 0.0f, 90.0f);
    drawLamp(2.5f, 19.5f);
    drawLamp(-19.5f, 2.5f);
    drawBush(-3.0f, 19.5f);
    drawBush(-19.5f, -3.0f);

    // 6. Cerul
    glDisable(GL_LIGHTING);
    glBindTexture(GL_TEXTURE_2D, cerRozTexture);
    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-50.0f, -10.0f, -50.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(50.0f, -10.0f, -50.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(50.0f, 20.0f, -50.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-50.0f, 20.0f, -50.0f);

    glTexCoord2f(0.0f, 0.0f); glVertex3f(50.0f, -10.0f, 50.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-50.0f, -10.0f, 50.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-50.0f, 20.0f, 50.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(50.0f, 20.0f, 50.0f);

    glTexCoord2f(0.0f, 0.0f); glVertex3f(-50.0f, -10.0f, 50.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-50.0f, -10.0f, -50.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-50.0f, 20.0f, -50.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-50.0f, 20.0f, 50.0f);

    glTexCoord2f(0.0f, 0.0f); glVertex3f(50.0f, -10.0f, -50.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(50.0f, -10.0f, 50.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(50.0f, 20.0f, 50.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(50.0f, 20.0f, -50.0f);
    glEnd();

    glEnable(GL_LIGHTING);
}

void init() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    iarbaRozTexture = loadTexture("nisip.jpg");
    cerRozTexture = loadTexture("cer_roz.jpg");
    munteTexture = loadTexture("munte7.jpg");
    drumTexture = loadTexture("drum8.jpg");
    frunzeTexture = loadTexture("frunze1.jpg");
    tufisTexture = loadTextureTransparent("tufis2.jpg");

    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.1f);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    GLfloat sunAmbient[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    GLfloat sunDiffuse[] = { 0.6f, 0.6f, 0.6f, 1.0f };
    glLightfv(GL_LIGHT0, GL_AMBIENT, sunAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, sunDiffuse);

    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHT2);
    GLfloat poleDiffuse[] = { 0.8f, 0.7f, 0.2f, 1.0f };
    glLightfv(GL_LIGHT1, GL_DIFFUSE, poleDiffuse);
    glLightfv(GL_LIGHT2, GL_DIFFUSE, poleDiffuse);

    GLfloat no_mat[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, no_mat);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.0f);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    gluLookAt(camX, camY, camZ,
        camX, camY, camZ - 1.0,
        0.0, 1.0, 0.0);

    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    glRotatef(angleUpDown, 1.0f, 0.0f, 0.0f);
    glRotatef(angle, 0.0f, 1.0f, 0.0f);

    drawScene();

    glutSwapBuffers();
}

void reshape(int w, int h) {
    if (h == 0) h = 1;
    float aspect = (float)w / (float)h;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, aspect, 0.1f, 200.0f);
    glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int x, int y) {
    if (key == 'q' || key == 'Q') rotatieBancuta += 5.0f;
    if (key == 'e' || key == 'E') rotatieBancuta -= 5.0f;

    float rad = angle * 3.14159f / 180.0f;
    float speed = 1.0f;

    if (key == 'w' || key == 'W') { camX += sin(rad) * speed; camZ -= cos(rad) * speed; }
    if (key == 's' || key == 'S') { camX -= sin(rad) * speed; camZ += cos(rad) * speed; }
    if (key == 'a' || key == 'A') { camX -= cos(rad) * speed; camZ -= sin(rad) * speed; }
    if (key == 'd' || key == 'D') { camX += cos(rad) * speed; camZ += sin(rad) * speed; }
    if (key == 'r' || key == 'R') camY += speed;
    if (key == 'f' || key == 'F') camY -= speed;

    glutPostRedisplay();
}

void specialKeys(int key, int x, int y) {
    if (key == GLUT_KEY_RIGHT) angle += 5.0f;
    if (key == GLUT_KEY_LEFT) angle -= 5.0f;
    if (key == GLUT_KEY_UP) angleUpDown -= 5.0f;
    if (key == GLUT_KEY_DOWN) angleUpDown += 5.0f;

    if (angleUpDown > 80.0f) angleUpDown = 80.0f;
    if (angleUpDown < -80.0f) angleUpDown = -80.0f;

    glutPostRedisplay();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1000, 700);
    glutCreateWindow("Proiect P1 - Scena 3D");

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutSpecialFunc(specialKeys);
    glutKeyboardFunc(keyboard);

    glutMainLoop();
    return 0;
}