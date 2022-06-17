#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <conio.h>
#include "GL\glew.h"
#include "GL\freeglut.h"

#include "shaderLoader.h" //narzedzie do ladowania i kompilowania shaderow z pliku

//funkcje algebry liniowej
#include "glm/vec3.hpp" // glm::vec3
#include "glm/vec4.hpp" // glm::vec4
#include "glm/mat4x4.hpp" // glm::mat4
#include "glm/gtc/matrix_transform.hpp" // glm::translate, glm::rotate, glm::scale, glm::perspective

//Labyrinth things
int readLabyrinth(const char* fileName, float** vertV, float** vertH);
int amIInTheWall(float preX, float preZ);
void pointToCube(float* verts, float pointX, float pointZ, float heightZero, float heightCube, float diffX, float diffZ);
void wallElems(GLuint* elems, int offset=0);
const float wallWidth = 0.1f;
const float wallHeight = 1.0f;
const float labyrinthFloorEdge = 3.0f;
const float floorHeight = 2.0f;
const float characterHeight = 0.5f;
const float visibilityClip = 0.1f;
int xLabDim = 0;
int yLabDim = 0;
int verticalWallCounter = 0;
int horizontalWallCounter = 0;
float verticesVertical[100 * 100 * 4*2 * 3];
float verticesHorizontal[100 * 100 * 4*2 * 3];
GLuint elementsVertical[100 * 100 * 24];
GLuint elementsHorizontal[100 * 100 * 24];
float spawn[] = { 0.0f, 0.0f };
float goal[] = { 0.0f, 0.0f };
bool isGame = 0;

//Wymiary okna
int screen_width = 640;
int screen_height = 480;


int pozycjaMyszyX; // na ekranie
int pozycjaMyszyY;
int mbutton; // wcisiety klawisz myszy

double kameraX = 0.0;
double kameraZ = 20.0;
double kameraD = -3.0;
double kameraPredkosc;
double kameraKat = 20;
double kameraPredkoscObrotu;
double poprzednie_kameraX;
double poprzednie_kameraZ;
double poprzednie_kameraD;

double rotation = 0;


//macierze
glm::mat4 MV; //modelview - macierz modelu i świata
glm::mat4 P;  //projection - macierz projekcji, czyli naszej perspektywy

// floor
float vertices[4*3*2];
GLuint elements[4*6];

// spawn
float verticesSpawn[24];
GLuint elementsSpawn[24];

// goal
float verticesGoal[24];
GLuint elementsGoal[24];

//shaders
GLuint programID = 0;

unsigned int VBO, VBO2, VBO3, VBO4, VBO5;
unsigned int ebo, ebo2, ebo3, ebo4, ebo5;
unsigned int VAO[5];


float camAngle = 0.0f;
/*###############################################################*/
void mysz(int button, int state, int x, int y)
{
	mbutton = button;
	switch (state)
	{
	case GLUT_UP:
		break;
	case GLUT_DOWN:
		pozycjaMyszyX = x;
		pozycjaMyszyY = y;
		//poprzednie_kameraX = kameraX;
		poprzednie_kameraX = camAngle;
		poprzednie_kameraZ = kameraZ;
		poprzednie_kameraD = kameraD;
		break;

	}
}
/*******************************************/

void mysz_ruch(int x, int y)
{
	if (mbutton == GLUT_LEFT_BUTTON)
	{
		//kameraX = poprzednie_kameraX - (pozycjaMyszyX - x) * 0.1;
		camAngle = (float)poprzednie_kameraX - (pozycjaMyszyX - x) * 0.1f;
		kameraX = camAngle;
		kameraZ = poprzednie_kameraZ - (pozycjaMyszyY - y) * 0.1;
	}
	if (mbutton == GLUT_RIGHT_BUTTON)
	{
		kameraD = poprzednie_kameraD + (pozycjaMyszyY - y) * 0.1;
	}

}
/******************************************/

float dx = 0.0f;
float dz = 0.0f;
float ms = 0.5f;

