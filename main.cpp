#include <string.h>

#include <cmath>
#include <limits>
#include <memory>

#include "model.h"
#include "my_gl.h"
#include "tgaimage.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const int width = 800;
const int height = 800;
const int depth = 255;
float zbuffer[width * height];

int main(int argc, char **argv) {
  TGAImage image(width, height, TGAImage::RGB);
  TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);

  Model model("obj/african_head.obj");

  Vec3f lightDir = Vec3f(0, 0, 1).normalize();
  Vec3f eye(0, 0, 3);
  Vec3f center(0, 0, 0);
  Vec3f up(0, 1, 0);

  // MVP transformation
  Matrix modelM = Matrix::identity();
  Matrix viewM = lookat(eye, center, up);
  Matrix projectM = projection(-1.f / (eye - center).norm());
  Matrix viewportM = viewport(0, 0, width, height);

  GourandShader shader;
  shader.setLightDir(lightDir);
  shader.setmvp(projectM * viewM * modelM);
  shader.setViewport(viewportM);
  for (int i = 0; i < model.nfaces(); i++) {
    Vec3f sreenCoords[3];
    for (int j = 0; j < 3; j++) {
      sreenCoords[j] = shader.vertex(i, j, model);
    }
    triangle(sreenCoords, shader, image, zbuffer);
  }

  image.flip_vertically();
  image.write_tga_file("output.tga");
  return 0;
}
