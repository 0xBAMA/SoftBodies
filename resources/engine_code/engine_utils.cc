#include "engine.h"

bool engine::mainLoop() {

  // one or both of these interesting?
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls



  /* ---------------------------------------------------------------------------
    I have to think about how this is going to be structured, with multiple threads need
  to make sure that all the worker threads have finished, and then send the new vertex
  data to the GPU to render, with the data after the simulation update is completed

  the way this will work:
    - make sure the simulation update is finished, for all threads - all threads should
      be in the waiting state
    - update the contents of the VBO
    - render the output, the same way as was done in the old iteration of the project

    I want to get this working first, just as practice with multithreading concepts, and
  then I want to figure out doing it entirely on the GPU - once this is taking place on
  the GPU, I can do all of the rendering straight out of the same SSBO that is being used
  to compute the physics and skip the VBO buffering entirely
  --------------------------------------------------------------------------- */


  // compute passes
    // invoke any shaders you want to use to do work on the GPU


  // clear color and depth info
  clear();

  // fullscreen triangle copying the image
  mainDisplay();

  // run the display function for the model data
  simulationModel.display();

  // do all the gui stuff
  imguiPass();

  // swap the double buffers to present
  SDL_GL_SwapWindow( window );

  // handle all events
  handleEvents();

  // break main loop when pQuit turns true
  return pQuit;
}

void engine::clear() {
  // clear the screen
  glClearColor( clearColor.x, clearColor.y, clearColor.z, clearColor.w ); // from hsv picker
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

void engine::mainDisplay() {
  // texture display
  glUseProgram( displayShader );
  glBindVertexArray( displayVAO );
  glBindBuffer( GL_ARRAY_BUFFER, displayVBO );

  ImGuiIO &io = ImGui::GetIO();
  glUniform2f( glGetUniformLocation( displayShader, "resolution"), io.DisplaySize.x, io.DisplaySize.y );
  glDrawArrays( GL_TRIANGLES, 0, 3 ); // the fullscreen triangle
}

void engine::imguiPass() {
  // start the imgui frame
  imguiFrameStart();

  // settings window, for the simulation parameters

  // show quit confirm window
  quitConf( &quitConfirm );

  // finish up the imgui stuff and put it in the framebuffer
  imguiFrameEnd();
}


void engine::handleEvents() {
  SDL_Event event;
  while ( SDL_PollEvent( &event ) ) {
    // imgui event handling
    ImGui_ImplSDL2_ProcessEvent( &event );

    if ( event.type == SDL_QUIT )
      pQuit = true;

    if ( event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID( window ) )
      pQuit = true;

    if ( ( event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE) || ( event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_X1 ))
      quitConfirm = !quitConfirm; // x1 is browser back on the mouse

    if ( event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE && SDL_GetModState() & KMOD_SHIFT )
      pQuit = true; // force quit on shift+esc ( bypasses confirm window )


    // keyboard controls for model orientation



  }
}