void klawisz(GLubyte key, int x, int y)
{
	switch (key) {

	case 27:    /* Esc - koniec */
		exit(1);
		break;
	case 'a':
	{
		float preX = spawn[0] + dx + ms * cos(glm::radians(camAngle));
		float preZ = spawn[1] + dz + ms * sin(glm::radians(camAngle));
		int k = amIInTheWall(preX, preZ);
		if (k == 0)
		{
			dx += ms * cos(glm::radians(camAngle));
			dz += ms * sin(glm::radians(camAngle));
		}
		break;
	}
	case 'd':
	{
		float preX = spawn[0] + dx - ms * cos(glm::radians(camAngle));
		float preZ = spawn[1] + dz - ms * sin(glm::radians(camAngle));
		int k = amIInTheWall(preX, preZ);
		if (k == 0)
		{
			dx -= ms * cos(glm::radians(camAngle));
			dz -= ms * sin(glm::radians(camAngle));
		}
		break;
	}
	case 'w':
	{
		float preX = spawn[0] + dx - ms * sin(glm::radians(camAngle));
		float preZ = spawn[1] + dz + ms * cos(glm::radians(camAngle));
		int k = amIInTheWall(preX, preZ);
		if (k == 0)
		{
			dx -= ms * sin(glm::radians(camAngle));
			dz += ms * cos(glm::radians(camAngle));
		}
		break;
	}
	case 's':
	{
		float preX = spawn[0] + dx + ms * sin(glm::radians(camAngle));
		float preZ = spawn[1] + dz - ms * cos(glm::radians(camAngle));
		// check for walls
		int k = amIInTheWall(preX, preZ);
		if (k == 0)
		{
			dx += ms * sin(glm::radians(camAngle));
			dz -= ms * cos(glm::radians(camAngle));
		}
		break;
	}
	case 'x':
		camAngle += 15.0f;
		break;
	case 'z':
		camAngle += 15.0f;
		break;
	}

	if ((dx + spawn[0] > goal[0] - 0.325f) && (dx + spawn[0] < goal[0] + 0.325f)
		&& (dz + spawn[1] > goal[1] - 0.1f) && (dz + spawn[1] < goal[1] + 0.3f))
	{
		glutLeaveMainLoop();
	}
}
/*###############################################################*/
void rysuj(void)
{
	glClearColor(0.0f, 0.0f, 0.4f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(programID);

	MV = glm::mat4(1.0f);
	if (isGame)
	{

		MV = glm::rotate(MV, (float)glm::radians(180.0f), glm::vec3(0, 1, 0));
		MV = glm::rotate(MV, (float)glm::radians(camAngle), glm::vec3(0, 1, 0));
		MV = glm::translate(MV, glm::vec3(-(spawn[0] + dx), -characterHeight, -(spawn[1] + dz)));
	}
	else
	{
		MV = glm::rotate(MV, (float)glm::radians(kameraZ), glm::vec3(1, 0, 0));
		MV = glm::rotate(MV, (float)glm::radians(kameraX), glm::vec3(0, 1, 0));
		MV = glm::translate(MV, glm::vec3(0, 0, kameraD));
		MV = glm::translate(MV, glm::vec3(-xLabDim / 2, -10, 0));

	}

	glm::mat4 MVP = P * MV;

	GLuint MVP_id = glGetUniformLocation(programID, "MVP");
	glUniformMatrix4fv(MVP_id, 1, GL_FALSE, &(MVP[0][0]));

	//floor
	GLfloat attrib[] = { 1.0f, 1.0f, 1.0f };
	glVertexAttrib3fv(1, attrib);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindVertexArray(VAO[0]);
	glDrawElements(GL_QUADS, 4*6, GL_UNSIGNED_INT, 0);

	//goalCube
	attrib[0] = 0.2f; attrib[1] = 1.0f; attrib[2] = 0.2f;
	glVertexAttrib3fv(1, attrib);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindVertexArray(VAO[4]);
	glDrawElements(GL_QUADS, 4*6, GL_UNSIGNED_INT, 0);

	//walls - vertical
	attrib[0] = 0.5f; attrib[1] = 0.5f; attrib[2] = 0.5f;
	glVertexAttrib3fv(1, attrib);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindVertexArray(VAO[1]);
	glDrawElements(GL_QUADS, 24*verticalWallCounter, GL_UNSIGNED_INT, 0);

	//walls - horizontal
	glBindVertexArray(VAO[2]);
	glDrawElements(GL_QUADS, 24 * horizontalWallCounter, GL_UNSIGNED_INT, 0);
	
	/*
	attrib[0] = 0.0f; attrib[1] = 0.0f; attrib[2] = 0.0f;   //outline color
	glVertexAttrib3fv(1, attrib);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBindVertexArray(VAO[1]);
	glDrawElements(GL_QUADS, 24 * verticalWallCounter, GL_UNSIGNED_INT, 0);

	glBindVertexArray(VAO[2]);
	glDrawElements(GL_QUADS, 24 * horizontalWallCounter, GL_UNSIGNED_INT, 0);
	*/

	glFlush();
	glutSwapBuffers();

}
/*###############################################################*/
void rozmiar(int width, int height)
{
	screen_width = width;
	screen_height = height;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, screen_width, screen_height);

	P = glm::perspective(glm::radians(60.0f), (GLfloat)screen_width / (GLfloat)screen_height, visibilityClip, 1000.0f);

	glutPostRedisplay(); // Przerysowanie sceny
}

