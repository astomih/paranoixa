#ifndef PARANOIXA_MATRIX4_HPP
#define PARANOIXA_MATRIX4_HPP

#include "math.hpp"
#include "vector3.hpp"

namespace paranoixa {
/**
 * @brief float matrix4x4 class
 *
 */
class Matrix4 {
public:
  union Type {
    float m[4][4];
    float m16[16];

    float *operator[](const std::size_t &i) { return m[i]; }
    const float *operator[](const std::size_t &i) const { return m[i]; }
  };
  Type mat;

  /**
   * @brief Construct a new matrix4 object
   *
   */
  constexpr Matrix4() { *this = Matrix4::identity; }
  /**
   * @brief Construct a new matrix4 object
   *
   * @param inMat  matrix4x4 to copy
   */
  explicit Matrix4(float inMat[4][4]) {
    memcpy(mat.m, inMat, 16 * sizeof(float));
  }
  /**
   * @brief Construct a new matrix4 object
   *
   * @param inMat  matrix4x4 to copy
   */
  explicit Matrix4(float inMat[16]) {
    memcpy(mat.m16, inMat, 16 * sizeof(float));
  }
  /**
   * @brief Cast to const float*
   *
   * @return const float* pointer to matrix4x4
   */
  float *get() { return reinterpret_cast<float *>(mat.m16); }

  float *operator[](const size_t index) { return mat[index]; }

  Vector3 operator*(const Vector3 &vec) const;

  // Matrix multiplication (a * b)
  friend Matrix4 operator*(const Matrix4 &a, const Matrix4 &b) {
    Matrix4 retVal;
    // row 0
    retVal.mat[0][0] = a.mat[0][0] * b.mat[0][0] + a.mat[0][1] * b.mat[1][0] +
                       a.mat[0][2] * b.mat[2][0] + a.mat[0][3] * b.mat[3][0];

    retVal.mat[0][1] = a.mat[0][0] * b.mat[0][1] + a.mat[0][1] * b.mat[1][1] +
                       a.mat[0][2] * b.mat[2][1] + a.mat[0][3] * b.mat[3][1];

    retVal.mat[0][2] = a.mat[0][0] * b.mat[0][2] + a.mat[0][1] * b.mat[1][2] +
                       a.mat[0][2] * b.mat[2][2] + a.mat[0][3] * b.mat[3][2];

    retVal.mat[0][3] = a.mat[0][0] * b.mat[0][3] + a.mat[0][1] * b.mat[1][3] +
                       a.mat[0][2] * b.mat[2][3] + a.mat[0][3] * b.mat[3][3];

    // row 1
    retVal.mat[1][0] = a.mat[1][0] * b.mat[0][0] + a.mat[1][1] * b.mat[1][0] +
                       a.mat[1][2] * b.mat[2][0] + a.mat[1][3] * b.mat[3][0];

    retVal.mat[1][1] = a.mat[1][0] * b.mat[0][1] + a.mat[1][1] * b.mat[1][1] +
                       a.mat[1][2] * b.mat[2][1] + a.mat[1][3] * b.mat[3][1];

    retVal.mat[1][2] = a.mat[1][0] * b.mat[0][2] + a.mat[1][1] * b.mat[1][2] +
                       a.mat[1][2] * b.mat[2][2] + a.mat[1][3] * b.mat[3][2];

    retVal.mat[1][3] = a.mat[1][0] * b.mat[0][3] + a.mat[1][1] * b.mat[1][3] +
                       a.mat[1][2] * b.mat[2][3] + a.mat[1][3] * b.mat[3][3];

    // row 2
    retVal.mat[2][0] = a.mat[2][0] * b.mat[0][0] + a.mat[2][1] * b.mat[1][0] +
                       a.mat[2][2] * b.mat[2][0] + a.mat[2][3] * b.mat[3][0];

    retVal.mat[2][1] = a.mat[2][0] * b.mat[0][1] + a.mat[2][1] * b.mat[1][1] +
                       a.mat[2][2] * b.mat[2][1] + a.mat[2][3] * b.mat[3][1];

    retVal.mat[2][2] = a.mat[2][0] * b.mat[0][2] + a.mat[2][1] * b.mat[1][2] +
                       a.mat[2][2] * b.mat[2][2] + a.mat[2][3] * b.mat[3][2];

    retVal.mat[2][3] = a.mat[2][0] * b.mat[0][3] + a.mat[2][1] * b.mat[1][3] +
                       a.mat[2][2] * b.mat[2][3] + a.mat[2][3] * b.mat[3][3];

    // row 3
    retVal.mat[3][0] = a.mat[3][0] * b.mat[0][0] + a.mat[3][1] * b.mat[1][0] +
                       a.mat[3][2] * b.mat[2][0] + a.mat[3][3] * b.mat[3][0];

    retVal.mat[3][1] = a.mat[3][0] * b.mat[0][1] + a.mat[3][1] * b.mat[1][1] +
                       a.mat[3][2] * b.mat[2][1] + a.mat[3][3] * b.mat[3][1];

    retVal.mat[3][2] = a.mat[3][0] * b.mat[0][2] + a.mat[3][1] * b.mat[1][2] +
                       a.mat[3][2] * b.mat[2][2] + a.mat[3][3] * b.mat[3][2];

    retVal.mat[3][3] = a.mat[3][0] * b.mat[0][3] + a.mat[3][1] * b.mat[1][3] +
                       a.mat[3][2] * b.mat[2][3] + a.mat[3][3] * b.mat[3][3];

    return retVal;
  }

