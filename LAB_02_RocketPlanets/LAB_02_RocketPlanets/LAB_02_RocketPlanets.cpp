#include <iostream>
#include <vector>
#include "ShaderMaker.h"
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <cmath>

// Include GLM; libreria matematica per le opengl
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "LineCircleCollisionDetection.h"

using namespace glm;

#define PI 3.14159265358979323846
#define rocketTriangles 11
#define circleTriangles 30
#define maxPlanets 7
#define delta 100

typedef struct { float x, y, z, r, g, b, a; } Point;
typedef struct { int posx, posy, scale, direction; }Planet;
typedef struct { int dir, size; }Star;

static unsigned int programId;
unsigned int VAO_Rocket, VAO_Planets, VAO_GameoverX, VAO_Stars, VAO_Checkpoint, VAO_WinningTick, VAO_Hitbox;
unsigned int VBO_Rocket, VBO_Planets, VBO_GameoverX, VBO_Stars, VBO_Checkpoint, VBO_WinningTick,VBO_Hitbox, loc, MatProj, MatModel;
mat4 Projection;  //Matrice di proiezione
mat4 Model; //Matrice per il cambiamento di sistema di riferimento: da Sistema diriferimento dell'oggetto a sistema di riferimento nel Mondo

vec4 red_peach = { 1.0f, 0.419f, 0.458f, 1.0f };
vec4 mild_peach = { 1.0f, 0.909f, 0.858f, 1.0f };
vec4 cherry = { 0.913f, 0.125f, 0.215f, 1.0f };
vec4 azure = { 0.745f, 0.890f, 0.956f, 1.0f };
vec4 white = { 1.0f, 1.0f, 1.0f, 1.0f };
vec4 p1 = { 0.592f, 0.866f, 0.784f, 1.0f };
vec4 p2 = { 0.270f, 0.631f, 0.521f, 1.0f };
vec4 red = { 1.0f,0.0f,0.0f,1.0f };
vec4 green = { 0.0f,1.0f,0.0f,1.0f };
vec4 transparent_black = { 0.0f, 0.0f, 0.0f, 0.7f };

int nRocketVertices = 3 * (rocketTriangles + (2 * circleTriangles));
Point* RocketVerts = new Point[nRocketVertices];

int planetVertices = circleTriangles * 3 * 5;
int maxPVertices = planetVertices * maxPlanets;

Point* Planets = new Point[maxPVertices];
int countPVertices = 0;

float planetDraw[][3] = { {0.0,0.0,1.0},{0.2,0.7,0.2},{0.5,0.5,0.08},{-0.3,-0.6,0.15},{0.7,0.0,0.15} };
int positions[][maxPlanets] = { {200,590},{250,360} ,{510,520},{560,290}, {800,400}, {1040,300}, {1050,560} };
Planet* InfoPlanets = new Planet[maxPlanets];
int currentPlanets = 0;

Point* GameoverX = new Point[10];

Point* WinningTick = new Point[12];
Point* Checkpoint = new Point[circleTriangles * 3];
int posCheck[2] = { 640, 640 };

Point* Stars = new Point[5000];
Star* InfoStars = new Star[5000];
int currentStars = 0;

// Viewport size
int width = 1280;
int height = 720;

//rocket motion
bool pressing_left;
bool pressing_right;
bool pressing_up;
bool pressing_down;
int HorizontalVelocity = 0;
int VerticalVelocity = 0;
int posxRocket = width / 2;
int posyRocket = 150;
float rocket_size = 70.0;
float angleRotation = 0.0;

//ROCKET HITBOX
Point RocketPolygonHitbox[5];

//gameover
bool gameover = false;
bool win = false;

/*********************************************************************************************************/
/********************************** BASIC DRAWING FUNCTIONS **********************************************/
/*********************************************************************************************************/
void drawPlane(float x, float y, float width, float height, vec4 color, Point* piano)
{
	//first triangle = top one
	piano[0].x = x;	piano[0].y = y; piano[0].z = 0;
	piano[1].x = x + width;	piano[1].y = y;	piano[1].z = 0;
	piano[2].x = x + width;	piano[2].y = y + height; piano[2].z = 0;
	//second triangle
	piano[3].x = x + width;	piano[3].y = y + height; piano[3].z = 0;
	piano[4].x = x;	piano[4].y = y + height; piano[4].z = 0;
	piano[5].x = x;	piano[5].y = y; piano[5].z = 0;

	for (int i = 0; i < 6; i++) {
		piano[i].r = color.r; piano[i].g = color.g; piano[i].b = color.b; piano[i].a = color.a;
	}
}

