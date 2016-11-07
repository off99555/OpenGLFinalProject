#include <iostream>
#include <vector>
#include <chrono>
#include <gl/glut.h>
using namespace std;
using namespace chrono;

struct Point {
	float x;
	float y;
};

time_point<system_clock> START_TIME;
time_point<system_clock> CURRENT_TIME;
duration<double> TIME; // time duration since the program is loaded
int W = 800;
int H = 600;
vector<Point> points;

double time() {
	return TIME.count(); // returns time since game loaded in seconds
}

void setDefaultColor() {
	glColor3f(0, 0, 0);
}

void display() {
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluOrtho2D(-W/2, W/2, -H/2, H/2);

	setDefaultColor();
	glPointSize(10);
	glBegin(GL_POINTS);
	for (size_t i = 0; i < points.size(); i++)
	{
		glVertex2f(points[i].x, points[i].y);
	}
	glEnd();

	glutSwapBuffers();
}

void click(int btn, int st, int x, int y) {
	if (st == GLUT_DOWN) {
		float sx = x - W / 2;
		float sy = (H - y) - H / 2;
		cout << "Clicked at: " << sx << " " << sy << endl;
		points.push_back({ sx, sy });
	}
}

void update() {
	static int framesDrawn = 0;
	static int lastTime = 0;
	CURRENT_TIME = system_clock::now();
	TIME = CURRENT_TIME - START_TIME;
	if ((int)time() > lastTime) {
		lastTime = (int)time();
		cout << "Average FPS: " << (float) framesDrawn / lastTime << endl;
	}
	glutPostRedisplay();
	framesDrawn++;
}

void reshape(int w, int h) {
	cout << "Please don't reshape my fragile window!" << endl;
	glutReshapeWindow(W, H);
}

void initialize() {
	glutDisplayFunc(display);
	glutIdleFunc(update);
	glutMouseFunc(click);
	glutReshapeFunc(reshape);
	points.push_back({ 0, 0 });
	START_TIME = system_clock::now();
}

int main(int argc, char **argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(W, H);
	glutInitWindowPosition(500, 100);
	glutCreateWindow("Shooting Game by Off");
	initialize();
	glutMainLoop();
	return 0;
}