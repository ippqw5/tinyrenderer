
#include "model.h"

#include <fstream>
#include <sstream>
#include <string>

Model::Model(const char* filename) : verts_(), faces_() {
  std::ifstream in;
  in.open(filename, std::ifstream::in);
  if (in.fail()) return;
  std::string line;
  while (!in.eof()) {
    std::getline(in, line);
    std::istringstream iss(line.c_str());
    char prefix;
    if (line.compare(0, 2, "v ") == 0) {
      // vertex
      iss >> prefix;
      Vec3f v;
      for (int i = 0; i < 3; i++) iss >> v.raw[i];
      verts_.push_back(v);
    } else if (line.compare(0, 2, "f ") == 0) {
      // face
      std::vector<int> f;
      iss >> prefix;

      int trash, idx;
      while (iss >> idx >> prefix >> trash >> prefix >> trash) {
        idx--;  // In wavefront.obj, idx start at 1, not zero
        f.push_back(idx);
      }
      faces_.push_back(f);
    }
  }
  std::cout << "# v# " << verts_.size() << " f# " << faces_.size() << std::endl;
}

Model::~Model() {}

int Model::nverts() { return (int)verts_.size(); }
int Model::nfaces() { return (int)faces_.size(); }
Vec3f Model::vert(int i) { return verts_[i]; }
std::vector<int> Model::face(int idx) { return faces_[idx]; }