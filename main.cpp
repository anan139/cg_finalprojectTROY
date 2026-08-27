/*
 * ============================================================
 *   THE FALL OF TROY  —  Four Animated OpenGL Scenes
 * ============================================================
 *   Controls
 *     N / n ........... Next scene
 *     P / p ........... Previous scene
 *     1 – 4 .......... Jump to scene
 *     ESC ............. Quit
 * ============================================================ */

#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/glut.h>
#include <cmath>
#include <cstdlib>
#include <cstdio>

/* ============================================================
 *  Constants
 *  ============================================================ */

static const int   WIN_W = 1200;
static const int   WIN_H = 750;
static const float PI    = 3.14159265358979f;

/* Gate centre & width — consistent across all wall scenes */
static const float GATE_CX         = 600.0f;
static const float GATE_W          = 240.0f;

/* Horse position constants */
static const float HORSE_START_X   = 950.0f;   /* scene 1 & scene 2 start (RIGHT side) */
static const float HORSE_END_SCALE = 0.50f;    /* scene 2 "inside" scale              */
static const float S3_SCALE        = 0.75f;    /* scene 3 "parked inside"             */

/* ============================================================
 *  Global animation state
 *  ============================================================ */

static int   scene     = 1;
static float gTime     = 0.0f;
static int   lastMs    = 0;

/* Scene 2 */
static float s2_horseX     = HORSE_START_X;
static float s2_scale      = 1.0f;
static float s2_wheelAng   = 0.0f;
static float s2_gateOpen   = 0.0f;
static float s2_timer      = 0.0f;

/* Scene 3 */
static float s3_trapAng    = 0.0f;
static float s3_ropeLen[6];
static float s3_timer      = 0.0f;
static float s3_gateOpen   = 0.0f;

/* Scene 4 */
static float s4_phase[12];

/* ============================================================
 *  Initialisation
 *  ============================================================ */

static void init()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glClearColor(0, 0, 0, 1);
    for (int i = 0; i < 12; i++)
        s4_phase[i] = (rand() % 1000) / 1000.0f * 2.0f * PI;
}

/* ============================================================
 *  Primitive helpers
 *  ============================================================ */

static void drawRect(float x, float y, float w, float h)
{
    glBegin(GL_QUADS);
    glVertex2f(x, y);     glVertex2f(x + w, y);
    glVertex2f(x + w, y + h); glVertex2f(x, y + h);
    glEnd();
}

static void drawCircleFill(float cx, float cy, float r, int n = 36)
{
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= n; i++) {
        float a = 2.0f * PI * i / n;
        glVertex2f(cx + r * cosf(a), cy + r * sinf(a));
    }
    glEnd();
}

static void drawCircleLine(float cx, float cy, float r, int n = 36)
{
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < n; i++) {
        float a = 2.0f * PI * i / n;
        glVertex2f(cx + r * cosf(a), cy + r * sinf(a));
    }
    glEnd();
}

static void drawTri(float x1, float y1,
                    float x2, float y2,
                    float x3, float y3)
{
    glBegin(GL_TRIANGLES);
    glVertex2f(x1, y1); glVertex2f(x2, y2); glVertex2f(x3, y3);
    glEnd();
}

static void drawLine(float x1, float y1, float x2, float y2)
{
    glBegin(GL_LINES);
    glVertex2f(x1, y1); glVertex2f(x2, y2);
    glEnd();
}

static void drawText(float x, float y, const char *s,
                     void *font = GLUT_BITMAP_HELVETICA_18)
{
    glRasterPos2f(x, y);
    for (; *s; ++s) glutBitmapCharacter(font, (unsigned char)*s);
}

/* ============================================================
 *  Wheel
 *  ============================================================ */

static void drawWheel(float cx, float cy, float r, float angle)
{
    glColor3f(0.35f, 0.20f, 0.08f);
    glLineWidth(4.0f);
    drawCircleLine(cx, cy, r);
    glColor3f(0.30f, 0.17f, 0.06f);
    glLineWidth(2.0f);
    drawCircleLine(cx, cy, r * 0.85f);
    glColor3f(0.32f, 0.18f, 0.07f);
    glLineWidth(3.0f);
    glBegin(GL_LINES);
    for (int i = 0; i < 8; i++) {
        float a = angle + 2.0f * PI * i / 8.0f;
        glVertex2f(cx, cy);
        glVertex2f(cx + r * 0.82f * cosf(a),
                   cy + r * 0.82f * sinf(a));
    }
    glEnd();
    glColor3f(0.25f, 0.14f, 0.04f);
    drawCircleFill(cx, cy, r * 0.13f, 12);
    glColor3f(0.50f, 0.35f, 0.10f);
    drawCircleFill(cx, cy, r * 0.05f, 8);
    glLineWidth(1.0f);
}

/* ============================================================
 *  Stick-figure soldier
 *  ============================================================ */

static void drawSoldier(float x, float y, float s, bool greek)
{
    glColor3f(0.85f, 0.70f, 0.55f);
    drawCircleFill(x, y + 28 * s, 6 * s, 12);
    if (greek) {
        glColor3f(0.70f, 0.60f, 0.20f);
        drawTri(x - 7 * s, y + 30 * s, x + 7 * s, y + 30 * s,
                x, y + 42 * s);
    } else {
        glColor3f(0.60f, 0.15f, 0.10f);
        drawRect(x - 7 * s, y + 28 * s, 14 * s, 8 * s);
    }
    glColor3f(greek ? 0.30f : 0.60f,
              greek ? 0.20f : 0.15f,
              greek ? 0.50f : 0.10f);
    glLineWidth(2.0f);
    drawLine(x, y + 22 * s, x, y + 5 * s);
    drawLine(x, y + 17 * s, x - 8 * s, y + 12 * s);
    drawLine(x, y + 17 * s, x + 8 * s, y + 22 * s);
    if (greek) {
        glColor3f(0.50f, 0.40f, 0.25f);
        drawLine(x + 8 * s, y + 22 * s, x + 8 * s, y + 48 * s);
        glColor3f(0.75f, 0.65f, 0.30f);
        drawTri(x + 6 * s, y + 48 * s, x + 10 * s, y + 48 * s,
                x + 8 * s, y + 53 * s);
    }
    glColor3f(0.30f, 0.20f, 0.12f);
    drawLine(x, y + 5 * s, x - 6 * s, y - 8 * s);
    drawLine(x, y + 5 * s, x + 6 * s, y - 8 * s);
    glLineWidth(1.0f);
}

/* ============================================================
 *  Cheering person (Scene 2 — crowds celebrating)
 *  ============================================================ */

