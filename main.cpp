#include <GL/freeglut.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define PI 3.141592
#define PI2 PI / 2
#define PI3 3 * PI2
#define WIDTH 1280
#define HEIGHT 800
#define DPI 160

// 플레이어 포지션
float px, py, pdx, pdy, pidx, pidy, pa;

// 키 입력
bool keyWDown;
bool keyADown;
bool keySDown;
bool keyDDown;

// 마우스 로테이션
int mouseX, mouseY;

// 월드 맵
int mapX = 12, mapY = 12, mapS = 32;
int map[] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1,
    1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1,
    1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1,
    1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

void drawPlayer() {
    // 외곽선
    glColor3f(0, 0, 0);
    glPointSize(12);
    glBegin(GL_POINTS);
    glVertex2i(px, py);
    glEnd();

    // 방향 선
    glLineWidth(3);
    glBegin(GL_LINES);
    glVertex2i(px, py);
    glVertex2i(px + pdx * 20, py + pdy * 20);
    glEnd();

    // 내부 채우기
    glColor3f(1, 1, 1);
    glPointSize(8);
    glBegin(GL_POINTS);
    glVertex2i(px, py);
    glEnd();
}

void drawWorldMap() {
    int x, y, x0, y0;
    for (y = 0; y < mapY; y++) {
        for (x = 0; x < mapX; x++) {
            int block = map[y * mapY + x];
            if (block == 0) {
                glColor3f(0.2, 0.2, 0.2);
            } else if (block == 1) {
                glColor3f(1, 1, 1);
            } else {
                glColor3f(0.3, 0.3, 0.3);
            }

            // 블록 칠하기
            x0 = x * mapS;
            y0 = y * mapS;

            glBegin(GL_QUADS);
            glVertex2i(x0 + 1, y0 + 1);
            glVertex2i(x0 + 1, y0 + mapS - 1);
            glVertex2i(x0 + mapS - 1, y0 + mapS - 1);
            glVertex2i(x0 + mapS - 1, y0 + 1);
            glEnd();
        }
    }
}

void drawText(float x, float y, const char* text) {
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(x, y);
    glutBitmapString(GLUT_BITMAP_HELVETICA_18, ((unsigned char*)text));
}

void drawDebugInfo() {
    char text[100];

    float x = 10;
    float y = 420;
    float yDelta = 20;

    snprintf(text, 100, "Player X: %.2f", px);
    drawText(x, y, text);
    y += yDelta;

    snprintf(text, 100, "Player Y: %.2f", py);
    drawText(x, y, text);
    y += yDelta;

    float playerAngleDegree = pa * 180 / PI;
    snprintf(text, 100, "Player Angle: %.1f", playerAngleDegree);
    drawText(x, y, text);
    y += yDelta;

    drawText(x, y, "Player Input: ");
    if (keyWDown) {
        drawText(x + 110, y, "W");
    }
    if (keyADown) {
        drawText(x + 140, y, "A");
    }
    if (keySDown) {
        drawText(x + 170, y, "S");
    }
    if (keyDDown) {
        drawText(x + 200, y, "D");
    }
    y += yDelta;
}

void drawBlock(int x, int y) {
    int x0, y0;
    x0 = x * mapS;
    y0 = y * mapS;
    glColor3f(0.6, 0.1, 0.1);

    glBegin(GL_QUADS);
    glVertex2i(x0 + 1, y0 + 1);
    glVertex2i(x0 + 1, y0 + mapS - 1);
    glVertex2i(x0 + mapS - 1, y0 + mapS - 1);
    glVertex2i(x0 + mapS - 1, y0 + 1);
    glEnd();
}

float dist(float x1, float y1, float x2, float y2) {
    return sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2));
}

