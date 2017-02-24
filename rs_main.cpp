#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <GLFW/glfw3.h>
#include <process.h> // _beginthreadex
#include <stdio.h>

#include "rs_gl.h"
#include "rs_scene.h"

#define WIDTH 800
#define HEIGHT 600

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
static HANDLE windowClose;
static HANDLE resize;
static CRITICAL_SECTION cs_resize;
//static EventQueue eventQueue;
static int width, height;
static GLFWwindow* window;
static LARGE_INTEGER lastTime, perfFreq;

float CalculateDeltaTime() {
    LARGE_INTEGER currentTime;
    QueryPerformanceCounter(&currentTime);
    float deltaTime = float(currentTime.QuadPart - lastTime.QuadPart) / float(perfFreq.QuadPart);
    // Breakpoint guard
    if (deltaTime > 1.0f) {
        deltaTime = 1.0f / 60.0f;
    }
    lastTime = currentTime;
    return deltaTime;
}

unsigned int __stdcall RenderThread(void*) {
    // Thread name
#ifdef _DEBUG
    const DWORD MS_VC_EXCEPTION = 0x406D1388;
#pragma pack(push,8)  
    typedef struct tagTHREADNAME_INFO
    {
        DWORD dwType; // Must be 0x1000.  
        LPCSTR szName; // Pointer to name (in user addr space).  
        DWORD dwThreadID; // Thread ID (-1=caller thread).  
        DWORD dwFlags; // Reserved for future use, must be zero.  
    } THREADNAME_INFO;
#pragma pack(pop)  
    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = "Render Thread";
    info.dwThreadID = -1;
    info.dwFlags = 0;
#pragma warning(push)
#pragma warning(disable: 6320 6322)
    __try {
        RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {}
#pragma warning(pop)
#endif
    glfwMakeContextCurrent(window);
    scene::Load();
    QueryPerformanceFrequency(&perfFreq);
    QueryPerformanceCounter(&lastTime);
    
    while (WaitForSingleObject(windowClose, 0) == WAIT_TIMEOUT) {
        // Resize
        if (WaitForSingleObject(resize, 0) == WAIT_OBJECT_0) {
            EnterCriticalSection(&cs_resize);
            gl::Resize(width, height);
            LeaveCriticalSection(&cs_resize);
        }

        // TODO: Remove after full-screen drawing
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        scene::Update(CalculateDeltaTime());
        gl::Render();
        glfwSwapBuffers(window);
    }

    return 0;
}

// GLFW Callbacks
void ErrorCallback(int32_t, const char *description) {
    printf("%s\n", description);
}
void WindowCloseCallback(GLFWwindow*) {
    SetEvent(windowClose);
}
void FramebufferSizeCallback(GLFWwindow*, int x, int y) {
    EnterCriticalSection(&cs_resize);
    width = x;
    height = y;
    LeaveCriticalSection(&cs_resize);
    SetEvent(resize);
}

int main() {
    glfwSetErrorCallback(ErrorCallback);

    if (glfwInit() == 0) {
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Rasterizer", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);
    gl::Init(WIDTH, HEIGHT);
    glfwMakeContextCurrent(NULL);

    glfwSetWindowCloseCallback(window, WindowCloseCallback);
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);

    windowClose = CreateEvent(NULL, TRUE, FALSE, NULL);
    resize = CreateEvent(NULL, FALSE, FALSE, NULL);
    InitializeCriticalSection(&cs_resize);
    HANDLE thread = (HANDLE) _beginthreadex(NULL, 0, RenderThread, NULL, 0, NULL);

    while ((WaitForSingleObject(windowClose, 0) == WAIT_TIMEOUT)) {
        glfwWaitEvents();
    }
    WaitForSingleObject(thread, INFINITE);

    CloseHandle(windowClose);
    CloseHandle(resize);
    DeleteCriticalSection(&cs_resize);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
