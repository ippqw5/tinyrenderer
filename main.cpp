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
const int depth = 255;
float zbuffer[width * height];

// View Matrix
Matrix LookAt(Vec3f eye, Vec3f center, Vec3f up)
{
    Vec3f z = (eye - center).normalize();
    Vec3f x = (up ^ z).normalize();
    Vec3f y = (z ^ x).normalize();

    Matrix view = Matrix::identity(4);

    for (int i = 0; i < 3; i++)
    {
        view[0][i] = x[i];
        view[1][i] = y[i];
        view[2][i] = z[i];
        view[i][3] = -center[i];
    }

    return view;
}

// [-1, 1] -> [0, width] or [0, height]
void word2screen(Vec3f v_pts[3])
{
    for (int i = 0; i < 3; i++)
    {
        // [-1,1]->[0,2]->[0,2*width]->[0,width]
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

void triangle(Vec3f v_pts[3], Vec2f uv[3], float ity[3], TGAImage &image, Model &model)
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

            if (zbuffer[idx] < interpolated_z) // bigger z means closer to the camera
            {
                zbuffer[idx] = interpolated_z;
                TGAColor color = model.diffuse(interpolated_uv);

                image.set(P.x, P.y, color);
            }
        }
    }
}

int main(int argc, char **argv)
{
    TGAImage image(width, height, TGAImage::RGB);

    memset(zbuffer, -100.0, sizeof(zbuffer));

    Model model("obj/african_head.obj");

    Vec3f light_dir = Vec3f(0, -0.1, -1).normalize();
    Vec3f eye(0, 0, 3);
    Vec3f center(0, 0, 0);
    Vec3f up(0, 1, 0);

    // MVP transformation
    Matrix Model = Matrix::identity(4);
    Matrix View = LookAt(eye, center, up);
    Matrix Project = Matrix::identity(4);
    Project[3][2] = -1.f / (eye - center).z;

    for (int i = 0; i < model.nfaces(); i++)
    {
        std::vector<int> face = model.face(i);
        Vec3f v_pts[3];
        Vec3f v_pts_perspective[3];
        Vec2f v_uv[3];
        float v_ity[3];
        for (int j = 0; j < 3; j++)
        {
            v_pts[j] = model.vert(face[j]);
            v_pts_perspective[j] = m2v(Project * View * Model * v2m(v_pts[j]));
            v_uv[j] = model.uv(i, j);
            v_ity[j] = model.norm(i, j) * light_dir;
        }
        Vec3f n = ((v_pts[2] - v_pts[0]) ^ (v_pts[1] - v_pts[0])).normalize();
        if (n * light_dir > 0)
            triangle(v_pts_perspective, v_uv, v_ity, image, model);
    }

    image.flip_vertically();
    image.write_tga_file("output.tga");
    return 0;
}