static void drawCheeringPerson(float x, float y, float s)
{
    float bounce = 3.0f * fabsf(sinf(gTime * 4.0f + x * 0.05f));
    y += bounce;

    /* Head */
    glColor3f(0.85f, 0.70f, 0.55f);
    drawCircleFill(x, y + 28 * s, 5 * s, 10);

    /* Body — Trojan red tunic */
    glColor3f(0.60f, 0.15f, 0.10f);
    drawRect(x - 6 * s, y + 10 * s, 12 * s, 18 * s);

    /* Arms raised cheering */
    glColor3f(0.85f, 0.70f, 0.55f);
    glLineWidth(2.0f);
    float armSwing = 5.0f * sinf(gTime * 5.0f + x * 0.1f);
    drawLine(x - 5 * s, y + 22 * s, x - 10 * s - armSwing, y + 38 * s);
    drawLine(x + 5 * s, y + 22 * s, x + 10 * s + armSwing, y + 38 * s);

    /* Legs */
    glColor3f(0.30f, 0.20f, 0.12f);
    drawLine(x, y + 10 * s, x - 5 * s, y - 5 * s);
    drawLine(x, y + 10 * s, x + 5 * s, y - 5 * s);
    glLineWidth(1.0f);
}

/* ============================================================
 *  Lit window (scene 4 buildings)
 *  ============================================================ */

static void drawLitWindow(float x, float y, float w, float h, int idx)
{
    float fl = 0.55f + 0.35f * sinf(gTime * 1.5f + s4_phase[idx % 12]);
    glColor4f(0.85f * fl, 0.55f * fl, 0.10f * fl, 0.9f);
    drawRect(x, y, w, h);
    glColor3f(0.15f, 0.12f, 0.08f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);       glVertex2f(x + w, y);
    glVertex2f(x + w, y + h); glVertex2f(x, y + h);
    glEnd();
    drawLine(x + w * 0.5f, y, x + w * 0.5f, y + h);
    drawLine(x, y + h * 0.5f, x + w, y + h * 0.5f);
}

/* ============================================================
 *  Building fire (Scene 4 — large flames on buildings)
 *  ============================================================ */

static void drawBuildingFire(float x, float y, float scale, int idx)
{
    float ph = s4_phase[idx % 12];

    /* Large glow halos */
    glColor4f(1.0f, 0.35f, 0.0f, 0.10f);
    drawCircleFill(x, y + 20 * scale, 80 * scale);
    glColor4f(1.0f, 0.50f, 0.10f, 0.15f);
    drawCircleFill(x, y + 15 * scale, 50 * scale);

    /* Multiple flame tongues */
    int numFlames = 5;
    for (int i = 0; i < numFlames; i++) {
        float fx = x + (i - numFlames / 2.0f) * 15 * scale;
        float fh = (40 + 20 * sinf(gTime * 5 + ph + i * 0.8f)) * scale;
        float fw = (12 + 4 * sinf(gTime * 7 + ph * 1.3f + i)) * scale;
        float sway = 5 * sinf(gTime * 6 + ph + i) * scale;

        /* Outer flame */
        glColor3f(0.85f, 0.20f, 0.0f);
        drawTri(fx - fw, y, fx + fw, y,
                fx + sway, y + fh);
        /* Middle flame */
        glColor3f(1.0f, 0.50f, 0.0f);
        drawTri(fx - fw * 0.7f, y, fx + fw * 0.7f, y,
                fx + sway * 0.7f, y + fh * 0.75f);
        /* Inner flame */
        glColor3f(1.0f, 0.80f, 0.20f);
        drawTri(fx - fw * 0.4f, y, fx + fw * 0.4f, y,
                fx + sway * 0.4f, y + fh * 0.5f);
        /* Core */
        glColor3f(1.0f, 0.95f, 0.50f);
        drawTri(fx - fw * 0.2f, y, fx + fw * 0.2f, y,
                fx + sway * 0.2f, y + fh * 0.3f);
    }

    /* Embers/sparks */
    glPointSize(2.5f);
    glBegin(GL_POINTS);
    for (int i = 0; i < 10; i++) {
        float st = fmodf(gTime * 0.7f + ph + i * 0.1f, 1.0f);
        float sx = x + 40 * scale * sinf(gTime * 2.5f + i * 1.7f + ph);
        float sy = y + 80 * scale * st;
        glColor4f(1.0f, 0.60f + 0.4f * st, 0.10f * st, (1 - st) * 0.8f);
        glVertex2f(sx, sy);
    }
    glEnd();
    glPointSize(1.0f);
}

/* ============================================================
 *  Smoke column (Scene 4)
 *  ============================================================ */

static void drawSmokeColumn(float x, float y, float scale, int idx)
{
    float ph = s4_phase[idx % 12];
    for (int i = 0; i < 15; i++) {
        float st = fmodf(gTime * 0.25f + ph + i * 0.08f, 1.0f);
        float sx = x + 30 * scale * sinf(gTime * 1.2f + i * 1.5f + ph) * st;
        float sy = y + 200 * scale * st;
        float sr = (10 + 25 * st) * scale;
        float alpha = (1 - st) * 0.25f;
        glColor4f(0.10f, 0.08f, 0.06f, alpha);
        drawCircleFill(sx, sy, sr);
    }
}

/* ============================================================
 *  City skyline (scene 4 — drawn behind the wall)
 *  ============================================================ */

