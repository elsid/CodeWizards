#ifndef STRATEGY_POINT
#define STRATEGY_POINT

#include "math.hpp"

#include <ostream>
#include <limits>
#include <iomanip>

namespace strategy {

template <class T>
class BasicPoint {
public:
    using Value = T;

    BasicPoint(Value x = 0, Value y = 0) : x_(x), y_(y) {}

    Value x() const { return x_; }
    Value y() const { return y_; }

    double distance(const BasicPoint& other) const {
        return std::hypot(other.x() - x(), other.y() - y());
    }

    double square() const {
        return x() * x() + y() * y();
    }

    double dot(const BasicPoint& other) const {
        return x() * other.x() + y() * other.y();
    }

    BasicPoint rotated(double angle) const {
        double cos = math::cos(angle);
        double sin = math::sin(angle);
        return BasicPoint(x() * cos - y() * sin, y() * cos + x() * sin);
    }

    double norm() const {
        return hypot(x(), y());
    }

    double cos(const BasicPoint& other) const {
        return dot(other) / (norm() * other.norm());
    }

    double absolute_rotation() const {
        return std::atan2(y(), x());
    }

    BasicPoint left_orthogonal() const {
        return BasicPoint(-y(), x());
    }

    BasicPoint<int> to_int() const {
        return BasicPoint<int>(int(std::round(x())), int(std::round(y())));
    }

    BasicPoint<double> to_double() const {
        return BasicPoint<double>(double(x()), double(y()));
    }

    double det(const BasicPoint& other) const {
        return x() * other.y() - y() * other.x();
    }

private:
    Value x_;
    Value y_;
};

using Point = BasicPoint<double>;
using PointInt = BasicPoint<int>;

template <class T>
inline bool operator ==(const BasicPoint<T>& lhs, const BasicPoint<T>& rhs) {
    return lhs.x() == rhs.x() && lhs.y() == rhs.y();
}

template <class T>
inline bool operator !=(const BasicPoint<T>& lhs, const BasicPoint<T>& rhs) {
    return !(lhs == rhs);
}

template <class T>
inline BasicPoint<T> operator *(const BasicPoint<T>& lhs, double rhs) {
    return BasicPoint<T>(lhs.x() * rhs, lhs.y() * rhs);
}

template <class T>
inline BasicPoint<T> operator *(double lhs, const BasicPoint<T>& rhs) {
    return BasicPoint<T>(lhs * rhs.x(), lhs * rhs.y());
}

template <class T>
inline BasicPoint<T> operator /(const BasicPoint<T>& lhs, double rhs) {
    return BasicPoint<T>(lhs.x() / rhs, lhs.y() / rhs);
}

template <class T>
inline BasicPoint<T> operator +(const BasicPoint<T>& lhs, const BasicPoint<T>& rhs) {
    return BasicPoint<T>(lhs.x() + rhs.x(), lhs.y() + rhs.y());
}

template <class T>
inline BasicPoint<T> operator -(const BasicPoint<T>& lhs, const BasicPoint<T>& rhs) {
    return BasicPoint<T>(lhs.x() - rhs.x(), lhs.y() - rhs.y());
}

template <class T>
inline bool operator <(const BasicPoint<T>& lhs, const BasicPoint<T>& rhs) {
    return lhs.x() < rhs.x() || (lhs.x() == rhs.x() && lhs.y() < rhs.y());
}

template <class T>
inline bool operator >(const BasicPoint<T>& lhs, const BasicPoint<T>& rhs) {
    return lhs.x() > rhs.x() || (lhs.x() == rhs.x() && lhs.y() > rhs.y());
}

inline BasicPoint<int> operator %(const BasicPoint<int>& lhs, int rhs) {
    return BasicPoint<int>(lhs.x() % rhs, lhs.y() % rhs);
}

template <class T>
inline std::ostream& operator <<(std::ostream& stream, const BasicPoint<T>& value) {
    return stream << std::setprecision(std::numeric_limits<double>::max_digits10)
                  << "BasicPoint(" << value.x() << ", " << value.y() << ")";
}

}

#endif
