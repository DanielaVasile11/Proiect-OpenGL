#include <freeglut.h>
#include <math.h>
#include <stdlib.h> 
#include <time.h>   

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// ID-urile texturilor
GLuint iarbaRozTexture;
GLuint cerRozTexture;
GLuint munteTexture;
GLuint drumTexture;
GLuint frunzeTexture;
GLuint tufisTexture;

// ==========================================
// VARIABILE PENTRU MAȘINA TA ȘI COLIZIUNI
// ==========================================
float carX = 10.0f;
float carZ = -10.0f;
float carAngle = 0.0f;
float carSpeed = 0.8f;
float carRadius = 1.5f;

// ==========================================
// VARIABILE PENTRU MAȘINILE AUTONOME (Trafic)
// ==========================================
float autoCarAngles[3] = { 0.0f, 2.094f, 4.188f };
float autoCarSpeed = 0.02f;

float autoCarColors[3][3] = {
    {0.1f, 0.8f, 0.1f}, // Verde
    {0.8f, 0.8f, 0.1f}, // Galben
    {0.6f, 0.1f, 0.8f}  // Mov
};

// ==========================================
// VARIABILE PENTRU AVION (Zbor Aleatoriu)
// ==========================================
float planeX = 0.0f;
float planeY = 12.0f; // Altitudinea avionului
float planeZ = 0.0f;
float planeAngle = 0.0f; // Direcția (în radiani)
float planeSpeed = 0.3f;

// Structură pentru obstacole statice
struct Obstacle {
    float x, z, radius;
};

// Lista de obstacole statice
Obstacle obstacles[] = {
    {25.0f, -25.0f, 2.0f},
    {-30.0f, 15.0f, 2.0f},
    {-20.0f, -30.0f, 2.0f},
    {35.0f, 10.0f, 2.0f},
    {0.0f, 19.5f, 3.5f},
    {-19.5f, 0.0f, 3.5f},
    {0.0f, 0.0f, 7.5f}
};
int numObstacles = 7;

// Funcția de verificare a coliziunilor (PENTRU MAȘINA ROȘIE)
bool checkCollision(float nextX, float nextZ) {
    if (nextX < -48.0f || nextX > 48.0f || nextZ < -48.0f || nextZ > 48.0f) return true;

    for (int i = 0; i < numObstacles; i++) {
        float dx = nextX - obstacles[i].x;
        float dz = nextZ - obstacles[i].z;
        if (sqrt(dx * dx + dz * dz) < (carRadius + obstacles[i].radius)) return true;
    }

    for (int i = 0; i < 3; i++) {
        float cX = 16.0f * cos(autoCarAngles[i]);
        float cZ = 16.0f * sin(autoCarAngles[i]);
        float dx = nextX - cX;
        float dz = nextZ - cZ;
        if (sqrt(dx * dx + dz * dz) < (carRadius + carRadius)) return true;
    }
    return false;
}

// ==========================================
// FUNCȚII DE ÎNCĂRCARE TEXTURI
// ==========================================
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
    if (data) glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    else printf("Eroare la incarcarea texturii: %s\n", path);
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
    else printf("Eroare la incarcarea texturii transparente: %s\n", path);
    stbi_image_free(data);
    return textureID;
}

// MATRICEA PENTRU UMBRE PLANE
void shadowMatrix(GLfloat shadowMat[16], GLfloat groundplane[4], GLfloat lightpos[4]) {
    GLfloat dot = groundplane[0] * lightpos[0] + groundplane[1] * lightpos[1] + groundplane[2] * lightpos[2] + groundplane[3] * lightpos[3];
    shadowMat[0] = dot - lightpos[0] * groundplane[0]; shadowMat[4] = 0.f - lightpos[0] * groundplane[1]; shadowMat[8] = 0.f - lightpos[0] * groundplane[2]; shadowMat[12] = 0.f - lightpos[0] * groundplane[3];
    shadowMat[1] = 0.f - lightpos[1] * groundplane[0]; shadowMat[5] = dot - lightpos[1] * groundplane[1]; shadowMat[9] = 0.f - lightpos[1] * groundplane[2]; shadowMat[13] = 0.f - lightpos[1] * groundplane[3];
    shadowMat[2] = 0.f - lightpos[2] * groundplane[0]; shadowMat[6] = 0.f - lightpos[2] * groundplane[1]; shadowMat[10] = dot - lightpos[2] * groundplane[2]; shadowMat[14] = 0.f - lightpos[2] * groundplane[3];
    shadowMat[3] = 0.f - lightpos[3] * groundplane[0]; shadowMat[7] = 0.f - lightpos[3] * groundplane[1]; shadowMat[11] = 0.f - lightpos[3] * groundplane[2]; shadowMat[15] = dot - lightpos[3] * groundplane[3];
}

// VARIABILE GLOBALE DE CAMERA
float camX = 0.0f; float camY = 5.0f; float camZ = 20.0f;
float angle = 0.0f; float angleUpDown = 0.0f; float rotatieBancuta = 0.0f;

