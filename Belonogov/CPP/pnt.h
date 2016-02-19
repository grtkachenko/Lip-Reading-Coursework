#include "main.h"

#ifndef PNT_H
#define PNT_H

using namespace std;

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
    Lips ();
    Lips (std::vector < pnt > lips);
    void normalize();
    void setScale(double k);
    void shift(pnt v);
    vector < std::pair < pnt, pnt > > getContour ();
    vector < pnt > data();
    double getDist(Lips other);
};

struct Average {
    double sum;
    int cnt;
    Average();
    void add(double x);
    double average();
};

#endif

