#include <malloc.h>
#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include "rs_gl.h"

static unsigned int framebufferTexID[2];
static GLuint fbPBO[2];
static Texture texture;

Texture gl::GetTexture() {
    return texture;
}

unsigned char* CreateTexturesAndBuffers(int width, int height) {
    glGenTextures(2, framebufferTexID);
    if (glGetError()) return false;
    for (int i = 0; i < 2; i++) {
        glBindTexture(GL_TEXTURE_2D, framebufferTexID[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
        glBindTexture(GL_TEXTURE_2D, 0);
        if (glGetError()) return false;
    }
    const int sizeMemory = 4 * width * height;
    glGenBuffers(2, fbPBO);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, fbPBO[0]);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, sizeMemory, NULL, GL_STREAM_DRAW);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, fbPBO[1]);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, sizeMemory, NULL, GL_STREAM_DRAW);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, framebufferTexID[0]);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, fbPBO[0]);
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
    if (texture.data) {

    }
    texture.data = CreateTexturesAndBuffers(width, height);

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