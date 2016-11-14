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

struct IUpdateBehavior {
	virtual void update(float time, float timeDelta) = 0;
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

struct SineWave : public IDrawable {
	Vector2f pos;
	float length = 100;
	float amplitude = 10;
	float frequency = 1;
	float shift = 0;
	Vector3f color = { 141 / 255.0f, 14 / 255.0f, 200 / 255.0f };
	void draw() {
		glPushMatrix();
		glTranslatef(pos.x, pos.y, 0);
		glTranslatef(-length / 2, 0, 0);
		glColor3f(color.x, color.y, color.z);
		glBegin(GL_LINE_STRIP);
		int rounds = round(length) * frequency * amplitude / 10; // made up number
		float factor = length / rounds;
		for (int i = 0; i <= rounds; i++) {
			float x = i * factor;
			glVertex2f(x, amplitude * sinf(frequency * (x + shift)));
		}
		glEnd();
		glPopMatrix();
	}
};

struct SineWaveBehavior : public IUpdateBehavior {
	SineWave *wave;
	float shift;
	float shiftRate = 0;
	SineWaveBehavior(SineWave *wave) : wave(wave) {
		shift = wave->shift;
	}
	void update(float time, float timeDelta) {
		wave->shift += shiftRate * timeDelta;
	}
};

vector<Vector3f> availableColors = {
	{ 255 / 255.0f, 87 / 255.0f, 51 / 255.0f },
	{ 255 / 255.0f, 189 / 255.0f, 51 / 255.0f },
	{ 219 / 255.0f, 255 / 255.0f, 51 / 255.0f },
	{ 117 / 255.0f, 255 / 255.0f, 51 / 255.0f },
	{ 51 / 255.0f, 255 / 255.0f, 87 / 255.0f }
};

Vector3f getRandomColor() {
	return availableColors[rand() % availableColors.size()];
}

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
		if (depth <= 0) return;
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

struct TreeBehavior : public IUpdateBehavior {
	Tree *tree;
	float splitAngle;
	float splitAngleDance = 0;
	float splitAngleDanceFreq = 1;
	int depth;
	int depthDance = 0;
	int depthDanceFreq = 1;
	float length;
	float lengthDance = 0;
	float lengthDanceFreq = 1;
	bool splitAngleDancing = 1;
	bool depthDancing = 1;
	bool lengthDancing = 1;
	TreeBehavior(Tree* tree) : tree(tree) {
		splitAngle = tree->splitAngle;
		depth = tree->depth;
		length = tree->length;
	}
	void update(float time, float timeDelta) {
		if (splitAngleDancing)
			tree->splitAngle = splitAngle + splitAngleDance * sin(splitAngleDanceFreq * time);
		if (depthDancing)
			tree->depth = depth + (int)roundf(depthDance * sin(depthDanceFreq * time));
		if (lengthDancing)
			tree->length = length + lengthDance * sin(lengthDanceFreq * time);
	}
	void toggleSplitAngleDance() {
		splitAngleDancing = !splitAngleDancing;
	}
	void toggleDepthDance() {
		depthDancing = !depthDancing;
	}
	void toggleLengthDance() {
		lengthDancing = !lengthDancing;
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

vector<IUpdateBehavior*> updateBehaviors;
vector<IDrawable*> drawables;
vector<Vector2f> points;
Vector2f playerPosition = { -326, -263 };
Vector2f playerSpeed;
float playerAngle = 0;
TreeBehavior *mainTree;
SineWaveBehavior *mainWave;
float sinAmplitude = 3.0;
float sinFrequency = 10 * PI;

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

float sineShiftFunc(float theta) {
	return sinAmplitude*sin(sinFrequency*theta);
}

// can also draw ellipse too
void drawCircle(int glPrimitive, Vector2f radius, float(*shiftFunc)(float theta) = NULL) {
	int rounds = radius.x + radius.y; // how precise the circle is, this number is made up
	float factor = 2 * PI / rounds;

	glBegin(glPrimitive);
	for (int i = 0; i < rounds; i++) {
		float theta = i * factor;
		float shift = shiftFunc ? shiftFunc(theta) : 0;
		float shiftX = cos(theta) * shift;
		float shiftY = sin(theta) * shift;
		glVertex2f(radius.x*cosf(theta) + shiftX, radius.y*sinf(theta) + shiftY);
	}
	glEnd();
}

struct IRotateAble {
	virtual void addAngle(float angle) = 0;
};

struct IScaleAble {
	virtual void setScale(float scale) = 0;
};

struct Circle : public IDrawable, public IRotateAble, public IScaleAble {
	Vector2f radius;
	Vector2f pos;
	float angle;
	float scale;
	float(*shiftFunc)(float theta) = NULL;
	Vector3f color;
	void draw() {
		glPushMatrix();
		glTranslatef(pos.x, pos.y, 0);
		glRotatef(angle, 0, 0, 1);
		glScalef(scale, scale, scale);
		glColor3f(color.x, color.y, color.z);
		drawCircle(GL_LINE_LOOP, radius, shiftFunc);
		glPopMatrix();
	}
	void addAngle(float angle) {
		this->angle += angle;
	}
	void setScale(float scale) {
		this->scale = scale;
	}
};

struct RotateBehavior : public IUpdateBehavior {
	IRotateAble *rotateAble;
	float rotateSpeed;
	void update(float time, float timeDelta) {
		rotateAble->addAngle(rotateSpeed * timeDelta);
	}
};

struct ScaleBehavior : public IUpdateBehavior {
	IScaleAble *scaleAble;
	float scale = 1;
	float scaleDance;
	float scaleDanceFreq = 1;
	void update(float time, float timeDelta) {
		scaleAble->setScale(scale + scaleDance * sinf(scaleDanceFreq * time));
	}
};

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

void genCircle(Vector2f p) {
	Circle *circle = new Circle;
	float rad = 30 + rand() % 30;
	circle->radius = { rad, rad };
	circle->pos = { p.x, p.y };
	circle->shiftFunc = sineShiftFunc;
	circle->angle = 0;
	circle->color = getRandomColor();
	drawables.push_back(circle);
	RotateBehavior *rotateBehavior = new RotateBehavior;
	rotateBehavior->rotateAble = circle;
	rotateBehavior->rotateSpeed = rand() % 300 - 150;
	updateBehaviors.push_back(rotateBehavior);
	ScaleBehavior *scaleBehavior = new ScaleBehavior;
	scaleBehavior->scaleAble = circle;
	scaleBehavior->scaleDance = (rand() % 20) / 19.0;
	updateBehaviors.push_back(scaleBehavior);
}

void click(int btn, int st, int x, int y) {
	if (st == GLUT_DOWN) {
		Vector2f p = screenToWorld(x, y);
		cout << "Clicked at: " << p.x << " " << p.y << endl;
		//Point *apoint = new Point;
		//apoint->pos = { p.x, p.y };
		//drawables.push_back(apoint);

		//Tree *tree = new Tree;
		//tree->pos = { p.x, p.y };
		//tree->startAngle = playerAngle - 90;
		//tree->length = distance(tree->pos, playerPosition) * 0.3;
		//tree->depth = 6;
		//tree->splitAngle = 30;
		//drawables.push_back(tree);
		genCircle(p);
	}
}

void beforeRedisplay() {
	playerPosition.x += playerSpeed.x * timeDelta();
	playerPosition.y += playerSpeed.y * timeDelta();
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
	for (size_t i = 0; i < updateBehaviors.size(); i++)
	{
		updateBehaviors[i]->update(time(), timeDelta());
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

void mainMenu(int val) {
	if (val == 0) {
		mainWave->shiftRate *= -1;
	}
	else if (val == 1) {
		mainTree->toggleSplitAngleDance();
	}
	else if (val == 2) {
		mainTree->toggleDepthDance();
	}
	else if (val == 3) {
		mainTree->toggleLengthDance();
	}
}

void initialize() {
	glutDisplayFunc(display);
	glutIdleFunc(update);
	glutMouseFunc(click);
	glutReshapeFunc(reshape);
	glutPassiveMotionFunc(passiveMotion);
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboardUp);

	glutCreateMenu(mainMenu);
	glutAddMenuEntry("Toggle Tree Split Angle Dance", 1);
	glutAddMenuEntry("Toggle Tree Depth Dance", 2);
	glutAddMenuEntry("Toggle Tree Length Dance", 3);
	glutAddMenuEntry("Switch wave direction", 0);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	//Point *middle = new Point;
	//middle->pos = { 0, 0 };
	//drawables.push_back(middle);
	Tree *tree = new Tree;
	tree->pos = { -5, -120 };
	tree->startAngle = 0;
	tree->splitAngle = 40;
	tree->depth = 8;
	tree->length = 70;
	tree->splitSizeFactor = 0.8;
	drawables.push_back(tree);
	TreeBehavior *tb = new TreeBehavior(tree);
	tb->splitAngleDance = 25;
	tb->splitAngleDanceFreq = 0.8;
	tb->depthDance = 4;
	tb->depthDanceFreq = 1.5;
	tb->lengthDance = 35;
	tb->lengthDanceFreq = 0.4;
	updateBehaviors.push_back(tb);
	mainTree = tb;

	SineWave *sineWave = new SineWave;
	sineWave->pos = { 0, -200 };
	sineWave->length = 200;
	sineWave->amplitude = 25;
	sineWave->frequency = 0.15;
	drawables.push_back(sineWave);
	SineWaveBehavior *sineBehavior = new SineWaveBehavior(sineWave);
	sineBehavior->shiftRate = 30.0;
	updateBehaviors.push_back(sineBehavior);
	mainWave = sineBehavior;

	for (int i = 0; i < 10; i++)
		genCircle({ -303, 228 });

	srand(time(NULL));
	START_TIME = system_clock::now();
	CURRENT_TIME = system_clock::now();
}

int main(int argc, char **argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(W, H);
	glutInitWindowPosition(500, 100);
	glutCreateWindow("Dancing Tree of Wisdom by Off");
	initialize();
	glutMainLoop();
	return 0;
}