static void drawCitySkyline()
{
    /* ---- central castle with twin towers ---- */
    glColor3f(0.20f, 0.17f, 0.12f);
    drawRect(480, 300, 240, 460);              /* main keep   */
    drawRect(450, 280, 50,  500);              /* left tower  */
    drawRect(700, 280, 50,  500);              /* right tower */

    /* tower peaks */
    glColor3f(0.25f, 0.14f, 0.07f);
    drawTri(448, 780, 502, 780, 475, 840);
    drawTri(698, 780, 752, 780, 725, 840);

    /* castle top battlements */
    glColor3f(0.18f, 0.15f, 0.10f);
    for (float x = 480; x < 720; x += 20)
        drawRect(x, 760, 10, 14);

    /* castle windows */
    drawLitWindow(510, 620, 25, 35, 0);
    drawLitWindow(560, 680, 22, 30, 1);
    drawLitWindow(630, 620, 25, 35, 2);
    drawLitWindow(680, 680, 22, 30, 3);
    drawLitWindow(590, 560, 25, 35, 4);
    drawLitWindow(640, 560, 25, 35, 5);

    /* ---- left-side buildings ---- */

    /* tall house 1 */
    glColor3f(0.22f, 0.19f, 0.13f);
    drawRect(40, 350, 110, 330);
    glColor3f(0.27f, 0.15f, 0.08f);
    drawTri(32, 680, 158, 680, 95, 750);
    drawLitWindow(65,  600, 22, 28, 6);
    drawLitWindow(108, 600, 22, 28, 7);
    drawLitWindow(65,  645, 22, 28, 8);
    drawLitWindow(108, 645, 22, 28, 9);

    /* house 2 */
    glColor3f(0.21f, 0.18f, 0.12f);
    drawRect(180, 370, 90, 300);
    glColor3f(0.26f, 0.14f, 0.07f);
    drawTri(172, 670, 278, 670, 225, 740);
    drawLitWindow(200, 600, 20, 25, 10);
    drawLitWindow(240, 600, 20, 25, 11);

    /* watch tower */
    glColor3f(0.23f, 0.20f, 0.14f);
    drawRect(300, 340, 50, 350);
    for (float x = 300; x < 350; x += 14)
        drawRect(x, 690, 7, 11);
    drawLitWindow(315, 610, 18, 25, 0);

    /* house 3 */
    glColor3f(0.20f, 0.17f, 0.11f);
    drawRect(370, 380, 80, 280);
    glColor3f(0.25f, 0.13f, 0.06f);
    drawTri(362, 660, 458, 660, 410, 730);
    drawLitWindow(390, 600, 18, 22, 1);
    drawLitWindow(420, 600, 18, 22, 2);

    /* ---- right-side buildings ---- */

    /* house 4 */
    glColor3f(0.22f, 0.19f, 0.13f);
    drawRect(770, 360, 100, 310);
    glColor3f(0.27f, 0.15f, 0.08f);
    drawTri(762, 670, 878, 670, 820, 740);
    drawLitWindow(790, 600, 22, 28, 3);
    drawLitWindow(835, 600, 22, 28, 4);
    drawLitWindow(790, 645, 22, 28, 5);

    /* temple with columns */
    glColor3f(0.24f, 0.21f, 0.15f);
    drawRect(900, 340, 130, 340);
    glColor3f(0.28f, 0.25f, 0.18f);
    for (float x = 908; x < 1030; x += 22)
        drawRect(x, 680, 9, 45);
    glColor3f(0.26f, 0.14f, 0.07f);
    drawTri(895, 725, 1035, 725, 965, 790);
    drawLitWindow(925, 570, 25, 32, 6);
    drawLitWindow(980, 570, 25, 32, 7);
    drawLitWindow(925, 630, 25, 32, 8);
    drawLitWindow(980, 630, 25, 32, 9);

    /* house 5 */
    glColor3f(0.21f, 0.18f, 0.12f);
    drawRect(1060, 370, 85, 300);
    glColor3f(0.26f, 0.14f, 0.07f);
    drawTri(1052, 670, 1153, 670, 1102, 735);
    drawLitWindow(1080, 600, 20, 25, 10);
    drawLitWindow(1120, 600, 20, 25, 11);

    /* right watch tower */
    glColor3f(0.23f, 0.20f, 0.14f);
    drawRect(1140, 350, 55, 340);
    for (float x = 1140; x < 1195; x += 14)
        drawRect(x, 690, 7, 11);
    drawLitWindow(1158, 610, 18, 25, 0);
}

/* ============================================================
 *  Environment helpers
 *  ============================================================ */

static void drawSky(bool night)
{
    glBegin(GL_QUADS);
    if (night) {
        glColor3f(0.01f, 0.01f, 0.06f);
        glVertex2f(0, WIN_H); glVertex2f(WIN_W, WIN_H);
        glColor3f(0.03f, 0.03f, 0.15f);
        glVertex2f(WIN_W, 280); glVertex2f(0, 280);
    } else {
        glColor3f(0.45f, 0.65f, 0.92f);
        glVertex2f(0, WIN_H); glVertex2f(WIN_W, WIN_H);
        glColor3f(0.75f, 0.82f, 0.55f);
        glVertex2f(WIN_W, 280); glVertex2f(0, 280);
    }
    glEnd();
}

static void drawBurningSky()
{
    glBegin(GL_QUADS);
    glColor3f(0.12f, 0.04f, 0.02f);
    glVertex2f(0, WIN_H); glVertex2f(WIN_W, WIN_H);
    glColor3f(0.45f, 0.12f, 0.02f);
    glVertex2f(WIN_W, 400); glVertex2f(0, 400);
    glColor3f(0.55f, 0.18f, 0.03f);
    glVertex2f(WIN_W, 280); glVertex2f(0, 280);
    glEnd();
}

static void drawGround()
{
    glBegin(GL_QUADS);
    glColor3f(0.72f, 0.62f, 0.42f);
    glVertex2f(0, 0); glVertex2f(WIN_W, 0);
    glColor3f(0.78f, 0.70f, 0.52f);
    glVertex2f(WIN_W, 280); glVertex2f(0, 280);
    glEnd();
    glColor3f(0.65f, 0.56f, 0.38f);
    for (int i = 0; i < 15; i++) {
        float gx = (float)((i * 73 + 17) % WIN_W);
        float gy = (float)((i * 31 + 5)  % 180) + 20;
        drawLine(gx, gy, gx + 25, gy - 3);
    }
}

static void drawStars()
{
    glPointSize(2.0f);
    glBegin(GL_POINTS);
    for (int i = 0; i < 100; i++) {
        float sx = (float)((i * 137 + 43) % WIN_W);
        float sy = (float)((i * 89  + 211) % 420) + 300;
        float tw = 0.3f + 0.7f * fabsf(sinf(gTime * 2.0f + i * 0.7f));
        glColor4f(1.0f, 1.0f, 0.85f, tw);
        glVertex2f(sx, sy);
    }
    glEnd();
}

static void drawSun()
{
    glColor4f(1.0f, 0.90f, 0.30f, 0.10f);
    drawCircleFill(WIN_W - 140, WIN_H - 100, 85);
    glColor4f(1.0f, 0.92f, 0.40f, 0.20f);
    drawCircleFill(WIN_W - 140, WIN_H - 100, 65);
    glColor3f(1.0f, 0.90f, 0.20f);
    drawCircleFill(WIN_W - 140, WIN_H - 100, 45);
}

static void drawMoon()
{
    glColor3f(0.92f, 0.92f, 0.95f);
    drawCircleFill(150, WIN_H - 100, 42);
    glColor3f(0.02f, 0.02f, 0.08f);
    drawCircleFill(168, WIN_H - 85, 37);
}

