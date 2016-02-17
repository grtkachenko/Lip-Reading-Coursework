//
// Created by vanya on 13.12.15.
//

#include "SegSet.h"


void SegSet::add(double l, double r) {
    data.pb(mp(l, r));
}

void SegSet::check() {
    for (auto x: data)
        assert(x.fr <= x.sc);
    for (int i = 0; i < (int) data.size() - 1; i++)
        assert(data[i].sc <= data[i + 1].fr);
}

SegSet SegSet::operator|(const SegSet &other) const {
    vector<pair<double, int> > event;
    for (auto x: data) {
        event.pb(mp(x.fr, 1));
        event.pb(mp(x.sc, -1));
    }
    for (auto x: other.data) {
        event.pb(mp(x.fr, 1));
        event.pb(mp(x.sc, -1));
    }
    sort(event.begin(), event.end());
    int cnt = 0;
    double last = -1;
    SegSet res;
    for (auto x: event) {
        cnt += x.sc;
        if (x.sc == 1 && cnt == 1) {
            last = x.fr;
        }
        if (x.sc == -1 && cnt == 0) {
            res.add(last, x.fr);
        }
    }
    assert(cnt == 0);
    return res;
}

SegSet SegSet::operator&(const SegSet &other) const {
    vector<pair<double, int> > event;
    for (auto x: data) {
        event.pb(mp(x.fr, 1));
        event.pb(mp(x.sc, -1));
    }
    for (auto x: other.data) {
        event.pb(mp(x.fr, 1));
        event.pb(mp(x.sc, -1));
    }
    sort(event.begin(), event.end());
    int cnt = 0;
    double last = -1;
    SegSet res;
    for (auto x: event) {
        cnt += x.sc;
        assert(cnt <= 2);
        if (x.sc == 1 && cnt == 2) {
            last = x.fr;
        }
        if (x.sc == -1 && cnt == 1) {
            res.add(last, x.fr);
        }
    }
    assert(cnt == 0);
    return res;
}

SegSet SegSet::operator~() const {
    SegSet res;
    double last = -INF;
    for (int i = 0; i < (int) data.size(); i++) {
        res.add(last, data[i].fr);
        last = data[i].sc;
    }
    res.add(last, INF);
    return res;
}

void SegSet::print() {
    cerr.precision(8);
    cerr << fixed;
    for (int i = 0; i < (int) data.size(); i++) {
        cerr << "[" << data[i].fr << ", " << data[i].sc << "]";
        if (i + 1 == (int) data.size())
            cerr << endl;
        else
            cerr << ", ";
    }
}

void SegSet::clear() {
    data.clear();
}

double SegSet::getLen() {
    double res = 0;
    for (auto x: data)
        res += x.sc - x.fr;
    return res;
}


