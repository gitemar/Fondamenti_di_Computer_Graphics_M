/*
 * This program draws curves starting from dots placed by the user.
 * It allows to use both classic de Casteljau/Bézier curves and 
 * adaptive subdivision just by commenting/uncommentig the
 * the needed code in drawScene() method (from line 505).
 *
 * Usage:
 *   Left click to place a control point.
 *		Maximum number of control points allowed is currently set at 64.
 *	 Press "f" to remove the first control point
 *	 Press "l" to remove the last control point.
 *	 Press escape to exit.
 */


#include <iostream>
#include "ShaderMaker.h"
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <vector>

 // Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

static unsigned int programId;

//for the control points polygon
unsigned int VAO;
unsigned int VBO;

//for the calculated curve
unsigned int VAO_curve;
unsigned int VBO_curve;

using namespace glm;

#define MaxNumPts 64
#define MaxNumPtsCurve 128
#define increment 0.01


/*************************************************************************************************
*											Variables											 *
*************************************************************************************************/
float PolygonPointsArray[MaxNumPts][3];
float CurvePointsArray[MaxNumPtsCurve][3];
int NumPolygonPts = 0;
int NumCurvePts = 0;

int selectedPointIndex = -1;
bool isMovingPoint = false;

// Window size in pixels
int		width = 500;
int		height = 500;

/**************** for classic algorithm *******************/
float currentCurvePoint[3];

/*************** for adaptive subdivision *****************/
int NumSegments = 0;
float controlPoints[MaxNumPts * 2][3];
int NumCtrlPts = 0;


/*************************************************************************************************
*											Prototypes											 *
*************************************************************************************************/
void addNewPoint(float x, float y);
void removeFirstPoint();
void removeLastPoint();

void deCasteljauAlgorithm(float pi[][3], float t, int nump);

/**************** for classic algorithm *******************/
void noSubdivisionCurveArray();

/*************** for adaptive subdivision *****************/
void adaptiveSubMethod(float tempPointArray[][3], int numPoints);
void getSubArrays(float initialControlPoints[][3], float(*sub1)[3], float(*sub2)[3], int numPoints);
float calc2DDistance(float pointA[3], float pointB[3], float pointP[3]);


/*************************************************************************************************
*										Definitions												 *
*************************************************************************************************/

/* calcolo della distanza di un punto P da una retta passante per i punti A e B */
float calc2DDistance(float pointA[2], float pointB[2], float pointP[2]) {
	float dist, m, q;
	float xA = pointA[0], yA = pointA[1], xB = pointB[0], yB = pointB[1];
	float xP = pointP[0], yP = pointP[1];

	//we are using explicit formula
	//y = xm + q
	//where m = (yB - yA)/(xB - xA)
	//and q = yA - (m*xA)

	m = (yB - yA) / (xB - xA);
	q = yA - (m * xA);

	//if point p is (xP, yP)
	//the distance will be d = abs(yP - ((m*xP) + q))/sqrt(1+(m*m))

	dist = abs(yP - ((m * xP) + q)) / sqrt(1 + (m * m));

	return dist;
}

/*/*
* Punto 3. : "Disegnare la curva di Bézier a partire dai punti di controllo
* inseriti, utilizzando l’algoritmo di de Casteljau."
*
* Il calcolo della curva è effettuato tramite incremento di t, con intervallo
* definito da "increments". Per ogni t, applico l'algoritmo di de Casteljau e
* ottengo il punto corrispondente, salvato nell'array currentCurvePoint[] */
void noSubdivisionCurveArray() {

	float t = 0.00f;

	NumCurvePts = 0;

	while (t <= 1.00f) {
		deCasteljauAlgorithm(PolygonPointsArray, t, NumPolygonPts);

		CurvePointsArray[NumCurvePts][0] = currentCurvePoint[0];
		CurvePointsArray[NumCurvePts][1] = currentCurvePoint[1];
		CurvePointsArray[NumCurvePts][2] = currentCurvePoint[2];
		
		printf("For t= %.2f - Point %d: (%f,%f,%f)\n", t, NumCurvePts+1, CurvePointsArray[NumCurvePts][0], CurvePointsArray[NumCurvePts][1], CurvePointsArray[NumCurvePts][2]);
		
		NumCurvePts++;
		t = t + increment;
	}
}


