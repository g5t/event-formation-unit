//
// Created by g on 9/22/23.
//

#ifndef EVENT_FORMATION_UNIT_POLYGONS_H
#define EVENT_FORMATION_UNIT_POLYGONS_H
#include <utility>
#include <vector>

namespace polygons {

  class Polygon {
    using Point = std::pair<double, double>;
    using Points = std::vector<Point>;
    Points _points;

  public:
    [[nodiscard]] const Points & points() const { return _points; }
    [[nodiscard]] double area() const;
    [[nodiscard]] double perimeter() const;
    [[nodiscard]] Point centroid() const;
    [[nodiscard]] bool isConvex() const;
    [[nodiscard]] bool contains(const Point & p) const;

    explicit Polygon(Points points) : _points(std::move(points)) {}
    Polygon(const Polygon & other) = default;
    Polygon(Polygon && other) noexcept : _points(std::move(other._points)) {}

    explicit Polygon(const std::vector<std::vector<double>> & points) {
      for (const auto & p : points) {
        _points.emplace_back(p[0], p[1]);
      }
    }

    Polygon & operator=(const Polygon & other) = default;
    Polygon & operator=(Polygon && other)  noexcept {
      _points = std::move(other._points);
      return *this;
    }
  };

} // polygons

#endif //EVENT_FORMATION_UNIT_POLYGONS_H