void drawRay() {
    float ra, rx, ry, xo, yo, distT;
    int count = 120, j = 0;

    float fov = PI / 4;
    float deltaAngle = fov / count;
    ra = pa - fov / 2;

    for (int i = 0; i < count; i++) {
        // 가로선
        xo = 0;
        yo = 0;
        j = 0;

        float distH = 1000000, hx = px, hy = py;

        if (ra < PI) {
            ry = (int)(py / mapS) * mapS;
            rx = px + (py - ry) / tan(ra);
            yo = -mapS;
        }

        if (ra > PI) {
            ry = ((int)(py / mapS) + 1) * mapS;
            rx = px + (py - ry) / tan(ra);
            yo = mapS;
        }

        xo = -yo / tan(ra);

        for (; j < 16; j++) {
            int mx = rx / mapS;
            int my = ry / mapS;

            if (ra < PI) {
                my--;
            }

            int index = mx + mapY * my;

            if (index >= 0 && index < mapX * mapY && map[index] == 1) {
                hx = rx;
                hy = ry;
                distH = dist(px, py, hx, hy);
                break;
            }

            rx += xo;
            ry += yo;
        }

        // 세로선

        xo = 0;
        yo = 0;
        j = 0;

        float distV = 1000000, vx = px, vy = py;

        if (ra < PI2 || ra > PI3) {
            rx = ((int)(px / mapS) + 1) * mapS;
            ry = py + (px - rx) * tan(ra);
            xo = mapS;
            yo = -xo * tan(ra);
        }

        if (ra > PI2 && ra < PI3) {
            rx = (int)(px / mapS) * mapS;
            ry = py + (px - rx) * tan(ra);
            xo = -mapS;
            yo = -xo * tan(ra);
        }

        for (; j < 16; j++) {
            int mx = rx / mapS;
            int my = ry / mapS;

            if (ra > PI2 && ra < PI3) {
                mx--;
            }

            int index = mx + mapY * my;

            if (index >= 0 && index < mapX * mapY && map[index] == 1) {
                vx = rx;
                vy = ry;
                distV = dist(px, py, vx, vy);
                break;
            }

            rx += xo;
            ry += yo;
        }

        if (distV < distH) {
            rx = vx;
            ry = vy;
            distT = distV;
            glColor3f(0.8, 0.8, 0.8);
        } else {
            rx = hx;
            ry = hy;
            distT = distH;
            glColor3f(0.7, 0.7, 0.7);
        }

        glLineWidth(2);
        glBegin(GL_LINES);
        glVertex2i(px, py);
        glVertex2i(rx, ry);
        glEnd();

        // 3D 벽 그리기

        int height3D = 640;
        int lineWidth = 6;

        float ca = pa - ra;
        if (ca < 0) {
            ca += 2 * PI;
        } else if (ca > 2 * PI) {
            ca -= 2 * PI;
        }

        distT *= cos(ca);

        float lineH = mapS * 1.5 * height3D / distT;
        if (lineH > height3D) {
            lineH = height3D;
        }
        float lineO = height3D / 2 - lineH / 2;

        glLineWidth(lineWidth);
        glBegin(GL_LINES);
        glVertex2i((count - i) * lineWidth + 480, lineO);
        glVertex2i((count - i) * lineWidth + 480, lineO + lineH);
        glEnd();

        ra += deltaAngle;

        if (ra < 0) {
            ra += 2 * PI;
        }
        if (ra > 2 * PI) {
            ra -= 2 * PI;
        }
    }
}

void movePlayer() {
    float speed = 0.8f;

    int keyCount = keyWDown + keySDown + keyADown + keyDDown;

    if (keyCount == 2) {
        speed *= sqrt(2) / 2;
    }

    if (keyWDown) {
        px += pdx * speed;
        py += pdy * speed;
    }
    if (keyADown) {
        px -= pidx * speed;
        py += pidy * speed;
    }
    if (keySDown) {
        px -= pdx * speed;
        py -= pdy * speed;
    }
    if (keyDDown) {
        px += pidx * speed;
        py -= pidy * speed;
    }
}

void display() {
    printf("");
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    movePlayer();
    drawWorldMap();
    drawRay();
    drawPlayer();
    drawDebugInfo();
    glutSwapBuffers();
    glutPostRedisplay();
}

void keyDown(unsigned char key, int x, int y) {
    if (key == 'w') {
        keyWDown = true;
    }
    if (key == 'a') {
        keyADown = true;
    }
    if (key == 's') {
        keySDown = true;
    }
    if (key == 'd') {
        keyDDown = true;
    }
}
void keyUp(unsigned char key, int x, int y) {
    if (key == 'w') {
        keyWDown = false;
    }
    if (key == 'a') {
        keyADown = false;
    }
    if (key == 's') {
        keySDown = false;
    }
    if (key == 'd') {
        keyDDown = false;
    }
}

void motion(int x, int y) {
    if (mouseX == -1) {
        mouseX = x;
    }

    if (mouseY == -1) {
        mouseY = y;
    }

    int deltaX = mouseX - x;
    int deltaY = mouseY - y;

    mouseX = x;
    mouseY = y;

    // mouseY, deltaY는 사용되지 않음. (추후 3D Z축 구현시 사용)
    pa += (float)deltaX / DPI;

    if (pa < 0) {
        pa += 2 * PI;
    } else if (pa > 2 * PI) {
        pa -= 2 * PI;
    }

    float pai = PI / 2 - pa;

    if (pai < 0) {
        pai += 2 * PI;
    } else if (pai > 2 * PI) {
        pai -= 2 * PI;
    }

    pdx = cos(pa);
    pdy = sin(pa) * -1;
    pidx = cos(pai);
    pidy = sin(pai) * -1;
}

void initScreen() {
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Dimension-3");
}

void init() {
    initScreen();
    glClearColor(0, 0, 0, 0);
    gluOrtho2D(0, WIDTH, HEIGHT, 0);
    mouseX = -1;
    mouseY = -1;
    // 플레이어 포지션을 (3, 9)로 설정
    px = mapS * (3 + 0.5f);
    py = mapS * (9 + 0.5f);
    pdx = cos(pa);
    pdy = sin(pa) * -1;
}

int main(int argc, char* argv[]) {
    glutInit(&argc, argv);
    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyDown);
    glutKeyboardUpFunc(keyUp);
    glutPassiveMotionFunc(motion);
    glutMainLoop();
}