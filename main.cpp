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

// Încarc o imagine și o transform în textură
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
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);

    if (data) {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    }
    else {
        printf("Eroare la incarcarea texturii: %s\n", path);
    }

    stbi_image_free(data);
    return textureID;
}

// Funcție pentru a elimina fundalul alb dintr-un JPG
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
            if (data[i] > 240 && data[i + 1] > 240 && data[i + 2] > 240) {
                data[i + 3] = 0;
            }
            else {
                data[i + 3] = 255;
            }
        }
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    }
    else {
        printf("Eroare la incarcarea texturii transparente: %s\n", path);
    }

    stbi_image_free(data);
    return textureID;
}


// MATRICEA PENTRU UMBRE 

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

float camX = 0.0f;
float camY = 5.0f;
float camZ = 20.0f;

float angle = 0.0f;
float angleUpDown = 0.0f;
float rotatieBancuta = 0.0f;

GLfloat light_position[] = { 10.0f, 30.0f, 10.0f, 1.0f }; // Soarele principal pe cer

// 1. FUNCȚIA PENTRU COPAC
void drawTree(float x, float z) {
    float y = sin(x / 5.0f) * cos(z / 5.0f) * 2.0f - 2.0f;
    glPushMatrix();
    glTranslatef(x, y, z);

    glDisable(GL_TEXTURE_2D);
    glColor3f(0.4f, 0.2f, 0.1f);

    glNormal3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex3f(-0.5f, 0.0f, 0.0f); glVertex3f(0.5f, 0.0f, 0.0f);
    glVertex3f(0.5f, 3.0f, 0.0f); glVertex3f(-0.5f, 3.0f, 0.0f);

    glVertex3f(0.0f, 0.0f, -0.5f); glVertex3f(0.0f, 0.0f, 0.5f);
    glVertex3f(0.0f, 3.0f, 0.5f); glVertex3f(0.0f, 3.0f, -0.5f);
    glEnd();
    glEnable(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, frunzeTexture);
    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_TRIANGLES);
    glNormal3f(0.0f, 0.5f, 1.0f);
    glTexCoord2f(0.5f, 1.0f); glVertex3f(0.0f, 6.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-2.5f, 2.0f, 2.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(2.5f, 2.0f, 2.5f);

    glNormal3f(1.0f, 0.5f, 0.0f);
    glTexCoord2f(0.5f, 1.0f); glVertex3f(0.0f, 6.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(2.5f, 2.0f, 2.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(2.5f, 2.0f, -2.5f);

    glNormal3f(0.0f, 0.5f, -1.0f);
    glTexCoord2f(0.5f, 1.0f); glVertex3f(0.0f, 6.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(2.5f, 2.0f, -2.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-2.5f, 2.0f, -2.5f);

    glNormal3f(-1.0f, 0.5f, 0.0f);
    glTexCoord2f(0.5f, 1.0f); glVertex3f(0.0f, 6.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-2.5f, 2.0f, -2.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-2.5f, 2.0f, 2.5f);
    glEnd();
    glPopMatrix();
}

// 2. FUNCȚIA PENTRU BĂNCUȚĂ 
void drawBench(float x, float z, float angle) {
    float y = sin(x / 5.0f) * cos(z / 5.0f) * 2.0f - 2.0f;
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(angle, 0.0f, 1.0f, 0.0f);

    glDisable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);

    // Șezutul (Săgeata în sus)
    glNormal3f(0.0f, 1.0f, 0.0f);
    glColor3f(0.9f, 0.8f, 0.7f);
    glVertex3f(-1.5f, 0.5f, -0.5f); glVertex3f(1.5f, 0.5f, -0.5f);
    glVertex3f(1.5f, 0.5f, 0.5f); glVertex3f(-1.5f, 0.5f, 0.5f);

    // Spătarul (Săgeata spre față, să prindă lumina din față/lateral)
    glNormal3f(0.0f, 0.5f, 1.0f);
    glVertex3f(-1.5f, 0.5f, -0.5f); glVertex3f(1.5f, 0.5f, -0.5f);
    glVertex3f(1.5f, 1.2f, -0.6f); glVertex3f(-1.5f, 1.2f, -0.6f);

    // Picioarele (Săgeata spre față)
    glNormal3f(0.0f, 0.0f, 1.0f);
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
    glNormal3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_QUADS);
    glColor3f(0.1f, 0.1f, 0.1f);
    glVertex3f(-0.1f, 0.0f, 0.0f); glVertex3f(0.1f, 0.0f, 0.0f);
    glVertex3f(0.1f, 3.5f, 0.0f); glVertex3f(-0.1f, 3.5f, 0.0f);

    glVertex3f(0.0f, 0.0f, -0.1f); glVertex3f(0.0f, 0.0f, 0.1f);
    glVertex3f(0.0f, 3.5f, 0.1f); glVertex3f(0.0f, 3.5f, -0.1f);

    // Oprim calculul de umbră pentru bec ca să pară mereu aprins (strălucitor)
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

// 4. FUNCȚIA PENTRU TUFIȘ 
void drawBush(float x, float z) {
    float y = sin(x / 5.0f) * cos(z / 5.0f) * 2.0f - 2.0f;
    glPushMatrix();
    glTranslatef(x, y, z);

    glBindTexture(GL_TEXTURE_2D, tufisTexture);
    glColor3f(1.0f, 1.0f, 1.0f);
    glNormal3f(0.0f, 1.0f, 0.0f);

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

void drawScene() {
    // 1. Iarba
    glBindTexture(GL_TEXTURE_2D, iarbaRozTexture);
    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);
    for (float x = -50.0f; x < 50.0f; x += 2.0f) {
        for (float z = -50.0f; z < 50.0f; z += 2.0f) {
            float y1 = sin(x / 5.0f) * cos(z / 5.0f) * 2.0f;
            float y2 = sin((x + 2.0f) / 5.0f) * cos(z / 5.0f) * 2.0f;
            float y3 = sin((x + 2.0f) / 5.0f) * cos((z + 2.0f) / 5.0f) * 2.0f;
            float y4 = sin(x / 5.0f) * cos((z + 2.0f) / 5.0f) * 2.0f;

            float u1 = (x + 50.0f) / 10.0f; float v1 = (z + 50.0f) / 10.0f;
            float u2 = (x + 52.0f) / 10.0f; float v2 = v1;
            float u3 = u2;                  float v3 = (z + 52.0f) / 10.0f;
            float u4 = u1;                  float v4 = v3;

            glNormal3f(0.0f, 1.0f, 0.0f);

            glTexCoord2f(u1, v1); glVertex3f(x, y1 - 2.0f, z);
            glTexCoord2f(u2, v2); glVertex3f(x + 2.0f, y2 - 2.0f, z);
            glTexCoord2f(u3, v3); glVertex3f(x + 2.0f, y3 - 2.0f, z + 2.0f);
            glTexCoord2f(u4, v4); glVertex3f(x, y4 - 2.0f, z + 2.0f);
        }
    }
    glEnd();

    // 2. Drumul
    glBindTexture(GL_TEXTURE_2D, drumTexture);
    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);
    for (int unghi = 0; unghi < 360; unghi += 10) {
        float rad1 = unghi * 3.14159f / 180.0f;
        float rad2 = (unghi + 10) * 3.14159f / 180.0f;

        float x_in1 = 14.0f * cos(rad1); float z_in1 = 14.0f * sin(rad1);
        float x_out1 = 18.0f * cos(rad1); float z_out1 = 18.0f * sin(rad1);
        float y_in1 = sin(x_in1 / 5.0f) * cos(z_in1 / 5.0f) * 2.0f - 1.5f;
        float y_out1 = sin(x_out1 / 5.0f) * cos(z_out1 / 5.0f) * 2.0f - 1.5f;

        float x_in2 = 14.0f * cos(rad2); float z_in2 = 14.0f * sin(rad2);
        float x_out2 = 18.0f * cos(rad2); float z_out2 = 18.0f * sin(rad2);
        float y_in2 = sin(x_in2 / 5.0f) * cos(z_in2 / 5.0f) * 2.0f - 1.5f;
        float y_out2 = sin(x_out2 / 5.0f) * cos(z_out2 / 5.0f) * 2.0f - 1.5f;

        glNormal3f(0.0f, 1.0f, 0.0f);

        glTexCoord2f(0.0f, unghi / 20.0f);        glVertex3f(x_in1, y_in1, z_in1);
        glTexCoord2f(1.0f, unghi / 20.0f);        glVertex3f(x_out1, y_out1, z_out1);
        glTexCoord2f(1.0f, (unghi + 10) / 20.0f); glVertex3f(x_out2, y_out2, z_out2);
        glTexCoord2f(0.0f, (unghi + 10) / 20.0f); glVertex3f(x_in2, y_in2, z_in2);
    }
    glEnd();

    // POZIȚIONAREA LUMINILOR DE LA STÂLPI
    // Calculăm unde este becul Stâlpului 1 (stânga)
    float y_stalp1 = sin(-19.5f / 5.0f) * cos(2.5f / 5.0f) * 2.0f - 2.0f;
    GLfloat lightPole1_pos[] = { -19.5f, y_stalp1 + 3.8f, 2.5f, 1.0f };
    glLightfv(GL_LIGHT1, GL_POSITION, lightPole1_pos); // Aprindem fizic Stâlpul 1

    // Calculăm unde este becul Stâlpului 2 (față)
    float y_stalp2 = sin(2.5f / 5.0f) * cos(19.5f / 5.0f) * 2.0f - 2.0f;
    GLfloat lightPole2_pos[] = { 2.5f, y_stalp2 + 3.8f, 19.5f, 1.0f };
    glLightfv(GL_LIGHT2, GL_POSITION, lightPole2_pos); // Aprindem fizic Stâlpul 2

   
    // 3. UMBRE MULTIPLE PENTRU AMBELE BĂNCUȚE
    
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLfloat shadowMat[16];

    // --- UMBRELE PENTRU BĂNCUȚA 1 (Stânga) ---
    float y_banca1 = sin(-19.5f / 5.0f) * cos(0.0f / 5.0f) * 2.0f - 2.0f;
    GLfloat groundPlane1[4] = { 0.0f, 1.0f, 0.0f, -(y_banca1 + 0.1f) };

    // Umbra de la Stâlpul 1
    glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
    shadowMatrix(shadowMat, groundPlane1, lightPole1_pos);
    glPushMatrix();
    glMultMatrixf(shadowMat);
    drawBench(-19.5f, 0.0f, 90.0f);
    glPopMatrix();

    // Umbra de la Soare
    glColor4f(0.0f, 0.0f, 0.0f, 0.3f);
    shadowMatrix(shadowMat, groundPlane1, light_position);
    glPushMatrix();
    glMultMatrixf(shadowMat);
    drawBench(-19.5f, 0.0f, 90.0f);
    glPopMatrix();


    // --- UMBRELE PENTRU BĂNCUȚA 2 (Față, Rotativă) ---
    float y_banca2 = sin(0.0f / 5.0f) * cos(19.5f / 5.0f) * 2.0f - 2.0f;
    GLfloat groundPlane2[4] = { 0.0f, 1.0f, 0.0f, -(y_banca2 + 0.1f) };

    // Umbra de la Stâlpul 2
    glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
    shadowMatrix(shadowMat, groundPlane2, lightPole2_pos);
    glPushMatrix();
    glMultMatrixf(shadowMat);
    drawBench(0.0f, 19.5f, 180.0f + rotatieBancuta);
    glPopMatrix();

    // Umbra de la Soare
    glColor4f(0.0f, 0.0f, 0.0f, 0.3f);
    shadowMatrix(shadowMat, groundPlane2, light_position);
    glPushMatrix();
    glMultMatrixf(shadowMat);
    drawBench(0.0f, 19.5f, 180.0f + rotatieBancuta);
    glPopMatrix();

    // Repornim totul la normal
    glDisable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    // 4. Muntele
    glBindTexture(GL_TEXTURE_2D, munteTexture);
    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_TRIANGLES);
    glNormal3f(0.0f, 0.5f, 1.0f);
    glTexCoord2f(0.5f, 1.0f); glVertex3f(0.0f, 6.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-6.0f, -2.0f, 6.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(6.0f, -2.0f, 6.0f);

    glNormal3f(1.0f, 0.5f, 0.0f);
    glTexCoord2f(0.5f, 1.0f); glVertex3f(0.0f, 6.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(6.0f, -2.0f, 6.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(6.0f, -2.0f, -6.0f);

    glNormal3f(0.0f, 0.5f, -1.0f);
    glTexCoord2f(0.5f, 1.0f); glVertex3f(0.0f, 6.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(6.0f, -2.0f, -6.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-6.0f, -2.0f, -6.0f);

    glNormal3f(-1.0f, 0.5f, 0.0f);
    glTexCoord2f(0.5f, 1.0f); glVertex3f(0.0f, 6.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-6.0f, -2.0f, -6.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-6.0f, -2.0f, 6.0f);

    glNormal3f(0.0f, 1.0f, 0.0f);
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

    // 5. Obiectele Statice
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

    iarbaRozTexture = loadTexture("amestec-flori.jpg");
    cerRozTexture = loadTexture("cer_roz.jpg");
    munteTexture = loadTexture("munte7.jpg");
    drumTexture = loadTexture("drum8.jpg");
    frunzeTexture = loadTexture("frunze1.jpg");
    tufisTexture = loadTextureTransparent("tufis2.jpg");

    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.1f);

    // SISTEMUL DE LUMINI (1 Soare + 2 Stâlpi)
    glEnable(GL_LIGHTING);

    // Soarele
    glEnable(GL_LIGHT0);
    GLfloat sunAmbient[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    GLfloat sunDiffuse[] = { 0.6f, 0.6f, 0.6f, 1.0f };
    glLightfv(GL_LIGHT0, GL_AMBIENT, sunAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, sunDiffuse);

    // Stâlpii de Iluminat (Lumină locală galbenă)
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