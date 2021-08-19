#pragma once

bool circlePolygonCollision(float cx, float cy, float cr, float(*polygonPoints)[2], float nPoints);
bool circlePointCollision(float cx, float cy, float cr, float* point);
bool circleSegmentCollision(float cx, float cy, float cr, float* pointA, float* pointB);
bool pointOnSegment(float* a, float* b, float* p);
float dist(float* p1, float* p2);