#include <cmath>

#include "model.h"
#include "tgaimage.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const int width = 600;
const int height = 600;
#define Deprcated

void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color) {
  bool steep = false;
  if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
    std::swap(x0, y0);
    std::swap(x1, y1);
    steep = true;
  }
  if (x0 > x1) {
    std::swap(x0, x1);
    std::swap(y0, y1);
  }

  int dx = x1 - x0;

  int dy = y1 - y0;

  int derror2 = std::abs(dy) * 2;
  int error2 = 0;
  int y = y0;
  for (int x = x0; x <= x1; x++) {
    if (steep) {
      image.set(y, x, color);
    } else {
      image.set(x, y, color);
    }
    error2 += derror2;

    if (error2 > dx) {
      y += (y1 > y0 ? 1 : -1);
      error2 -= dx * 2;
    }
  }
}

#ifdef Deprcated
#else
void triangle(Vec2i v1, Vec2i v2, Vec2i v3, TGAImage& image, TGAColor color) {
  if (v1.y > v2.y) std::swap(v1, v2);
  if (v1.y > v3.y) std::swap(v1, v3);
  if (v2.y > v3.y) std::swap(v2, v3);

  for (int y = v1.y; y <= v2.y; y++) {
    int xs = v1.x + (v2.x - v1.x) * (y - v1.y) / (float)(v2.y - v1.y);
    int xe = v1.x + (v3.x - v1.x) * (y - v1.y) / (float)(v3.y - v1.y);
    line(xs, y, xe, y, image, color);
  }
  for (int y = v2.y; y <= v3.y; y++) {
    int xs = v2.x + (v3.x - v2.x) * (y - v2.y) / (float)(v3.y - v2.y);
    int xe = v1.x + (v3.x - v1.x) * (y - v1.y) / (float)(v3.y - v1.y);
    line(xs, y, xe, y, image, color);
  }
  line(v1.x, v1.y, v2.x, v2.y, image, color);
  line(v2.x, v2.y, v3.x, v3.y, image, color);
  line(v3.x, v3.y, v1.x, v1.y, image, color);
}
#endif

template <class T>
struct BoundBox {
  Vec2<T> bmin;
  Vec2<T> bmax;
  BoundBox(Vec2<T> pts[3])
      : bmin(std::numeric_limits<T>::max(), std::numeric_limits<T>::max()),
        bmax(std::numeric_limits<T>::min(), std::numeric_limits<T>::min()) {
    for (int i = 0; i < 3; i++) {
      bmin.x = std::min(bmin.x, pts[i].x);
      bmin.y = std::min(bmin.y, pts[i].y);
      bmax.x = std::max(bmax.x, pts[i].x);
      bmax.y = std::max(bmax.y, pts[i].y);
    }
  }
};

Vec3f barycentric(Vec2i pts[3], Vec2i p) {
  Vec3f u = Vec3f(pts[2].x - pts[0].x, pts[1].x - pts[0].x, pts[0].x - p.x) ^
            Vec3f(pts[2].y - pts[0].y, pts[1].y - pts[0].y, pts[0].y - p.y);
  if (std::abs(u.z) < 1) return Vec3f(-1, 1, 1);
  return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
}

void triangle(Vec2i pts[3], TGAImage& image, TGAColor color) {
  BoundBox<int> bbox(pts);
  for (int x = bbox.bmin.x; x <= bbox.bmax.x; x++) {
    for (int y = bbox.bmin.y; y <= bbox.bmax.y; y++) {
      Vec3f bc_screen = barycentric(pts, {x, y});
      if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue;
      image.set(x, y, color);
    }
  }
}

int main(int argc, char** argv) {
  TGAImage image(width, height, TGAImage::RGB);
  // Vec2i t0[3] = {Vec2i(10, 70), Vec2i(50, 160), Vec2i(70, 80)};
  // Vec2i t1[3] = {Vec2i(180, 50), Vec2i(150, 1), Vec2i(70, 180)};
  // Vec2i t2[3] = {Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180)};
  // triangle(t0, image, red);
  // // triangle(t1[0], t1[1], t1[2], image, white);
  // // triangle(t2[0], t2[1], t2[2], image, green);

  Model model("obj/african_head.obj");
  Vec3f light_dir(0, 0, -1);  // define light_dir
  for (int i = 0; i < model.nfaces(); i++) {
    std::vector<int> face = model.face(i);
    Vec2i screen_coords[3];
    Vec3f world_coords[3];
    for (int j = 0; j < 3; j++) {
      Vec3f v = model.vert(face[j]);
      screen_coords[j] =
          Vec2i((v.x + 1.) * width / 2., (v.y + 1.) * height / 2.);
      world_coords[j] = v;
    }
    Vec3f n = (world_coords[2] - world_coords[0]) ^
              (world_coords[1] - world_coords[0]);
    n.normalize();
    float intensity = n * light_dir;
    if (intensity > 0.0)
      triangle(
          screen_coords, image,
          TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));
  }

  image.flip_vertically();  // i want to have the origin at the left bottom
                            // corner of the image
  image.write_tga_file("output.tga");
  return 0;
}