void drawTrapez(float x, float y, float width, float height, float bias, vec4 color, Point* piano, bool upside) {

	if (!upside) {
		//primo triangolo = base minore
		piano[0].x = x;	piano[0].y = y; piano[0].z = 0;
		piano[1].x = x + width;	piano[1].y = y;	piano[1].z = 0;
		piano[2].x = x + width + bias;	piano[2].y = y + height; piano[2].z = 0;
		//secondo triangolo = base maggiore
		piano[3].x = x + width + bias;	piano[3].y = y + height; piano[3].z = 0;
		piano[4].x = x - bias;	piano[4].y = y + height; piano[4].z = 0;
		piano[5].x = x;	piano[5].y = y; piano[5].z = 0;
	}
	else {
		//primo triangolo = base maggiore
		piano[0].x = x;	piano[0].y = y; piano[0].z = 0;
		piano[1].x = x + width;	piano[1].y = y;	piano[1].z = 0;
		piano[2].x = x + width - bias;	piano[2].y = y + height; piano[2].z = 0;
		//secondo triangolo = base minore
		piano[3].x = x + width - bias;	piano[3].y = y + height; piano[3].z = 0;
		piano[4].x = x + bias;	piano[4].y = y + height; piano[4].z = 0;
		piano[5].x = x;	piano[5].y = y; piano[5].z = 0;
	}

	for (int i = 0; i < 6; i++) {
		piano[i].r = color.r; piano[i].g = color.g; piano[i].b = color.b; piano[i].a = color.a;
	}

}

void drawCircle(float centerX, float centerY, float ray, vec4 color_in, vec4 color_out, Point* Cerchio) {
	int i;
	float stepA = (2 * PI) / circleTriangles;

	int comp = 0;
	// step = 1 -> triangoli adiacenti, step = n -> triangoli distanti step l'uno dall'altro
	for (i = 0; i < circleTriangles; i++)
	{
		Cerchio[comp].x = (cos((double)i * stepA) * ray) + centerX;
		Cerchio[comp].y = (sin((double)i * stepA) * ray) + centerY;
		Cerchio[comp].z = 0.0;
		Cerchio[comp].r = color_out.r; Cerchio[comp].g = color_out.g; Cerchio[comp].b = color_out.b; Cerchio[comp].a = color_out.a;

		Cerchio[comp + 1].x = (cos((double)(i + 1) * stepA) * ray) + centerX;
		Cerchio[comp + 1].y = (sin((double)(i + 1) * stepA) * ray) + centerY;
		Cerchio[comp + 1].z = 0.0;
		Cerchio[comp + 1].r = color_out.r; Cerchio[comp + 1].g = color_out.g; Cerchio[comp + 1].b = color_out.b; Cerchio[comp + 1].a = color_out.a;

		Cerchio[comp + 2].x = centerX;
		Cerchio[comp + 2].y = centerY;
		Cerchio[comp + 2].z = 0.0;
		Cerchio[comp + 2].r = color_in.r; Cerchio[comp + 2].g = color_in.g; Cerchio[comp + 2].b = color_in.b; Cerchio[comp + 2].a = color_in.a;

		comp += 3;
	}
}

