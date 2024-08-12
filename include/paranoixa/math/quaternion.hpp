#ifndef PARANOIXA_QUATERNION_HPP
#define PARANOIXA_QUATERNION_HPP
#include "math.hpp"
#include "vector3.hpp"
namespace paranoixa {
/**
 * @brief Quaternion class
 *
 */
class Quaternion {
public:
  float x{};
  float y{};
  float z{};
  float w{};
  /**
   * @brief Construct a new quaternion object
   *
   */
  constexpr Quaternion() { *this = Quaternion::Identity; }
  /**
   * @brief Construct a new quaternion object
   *
   * @param inX
   * @param inY
   * @param inZ
   * @param inW
   */
  explicit Quaternion(float inX, float inY, float inZ, float inW) {
    set(inX, inY, inZ, inW);
  }
  explicit Quaternion(const Vector3 &axis, float angle) {
    const auto scalar = Math::sin(angle / 2.0f);
    x = axis.x * scalar;
    y = axis.y * scalar;
    z = axis.z * scalar;
    w = Math::cos(angle / 2.0f);
  }
  /**
   * @brief Directly set the quaternion values
   *
   * @param x x
   * @param y y
   * @param z z
   * @param w w
   */
  void set(float x, float y, float z, float w) {
    this->x = x;
    this->y = y;
    this->z = z;
    this->w = w;
  }
  void conjugate() {
    x *= -1.0f;
    y *= -1.0f;
    z *= -1.0f;
  }
  [[nodiscard]] float lengthSqrt() const {
    return (x * x + y * y + z * z + w * w);
  }
  [[nodiscard]] float length() const { return Math::sqrt(lengthSqrt()); }
  void normalize() {
    const auto len = length();
    x /= len;
    y /= len;
    z /= len;
    w /= len;
  }
  // Normalize the provided quaternion
  static Quaternion normalize(const Quaternion &q) {
    auto retVal = q;
    retVal.normalize();
    return retVal;
  }
  // Linear interpolation
  static Quaternion lerp(const Quaternion &a, const Quaternion &b, float f) {
    Quaternion retVal;
    retVal.x = Math::lerp(a.x, b.x, f);
    retVal.y = Math::lerp(a.y, b.y, f);
    retVal.z = Math::lerp(a.z, b.z, f);
    retVal.w = Math::lerp(a.w, b.w, f);
    retVal.normalize();
    return retVal;
  }
  static float dot(const Quaternion &a, const Quaternion &b) {
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
  }
  // Spherical Linear Interpolation
  static Quaternion slerp(const Quaternion &a, const Quaternion &b, float f) {
    const auto rawCosm = Quaternion::dot(a, b);
    auto cosom = -rawCosm;
    if (rawCosm >= 0.0f) {
      cosom = rawCosm;
    }
    float scale0, scale1;
    if (cosom < 0.9999f) {
      const auto omega = Math::acos(cosom);
      const auto invSin = 1.f / Math::sin(omega);
      scale0 = Math::sin((1.f - f) * omega) * invSin;
      scale1 = Math::sin(f * omega) * invSin;
    } else {
      scale0 = 1.0f - f;
      scale1 = f;
    }
    if (rawCosm < 0.0f) {
      scale1 = -scale1;
    }
    Quaternion retVal;
    retVal.x = scale0 * a.x + scale1 * b.x;
    retVal.y = scale0 * a.y + scale1 * b.y;
    retVal.z = scale0 * a.z + scale1 * b.z;
    retVal.w = scale0 * a.w + scale1 * b.w;
    retVal.normalize();
    return retVal;
  }

  // Concatenate
  // Rotate by q FOLLOWED BY p
  static Quaternion concatenate(const Quaternion &q, const Quaternion &p) {
    Quaternion retVal;

    const Vector3 qv(q.x, q.y, q.z);
    const Vector3 pv(p.x, p.y, p.z);
    const auto newVec = p.w * qv + q.w * pv + Vector3::cross(pv, qv);
    retVal.x = newVec.x;
    retVal.y = newVec.y;
    retVal.z = newVec.z;

    retVal.w = p.w * q.w - Vector3::dot(pv, qv);

    return retVal;
  }
  /**
   * @brief Euler angles to quaternion
   *
   * @param euler Angles
   * @return quaternion Output
   */
  static Quaternion fromEuler(const Vector3 &euler);
  /**
   * @brief Quaternion to Euler angles
   *
   * @param r rotation quaternion
   * @return vector3 euler angles
   */
  static Vector3 toEuler(const Quaternion &r);
  static const Quaternion Identity;
};
} // namespace paranoixa
#endif // !PARANOXIA_QUATERNION_HPP
