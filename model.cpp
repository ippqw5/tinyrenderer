
#include "model.h"

#include <fstream>
#include <sstream>
#include <string>

Model::Model(const char *filename) : verts_(), faces_()
{
    std::ifstream in;
    in.open(filename, std::ifstream::in);
    if (in.fail())
        return;
    std::string line;
    while (!in.eof())
    {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char prefix;
        if (line.compare(0, 2, "v ") == 0)
        {
            // vertex
            iss >> prefix;
            Vec3f v;
            for (int i = 0; i < 3; i++)
                iss >> v.raw[i];
            verts_.push_back(v);
        }
        else if (line.compare(0, 3, "vt ") == 0)
        {
            // uv
            iss >> prefix >> prefix;
            Vec2f uv;
            for (int i = 0; i < 2; i++)
                iss >> uv.raw[i];
            uv_.push_back(uv);
        }
        else if (line.compare(0, 2, "f ") == 0)
        {
            // face
            std::vector<int> f;
            std::vector<int> f_uv;
            iss >> prefix;

            int trash, idx, uv_idx;
            while (iss >> idx >> prefix >> uv_idx >> prefix >> trash)
            {
                idx--; // In wavefront.obj, idx start at 1, not zero
                uv_idx--;
                f.push_back(idx);
                f_uv.push_back(uv_idx);
            }
            faces_.push_back(f);
            faces_uv_.push_back(f_uv);
        }
    }
    std::cout << "# v# " << verts_.size() << " f# " << faces_.size() << std::endl;
}

Model::~Model()
{
}

int Model::nverts()
{
    return (int)verts_.size();
}
int Model::nfaces()
{
    return (int)faces_.size();
}
Vec3f Model::vert(int i)
{
    return verts_[i];
}

Vec2f Model::uv(int i)
{
    return uv_[i];
}

std::vector<int> Model::face(int idx)
{
    return faces_[idx];
}
std::vector<int> Model::face_uv(int idx)
{
    return faces_uv_[idx];
}