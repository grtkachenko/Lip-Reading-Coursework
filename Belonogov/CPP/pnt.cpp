#include "pnt.h"

pnt::pnt () {}

pnt::pnt (double x, double y): x(x), y(y) {}

pnt::pnt(cv::Point other): x(other.x), y(other.y) { }

pnt pnt::operator + (pnt A) {
    return pnt(x + A.x, y + A.y);
}

pnt pnt::operator - (pnt A) {
    return pnt(x - A.x, y - A.y);
}

pnt pnt::operator * (double k) {
    return pnt(x * k, y * k);
}

pnt pnt::operator / (double k) {
    return pnt(x / k, y / k);
}

double pnt::len() {
    return sqrt(x * x + y * y);
}

pnt pnt::rotate(double ang) {
    return pnt(x * cos(ang) - y * sin(ang), x * sin(ang) + y * cos(ang));
}

cv::Point pnt::getCVPoint() {
    return cv::Point(x, y);
}

void pnt::Epr() {
    cerr << "x y: " << x << " " << y << endl;
}

Lips::Lips(std::vector < pnt > lips): lips(lips) { }
Lips::Lips() { }

void Lips::normalize() {
    pnt Mid = (lips[0] + lips[6]) / 2;
    for (auto &p: lips)
        p = p - Mid;
    double len = (lips[0] - lips[6]).len() / 2;
    assert(equal(lips[0].len(), lips[6].len()));
    for (auto &p: lips)
        p = p / len;
    double ang = atan2(lips[6].y, lips[6].x);
    for (auto &p: lips)
        p = p.rotate(-ang);
        assert(equal(0, atan2(lips[6].y, lips[6].x)));
}

void Lips::setScale(double k) {
    double oldValue = (lips[0] - lips[6]).len() / 2;
    //db(oldValue);
    //assert(equal(oldValue, 1));
    k = k / oldValue;
    for (auto &p: lips)
        p = p * k;
}

double Lips::getScale() {
    return (lips[0] - lips[6]).len() / 2;
};

void Lips::shift(pnt v) {
    for (auto &p: lips)
        p = p + v;
}

void Lips::print() {
    for (auto p: lips)
        cerr << "(" << p.x << ", " << p.y << ")  ";
    cerr << endl;
}

std::vector < std::pair < pnt, pnt > > Lips::getContour() {
    std::vector < std::pair < pnt, pnt > > result;
    const int outer = 12;
    const int inner = 8;
    assert(outer + inner == (int)lips.size());
    for (int i = 0; i < outer; i++) 
        result.push_back(std::make_pair(lips[i], lips[(i + 1) % outer]));
    for (int i = 0; i < inner; i++) 
        result.push_back(std::make_pair(lips[i + outer], lips[outer + (i + 1) % inner]));
    return result;
}

std::vector < pnt > Lips::data() {
    return lips;
}

double lipsDist(Lips l1, Lips l2) {
    //db2(l1.getScale(), l2.getScale());

    l1.setScale(1);
    l2.setScale(1);
    double sum = 0;
    for (int i = 0; i < (int)l1.lips.size(); i++)
        sum += (l1.lips[i] - l2.lips[i]).len();
    return sum;
}

Average::Average(): sum(0), cnt(0) { }
void Average::add(double x) {
    cnt++;
    sum += x;
}

double Average::average() {
    assert(cnt != 0);
    return sum / cnt;
}


