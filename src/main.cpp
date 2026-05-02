#include <GL/freeglut.h>
#include "util/direction.h"

namespace {

void on_display() {
    glClearColor(0.04f, 0.04f, 0.06f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glutSwapBuffers();
}

void on_reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, w, h, 0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void on_idle() {
    glutPostRedisplay();
}

}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(672, 824);
    glutCreateWindow("Pac-Man (CSE 426) - work in progress");

    glutDisplayFunc(on_display);
    glutReshapeFunc(on_reshape);
    glutIdleFunc(on_idle);

    glutMainLoop();
    (void)util::Direction::Up;
    return 0;
}