// Pozitiile Luminilor
GLfloat light_position[] = { 10.0f, 30.0f, 10.0f, 0.0f };
GLfloat mountainSun_pos[] = { 30.0f, 10.0f, 30.0f, 0.0f };
GLfloat lightPole1_pos[] = { -19.5f, 3.8f, 2.5f, 1.0f };
GLfloat lightPole2_pos[] = { 2.5f, 3.8f, 19.5f, 1.0f };
GLfloat treePole1_pos[] = { -19.5f, 12.0f, 2.5f, 1.0f };
GLfloat treePole2_pos[] = { 2.5f, 12.0f, 19.5f, 1.0f };

// ==========================================
// FUNCȚII DE DESENARE GEOMETRIE DE BAZĂ
// ==========================================
void drawBox(float sx, float sy, float sz) {
    glBegin(GL_QUADS);
    glNormal3f(0, 1, 0); glVertex3f(-sx, sy, -sz); glVertex3f(sx, sy, -sz); glVertex3f(sx, sy, sz); glVertex3f(-sx, sy, sz);
    glNormal3f(0, -1, 0); glVertex3f(-sx, -sy, -sz); glVertex3f(sx, -sy, -sz); glVertex3f(sx, -sy, sz); glVertex3f(-sx, -sy, sz);
    glNormal3f(0, 0, 1); glVertex3f(-sx, -sy, sz); glVertex3f(sx, -sy, sz); glVertex3f(sx, sy, sz); glVertex3f(-sx, sy, sz);
    glNormal3f(0, 0, -1); glVertex3f(-sx, -sy, -sz); glVertex3f(sx, -sy, -sz); glVertex3f(sx, sy, -sz); glVertex3f(-sx, sy, -sz);
    glNormal3f(1, 0, 0); glVertex3f(sx, -sy, -sz); glVertex3f(sx, -sy, sz); glVertex3f(sx, sy, sz); glVertex3f(sx, sy, -sz);
    glNormal3f(-1, 0, 0); glVertex3f(-sx, -sy, -sz); glVertex3f(-sx, -sy, sz); glVertex3f(-sx, sy, sz); glVertex3f(-sx, sy, -sz);
    glEnd();
}

void drawCylinder(float radius, float height, int slices) {
    float x, z, angleCyl;
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= slices; i++) {
        angleCyl = 2.0f * 3.14159f * i / slices;
        x = radius * cos(angleCyl); z = radius * sin(angleCyl);
        glNormal3f(cos(angleCyl), 0, sin(angleCyl));
        glVertex3f(x, height / 2.0f, z); glVertex3f(x, -height / 2.0f, z);
    }
    glEnd();
    glNormal3f(0, 1, 0); glBegin(GL_TRIANGLE_FAN); glVertex3f(0, height / 2.0f, 0);
    for (int i = 0; i <= slices; i++) { angleCyl = 2.0f * 3.14159f * i / slices; glVertex3f(radius * cos(angleCyl), height / 2.0f, radius * sin(angleCyl)); }
    glEnd();
    glNormal3f(0, -1, 0); glBegin(GL_TRIANGLE_FAN); glVertex3f(0, -height / 2.0f, 0);
    for (int i = 0; i <= slices; i++) { angleCyl = -2.0f * 3.14159f * i / slices; glVertex3f(radius * cos(angleCyl), -height / 2.0f, radius * sin(angleCyl)); }
    glEnd();
}

// ==========================================
// FUNCȚIILE DE DESENARE OBIECTE
// ==========================================

// NOU: Desenează Avionul
void drawAirplane(float x, float y, float z, float angleRot, bool isShadow = false) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(angleRot, 0.0f, 1.0f, 0.0f); // Rotația pe direcția de zbor
    glDisable(GL_TEXTURE_2D);

    // 1. Fuzelajul (Corpul principal - Alb)
    if (!isShadow) glColor3f(0.9f, 0.9f, 0.9f);
    glPushMatrix();
    drawBox(0.2f, 0.2f, 1.0f);
    glPopMatrix();

    // 2. Aripile (Roșii)
    if (!isShadow) glColor3f(0.8f, 0.1f, 0.1f);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, -0.2f); // Aripile puțin mai în față
    drawBox(1.2f, 0.05f, 0.3f);
    glPopMatrix();

    // 3. Coada (Verticală - Roșie)
    if (!isShadow) glColor3f(0.8f, 0.1f, 0.1f);
    glPushMatrix();
    glTranslatef(0.0f, 0.3f, 0.8f); // În spate și în sus
    drawBox(0.05f, 0.3f, 0.2f);
    glPopMatrix();

    glEnable(GL_TEXTURE_2D);
    glPopMatrix();
}

