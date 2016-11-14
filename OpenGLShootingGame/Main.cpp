#include <iostream>
#include <vector>
#include <chrono>
#include <gl/glut.h>

#define PI 3.14159

using namespace std;
using namespace chrono;
void setDefaultLineWidth();

struct Vector2f {
	float x;
	float y;
};

struct Vector3f {
	float x;
	float y;
	float z;
};

float distance(Vector2f a, Vector2f b) {
	float diffX = a.x - b.x;
	float diffY = a.y - b.y;
	return sqrt(diffX * diffX + diffY * diffY);
}

struct IDrawable {
	virtual void draw() = 0;
};

struct Point : public IDrawable {
	Vector2f pos;
	float size = 10;
	void draw() {
		glPointSize(size);
		glBegin(GL_POINTS);
		glVertex2f(pos.x, pos.y);
		glEnd();
	}
};

vector<Vector3f> availableColors = {
	{ 255 / 255.0f, 87 / 255.0f, 51 / 255.0f },
	{ 255 / 255.0f, 189 / 255.0f, 51 / 255.0f },
	{ 219 / 255.0f, 255 / 255.0f, 51 / 255.0f },
	{ 117 / 255.0f, 255 / 255.0f, 51 / 255.0f },
	{ 51 / 255.0f, 255 / 255.0f, 87 / 255.0f }
};

struct Tree : public IDrawable {
	Vector2f pos;
	int depth = 7;
	float length = 30;
	float startAngle;
	float splitAngle = 15;
	float splitSizeFactor = 0.9;
	int colorCounter = 0;
	float width = 10.0f;
	void draw() {
		glPushMatrix();
		glTranslatef(pos.x, pos.y, 0);
		glRotatef(startAngle, 0, 0, 1);
		colorCounter = 0;
		makeTree(length, depth, width);
		glPopMatrix();
	}
	void setNextColor() {
		Vector3f currentColor = availableColors[colorCounter];
		glColor3f(currentColor.x, currentColor.y, currentColor.z);
		++colorCounter %= availableColors.size();
	}
	void makeTree(float length, int depth, float currentWidth) {
		if (depth == 0) return;
		glPushMatrix();
		glLineWidth(currentWidth);
		setNextColor();
		glBegin(GL_LINES);
		glVertex2f(0, 0);
		glVertex2f(0, length);
		glEnd();
		glTranslatef(0, length, 0);

		glPushMatrix();
		glRotatef(splitAngle, 0, 0, 1);
		makeTree(length * splitSizeFactor, depth - 1, currentWidth * splitSizeFactor);
		glPopMatrix();

		glPushMatrix();
		glRotatef(-splitAngle, 0, 0, 1);
		makeTree(length * splitSizeFactor, depth - 1, currentWidth * splitSizeFactor);
		glPopMatrix();

		setDefaultLineWidth();
		glPopMatrix();
	}
};

time_point<system_clock> START_TIME;
time_point<system_clock> PREVIOUS_TIME;
time_point<system_clock> CURRENT_TIME;
duration<double> TIME; // time duration since the program is loaded
duration<double> TIME_DELTA;
int W = 800;
int H = 600;
float PLAYER_SPEED = 500.0f;

vector<IDrawable*> drawables;
vector<Vector2f> points;
Vector2f playerPosition = { -326, -263 };
Vector2f playerSpeed;
float playerAngle = 0;
Tree *mainTree;

double time() {
	return TIME.count(); // returns time since game loaded in seconds
}

double timeDelta() { // time between last frame and current frame
	return TIME_DELTA.count();
}

void setDefaultColor() {
	glColor3f(0, 0, 0);
}
void setDefaultLineWidth() {
	glLineWidth(1);
}

// can also draw ellipse too
void drawCircle(int glPrimitive, Vector2f radius) {
	int rounds = radius.x + radius.y; // how precise the circle is, this number is made up
	float factor = 2 * PI / rounds;

	glBegin(glPrimitive);
	for (int i = 0; i < rounds; i++) {
		float theta = i * factor;
		glVertex2f(radius.x*cosf(theta), radius.y*sinf(theta));
	}
	glEnd();
}

void drawRect(int glPrimitve, float w, float h) {
	// pivot is at the base
	glBegin(glPrimitve);
	glVertex2f(-w / 2.0f, 0);
	glVertex2f(w / 2.0f, 0);
	glVertex2f(w / 2.0f, h);
	glVertex2f(-w / 2.0f, h);
	glEnd();
}

