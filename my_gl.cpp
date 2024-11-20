#include "my_gl.h"
#include <cassert>
#include <limits>

#define CLAMP(x, a, b) (x < a ? a : (x > b ? b : x))

Shader::~Shader() {}

Vec3f GourandShader::vertex(int iface, int nthvert, Model &model) {
  assert(nthvert >= 0 && nthvert < 3);

  vertexIntensity_[nthvert] =
      CLAMP(model.normal(iface, nthvert) * lightDir_, 0.0f, 1.0f);

  Vec4f v = embed<4>(model.vert(iface, nthvert), 1.0f);
  Vec4f res = (viewport_ * mvp_ * v);
  return {res[0] / res[3], res[1] / res[3], res[2] / res[3]};
}

bool GourandShader::fragment(Vec3f barycentric, TGAColor &color) {
  float intensity = vertexIntensity_ * barycentric;
  color = TGAColor(255, 255, 255, 255) * intensity;
  return false;
}

Matrix viewport(int x, int y, int w, int h) {
  Matrix m = Matrix::identity();
  m[0][3] = x + w / 2.f;
  m[1][3] = y + h / 2.f;
  m[2][3] = 255.f / 2.f;

  m[0][0] = w / 2.f;
  m[1][1] = h / 2.f;
  m[2][2] = 255.f / 2.f;
  return m;
}

Matrix projection(double f) {
  Matrix m = Matrix::identity();
  m[3][2] = f;
  return m;
}

Matrix lookat(Vec3f eye, Vec3f center, Vec3f up) {
  Vec3f z = (eye - center).normalize();
  Vec3f x = cross(up, z).normalize();
  Vec3f y = cross(z, x).normalize();
  Matrix m = Matrix::identity();
  for (int i = 0; i < 3; i++) {
    m[0][i] = x[i];
    m[1][i] = y[i];
    m[2][i] = z[i];
    m[i][3] = -center[i];
  }
  return m;
}

Vec3f barycentric(Vec3f v_pts[3], Vec3f p) {
  Vec3f v[2];
  for (int i = 2; i--;) {
    v[i][0] = v_pts[2][i] - v_pts[0][i];
    v[i][1] = v_pts[1][i] - v_pts[0][i];
    v[i][2] = v_pts[0][i] - p[i];
  }
  Vec3f u = cross(v[0], v[1]);
  if (std::abs(u[2]) > 1e-2)
    return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
  return Vec3f(-1, 1, 1);
}

void triangle(Vec3f sc[3], Shader &shader, TGAImage &image, TGAImage &zbuffer) {
  Vec2f bboxmin(std::numeric_limits<float>::max(),
                std::numeric_limits<float>::max());
  Vec2f bboxmax(-std::numeric_limits<float>::max(),
                -std::numeric_limits<float>::max());
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 2; j++) {
      bboxmin[j] = std::min(bboxmin[j], sc[i][j]);
      bboxmax[j] = std::max(bboxmax[j], sc[i][j]);
    }
  }

  Vec3f P;
  for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
    for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
      Vec3f bc = barycentric(sc, P);
      float interp_z = sc[0].z * bc.x + sc[1].z * bc.y + sc[2].z * bc.z;
      if (bc.x < 0 || bc.y < 0 || bc.z < 0 ||
          interp_z < zbuffer.get(P.x, P.y)[0])
        continue;

      TGAColor color;
      shader.fragment(bc, color);
      zbuffer.set(P.x, P.y, TGAColor(interp_z));
      image.set(P.x, P.y, color);
    }
  }
}