/*Calcolo della curva tramite suddivisione adattiva*/
void adaptiveSubMethod(float tempPointArray[][3], int numPoints) {

	bool plane = true;
	float threshold = 0.005f;

	float pA[2] = { tempPointArray[0][0], tempPointArray[0][1] };
	float pB[2] = { tempPointArray[numPoints - 1][0], tempPointArray[numPoints - 1][1] };
	float pP[2];
	float d;

	//la retta è calcolata quando si calcola la distanza nella funzione calc2DDistance()
	//test di planarità su controlpoint interni
	int i = 1;
	while (i < numPoints - 1 && plane) {
		pP[0] = tempPointArray[i][0];
		pP[1] = tempPointArray[i][1];

		d = calc2DDistance(pA, pB, pP);

		if (d > threshold) plane = false;

		i++;
	}

	//se è plane (ovvero tutti i punti di controllo hanno una distanza inferiore alla threshold)
	if (plane) {

		//i punti di controllo interni possono non essere inclusi nell'insieme dei punti finali
		//includo i punti di controllo esterni nell'elenco dei punti della curva di cui andrò a disegnare i segmenti successivamente
		CurvePointsArray[NumCurvePts][0] = pA[0];
		CurvePointsArray[NumCurvePts][1] = pA[1];
		CurvePointsArray[NumCurvePts][2] = 0.0f;
		NumCurvePts++;

		CurvePointsArray[NumCurvePts][0] = pB[0];
		CurvePointsArray[NumCurvePts][1] = pB[1];
		CurvePointsArray[NumCurvePts][2] = 0.0f;
		NumCurvePts++;

		NumSegments++;
	}
	//se non è plane, ovvere almeno un punto ha una distanza dalla retta superiore a threshold
	else {

		float sub1[MaxNumPts][3];
		float sub2[MaxNumPts][3];

		//calcolo dei due sub-array (sfruttando l'algoritmo di de Casteljau - vedere definizione 
		//della funzione getSubArrays()) che verranno poi memorizzati in sub1[] e sub2[]
		getSubArrays(tempPointArray, sub1, sub2, numPoints);

		//chiamo ricorsivamente
		adaptiveSubMethod(sub1, numPoints);
		adaptiveSubMethod(sub2, numPoints);
	}

}

void getSubArrays(float initialControlPoints[][3], float(*sub1)[3], float(*sub2)[3], int numPoints) {

	//i due subarray vengono calcolati con l'algoritmo di de Casteljau ponendo t a 0.5
	float t = 0.5f;

	deCasteljauAlgorithm(initialControlPoints, t, numPoints);


	//una volta ottenuto l'intero array di control points da deCasteljauAlgorithm(), lo si
	//divide ponendo la prima metà dei punti in sub1 e la seconda metà in sub2
	for (int i = 0; i < NumCtrlPts; i++) {

		if (i < numPoints - 1) {
			sub1[i][0] = controlPoints[i][0];
			sub1[i][1] = controlPoints[i][1];
			sub1[i][2] = controlPoints[i][2];
		}
		else if (i == numPoints - 1) {
			sub1[i][0] = controlPoints[i][0];
			sub1[i][1] = controlPoints[i][1];
			sub1[i][2] = controlPoints[i][2];

			sub2[i - (numPoints - 1)][0] = controlPoints[i][0];
			sub2[i - (numPoints - 1)][1] = controlPoints[i][1];
			sub2[i - (numPoints - 1)][2] = controlPoints[i][2];
		}
		else {
			sub2[i - (numPoints - 1)][0] = controlPoints[i][0];
			sub2[i - (numPoints - 1)][1] = controlPoints[i][1];
			sub2[i - (numPoints - 1)][2] = controlPoints[i][2];
		}
	}


}

