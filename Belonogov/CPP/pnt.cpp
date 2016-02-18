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


Lips::Lips(std::vector < pnt > lips): lips(lips) { }

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
    assert(equal(oldValue, 1));
    k = k / oldValue;
    for (auto &p: lips)
        p = p * k;
}

void Lips::shift(pnt v) {
    for (auto &p: lips)
        p = p + v;
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

std::vector < pnt > Lips::getData() {
    return lips;
}

