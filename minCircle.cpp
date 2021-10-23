#include "minCircle.h"
#include <cmath>

// implement
Circle findMinCircle(Point **points, size_t size) {
    vector<Point> R;
    Circle circle = welzl_algorithm(points, R, size);
    return circle;
}

// you may add helper functions here

// distance between two points
double distance(const Point &a, const Point &b) {
    return sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}

// check if one point is in circle
bool pointInCircle(const Circle &c, const Point &p) {
    if (distance(c.center, p) <= c.radius)
        return true;
    return false;
}

// check if all given points in circle
bool allPointsInCircle(const Circle &c, const vector<Point> &P) {
    for (const Point &p : P)
        if (!pointInCircle(c, p))
            return false;
    return true;
}

Point middlePoint(const Point &p1, const Point &p2) {
    float x = (p1.x + p2.x) / 2.0;
    float y = (p1.y + p2.y) / 2.0;
    return Point(x, y);
}

// create circle from 2 points
Circle createCircleFromTwoPoints(const Point &p1, const Point &p2) {
    // the center is the middle
    Point center = middlePoint(p1, p2);
    // the radius is half of distance
    float radius = distance(p1, p2) / 2.0;
    return Circle(center, radius);
}

// finds center of circle by math equations
Point getCircleCenterFromThreePoints(const Point &p1, const Point &p2, const Point &p3) {
    float bx = p2.x - p1.x;
    float by = p2.y - p1.y;
    float cx = p3.x - p1.x;
    float cy = p3.y - p1.y;

    double B = bx * bx + by * by;
    double C = cx * cx + cy * cy;
    double D = bx * cy - by * cx;

    Point center((cy * B - by * C) / (2 * D), (bx * C - cx * B) / (2 * D));
    center.x += p1.x;
    center.y += p1.y;
    return center;
}

// create circle from 3 points
Circle createCircleFromThreePoints(const Point &p1, const Point &p2, const Point &p3) {
    Point center = getCircleCenterFromThreePoints(p1, p2, p3);
    float radius = distance(center, p1);
    return Circle(center, radius);
}

// circle by 3 or less points
Circle trivial_circle(vector<Point> &R) {
    //assert(R.size() <= 3);
    if (R.empty()) {
        return Circle(Point(0, 0), 0);
    } else if (R.size() == 1) {
        return Circle(R[0], 0);
    } else if (R.size() == 2) {
        return createCircleFromTwoPoints(R[0], R[1]);
    }

    // if the circle can be determined by 2 points
    for (int i = 0; i < 3; i++) {
        for (int j = i + 1; j < 3; j++) {
            Circle c = createCircleFromTwoPoints(R[i], R[j]);
            if (allPointsInCircle(c, R))
                return c;
        }
    }
    return createCircleFromThreePoints(R[0], R[1], R[2]);
}

Circle welzl_algorithm(Point **points, vector<Point> R, int number_of_points) {
    // Base case: all points processed or |R| = 3
    if (number_of_points == 0 || R.size() == 3) {
        return trivial_circle(R);
    }

    // Pick the first point
    Point p = *points[0];
    points++;

    // get the circle out of n-1 points remain
    Circle d = welzl_algorithm(points, R, number_of_points - 1);

    // If d contains p, return d
    if (pointInCircle(d, p)) {
        return d;
    }

    // Otherwise, must be on the boundary of the MEC
    R.push_back(p);

    // Return the MEC for P - {p} and R U {p}
    return welzl_algorithm(points, R, number_of_points - 1);
}
