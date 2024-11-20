#ifndef __MY_GL_H__
#define __MY_GL_H__

#include "geometry.h"
#include "tgaimage.h"

Matrix viewport(int x, int y, int w, int h);
Matrix projection(float coeff = 0.f); // coeff = -1/c
Matrix lookat(Vec3f eye, Vec3f center, Vec3f up);

struct IShader {
  virtual ~IShader();
  virtual Vec3f vertex(int iface, int nthvert) = 0;
  virtual bool fragment(Vec3f bar, TGAColor &color) = 0;

  void setmvp(const Matrix &mvp) { mvp_ = mvp; }
  void setviewport(const Matrix &viewport) { viewport_ = viewport; }

protected:
  Matrix mvp_;
  Matrix viewport_;
};

void triangle(Vec3f *pts, IShader &shader, TGAImage &image, TGAImage &zbuffer);

#endif //__MY_GL_H__
