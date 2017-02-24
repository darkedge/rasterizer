#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <GLFW/glfw3.h>
#include <process.h> // _beginthreadex
#include <stdio.h>

#include "rs_gl.h"
#include "rs_scene.h"

#if 0
struct WindowEvent {
    HWND hWnd;
    UINT msg;
    WPARAM wParam;
    LPARAM lParam;
};

// http://stackoverflow.com/questions/11706985/win32-thread-safe-queue-implementation-using-native-windows-api
class EventQueue {
    CRITICAL_SECTION cs;
    std::deque<WindowEvent> queue;
    HANDLE semaphore;
public:
    EventQueue() {
        InitializeCriticalSection(&cs);
        semaphore = CreateSemaphore(NULL, 0, MAXINT, NULL);
    };

#if 0
    ~EventQueue() {
        DeleteCriticalSection(&cs);
        CloseHandle(semaphore);
    }
#endif

    void Push(const WindowEvent& ref) {
        EnterCriticalSection(&cs);
        queue.push_front(ref);
        LeaveCriticalSection(&cs);
        ReleaseSemaphore(semaphore, 1, NULL);
    };

    bool Pop(WindowEvent* ref) {
        if (WaitForSingleObject(semaphore, 0) == WAIT_TIMEOUT) {
            return false;
        }
        EnterCriticalSection(&cs);
        *ref = queue.back();
        queue.pop_back();
        LeaveCriticalSection(&cs);
        return true;
    };
};
#endif

static HWND hwnd;
static HDC hdc;
static HANDLE shouldClose;
//static EventQueue eventQueue;
static GLFWwindow* window;

unsigned int __stdcall RenderThread(void*) {
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);

    gl::Init(hdc);

    float i = 0.0f;
    float j = 0.0001f;
    while (WaitForSingleObject(shouldClose, 0) == WAIT_TIMEOUT) {
#if 0
        WindowEvent message;
        while (eventQueue.Pop(&message)) {
            // TODO
        }
#endif

        i += j;
        
        if(i > 1.0f) {
            i = 1.0f;
            j = -j;
        }
        if (i < 0.0f) {
            i = 0.0f;
            j = -j;
        }
        glClearColor(i, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(window);
    }

    return 0;
}

// GLFW Callbacks
void ErrorCallback(int32_t, const char *description) {
    printf("%s\n", description);
}
void WindowCloseCallback(GLFWwindow*) {
    SetEvent(shouldClose);
}

int main() {
    glfwSetErrorCallback(ErrorCallback);

    if (glfwInit() == 0) {
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

    window = glfwCreateWindow(800, 600, "Rasterizer", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return 1;
    }

    glfwSetWindowCloseCallback(window, WindowCloseCallback);

    shouldClose = CreateEvent(NULL, TRUE, FALSE, NULL);
    HANDLE thread = (HANDLE)_beginthreadex(NULL, 0, RenderThread, NULL, 0, NULL);

    while ((WaitForSingleObject(shouldClose, 0) == WAIT_TIMEOUT)) {
        glfwWaitEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
