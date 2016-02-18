#include "main.h"

#ifndef PNT_H
#define PNT_H

struct pnt {
    double x, y;
    pnt (); 
    pnt (double x, double y);
    pnt (cv::Point A);
    pnt operator + (pnt A);
    pnt operator - (pnt A) ;
    pnt operator * (double k);
    pnt operator / (double k);
    double len();
    pnt rotate(double ang); 
    cv::Point getCVPoint();
};

struct Lips {
    std::vector < pnt > lips;
    Lips (std::vector < pnt > lips);
    void normalize();
    void setScale(double k);
    void shift(pnt v);
    std::vector < std::pair < pnt, pnt > > getContour ();
    std::vector < pnt > getData();
};

#endif

