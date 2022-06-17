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
GLuint elementsSpawn[] = {
	//floor
	0, 1, 2, 3,
	//front
	1,2,6,5,
	//back
	3,0,4,7,
	//right
	2,3,7,6,
	//left
	0,1,5,4,
	//roof
	4,5,6,7,
};

// goal
float verticesGoal[24];
GLuint elementsGoal[] = {
	//floor
	0, 1, 2, 3,
	//front
	1,2,6,5,
	//back
	3,0,4,7,
	//right
	2,3,7,6,
	//left
	0,1,5,4,
	//roof
	4,5,6,7,
};

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
		// check for walls
		float l, r, u, d;

		// vertical:
		int i = 0;
		for (i = 0; i < verticalWallCounter; i++)
		{
			// between x?
			//                   const  +  3*v# + x         x-0 y-1 z-2
			l = verticesVertical[24 * i + 3 * 1 + 0];
			r = verticesVertical[24 * i + 3 * 2 + 0];
			if (preX < r && preX > l)
			{
				// between z
				u = verticesVertical[24 * i + 3 * 0 + 2];
				d = verticesVertical[24 * i + 3 * 1 + 2];
				if (preZ < u && preZ > d)
				{
					// revert the changes
					//dx -= ms * cos(glm::radians(camAngle));
					//dz -= ms * sin(glm::radians(camAngle));
					break;
				}
			}
		}
		if (i == verticalWallCounter) // only if not in any of the vertical walls - check for horizontal walls
		{
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
						//dx -= ms * cos(glm::radians(camAngle));
						//dz -= ms * sin(glm::radians(camAngle));
						break;
					}
				}
			}
			if (i == horizontalWallCounter) // if not in any wall
			{
				dx += ms * cos(glm::radians(camAngle));
				dz += ms * sin(glm::radians(camAngle));
			}
		}
		break;
	}
	case 'd':
	{
		float preX = spawn[0] + dx - ms * cos(glm::radians(camAngle));
		float preZ = spawn[1] + dz - ms * sin(glm::radians(camAngle));
		// check for walls
		float l, r, u, d;

		// vertical:
		int i = 0;
		for (i = 0; i < verticalWallCounter; i++)
		{
			// between x?
			//                   const  +  3*v# + x         x-0 y-1 z-2
			l = verticesVertical[24 * i + 3 * 1 + 0];
			r = verticesVertical[24 * i + 3 * 2 + 0];
			if (preX < r && preX > l)
			{
				// between z
				u = verticesVertical[24 * i + 3 * 0 + 2];
				d = verticesVertical[24 * i + 3 * 1 + 2];
				if (preZ < u && preZ > d)
				{
					// revert the changes
					//dx -= ms * cos(glm::radians(camAngle));
					//dz -= ms * sin(glm::radians(camAngle));
					break;
				}
			}
		}
		if (i == verticalWallCounter) // only if not in any of the vertical walls - check for horizontal walls
		{
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
						//dx -= ms * cos(glm::radians(camAngle));
						//dz -= ms * sin(glm::radians(camAngle));
						break;
					}
				}
			}
			if (i == horizontalWallCounter) // if not in any wall
			{
				dx -= ms * cos(glm::radians(camAngle));
				dz -= ms * sin(glm::radians(camAngle));
			}
		}
		break;
	}
	case 'w':
	{
		float preX = spawn[0] + dx - ms * sin(glm::radians(camAngle));
		float preZ = spawn[1] + dz + ms * cos(glm::radians(camAngle));
		// check for walls
		float l, r, u, d;

		// vertical:
		int i = 0;
		for (i = 0; i < verticalWallCounter; i++)
		{
			// between x?
			//                   const  +  3*v# + x         x-0 y-1 z-2
			l = verticesVertical[24 * i + 3 * 1 + 0];
			r = verticesVertical[24 * i + 3 * 2 + 0];
			if (preX < r && preX > l)
			{
				// between z
				u = verticesVertical[24 * i + 3 * 0 + 2];
				d = verticesVertical[24 * i + 3 * 1 + 2];
				if (preZ < u && preZ > d)
				{
					// revert the changes
					//dx -= ms * cos(glm::radians(camAngle));
					//dz -= ms * sin(glm::radians(camAngle));
					break;
				}
			}
		}
		if (i == verticalWallCounter) // only if not in any of the vertical walls - check for horizontal walls
		{
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
						//dx -= ms * cos(glm::radians(camAngle));
						//dz -= ms * sin(glm::radians(camAngle));
						break;
					}
				}
			}
			if (i == horizontalWallCounter) // if not in any wall
			{
				dz += ms * cos(glm::radians(camAngle));
				dx -= ms * sin(glm::radians(camAngle));
			}
		}
		break;
	}
	case 's':
	{
		float preX = spawn[0] + dx + ms * sin(glm::radians(camAngle));
		float preZ = spawn[1] + dz - ms * cos(glm::radians(camAngle));
		// check for walls
		float l, r, u, d;

		// vertical:
		int i = 0;
		for (i = 0; i < verticalWallCounter; i++)
		{
			// between x?
			//                   const  +  3*v# + x         x-0 y-1 z-2
			l = verticesVertical[24 * i + 3 * 1 + 0];
			r = verticesVertical[24 * i + 3 * 2 + 0];
			if (preX < r && preX > l)
			{
				// between z
				u = verticesVertical[24 * i + 3 * 0 + 2];
				d = verticesVertical[24 * i + 3 * 1 + 2];
				if (preZ < u && preZ > d)
				{
					// revert the changes
					//dx -= ms * cos(glm::radians(camAngle));
					//dz -= ms * sin(glm::radians(camAngle));
					break;
				}
			}
		}
		if (i == verticalWallCounter) // only if not in any of the vertical walls - check for horizontal walls
		{
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
						//dx -= ms * cos(glm::radians(camAngle));
						//dz -= ms * sin(glm::radians(camAngle));
						break;
					}
				}
			}
			if (i == horizontalWallCounter) // if not in any wall
			{
				dz -= ms * cos(glm::radians(camAngle));
				dx += ms * sin(glm::radians(camAngle));
			}
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

		//MV = glm::translate(MV, glm::vec3(-(spawn[0] + dx) * cos(camAngle), -characterHeight, -(spawn[1] + dz) * cos(camAngle)));
		MV = glm::translate(MV, glm::vec3(-(spawn[0] + dx), -characterHeight, -(spawn[1] + dz)));
		//MV = glm::translate(MV, glm::vec3(-dx * cos(glm::radians(camAngle)), 0, -dz * sin(glm::radians(camAngle))));
		//MV = glm::rotate(MV, (float)glm::radians(kameraZ), glm::vec3(0, 1, 0));
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

	//ad+= k;

	//if(ad>1 || ad<0)
	//k=-k;

	//GLfloat attrib[] = { ad, 0.0f,0.0f };
	// Aktualizacja wartości atrybutu wejściowego 1.
	//glVertexAttrib3fv(1, attrib);

	/*

	W vertex_shader np:
	layout (location = 1) in vec3 incolor;

	*/
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

	vertices[0] = -floorMargin;
	vertices[1] = -floorHeight;
	vertices[2] = -floorMargin;

	vertices[3] = xLabDim - 1 + floorMargin;
	vertices[4] = -floorHeight;
	vertices[5] = -floorMargin;

	vertices[6] = xLabDim - 1 + floorMargin;
	vertices[7] = -floorHeight;
	vertices[8] = yLabDim - 1 + floorMargin;

	vertices[9] = -floorMargin;
	vertices[10] = -floorHeight;
	vertices[11] = yLabDim - 1 + floorMargin;

	vertices[12] = vertices[0];
	vertices[13] = 0.0f;
	vertices[14] = vertices[2];

	vertices[15] = vertices[3];
	vertices[16] = 0.0f;
	vertices[17] = vertices[5];

	vertices[18] = vertices[6];
	vertices[19] = 0.0f;
	vertices[20] = vertices[8];

	vertices[21] = vertices[9];
	vertices[22] = 0.0f;
	vertices[23] = vertices[11];

	//floor
	elements[0] = 0;
	elements[1] = 1;
	elements[2] = 2;
	elements[3] = 3;

	//front
	elements[4] = 1;
	elements[5] = 2;
	elements[6] = 6;
	elements[7] = 5;

	//back
	elements[8] = 3;
	elements[9] = 0;
	elements[10] = 4;
	elements[11] = 7;

	//right
	elements[12] = 2;
	elements[13] = 3;
	elements[14] = 7;
	elements[15] = 6;

	//left
	elements[16] = 0;
	elements[17] = 1;
	elements[18] = 5;
	elements[19] = 4;

	//roof
	elements[20] = 4;
	elements[21] = 5;
	elements[22] = 6;
	elements[23] = 7;

	// vertices - spawn & goal
	verticesGoal[0] = goal[0] - 0.125f;
	verticesGoal[1] = 0.4f;
	verticesGoal[2] = goal[1] + 0.5f;

	verticesGoal[3] = goal[0] - 0.125f;
	verticesGoal[4] = 0.4f;
	verticesGoal[5] = goal[1] + 0.25f;

	verticesGoal[6] = goal[0] + 0.125f;
	verticesGoal[7] = 0.4f;
	verticesGoal[8] = goal[1] + 0.25f;

	verticesGoal[9] = goal[0] + 0.125f;
	verticesGoal[10] = 0.4f;
	verticesGoal[11] = goal[1] + 0.5f;

	verticesGoal[12] = goal[0] - 0.125f;
	verticesGoal[13] = 0.6f;
	verticesGoal[14] = goal[1] + 0.5f;

	verticesGoal[15] = goal[0] - 0.125f;
	verticesGoal[16] = 0.6f;
	verticesGoal[17] = goal[1] + 0.25f;

	verticesGoal[18] = goal[0] + 0.125f;
	verticesGoal[19] = 0.6f;
	verticesGoal[20] = goal[1] + 0.25f;

	verticesGoal[21] = goal[0] + 0.125f;
	verticesGoal[22] = 0.6f;
	verticesGoal[23] = goal[1] + 0.5f;

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
		verticesVertical[24*i] = vertV[2*i] - wallWidth;			// x
		verticesVertical[24*i + 1] = 0.0f;							// y
		verticesVertical[24*i + 2] = vertV[2*i+1] + wallWidth;      // z

		verticesVertical[24*i+3] = vertV[2*i] - wallWidth;			// x
		verticesVertical[24*i + 4] = 0.0f;							// y
		verticesVertical[24*i + 5] = vertV[2*i+1] - 1 - wallWidth;  // z

		verticesVertical[24*i+6] = vertV[2*i] + wallWidth;			// x
		verticesVertical[24*i + 7] = 0.0f;							// y
		verticesVertical[24*i + 8] = vertV[2*i+1] - 1 - wallWidth;  // z

		verticesVertical[24*i+9] = vertV[2*i] + wallWidth;			// x
		verticesVertical[24*i + 10] = 0.0f;							// y
		verticesVertical[24*i + 11] = vertV[2*i+1] + wallWidth;     // z

		//----------------------------------------------

		verticesVertical[24*i+12] = vertV[2*i] - wallWidth;			// x
		verticesVertical[24*i + 13] = wallHeight;					// y
		verticesVertical[24*i + 14] = vertV[2*i+1] + wallWidth;     // z

		verticesVertical[24*i+15] = vertV[2*i] - wallWidth;			// x
		verticesVertical[24*i + 16] = wallHeight;					// y
		verticesVertical[24*i + 17] = vertV[2*i+1] - 1 - wallWidth; // z

		verticesVertical[24*i+18] = vertV[2*i] + wallWidth;			 // x
		verticesVertical[24*i + 19] = wallHeight;					 // y
		verticesVertical[24*i + 20] = vertV[2*i+1] - 1 - wallWidth;  // z

		verticesVertical[24*i+21] = vertV[2*i] + wallWidth;			// x
		verticesVertical[24*i + 22] = wallHeight;					// y
		verticesVertical[24*i + 23] = vertV[2*i+1] + wallWidth;     // z

		//------------------------------------------------

		// floor
		elementsVertical[24*i] = 8*i;
		elementsVertical[24*i+1] = 8*i+1;
		elementsVertical[24*i+2] = 8*i+2;
		elementsVertical[24*i+3] = 8*i+3;

		//front
		elementsVertical[24 * i+4] = 8 * i+1;
		elementsVertical[24 * i + 5] = 8 * i + 2;
		elementsVertical[24 * i + 6] = 8 * i + 6;
		elementsVertical[24 * i + 7] = 8 * i + 5;

		//back
		elementsVertical[24 * i + 8] = 8 * i + 3;
		elementsVertical[24 * i + 9] = 8 * i + 0;
		elementsVertical[24 * i + 10] = 8 * i + 4;
		elementsVertical[24 * i + 11] = 8 * i + 7;

		//left
		elementsVertical[24 * i + 12] = 8 * i + 0;
		elementsVertical[24 * i + 13] = 8 * i + 1;
		elementsVertical[24 * i + 14] = 8 * i + 5;
		elementsVertical[24 * i + 15] = 8 * i + 4;

		//right
		elementsVertical[24 * i + 16] = 8 * i + 2;
		elementsVertical[24 * i + 17] = 8 * i + 3;
		elementsVertical[24 * i + 18] = 8 * i + 7;
		elementsVertical[24 * i + 19] = 8 * i + 6;

		//roof
		elementsVertical[24 * i + 20] = 8 * i + 4;
		elementsVertical[24 * i + 21] = 8 * i + 5;
		elementsVertical[24 * i + 22] = 8 * i + 6;
		elementsVertical[24 * i + 23] = 8 * i + 7;
	}

	// 0        3
	// 1        2

	//the edge-square amendment: +-wallWidth in x

	for (int i = 0; i < horizontalWallCounter; i++)
	{
		verticesHorizontal[24 * i] = vertH[2*i] - wallWidth;				 // x
		verticesHorizontal[24 * i + 1] = 0.0f;							    // y
		verticesHorizontal[24 * i + 2] = vertH[2*i + 1] + wallWidth;       // z

		verticesHorizontal[24 * i + 3] = vertH[2*i] - wallWidth;			// x
		verticesHorizontal[24 * i + 4] = 0.0f;							    // y
		verticesHorizontal[24 * i + 5] = vertH[2*i + 1] - wallWidth;		// z

		verticesHorizontal[24 * i + 6] = vertH[2*i] + 1 + wallWidth;		// x
		verticesHorizontal[24 * i + 7] = 0.0f;							    // y
		verticesHorizontal[24 * i + 8] = vertH[2*i + 1] - wallWidth;		// z

		verticesHorizontal[24 * i + 9] = vertH[2*i] + 1 + wallWidth;		// x
		verticesHorizontal[24 * i + 10] = 0.0f;						       // y
		verticesHorizontal[24 * i + 11] = vertH[2*i + 1] + wallWidth;     // z

		//----------------------------------------------

		verticesHorizontal[24 * i + 12] = vertH[2*i] - wallWidth;		// x
		verticesHorizontal[24 * i + 13] = wallHeight;					// y
		verticesHorizontal[24 * i + 14] = vertH[2*i + 1] + wallWidth;   // z

		verticesHorizontal[24 * i + 15] = vertH[2*i] - wallWidth;		// x
		verticesHorizontal[24 * i + 16] = wallHeight;					// y
		verticesHorizontal[24 * i + 17] = vertH[2*i + 1] - wallWidth;	// z

		verticesHorizontal[24 * i + 18] = vertH[2*i] + 1 + wallWidth;	// x
		verticesHorizontal[24 * i + 19] = wallHeight;					// y
		verticesHorizontal[24 * i + 20] = vertH[2*i + 1] - wallWidth;	// z

		verticesHorizontal[24 * i + 21] = vertH[2*i] + 1 + wallWidth;	// x
		verticesHorizontal[24 * i + 22] = wallHeight;					// y
		verticesHorizontal[24 * i + 23] = vertH[2*i + 1] + wallWidth;   // z

		//------------------------------------------------

		// floor
		elementsHorizontal[24 * i] = 8 * i;
		elementsHorizontal[24 * i + 1] = 8 * i + 1;
		elementsHorizontal[24 * i + 2] = 8 * i + 2;
		elementsHorizontal[24 * i + 3] = 8 * i + 3;

		//front
		elementsHorizontal[24 * i + 4] = 8 * i + 1;
		elementsHorizontal[24 * i + 5] = 8 * i + 2;
		elementsHorizontal[24 * i + 6] = 8 * i + 6;
		elementsHorizontal[24 * i + 7] = 8 * i + 5;

		//back
		elementsHorizontal[24 * i + 8] = 8 * i + 3;
		elementsHorizontal[24 * i + 9] = 8 * i + 0;
		elementsHorizontal[24 * i + 10] = 8 * i + 4;
		elementsHorizontal[24 * i + 11] = 8 * i + 7;

		//left
		elementsHorizontal[24 * i + 12] = 8 * i + 0;
		elementsHorizontal[24 * i + 13] = 8 * i + 1;
		elementsHorizontal[24 * i + 14] = 8 * i + 5;
		elementsHorizontal[24 * i + 15] = 8 * i + 4;

		//right
		elementsHorizontal[24 * i + 16] = 8 * i + 2;
		elementsHorizontal[24 * i + 17] = 8 * i + 3;
		elementsHorizontal[24 * i + 18] = 8 * i + 7;
		elementsHorizontal[24 * i + 19] = 8 * i + 6;

		//roof
		elementsHorizontal[24 * i + 20] = 8 * i + 4;
		elementsHorizontal[24 * i + 21] = 8 * i + 5;
		elementsHorizontal[24 * i + 22] = 8 * i + 6;
		elementsHorizontal[24 * i + 23] = 8 * i + 7;
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