#include <cassert>
#include <iostream>
#include <vector>

#include "geometry.h"
#include "model.h"
#include "my_gl.h"
#include "tgaimage.h"

const int width = 800;
const int height = 800;
Vec3f lightDir = Vec3f(1, 1, 1).normalize();
Vec3f eye(2, 1, 3);
Vec3f center(0, 0, 0);
Vec3f up(0, 1, 0);

struct PhongShader : public IShader {
  virtual Vec3f vertex(int iface, int nthvert, Model *model) override {
    Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert));
    gl_Vertex = uViewport * uMVP * gl_Vertex;
    vUV[nthvert] = model->uv(iface, nthvert);
    return proj<3>(gl_Vertex / gl_Vertex[3]);
  }

  virtual bool fragment(Vec3f bc, TGAColor &color, Model *model) override {
    Vec2f uv = vUV[0] * bc[0] + vUV[1] * bc[1] + vUV[2] * bc[2];
    Vec3f n = proj<3>(uMVP_IT * embed<4>(model->normal(uv))).normalize();
    Vec3f l = proj<3>(uMVP * embed<4>(uLightDir)).normalize();
    Vec3f r = n * 2.0f * (l * n) - l;
    float specular = std::pow(std::max(r.z, 0.0f), model->specular(uv));
    float diffuse = std::max(0.0f, n * l);
    TGAColor c = model->diffuse(uv);
    color = c;
    for (int i = 0; i < 3; i++) {
      color[i] = std::min<float>(5 + c[i] * (diffuse + .6 * specular), 255);
    }
    return false;
  }
};

int main(int argc, char **argv) {
  Model *model = nullptr;
  if (2 == argc) {
    model = new Model(argv[1]);
  } else {
    model = new Model("obj/african_head.obj");
  }

  Matrix viewportM = viewport(0, 0, width, height);
  Matrix viewM = lookat(eye, center, up);
  Matrix projectM = projection(-1.f / (eye - center).norm());

  TGAImage image(width, height, TGAImage::RGB);
  TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);

  PhongShader shader;
  shader.setLightDir(lightDir);
  shader.setmvp(viewM * projectM);
  shader.setviewport(viewportM);
  for (int i = 0; i < model->nfaces(); i++) {
    Vec3f screen_coords[3];
    for (int j = 0; j < 3; j++) {
      screen_coords[j] = shader.vertex(i, j, model);
    }
    triangle(screen_coords, shader, image, zbuffer, model);
  }

  image.flip_vertically();
  zbuffer.flip_vertically();
  image.write_tga_file("output.tga");
  zbuffer.write_tga_file("zbuffer.tga");

  delete model;
  return 0;
}