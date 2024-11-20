#ifndef MY_GL_H
#define MY_GL_H

#include <memory>

#include "geometry.h"
#include "model.h"
#include "tgaimage.h"

Matrix viewport(int x, int y, int w, int h);
Matrix projection(double f);
Matrix lookat(Vec3f eye, Vec3f center, Vec3f up);
Vec3f barycentric(Vec3f v_pts[3], Vec3f p);

class Shader {
public:
  Shader() = default;
  virtual ~Shader();
  virtual Vec3f vertex(int iface, int nthvert, Model &model) = 0;
  virtual bool fragment(Vec3f barycentric, TGAColor &color) = 0;

public:
  virtual void setmvp(const Matrix &MVP) { this->mvp_ = MVP; }
  virtual void setLightDir(const Vec3f &lightDir) {
    this->lightDir_ = lightDir;
  }
  virtual void setViewport(const Matrix &Viewport) {
    this->viewport_ = Viewport;
  }

protected:
  Vec3f lightDir_;
  Matrix mvp_, viewport_;
};

void triangle(Vec3f screenCoords[3], Shader &shader, TGAImage &image,
              TGAImage &zbuffer);

class GourandShader : public Shader {
public:
  GourandShader() {}
  ~GourandShader() {}
  Vec3f vertex(int iface, int nthvert, Model &model) override;
  bool fragment(Vec3f barycentric, TGAColor &color) override;

private:
  Vec3f vertexIntensity_;
};

#endif