/*********************************************************************************************************/
/********************************** DRAW ROCKET FUNCTION *************************************************/
/*********************************************************************************************************/
void drawRocket(vec4 color_top, vec4 color_middle, vec4 color_center, Point* Rocket) {

	int countVertices = 0;
	int nVertices;

	//-------------------the top part is only a triangle---------------------------
	//-----------------------------------------------------------------------------

	float vertices[][3] = { {0.0,-1.0,0.0},{-0.2,-0.7,-0.0},{0.2,-0.7,0.0} };
	for (int i = 0; i < 3; i++) {
		Rocket[countVertices + i].x = vertices[i][0]; Rocket[countVertices + i].y = vertices[i][1]; Rocket[countVertices + i].z = vertices[i][2];
		Rocket[countVertices + i].r = color_top.r; Rocket[countVertices + i].g = color_top.g; Rocket[countVertices + i].b = color_top.b; Rocket[countVertices + i].a = color_top.a;
	}
	countVertices += 3;

	//---------------------Trapezio - triangoli=2-----------------------------------
	//-----------------------------------------------------------------------------
	nVertices = 3 * 2;
	Point* trap = new Point[nVertices];
	drawTrapez(-0.2f, -0.7f, 0.4, 0.3, 0.1, color_middle, trap, false);
	for (int i = 0; i < nVertices; i++) {
		Rocket[countVertices + i].x = trap[i].x; Rocket[countVertices + i].y = trap[i].y; Rocket[countVertices + i].z = trap[i].z;
		Rocket[countVertices + i].r = trap[i].r; Rocket[countVertices + i].g = trap[i].g; Rocket[countVertices + i].b = trap[i].b; Rocket[countVertices + i].a = trap[i].a;
	}
	countVertices = countVertices + nVertices;

	//-------------------Rettangolo - triangoli=2-----------------------------------
	//------------------------------------------------------------------------------
	nVertices = 3 * 2;
	Point* rett = new Point[nVertices];
	drawPlane(-0.3f, -0.4f, 0.6, 0.9, color_center, rett);
	for (int i = 0; i < nVertices; i++) {
		Rocket[countVertices + i].x = rett[i].x; Rocket[countVertices + i].y = rett[i].y; Rocket[countVertices + i].z = rett[i].z;
		Rocket[countVertices + i].r = rett[i].r; Rocket[countVertices + i].g = rett[i].g; Rocket[countVertices + i].b = rett[i].b; Rocket[countVertices + i].a = rett[i].a;
	}
	countVertices = countVertices + nVertices;

	//-----------------Trapezio ribaltato - triangoli = 2--------------------------
	//-----------------------------------------------------------------------------
	nVertices = 3 * 2;
	trap = new Point[nVertices];
	drawTrapez(-0.3f, 0.5f, 0.6, 0.3, 0.1, color_middle, trap, true);
	for (int i = 0; i < nVertices; i++) {
		Rocket[countVertices + i].x = trap[i].x; Rocket[countVertices + i].y = trap[i].y;  Rocket[countVertices + i].z = trap[i].z;
		Rocket[countVertices + i].r = trap[i].r; Rocket[countVertices + i].g = trap[i].g; Rocket[countVertices + i].b = trap[i].b; Rocket[countVertices + i].a = trap[i].a;
	}
	countVertices = countVertices + nVertices;

	//-------------------Due degli alettoni sono triangoli--------------------------
	//------------------------------------------------------------------------------

	float al1[][3] = { {-0.3,0.5,0.0},{-0.2,0.8,0.0},{-0.4,1.0,0.0} };
	for (int i = 0; i < 3; i++) {
		Rocket[countVertices + i].x = al1[i][0]; Rocket[countVertices + i].y = al1[i][1]; Rocket[countVertices + i].z = al1[i][2];
		Rocket[countVertices + i].r = color_top.r; Rocket[countVertices + i].g = color_top.g; Rocket[countVertices + i].b = color_top.b; Rocket[countVertices + i].a = color_top.a;
	}
	countVertices += 3;

	float al2[][3] = { {0.3,0.5,0.0},{0.4,1.0,0.0},{0.2,0.8,0.0} };
	for (int i = 0; i < 3; i++) {
		Rocket[countVertices + i].x = al2[i][0]; Rocket[countVertices + i].y = al2[i][1]; Rocket[countVertices + i].z = al2[i][2];
		Rocket[countVertices + i].r = color_top.r; Rocket[countVertices + i].g = color_top.g; Rocket[countVertices + i].b = color_top.b; Rocket[countVertices + i].a = color_top.a;
	}
	countVertices += 3;

	//--------------------il terzo è un rettangolo-----------------------------------
	//-------------------------------------------------------------------------------
	nVertices = 3 * 2;
	rett = new Point[nVertices];
	drawPlane(-0.05f, 0.5f, 0.1, 0.5, color_top, rett);
	for (int i = 0; i < nVertices; i++) {
		Rocket[countVertices + i].x = rett[i].x; Rocket[countVertices + i].y = rett[i].y; Rocket[countVertices + i].z = rett[i].z;
		Rocket[countVertices + i].r = rett[i].r; Rocket[countVertices + i].g = rett[i].g; Rocket[countVertices + i].b = rett[i].b; Rocket[countVertices + i].a = rett[i].a;
	}
	countVertices = countVertices + nVertices;


	//----disegno oblò-------------------------------------
	//--------------------------------------------------------
	Point* cerchio = new Point[circleTriangles * 3];
	nVertices = 3 * circleTriangles;
	drawCircle(0.0, -0.4, 0.15, azure, azure, cerchio);
	for (int i = 0; i < nVertices; i++) {
		Rocket[countVertices + i].x = cerchio[i].x; Rocket[countVertices + i].y = cerchio[i].y; Rocket[countVertices + i].z = cerchio[i].z;
		Rocket[countVertices + i].r = cerchio[i].r; Rocket[countVertices + i].g = cerchio[i].g; Rocket[countVertices + i].b = cerchio[i].b; Rocket[countVertices + i].a = cerchio[i].a;
	}
	countVertices = countVertices + nVertices;

	//----disegno riflesso oblò-------------------------------------
	//--------------------------------------------------------
	cerchio = new Point[circleTriangles * 3];
	nVertices = 3 * circleTriangles;
	drawCircle(-0.05, -0.45, 0.05, white, white, cerchio);
	for (int i = 0; i < nVertices; i++) {
		Rocket[countVertices + i].x = cerchio[i].x; Rocket[countVertices + i].y = cerchio[i].y; Rocket[countVertices + i].z = cerchio[i].z;
		Rocket[countVertices + i].r = cerchio[i].r; Rocket[countVertices + i].g = cerchio[i].g; Rocket[countVertices + i].b = cerchio[i].b; Rocket[countVertices + i].a = cerchio[i].a;
	}
	countVertices = countVertices + nVertices;

	//---memorizzo l'hitbox del razzo----------------------------
	//-----------------------------------------------------------
	RocketPolygonHitbox[0].x = (float)posxRocket - ((rocket_size) * 0.3); RocketPolygonHitbox[0].y = posyRocket - ((rocket_size));
	RocketPolygonHitbox[1].x = (float)posxRocket + ((rocket_size) * 0.3); RocketPolygonHitbox[1].y = posyRocket - ((rocket_size));
	RocketPolygonHitbox[2].x = (float)posxRocket + ((rocket_size) * 0.3); RocketPolygonHitbox[2].y = posyRocket + ((rocket_size)*0.4);
	RocketPolygonHitbox[3].x = (float)posxRocket;								RocketPolygonHitbox[3].y = posyRocket + ((rocket_size));
	RocketPolygonHitbox[4].x = (float)posxRocket - ((rocket_size) * 0.3); RocketPolygonHitbox[4].y = posyRocket + ((rocket_size)*0.4);

	for (int i = 0; i < 5; i++) {
		RocketPolygonHitbox[i].r = 0.0; RocketPolygonHitbox[i].g = 1.0; RocketPolygonHitbox[i].b = 0.0; RocketPolygonHitbox[i].a = 1.0;
		printf("V1 - x=%.2f , y=%.2f\n", RocketPolygonHitbox[i].x, RocketPolygonHitbox[i].y);
	}
}

