#include "anomaly_detection_util.h"
#include <cmath>

float avg(float *x, int size) {
    float sum = 0;
    for (int i = 0; i < size; i++) {
        sum += x[i];
    }
    return sum / size;
}

// returns the variance of X and Y
float var(float *x, int size) {
    //calculate E[X^2]
    float powerTwoArray[size];
    for (int i = 0; i < size; i++) {
        powerTwoArray[i] = pow(x[i], 2);
    }
    float Ex1 = avg(powerTwoArray, size);
    //calculate E[x]^2
    float Ex2 = pow(avg(x, size), 2);
    //calculate var
    return Ex1 - Ex2;
}

// returns the covariance of X and Y
float cov(float *x, float *y, int size) {
    //calculate E[XY]
    float XYArray[size];
    for (int i = 0; i < size; i++) {
        XYArray[i] = x[i] * y[i];
    }
    float Exy = avg(XYArray, size);
    //calculate E[X]
    float Ex = avg(x, size);
    //calculate E[Y]
    float Ey = avg(y, size);
    // COV(X,Y) = E[XY] - (E[X]E[Y])
    return Exy - (Ex * Ey);
}


// returns the Pearson correlation coefficient of X and Y
float pearson(float *x, float *y, int size) {
    //calculate cov(x,y)
    float COVxy = cov(x, y, size);
    //calculate Tx
    float Tx = sqrt(var(x, size));
    //calculate Ty
    float Ty = sqrt(var(y, size));
    // cov(x,y) / TxTy
    return COVxy / (Tx * Ty);
    //supose to be error in case divide by zero
}

// performs a linear regression and returns the line equation
Line linear_reg(Point **points, int size) {
    //create x and y arrays
    float x[size], y[size];
    for (int i = 0; i < size; i++) {
        x[i] = points[i]->x;
        y[i] = points[i]->y;
    }
    //calculate a = cov(x,y)/var(x)
    float a = cov(x, y, size) / var(x, size);
    //(supose to be error in case divide by zero)
    //calculate b = y'-ax'
    float b = avg(y, size) - (a * avg(x, size));
    //final
    return Line(a, b);
}

// returns the deviation between point p and the line equation of the points
float dev(Point p, Point **points, int size) {
    //find line
    Line l = linear_reg(points, size);
    //find devation
    return dev(p, l);
}

// returns the deviation between point p and the line
float dev(Point p, Line l) {
    //find y to calculate devation
    float y = l.f(p.x);
    // calculate deation
    return std::abs(p.y - y);
}