static void drawClouds(bool night)
{
    float a = night ? 0.06f : 0.30f;
    if (night) glColor4f(0.3f, 0.3f, 0.4f, a);
    else       glColor4f(1.0f, 1.0f, 1.0f, a);

    float c1 = fmodf(gTime * 15.0f, WIN_W + 300.0f) - 150.0f;
    drawCircleFill(c1,      WIN_H - 180, 50);
    drawCircleFill(c1 + 40, WIN_H - 170, 40);
    drawCircleFill(c1 - 30, WIN_H - 175, 35);
    drawCircleFill(c1 + 15, WIN_H - 158, 45);

    float c2 = fmodf(gTime * 10.0f + 500.0f, WIN_W + 300.0f) - 150.0f;
    drawCircleFill(c2,      WIN_H - 230, 45);
    drawCircleFill(c2 + 35, WIN_H - 220, 38);
    drawCircleFill(c2 - 25, WIN_H - 225, 30);
}

static void drawBirds()
{
    glColor3f(0.15f, 0.15f, 0.15f);
    glLineWidth(1.5f);
    for (int i = 0; i < 5; i++) {
        float bx = fmodf(gTime * 30.0f + i * 220.0f,
                         WIN_W + 100.0f) - 50.0f;
                         float by = WIN_H - 140.0f - i * 28.0f
                         + 10.0f * sinf(gTime * 3.0f + i);
                         float w  = 8.0f + 5.0f * sinf(gTime * 8.0f + i * 2.0f);
                         glBegin(GL_LINE_STRIP);
                         glVertex2f(bx - 15, by - w);
                         glVertex2f(bx - 5,  by);
                         glVertex2f(bx + 5,  by);
                         glVertex2f(bx + 15, by - w);
                         glEnd();
    }
    glLineWidth(1.0f);
}

/* ============================================================
 *  Troy Walls
 *  ============================================================ */

struct WallCfg {
    float gateCX, gateW, gateOpen;
    float wallBot, wallTop;
    bool  night;
};

static void drawTroyWalls(WallCfg w)
{
    float gx  = w.gateCX;
    float gw  = w.gateW;
    float bot = w.wallBot;
    float top = w.wallTop;

    float br = w.night ? 0.28f : 0.60f;
    float bg = w.night ? 0.25f : 0.53f;
    float bb = w.night ? 0.18f : 0.40f;
    float dr = w.night ? 0.20f : 0.50f;
    float dg = w.night ? 0.18f : 0.43f;
    float db = w.night ? 0.12f : 0.32f;

    /* main wall panels */
    glColor3f(br, bg, bb);
    drawRect(0, bot, gx - gw / 2 - 20, top - bot);
    drawRect(gx + gw / 2 + 20, bot,
             WIN_W - gx - gw / 2 - 20, top - bot);

    /* mortar lines */
    glColor3f(dr, dg, db);
    for (float y = bot + 35; y < top; y += 42) {
        drawLine(0, y, gx - gw / 2 - 20, y);
        drawLine(gx + gw / 2 + 20, y, WIN_W, y);
    }
    for (int row = 0; row < 10; row++) {
        float y0  = bot + row * 42.0f;
        float off = (row % 2) * 50.0f;
        for (float x = 30 + off; x < gx - gw / 2 - 20; x += 100)
            drawLine(x, y0, x, y0 + 42);
        for (float x = gx + gw / 2 + 20 + 30 + off;
             x < WIN_W; x += 100)
             drawLine(x, y0, x, y0 + 42);
    }

    /* battlements */
    glColor3f(br * 0.92f, bg * 0.92f, bb * 0.92f);
    for (float x = 0; x < gx - gw / 2 - 20; x += 55)
        drawRect(x, top, 28, 26);
    for (float x = gx + gw / 2 + 20; x < WIN_W; x += 55)
        drawRect(x, top, 28, 26);

    /* gate arch */
    float gateL   = gx - gw / 2.0f;
    float gateTop = bot + gw * 1.05f;

    glColor3f(br * 0.65f, bg * 0.65f, bb * 0.65f);
    glLineWidth(8.0f);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= 20; i++) {
        float a = PI * i / 20.0f;
        glVertex2f(gx + (gw / 2 + 12) * cosf(a),
                   gateTop + 5 + 45 * sinf(a));
    }
    glEnd();
    glLineWidth(1.0f);

    /* gate doors */
    float doorW   = gw / 2.0f;
    float doorH   = gateTop - bot + 10.0f;
    float openOff = doorW * w.gateOpen;

    glColor3f(0.30f, 0.18f, 0.08f);
    drawRect(gateL - openOff, bot, doorW, doorH);
    drawRect(gx   + openOff, bot, doorW, doorH);

    /* planks */
    glColor3f(0.22f, 0.12f, 0.05f);
    for (float px = doorW / 4; px < doorW; px += doorW / 4) {
        drawLine(gateL - openOff + px, bot,
                 gateL - openOff + px, bot + doorH);
        drawLine(gx + openOff + px, bot,
                 gx + openOff + px, bot + doorH);
    }
    for (float py = bot + 40; py < bot + doorH; py += 45) {
        drawLine(gateL - openOff, py,
                 gateL - openOff + doorW, py);
        drawLine(gx + openOff, py,
                 gx + openOff + doorW, py);
    }

    /* metal rivets */
    glColor3f(0.55f, 0.45f, 0.15f);
    for (float py = bot + 40; py < bot + doorH; py += 45) {
        drawCircleFill(gateL - openOff + doorW / 2, py, 4, 8);
        drawCircleFill(gx   + openOff + doorW / 2,  py, 4, 8);
    }
}

/* ============================================================
 *  Wooden Horse
 *  ============================================================ */