/*********************************************************************************************************/
/********************************** DRAW PLANETS FUNCTION *************************************************/
/*********************************************************************************************************/
void drawPlanet(vec4 colorP, vec4 colorO) {

	//Disegno il cerchio principale
	Point* cerchio = new Point[circleTriangles * 3];
	int nVertices = 3 * circleTriangles;

	for (int j = 0; j < 5; j++) {

		vec4 color;
		if (j == 0) color = colorP;
		else color = colorO;

		drawCircle(planetDraw[j][0], planetDraw[j][1], planetDraw[j][2], color, color, cerchio);
		for (int i = 0; i < nVertices; i++) {
			Planets[countPVertices + i].x = cerchio[i].x; Planets[countPVertices + i].y = cerchio[i].y; Planets[countPVertices + i].z = cerchio[i].z;
			Planets[countPVertices + i].r = cerchio[i].r; Planets[countPVertices + i].g = cerchio[i].g; Planets[countPVertices + i].b = cerchio[i].b;
			Planets[countPVertices + i].a = cerchio[i].a;
		}
		countPVertices = countPVertices + nVertices;

	}

	//set transform parameter as random numbers
	InfoPlanets[currentPlanets].scale = rand() % 30 + 20;

	InfoPlanets[currentPlanets].posx = positions[currentPlanets][0];
	InfoPlanets[currentPlanets].posy = positions[currentPlanets][1];

	if (currentPlanets % 2 == 0) {
		InfoPlanets[currentPlanets].direction = 1;
	}
	else {
		InfoPlanets[currentPlanets].direction = -1;
	}

	currentPlanets++;
}

/*********************************************************************************************************/
/********************************** DRAW GAMEOVER X FUNCTION *********************************************/
/*********************************************************************************************************/

