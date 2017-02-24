#include <malloc.h>
#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include "rs_gl.h"

unsigned int framebufferTexID[2];
GLuint fbPBO[2];
unsigned char* framedata;

static int SCRPITCH;
int ACTWIDTH, ACTHEIGHT;
static bool FULLSCREEN = false, firstframe = true;
static int width, height;

double lastftime;
static void* buffer;
LARGE_INTEGER lasttime, ticksPS;
HDC hdc;

bool createFBtexture()
{
    glGenTextures(2, framebufferTexID);
    if (glGetError()) return false;
    for (int i = 0; i < 2; i++)
    {
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
    framedata = (unsigned char*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
    if (!framedata) return false;
    memset(framedata, 0, width * height * 4);
    return (glGetError() == 0);
}

bool gl::Init(HDC hdc) {
    ::hdc = hdc;
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        return false;
    }

    //glViewport(0, 0, SCRWIDTH, SCRHEIGHT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 1, 0, 1, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glEnable(GL_TEXTURE_2D);
    glShadeModel(GL_SMOOTH);
    if (!createFBtexture()) return false;
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    //if (wglSwapIntervalEXT) wglSwapIntervalEXT(0);
    QueryPerformanceFrequency(&ticksPS);

    return true;
}

bool gl::Resize(int width, int height) {
    ::width = width;
    ::height = height;
    glViewport(0, 0, width, height);

    if (buffer) {
        _aligned_free(buffer);
    }
    buffer = _aligned_malloc(width * height * sizeof(int), 64);

    return true;
}

void gl::Render() {
    static int index = 0;
    int nextindex;
    glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
    glBindTexture(GL_TEXTURE_2D, framebufferTexID[index]);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, fbPBO[index]);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_BGRA, GL_UNSIGNED_BYTE, 0);
    nextindex = (index + 1) % 2;
    index = (index + 1) % 2;
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, fbPBO[nextindex]);
    framedata = (unsigned char*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glNormal3f(0, 0, 1);
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
    SwapBuffers(hdc);
}