static void drawHorse(float cx, float baseY, float wheelR,
                      float wAng,
                      bool  showTrap = false,
                      float trapAng  = 0.0f)
{
    float bodyW = 220, bodyH = 100;
    float axleY = baseY + wheelR;
    float bodyY = axleY + 30;
    float bodyL = cx - bodyW / 2;

    /* axle */
    glColor3f(0.40f, 0.25f, 0.10f);
    drawRect(bodyL - 18, axleY - 4, bodyW + 36, 8);

    /* wheels */
    drawWheel(bodyL + 14,         axleY, wheelR, wAng);
    drawWheel(bodyL + bodyW - 14, axleY, wheelR, wAng);

    /* legs */
    glColor3f(0.55f, 0.35f, 0.15f);
    drawRect(bodyL + 20,         axleY + 4, 14, bodyY - axleY - 4);
    drawRect(bodyL + 50,         axleY + 4, 14, bodyY - axleY - 4);
    drawRect(bodyL + bodyW - 64, axleY + 4, 14, bodyY - axleY - 4);
    drawRect(bodyL + bodyW - 34, axleY + 4, 14, bodyY - axleY - 4);

    /* body */
    glColor3f(0.65f, 0.42f, 0.20f);
    drawRect(bodyL, bodyY, bodyW, bodyH);

    /* wood grain */
    glColor3f(0.55f, 0.35f, 0.15f);
    for (float y = bodyY + 18; y < bodyY + bodyH; y += 22)
        drawLine(bodyL, y, bodyL + bodyW, y);

    /* outline */
    glColor3f(0.45f, 0.28f, 0.10f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(bodyL,         bodyY);
    glVertex2f(bodyL + bodyW, bodyY);
    glVertex2f(bodyL + bodyW, bodyY + bodyH);
    glVertex2f(bodyL,         bodyY + bodyH);
    glEnd();
    glLineWidth(1.0f);

    /* ---- trap-door (scene 3) ---- */
    if (showTrap) {
        float tdW = 60, tdL = cx - tdW / 2;
        glPushMatrix();
        glTranslatef(tdL, bodyY, 0);
        glRotatef(-trapAng, 0, 0, 1);
        glColor3f(0.50f, 0.32f, 0.12f);
        drawRect(0, 0, tdW / 2, 5);
        glPopMatrix();
        glPushMatrix();
        glTranslatef(tdL + tdW, bodyY, 0);
        glRotatef(trapAng, 0, 0, 1);
        glColor3f(0.50f, 0.32f, 0.12f);
        drawRect(-tdW / 2, 0, tdW / 2, 5);
        glPopMatrix();
    }

    /* ---- neck ---- */
    float nBX = bodyL - 10, nBY = bodyY + bodyH, nTY = nBY + 100;
    glColor3f(0.60f, 0.38f, 0.18f);
    glBegin(GL_QUADS);
    glVertex2f(nBX + 35, nBY); glVertex2f(nBX + 65, nBY);
    glVertex2f(nBX + 42, nTY); glVertex2f(nBX + 12, nTY);
    glEnd();
    glColor3f(0.50f, 0.30f, 0.12f);
    for (float t = 0.15f; t < 1.0f; t += 0.20f) {
        float ly = nBY + (nTY - nBY) * t;
        drawLine(nBX + 35 + (12 - 35) * t, ly,
                 nBX + 65 + (42 - 65) * t, ly);
    }

    /* ---- head ---- */
    float hx = nBX - 5, hy = nTY;
    glColor3f(0.62f, 0.40f, 0.19f);
    glBegin(GL_QUADS);
    glVertex2f(hx,      hy - 5);  glVertex2f(hx + 50, hy + 10);
    glVertex2f(hx + 50, hy + 30); glVertex2f(hx - 5,  hy + 22);
    glEnd();
    glColor3f(0.55f, 0.35f, 0.15f);
    drawTri(hx - 5, hy + 3, hx - 38, hy + 13, hx - 5, hy + 22);
    glColor3f(0.30f, 0.18f, 0.08f);
    drawCircleFill(hx - 22, hy + 13, 3, 8);
    glColor3f(0.12f, 0.08f, 0.03f);
    drawCircleFill(hx + 14, hy + 20, 5, 12);
    glColor3f(0.85f, 0.85f, 0.75f);
    drawCircleFill(hx + 13, hy + 22, 2, 8);
    glColor3f(0.58f, 0.36f, 0.16f);
    drawTri(hx + 16, hy + 30, hx + 26, hy + 30, hx + 21, hy + 48);
    drawTri(hx + 30, hy + 30, hx + 40, hy + 30, hx + 35, hy + 46);

    /* ---- tail ---- */
    glColor3f(0.30f, 0.18f, 0.06f);
    glLineWidth(3.0f);
    {
        float tx0 = bodyL + bodyW, ty0 = bodyY + bodyH - 12;
        glBegin(GL_LINE_STRIP);
        for (int i = 0; i <= 12; i++) {
            float t = i / 12.0f;
            glVertex2f(tx0 + 35 * t + 12 * sinf(t * 4 + gTime * 2.5f),
                       ty0 - 50 * t);
        }
        glEnd();
    }

    /* ---- mane ---- */
    glColor3f(0.25f, 0.15f, 0.04f);
    glLineWidth(2.5f);
    for (int i = 0; i < 6; i++) {
        float t  = i / 6.0f;
        float mx = nBX + 50 + (nBX + 15 - nBX - 50) * t;
        float my = nBY + (nTY - nBY) * t + 5;
        float sw = 8.0f * sinf(gTime * 2.0f + i * 0.8f);
        drawLine(mx, my + 8, mx + 12 + sw, my + 2);
    }
    glLineWidth(1.0f);
}

/* ============================================================
 *  Torch with animated fire
 *  ============================================================ */

static void drawTorch(float x, float y, float h, int idx)
{
    glColor3f(0.40f, 0.30f, 0.20f);
    drawRect(x - 4, y, 8, h);
    glColor3f(0.35f, 0.25f, 0.15f);
    drawRect(x - 10, y + h - 5, 20, 10);

    float fy = y + h + 5;
    float ph = s4_phase[idx % 12];

    /* glow halos */
    glColor4f(1.0f, 0.50f, 0.0f, 0.07f);
    drawCircleFill(x, fy + 15, 50);
    glColor4f(1.0f, 0.60f, 0.10f, 0.10f);
    drawCircleFill(x, fy + 10, 30);

    /* outer flame */
    float h1 = 34 + 12 * sinf(gTime * 8 + ph);
    float w1 = 15 +  5 * sinf(gTime * 6 + ph * 1.3f);
    glColor3f(0.90f, 0.30f, 0.0f);
    drawTri(x - w1, fy, x + w1, fy,
            x + 3 * sinf(gTime * 7 + ph), fy + h1);

    /* middle flame */
    float h2 = 25 + 8 * sinf(gTime * 9 + ph * 0.7f);
    float w2 = 10 + 3 * sinf(gTime * 7 + ph * 1.1f);
    glColor3f(1.0f, 0.55f, 0.0f);
    drawTri(x - w2, fy, x + w2, fy,
            x + 2 * sinf(gTime * 8 + ph), fy + h2);

    /* inner flame */
    float h3 = 16 + 5 * sinf(gTime * 10 + ph * 0.5f);
    float w3 =  5 + 2 * sinf(gTime * 8.5f + ph * 0.9f);
    glColor3f(1.0f, 0.85f, 0.20f);
    drawTri(x - w3, fy, x + w3, fy,
            x + sinf(gTime * 9 + ph), fy + h3);

    /* sparks */
    glPointSize(2.0f);
    glBegin(GL_POINTS);
    for (int i = 0; i < 4; i++) {
        float st = fmodf(gTime * 1.5f + ph + i * 0.25f, 1.0f);
        float sx = x + 12 * sinf(gTime * 5 + i * 2.1f + ph);
        float sy = fy + h1 * st + 30 * st;
        glColor4f(1.0f, 0.80f, 0.20f, (1 - st) * 0.6f);
        glVertex2f(sx, sy);
    }
    glEnd();
}

/* ============================================================
 *  Scene 1 — Outside the Walls
 *  ============================================================ */

static void drawScene1()
{
    drawSky(false); drawSun(); drawClouds(false); drawBirds(); drawGround();

    WallCfg w = { GATE_CX, GATE_W, 0, 200, 530, false };
    drawTroyWalls(w);

    /* horse on the RIGHT side of the door — same spot scene 2 starts from */
    drawHorse(HORSE_START_X, 200, 38, 0);

    glColor3f(0.92f, 0.88f, 0.72f);
    drawText(50, WIN_H - 48,
             "Scene I:  The Gift at the Gates",
             GLUT_BITMAP_TIMES_ROMAN_24);
    glColor3f(0.65f, 0.60f, 0.45f);
    drawText(50, WIN_H - 76,
             "The great wooden horse stands outside the walls of Troy");
}

/* ============================================================
 *  Scene 2 — The Horse Enters (two-phase animation)
 *  ============================================================ */

static void drawScene2()
{
    drawSky(false); drawSun(); drawClouds(false); drawGround();

    float gateL   = GATE_CX - GATE_W / 2.0f;
    float gateR   = GATE_CX + GATE_W / 2.0f;
    float wallBot = 200.0f;
    float gateTopY = wallBot + GATE_W * 1.05f + 10.0f;

    /* 1. Interior environment inside the gate (blends with daytime) */
    // Deep background wall behind the street
    glColor3f(0.55f, 0.48f, 0.35f);
    drawRect(gateL + 10, wallBot + 90, GATE_W - 20, gateTopY - (wallBot + 90));

    // Interior buildings at the back
    glColor3f(0.72f, 0.63f, 0.43f);
    drawRect(gateL + 30, wallBot + 110, 50, 150);
    drawRect(gateR - 80, wallBot + 110, 50, 150);
    glColor3f(0.45f, 0.25f, 0.12f);
    drawTri(gateL + 30, wallBot + 260, gateL + 80, wallBot + 260, gateL + 55, wallBot + 290);
    drawTri(gateR - 80, wallBot + 260, gateR - 30, wallBot + 260, gateR - 55, wallBot + 290);

    // Windows
    glColor3f(0.15f, 0.12f, 0.08f);
    drawRect(gateL + 40, wallBot + 150, 12, 15);
    drawRect(gateL + 55, wallBot + 180, 12, 15);
    drawRect(gateR - 70, wallBot + 150, 12, 15);
    drawRect(gateR - 55, wallBot + 180, 12, 15);

    // Interior street ground (perspective)
    glBegin(GL_QUADS);
    glColor3f(0.68f, 0.58f, 0.38f);
    glVertex2f(gateL, wallBot); glVertex2f(gateR, wallBot);
    glColor3f(0.52f, 0.42f, 0.25f);
    glVertex2f(GATE_CX - 30, wallBot + 90); glVertex2f(GATE_CX + 30, wallBot + 90);
    glEnd();

    /* 2. Cheering people inside, lining up perfectly with perspective lines */
    // Left side
    drawCheeringPerson(480, 200, 0.9f);
    drawCheeringPerson(510, 235, 0.8f);
    drawCheeringPerson(540, 270, 0.7f);

    // Right side
    drawCheeringPerson(720, 200, 0.9f);
    drawCheeringPerson(690, 235, 0.8f);
    drawCheeringPerson(660, 270, 0.7f);

    /* 3. Troy Walls (will frame the gate, revealing interior through opening) */
    WallCfg w = { GATE_CX, GATE_W, s2_gateOpen, 200, 530, false };
    drawTroyWalls(w);

    /* 4. Perspective vertical lines on both sides of the door */
    glColor3f(0.40f, 0.32f, 0.18f);
    glLineWidth(3.0f);
    drawLine(gateL, wallBot, GATE_CX - 30, wallBot + 90);
    drawLine(gateR, wallBot, GATE_CX + 30, wallBot + 90);
    glLineWidth(1.0f);

    /* 5. draw horse with scale transform (for "going inside" effect) */
    glPushMatrix();
    glTranslatef(GATE_CX, 200, 0);
    glScalef(s2_scale, s2_scale, 1.0f);
    glTranslatef(-GATE_CX, -200, 0);
    drawHorse(s2_horseX, 200, 38, s2_wheelAng);
    glPopMatrix();

    /* 6. Trojans pushing — only while horse is full-size and sliding */
    if (s2_timer < 3.5f && s2_scale > 0.9f) {
        float px = s2_horseX + 135;
        drawSoldier(px,      200, 1.00f, false);
        drawSoldier(px + 28, 205, 0.90f, false);
        drawSoldier(px + 54, 198, 1.05f, false);
    }

    /* 7. dust cloud behind wheels */
    if (s2_scale > 0.9f) {
        glPointSize(3.0f);
        glBegin(GL_POINTS);
        for (int i = 0; i < 12; i++) {
            float dx = s2_horseX + 130
            + 20 * sinf(gTime * 3 + i * 1.5f);
            float dy = 205
            + 15 * fabsf(sinf(gTime * 4 + i * 2.0f));
            glColor4f(0.70f, 0.60f, 0.40f,
                      0.15f + 0.10f * sinf(gTime * 5 + i));
            glVertex2f(dx, dy);
        }
        glEnd();
    }

    glColor3f(0.92f, 0.88f, 0.72f);
    drawText(50, WIN_H - 48,
             "Scene II:  The Wheeling of the Horse",
             GLUT_BITMAP_TIMES_ROMAN_24);
    glColor3f(0.65f, 0.60f, 0.45f);
    drawText(50, WIN_H - 76,
             "The Trojans wheel the great horse through their open gates");
}

/* ============================================================
 *  Scene 3 — Inside Troy (view from the opposite side)
 *  ============================================================ */

static void drawScene3()
{
    drawSky(true); drawMoon(); drawStars(); drawClouds(true); drawGround();

    /* Darker interior atmosphere overlay */
    glColor4f(0.02f, 0.02f, 0.05f, 0.35f);
    drawRect(0, 0, WIN_W, 280);

    /* Moonlight through gate as it opens */
    float gateL = GATE_CX - GATE_W / 2.0f;
    if (s3_gateOpen > 0.0f) {
        float gateTopY = 200.0f + GATE_W * 1.05f + 10.0f;
        glColor4f(0.12f, 0.13f, 0.22f, s3_gateOpen * 0.35f);
        drawRect(gateL + 5, 200, GATE_W - 10, gateTopY - 200);
    }

    WallCfg w = { GATE_CX, GATE_W, s3_gateOpen, 200, 530, true };
    drawTroyWalls(w);

    /* Interior wall torches for lighting */
    drawTorch(120,  350, 45, 0);
    drawTorch(300,  350, 45, 1);
    drawTorch(900,  350, 45, 2);
    drawTorch(1080, 350, 45, 3);

    /* horse parked inside courtyard — shifted towards viewer (Y=150) and scaled down */
    float s3_horseY = 150.0f;
    glPushMatrix();
    glTranslatef(GATE_CX, s3_horseY, 0);
    glScalef(S3_SCALE, S3_SCALE, 1.0f);
    glTranslatef(-GATE_CX, -s3_horseY, 0);
    drawHorse(GATE_CX, s3_horseY, 38, 0, true, s3_trapAng);
    glPopMatrix();

    /* world-space Y of the trap door after scaling */
    float trapWorldY = s3_horseY + 68.0f * S3_SCALE;

    /* ropes & soldiers drawn at normal scale for visibility */
    for (int i = 0; i < 6; i++) {
        if (s3_ropeLen[i] <= 0) continue;
        float rx = GATE_CX + (i % 2) * 15.0f
        + (i / 2) * 8.0f - 12.0f;
        float ry = trapWorldY - s3_ropeLen[i];

        glColor3f(0.55f, 0.50f, 0.35f);
        glLineWidth(2.0f);
        drawLine(rx, trapWorldY, rx, ry);
        glLineWidth(1.0f);

        if (s3_ropeLen[i] > 25.0f)
            drawSoldier(rx, ry, 0.50f, true);
    }

    /* Soldiers at the base who have descended and are opening the gate */
    if (s3_timer > 6.5f) {
        drawSoldier(GATE_CX - 100, 190, 0.80f, true);
        drawSoldier(GATE_CX + 100, 190, 0.80f, true);
    }

    glColor3f(0.92f, 0.88f, 0.72f);
    drawText(50, WIN_H - 48,
             "Scene III:  Inside Troy -- The Gates Open",
             GLUT_BITMAP_TIMES_ROMAN_24);
    glColor3f(0.65f, 0.60f, 0.45f);
    drawText(50, WIN_H - 76,
             "Greek soldiers descend from the horse and open the gates from within");
}

/* ============================================================
 *  Scene 4 — Victory with city of Troy burning
 *  ============================================================ */

static void drawScene4()
{
    /* Burning sky — red/orange glow from the city fire */
    drawBurningSky();
    drawStars();
    drawClouds(true);

    /* Ground */
    drawGround();

    /* city skyline — drawn before wall so wall covers building bases */
    drawCitySkyline();

    /* Building fires on rooftops — makes it look like the city is burning */
    drawBuildingFire(95,   750, 1.0f, 0);   /* tall house 1 */
    drawBuildingFire(225,  740, 0.9f, 1);   /* house 2 */
    drawBuildingFire(325,  690, 0.7f, 2);   /* watch tower */
    drawBuildingFire(410,  730, 0.8f, 3);   /* house 3 */
    drawBuildingFire(475,  840, 1.1f, 4);   /* castle left tower */
    drawBuildingFire(725,  840, 1.1f, 5);   /* castle right tower */
    drawBuildingFire(600,  774, 1.2f, 6);   /* castle main roof */
    drawBuildingFire(820,  740, 0.9f, 7);   /* house 4 */
    drawBuildingFire(965,  790, 1.0f, 8);   /* temple */
    drawBuildingFire(1102, 735, 0.85f, 9);  /* house 5 */
    drawBuildingFire(1167, 690, 0.7f, 10);  /* right watch tower */

    /* Additional fires at window positions */
    drawBuildingFire(510, 655, 0.5f, 0);   /* castle window */
    drawBuildingFire(630, 655, 0.5f, 1);
    drawBuildingFire(65,  628, 0.4f, 2);    /* tall house window */
    drawBuildingFire(925, 602, 0.45f, 3);   /* temple window */

    /* full-width wall */
    float wBot = 200, wTop = 530;
    glColor3f(0.28f, 0.25f, 0.18f);
    drawRect(0, wBot, WIN_W, wTop - wBot);

    /* mortar lines */
    glColor3f(0.20f, 0.18f, 0.13f);
    for (float y = wBot + 35; y < wTop; y += 42)
        drawLine(0, y, WIN_W, y);
    for (int row = 0; row < 10; row++) {
        float y0  = wBot + row * 42.0f;
        float off = (row % 2) * 50.0f;
        for (float x = 30 + off; x < WIN_W; x += 100)
            drawLine(x, y0, x, y0 + 42);
    }

    /* battlements */
    glColor3f(0.26f, 0.23f, 0.17f);
    for (float x = 0; x < WIN_W; x += 55)
        drawRect(x, wTop, 28, 26);

    /* Smoke columns rising from burning buildings */
    drawSmokeColumn(95,   750, 1.0f, 0);
    drawSmokeColumn(225,  740, 0.9f, 1);
    drawSmokeColumn(475,  840, 1.1f, 2);
    drawSmokeColumn(600,  774, 1.0f, 3);
    drawSmokeColumn(725,  840, 1.1f, 4);
    drawSmokeColumn(965,  790, 1.0f, 5);
    drawSmokeColumn(1102, 735, 0.85f, 6);

    /* A few wall torches */
    for (int i = 0; i < 5; i++) {
        float tx = 150.0f + (WIN_W - 300.0f) * i / 4.0f;
        drawTorch(tx, wTop, 55, i);
        float flk = 0.04f + 0.02f * sinf(gTime * 7 + s4_phase[i]);
        glColor4f(1.0f, 0.6f, 0.1f, flk);
        drawCircleFill(tx, wTop + 10, 40);
    }

    /* warm ambient glow above wall */
    float glow = 0.035f + 0.020f * sinf(gTime * 3.0f);
    glColor4f(1.0f, 0.50f, 0.08f, glow);
    drawRect(0, wTop, WIN_W, WIN_H - wTop);

    /* Floating embers across the scene */
    glPointSize(2.0f);
    glBegin(GL_POINTS);
    for (int i = 0; i < 30; i++) {
        float ex = fmodf(gTime * 15.0f + i * 47.0f, WIN_W + 50.0f) - 25.0f;
        float ey = 280.0f + fmodf(i * 137.0f + gTime * 10.0f, WIN_H - 280.0f);
        float flicker = 0.5f + 0.5f * sinf(gTime * 5.0f + i);
        glColor4f(1.0f, 0.60f * flicker, 0.10f * flicker, 0.5f * flicker);
        glVertex2f(ex, ey);
    }
    glEnd();
    glPointSize(1.0f);

    /* title */
    glColor3f(1.0f, 0.85f, 0.30f);
    drawText(WIN_W / 2.0f - 210, WIN_H - 50,
             "Scene IV:  TROY HAS FALLEN",
             GLUT_BITMAP_TIMES_ROMAN_24);
    glColor3f(0.90f, 0.75f, 0.30f);
    drawText(WIN_W / 2.0f - 145, WIN_H - 82,
             "The flames of victory burn bright");

    /* Greek emblem */
    glColor3f(0.90f, 0.80f, 0.20f);
    drawCircleFill(WIN_W / 2, WIN_H - 155, 34);
    glColor3f(0.15f, 0.12f, 0.05f);
    drawCircleFill(WIN_W / 2, WIN_H - 155, 29);
    glColor3f(0.90f, 0.80f, 0.20f);
    drawText(WIN_W / 2.0f - 8, WIN_H - 165, "V",
             GLUT_BITMAP_TIMES_ROMAN_24);
}

/* ============================================================
 *  Animation update
 *  ============================================================ */

static void updateAnim(float dt)
{
    gTime += dt;

    /* ---- scene 2: two-phase entrance ---- */
    if (scene == 2) {
        s2_timer += dt;

        /* gates open over first 1.5 s */
        s2_gateOpen = (s2_timer < 1.5f)
        ? s2_timer / 1.5f : 1.0f;

        /* Phase 1: slide from HORSE_START_X (right) → GATE_CX  (0.5 s – 3.5 s) */
        if (s2_timer >= 0.5f && s2_timer < 3.5f) {
            float t = (s2_timer - 0.5f) / 3.0f;
            if (t > 1.0f) t = 1.0f;
            t = t * t * (3.0f - 2.0f * t);          /* smoothstep */
            s2_horseX   = HORSE_START_X
            + (GATE_CX - HORSE_START_X) * t;
            s2_wheelAng = t * 20.0f * PI;
            s2_scale    = 1.0f;
        } else if (s2_timer >= 3.5f) {
            s2_horseX = GATE_CX;
        }

        /* Phase 2: scale 1 → HORSE_END_SCALE  (3.5 s – 5.5 s) */
        if (s2_timer >= 3.5f && s2_timer < 5.5f) {
            float t = (s2_timer - 3.5f) / 2.0f;
            if (t > 1.0f) t = 1.0f;
            t = t * t * (3.0f - 2.0f * t);
            s2_scale = 1.0f + (HORSE_END_SCALE - 1.0f) * t;
        } else if (s2_timer >= 5.5f) {
            s2_scale = HORSE_END_SCALE;
        }

        /* loop after 7 s */
        if (s2_timer > 7.0f) {
            s2_timer    = 0;
            s2_horseX   = HORSE_START_X;
            s2_gateOpen = 0;
            s2_wheelAng = 0;
            s2_scale    = 1.0f;
        }
    }

    /* ---- scene 3: midnight descent & gate opening ---- */
    if (scene == 3) {
        s3_timer += dt;

        /* Phase 1: trap-door opens over 2 s */
        s3_trapAng = (s3_timer < 2.0f)
        ? (s3_timer / 2.0f) * 80.0f : 80.0f;

        /* Phase 2: soldiers lower one-by-one (2.5 s – 6.5 s) */
        if (s3_timer > 2.5f) {
            for (int i = 0; i < 6; i++) {
                float start = 2.5f + i * 1.2f;
                if (s3_timer > start) {
                    float p = (s3_timer - start) / 2.0f;
                    if (p > 1.0f) p = 1.0f;
                    s3_ropeLen[i] = p * 65.0f;
                }
            }
        }

        /* Phase 3: soldiers open the gate from inside (7 s – 10 s) */
        if (s3_timer > 7.0f) {
            float p = (s3_timer - 7.0f) / 3.0f;
            if (p > 1.0f) p = 1.0f;
            p = p * p * (3.0f - 2.0f * p);   /* smoothstep */
            s3_gateOpen = p;
        }

        /* loop after 13 s */
        if (s3_timer > 13.0f) {
            s3_timer   = 0;
            s3_trapAng = 0;
            s3_gateOpen = 0;
            for (int i = 0; i < 6; i++) s3_ropeLen[i] = 0;
        }
    }
}

/* ============================================================
 *  GLUT callbacks
 *  ============================================================ */

static void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    switch (scene) {
        case 1: drawScene1(); break;
        case 2: drawScene2(); break;
        case 3: drawScene3(); break;
        case 4: drawScene4(); break;
    }

    glColor3f(0.50f, 0.50f, 0.50f);
    char buf[120];
    sprintf(buf,
            "Scene %d/4   [N]ext  [P]rev  [1-4] Jump  [ESC] Quit",
            scene);
    drawText(10, 12, buf, GLUT_BITMAP_HELVETICA_12);

    glutSwapBuffers();
}

