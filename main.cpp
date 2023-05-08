#include <stdio.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include <webgpu/webgpu.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_wgpu.h"
#include <iostream>

GLFWwindow* g_window;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
bool show_demo_window = true;
bool show_another_window = false;
int g_width;
int g_height;
WGPUInstance g_instance;
WGPUAdapter g_adapter;
WGPUDevice g_device;
WGPUQueue g_queue;
WGPUSurface g_surface;
WGPUTextureFormat g_surfaceFormat;
WGPURenderPipeline g_renderPipeline;
WGPUSwapChain g_swapChain;
WGPUTexture g_depthTexture;
WGPUTextureView g_depthTextureView;
WGPUTextureFormat g_depthTextureFormat;

// Function used by c++ to get the size of the html canvas
EM_JS(int, canvas_get_width, (), {
        return Module.canvas.width;
    });

// Function used by c++ to get the size of the html canvas
EM_JS(int, canvas_get_height, (), {
        return Module.canvas.height;
    });

// Function called by javascript
EM_JS(void, resizeCanvas, (), {
        js_resizeCanvas();
    });

void on_size_changed()
{
    glfwSetWindowSize(g_window, g_width, g_height);

    ImGui::SetCurrentContext(ImGui::GetCurrentContext());
}

void loop()
{
    int width = canvas_get_width();
    int height = canvas_get_height();

    if (width != g_width || height != g_height)
    {
        g_width = width;
        g_height = height;
        on_size_changed();
    }

    glfwPollEvents();

    ImGui_ImplWGPU_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    WGPUTextureView nextTexture = wgpuSwapChainGetCurrentTextureView(g_swapChain);
    if (!nextTexture) {
        std::cerr << "Cannot acquire next swap chain texture" << std::endl;
        return;
    }

    WGPUCommandEncoderDescriptor commandEncoderDesc{};
    commandEncoderDesc.label = "Command Encoder";
    WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(g_device, &commandEncoderDesc);

    WGPURenderPassDescriptor renderPassDesc{};

    WGPURenderPassColorAttachment colorAttachment;
    colorAttachment.view = nextTexture;
    colorAttachment.resolveTarget = nullptr;
    colorAttachment.loadOp = WGPULoadOp_Clear;
    colorAttachment.storeOp = WGPUStoreOp_Store;
    colorAttachment.clearValue = WGPUColor{ 0.05, 0.05, 0.05, 1.0 };
    renderPassDesc.colorAttachmentCount = 1;
    renderPassDesc.colorAttachments = &colorAttachment;

    WGPURenderPassDepthStencilAttachment depthStencilAttachment;
    depthStencilAttachment.view = g_depthTextureView;
    depthStencilAttachment.depthClearValue = 100.0f;
    depthStencilAttachment.depthLoadOp = WGPULoadOp_Clear;
    depthStencilAttachment.depthStoreOp = WGPUStoreOp_Store;
    depthStencilAttachment.depthReadOnly = false;
    depthStencilAttachment.stencilClearValue = 0;
    depthStencilAttachment.stencilLoadOp = WGPULoadOp_Clear;
    depthStencilAttachment.stencilStoreOp = WGPUStoreOp_Store;
    depthStencilAttachment.stencilReadOnly = true;

    renderPassDesc.depthStencilAttachment = &depthStencilAttachment;
    renderPassDesc.timestampWriteCount = 0;
    renderPassDesc.timestampWrites = nullptr;
    WGPURenderPassEncoder renderPass = wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDesc);

    // 1. Show a simple window.
    // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets automatically appears in a window called "Debug".
    {
        static float f = 0.0f;
        static int counter = 0;
        ImGui::Text("Hello, world!");                           // Display some text (you can use a format string too)
        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
        ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

        ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our windows open/close state
        ImGui::Checkbox("Another Window", &show_another_window);

        if (ImGui::Button("Button"))                            // Buttons return true when clicked (NB: most widgets return true when edited/activated)
            counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    }

    // 2. Show another simple window. In most cases you will use an explicit Begin/End pair to name your windows.
    if (show_another_window)
    {
        ImGui::Begin("Another Window", &show_another_window);
        ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me"))
            show_another_window = false;
        ImGui::End();
    }

    // 3. Show the ImGui demo window. Most of the sample code is in ImGui::ShowDemoWindow(). Read its code to learn more about Dear ImGui!
    if (show_demo_window)
    {
        ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver); // Normally user code doesn't need/want to call this because positions are saved in .ini file anyway. Here we just want to make the demo initial state a bit more friendly!
        ImGui::ShowDemoWindow(&show_demo_window);
    }

    ImGui::Render();

    ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), renderPass);

    wgpuRenderPassEncoderEnd(renderPass);

    WGPUCommandBufferDescriptor cmdbufDescr = {};
    WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, &cmdbufDescr);
    wgpuQueueSubmit(g_queue, 1, &command);

    wgpuTextureViewRelease(nextTexture);
    wgpuSwapChainPresent(g_swapChain);
}