/*Algoritmo di de Casteljau che contiene sia la logica per l'algoritmp classico
* sia la logica per la suddivisione adattiva */
void deCasteljauAlgorithm(float pi[][3], float t, int nump) {

	float deCasteljauArray[MaxNumPts][3];

	/************* local variables for subdivision **************/
	float allControlPointsX[MaxNumPts][MaxNumPts];
	float allControlPointsY[MaxNumPts][MaxNumPts];
	int numberOfControlPts[MaxNumPts];

	for (int i = 0; i < nump; i++) {
		deCasteljauArray[i][0] = pi[i][0];
		deCasteljauArray[i][1] = pi[i][1];
		deCasteljauArray[i][2] = pi[i][2];
	}

	for (int i = 1; i < nump; i++) {
		numberOfControlPts[i - 1] = nump - i;

		for (int j = 0; j < nump - i; j++) {
			deCasteljauArray[j][0] = (1 - t) * deCasteljauArray[j][0] + (t)*deCasteljauArray[j + 1][0];
			deCasteljauArray[j][1] = (1 - t) * deCasteljauArray[j][1] + (t)*deCasteljauArray[j + 1][1];
			deCasteljauArray[j][2] = (1 - t) * deCasteljauArray[j][2] + (t)*deCasteljauArray[j + 1][2];

			/************* saving all control points for subdivision method **************/
			allControlPointsX[i - 1][j] = deCasteljauArray[j][0];
			allControlPointsY[i - 1][j] = deCasteljauArray[j][1];

		}
	}

	/*******for classic algorithm*******************/
	/*---------------------------------------------*/

	currentCurvePoint[0] = deCasteljauArray[0][0];
	currentCurvePoint[1] = deCasteljauArray[0][1];
	currentCurvePoint[2] = deCasteljauArray[0][2];

	/*******for adaptive subdivision****************/
	/*---------------------------------------------*/

	int nround = nump - 1;
	NumCtrlPts = 0;

	//memorizzo i control point che potrebbero servire in futuro
	controlPoints[0][0] = pi[0][0];
	controlPoints[0][1] = pi[0][1];
	controlPoints[0][2] = 0.0f;
	NumCtrlPts++;

	//first half
	for (int i = 0; i < nround; i++) {

		controlPoints[NumCtrlPts][0] = allControlPointsX[i][0];
		controlPoints[NumCtrlPts][1] = allControlPointsY[i][0];
		controlPoints[NumCtrlPts][2] = 0.0f;
		NumCtrlPts++;
	}

	//second half
	for (int i = nround - 2; i >= 0; i--) {
		int last_element = numberOfControlPts[i] - 1;
		controlPoints[NumCtrlPts][0] = allControlPointsX[i][last_element];
		controlPoints[NumCtrlPts][1] = allControlPointsY[i][last_element];
		controlPoints[NumCtrlPts][2] = 0.0f;
		NumCtrlPts++;

	}

	controlPoints[NumCtrlPts][0] = pi[nump - 1][0];
	controlPoints[NumCtrlPts][1] = pi[nump - 1][1];
	controlPoints[NumCtrlPts][2] = 0.0f;
	NumCtrlPts++;

}

/*
* Punto 1. : "I comandi ’f’ e ’l’ rimuovono il primo e l’ultimo
* punto dalla lista di punti, rispettivamente."
*/
void myKeyboardFunc(unsigned char key, int x, int y)
{
	switch (key) {
	case 'f':
		removeFirstPoint();
		glutPostRedisplay();
		break;
	case 'l':
		removeLastPoint();
		glutPostRedisplay();
		break;
	case 27:			// Escape key
		exit(0);
		break;
	}
}

void removeFirstPoint() {
	int i;
	if (NumPolygonPts > 0) {
		// Remove the first point, slide the rest down
		NumPolygonPts--;
		for (i = 0; i < NumPolygonPts; i++) {
			PolygonPointsArray[i][0] = PolygonPointsArray[i + 1][0];
			PolygonPointsArray[i][1] = PolygonPointsArray[i + 1][1];
			PolygonPointsArray[i][2] = 0;
		}
	}
}
void resizeWindow(int w, int h)
{
	height = (h > 1) ? h : 2;
	width = (w > 1) ? w : 2;
	gluOrtho2D(-1.0f, 1.0f, -1.0f, 1.0f);
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
}

/*
* Punto 1. : "Provare i controlli da keyboard. Il left mouse button aggiunge un punto. [...]
* Oltre ai 64 punti, i primi punti sono rimossi."
*/
void myMouseFunc(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		if (selectedPointIndex > -1) {
			isMovingPoint = true;
		}
		else {
			// (x,y) viewport(0,width)x(0,height)   -->   (xPos,yPos) window(-1,1)x(-1,1)
			float xPos = -1.0f + ((float)x) * 2 / ((float)(width));
			float yPos = -1.0f + ((float)(height - y)) * 2 / ((float)(height));

			printf("new point Xpos %f Ypos %f \n", xPos, yPos);
			printf("new pixel x %d y %d \n", x, y);
			addNewPoint(xPos, yPos);
		}

		glutPostRedisplay();
	}
	else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
	{
		if (isMovingPoint) {
			isMovingPoint = false;
		}
	}
}

// Add a new point to the end of the list.  
// Remove the first point in the list if too many points.
void removeLastPoint() {
	if (NumPolygonPts > 0) {
		NumPolygonPts--;
	}
}

// Add a new point to the end of the list.  
// Remove the first point in the list if too many points.
void addNewPoint(float x, float y) {
	if (NumPolygonPts >= MaxNumPts) {
		removeFirstPoint();
	}
	PolygonPointsArray[NumPolygonPts][0] = x;
	PolygonPointsArray[NumPolygonPts][1] = y;
	PolygonPointsArray[NumPolygonPts][2] = 0;
	NumPolygonPts++;
}

