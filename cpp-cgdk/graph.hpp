#ifndef STRATEGY_GRAPH
#define STRATEGY_GRAPH

#include <limits>
#include <queue>

namespace strategy {

class Matrix {
public:
    Matrix(std::size_t size, double initial) : size_(size), values_(size * size, initial) {}

    double get(std::size_t column, std::size_t row) const {
        return values_[index(column, row)];
    }

    void set(std::size_t column, std::size_t row, double value) {
        values_[index(column, row)] = value;
    }

private:
    std::size_t size_;
    std::vector<double> values_;

    std::size_t index(std::size_t column, std::size_t row) const {
        return size_ * column + row;
    }
};

class Graph {
public:
    using Node = std::size_t;

    struct Path {
        double length;
        std::vector<Node> nodes;
    };

    Graph(std::size_t size) : size_(size), arcs_(size, std::numeric_limits<double>::max()) {}

    void arc(Node src, Node dst, double weight) {
        arcs_.set(src, dst, weight);
    }

    const Matrix& arcs() const {
        return arcs_;
    }

    Path get_shortest_path(Node src, Node dst) const;

private:
    std::size_t size_;
    Matrix arcs_;

    std::vector<Node> reconstruct_path(Node node, const std::vector<Node>& came_from) const;
};

}

#endif
