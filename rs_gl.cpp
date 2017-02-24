#include <malloc.h>
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <assert.h>

#include "rs_gl.h"

static unsigned int framebufferTexID[2];
static GLuint fbPBO[2];
static Texture texture;

void CheckOpenGLError(const char* expr, const char* fname, int line) {
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        printf("OpenGL error %08x, at %s:%i - for %s\n", err, fname, line, expr);
    }
}

#ifdef _DEBUG
#define GL_TRY(expr) do { \
    expr; \
    CheckOpenGLError(#expr, __FILE__, __LINE__); \
} while (0)
#else
#define GL_TRY(expr) expr
#endif

Texture gl::GetTexture() {
    return texture;
}

unsigned char* ResizeTexturesAndBuffers(int width, int height) {
    if (glGetError()) return false;
    
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    for (int i = 0; i < 2; i++) {
        GL_TRY(glBindTexture(GL_TEXTURE_2D, framebufferTexID[i]));
        GL_TRY(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL));
        GL_TRY(glBindTexture(GL_TEXTURE_2D, 0));
        if (glGetError()) return false;
    }
    const int sizeMemory = 4 * width * height;
    GL_TRY(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, fbPBO[0]));
    GL_TRY(glBufferData(GL_PIXEL_UNPACK_BUFFER, sizeMemory, NULL, GL_STREAM_DRAW));
    GL_TRY(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, fbPBO[1]));
    GL_TRY(glBufferData(GL_PIXEL_UNPACK_BUFFER, sizeMemory, NULL, GL_STREAM_DRAW));
    GL_TRY(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
    GL_TRY(glBindTexture(GL_TEXTURE_2D, framebufferTexID[0]));
    GL_TRY(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, fbPBO[0]));
    unsigned char* framedata = (unsigned char*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
    if (!framedata) return NULL;

    // TODO: Remove after full screen drawing
    memset(framedata, 0, width * height * 4);
    //return (glGetError() == 0);
    return framedata;
}

bool gl::Init(int width, int height) {
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        return false;
    }

    // TODO: Destroy
    glGenTextures(2, framebufferTexID);
    for (int i = 0; i < 2; i++) {
        glBindTexture(GL_TEXTURE_2D, framebufferTexID[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glBindTexture(GL_TEXTURE_2D, 0);
        if (glGetError()) return false;
    }
    glGenBuffers(2, fbPBO);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 1, 0, 1, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glEnable(GL_TEXTURE_2D);

    Resize(width, height);

    return true;
}

bool gl::Resize(int width, int height) {
    texture.width = width;
    texture.height = height;
    texture.numComponents = 4;
    texture.data = ResizeTexturesAndBuffers(width, height);

    glViewport(0, 0, width, height);

    return true;
}

static int index;
void gl::Render() {
    glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
    glBindTexture(GL_TEXTURE_2D, framebufferTexID[index]);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, fbPBO[index]);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture.width, texture.height, GL_BGRA, GL_UNSIGNED_BYTE, 0);
    int nextindex = index ^ 1;
    index ^= 1;
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, fbPBO[nextindex]);
    texture.data = (unsigned char*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
    assert(texture.data);
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    //glNormal3f(0, 0, 1);
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(0.0f, 1.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(1.0f, 1.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(1.0f, 0.0f);
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(0.0f, 0.0f);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
}