void drawGameoverX(vec4 colorB, vec4 colorX) {

	int count = 0;

	//backgorund
	int nVertices = 6;
	Point* piano = new Point[6];
	drawPlane(0.0, 0.0, 1.0, 1.0, colorB, piano);
	for (int i = 0; i < nVertices; i++) {
		GameoverX[count + i].x = piano[i].x; GameoverX[count + i].y = piano[i].y; GameoverX[count + i].z = piano[i].z;
		GameoverX[count + i].r = piano[i].r; GameoverX[count + i].g = piano[i].g; GameoverX[count + i].b = piano[i].b; 
		GameoverX[count + i].a = piano[i].a;
	}
	count = count + nVertices;

	float xPoints[][2] = { {1.0,1.0},{-1.0,-1.0},{-1.0,1.0},{1.0,-1.0} };
	//x
	for (int i = 0; i < 4; i++) {
		GameoverX[count].x = xPoints[i][0]; GameoverX[count].y = xPoints[i][1]; GameoverX[count].z = 0.0;
		GameoverX[count].r = colorX.r; GameoverX[count].g = colorX.g; GameoverX[count].b = colorX.b; GameoverX[count].a = colorX.a;
		count++;
	}
}

/*********************************************************************************************************/
/******************************************* DRAW CHECKPOINT *********************************************/
/*********************************************************************************************************/
void drawCheckpoint(vec4 color_in, vec4 color_out) {
	drawCircle(0.0, 0.0, 1.0, color_in, color_out, Checkpoint);
}

/*********************************************************************************************************/
/***************************************** DRAW CHECKPOINT TICK ******************************************/
/*********************************************************************************************************/
void drawCheckpointTick(vec4 color) {
	float xPoints[][3] = { {-1.0,0.2},{-1.0,-0.2},{0.0,-1.0},{0.0,-1.0} ,{0.0,-0.6},{-1.0,0.2},
						   {0.0,-1.0},{0.0,-0.6},{1.0,1.0},{1.0,1.0},{1.0,0.6}, {0.0,-1.0}};

	for (int i = 0; i < 12; i++) {
		WinningTick[i].x = xPoints[i][0]; WinningTick[i].y = xPoints[i][1]; WinningTick[i].z = 0.0;
		WinningTick[i].r = color.r; WinningTick[i].g = color.g; WinningTick[i].b = color.b; WinningTick[i].a = color.a;
	}
}

/*********************************************************************************************************/
/**************************************** INIT FUNCTIONS *************************************************/
/*********************************************************************************************************/

void initShader(void)
{
	GLenum ErrorCheckValue = glGetError();
	char* vertexShader = (char*)"vertexShader_C_M.glsl";
	char* fragmentShader = (char*)"fragmentShader_C_M.glsl";
	programId = ShaderMaker::createProgram(vertexShader, fragmentShader);
	glUseProgram(programId);

}


