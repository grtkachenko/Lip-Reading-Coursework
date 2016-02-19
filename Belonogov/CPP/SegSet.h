#include "main.h"

#ifndef SEGSET_H
#define SEGSET_H

using namespace std;

const int INF = 1e9;

struct SegSet {
    std::vector < std::pair < double, double > > data;

    void add(double l, double r) ;
    SegSet operator | (const SegSet & other) const;
    SegSet operator & (const SegSet & other) const;
    SegSet operator ~ () const;
    void print();
    void check();
    void clear();
    double getLen();
};



#endif
