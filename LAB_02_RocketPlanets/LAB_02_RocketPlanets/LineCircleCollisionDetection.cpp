#include <cmath>
#include <stdio.h>
#include "LineCircleCollisionDetection.h"

//collisione tra poligono che rappresenta l'hitbox e il cerchio 
bool circlePolygonCollision(float cx, float cy, float cr, float(*polygonPoints)[2], float nPoints) {

	int i;
	bool collision = false;

	//prima di tutto, controllo che uno dei vertici non collidaccon il cerchio (distanza minore o uguale al raggio)
	for (i = 0; i < nPoints && !collision; i++) {
		if (circlePointCollision(cx, cy, cr, polygonPoints[i])) {
			collision = true;
		}
	}

	//la collisione del poligono può essere semplificata con la collisione dei segmenti che costituiscono i suoi lati
	//quindi, per ogni lato effettua la circleSegmentCollision
	if (!collision) {
		for (i = 0; i < nPoints && !collision; i++) {

			float* pointA = polygonPoints[i];
			float* pointB;

			if (i == nPoints - 1)
				pointB = polygonPoints[0];
			else
				pointB = polygonPoints[i + 1];

			if (circleSegmentCollision(cx, cy, cr, pointA, pointB)) {
				collision = true;
			}
		}
	}

	return collision;
}

//collisione tra punto e cerchio 
bool circlePointCollision(float cx, float cy, float cr, float* point) {

	float c[2] = { cx, cy };
	
	float distance = dist(c, point);

	if ((float)distance < cr)
		return true;
	else
		return false;

}

//collisione tra segmento e cerchio: si sfrutta il prodotto scalare per calcolare la distanza PC tra il centro del cerchio
//e il segmento, si controlla che il punto P appartenga al segmento e si verifica che tale distanza sia minore del raggio
bool circleSegmentCollision(float cx, float cy, float cr, float* pointA, float* pointB) {

	float segmentLen = dist(pointA, pointB);

	float dot = ( ( (cx - pointA[0])*(pointB[0] - pointA[0]) ) + ( (cy - pointA[1])*(pointB[1] - pointA[1]) ) ) / pow(segmentLen, 2);

	float closestPoint[2];
	closestPoint[0] = pointA[0] + (dot * (pointB[0] - pointA[0]));
	closestPoint[1] = pointA[1] + (dot * (pointB[1] - pointA[1]));


	if (pointOnSegment(pointA, pointB, closestPoint)) {

		float c[2] = { cx, cy };
		float distance = dist(c, closestPoint);


		if ((float)distance < cr)
			return true;
		else
			return false;
	}
	else {
		return false;
	}

}

//per verificare se un punto appartiene al segmento
bool pointOnSegment(float* a, float* b, float* p) {

	float d1 = dist(p, a);
	float d2 = dist(p, b);

	float lineLen = dist(a, b);

	float buffer = 0.00;

	if (d1 + d2 >= lineLen - buffer && d1 + d2 <= lineLen + buffer)
		return true;
	else
		return false;
	
}


//distanza tra due punti
float dist(float* p1, float* p2) {

	return sqrt(pow((p1[0] - p2[0]), 2) + pow((p1[1] - p2[1]), 2));

}