void init(void)
{
	drawRocket(cherry, red_peach, mild_peach, RocketVerts);
	/*printf("Rocket vertices: %d\n", nRocketVertices);*/
	//Genero un VAO razzo
	glGenVertexArrays(1, &VAO_Rocket);
	glBindVertexArray(VAO_Rocket);
	glGenBuffers(1, &VBO_Rocket);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_Rocket);
	glBufferData(GL_ARRAY_BUFFER, nRocketVertices * sizeof(Point), &RocketVerts[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glGenVertexArrays(1, &VAO_Hitbox);
	glBindVertexArray(VAO_Hitbox);
	glGenBuffers(1, &VBO_Hitbox);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_Hitbox);
	glBufferData(GL_ARRAY_BUFFER, 5* sizeof(Point), &RocketPolygonHitbox[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	printf("\nGenerating...\n");
	for (int i = 0; i < maxPlanets; i++) {
		drawPlanet(p1, p2);
		printf("Planet %d - posx=%d - posy=%d - scale=%d\n", i + 1, InfoPlanets[i].posx, InfoPlanets[i].posy, InfoPlanets[i].scale);
	}

	printf("Generated %d planets\nTotal = %d vertices\nVertices per planet: %d", currentPlanets, countPVertices, planetVertices);

	//Genero un VAO pianeti
	glGenVertexArrays(1, &VAO_Planets);
	glBindVertexArray(VAO_Planets);
	glGenBuffers(1, &VBO_Planets);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_Planets);
	glBufferData(GL_ARRAY_BUFFER, maxPVertices * sizeof(Point), &Planets[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);


	//GameoverX
	drawGameoverX(transparent_black, red);
	glGenVertexArrays(1, &VAO_GameoverX);
	glBindVertexArray(VAO_GameoverX);
	glGenBuffers(1, &VBO_GameoverX);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_GameoverX);
	glBufferData(GL_ARRAY_BUFFER, 10 * sizeof(Point), &GameoverX[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	//Winning Tick
	drawCheckpointTick(green);
	glGenVertexArrays(1, &VAO_WinningTick);
	glBindVertexArray(VAO_WinningTick);
	glGenBuffers(1, &VBO_WinningTick);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_WinningTick);
	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(Point), &WinningTick[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	//Stars
	for (int i = 0; i < 500; i++) {
		int x = rand() % 1000 + 1; double px = (float)x; px = -1.0f + ((px *2.0f) / 1000.0f);
		int y = rand() % 1000 + 1; double py = (float)y; py = -1.0f + ((py * 2.0f) / 1000.0f);
		/*printf("%f , %f\n", px, py);*/
		Stars[i].x = px; Stars[i].y = py; Stars[i].z = 0.0f;
		Stars[i].r = 1.0f; Stars[i].g = 1.0f; Stars[i].b = 1.0f; Stars[i].a = 1.0f;

		if (i % 2 == 0) {
			InfoStars[i].dir = 1;
		}
		else {
			InfoStars[i].dir = -1;
		}

		InfoStars[i].size = rand() % 3 + 3;
		/*printf("%d\n", InfoStars[i].size);*/
		currentStars++;
	}
	glGenVertexArrays(1, &VAO_Stars);
	glBindVertexArray(VAO_Stars);
	glGenBuffers(1, &VBO_Stars);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_Stars);
	glBufferData(GL_ARRAY_BUFFER, 5000 * sizeof(Point), &Stars[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);


	//Checkpoint
	drawCheckpoint(red, white);
	glGenVertexArrays(1, &VAO_Checkpoint);
	glBindVertexArray(VAO_Checkpoint);
	glGenBuffers(1, &VBO_Checkpoint);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_Checkpoint);
	glBufferData(GL_ARRAY_BUFFER, circleTriangles*3 * sizeof(Point), &Checkpoint[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);


	//Definisco il colore che verrà assegnato allo schermo
	glClearColor(0.0, 0.0, 0.0, 1.0);

	Projection = ortho(0.0f, float(width), 0.0f, float(height));
	MatProj = glGetUniformLocation(programId, "Projection");
	MatModel = glGetUniformLocation(programId, "Model");
}

/*********************************************************************************************************/
/*************************************** KEYBOARD EVENT FUNCTIONS ****************************************/
/*********************************************************************************************************/

void keyboardPressedEvent(unsigned char key, int x, int y) {

	switch (key)
	{
	case 'a':
		pressing_left = true;
		break;
	case 'd':
		pressing_right = true;
		break;
	case 'w':
		pressing_up = true;
		break;
	case 's':
		pressing_down = true;
		break;
	case 27:
		exit(0);
		break;
	default:
		break;
	}
}

void keyboardReleasedEvent(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'a':
		pressing_left = false;
		break;
	case 'd':
		pressing_right = false;
		break;
	case 'w':
		pressing_up = false;
		break;
	case 's':
		pressing_down = false;
		break;
	case 27:
		exit(0);
		break;
	default:
		break;
	}
}

/*********************************************************************************************************/
/************************************ UPDATE HITBOX AND COLLISION DETECTION*******************************/
/*********************************************************************************************************/

void calculateRocketHitbox(float (*newHit)[2]) {

	RocketPolygonHitbox[0].x = (float)posxRocket - ((rocket_size) * 0.3); RocketPolygonHitbox[0].y = posyRocket - ((rocket_size));
	RocketPolygonHitbox[1].x = (float)posxRocket + ((rocket_size) * 0.3); RocketPolygonHitbox[1].y = posyRocket - ((rocket_size));
	RocketPolygonHitbox[2].x = (float)posxRocket + ((rocket_size) * 0.3); RocketPolygonHitbox[2].y = posyRocket + ((rocket_size) * 0.4);
	RocketPolygonHitbox[3].x = (float)posxRocket;						  RocketPolygonHitbox[3].y = posyRocket + ((rocket_size));
	RocketPolygonHitbox[4].x = (float)posxRocket - ((rocket_size) * 0.3); RocketPolygonHitbox[4].y = posyRocket + ((rocket_size) * 0.4);

	printf("Angle: %f\n", angleRotation);
	for (int i = 0; i < 5; i++) {
		newHit[i][0] = RocketPolygonHitbox[i].x - posxRocket;
		newHit[i][1] = RocketPolygonHitbox[i].y - posyRocket;

		newHit[i][0] = (newHit[i][0] * glm::cos(radians(angleRotation))) - (newHit[i][1] * glm::sin(radians(angleRotation)));
		newHit[i][1] = (newHit[i][0] * glm::sin(radians(angleRotation))) + (newHit[i][1] * glm::cos(radians(angleRotation)));

		newHit[i][0] = newHit[i][0] + posxRocket;
		newHit[i][1] = newHit[i][1] + posyRocket;
	}

}

void collisionDetection(float (*rocketHit)[2]) {
	
	bool collide = false;

	//first, check for checkpoint
	collide = circlePolygonCollision(posCheck[0], posCheck[1], 25.0, rocketHit, 5);

	if (collide) 
		win = true;
	
	else {
		for (int i = 0; i < 7 && !collide; i++) {
			collide = circlePolygonCollision(InfoPlanets[i].posx, InfoPlanets[i].posy, InfoPlanets[i].scale, rocketHit, 5);
		}

		if (collide)
			gameover = true;
	}

}


/*********************************************************************************************************/
/**************************************** TIMER UPDATE FUNCTIONS *****************************************/
/*********************************************************************************************************/

void update(int a)
{
	bool moving = false;

	float newRocketHitbox[5][2];
	calculateRocketHitbox(newRocketHitbox);

	collisionDetection(newRocketHitbox);

	if (gameover)
		printf("Gameover\n");
	else if (win) {
		printf("Win!\n");
	}

	if (pressing_left)
	{
		HorizontalVelocity -= 1;
		angleRotation += 1;
		moving = true;
	}
	if (pressing_right)
	{
		HorizontalVelocity += 1;
		angleRotation -= 1;
		moving = true;
	}
	if (pressing_up) {
		VerticalVelocity += 1;
		moving = true;
	}
	if (pressing_down) {
		VerticalVelocity -= 1;
		moving = true;
	}
	if (!moving) {  
		if (HorizontalVelocity > 0)
		{
			HorizontalVelocity -= 1;
			if (HorizontalVelocity < 0)
				HorizontalVelocity = 0;
		}
		if (HorizontalVelocity < 0)
		{
			HorizontalVelocity += 1;
			if (HorizontalVelocity > 0)
				HorizontalVelocity = 0;
		}
		if (VerticalVelocity > 0)
		{
			VerticalVelocity -= 1;
			if (VerticalVelocity < 0)
				VerticalVelocity = 0;
		}
		if (VerticalVelocity < 0)
		{
			VerticalVelocity += 1;
			if (VerticalVelocity > 0)
				VerticalVelocity = 0;
		}

		if (angleRotation > 0)
		{
			angleRotation -= 1;
			if (angleRotation < 0)
				angleRotation = 0;
		}
		if (angleRotation < 0)
		{
			angleRotation += 1;
			if (angleRotation > 0)
				angleRotation = 0;
		}

	}

	posxRocket += HorizontalVelocity;
	posyRocket += VerticalVelocity;
	//printf("Position: %d, %d\n", posxRocket, posyRocket);
	if (posxRocket < 0) {
		posxRocket = 0;
		HorizontalVelocity = -HorizontalVelocity * 0.5;
		angleRotation = 0.0;
	}
	if (posxRocket > width) {
		posxRocket = width;
		HorizontalVelocity = -HorizontalVelocity * 0.5;
		angleRotation = 0.0;
	}
	if (posyRocket < 0) {
		posyRocket = 0;
		VerticalVelocity = -VerticalVelocity * 0.5;
	}
	if (posyRocket > height) {
		posyRocket = height;
		VerticalVelocity = -VerticalVelocity * 0.5;
	}

	if (angleRotation < -20.0f) {
		angleRotation = -20.0f;
	}
	if (angleRotation > 20.0f) {
		angleRotation = 20.0f;
	}


	/*for (int i = 0; i < 5; i++) {
		printf("Hitbox x=%.2f , y=%.2f\n", newRocketHitbox[i][0], newRocketHitbox[i][1]);
	}*/

	glutPostRedisplay();
	
	if (!gameover && !win) {
		glutTimerFunc(24, update, 0);
	}

	
}

void updatePlanetMovement(int a) {

	//la posizione del pianeta aumenta di 1 ogni 50 ms, la velocità è costante (1/50 pixel/ms)
	for (int i = 0; i < currentPlanets; i++) {

		if (InfoPlanets[i].posx == positions[i][0] + delta || InfoPlanets[i].posx == positions[i][0] - delta) {
			InfoPlanets[i].direction = -1 * InfoPlanets[i].direction;
		}

		InfoPlanets[i].posx = InfoPlanets[i].posx + InfoPlanets[i].direction;
	}

	glutPostRedisplay();
	
	if (!gameover && !win) {
		glutTimerFunc(25, updatePlanetMovement, 0);
	}

}

void updateStarMovement(int a) {
	
	for (int i = 0; i < currentStars; i++) {

		if (InfoStars[i].size == 6 || InfoStars[i].size == 3) {
			//inverto la direzione di modifica dell'alfa
			InfoStars[i].dir = -1 * InfoStars[i].dir;
		}

		InfoStars[i].size = InfoStars[i].size + InfoStars[i].dir;
	}

	glutPostRedisplay();

	if (!gameover && !win) {
		glutTimerFunc(250, updateStarMovement, 0);
	}
}

/*********************************************************************************************************/
/***************************************** DRAW SCENE FUNCTION *******************************************/
/*********************************************************************************************************/

void drawScene(void)
{
	

	glUniformMatrix4fv(MatProj, 1, GL_FALSE, value_ptr(Projection));
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(programId);  // attiva fragment shader basic per tutta la scena meno i nemici 
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	//draw stars particles
	/*printf("%f , %f , %f , %f\n", Stars[0].r, Stars[0].g, Stars[0].b, Stars[0].a);*/
	glBindVertexArray(VAO_Stars);
	for (int i = 0; i < currentStars; i++) {
		Model = mat4(1.0);
		Model = translate(Model, vec3(0.0, 0.0, 0.0f));
		Model = scale(Model, vec3((float)width, (float)height, 1.0));
		/*Model = rotate(Model, radians(0.0f), vec3(0.0, 0.0, 1.0));*/
		glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
		glPointSize((float)InfoStars[i].size);
		glDrawArrays(GL_POINTS, i, 1);
	}


	//Drawing planets
	glBindVertexArray(VAO_Planets);
	for (int i = 0; i < currentPlanets; i++) {
		Model = mat4(1.0);
		Model = translate(Model, vec3((float)InfoPlanets[i].posx, (float)InfoPlanets[i].posy, 0.0f));
		Model = scale(Model, vec3((float)InfoPlanets[i].scale, (float)InfoPlanets[i].scale, 0.0));
		Model = rotate(Model, radians(0.0f), vec3(0.0, 0.0, 1.0));
		glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
		glDrawArrays(GL_TRIANGLES, planetVertices * i, planetVertices);
	}

	//Draw Checkpoint
	glBindVertexArray(VAO_Checkpoint);
	Model = mat4(1.0);
	Model = translate(Model, vec3((float)posCheck[0], (float)posCheck[1], 0.0f));
	Model = scale(Model, vec3(25.0, 25.0, 0.0));
	glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
	glDrawArrays(GL_TRIANGLES, 0, nRocketVertices);

	//Draw Rocket body
	glBindVertexArray(VAO_Rocket);
	Model = mat4(1.0);
	Model = translate(Model, vec3((float)posxRocket, (float)posyRocket, 0.0f));
	Model = scale(Model, vec3((float)rocket_size, (float)rocket_size, 0.0));
	Model = rotate(Model, radians(angleRotation + 180.0f), vec3(0.0, 0.0, 1.0));
	glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
	glDrawArrays(GL_TRIANGLES, 0, nRocketVertices);
	
	if (gameover || win) {
		
		glBindVertexArray(VAO_GameoverX);
		Model = mat4(1.0);
		Model = scale(Model, vec3((float)width, (float)height, 1.0));
		glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
		glDrawArrays(GL_TRIANGLES, 0, 6);

		Model = mat4(1.0);
		Model = translate(Model, vec3((float)width / 2, (float)height / 2, 0.0f));
		Model = scale(Model, vec3(100.0, 100.0, 0.0));
		Model = rotate(Model, radians(0.0f), vec3(0.0, 0.0, 1.0));
		glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));

		if(win){
			glBindVertexArray(VAO_WinningTick);			
			glDrawArrays(GL_TRIANGLES, 0, 12);
		}
		else if (gameover) {
			glLineWidth(50.0);
			glDrawArrays(GL_LINE_STRIP, 6, 2);
			glDrawArrays(GL_LINE_STRIP, 8, 2);
		}
	}

	glutSwapBuffers();

}

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);

	glutInitContextVersion(4, 0);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);

	srand(time(NULL));

	glutInitWindowSize(width, height);
	glutInitWindowPosition(50, 50);
	glutCreateWindow("2D Rocket-Planets Game");
	glutDisplayFunc(drawScene);

	glutKeyboardFunc(keyboardPressedEvent);
	//Evento tastiera tasto rilasciato (per fermare lo spostamento a dx e sx mantenendo il rimbalzo sul posto)
	glutKeyboardUpFunc(keyboardReleasedEvent);

	glutTimerFunc(66, update, 0);
	glutTimerFunc(66, updatePlanetMovement, 0);
	glutTimerFunc(66, updateStarMovement, 0);
	glewExperimental = GL_TRUE;
	glewInit();

	initShader();
	init();

	glEnable(GL_BLEND);
	glEnable(GL_ALPHA_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glutMainLoop();
}

