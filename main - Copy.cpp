#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "GL\glew.h"
#include "GL\freeglut.h"

#include "shaderLoader.h" //narzedzie do ladowania i kompilowania shaderow z pliku

//funkcje algebry liniowej
#include "glm/vec3.hpp" // glm::vec3
#include "glm/vec4.hpp" // glm::vec4
#include "glm/mat4x4.hpp" // glm::mat4
#include "glm/gtc/matrix_transform.hpp" // glm::translate, glm::rotate, glm::scale, glm::perspective

//Labirynth things
int readLabirynth(const char* fileName, float** vertV, float** vertH);
const float wallWidth = 0.1f;
const float wallHeight = 1.0f;
const float labirynthFloorEdge = 3.0f;
const float floorHeight = 2.0f;
int xLabDim = 0;
int yLabDim = 0;
int verticalWallCounter = 0;
int horizontalWallCounter = 0;
float verticesVertical[100 * 100 * 4*2 * 3];
float verticesHorizontal[100 * 100 * 4*2 * 3];
GLuint elementsVertical[100 * 100 * 24];
GLuint elementsHorizontal[100 * 100 * 24];

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
glm::vec3 lightPos(100, 100.0f, 500.0f);

// floor
float vertices[4*3*2];
GLuint elements[4*6];
float verticesNormal[] = {
	//floor
	0.0f, 1.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	0.0f, 1.0f, 0.0f,

	//front
	0.0f, 0.0f, -1.0f,
	0.0f, 0.0f, -1.0f,
	0.0f, 0.0f, -1.0f,
	0.0f, 0.0f, -1.0f,

	//back
	0.0f, 0.0f, 1.0f,
	0.0f, 0.0f, 1.0f,
	0.0f, 0.0f, 1.0f,
	0.0f, 0.0f, 1.0f,

	//right
	1.0f, 0.0f, 0.0f,
	1.0f, 0.0f, 0.0f,
	1.0f, 0.0f, 0.0f,
	1.0f, 0.0f, 0.0f,

	//left
	-1.0f, 0.0f, 0.0f,
	-1.0f, 0.0f, 0.0f,
	-1.0f, 0.0f, 0.0f,
	-1.0f, 0.0f, 0.0f,

	//roof
	0.0f, -1.0f, 0.0f,
	0.0f, -1.0f, 0.0f,
	0.0f, -1.0f, 0.0f,
	0.0f, -1.0f, 0.0f,
};

//shaders
GLuint programID = 0;

//light
GLuint alfa_id = 0;
GLuint lightColor_id = 0;
GLuint lightPos_id = 0;
//GLuint viewPos_id = 0;

GLuint materialambient_id = 0;
GLuint materialdiffuse_id = 0;
GLuint materialspecular_id = 0;
GLuint materialshine_id = 0;



unsigned int VBO, VBO2, VBO3;
unsigned int VBONormal, VBO2Normal, VBO3Normal;
unsigned int ebo, ebo2, ebo3;
unsigned int VAO[3];


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
		poprzednie_kameraX = kameraX;
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
		kameraX = poprzednie_kameraX - (pozycjaMyszyX - x) * 0.1;
		kameraZ = poprzednie_kameraZ - (pozycjaMyszyY - y) * 0.1;
	}
	if (mbutton == GLUT_RIGHT_BUTTON)
	{
		kameraD = poprzednie_kameraD + (pozycjaMyszyY - y) * 0.1;
	}

}
/******************************************/



