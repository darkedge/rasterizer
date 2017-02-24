#pragma once
#include "rs_texture.h"

namespace gl {
    bool Init(int width, int height);
    bool Resize(int width, int height);
    Texture GetTexture();
    void Render();
}