/*###############################################################*/
void idle()
{

	glutPostRedisplay();
}

/*###############################################################*/
GLfloat k = 0.05f;
GLfloat ad = 0.0;

void timer(int value) {

	glutTimerFunc(20, timer, 0);
}
/*###############################################################*/
int main(int argc, char** argv)
{
	printf("Do you want to play a game?\n1) Of course I do!\n2) No, I just want to look at the labyrinth. Maybe next time!\n");
	char c;
	c = _getch();
	if (c == '1')
	{
		isGame = 1;
		ms = 0.05f;
	}

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(screen_width, screen_height);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Labyrinth");

	glewInit(); //init rozszerzeszen OpenGL z biblioteki GLEW
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);

	glutDisplayFunc(rysuj);			// def. funkcji rysujacej
	glutIdleFunc(idle);				// def. funkcji rysujacej w czasie wolnym procesora (w efekcie: ciagle wykonywanej)
	glutTimerFunc(20, timer, 0);
	glutReshapeFunc(rozmiar);		// def. obslugi zdarzenia resize (GLUT)

	glutKeyboardFunc(klawisz);		// def. obslugi klawiatury
	glutMouseFunc(mysz); 			// def. obslugi zdarzenia przycisku myszy (GLUT)
	glutMotionFunc(mysz_ruch);		// def. obslugi zdarzenia ruchu myszy (GLUT)


	glEnable(GL_DEPTH_TEST);


	const char* fileName = "LabyrinthTemplates/L2.txt";
	float* vertV;
	float* vertH;

	if (readLabyrinth(fileName, &vertV, &vertH) == 0)
	{
		printf("Error while reading from file: %s", fileName);
		return 0;
	}

	kameraD = -2.0 * yLabDim;

	// vertices - floor
	float floorMargin = wallWidth + labyrinthFloorEdge;

	pointToCube(vertices, xLabDim / 2.0f, yLabDim / 2.0f, -floorHeight, 0.0f, floorMargin + xLabDim/2.0f, floorMargin + yLabDim/2.0f);
	wallElems(elements);

	// vertices - spawn & goal
	//wallElems(elementsSpawn);
	pointToCube(verticesGoal, goal[0], goal[1] + 0.375f, 0.4f, 0.6f, 0.125f, 0.125f);
	wallElems(elementsGoal);

	// 0   P   3
	//
	// 1       2
	
	// every wall touches the edge-square:
	//    |
	//   -X-
	//    |
	// the edge-square amendment: +-wallWidth in z

	for (int i = 0; i < verticalWallCounter; i++)
	{
		pointToCube(verticesVertical + (24 * i), vertV[2 * i], vertV[2 * i + 1] - 0.5f, 0.0f, wallHeight, wallWidth, wallWidth + 0.5f);
	
		wallElems(elementsVertical + 24 * i, 8*i);
	}

	// 0        3
	// 1        2

	//the edge-square amendment: +-wallWidth in x

	for (int i = 0; i < horizontalWallCounter; i++)
	{
		pointToCube(verticesHorizontal + 24 * i, vertH[2 * i] + 0.5f, vertH[2 * i + 1], 0.0f, wallHeight, wallWidth + 0.5f, wallWidth);
		
		wallElems(elementsHorizontal + 24*i, 8 * i);
	}

	glGenVertexArrays(5, VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glGenBuffers(1, &VBO2);
	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesVertical), verticesVertical, GL_STATIC_DRAW);
	glGenBuffers(1, &VBO3);
	glBindBuffer(GL_ARRAY_BUFFER, VBO3);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesHorizontal), verticesHorizontal, GL_STATIC_DRAW);
	glGenBuffers(1, &VBO5);
	glBindBuffer(GL_ARRAY_BUFFER, VBO5);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesGoal), verticesGoal, GL_STATIC_DRAW);

	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);
	glGenBuffers(1, &ebo2);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo2);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elementsVertical), elementsVertical, GL_STATIC_DRAW);
	glGenBuffers(1, &ebo3);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo3);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elementsHorizontal), elementsHorizontal, GL_STATIC_DRAW);
	glGenBuffers(1, &ebo5);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo5);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elementsGoal), elementsGoal, GL_STATIC_DRAW);


	glBindVertexArray(VAO[0]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(VAO[1]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo2);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindVertexArray(VAO[2]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO3);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo3);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindVertexArray(VAO[4]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO5);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo5);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	


	programID = loadShaders("vertex_shader.glsl", "fragment_shader.glsl");

	glUseProgram(programID);

	glutMainLoop();

	printf("\n********\nYou win!\n********\n");
	scanf_s("%c", &c, 2);

	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &VBO2);
	glDeleteBuffers(1, &VBO3);
	glDeleteBuffers(1, &VBO5);
	glDeleteBuffers(1, &ebo);
	glDeleteBuffers(1, &ebo2);
	glDeleteBuffers(1, &ebo3);
	glDeleteBuffers(1, &ebo5);
	glDeleteBuffers(5, VAO);
	return(0);
}