void drawPlayer(float rad, Vector2f gunSize) {
	glPushMatrix();
	glTranslatef(playerPosition.x, playerPosition.y, 0);
	drawCircle(GL_POLYGON, { rad, rad });
	glRotatef(playerAngle - 90, 0, 0, 1);
	drawRect(GL_LINE_LOOP, gunSize.x, gunSize.y);
	glPopMatrix();
}

void display() {
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(14 / 255.0f, 167 / 255.0f, 200 / 255.0f, 1.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluOrtho2D(-W / 2, W / 2, -H / 2, H / 2);

	for (size_t i = 0; i < drawables.size(); i++)
	{
		setDefaultColor();
		setDefaultLineWidth();
		drawables[i]->draw();
	}
	setDefaultColor();
	setDefaultLineWidth();
	drawPlayer(25, { 10, 60 });
	glutSwapBuffers();
}

Vector2f screenToWorld(int x, int y) {
	float sx = x - W / 2.0f;
	float sy = (H - y) - H / 2.0f;
	return{ sx, sy };
}

void click(int btn, int st, int x, int y) {
	if (st == GLUT_DOWN) {
		Vector2f p = screenToWorld(x, y);
		cout << "Clicked at: " << p.x << " " << p.y << endl;
		Point *apoint = new Point;
		apoint->pos = { p.x, p.y };
		drawables.push_back(apoint);

		Tree *tree = new Tree;
		tree->pos = { p.x, p.y };
		tree->startAngle = playerAngle - 90;
		tree->length = distance(tree->pos, playerPosition) * 0.3;
		tree->depth = 6;
		tree->splitAngle = 30;
		drawables.push_back(tree);
	}
}

void beforeRedisplay() {
	playerPosition.x += playerSpeed.x * timeDelta();
	playerPosition.y += playerSpeed.y * timeDelta();
	mainTree->splitAngle = 30 + 10 * sin(time());
}

void update() {
	static int framesDrawn = 0;
	static int lastTime = 0;
	PREVIOUS_TIME = CURRENT_TIME;
	CURRENT_TIME = system_clock::now();
	TIME = CURRENT_TIME - START_TIME;
	TIME_DELTA = CURRENT_TIME - PREVIOUS_TIME;
	if ((int)time() > lastTime) {
		lastTime = (int)time();
		cout << "Average FPS: " << (float)framesDrawn / lastTime << endl;
	}
	beforeRedisplay();
	glutPostRedisplay();
	framesDrawn++;
}

void reshape(int w, int h) {
	cout << "Please don't reshape my fragile window!" << endl;
	glutReshapeWindow(W, H);
}

void pointTowards(int x, int y) {
	Vector2f p = screenToWorld(x, y);
	// make player gun points towards the mouse cursor
	playerAngle = atan2f(p.y - playerPosition.y, p.x - playerPosition.x) * 180 / PI;
}

void passiveMotion(int x, int y) {
	pointTowards(x, y);
}

void keyboard(unsigned char c, int x, int y) {
	if (c == 'a') {
		playerSpeed.x = -PLAYER_SPEED;
	}
	else if (c == 'd') {
		playerSpeed.x = PLAYER_SPEED;

	}
	else if (c == 'w') {
		playerSpeed.y = PLAYER_SPEED;

	}
	else if (c == 's') {
		playerSpeed.y = -PLAYER_SPEED;
	}
	pointTowards(x, y);
}

void keyboardUp(unsigned char c, int x, int y) {
	if (c == 'a') {
		playerSpeed.x += PLAYER_SPEED;
	}
	else if (c == 'd') {
		playerSpeed.x -= PLAYER_SPEED;

	}
	else if (c == 'w') {
		playerSpeed.y -= PLAYER_SPEED;

	}
	else if (c == 's') {
		playerSpeed.y += PLAYER_SPEED;
	}
	pointTowards(x, y);
}

void initialize() {
	glutDisplayFunc(display);
	glutIdleFunc(update);
	glutMouseFunc(click);
	glutReshapeFunc(reshape);
	glutPassiveMotionFunc(passiveMotion);
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboardUp);
	//Point *middle = new Point;
	//middle->pos = { 0, 0 };
	//drawables.push_back(middle);
	Tree *tree = new Tree;
	tree->pos = { -5, -120 };
	tree->startAngle = 0;
	tree->splitAngle = 30;
	tree->depth = 8;
	tree->length = 70;
	tree->splitSizeFactor = 0.8;
	drawables.push_back(tree);
	mainTree = tree;
	srand(time(NULL));
	START_TIME = system_clock::now();
	CURRENT_TIME = system_clock::now();
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