// dear imgui: standalone example application for SDL2 + OpenGL
// If you are new to dear imgui, see examples/README.txt and documentation at the top of imgui.cpp.
// (SDL is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)
// (GL3W is a helper library to access OpenGL functions since there is no standard header to access modern OpenGL functions easily. Alternatives are GLEW, Glad, etc.)

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>


//including my graph code
#include "graph.h"


// About Desktop OpenGL function loaders:
//  Modern desktop OpenGL doesn't have a standard portable header file to load OpenGL function pointers.
//  Helper libraries are often used for this purpose! Here we are supporting a few common ones (gl3w, glew, glad).
//  You may use another loader/header of your choice (glext, glLoadGen, etc.), or chose to manually implement your own.

#if defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>    // Initialize with glewInit()
#endif

// Main code
int main(int, char**)
{
    // Setup SDL
    // (Some versions of SDL before <2.0.10 appears to have performance/stalling issues on a minority of Windows systems,
    // depending on whether SDL_INIT_GAMECONTROLLER is enabled or disabled.. updating to latest version of SDL is recommended!)
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // Decide GL+GLSL versions
    // GL 4.5 + GLSL 450
    const char* glsl_version = "#version 450";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_BORDERLESS | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("Dear ImGui SDL2+OpenGL3 example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
    bool err = glewInit() != GLEW_OK;
#endif
    if (err)
    {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);

    // Setup Platform/Renderer bindings
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Our state
    ImVec4 clear_color = ImVec4(48.0f/256.0f, 7.0f/256.0f, 17.0f/256.0f, 1.00f);


    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
            if (event.type == SDL_KEYUP  && event.key.keysym.sym == SDLK_ESCAPE)
            {
                done = true;
            }
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();

        //start control window
        static float timescale = 0.0f;
        static float gravity = 10.0f;
        static float noise_scale = 1.0f;
        static bool run_simulation = false;
        static bool show_controls = true;

        ImGui::SetNextWindowSize(ImVec2(315,465));
        ImGui::Begin("Controls", &show_controls);             

        ImGui::Checkbox("Run simulation", &run_simulation);  

        ImGui::SameLine();
        if(ImGui::Button("Single Step"))
        {
            //call the update function
        }
        
        ImGui::SameLine();
        if(ImGui::Button("Reset"))
        {
            //call the reinit function
        }

        ImGui::Separator();
        ImGui::Text("Simulation general settings");
        ImGui::Separator();
        ImGui::SetCursorPosX(25); 
        ImGui::SliderFloat("time step", &timescale, 0.0f, 0.1f);
        ImGui::SetCursorPosX(25);
        ImGui::SliderFloat("gravity", &gravity, -5.0f, 10.0f);
        ImGui::SetCursorPosX(25);
        ImGui::SliderFloat("noise scale", &noise_scale, 0, 5.0f);

        static float chassis_k;
        static float chassis_damp;

        static ImVec4 chassis_tension_color = {0x45,0x23,0x0,0xff};
        static ImVec4 chassis_compression_color = {0x0,0x23,0x43,0xff};

        ImGui::Separator();
        ImGui::Text("Chassis spring/damp constants  ");
        ImGui::SetCursorPosX(25);
        ImGui::ColorEdit4("tension", (float*)&chassis_tension_color, ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);
        ImGui::SetCursorPosX(25);
        ImGui::ColorEdit4("compression", (float*)&chassis_compression_color, ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);

        ImGui::SetCursorPosX(25);
        ImGui::SliderFloat("k", &chassis_k, 0.0f, 100.0f);
        ImGui::SetCursorPosX(25);
        ImGui::SliderFloat("damp", &chassis_damp, 0.0f, 40.0f);



        static float suspension_k;
        static float suspension_damp;
        
        static ImVec4 suspension_tension_color = {0x11,0x45,0x2,0xff};
        static ImVec4 suspension_compression_color = {0x45,0x0,0x53,0xff};

        ImGui::Separator();
        ImGui::Text("Suspension spring/damp constants"); 
        ImGui::SetCursorPosX(25);
        ImGui::ColorEdit4("tension", (float*)&suspension_tension_color, ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);
        ImGui::SetCursorPosX(25);
        ImGui::ColorEdit4("compression", (float*)&suspension_compression_color, ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);

        ImGui::SetCursorPosX(25);
        ImGui::SliderFloat("k", &suspension_k, 0.0f, 100.0f);
        ImGui::SetCursorPosX(25);
        ImGui::SliderFloat("damp", &suspension_damp, 0.0f, 40.0f);

        ImGui::Text(" "); 
        ImGui::Text(" "); 
        ImGui::Text(" "); 






        ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

        ImGui::Text("average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();

        // Rendering
        ImGui::Render();
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
