// dear imgui: standalone example application for SDL2 + OpenGL
// If you are new to dear imgui, see examples/README.txt and documentation at the top of imgui.cpp.
// (SDL is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)
// (GL3W is a helper library to access OpenGL functions since there is no standard header to access modern OpenGL functions easily. Alternatives are GLEW, Glad, etc.)

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl3.h"
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
    SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, 8);





    // Create window with graphics context
    //
    //this is how you query the screen resolution
    SDL_DisplayMode dm;
    SDL_GetDesktopDisplayMode(0, &dm);

    //pulling these out because I'm going to try to span the whole screen with
    //the window, in a way that's flexible on different resolution screens
    int total_screen_width = dm.w;
    int total_screen_height = dm.h;

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    /* SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_BORDERLESS | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI); */
    SDL_WindowFlags window_flags = (SDL_WindowFlags)( SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("SoftBodies", 75, 75, total_screen_width-150, total_screen_height-150, window_flags);
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
    //ImVec4 clear_color = ImVec4(48.0f/256.0f, 7.0f/256.0f, 17.0f/256.0f, 1.00f);
    //ImVec4 clear_color = ImVec4(10.0f/256.0f, 99.0f/256.0f, 99.0f/256.0f, 1.00f);
    ImVec4 clear_color = ImVec4(99.0f/256.0f, 62.0f/256.0f, 10.0f/256.0f, 1.00f);


    graph g;    //the graph

    bool run_simulation = false;
    bool threaded = !true;
    bool show_controls = true;

    float theta = -0.263;
    float phi = 2.186;
    float roll = 0.035;


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



            if(event.type == SDL_KEYDOWN)
            {
                if(event.key.keysym.sym == SDLK_RIGHT)
                    phi += 0.01;
                if(event.key.keysym.sym == SDLK_LEFT)
                    phi -= 0.01;

                if(event.key.keysym.sym == SDLK_UP)
                    theta += 0.01;
                if(event.key.keysym.sym == SDLK_DOWN)
                    theta -= 0.01;

                if(event.key.keysym.sym == SDLK_RIGHTBRACKET)
                    roll += 0.01;
                if(event.key.keysym.sym == SDLK_LEFTBRACKET)
                    roll -= 0.01;

                static int index = 0;
                if(event.key.keysym.sym == SDLK_a)
                {index++; g.set_highlight_index(index); g.send_points_and_edges_to_gpu(); cout << index << endl;}

                if(event.key.keysym.sym == SDLK_s)
                {index--; g.set_highlight_index(index); g.send_points_and_edges_to_gpu(); cout << index << endl;}


                //toggle simulation running every frame
                if(event.key.keysym.sym == SDLK_SPACE)
                    run_simulation = !run_simulation;

                // if(event.key.keysym.sym == SDLK_x)
                //


            }
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();

        //ImGui::ShowDemoWindow();

        ImGui::SetNextWindowSize(ImVec2(390,550));
        ImGui::Begin("Controls", &show_controls);

       ImGui::Checkbox("Run simulation", &run_simulation);
       ImGui::Checkbox("Threaded", &threaded);

        ImGui::SameLine();
        if(ImGui::Button("Single Step"))
        {
            //call the update function
            g.update();
            g.send_points_and_edges_to_gpu();
        }

        ImGui::SameLine();
        if(ImGui::Button("Reset"))
        {
            //call the reinit function
            g.load_frame_points();
            g.send_points_and_edges_to_gpu();
        }



        ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
        if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags))
        {
            if (ImGui::BeginTabItem("Simulation"))
            {
                ImGui::Separator();
                ImGui::Separator();
                ImGui::Text("Simulation general settings");
                ImGui::Separator();
                ImGui::SetCursorPosX(20);
                ImGui::SliderFloat("time step", &g.timescale, 0.0f, 0.01f);
                ImGui::SetCursorPosX(20);
                ImGui::SliderFloat("gravity", &g.gravity, -4.0f, 4.0f);
                ImGui::SetCursorPosX(20);
                ImGui::SliderFloat("noise scale", &g.noise_scale, 0, 1.0f);
                ImGui::SetCursorPosX(20);
                ImGui::SliderFloat("noise speed", &g.noise_speed, 0, 2.0f);

                ImGui::Text(" ");
                ImGui::Separator();
                ImGui::Separator();
                ImGui::Text("Chassis spring/damp constants  ");
                ImGui::Separator();

                ImGui::SetCursorPosX(20);
                ImGui::SliderFloat("chassis k", &g.chassis_k, 0.0f, 2500.0f);
                ImGui::SetCursorPosX(20);
                ImGui::SliderFloat("chassis damp", &g.chassis_damp, 0.0f, 80.0f);

                ImGui::Text(" ");
                ImGui::Separator();
                ImGui::Separator();
                ImGui::Text("Suspension spring/damp constants");
                ImGui::Separator();

                ImGui::SetCursorPosX(20);
                ImGui::SliderFloat("suspension k", &g.suspension_k, 0.0f, 1500.0f);
                ImGui::SetCursorPosX(20);
                ImGui::SliderFloat("suspension damp", &g.suspension_damp, 0.0f, 40.0f);

                ImGui::Text(" ");
                ImGui::Text(" ");
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Rendering"))
            {
                ImGui::Separator();
                ImGui::Separator();
                ImGui::Text("Rotation");
                ImGui::Separator();

                ImGui::SetCursorPosX(20);
                ImGui::SliderFloat("phi", &phi, -6.28f, 6.28f);
                ImGui::SetCursorPosX(20);
                ImGui::SliderFloat("theta", &theta, -6.28f, 6.28f);
                ImGui::SetCursorPosX(20);
                ImGui::SliderFloat("roll", &roll, -6.28f, 6.28f);

                ImGui::Separator();

                ImGui::Text(" ");

                ImGui::SetCursorPosX(20);
                ImGui::Checkbox("Tension Color", &g.tension_color_only);
                ImGui::Text(" ");
                ImGui::Text(" ");

                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Instructions"))
            {
                ImGui::Text(" ");
                ImGui::Text("  Left/Right - rotate side to side");
                ImGui::Text("  Up/Down    - rotate up and down");
                ImGui::Text("  square brackets to roll");
                ImGui::Text(" ");
                ImGui::Text("  Space      - play/pause simulation");
                ImGui::Text(" ");
                ImGui::Text(" ");

                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }




        ImGui::SetCursorPosX(20);
        ImGui::SetCursorPosY(500);
        ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

        ImGui::SetCursorPosX(20);
        ImGui::Text("average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();



        // Rendering
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);




        auto t1 = std::chrono::high_resolution_clock::now();

        //update the softbody sim
        if(run_simulation)
        {   if(threaded)
            {
                g.threaded_update();
            }
            else
            {
                g.update();
            }
        }

        auto t2 = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();

        if(threaded)
        {
            std::cout << "threaded update took " << duration << " microseconds" << endl << std::flush;
        }
        else
        {
            std::cout << "unthreaded update took " << duration << " microseconds" << endl << std::flush;
        }




        //update the rotation values
        g.set_rotate_phi(phi);
        g.set_rotate_theta(theta);
        g.set_rotate_roll(roll);

        g.display();
        ImGui::Render();

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
