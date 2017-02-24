#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#define STB_IMAGE_IMPLEMENTATION
#pragma warning(push)
#pragma warning(disable: 4244)
#include "stb_image.h"
#pragma warning(pop)

#include "rs_texture.h"
#include "rs_gl.h"
#include "rs_scene.h"

class Model {
    std::vector<float> xs;
    std::vector<float> ys;
    std::vector<float> zs;
    std::vector<float> us;
    std::vector<float> vs;
    size_t size;

public:
    void PushBack(float x, float y, float z, float u, float v) {
        xs.push_back(x);
        ys.push_back(y);
        zs.push_back(z);
        us.push_back(u);
        vs.push_back(v);
        size++;
    }
};

static Model model;
static Texture texture;

void scene::Load() {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string err;
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, "teapot/teapot.obj", "teapot/");

    if (!err.empty()) { // `err` may contain warning message.
        printf("%s\n", err.c_str());
    }

    if (!ret) {
        exit(1);
    }

    // Loop over shapes
    for (size_t s = 0; s < shapes.size(); s++) {
        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            int fv = shapes[s].mesh.num_face_vertices[f];

            // Loop over vertices in the face.
            for (size_t v = 0; v < fv; v++) {
                // access to vertex
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                float vx = attrib.vertices[3 * idx.vertex_index + 0];
                float vy = attrib.vertices[3 * idx.vertex_index + 1];
                float vz = attrib.vertices[3 * idx.vertex_index + 2];
                //float nx = attrib.normals[3 * idx.normal_index + 0];
                //float ny = attrib.normals[3 * idx.normal_index + 1];
                //float nz = attrib.normals[3 * idx.normal_index + 2];
                float tx = attrib.texcoords[2 * idx.texcoord_index + 0];
                float ty = attrib.texcoords[2 * idx.texcoord_index + 1];

                model.PushBack(vx, vy, vz, tx, ty);
            }
            index_offset += fv;

            // per-face material
            //int mat = shapes[s].mesh.material_ids[f];
        }
    }

    for (size_t i = 0; i < materials.size(); i++) {
        std::string file = "teapot/" + materials[i].diffuse_texname;
        texture.data = stbi_load(file.c_str(), &texture.width, &texture.height, &texture.numComponents, 0);
        assert(texture.data);
    }
}

void SetPixel(Texture texture, int x, int y, unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
    assert(x < texture.width);
    assert(y < texture.height);
    assert(x >= 0);
    assert(y >= 0);
    int start = y * texture.width * texture.numComponents + x * texture.numComponents;
    texture.data[start + 0] = b;
    texture.data[start + 1] = g;
    texture.data[start + 2] = r;
    texture.data[start + 3] = a;
}

void scene::Update(float dt) {
    dt;
    Texture texture = gl::GetTexture();
    for (int x = 0; x < 40; x++) {
        for (int y = 0; y < 40; y++) {
            SetPixel(texture, x, y, 255, 0, 0, 255);
        }
    }
}
