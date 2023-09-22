//
// Created by g on 9/22/23.
//
#include <math.h>
#include "Polygons.h"

namespace polygons {
  double Polygon::area() const {
    double area = 0.0;
    for (std::size_t i = 0; i < _points.size(); ++i) {
      const auto & p1 = _points[i];
      const auto & p2 = _points[(i + 1) % _points.size()];
      area += p1.first * p2.second - p2.first * p1.second;
    }
    return area / 2.0;
  }

  double Polygon::perimeter() const {
    double perimeter = 0.0;
    for (std::size_t i = 0; i < _points.size(); ++i) {
      const auto & p1 = _points[i];
      const auto & p2 = _points[(i + 1) % _points.size()];
      const auto dx = p1.first - p2.first;
      const auto dy = p1.second - p2.second;
      perimeter += std::sqrt(dx * dx + dy * dy);
    }
    return perimeter;
  }

  Polygon::Point Polygon::centroid() const {
    double cx = 0.0;
    double cy = 0.0;
    double area = 0.0;
    for (std::size_t i = 0; i < _points.size(); ++i) {
      const auto & p1 = _points[i];
      const auto & p2 = _points[(i + 1) % _points.size()];
      const auto a = p1.first * p2.second - p2.first * p1.second;
      cx += (p1.first + p2.first) * a;
      cy += (p1.second + p2.second) * a;
      area += a;
    }
    area /= 2.0;
    cx /= 6.0 * area;
    cy /= 6.0 * area;
    return {cx, cy};
  }

  bool Polygon::isConvex() const {
    bool got_negative = false;
    bool got_positive = false;
    const auto size = _points.size();
    for (std::size_t a = 0; a < size; ++a) {
      const auto & p1 = _points[a];
      const auto & p2 = _points[(a + 1) % size];
      const auto & p3 = _points[(a + 2) % size];
      const auto cross_product =
          (p2.first - p1.first) * (p3.second - p2.second) - (p2.second - p1.second) * (p3.first - p2.first);
      if (cross_product < 0.0) {
        got_negative = true;
      } else if (cross_product > 0.0) {
        got_positive = true;
      }
      if (got_negative && got_positive) {
        return false;
      }
    }
    return true;
  }

  bool Polygon::contains(const polygons::Polygon::Point &p) const {
    bool inside = false;
    const auto size = _points.size();
    for (std::size_t i = 0, j = size - 1; i < size; j = i++) {
      const auto &pi = _points[i];
      const auto &pj = _points[j];
      if (((pi.second > p.second) != (pj.second > p.second)) &&
          (p.first < (pj.first - pi.first) * (p.second - pi.second) / (pj.second - pi.second) + pi.first)) {
        inside = !inside;
      }
    }
    return inside;
  }
} // polygons