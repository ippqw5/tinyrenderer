#ifndef __MY_GL_H__
#define __MY_GL_H__

#include "geometry.h"
#include "model.h"
#include "tgaimage.h"
Matrix viewport(int x, int y, int w, int h);
Matrix projection(float coeff = 0.f); // coeff = -1/c
Matrix lookat(Vec3f eye, Vec3f center, Vec3f up);

struct IShader {
  virtual ~IShader();
  virtual Vec3f vertex(int iface, int nthvert, Model *) = 0;
  virtual bool fragment(Vec3f vc, TGAColor &color, Model *) = 0;

  void setMVP(const Matrix &mvp) {
    uMVP = mvp;
    uMVP_IT = uMVP.invert_transpose();
  }
  void setviewport(const Matrix &viewport) { uViewport = viewport; }
  void setLightDir(const Vec3f &lightDir) { this->uLightDir = lightDir; }

protected:
  Vec3f uLightDir;
  Matrix uMVP;
  Matrix uMVP_IT;
  Matrix uViewport;
  mat<2, 3, float> vUV;
};

void triangle(Vec3f *pts, IShader &shader, TGAImage &image, TGAImage &zbuffer,
              Model *model);

struct GouraudShader : public IShader {
  // written by vertex shader, read by fragment shader

  virtual Vec3f vertex(int iface, int nthvert, Model *model) override {
    Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert));

    gl_Vertex = uViewport * uMVP * gl_Vertex;

    vUV.set_col(nthvert, model->uv(iface, nthvert));

    // homogenous coordinates --> cartesian coordinates
    return proj<3>(gl_Vertex / gl_Vertex[3]);
  }

  virtual bool fragment(Vec3f bc, TGAColor &color, Model *model) override {
    Vec2f uv = vUV * bc;
    Vec3f n = proj<3>(uMVP_IT * embed<4>(model->normal(uv))).normalize();
    Vec3f l = proj<3>(uMVP * embed<4>(uLightDir)).normalize();
    float intensity = std::max(0.f, n * l);
    color = model->diffuse(uv) * intensity;
    return false;
  }
};

#endif //__MY_GL_H__