int readLabyrinth(const char* fileName, float** vertV, float** vertH)
{
	// File Structure:
	// x y
	// /vertical wall/ /horizontal wall/
	// labyrinth matrix


	char verticalWall;
	char horizontalWall;
	char bothWall;
	char spawnChar;
	char goalChar;
	char wall;
	int nVertices;
	int res;

	FILE* f1;
	fopen_s(&f1, fileName, "r");
	if (f1 == NULL)
	{
		return 0;
	}

	//res = fscanf_s(f1, "%s", line, 2);
	/*
	fscanf_s(f1, "%c", &line, 2);
	while (line != '\n')
	{
		fscanf_s(f1, "%c", &line, 2);
	}
	*/
	res = fscanf_s(f1, "%d %d", &xLabDim, &yLabDim);
	res = fscanf_s(f1, "%c", &wall, 2); // \n
	//res = fscanf_s(f1, "%c %c", &verticalWall, &horizontalWall);
	res = fscanf_s(f1, "%c", &verticalWall, 2);
	res = fscanf_s(f1, "%c", &wall, 2);   // ' '
	res = fscanf_s(f1, "%c", &horizontalWall, 2);
	res = fscanf_s(f1, "%c", &wall, 2);   // ' '
	res = fscanf_s(f1, "%c", &bothWall, 2);
	res = fscanf_s(f1, "%c", &wall, 2);   // ' '
	res = fscanf_s(f1, "%c", &spawnChar, 2);
	res = fscanf_s(f1, "%c", &wall, 2);   // ' '
	res = fscanf_s(f1, "%c", &goalChar, 2);
	res = fscanf_s(f1, "%c", &wall, 2);   // \n

	nVertices = xLabDim * yLabDim * 2;
	*vertV = new float[nVertices];
	*vertH = new float[nVertices];
	verticalWallCounter = 0;
	horizontalWallCounter = 0;
	for (int j = yLabDim-1; j >= 0; j--)
	{
		for (int i = 0; i < xLabDim; i++)
		{
			res = fscanf_s(f1, "%c", &wall, 2);

			if (wall == spawnChar)
			{
				spawn[0] = i + 0.5f;
				spawn[1] = j - 0.5f;
				continue;
			}
			if (wall == goalChar)
			{
				goal[0] = i + 0.5f;
				goal[1] = j - 0.5f;
				continue;
			}

			if (wall == verticalWall || wall==bothWall)
			{
				*(*vertV + verticalWallCounter) = (float)i;
				*(*vertV + verticalWallCounter + 1) = j*1.0f;
				verticalWallCounter += 2;
			}
			if (wall == horizontalWall || wall == bothWall)
			{
				*(*vertH + horizontalWallCounter) = (float)i;
				*(*vertH + horizontalWallCounter + 1) = j*1.0f;
				horizontalWallCounter += 2;
			}

			//res = fscanf_s(f1, "%f %f %f", *vert + 3 * i, *vert + 3 * i + 1, *vert + 3 * i + 2);
		}
		res = fscanf_s(f1, "%c", &wall, 2); // \n is read
	}

	verticalWallCounter /= 2;
	horizontalWallCounter /= 2;

	return 1;
}

