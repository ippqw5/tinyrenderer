#include <cassert>
#include <iostream>
#include <vector>

#include "geometry.h"
#include "model.h"
#include "my_gl.h"
#include "tgaimage.h"

Model *model = NULL;
const int width = 800;
const int height = 800;

Vec3f light_dir(1, 1, 1);
Vec3f eye(0, -1, 3);
Vec3f center(0, 0, 0);
Vec3f up(0, 1, 0);

struct GouraudShader : public IShader {
  Vec3f varying_intensity; // written by vertex shader, read by fragment shader

  virtual Vec3f vertex(int iface, int nthvert) {
    Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert));

    // transform it to screen coordinates
    gl_Vertex = viewport_ * mvp_ * gl_Vertex;

    // get diffuse lighting intensity
    varying_intensity[nthvert] =
        std::max(0.f, model->normal(iface, nthvert) * light_dir);

    // project the vertex to screen coordinates
    return proj<3>(gl_Vertex / gl_Vertex[3]);
  }

  virtual bool fragment(Vec3f bar, TGAColor &color) {
    float intensity = varying_intensity * bar;
    color = TGAColor(255, 255, 255) * intensity;
    return false;
  }
};

int main(int argc, char **argv) {
  if (2 == argc) {
    model = new Model(argv[1]);
  } else {
    model = new Model("obj/african_head.obj");
  }

  //   viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
  Matrix viewportM = viewport(0, 0, width, height);
  Matrix viewM = lookat(eye, center, up);
  Matrix projectM = projection(-1.f / (eye - center).norm());
  light_dir.normalize();

  TGAImage image(width, height, TGAImage::RGB);
  TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);

  GouraudShader shader;
  shader.setmvp(viewM * projectM);
  shader.setviewport(viewportM);
  for (int i = 0; i < model->nfaces(); i++) {
    Vec3f screen_coords[3];
    for (int j = 0; j < 3; j++) {
      screen_coords[j] = shader.vertex(i, j);
      // screen_coords[j] = screen_coords[j] / screen_coords[j][3];
      // assert(screen_coords[j][3] == 1.0f);
    }
    triangle(screen_coords, shader, image, zbuffer);
  }

  image.flip_vertically();
  zbuffer.flip_vertically();
  image.write_tga_file("output.tga");
  zbuffer.write_tga_file("zbuffer.tga");

  delete model;
  return 0;
}