  Matrix4 &operator*=(const Matrix4 &right) {
    *this = *this * right;
    return *this;
  }

  // Invert the matrix - super slow
  void invert();

  // Get the translation component of the matrix
  [[nodiscard]] Vector3 getTranslation() const {
    return Vector3(mat[3][0], mat[3][1], mat[3][2]);
  }

  // Get the X axis of the matrix (forward)
  [[nodiscard]] Vector3 getXAxis() const {
    return Vector3::normalize(Vector3(mat[0][0], mat[0][1], mat[0][2]));
  }

  // Get the Y axis of the matrix (left)
  [[nodiscard]] Vector3 getYAxis() const {
    return Vector3::normalize(Vector3(mat[1][0], mat[1][1], mat[1][2]));
  }

  // Get the Z axis of the matrix (up)
  [[nodiscard]] Vector3 getZAxis() const {
    return Vector3::normalize(Vector3(mat[2][0], mat[2][1], mat[2][2]));
  }

  // Extract the scale component from the matrix
  [[nodiscard]] Vector3 getScale() const {
    Vector3 retVal;
    retVal.x = Vector3(mat[0][0], mat[0][1], mat[0][2]).length();
    retVal.y = Vector3(mat[1][0], mat[1][1], mat[1][2]).length();
    retVal.z = Vector3(mat[2][0], mat[2][1], mat[2][2]).length();
    return retVal;
  }

  // Create a scale matrix with x, y, and z scales

  // Rotation about x-axis
  static Matrix4 createRotaionX(const float theta) {
    float temp[4][4] = {
        {1.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, Math::cos(theta), Math::sin(theta), 0.0f},
        {0.0f, -Math::sin(theta), Math::cos(theta), 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
    };
    return Matrix4(temp);
  }

  // Rotation about y-axis
  static Matrix4 createRotationY(const float theta) {
    float temp[4][4] = {
        {Math::cos(theta), 0.0f, -Math::sin(theta), 0.0f},
        {0.0f, 1.0f, 0.0f, 0.0f},
        {Math::sin(theta), 0.0f, Math::cos(theta), 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
    };
    return Matrix4(temp);
  }

  // Rotation about z-axis
  static Matrix4 createRotationZ(const float theta) {
    float temp[4][4] = {
        {Math::cos(theta), Math::sin(theta), 0.0f, 0.0f},
        {-Math::sin(theta), Math::cos(theta), 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
    };
    return Matrix4(temp);
  }
  static Matrix4 transpose(const Matrix4 &m);

  // Create a rotation matrix from a Quaternion
  static Matrix4 createFromQuaternion(const class Quaternion &q);

  static Matrix4 createTranslation(const Vector3 &trans);

  static Quaternion toQuaternion(const Matrix4 &m);
  static Matrix4 createScale(const Vector3 &scale);

  static Matrix4 lookAt(const Vector3 &eye, const Vector3 &at,
                        const Vector3 &up);

  static Matrix4 perspective(const float angle, const float aspect,
                             const float near, const float far);

  static Matrix4 ortho(float width, float height, float near, float far);

  static Matrix4 ortho(float left, float right, float bottom, float top,
                       float near, float far);

  static const Matrix4 identity;
};
} // namespace paranoixa
#endif // !PARANOIXA_MATRIX4_HPP
