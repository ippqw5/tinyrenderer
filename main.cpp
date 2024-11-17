#include <string.h>

#include <cmath>
#include <memory>

#include "model.h"
#include "tgaimage.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const int width = 600;
const int height = 600;
float zbuffer[width * height];

void word2screen(Vec3f v_pts[3])
{
    for (int i = 0; i < 3; i++)
    {
        v_pts[i].x = int((v_pts[i].x + 1.f) * width / 2.f);
        v_pts[i].y = int((v_pts[i].y + 1.f) * height / 2.f);
    }
}

Vec3f barycentric(Vec3f v_pts[3], Vec3f p)
{
    Vec3f u = Vec3f(v_pts[2].x - v_pts[0].x, v_pts[1].x - v_pts[0].x, v_pts[0].x - p.x) ^
              Vec3f(v_pts[2].y - v_pts[0].y, v_pts[1].y - v_pts[0].y, v_pts[0].y - p.y);
    if (std::abs(u.z) < 1)
        return Vec3f(-1, 1, 1);
    return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
}

void triangle(Vec3f v_pts[3], Vec2f uv[3], TGAImage &image, TGAImage &texture)
{
    word2screen(v_pts);
    Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    Vec2f clamp(width - 1, height - 1);
    for (int i = 0; i < 3; i++)
    {
        bboxmin.x = std::max(0.f, std::min(bboxmin.x, v_pts[i].x));
        bboxmin.y = std::max(0.f, std::min(bboxmin.y, v_pts[i].y));
        bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, v_pts[i].x));
        bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, v_pts[i].y));
    }

    Vec3f P;
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++)
    {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++)
        {
            Vec3f bc_screen = barycentric(v_pts, P);
            if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0)
                continue;
            float interpolated_z = v_pts[0].z * bc_screen.x + v_pts[1].z * bc_screen.y + v_pts[2].z * bc_screen.z;
            Vec2f interpolated_uv = uv[0] * bc_screen.x + uv[1] * bc_screen.y + uv[2] * bc_screen.z;
            int idx = int(P.x + P.y * width);

            if (zbuffer[idx] < interpolated_z)
            {
                zbuffer[idx] = interpolated_z;
                TGAColor color = texture.get((interpolated_uv.x) * texture.get_width(),
                                             (1.0 - interpolated_uv.y) * texture.get_height());
                image.set(P.x, P.y, color);
            }
        }
    }
}

int main(int argc, char **argv)
{
    TGAImage image(width, height, TGAImage::RGB);
    TGAImage texture;
    texture.read_tga_file("obj/african_head_diffuse.tga");

    memset(zbuffer, -100.0, sizeof(zbuffer));

    Model model("obj/african_head.obj");
    Vec3f light_dir(0, 0, -1); // define light_dir
    for (int i = 0; i < model.nfaces(); i++)
    {
        std::vector<int> face = model.face(i);
        std::vector<int> face_uv = model.face_uv(i);
        Vec3f v_pts[3];
        Vec2f uv[3];
        for (int j = 0; j < 3; j++)
        {
            v_pts[j] = model.vert(face[j]);
            uv[j] = model.uv(face_uv[j]);
        }
        Vec3f n = (v_pts[2] - v_pts[0]) ^ (v_pts[1] - v_pts[0]);
        n.normalize();
        float intensity = n * light_dir;
        if (intensity > 0.0)
        {
            triangle(v_pts, uv, image, texture);
        }
    }

    image.flip_vertically();
    image.write_tga_file("output.tga");
    return 0;
}