int amIInTheWall(float preX, float preZ)
{
	// check for walls
	float l, r, u, d;

	// vertical:
	int i = 0;
	for (i = 0; i < verticalWallCounter; i++)
	{
		// between x?
		//                   const  +  3*v# + x         x-0 y-1 z-2
		l = verticesVertical[24 * i + 3 * 1 + 0] - visibilityClip;
		r = verticesVertical[24 * i + 3 * 2 + 0] + visibilityClip;
		if (preX < r && preX > l)
		{
			// between z
			u = verticesVertical[24 * i + 3 * 0 + 2] + visibilityClip;
			d = verticesVertical[24 * i + 3 * 1 + 2] - visibilityClip;
			if (preZ < u && preZ > d)
			{
				// revert the changes
				//break;
				return 1;
			}
		}
	}
	for (i = 0; i < horizontalWallCounter; i++)
	{
		// between x?
		//                      const  +  3*v# + x         x-0 y-1 z-2
		l = verticesHorizontal[24 * i + 3 * 1 + 0] - visibilityClip;
		r = verticesHorizontal[24 * i + 3 * 2 + 0] + visibilityClip;
		if (preX < r && preX > l)
		{
			// between z
			u = verticesHorizontal[24 * i + 3 * 0 + 2] + visibilityClip;
			d = verticesHorizontal[24 * i + 3 * 1 + 2] - visibilityClip;
			if (preZ < u && preZ > d)
			{
				// revert the changes
				//break;
				return 1;
			}
		}
	}
	// if not in any wall
	return 0;
}

void pointToCube(float* verts, float pointX, float pointZ, float heightZero, float heightCube, float diffX, float diffZ)
{
	verts[0] = pointX - diffX;
	verts[1] = heightZero;
	verts[2] = pointZ + diffZ;

	verts[3] = pointX - diffX;
	verts[4] = heightZero;
	verts[5] = pointZ - diffZ;

	verts[6] = pointX + diffX;
	verts[7] = heightZero;
	verts[8] = pointZ - diffZ;

	verts[9] = pointX + diffX;
	verts[10] = heightZero;
	verts[11] = pointZ + diffZ;

	verts[12] = verts[0];
	verts[13] = heightCube;
	verts[14] = verts[2];

	verts[15] = verts[3];
	verts[16] = heightCube;
	verts[17] = verts[5];

	verts[18] = verts[6];
	verts[19] = heightCube;
	verts[20] = verts[8];

	verts[21] = verts[9];
	verts[22] = heightCube;
	verts[23] = verts[11];
}

void wallElems(GLuint* elems, int offset)
{
	//floor
	elems[0] = offset + 0;
	elems[1] = offset + 1;
	elems[2] = offset + 2;
	elems[3] = offset + 3;

	//front
	elems[4] = offset + 1;
	elems[5] = offset + 2;
	elems[6] = offset + 6;
	elems[7] = offset + 5;

	//back
	elems[8] = offset + 3;
	elems[9] = offset + 0;
	elems[10] = offset + 4;
	elems[11] = offset + 7;

	//right
	elems[12] = offset + 2;
	elems[13] = offset + 3;
	elems[14] = offset + 7;
	elems[15] = offset + 6;

	//left
	elems[16] = offset + 0;
	elems[17] = offset + 1;
	elems[18] = offset + 5;
	elems[19] = offset + 4;

	//roof
	elems[20] = offset + 4;
	elems[21] = offset + 5;
	elems[22] = offset + 6;
	elems[23] = offset + 7;
}