#include <cmath>
#include <stdio.h>
#include "LineCircleCollisionDetection.h"

bool circlePolygonCollision(float cx, float cy, float cr, float(*polygonPoints)[2], float nPoints) {

	int i;
	bool collision = false;

	//first check if point is touching/inside the circle
	for (i = 0; i < nPoints && !collision; i++) {
		if (circlePointCollision(cx, cy, cr, polygonPoints[i])) {
			collision = true;
		}
	}

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


bool circlePointCollision(float cx, float cy, float cr, float* point) {

	float c[2] = { cx, cy };
	
	float distance = dist(c, point);

	if ((float)distance < cr)
		return true;
	else
		return false;

}


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


float dist(float* p1, float* p2) {

	return sqrt(pow((p1[0] - p2[0]), 2) + pow((p1[1] - p2[1]), 2));

}









