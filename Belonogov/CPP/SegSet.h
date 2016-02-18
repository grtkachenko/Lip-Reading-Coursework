//
// Created by vanya on 13.12.15.
//

#ifndef SEGSET_H
#define SEGSET_H


#include <iostream>
#include <vector>
#include <algorithm>

#undef NDEBUG
#include <cassert>

using namespace std;

#define pb push_back
#define mp make_pair
#define fr first
#define sc second

const int INF = 1e9;

struct SegSet {
    vector < pair < double, double > > data;

    void add(double l, double r) ;
    SegSet operator | (const SegSet & other) const;
    SegSet operator & (const SegSet & other) const;
    SegSet operator ~ () const;
    void print();
    void check();
    void clear();
    double getLen();
};



#endif //EXAMPLES_SEGSET_H