/*Il mouse è considerato "vicino" ad un punto se la distanza tra la posizione del mouse
* e il punto di indice i è minore di una data soglia (0.035)*/
void checkIfNearPoint(int x, int y) {
	// (x,y) viewport(0,width)x(0,height)   -->   (xPos,yPos) window(-1,1)x(-1,1)
	float xPos = -1.0f + ((float)x) * 2 / ((float)(width));
	float yPos = -1.0f + ((float)(height - y)) * 2 / ((float)(height));

	for (int i = 0; i < NumPolygonPts; i++) {
		float dx = xPos - PolygonPointsArray[i][0];
		float dy = yPos - PolygonPointsArray[i][1];
		float dist = sqrt(pow(dx, 2) + pow(dy, 2));

		if (dist < 0.035) {
			selectedPointIndex = i;
			glutPostRedisplay();
			return;
		}
		else {
			//we are not near any point
			selectedPointIndex = -1;
		}

	}
}

void dragPoint(int x, int y) {

	if (isMovingPoint) {
		float newX = -1.0f + ((float)x) * 2 / ((float)(width));
		float newY = -1.0f + ((float)(height - y)) * 2 / ((float)(height));

		PolygonPointsArray[selectedPointIndex][0] = newX;
		PolygonPointsArray[selectedPointIndex][1] = newY;

		glutPostRedisplay();
	}
}

void initShader(void)
{
	GLenum ErrorCheckValue = glGetError();

	char* vertexShader = (char*)"vertexShader.glsl";
	char* fragmentShader = (char*)"fragmentShader.glsl";

	programId = ShaderMaker::createProgram(vertexShader, fragmentShader);
	glUseProgram(programId);

}

void init(void)
{
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);


	glGenVertexArrays(1, &VAO_curve);
	glBindVertexArray(VAO_curve);

	glGenBuffers(1, &VBO_curve);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_curve);


	// Background color
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glViewport(0, 0, width, height);

}

void drawScene(void)
{
	glClear(GL_COLOR_BUFFER_BIT);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(PolygonPointsArray), &PolygonPointsArray[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Draw the line segments
	//glBindVertexArray(VAO);
	glLineWidth(2.5);
	glDrawArrays(GL_LINE_STRIP, 0, NumPolygonPts);

	// Draw the points
	//glBindVertexArray(VAO);
	glPointSize(8.0);
	glDrawArrays(GL_POINTS, 0, NumPolygonPts);


	//dobbiamo aggiungere il disegno della curva: di fatto altro non è che una poligonale molto più densa di quella originale.
	//devo utilizzare un altro vao e vbo dato che il numero di punti è diverso e andrei a fare un'oggetto diverso
	//come per sierpinsky che avevo due oggetti diversi per frame esterno e triangolo.
	if (NumPolygonPts > 1) {

		//classic algorithm
		/*noSubdivisionCurveArray();*/

		//for adaptive subdivision
		NumCurvePts = 0;
		NumSegments = 0;

		adaptiveSubMethod(PolygonPointsArray, NumPolygonPts);
		printf("Adaptive subdivision methos found: %d segments\n", NumSegments);

		glBindVertexArray(VAO_curve);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_curve);
		glBufferData(GL_ARRAY_BUFFER, sizeof(CurvePointsArray), &CurvePointsArray[0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glLineWidth(1.5);
		glDrawArrays(GL_LINE_STRIP, 0, NumCurvePts);

	}

	glutSwapBuffers();
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);

	glutInitContextVersion(4, 0);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);

	glutInitWindowSize(width, height);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Draw curves 2D");

	glutDisplayFunc(drawScene);
	glutReshapeFunc(resizeWindow);

	/*
	* Punto 2. : l programma usa le OpenGL GLUT callback per catturare gli eventi
	* click del mouse e determinare le posizioni (x, y) relative
	*/
	glutKeyboardFunc(myKeyboardFunc);
	glutMouseFunc(myMouseFunc);

	/*
	* Punto 5. : "Permettere la modifica della posizione dei punti di controllo
	* tramite trascinamento con il mouse."
	*
	* Per il trascinamento dei punti devo sfruttare le funzioni che mi rilevano
	* il movimento del mouse come eventi: glutPassiveMotionFunc() e glutMotionFunc()
	*/
	glutPassiveMotionFunc(checkIfNearPoint);
	glutMotionFunc(dragPoint);


	glewExperimental = GL_TRUE;
	glewInit();

	initShader();
	init();

	glutMainLoop();
}

