// 20919279 31253197

#ifndef MINCIRCLE_H_
#define MINCIRCLE_H_

#include <iostream>
#include <vector>
#include "anomaly_detection_util.h"

using namespace std;

class Circle {
public:
    Point center;
    float radius;
    //Line():a(0),b(0){};
    Circle() : center(Point(0,0)), radius(0) {}
    Circle(Point c, float r) : center(c), radius(r) {}
};

Circle welzl_algorithm(Point **points, vector<Point> R, int number_of_points);
Circle findMinCircle(Point **points, size_t size);

// distance between two points
double distance(const Point &a, const Point &b);

// check if one point is in circle
bool pointInCircle(const Circle &c, const Point &p);

// check if all given points in circle
bool allPointsInCircle(const Circle &c, const vector<Point> &P);

Point middlePoint(const Point &p1, const Point &p2);

// create circle from 2 points
Circle createCircleFromTwoPoints(const Point &p1, const Point &p2);

// finds center of circle by math equations
Point getCircleCenterFromThreePoints(const Point &p1, const Point &p2, const Point &p3);

// create circle from 3 points
Circle createCircleFromThreePoints(const Point &p1, const Point &p2, const Point &p3);

// circle by 3 or less points
Circle trivial_circle(vector<Point> &R);


#endif /* MINCIRCLE_H_ */