int init_glfw()
{
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        return 1;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    // Open a window and create its OpenGL context
    int canvasWidth = g_width;
    int canvasHeight = g_height;
    g_window = glfwCreateWindow(canvasWidth, canvasHeight, "WebGui Demo", NULL, NULL);
    if( g_window == NULL )
    {
        fprintf( stderr, "Failed to open GLFW window.\n" );
        glfwTerminate();
        return -1;
    }

    return 0;
}


int init_imgui()
{
    // Setup Dear ImGui binding
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(g_window, true);
    ImGui_ImplWGPU_Init(g_device, 3, g_surfaceFormat, g_depthTextureFormat);

    // Setup style
    ImGui::StyleColorsDark();

    ImGuiIO& io = ImGui::GetIO();

    // Load Fonts
    io.Fonts->AddFontFromFileTTF("data/xkcd-script.ttf", 23.0f);
    io.Fonts->AddFontFromFileTTF("data/xkcd-script.ttf", 18.0f);
    io.Fonts->AddFontFromFileTTF("data/xkcd-script.ttf", 26.0f);
    io.Fonts->AddFontFromFileTTF("data/xkcd-script.ttf", 32.0f);
    io.Fonts->AddFontDefault();

    resizeCanvas();

    return 0;
}

void ObtainedWebGPUDevice(WGPURequestDeviceStatus status, WGPUDevice device, char const * message, void * userdata)
{
    g_depthTextureFormat = WGPUTextureFormat_Depth24Plus;
    g_device = device;
    g_queue = wgpuDeviceGetQueue(g_device);

    WGPUSurfaceDescriptor surfaceDesc = {};
    WGPUSurfaceDescriptorFromCanvasHTMLSelector canvasDesc = {};
    canvasDesc.chain.next = nullptr;
    canvasDesc.chain.sType = WGPUSType_SurfaceDescriptorFromCanvasHTMLSelector;
    canvasDesc.selector = "#canvas";
    surfaceDesc.nextInChain = reinterpret_cast<WGPUChainedStruct*>(&canvasDesc);

    g_surface = wgpuInstanceCreateSurface(g_instance, &surfaceDesc);
    g_surfaceFormat = wgpuSurfaceGetPreferredFormat(g_surface, g_adapter);

    WGPUSwapChainDescriptor swapDescr = {};
    swapDescr.usage = WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_CopySrc | WGPUTextureUsage_CopyDst;
    swapDescr.format = g_surfaceFormat;
    swapDescr.width = canvas_get_width();
    swapDescr.height = canvas_get_height();
    swapDescr.presentMode = WGPUPresentMode_Mailbox;
    g_swapChain = wgpuDeviceCreateSwapChain(g_device, g_surface, &swapDescr);

    // Create the depth texture
    WGPUTextureDescriptor depthTextureDesc;
    depthTextureDesc.dimension = WGPUTextureDimension_2D;
    depthTextureDesc.format = g_depthTextureFormat;
    depthTextureDesc.mipLevelCount = 1;
    depthTextureDesc.sampleCount = 1;
    depthTextureDesc.size = {
        static_cast<uint32_t>(canvas_get_width()),
        static_cast<uint32_t>(canvas_get_height()),
        static_cast<uint32_t>(1)
    };
    depthTextureDesc.usage = WGPUTextureUsage_RenderAttachment;
    depthTextureDesc.viewFormatCount = 1;
    depthTextureDesc.viewFormats = (WGPUTextureFormat*)&g_depthTextureFormat;
    g_depthTexture = wgpuDeviceCreateTexture(g_device, &depthTextureDesc);

    // Create the view of the depth texture manipulated by the rasterizer
    WGPUTextureViewDescriptor depthTextureViewDesc;
    depthTextureViewDesc.aspect = WGPUTextureAspect_DepthOnly;
    depthTextureViewDesc.baseArrayLayer = 0;
    depthTextureViewDesc.arrayLayerCount = 1;
    depthTextureViewDesc.baseMipLevel = 0;
    depthTextureViewDesc.mipLevelCount = 1;
    depthTextureViewDesc.dimension = WGPUTextureViewDimension_2D;
    depthTextureViewDesc.format = g_depthTextureFormat;
    g_depthTextureView = wgpuTextureCreateView(g_depthTexture, &depthTextureViewDesc);

    init_glfw();
    init_imgui();
}

void ObtainedWebGPUAdapter(WGPURequestAdapterStatus status, WGPUAdapter adapter, char const * message, void * userdata)
{
    g_adapter = adapter;

    WGPUDeviceDescriptor deviceDesc = {};
    wgpuAdapterRequestDevice(g_adapter, &deviceDesc, ObtainedWebGPUDevice, nullptr);
}

void initWgpu()
{
    const WGPUInstanceDescriptor instanceDescr = {};
    g_instance = wgpuCreateInstance(&instanceDescr);

    WGPURequestAdapterOptions adapterOpts = {};
    wgpuInstanceRequestAdapter(g_instance, &adapterOpts, ObtainedWebGPUAdapter, nullptr);
}

void quit()
{
    glfwTerminate();
}


extern "C" int main(int argc, char** argv)
{
    g_width = canvas_get_width();
    g_height = canvas_get_height();
    initWgpu();

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(loop, 0, 1);
#endif

    quit();

    return 0;
}
