#ifndef POINT2_H
#define POINT2_H

#include "tuple.h"

template<typename T>
struct Point2 : public Tuple2<Point2, T> {
    using Tuple2<Point2, T>::Tuple2;
};

#endif
