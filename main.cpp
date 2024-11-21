#include <cassert>
#include <iostream>
#include <vector>

#include "geometry.h"
#include "model.h"
#include "my_gl.h"
#include "tgaimage.h"

const int width = 800;
const int height = 800;

TGAImage image(width, height, TGAImage::RGB);
TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);
TGAImage shadowMap(width, height, TGAImage::GRAYSCALE);

Vec3f lightDir = Vec3f(1, 1, 0).normalize();
Vec3f eye(1, 1, 4);
Vec3f center(0, 0, 0);
Vec3f up(0, 1, 0);

struct ShadowMapShader : public IShader {
  virtual Vec3f vertex(int iface, int nthvert, Model *model) override {
    Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert));
    gl_Vertex = uViewport * uMVP * gl_Vertex;
    return proj<3>(gl_Vertex / gl_Vertex[3]);
  }

  virtual bool fragment(Vec3f bc, TGAColor &color, Model *model) override {
    return true;
  }
};

struct PhongShader : public IShader {
  Matrix uMShadow;
  mat<3, 3, float> vPos;

  virtual Vec3f vertex(int iface, int nthvert, Model *model) override {
    Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert));
    gl_Vertex = uViewport * uMVP * gl_Vertex;
    vUV.set_col(nthvert, model->uv(iface, nthvert));
    Vec3f res = proj<3>(gl_Vertex / gl_Vertex[3]);
    vPos.set_col(nthvert, res);
    return res;
  }

  virtual bool fragment(Vec3f bc, TGAColor &color, Model *model) override {
    Vec4f sMapCoords = uMShadow * embed<4>(vPos * bc);
    sMapCoords = sMapCoords / sMapCoords[3];
    float shadow = .3 + .7 * (shadowMap.get(sMapCoords[0], sMapCoords[1])[0] <
                              sMapCoords[2]);
    Vec2f uv = vUV * bc;
    Vec3f n = proj<3>(uMVP_IT * embed<4>(model->normal(uv))).normalize();
    Vec3f l = proj<3>(uMVP * embed<4>(uLightDir)).normalize();
    Vec3f r = (n * 2.0f * (l * n) - l).normalize();
    float specular = std::pow(std::max(r.z, 0.0f), model->specular(uv));
    float diffuse = std::max(0.0f, n * l);
    TGAColor c = model->diffuse(uv);
    color = c;
    for (int i = 0; i < 3; i++)
      color[i] = std::min<float>(
          20 + c[i] * shadow * (1.2 * diffuse + .6 * specular), 255);
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
  Matrix viewM = lookat(lightDir, center, up);
  Matrix projectM = projection(0.0f); // orthographic projection

  {
    ShadowMapShader shader;
    shader.setLightDir(lightDir);
    shader.setMVP(projectM * viewM);
    shader.setviewport(viewportM);
    for (int i = 0; i < model->nfaces(); i++) {
      Vec3f screen_coords[3];
      for (int j = 0; j < 3; j++) {
        screen_coords[j] = shader.vertex(i, j, model);
      }
      triangle(screen_coords, shader, shadowMap, zbuffer, model);
    }
  }

  shadowMap = zbuffer;
  zbuffer.clear();
  Matrix M = viewportM * projectM * viewM;

  {
    viewM = lookat(eye, center, up);
    projectM = projection(-1.f / (eye - center).norm());

    PhongShader shader;
    shader.setLightDir(lightDir);
    shader.setMVP(projectM * viewM);
    shader.setviewport(viewportM);
    shader.uMShadow = M * (viewportM * projectM * viewM).invert();
    for (int i = 0; i < model->nfaces(); i++) {
      Vec3f screen_coords[3];
      for (int j = 0; j < 3; j++) {
        screen_coords[j] = shader.vertex(i, j, model);
      }
      triangle(screen_coords, shader, image, zbuffer, model);
    }
  }

  image.flip_vertically();
  image.write_tga_file("output.tga");
  // zbuffer.flip_vertically();
  // zbuffer.write_tga_file("zbuffer.tga");

  delete model;
  return 0;
}