void klawisz(GLubyte key, int x, int y)
{
	switch (key) {

	case 27:    /* Esc - koniec */
		exit(1);
		break;

	case 'x':

		break;
	case '1':

		break;
	case '2':

		break;
	}


}
/*###############################################################*/
void rysuj(void)
{

	//GLfloat color[] = { 0.0f, 0.0f, 0.6f, 1.0f };
	//glClearBufferfv(GL_COLOR, 0, color);

	glClearColor(0.0f, 0.0f, 0.6f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(programID);

	MV = glm::mat4(1.0f);
	MV = glm::translate(MV, glm::vec3(0, 0, kameraD));
	MV = glm::translate(MV, glm::vec3(-xLabDim/2, 0, 0));
	MV = glm::rotate(MV, (float)glm::radians(kameraZ), glm::vec3(1, 0, 0));
	MV = glm::rotate(MV, (float)glm::radians(kameraX), glm::vec3(0, 1, 0));

	glm::mat4 MVP = P * MV;

	GLuint MVP_id = glGetUniformLocation(programID, "MVP");
	glUniformMatrix4fv(MVP_id, 1, GL_FALSE, &(MVP[0][0]));

	GLfloat attrib[] = { 1.0f, 1.0f, 1.0f };
	glVertexAttrib3fv(1, attrib);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindVertexArray(VAO[0]);
	glDrawElements(GL_QUADS, 4*6, GL_UNSIGNED_INT, 0);


	attrib[0] = 0.5f; attrib[1] = 0.5f; attrib[2] = 0.5f;
	glVertexAttrib3fv(1, attrib);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindVertexArray(VAO[1]);
	glDrawElements(GL_QUADS, 24*verticalWallCounter, GL_UNSIGNED_INT, 0);

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

	P = glm::perspective(glm::radians(60.0f), (GLfloat)screen_width / (GLfloat)screen_height, 1.0f, 1000.0f);

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

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(screen_width, screen_height);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Przyklad 5");

	glewInit(); //init rozszerzeszen OpenGL z biblioteki GLEW

	glutDisplayFunc(rysuj);			// def. funkcji rysujacej
	glutIdleFunc(idle);				// def. funkcji rysujacej w czasie wolnym procesora (w efekcie: ciagle wykonywanej)
	glutTimerFunc(20, timer, 0);
	glutReshapeFunc(rozmiar);		// def. obslugi zdarzenia resize (GLUT)

	glutKeyboardFunc(klawisz);		// def. obslugi klawiatury
	glutMouseFunc(mysz); 			// def. obslugi zdarzenia przycisku myszy (GLUT)
	glutMotionFunc(mysz_ruch);		// def. obslugi zdarzenia ruchu myszy (GLUT)


	glEnable(GL_DEPTH_TEST);

	const char* fileName = "LabirynthTemplates/L1.txt";
	float* vertV;
	float* vertH;

	if (readLabirynth(fileName, &vertV, &vertH) == 0)
	{
		printf("Error while reading from file: %s", fileName);
		return 0;
	}

	kameraD = -2.0 * yLabDim;

	// vertices - floor
	float floorMargin = wallWidth + labirynthFloorEdge;

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

	glGenVertexArrays(3, VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glGenBuffers(1, &VBO2);
	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesVertical), verticesVertical, GL_STATIC_DRAW);
	glGenBuffers(1, &VBO3);
	glBindBuffer(GL_ARRAY_BUFFER, VBO3);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesHorizontal), verticesHorizontal, GL_STATIC_DRAW);

	glGenBuffers(1, &VBONormal);
	glBindBuffer(GL_ARRAY_BUFFER, VBONormal);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesNormal), verticesNormal, GL_STATIC_DRAW);
	glGenBuffers(1, &VBO2Normal);
	glBindBuffer(GL_ARRAY_BUFFER, VBO2Normal);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesVertical), verticesVertical, GL_STATIC_DRAW);
	glGenBuffers(1, &VBO3Normal);
	glBindBuffer(GL_ARRAY_BUFFER, VBO3Normal);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesHorizontal), verticesHorizontal, GL_STATIC_DRAW);

	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);
	glGenBuffers(1, &ebo2);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo2);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elementsVertical), elementsVertical, GL_STATIC_DRAW);
	glGenBuffers(1, &ebo3);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo3);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elementsHorizontal), elementsHorizontal, GL_STATIC_DRAW);


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
	


	programID = loadShaders("vertex_shader.glsl", "fragment_shader.glsl");

	glUseProgram(programID);


	lightColor_id = glGetUniformLocation(programID, "lightColor");
	lightPos_id = glGetUniformLocation(programID, "lightPos");


	materialambient_id = glGetUniformLocation(programID, "material.ambient");
	materialdiffuse_id = glGetUniformLocation(programID, "material.diffuse");
	materialspecular_id = glGetUniformLocation(programID, "material.specular");
	materialshine_id = glGetUniformLocation(programID, "material.shininess");
	alfa_id = glGetUniformLocation(programID, "alfa");


	glutMainLoop();

	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &VBO2);
	glDeleteBuffers(1, &VBO3);
	glDeleteBuffers(1, &VBONormal);
	glDeleteBuffers(1, &VBO2Normal);
	glDeleteBuffers(1, &VBO3Normal);
	glDeleteBuffers(1, &ebo);
	glDeleteBuffers(1, &ebo2);
	glDeleteBuffers(1, &ebo3);
	glDeleteBuffers(3, VAO);
	return(0);
}

int readLabirynth(const char* fileName, float** vertV, float** vertH)
{
	// File Structure:
	// x y
	// /vertical wall/ /horizontal wall/
	// labirynth matrix


	char verticalWall;
	char horizontalWall;
	char bothWall;
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