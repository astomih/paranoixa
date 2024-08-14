#ifndef PARANOIXA_MATH_RECT_HPP
#define PARANOIXA_MATH_RECT_HPP
namespace paranoixa {
class Rect {
public:
  float x, y, w, h;
  Rect() : x(0.f), y(0.f), w(0.f), h(0.f) {}

  /**
   * @brief Construct a new rect object
   *
   * @param x x position
   * @param y y position
   * @param w width
   * @param h height
   */
  void set(float x, float y, float w, float h) {
    this->x = x;
    this->y = y;
    this->w = w;
    this->h = h;
  }

  /**
   * @brief Calculate the area of the rect
   *
   * @return float
   */
  float area() const { return w * h; }

  /**
   * @brief Check if the rect contains a point
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  bool contains(float x, float y) const {
    return x >= this->x && x <= this->x + this->w && y >= this->y &&
           y <= this->y + this->h;
  }

  /**
   * @brief Check if the rect contains another rect
   *
   * @param r
   * @return true
   * @return false
   */
  bool contains(const Rect &r) const {
    return contains(r.x, r.y) && contains(r.x + r.w, r.y + r.h);
  }
};
} // namespace paranoixa
#endif // !PARANOIXA_MATH_RECT_HPP