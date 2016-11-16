/*
 * Author: Chanchana Sornsoontorn
 */
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

struct IMover {
	virtual void move(Vector2f position) = 0;
	virtual Vector2f getPosition() = 0;
};

struct Point : public IDrawable, public IMover {
	Vector2f pos;
	float size = 10;
	void draw() {
		glPointSize(size);
		glBegin(GL_POINTS);
		glVertex2f(pos.x, pos.y);
		glEnd();
	}
	void move(Vector2f position) {
		pos = position;
	}
	Vector2f getPosition() {
		return pos;
	}
};

struct PathFollowingBehavior : IUpdateBehavior {
	vector<Vector2f> positions;
	IMover *mover;
	float moveDelay = 0; // in seconds
	bool running = false;
	int idx = 0;
	int nextIndex() {
		return ++idx %= positions.size();
	}
	float elapsedTime = 0;
	void update(float time, float timeDelta) {
		if (!running) return;
		if (elapsedTime > moveDelay) {
			mover->move(positions[nextIndex()]);
			elapsedTime -= moveDelay;
		}
		elapsedTime += timeDelta;
	}
	void toggleRunningState() {
		running = !running;
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
	{ 51 / 255.0f, 255 / 255.0f, 87 / 255.0f },
	{ 141 / 255.0f, 14 / 255.0f, 200 / 255.0f }
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
	float width = 10.0f;
	float randomRange = 0.3; // should be between 0 and 1
	int state; // set to random value for different state on each tree
	Tree() {
		state = rand();
	}
	void draw() {
		glPushMatrix();
		glTranslatef(pos.x, pos.y, 0);
		glRotatef(startAngle, 0, 0, 1);
		int r = rand();
		makeTree(length, depth, width, state);
		glPopMatrix();
		srand(r);
	}
	void setNextColor(int state) {
		Vector3f currentColor = availableColors[state % availableColors.size()];
		glColor3f(currentColor.x, currentColor.y, currentColor.z);
	}
	// returns a float value between (1-ranRange) and (1+ranRange) inclusively
	float randomness(int state) {
		return (1.0 - randomRange) + randomRange * 2 * (state % 101 / 100.0f);
	}
	int printed = 0;
	void makeTree(float length, int depth, float currentWidth, int state) {
		if (depth <= 0) return;
		glPushMatrix();
		glLineWidth(currentWidth);
		setNextColor(state);
		// draw the trunk
		glBegin(GL_LINES);
		glVertex2f(0, 0);
		glVertex2f(0, length);
		glEnd();
		// then recursively draw the left and right tree
		glTranslatef(0, length, 0);

		srand(state);
		int s1 = rand(), s2 = rand();
		float r1 = randomness(s1), r2 = randomness(s2);
		float r3 = randomness(s1), r4 = randomness(s2);

		glPushMatrix();
		glRotatef(splitAngle * r3, 0, 0, 1);
		makeTree(length * splitSizeFactor * r1, depth - 1, currentWidth * splitSizeFactor * r1, s1);
		glPopMatrix();

		glPushMatrix();
		glRotatef(-splitAngle * r4, 0, 0, 1);
		makeTree(length * splitSizeFactor * r2, depth - 1, currentWidth * splitSizeFactor * r2, s2);
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
	float depthDanceFreq = 1;
	float length;
	float lengthDance = 0;
	float lengthDanceFreq = 1;
	float randomRange;
	bool splitAngleDancing = 0;
	bool depthDancing = 0;
	bool lengthDancing = 0;
	bool randomness = 0;
	TreeBehavior(Tree* tree) : tree(tree) {
		splitAngle = tree->splitAngle;
		depth = tree->depth;
		length = tree->length;
		randomRange = tree->randomRange;
	}
	void update(float time, float timeDelta) {
		if (splitAngleDancing)
			tree->splitAngle = splitAngle + splitAngleDance * sin(splitAngleDanceFreq * time);
		if (depthDancing)
			tree->depth = depth + (int)roundf(depthDance * sin(depthDanceFreq * time));
		if (lengthDancing)
			tree->length = length + lengthDance * sin(lengthDanceFreq * time);
		tree->randomRange = randomness ? randomRange : 0;
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
	void toggleRandomness() {
		randomness = !randomness;
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
vector<TreeBehavior*> mainTree;
vector<SineWaveBehavior*> mainWave;
float sinAmplitude = 3.0;
float sinFrequency = 10 * PI;
vector<PathFollowingBehavior*> following;

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

float analogSineShiftFunc(float theta) {
	return sinAmplitude*roundf(sin(2 * PI*theta));
}

// can also draw ellipse too
void drawCircle(int glPrimitive, Vector2f radius, float(*shiftFunc)(float theta) = NULL, int totalRounds = 0) {
	int rounds = totalRounds ? totalRounds : radius.x + radius.y; // how precise the circle is, this number is made up
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

struct Circle : public IDrawable, public IRotateAble, public IScaleAble, public IMover {
	Vector2f radius;
	Vector2f pos;
	float angle;
	float scale;
	float(*shiftFunc)(float theta) = NULL;
	Vector3f color;
	int rounds = 0;
	void draw() {
		glPushMatrix();
		glTranslatef(pos.x, pos.y, 0);
		glRotatef(angle, 0, 0, 1);
		glScalef(scale, scale, scale);
		glColor3f(color.x, color.y, color.z);
		drawCircle(GL_LINE_LOOP, radius, shiftFunc, rounds);
		glPopMatrix();
	}
	void addAngle(float angle) {
		this->angle += angle;
	}
	void setScale(float scale) {
		this->scale = scale;
	}
	void move(Vector2f position) {
		pos = position;
	}
	Vector2f getPosition() {
		return pos;
	}
};

struct Triangle : public IDrawable, public IRotateAble, public IScaleAble {
	Vector2f points[3];
	Vector2f pos;
	float angle;
	float scale;
	Vector3f color[3];
	Vector2f *center = 0;
	bool middle; // determine whether you want the triangle's pivot point to be in the middle
	void draw() {
		if (center == 0 && middle) {
			center = new Vector2f;
			center->x = (points[0].x + points[1].x + points[2].x) / 3.0f;
			center->y = (points[0].y + points[1].y + points[2].y) / 3.0f;
		}
		glPushMatrix();
		glTranslatef(pos.x, pos.y, 0);
		glRotatef(angle, 0, 0, 1);
		glScalef(scale, scale, scale);
		if (middle)
			glTranslatef(-center->x, -center->y, 0);
		glBegin(GL_TRIANGLES);
		for (int i = 0; i < 3; i++) {
			glColor3f(color[i].x, color[i].y, color[i].z);
			glVertex2f(points[i].x, points[i].y);
		}
		glEnd();
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

Circle* genCircle(Vector2f p) {
	Circle *circle = new Circle;
	float rad = 30 + rand() % 30;
	circle->radius = { rad, rad };
	circle->pos = { p.x, p.y };
	int ran = rand() % 2;
	if (ran)
		circle->shiftFunc = sineShiftFunc;
	else
		circle->shiftFunc = analogSineShiftFunc;
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
	return circle;
}

void genTriangle(Vector2f p) {
	Triangle *triangle = new Triangle;
	triangle->pos = { p.x, p.y };
	triangle->angle = 0;
	for (int i = 0; i < 3; i++) {
		float rx = 20 + rand() % 50;
		float ry = 20 + rand() % 50;
		if (rand() % 2) rx = -rx;
		if (rand() % 2) ry = -ry;
		triangle->points[i] = { rx, ry };
		triangle->color[i] = getRandomColor();
	}
	triangle->middle = rand() % 2;
	drawables.push_back(triangle);
	RotateBehavior *rotateBehavior = new RotateBehavior;
	rotateBehavior->rotateAble = triangle;
	rotateBehavior->rotateSpeed = rand() % 300 - 150;
	updateBehaviors.push_back(rotateBehavior);
	ScaleBehavior *scaleBehavior = new ScaleBehavior;
	scaleBehavior->scaleAble = triangle;
	scaleBehavior->scaleDance = 0.5 * (rand() % 20) / 19.0;
	updateBehaviors.push_back(scaleBehavior);
}

void click(int btn, int st, int x, int y) {
	if (st == GLUT_DOWN && btn == GLUT_LEFT_BUTTON) {
		Vector2f p = screenToWorld(x, y);
		points.push_back(p);
		//for (size_t i = 0; i < points.size(); i++)
		//{
		//	Vector2f p = points[i];
		//	cout << "{" << p.x << "," << p.y << "}, ";
		//}
		//cout << endl;
		//cout << "Clicked at: " << p.x << " " << p.y << endl;
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
		int ran = rand() % 2;
		if (ran)
			genCircle(p);
		else
			genTriangle(p);
	}
}

// make player gun points toward the position
void pointTowards(Vector2f p) {
	playerAngle = atan2f(p.y - playerPosition.y, p.x - playerPosition.x) * 180 / PI;
}

void beforeRedisplay() {
	playerPosition.x += playerSpeed.x * timeDelta();
	playerPosition.y += playerSpeed.y * timeDelta();
	pointTowards(following[0]->mover->getPosition());
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

void passiveMotion(int x, int y) {
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
}

void mainMenu(int val) {
	if (val == 0) {
		for (int i = 0; i < mainWave.size(); i++)
			mainWave[i]->shiftRate *= -1;
	}
	else if (val == 1) {
		for (int i = 0; i < mainTree.size(); i++)
			mainTree[i]->toggleSplitAngleDance();
	}
	else if (val == 2) {
		for (int i = 0; i < mainTree.size(); i++)
			mainTree[i]->toggleDepthDance();
	}
	else if (val == 3) {
		for (int i = 0; i < mainTree.size(); i++)
			mainTree[i]->toggleLengthDance();
	}
	else if (val == 4) {
		for (int i = 0; i < mainTree.size(); i++) {
			mainTree[i]->toggleRandomness();
			mainTree[i]->tree->state = rand();
		}
	}
	else if (val == 5) {
		for (int i = 0; i < following.size(); i++)
			following[i]->toggleRunningState();
	}
}

void genWave(Vector2f p) {
	SineWave *sineWave = new SineWave;
	sineWave->pos = p;
	sineWave->length = 200 + rand() % 100;
	sineWave->amplitude = 20 + rand() % 10;
	sineWave->frequency = 0.15 + 0.10 * (rand() % 31) / 30.0;
	sineWave->color = getRandomColor();
	drawables.push_back(sineWave);
	SineWaveBehavior *sineBehavior = new SineWaveBehavior(sineWave);
	sineBehavior->shiftRate = 30 + rand() % 30;
	if (rand() % 2) sineBehavior->shiftRate *= -1;
	updateBehaviors.push_back(sineBehavior);
	mainWave.push_back(sineBehavior);
}

void genTree(Vector2f p, int length = 70, int lengthDance = 35, int depth = 8, int startAngle = 0,
	float splitAngleDance = 24, float splitAngleDanceFreq = 0.8, float depthDanceFreq = 0.2,
	float lengthDanceFreq = 0.3, int depthDance = 4, float splitAngle = 40, float splitSizeFactor = 0.8) {
	Tree *tree = new Tree;
	tree->pos = p;
	tree->startAngle = startAngle;
	tree->splitAngle = splitAngle;
	tree->depth = depth;
	tree->length = length;
	tree->splitSizeFactor = splitSizeFactor;
	drawables.push_back(tree);
	TreeBehavior *tb = new TreeBehavior(tree);
	tb->splitAngleDance = splitAngleDance;
	tb->splitAngleDanceFreq = splitAngleDanceFreq;
	tb->depthDance = depthDance;
	tb->depthDanceFreq = depthDanceFreq;
	tb->lengthDance = lengthDance;
	tb->lengthDanceFreq = lengthDanceFreq;
	updateBehaviors.push_back(tb);
	mainTree.push_back(tb);
}

PathFollowingBehavior* genMovingCircle(vector<Vector2f> &positions, int rounds, int index, Vector3f color, float radius) {
	PathFollowingBehavior *following = new PathFollowingBehavior;
	following->positions = positions;
	Circle *circle = genCircle(following->positions[0]);
	circle->shiftFunc = NULL;
	circle->radius = { radius, radius };
	circle->color = color;
	circle->rounds = rounds;
	following->mover = circle;
	following->moveDelay = 0.1;
	following->idx = index;
	drawables.push_back(circle);
	updateBehaviors.push_back(following);
	::following.push_back(following);
	return following;
}
void initialize() {
	srand(time(NULL));
	glutDisplayFunc(display);
	glutIdleFunc(update);
	glutMouseFunc(click);
	glutReshapeFunc(reshape);
	glutPassiveMotionFunc(passiveMotion);
	cout << "Move the player using WASD key" << endl;
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboardUp);

	cout << "Right-click on the window to open menu" << endl;
	glutCreateMenu(mainMenu);
	glutAddMenuEntry("Toggle Tree Split Angle Dance", 1);
	glutAddMenuEntry("Toggle Tree Depth Dance", 2);
	glutAddMenuEntry("Toggle Tree Length Dance", 3);
	glutAddMenuEntry("Toggle Tree Symmetery", 4);
	glutAddMenuEntry("Toggle Mover Running State", 5);
	glutAddMenuEntry("Toggle Wave Direction", 0);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	//Point *middle = new Point;
	//middle->pos = { 0, 0 };
	//drawables.push_back(middle);
	genTree({ -5, -120 });
	genTree({ -312, -173 }, 30, 10, 3, 10, 10, 1.5, 0.75, 0.4, 2, 25, 0.9);
	genTree({ 281, -232 }, 30, 10, 7, -20, 15, 2.0, 0.9, 0.8, 3, 30, 0.8);

	genWave({ 0, -200 });
	genWave({ 0, -230 });
	genWave({ 0, -260 });

	for (int i = 0; i < 10; i++)
		genCircle({ -303, 228 });

	for (int i = 0; i < 5; i++)
		genTriangle({ 300, 200 });

	vector<Vector2f> positions = { {-330,-254}, {-332,-254}, {-340,-247}, {-344,-243}, {-347,-239}, {-350,-236}, {-354,-233}, {-359,-228}, {-364,-222}, {-367,-214}, {-369,-210}, {-370,-207}, {-372,-198}, {-374,-189}, {-374,-174}, {-375,-168}, {-377,-156}, {-378,-148}, {-378,-139}, {-379,-116}, {-379,-93}, {-376,-72}, {-369,-59}, {-356,-49}, {-344,-43}, {-329,-43}, {-314,-46}, {-305,-48}, {-288,-50}, {-275,-58}, {-263,-81}, {-249,-123}, {-243,-140}, {-240,-161}, {-237,-183}, {-247,-216}, {-261,-236}, {-270,-255}, {-266,-271}, {-234,-278}, {-220,-278}, {-203,-275}, {-183,-268}, {-173,-260}, {-170,-242}, {-175,-223}, {-182,-200}, {-209,-196}, {-242,-214}, {-262,-223}, {-301,-226}, {-320,-224}, {-344,-213}, {-370,-185}, {-373,-152}, {-373,-122}, {-373,-100}, {-370,-73}, {-367,-57}, {-364,-35}, {-355,-11}, {-344,10}, {-334,30}, {-320,44}, {-309,63}, {-300,82}, {-288,92}, {-260,104}, {-239,115}, {-227,121}, {-191,142}, {-151,174}, {-126,192}, {-102,206}, {-86,201}, {-88,167}, {-170,145}, {-187,189}, {-166,218}, {-126,239}, {-97,254}, {-70,262}, {-8,263}, {34,261}, {55,257}, {79,246}, {100,223}, {116,200}, {134,182}, {152,169}, {183,169}, {212,205}, {220,240}, {207,267}, {134,286}, {32,271}, {-4,256}, {-25,233}, {-28,217}, {34,198}, {106,183}, {174,150}, {258,107}, {323,86}, {352,45}, {363,-29}, {327,-68}, {231,6}, {304,108}, {315,7}, {233,-9}, {290,87}, {348,40}, {354,10}, {299,-28}, {243,16}, {278,76}, {347,54}, {367,-12}, {337,-42}, {246,-75}, {198,-143}, {195,-177}, {239,-213}, {282,-234}, {318,-253}, {366,-250}, {364,-229}, {331,-240}, {345,-267}, {299,-279}, {235,-274}, {207,-274}, {175,-264}, {190,-243}, {233,-257}, {221,-272}, {193,-259}, {211,-237}, {228,-228}, {217,-200}, {174,-200}, {196,-229}, {211,-192}, {194,-178}, {146,-156}, {182,-130}, {197,-143}, {185,-182}, {142,-183}, {126,-162}, {102,-158}, {107,-179}, {91,-186}, {7,-207}, {-88,-200}, {-161,-185}, {-107,-167}, {-107,-223}, {-98,-198}, {-96,-254}, {-87,-196}, {-57,-215}, {-48,-244}, {-17,-243}, {37,-222}, {110,-230}, {158,-246}, {168,-218}, {161,-176}, {114,-165}, {19,-162}, {-42,-159}, {-110,-159}, {-168,-154}, {-198,-146}, {-201,-137}, {-178,-134}, {-135,-140}, {-40,-145}, {13,-143}, {86,-146}, {142,-146}, {177,-142}, {183,-130}, {159,-124}, {129,-138}, {84,-153}, {21,-153}, {-45,-150}, {-85,-146}, {-115,-145}, {-142,-143}, {-161,-139}, {-150,-137}, {-117,-138}, {-101,-141}, {-81,-140}, {-51,-142}, {-1,-144}, {39,-144}, {72,-146}, {124,-149}, {159,-146}, {157,-137}, {115,-136}, {90,-154}, {71,-130}, {118,-143}, {95,-155}, {40,-147}, {66,-126}, {108,-133}, {85,-149}, {1,-148}, {27,-126}, {42,-153}, {-48,-148}, {2,-130}, {-31,-154}, {-84,-134}, {-68,-134}, {-119,-153}, {-112,-130}, {-138,-146}, {-123,-142}, {-155,-133}, {-158,-141}, {-113,-122}, {-72,-126}, {-21,-136}, {15,-139}, {61,-145}, {128,-141}, {154,-120}, {147,-120}, {107,-132}, {47,-139}, {-17,-141}, {-71,-142}, {-149,-128}, {-176,-145}, {-189,-164}, {-188,-199}, {-183,-243}, {-206,-268}, {-223,-221}, {-214,-183}, {-214,-154}, {-221,-139}, {-253,-73}, {-257,18}, {-203,150}, {-134,207}, {38,227}, {105,218}, {-18,239}, {-138,161}, {-206,59}, {-225,-48}, {-209,-123}, {-198,-138}, {-190,-153}, {-176,-181}, {-171,-206}, {-171,-221}, {-172,-234}, {-171,-239}, {-170,-248}, {-170,-250}, {-170,-255}, {-170,-256}, {-169,-256}, {-169,-256}, {-169,-256}, {-169,-256}, {-169,-256}, {-169,-256}, {-169,-256}, {-174,-256}, {-177,-256}, {-181,-255}, {-182,-255}, {-185,-255}, {-185,-255}, {-185,-255}, {-185,-255}, {-185,-255}, {-185,-255}, {-196,-254}, {-196,-254}, {-202,-254}, {-203,-254}, {-218,-254}, {-218,-254}, {-226,-254}, {-226,-254}, {-233,-254}, {-233,-254}, {-247,-254}, {-247,-254}, {-250,-254}, {-250,-254}, {-253,-254}, {-253,-254}, {-262,-254}, {-262,-254}, {-267,-254}, {-267,-254}, {-281,-256}, {-281,-256}, {-302,-258}, {-302,-258}, {-314,-258}, {-316,-258}, {-322,-258}, {-330,-254}, {-332,-254}, {-340,-247}, {-344,-243}, {-347,-239}, {-350,-236}, {-354,-233}, {-359,-228}, {-364,-222}, {-367,-214}, {-369,-210}, {-370,-207}, {-372,-198}, {-374,-189}, {-374,-174}, {-375,-168}, {-377,-156}, {-378,-148}, {-378,-139}, {-379,-116}, {-379,-93}, {-376,-72}, {-369,-59}, {-356,-49}, {-344,-43}, {-329,-43}, {-314,-46}, {-305,-48}, {-288,-50}, {-275,-58}, {-263,-81}, {-249,-123}, {-243,-140}, {-240,-161}, {-237,-183}, {-247,-216}, {-261,-236}, {-270,-255}, {-266,-271}, {-234,-278}, {-220,-278}, {-203,-275}, {-183,-268}, {-173,-260}, {-170,-242}, {-175,-223}, {-182,-200}, {-209,-196}, {-242,-214}, {-262,-223}, {-301,-226}, {-320,-224}, {-344,-213}, {-370,-185}, {-373,-152}, {-373,-122}, {-373,-100}, {-370,-73}, {-367,-57}, {-364,-35}, {-355,-11}, {-344,10}, {-334,30}, {-320,44}, {-309,63}, {-300,82}, {-288,92}, {-260,104}, {-239,115}, {-227,121}, {-191,142}, {-151,174}, {-126,192}, {-102,206}, {-86,201}, {-88,167}, {-170,145}, {-187,189}, {-166,218}, {-126,239}, {-97,254}, {-70,262}, {-8,263}, {34,261}, {55,257}, {79,246}, {100,223}, {116,200}, {134,182}, {152,169}, {183,169}, {212,205}, {220,240}, {207,267}, {134,286}, {32,271}, {-4,256}, {-25,233}, {-28,217}, {34,198}, {106,183}, {174,150}, {258,107}, {323,86}, {352,45}, {363,-29}, {327,-68}, {231,6}, {304,108}, {315,7}, {233,-9}, {290,87}, {348,40}, {354,10}, {299,-28}, {243,16}, {278,76}, {347,54}, {367,-12}, {337,-42}, {246,-75}, {198,-143}, {195,-177}, {239,-213}, {282,-234}, {318,-253}, {366,-250}, {364,-229}, {331,-240}, {345,-267}, {299,-279}, {235,-274}, {207,-274}, {175,-264}, {190,-243}, {233,-257}, {221,-272}, {193,-259}, {211,-237}, {228,-228}, {217,-200}, {174,-200}, {196,-229}, {211,-192}, {194,-178}, {146,-156}, {182,-130}, {197,-143}, {185,-182}, {142,-183}, {126,-162}, {102,-158}, {107,-179}, {91,-186}, {7,-207}, {-88,-200}, {-161,-185}, {-107,-167}, {-107,-223}, {-98,-198}, {-96,-254}, {-87,-196}, {-57,-215}, {-48,-244}, {-17,-243}, {37,-222}, {110,-230}, {158,-246}, {168,-218}, {161,-176}, {114,-165}, {19,-162}, {-42,-159}, {-110,-159}, {-168,-154}, {-198,-146}, {-201,-137}, {-178,-134}, {-135,-140}, {-40,-145}, {13,-143}, {86,-146}, {142,-146}, {177,-142}, {183,-130}, {159,-124}, {129,-138}, {84,-153}, {21,-153}, {-45,-150}, {-85,-146}, {-115,-145}, {-142,-143}, {-161,-139}, {-150,-137}, {-117,-138}, {-101,-141}, {-81,-140}, {-51,-142}, {-1,-144}, {39,-144}, {72,-146}, {124,-149}, {159,-146}, {157,-137}, {115,-136}, {90,-154}, {71,-130}, {118,-143}, {95,-155}, {40,-147}, {66,-126}, {108,-133}, {85,-149}, {1,-148}, {27,-126}, {42,-153}, {-48,-148}, {2,-130}, {-31,-154}, {-84,-134}, {-68,-134}, {-119,-153}, {-112,-130}, {-138,-146}, {-123,-142}, {-155,-133}, {-158,-141}, {-113,-122}, {-72,-126}, {-21,-136}, {15,-139}, {61,-145}, {128,-141}, {154,-120}, {147,-120}, {107,-132}, {47,-139}, {-17,-141}, {-71,-142}, {-149,-128}, {-176,-145}, {-189,-164}, {-188,-199}, {-183,-243}, {-206,-268}, {-223,-221}, {-214,-183}, {-214,-154}, {-221,-139}, {-253,-73}, {-257,18}, {-203,150}, {-134,207}, {38,227}, {105,218}, {-18,239}, {-138,161}, {-206,59}, {-225,-48}, {-209,-123}, {-198,-138}, {-190,-153}, {-176,-181}, {-171,-206}, {-171,-221}, {-172,-234}, {-171,-239}, {-170,-248}, {-170,-250}, {-170,-255}, {-170,-256}, {-169,-256}, {-169,-256}, {-169,-256}, {-169,-256}, {-169,-256}, {-169,-256}, {-169,-256}, {-174,-256}, {-177,-256}, {-181,-255}, {-182,-255}, {-185,-255}, {-185,-255}, {-185,-255}, {-185,-255}, {-185,-255}, {-185,-255}, {-196,-254}, {-196,-254}, {-202,-254}, {-203,-254}, {-218,-254}, {-218,-254}, {-226,-254}, {-226,-254}, {-233,-254}, {-233,-254}, {-247,-254}, {-247,-254}, {-250,-254}, {-250,-254}, {-253,-254}, {-253,-254}, {-262,-254}, {-262,-254}, {-267,-254}, {-267,-254}, {-281,-256}, {-281,-256}, {-302,-258}, {-302,-258}, {-314,-258}, {-316,-258}, {-322,-258}, {-322,-258}, {-330,-254}, {-332,-254}, {-340,-247}, {-344,-243}, {-347,-239}, {-350,-236}, {-354,-233}, {-359,-228}, {-364,-222}, {-367,-214}, {-369,-210}, {-370,-207}, {-372,-198}, {-374,-189}, {-374,-174}, {-375,-168}, {-377,-156}, {-378,-148}, {-378,-139}, {-379,-116}, {-379,-93}, {-376,-72}, {-369,-59}, {-356,-49}, {-344,-43}, {-329,-43}, {-314,-46}, {-305,-48}, {-288,-50}, {-275,-58}, {-263,-81}, {-249,-123}, {-243,-140}, {-240,-161}, {-237,-183}, {-247,-216}, {-261,-236}, {-270,-255}, {-266,-271}, {-234,-278}, {-220,-278}, {-203,-275}, {-183,-268}, {-173,-260}, {-170,-242}, {-175,-223}, {-182,-200}, {-209,-196}, {-242,-214}, {-262,-223}, {-301,-226}, {-320,-224}, {-344,-213}, {-370,-185}, {-373,-152}, {-373,-122}, {-373,-100}, {-370,-73}, {-367,-57}, {-364,-35}, {-355,-11}, {-344,10}, {-334,30}, {-320,44}, {-309,63}, {-300,82}, {-288,92}, {-260,104}, {-239,115}, {-227,121}, {-191,142}, {-151,174}, {-126,192}, {-102,206}, {-86,201}, {-88,167}, {-170,145}, {-187,189}, {-166,218}, {-126,239}, {-97,254}, {-70,262}, {-8,263}, {34,261}, {55,257}, {79,246}, {100,223}, {116,200}, {134,182}, {152,169}, {183,169}, {212,205}, {220,240}, {207,267}, {134,286}, {32,271}, {-4,256}, {-25,233}, {-28,217}, {34,198}, {106,183}, {174,150}, {258,107}, {323,86}, {352,45}, {363,-29}, {327,-68}, {231,6}, {304,108}, {315,7}, {233,-9}, {290,87}, {348,40}, {354,10}, {299,-28}, {243,16}, {278,76}, {347,54}, {367,-12}, {337,-42}, {246,-75}, {198,-143}, {195,-177}, {239,-213}, {282,-234}, {318,-253}, {366,-250}, {364,-229}, {331,-240}, {345,-267}, {299,-279}, {235,-274}, {207,-274}, {175,-264}, {190,-243}, {233,-257}, {221,-272}, {193,-259}, {211,-237}, {228,-228}, {217,-200}, {174,-200}, {196,-229}, {211,-192}, {194,-178}, {146,-156}, {182,-130}, {197,-143}, {185,-182}, {142,-183}, {126,-162}, {102,-158}, {107,-179}, {91,-186}, {7,-207}, {-88,-200}, {-161,-185}, {-107,-167}, {-107,-223}, {-98,-198}, {-96,-254}, {-87,-196}, {-57,-215}, {-48,-244}, {-17,-243}, {37,-222}, {110,-230}, {158,-246}, {168,-218}, {161,-176}, {114,-165}, {19,-162}, {-42,-159}, {-110,-159}, {-168,-154}, {-198,-146}, {-201,-137}, {-178,-134}, {-135,-140}, {-40,-145}, {13,-143}, {86,-146}, {142,-146}, {177,-142}, {183,-130}, {159,-124}, {129,-138}, {84,-153}, {21,-153}, {-45,-150}, {-85,-146}, {-115,-145}, {-142,-143}, {-161,-139}, {-150,-137}, {-117,-138}, {-101,-141}, {-81,-140}, {-51,-142}, {-1,-144}, {39,-144}, {72,-146}, {124,-149}, {159,-146}, {157,-137}, {115,-136}, {90,-154}, {71,-130}, {118,-143}, {95,-155}, {40,-147}, {66,-126}, {108,-133}, {85,-149}, {1,-148}, {27,-126}, {42,-153}, {-48,-148}, {2,-130}, {-31,-154}, {-84,-134}, {-68,-134}, {-119,-153}, {-112,-130}, {-138,-146}, {-123,-142}, {-155,-133}, {-158,-141}, {-113,-122}, {-72,-126}, {-21,-136}, {15,-139}, {61,-145}, {128,-141}, {154,-120}, {147,-120}, {107,-132}, {47,-139}, {-17,-141}, {-71,-142}, {-149,-128}, {-176,-145}, {-189,-164}, {-188,-199}, {-183,-243}, {-206,-268}, {-223,-221}, {-214,-183}, {-214,-154}, {-221,-139}, {-253,-73}, {-257,18}, {-203,150}, {-134,207}, {38,227}, {105,218}, {-18,239}, {-138,161}, {-206,59}, {-225,-48}, {-209,-123}, {-198,-138}, {-190,-153}, {-176,-181}, {-171,-206}, {-171,-221}, {-172,-234}, {-171,-239}, {-170,-248}, {-170,-250}, {-170,-255}, {-170,-256}, {-169,-256}, {-169,-256}, {-169,-256}, {-169,-256}, {-169,-256}, {-169,-256}, {-169,-256}, {-174,-256}, {-177,-256}, {-181,-255}, {-182,-255}, {-185,-255}, {-185,-255}, {-185,-255}, {-185,-255}, {-185,-255}, {-185,-255}, {-196,-254}, {-196,-254}, {-202,-254}, {-203,-254}, {-218,-254}, {-218,-254}, {-226,-254}, {-226,-254}, {-233,-254}, {-233,-254}, {-247,-254}, {-247,-254}, {-250,-254}, {-250,-254}, {-253,-254}, {-253,-254}, {-262,-254}, {-262,-254}, {-267,-254}, {-267,-254}, {-281,-256}, {-281,-256}, {-302,-258}, {-302,-258}, {-314,-258}, {-316,-258}, {-322,-258}, {-322,-258}, {-326,-258}, {-330,-254}, {-332,-254}, {-340,-247}, {-344,-243}, {-347,-239}, {-350,-236}, {-354,-233}, {-359,-228}, {-364,-222}, {-367,-214}, {-369,-210}, {-370,-207}, {-372,-198}, {-374,-189}, {-374,-174}, {-375,-168}, {-377,-156}, {-378,-148}, {-378,-139}, {-379,-116}, {-379,-93}, {-376,-72}, {-369,-59}, {-356,-49}, {-344,-43}, {-329,-43}, {-314,-46}, {-305,-48}, {-288,-50}, {-275,-58}, {-263,-81}, {-249,-123}, {-243,-140}, {-240,-161}, {-237,-183}, {-247,-216}, {-261,-236}, {-270,-255}, {-266,-271}, {-234,-278}, {-220,-278}, {-203,-275}, {-183,-268}, {-173,-260}, {-170,-242}, {-175,-223}, {-182,-200}, {-209,-196}, {-242,-214}, {-262,-223}, {-301,-226}, {-320,-224}, {-344,-213}, {-370,-185}, {-373,-152}, {-373,-122}, {-373,-100}, {-370,-73}, {-367,-57}, {-364,-35}, {-355,-11}, {-344,10}, {-334,30}, {-320,44}, {-309,63}, {-300,82}, {-288,92}, {-260,104}, {-239,115}, {-227,121}, {-191,142}, {-151,174}, {-126,192}, {-102,206}, {-86,201}, {-88,167}, {-170,145}, {-187,189}, {-166,218}, {-126,239}, {-97,254}, {-70,262}, {-8,263}, {34,261}, {55,257}, {79,246}, {100,223}, {116,200}, {134,182}, {152,169}, {183,169}, {212,205}, {220,240}, {207,267}, {134,286}, {32,271}, {-4,256}, {-25,233}, {-28,217}, {34,198}, {106,183}, {174,150}, {258,107}, {323,86}, {352,45}, {363,-29}, {327,-68}, {231,6}, {304,108}, {315,7}, {233,-9}, {290,87}, {348,40}, {354,10}, {299,-28}, {243,16}, {278,76}, {347,54}, {367,-12}, {337,-42}, {246,-75}, {198,-143}, {195,-177}, {239,-213}, {282,-234}, {318,-253}, {366,-250}, {364,-229}, {331,-240}, {345,-267}, {299,-279}, {235,-274}, {207,-274}, {175,-264}, {190,-243}, {233,-257}, {221,-272}, {193,-259}, {211,-237}, {228,-228}, {217,-200}, {174,-200}, {196,-229}, {211,-192}, {194,-178}, {146,-156}, {182,-130}, {197,-143}, {185,-182}, {142,-183}, {126,-162}, {102,-158}, {107,-179}, {91,-186}, {7,-207}, {-88,-200}, {-161,-185}, {-107,-167}, {-107,-223}, {-98,-198}, {-96,-254}, {-87,-196}, {-57,-215}, {-48,-244}, {-17,-243}, {37,-222}, {110,-230}, {158,-246}, {168,-218}, {161,-176}, {114,-165}, {19,-162}, {-42,-159}, {-110,-159}, {-168,-154}, {-198,-146}, {-201,-137}, {-178,-134}, {-135,-140}, {-40,-145}, {13,-143}, {86,-146}, {142,-146}, {177,-142}, {183,-130}, {159,-124}, {129,-138}, {84,-153}, {21,-153}, {-45,-150}, {-85,-146}, {-115,-145}, {-142,-143}, {-161,-139}, {-150,-137}, {-117,-138}, {-101,-141}, {-81,-140}, {-51,-142}, {-1,-144}, {39,-144}, {72,-146}, {124,-149}, {159,-146}, {157,-137}, {115,-136}, {90,-154}, {71,-130}, {118,-143}, {95,-155}, {40,-147}, {66,-126}, {108,-133}, {85,-149}, {1,-148}, {27,-126}, {42,-153}, {-48,-148}, {2,-130}, {-31,-154}, {-84,-134}, {-68,-134}, {-119,-153}, {-112,-130}, {-138,-146}, {-123,-142}, {-155,-133}, {-158,-141}, {-113,-122}, {-72,-126}, {-21,-136}, {15,-139}, {61,-145}, {128,-141}, {154,-120}, {147,-120}, {107,-132}, {47,-139}, {-17,-141}, {-71,-142}, {-149,-128}, {-176,-145}, {-189,-164}, {-188,-199}, {-183,-243}, {-206,-268}, {-223,-221}, {-214,-183}, {-214,-154}, {-221,-139}, {-253,-73}, {-257,18}, {-203,150}, {-134,207}, {38,227}, {105,218}, {-18,239}, {-138,161}, {-206,59}, {-225,-48}, {-209,-123}, {-198,-138}, {-190,-153}, {-176,-181}, {-171,-206}, {-171,-221}, {-172,-234}, {-171,-239}, {-170,-248}, {-170,-250}, {-170,-255}, {-170,-256}, {-169,-256}, {-169,-256}, {-169,-256}, {-169,-256}, {-169,-256}, {-169,-256}, {-169,-256}, {-174,-256}, {-177,-256}, {-181,-255}, {-182,-255}, {-185,-255}, {-185,-255}, {-185,-255}, {-185,-255}, {-185,-255}, {-185,-255}, {-196,-254}, {-196,-254}, {-202,-254}, {-203,-254}, {-218,-254}, {-218,-254}, {-226,-254}, {-226,-254}, {-233,-254}, {-233,-254}, {-247,-254}, {-247,-254}, {-250,-254}, {-250,-254}, {-253,-254}, {-253,-254}, {-262,-254}, {-262,-254}, {-267,-254}, {-267,-254}, {-281,-256}, {-281,-256}, {-302,-258}, {-302,-258}, {-314,-258}, {-316,-258}, {-322,-258}, {-322,-258}, {-326,-258}, {-326,-258} };
	cout << "Number of moving positions: " << positions.size() << endl;
	for (int i = 0; i < 10; i++)
	genMovingCircle(positions, i+3, 9 - i, { 1 - i /9.0f, 0, i/9.0f }, 5 + (10 - i) * 2);

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