void drawCar(float x, float z, float angleRot, bool isShadow = false, float r = 0.8f, float g = 0.1f, float b = 0.1f) {
    glPushMatrix(); glTranslatef(x, -1.3f, z); glRotatef(angleRot, 0.0f, 1.0f, 0.0f); glDisable(GL_TEXTURE_2D);

    if (!isShadow) glColor3f(r, g, b);
    glPushMatrix(); drawBox(0.85f, 0.3f, 1.7f); glPopMatrix();

    if (!isShadow) glColor3f(0.2f, 0.2f, 0.8f);
    glPushMatrix(); glTranslatef(0.0f, 0.6f, -0.1f); drawBox(0.75f, 0.35f, 0.9f);
    if (!isShadow) {
        glColor3f(0.6f, 0.8f, 0.9f);
        glBegin(GL_QUADS);
        glNormal3f(0, 0, 1); glVertex3f(-0.7f, 0.1f, 0.91f); glVertex3f(0.7f, 0.1f, 0.91f); glVertex3f(0.7f, 0.3f, 0.91f); glVertex3f(-0.7f, 0.3f, 0.91f);
        glNormal3f(0, 0, -1); glVertex3f(-0.7f, 0.1f, -0.91f); glVertex3f(0.7f, 0.1f, -0.91f); glVertex3f(0.7f, 0.3f, -0.91f); glVertex3f(-0.7f, 0.3f, -0.91f);
        glNormal3f(1, 0, 0); glVertex3f(0.76f, 0.1f, -0.8f); glVertex3f(0.76f, 0.1f, 0.8f); glVertex3f(0.76f, 0.3f, 0.8f); glVertex3f(0.76f, 0.3f, -0.8f);
        glNormal3f(-1, 0, 0); glVertex3f(-0.76f, 0.1f, -0.8f); glVertex3f(-0.76f, 0.1f, 0.8f); glVertex3f(-0.76f, 0.3f, 0.8f); glVertex3f(-0.76f, 0.3f, -0.8f);
        glEnd();
    }
    glPopMatrix();

    if (!isShadow) glColor3f(0.1f, 0.1f, 0.1f);
    float wR = 0.4f, wW = 0.25f;
    glPushMatrix(); glTranslatef(-0.9f, -0.3f, 1.2f); glRotatef(90, 0, 0, 1); drawCylinder(wR, wW, 16); glPopMatrix();
    glPushMatrix(); glTranslatef(0.9f, -0.3f, 1.2f); glRotatef(90, 0, 0, 1); drawCylinder(wR, wW, 16); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.9f, -0.3f, -1.2f); glRotatef(90, 0, 0, 1); drawCylinder(wR, wW, 16); glPopMatrix();
    glPushMatrix(); glTranslatef(0.9f, -0.3f, -1.2f); glRotatef(90, 0, 0, 1); drawCylinder(wR, wW, 16); glPopMatrix();

    if (!isShadow) {
        glColor3f(1.0f, 0.9f, 0.0f);
        glBegin(GL_QUADS); glNormal3f(0, 0, 1); glVertex3f(-0.7f, 0.0f, 1.71f); glVertex3f(-0.4f, 0.0f, 1.71f); glVertex3f(-0.4f, 0.2f, 1.71f); glVertex3f(-0.7f, 0.2f, 1.71f); glVertex3f(0.7f, 0.0f, 1.71f); glVertex3f(0.4f, 0.0f, 1.71f); glVertex3f(0.4f, 0.2f, 1.71f); glVertex3f(0.7f, 0.2f, 1.71f); glEnd();
        glColor3f(1.0f, 0.0f, 0.0f);
        glBegin(GL_QUADS); glNormal3f(0, 0, -1); glVertex3f(-0.7f, 0.0f, -1.71f); glVertex3f(-0.4f, 0.0f, -1.71f); glVertex3f(-0.4f, 0.2f, -1.71f); glVertex3f(-0.7f, 0.2f, -1.71f); glVertex3f(0.7f, 0.0f, -1.71f); glVertex3f(0.4f, 0.0f, -1.71f); glVertex3f(0.4f, 0.2f, -1.71f); glVertex3f(0.7f, 0.2f, -1.71f); glEnd();
    }
    glEnable(GL_TEXTURE_2D); glPopMatrix();
}