static void keyboard(unsigned char key, int /*x*/, int /*y*/)
{
    if (key == 27) exit(0);

    int ns = scene;
    if (key == 'n' || key == 'N') ns = scene % 4 + 1;
    if (key == 'p' || key == 'P') ns = (scene + 2) % 4 + 1;
    if (key >= '1' && key <= '4') ns = key - '0';

    if (ns != scene) {
        scene      = ns;
        s2_timer   = 0;
        s2_horseX  = HORSE_START_X;
        s2_gateOpen= 0;
        s2_wheelAng= 0;
        s2_scale   = 1.0f;
        s3_timer   = 0;
        s3_trapAng = 0;
        s3_gateOpen = 0;
        for (int i = 0; i < 6; i++) s3_ropeLen[i] = 0;
    }
    glutPostRedisplay();
}

static void timerCB(int /*value*/)
{
    int   now = glutGet(GLUT_ELAPSED_TIME);
    float dt  = (now - lastMs) / 1000.0f;
    if (dt > 0.1f) dt = 0.1f;
    lastMs = now;

    updateAnim(dt);
    glutPostRedisplay();
    glutTimerFunc(1000 / 60, timerCB, 0);
}

static void reshape(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WIN_W, 0, WIN_H, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

/* ============================================================
 *  Main
 *  ============================================================ */

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(WIN_W, WIN_H);
    glutInitWindowPosition(80, 40);
    glutCreateWindow("The Fall of Troy");

    init();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutReshapeFunc(reshape);
    glutTimerFunc(0, timerCB, 0);

    lastMs = glutGet(GLUT_ELAPSED_TIME);

    printf("========================================\n");
    printf("   The Fall of Troy  (OpenGL Scenes)\n");
    printf("========================================\n");
    printf("   N ......... Next scene\n");
    printf("   P ......... Previous scene\n");
    printf("   1-4 ....... Jump to scene\n");
    printf("   ESC ....... Quit\n");
    printf("========================================\n");

    glutMainLoop();
    return 0;
}