void drawTree(float x, float z, bool isShadow = false) {
    float y = -2.0f; glPushMatrix(); glTranslatef(x, y, z); glDisable(GL_TEXTURE_2D);
    if (!isShadow) { glColor3f(0.4f, 0.2f, 0.1f); glNormal3f(0.0f, 1.0f, 0.0f); }
    glBegin(GL_QUADS); glVertex3f(-0.5f, 0.0f, 0.0f); glVertex3f(0.5f, 0.0f, 0.0f); glVertex3f(0.5f, 3.0f, 0.0f); glVertex3f(-0.5f, 3.0f, 0.0f); glVertex3f(0.0f, 0.0f, -0.5f); glVertex3f(0.0f, 0.0f, 0.5f); glVertex3f(0.0f, 3.0f, 0.5f); glVertex3f(0.0f, 3.0f, -0.5f); glEnd();
    glEnable(GL_TEXTURE_2D); glBindTexture(GL_TEXTURE_2D, frunzeTexture);
    if (!isShadow) glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_TRIANGLES);
    if (!isShadow) glNormal3f(0.0f, 0.5f, 1.0f); glTexCoord2f(0.5f, 1.0f); glVertex3f(0.0f, 6.0f, 0.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f(-2.5f, 2.0f, 2.5f); glTexCoord2f(1.0f, 0.0f); glVertex3f(2.5f, 2.0f, 2.5f);
    if (!isShadow) glNormal3f(1.0f, 0.5f, 0.0f); glTexCoord2f(0.5f, 1.0f); glVertex3f(0.0f, 6.0f, 0.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f(2.5f, 2.0f, 2.5f); glTexCoord2f(1.0f, 0.0f); glVertex3f(2.5f, 2.0f, -2.5f);
    if (!isShadow) glNormal3f(0.0f, 0.5f, -1.0f); glTexCoord2f(0.5f, 1.0f); glVertex3f(0.0f, 6.0f, 0.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f(2.5f, 2.0f, -2.5f); glTexCoord2f(1.0f, 0.0f); glVertex3f(-2.5f, 2.0f, -2.5f);
    if (!isShadow) glNormal3f(-1.0f, 0.5f, 0.0f); glTexCoord2f(0.5f, 1.0f); glVertex3f(0.0f, 6.0f, 0.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f(-2.5f, 2.0f, -2.5f); glTexCoord2f(1.0f, 0.0f); glVertex3f(-2.5f, 2.0f, 2.5f);
    glEnd(); glPopMatrix();
}

void drawBench(float x, float z, float angle, bool isShadow = false) {
    float y = -2.0f; glPushMatrix(); glTranslatef(x, y, z); glRotatef(angle, 0.0f, 1.0f, 0.0f); glDisable(GL_TEXTURE_2D);
    if (!isShadow) { glNormal3f(0.0f, 1.0f, 0.0f); glColor3f(0.9f, 0.8f, 0.7f); }
    glBegin(GL_QUADS); glVertex3f(-1.5f, 0.5f, -0.5f); glVertex3f(1.5f, 0.5f, -0.5f); glVertex3f(1.5f, 0.5f, 0.5f); glVertex3f(-1.5f, 0.5f, 0.5f); glEnd();
    if (!isShadow) glNormal3f(0.0f, 0.5f, 1.0f);
    glBegin(GL_QUADS); glVertex3f(-1.5f, 0.5f, -0.5f); glVertex3f(1.5f, 0.5f, -0.5f); glVertex3f(1.5f, 1.2f, -0.6f); glVertex3f(-1.5f, 1.2f, -0.6f); glEnd();
    if (!isShadow) { glNormal3f(0.0f, 0.0f, 1.0f); glColor3f(0.7f, 0.6f, 0.5f); }
    glBegin(GL_QUADS); glVertex3f(-1.3f, 0.0f, -0.3f); glVertex3f(-1.1f, 0.0f, -0.3f); glVertex3f(-1.1f, 0.5f, -0.3f); glVertex3f(-1.3f, 0.5f, -0.3f); glVertex3f(1.1f, 0.0f, -0.3f); glVertex3f(1.3f, 0.0f, -0.3f); glVertex3f(1.3f, 0.5f, -0.3f); glVertex3f(1.1f, 0.5f, -0.3f); glVertex3f(-1.3f, 0.0f, 0.3f); glVertex3f(-1.1f, 0.0f, 0.3f); glVertex3f(-1.1f, 0.5f, 0.3f); glVertex3f(-1.3f, 0.5f, 0.3f); glVertex3f(1.1f, 0.0f, 0.3f); glVertex3f(1.3f, 0.0f, 0.3f); glVertex3f(1.3f, 0.5f, 0.3f); glVertex3f(1.1f, 0.5f, 0.3f); glEnd();
    glEnable(GL_TEXTURE_2D); glPopMatrix();
}

void drawBush(float x, float z, bool isShadow = false) {
    float y = -2.0f; glPushMatrix(); glTranslatef(x, y, z);
    glEnable(GL_TEXTURE_2D); glBindTexture(GL_TEXTURE_2D, tufisTexture); glEnable(GL_ALPHA_TEST); glAlphaFunc(GL_GREATER, 0.1f);
    if (!isShadow) { glColor3f(1.0f, 1.0f, 1.0f); glNormal3f(0.0f, 1.0f, 0.0f); }
    float size = 1.0f; float height = 2.0f;
    glBegin(GL_QUADS); glTexCoord2f(0.0f, 0.0f); glVertex3f(-size, 0.0f, -size); glTexCoord2f(1.0f, 0.0f); glVertex3f(size, 0.0f, size); glTexCoord2f(1.0f, 1.0f); glVertex3f(size, height, size); glTexCoord2f(0.0f, 1.0f); glVertex3f(-size, height, -size); glTexCoord2f(0.0f, 0.0f); glVertex3f(-size, 0.0f, size); glTexCoord2f(1.0f, 0.0f); glVertex3f(size, 0.0f, -size); glTexCoord2f(1.0f, 1.0f); glVertex3f(size, height, -size); glTexCoord2f(0.0f, 1.0f); glVertex3f(-size, height, size); glEnd();
    glPopMatrix();
}

void drawMountain(bool isShadow = false) {
    if (!isShadow) { glEnable(GL_TEXTURE_2D); glBindTexture(GL_TEXTURE_2D, munteTexture); glColor3f(1.0f, 1.0f, 1.0f); }
    else glDisable(GL_TEXTURE_2D);
    glBegin(GL_TRIANGLES);
    if (!isShadow) glNormal3f(0.0f, 0.5f, 1.0f); glTexCoord2f(0.5f, 1.0f); glVertex3f(0.0f, 6.0f, 0.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f(-6.0f, -2.0f, 6.0f); glTexCoord2f(1.0f, 0.0f); glVertex3f(6.0f, -2.0f, 6.0f);
    if (!isShadow) glNormal3f(1.0f, 0.5f, 0.0f); glTexCoord2f(0.5f, 1.0f); glVertex3f(0.0f, 6.0f, 0.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f(6.0f, -2.0f, 6.0f); glTexCoord2f(1.0f, 0.0f); glVertex3f(6.0f, -2.0f, -6.0f);
    if (!isShadow) glNormal3f(0.0f, 0.5f, -1.0f); glTexCoord2f(0.5f, 1.0f); glVertex3f(0.0f, 6.0f, 0.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f(6.0f, -2.0f, -6.0f); glTexCoord2f(1.0f, 0.0f); glVertex3f(-6.0f, -2.0f, -6.0f);
    if (!isShadow) glNormal3f(-1.0f, 0.5f, 0.0f); glTexCoord2f(0.5f, 1.0f); glVertex3f(0.0f, 6.0f, 0.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f(-6.0f, -2.0f, -6.0f); glTexCoord2f(1.0f, 0.0f); glVertex3f(-6.0f, -2.0f, 6.0f);
    if (!isShadow) glNormal3f(0.0f, 1.0f, 0.0f); glTexCoord2f(0.5f, 1.0f); glVertex3f(-3.0f, 3.5f, 3.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f(-8.0f, -2.0f, 5.0f); glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, -2.0f, 7.0f); glTexCoord2f(0.5f, 1.0f); glVertex3f(-3.0f, 3.5f, 3.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, -2.0f, 7.0f); glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -2.0f, -1.0f); glTexCoord2f(0.5f, 1.0f); glVertex3f(4.0f, 4.0f, 1.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, -2.0f, 6.0f); glTexCoord2f(1.0f, 0.0f); glVertex3f(8.0f, -2.0f, 3.0f); glTexCoord2f(0.5f, 1.0f); glVertex3f(4.0f, 4.0f, 1.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f(8.0f, -2.0f, 3.0f); glTexCoord2f(1.0f, 0.0f); glVertex3f(5.0f, -2.0f, -4.0f);
    glEnd(); glEnable(GL_TEXTURE_2D);
}

void drawLamp(float x, float z) {
    float y = -2.0f; glPushMatrix(); glTranslatef(x, y, z); glDisable(GL_TEXTURE_2D); glNormal3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_QUADS); glColor3f(0.1f, 0.1f, 0.1f); glVertex3f(-0.1f, 0.0f, 0.0f); glVertex3f(0.1f, 0.0f, 0.0f); glVertex3f(0.1f, 3.5f, 0.0f); glVertex3f(-0.1f, 3.5f, 0.0f); glVertex3f(0.0f, 0.0f, -0.1f); glVertex3f(0.0f, 0.0f, 0.1f); glVertex3f(0.0f, 3.5f, 0.1f); glVertex3f(0.0f, 3.5f, -0.1f);
    glDisable(GL_LIGHTING); glColor3f(1.0f, 0.9f, 0.5f); glVertex3f(-0.4f, 3.5f, 0.0f); glVertex3f(0.4f, 3.5f, 0.0f); glVertex3f(0.4f, 4.2f, 0.0f); glVertex3f(-0.4f, 4.2f, 0.0f); glVertex3f(0.0f, 3.5f, -0.4f); glVertex3f(0.0f, 3.5f, 0.4f); glVertex3f(0.0f, 4.2f, 0.4f); glVertex3f(0.0f, 4.2f, -0.4f); glEnable(GL_LIGHTING); glEnd();
    glEnable(GL_TEXTURE_2D); glPopMatrix();
}

void castShadow(int type, float x, float z, float angleRot, GLfloat lightPos[]) {
    GLfloat groundPlane[4] = { 0.0f, 1.0f, 0.0f, 1.90f };
    GLfloat shadowMat[16]; shadowMatrix(shadowMat, groundPlane, lightPos);
    glPushMatrix(); glMultMatrixf(shadowMat);
    if (type == 1) drawTree(x, z, true);
    else if (type == 2) drawBench(x, z, angleRot, true);
    else if (type == 3) drawBush(x, z, true);
    else if (type == 4) drawMountain(true);
    else if (type == 5) drawCar(x, z, angleRot, true);
    else if (type == 6) drawAirplane(x, planeY, z, angleRot, true); // UMBRA AVIONULUI
    glPopMatrix();
}

// ==========================================
// FUNCȚIA SCENEI PRINCIPALE
// ==========================================
void drawScene() {
    glLightfv(GL_LIGHT1, GL_POSITION, lightPole1_pos);
    glLightfv(GL_LIGHT2, GL_POSITION, lightPole2_pos);

    // 1. Iarba 
    glBindTexture(GL_TEXTURE_2D, iarbaRozTexture); glColor3f(1.0f, 1.0f, 1.0f); glBegin(GL_QUADS); glNormal3f(0.0f, 1.0f, 0.0f);
    for (float x = -50.0f; x < 50.0f; x += 2.0f) {
        for (float z = -50.0f; z < 50.0f; z += 2.0f) {
            float u1 = (x + 50.0f) / 10.0f, v1 = (z + 50.0f) / 10.0f, u2 = (x + 52.0f) / 10.0f, v2 = v1, u3 = u2, v3 = (z + 52.0f) / 10.0f, u4 = u1, v4 = v3;
            glTexCoord2f(u1, v1); glVertex3f(x, -2.0f, z); glTexCoord2f(u2, v2); glVertex3f(x + 2.0f, -2.0f, z); glTexCoord2f(u3, v3); glVertex3f(x + 2.0f, -2.0f, z + 2.0f); glTexCoord2f(u4, v4); glVertex3f(x, -2.0f, z + 2.0f);
        }
    } glEnd();

    // 2. Drumul 
    glBindTexture(GL_TEXTURE_2D, drumTexture); glBegin(GL_QUADS); glNormal3f(0.0f, 1.0f, 0.0f);
    for (int unghi = 0; unghi < 360; unghi += 10) {
        float rad1 = unghi * 3.14159f / 180.0f, rad2 = (unghi + 10) * 3.14159f / 180.0f;
        float x_in1 = 14.0f * cos(rad1), z_in1 = 14.0f * sin(rad1), x_out1 = 18.0f * cos(rad1), z_out1 = 18.0f * sin(rad1), x_in2 = 14.0f * cos(rad2), z_in2 = 14.0f * sin(rad2), x_out2 = 18.0f * cos(rad2), z_out2 = 18.0f * sin(rad2), y_road = -1.95f;
        glTexCoord2f(0.0f, unghi / 20.0f); glVertex3f(x_in1, y_road, z_in1); glTexCoord2f(1.0f, unghi / 20.0f); glVertex3f(x_out1, y_road, z_out1); glTexCoord2f(1.0f, (unghi + 10) / 20.0f); glVertex3f(x_out2, y_road, z_out2); glTexCoord2f(0.0f, (unghi + 10) / 20.0f); glVertex3f(x_in2, y_road, z_in2);
    } glEnd();

    // --------------------------------------------------------
    // 3. UMBRELE 
    // --------------------------------------------------------
    glDisable(GL_LIGHTING); glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); glEnable(GL_TEXTURE_2D);

    glColor4f(0.1f, 0.1f, 0.1f, 0.4f);
    castShadow(1, 25.0f, -25.0f, 0.0f, light_position); castShadow(1, -30.0f, 15.0f, 0.0f, light_position); castShadow(1, -20.0f, -30.0f, 0.0f, light_position); castShadow(1, 35.0f, 10.0f, 0.0f, light_position);
    castShadow(2, 0.0f, 19.5f, 180.0f + rotatieBancuta, light_position); castShadow(2, -19.5f, 0.0f, 90.0f, light_position);
    castShadow(3, -3.0f, 19.5f, 0.0f, light_position); castShadow(3, -19.5f, -3.0f, 0.0f, light_position);
    castShadow(4, 0.0f, 0.0f, 0.0f, mountainSun_pos);
    castShadow(5, carX, carZ, carAngle, light_position);
    castShadow(6, planeX, planeZ, planeAngle * 180.0f / 3.14159f, light_position); // UMBRA AVIONULUI de la SOARE

    for (int i = 0; i < 3; i++) {
        float cX = 16.0f * cos(autoCarAngles[i]), cZ = 16.0f * sin(autoCarAngles[i]), cRot = -autoCarAngles[i] * 180.0f / 3.14159f;
        castShadow(5, cX, cZ, cRot, light_position);
    }

    glColor4f(0.1f, 0.1f, 0.1f, 0.6f);
    castShadow(2, -19.5f, 0.0f, 90.0f, lightPole1_pos); castShadow(3, -19.5f, -3.0f, 0.0f, lightPole1_pos); castShadow(1, -30.0f, 15.0f, 0.0f, treePole1_pos); castShadow(1, -20.0f, -30.0f, 0.0f, treePole1_pos);
    castShadow(2, 0.0f, 19.5f, 180.0f + rotatieBancuta, lightPole2_pos); castShadow(3, -3.0f, 19.5f, 0.0f, lightPole2_pos); castShadow(1, 25.0f, -25.0f, 0.0f, treePole2_pos); castShadow(1, 35.0f, 10.0f, 0.0f, treePole2_pos);
    castShadow(5, carX, carZ, carAngle, treePole2_pos);

    for (int i = 0; i < 3; i++) {
        float cX = 16.0f * cos(autoCarAngles[i]), cZ = 16.0f * sin(autoCarAngles[i]), cRot = -autoCarAngles[i] * 180.0f / 3.14159f;
        castShadow(5, cX, cZ, cRot, treePole1_pos); castShadow(5, cX, cZ, cRot, treePole2_pos);
    }

    glDisable(GL_BLEND); glEnable(GL_LIGHTING); glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    // 4. Muntele 
    drawMountain(false);

    // 5. Obiectele Principale 3D
    drawTree(25.0f, -25.0f); drawTree(-30.0f, 15.0f); drawTree(-20.0f, -30.0f); drawTree(35.0f, 10.0f);
    drawBench(0.0f, 19.5f, 180.0f + rotatieBancuta); drawBench(-19.5f, 0.0f, 90.0f);
    drawLamp(2.5f, 19.5f); drawLamp(-19.5f, 2.5f);
    drawBush(-3.0f, 19.5f); drawBush(-19.5f, -3.0f);

    // Mașina Ta 
    drawCar(carX, carZ, carAngle);

    // Mașinile din Trafic
    for (int i = 0; i < 3; i++) {
        float cX = 16.0f * cos(autoCarAngles[i]), cZ = 16.0f * sin(autoCarAngles[i]), cRot = -autoCarAngles[i] * 180.0f / 3.14159f;
        drawCar(cX, cZ, cRot, false, autoCarColors[i][0], autoCarColors[i][1], autoCarColors[i][2]);
    }

    // DESENEAZĂ AVIONUL ZBURĂTOR 
    drawAirplane(planeX, planeY, planeZ, planeAngle * 180.0f / 3.14159f);

    // 6. Cerul
    glDisable(GL_LIGHTING); glBindTexture(GL_TEXTURE_2D, cerRozTexture); glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-50.0f, -10.0f, -50.0f); glTexCoord2f(1.0f, 0.0f); glVertex3f(50.0f, -10.0f, -50.0f); glTexCoord2f(1.0f, 1.0f); glVertex3f(50.0f, 20.0f, -50.0f); glTexCoord2f(0.0f, 1.0f); glVertex3f(-50.0f, 20.0f, -50.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(50.0f, -10.0f, 50.0f); glTexCoord2f(1.0f, 0.0f); glVertex3f(-50.0f, -10.0f, 50.0f); glTexCoord2f(1.0f, 1.0f); glVertex3f(-50.0f, 20.0f, 50.0f); glTexCoord2f(0.0f, 1.0f); glVertex3f(50.0f, 20.0f, 50.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-50.0f, -10.0f, 50.0f); glTexCoord2f(1.0f, 0.0f); glVertex3f(-50.0f, -10.0f, -50.0f); glTexCoord2f(1.0f, 1.0f); glVertex3f(-50.0f, 20.0f, -50.0f); glTexCoord2f(0.0f, 1.0f); glVertex3f(-50.0f, 20.0f, 50.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(50.0f, -10.0f, -50.0f); glTexCoord2f(1.0f, 0.0f); glVertex3f(50.0f, -10.0f, 50.0f); glTexCoord2f(1.0f, 1.0f); glVertex3f(50.0f, 20.0f, 50.0f); glTexCoord2f(0.0f, 1.0f); glVertex3f(50.0f, 20.0f, -50.0f);
    glEnd(); glEnable(GL_LIGHTING);
}

void init() {
    glEnable(GL_DEPTH_TEST); glEnable(GL_TEXTURE_2D);
    iarbaRozTexture = loadTexture("amestec-flori.jpg"); cerRozTexture = loadTexture("cer_roz.jpg"); munteTexture = loadTexture("munte7.jpg"); drumTexture = loadTexture("drum8.jpg"); frunzeTexture = loadTexture("frunze1.jpg"); tufisTexture = loadTextureTransparent("tufis2.jpg");
    glEnable(GL_ALPHA_TEST); glAlphaFunc(GL_GREATER, 0.1f);
    glEnable(GL_LIGHTING); glEnable(GL_LIGHT0);
    GLfloat sunAmbient[] = { 0.3f, 0.3f, 0.3f, 1.0f }; GLfloat sunDiffuse[] = { 0.6f, 0.6f, 0.6f, 1.0f }; glLightfv(GL_LIGHT0, GL_AMBIENT, sunAmbient); glLightfv(GL_LIGHT0, GL_DIFFUSE, sunDiffuse);
    glEnable(GL_LIGHT1); glEnable(GL_LIGHT2); GLfloat poleDiffuse[] = { 0.8f, 0.7f, 0.2f, 1.0f }; glLightfv(GL_LIGHT1, GL_DIFFUSE, poleDiffuse); glLightfv(GL_LIGHT2, GL_DIFFUSE, poleDiffuse);
    GLfloat no_mat[] = { 0.0f, 0.0f, 0.0f, 1.0f }; glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, no_mat); glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.0f);
    glEnable(GL_COLOR_MATERIAL); glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); glLoadIdentity();
    gluLookAt(camX, camY, camZ, camX, camY, camZ - 1.0, 0.0, 1.0, 0.0);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glRotatef(angleUpDown, 1.0f, 0.0f, 0.0f); glRotatef(angle, 0.0f, 1.0f, 0.0f);
    drawScene(); glutSwapBuffers();
}

void reshape(int w, int h) {
    if (h == 0) h = 1; float aspect = (float)w / (float)h; glViewport(0, 0, w, h); glMatrixMode(GL_PROJECTION); glLoadIdentity(); gluPerspective(45.0f, aspect, 0.1f, 200.0f); glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int x, int y) {
    if (key == 'q' || key == 'Q') rotatieBancuta += 5.0f;
    if (key == 'e' || key == 'E') rotatieBancuta -= 5.0f;
    float radCam = angle * 3.14159f / 180.0f, speed = 1.0f;
    if (key == 'w' || key == 'W') { camX += sin(radCam) * speed; camZ -= cos(radCam) * speed; }
    if (key == 's' || key == 'S') { camX -= sin(radCam) * speed; camZ += cos(radCam) * speed; }
    if (key == 'a' || key == 'A') { camX -= cos(radCam) * speed; camZ -= sin(radCam) * speed; }
    if (key == 'd' || key == 'D') { camX += cos(radCam) * speed; camZ += sin(radCam) * speed; }
    if (key == 'r' || key == 'R') camY += speed;
    if (key == 'f' || key == 'F') camY -= speed;

    if (key == 'i' || key == 'I') {
        float radCar = carAngle * 3.14159f / 180.0f, nextX = carX + sin(radCar) * carSpeed, nextZ = carZ + cos(radCar) * carSpeed;
        if (!checkCollision(nextX, nextZ)) { carX = nextX; carZ = nextZ; }
    }
    if (key == 'k' || key == 'K') {
        float radCar = carAngle * 3.14159f / 180.0f, nextX = carX - sin(radCar) * carSpeed, nextZ = carZ - cos(radCar) * carSpeed;
        if (!checkCollision(nextX, nextZ)) { carX = nextX; carZ = nextZ; }
    }
    if (key == 'j' || key == 'J') carAngle += 5.0f;
    if (key == 'l' || key == 'L') carAngle -= 5.0f;

    glutPostRedisplay();
}

void specialKeys(int key, int x, int y) {
    if (key == GLUT_KEY_RIGHT) angle += 5.0f; if (key == GLUT_KEY_LEFT) angle -= 5.0f;
    if (key == GLUT_KEY_UP) angleUpDown -= 5.0f; if (key == GLUT_KEY_DOWN) angleUpDown += 5.0f;
    if (angleUpDown > 80.0f) angleUpDown = 80.0f; if (angleUpDown < -80.0f) angleUpDown = -80.0f;
    glutPostRedisplay();
}

// MOTORUL DE ANIMAȚIE PENTRU TRAFIC ȘI AVION
void timer(int value) {
    // 1. Calcul Trafic Mașini
    for (int i = 0; i < 3; i++) {
        float nextAngle = autoCarAngles[i] + autoCarSpeed;
        float nextX = 16.0f * cos(nextAngle);
        float nextZ = 16.0f * sin(nextAngle);
        bool hasCollision = false;

        float dx = nextX - carX; float dz = nextZ - carZ;
        if (sqrt(dx * dx + dz * dz) < (carRadius * 2.0f)) hasCollision = true;

        for (int j = 0; j < 3; j++) {
            if (i != j) {
                float otherX = 16.0f * cos(autoCarAngles[j]);
                float otherZ = 16.0f * sin(autoCarAngles[j]);
                if (sqrt((nextX - otherX) * (nextX - otherX) + (nextZ - otherZ) * (nextZ - otherZ)) < (carRadius * 2.0f)) hasCollision = true;
            }
        }
        if (!hasCollision) {
            autoCarAngles[i] = nextAngle;
            if (autoCarAngles[i] > 2.0f * 3.14159f) autoCarAngles[i] -= 2.0f * 3.14159f;
        }
    }

    // 2. Calcul Zbor Avion (Mișcare aleatoare fluidă)
    planeX += sin(planeAngle) * planeSpeed;
    planeZ += cos(planeAngle) * planeSpeed;

    // Se întoarce la 180 grade dacă ajunge la marginea hărții
    if (planeX < -40.0f || planeX > 40.0f || planeZ < -40.0f || planeZ > 40.0f) {
        planeAngle += 3.14159f;
    }
    // Are 2% șanse în fiecare cadru să facă un viraj fin
    else if (rand() % 100 < 2) {
        planeAngle += ((rand() % 100) / 50.0f) - 1.0f;
    }

    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

int main(int argc, char** argv) {
    srand((unsigned int)time(NULL));

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1000, 700);
    glutCreateWindow("Proiect P1 - Scena 3D Trafic + Avion");

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutSpecialFunc(specialKeys);
    glutKeyboardFunc(keyboard);

    glutTimerFunc(16, timer, 0);

    glutMainLoop();
    return 0;
}