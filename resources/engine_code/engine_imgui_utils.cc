#include "engine.h"

void engine::quitConf( bool *open ) {
  if ( *open ) {
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration;
    // create centered window
    ImGui::SetNextWindowPos( ImVec2( totalScreenWidth / 2 - 120, totalScreenHeight / 2 - 25 ) );
    ImGui::SetNextWindowSize( ImVec2( 230, 55 ) );
    ImGui::Begin( "quit", open, flags );
    ImGui::Text( "Are you sure you want to quit?" );
    ImGui::Text( "  " );
    ImGui::SameLine();
    // button to cancel -> set this window's bool to false
    if ( ImGui::Button( " Cancel " ) )
      *open = false;
    ImGui::SameLine();
    ImGui::Text("      ");
    ImGui::SameLine();
    // button to quit -> set pquit to true
    if ( ImGui::Button( " Quit " ) )
      pQuit = true;
    ImGui::End();
  }
}

static void HelpMarker( const char *desc ) {
  ImGui::TextDisabled( "(?)" );
  if ( ImGui::IsItemHovered() ) {
    ImGui::BeginTooltip();
    ImGui::PushTextWrapPos( ImGui::GetFontSize() * 35.0f );
    ImGui::TextUnformatted( desc );
    ImGui::PopTextWrapPos();
    ImGui::EndTooltip();
  }
}


void engine::drawTextEditor() {
  ImGui::Begin( "Editor", NULL, 0 );
  static TextEditor editor;
  // static auto lang = TextEditor::LanguageDefinition::CPlusPlus();
  static auto lang = TextEditor::LanguageDefinition::GLSL();
  editor.SetLanguageDefinition( lang );

  auto cpos = editor.GetCursorPosition();
  // editor.SetPalette(TextEditor::GetLightPalette());
  editor.SetPalette( TextEditor::GetDarkPalette() );
  // editor.SetPalette(TextEditor::GetRetroBluePalette());

  static const char *fileToEdit = "resources/engine_code/shaders/blit.vs.glsl";
  std::ifstream t( fileToEdit );
  static bool loaded = false;
  if ( !loaded ) {
    editor.SetLanguageDefinition( lang );
    if ( t.good() ) {
      editor.SetText( std::string( ( std::istreambuf_iterator<char>(t)), std::istreambuf_iterator< char >() ) );
      loaded = true;
    }
  }

  // add dropdown for different shaders?
  ImGui::Text( "%6d/%-6d %6d lines  | %s | %s | %s | %s", cpos.mLine + 1,
    cpos.mColumn + 1, editor.GetTotalLines(),
    editor.IsOverwrite() ? "Ovr" : "Ins",
    editor.CanUndo() ? "*" : " ",
    editor.GetLanguageDefinition().mName.c_str(), fileToEdit );

  editor.Render( "Editor" );
  ImGui::End();
}

void engine::showConfigWindow() {
  ImGui::Begin( "Model Config", NULL, 0 );
  if ( ImGui::BeginTabBar( "Config Sections", ImGuiTabBarFlags_None ) ) {
    ImGui::SameLine();
    HelpMarker( "Softbody Simulation Model" );
    if ( ImGui::BeginTabItem( "Simulation" ) ) {
      ImGui::SliderFloat( "Time Scale", &simulationModel.simParameters.timeScale, 0.0f, 0.01f );
      ImGui::SliderFloat( "Gravity", &simulationModel.simParameters.gravity, -5.0f, 5.0f );
      ImGui::Text(" ");
      ImGui::SliderFloat( "Noise Amplitude", &simulationModel.simParameters.noiseAmplitudeScale, 0.0f, 1.0f );
      ImGui::SliderFloat( "Noise Speed", &simulationModel.simParameters.noiseSpeed, 0.0f, 10.0f );
      ImGui::Text(" ");
      ImGui::SliderFloat( "Chassis Node Mass", &simulationModel.simParameters.chassisNodeMass, 0.1f, 10.0f );
      ImGui::SliderFloat( "Chassis K", &simulationModel.simParameters.chassisKConstant, 0.0f, 3000.0f );
      ImGui::SliderFloat( "Chassis Damping", &simulationModel.simParameters.chassisDamping, 0.0f, 100.0f );
      ImGui::Text(" ");
      ImGui::SliderFloat( "Suspension K", &simulationModel.simParameters.suspensionKConstant, 0.0f, 3000.0f );
      ImGui::SliderFloat( "Suspension Damping", &simulationModel.simParameters.suspensionDamping, 0.0f, 100.0f );
      ImGui::EndTabItem();
    }
    if ( ImGui::BeginTabItem( "Render" ) ) {
      ImGui::Text("Geometry Toggles");
      ImGui::Separator();
      ImGui::Checkbox( "Draw Body Panels", &simulationModel.displayParameters.showChassisFaces );
      ImGui::Checkbox( "Draw Chassis Edges", &simulationModel.displayParameters.showChassisEdges );
      ImGui::Checkbox( "Draw Chassis Nodes", &simulationModel.displayParameters.showChassisNodes );
      ImGui::Checkbox( "Draw Suspension Edges", &simulationModel.displayParameters.showSuspensionEdges );
      ImGui::Text(" ");
      ImGui::Text("Drawing Mode");
      ImGui::Separator();
      ImGui::Checkbox( "Tension Color Only", &simulationModel.displayParameters.tensionColorOnly );
      ImGui::Text(" ");
      ImGui::Text("Scaling");
      ImGui::Separator();
      ImGui::SliderFloat( "Global Scale", &simulationModel.displayParameters.scale, 0.25f, 0.75f );
      ImGui::Text(" ");
      ImGui::SliderFloat( "Chassis Rescale", &simulationModel.displayParameters.chassisRescaleAmnt, 0.8f, 1.1f );
      ImGui::Text(" ");
      ImGui::SliderFloat( "Line Scale", &simulationModel.drawParameters.lineScale, 1.0f, 20.0f );
      ImGui::SliderFloat( "Outline Ratio", &simulationModel.drawParameters.outlineRatio, 0.8f, 2.0f );
      ImGui::Text(" ");
      ImGui::SliderFloat( "Point Scale", &simulationModel.drawParameters.pointScale, 0.8f, 20.0f );
      ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
  }
  ImGui::End();
}

void engine::imguiFrameStart() {
  // Start the Dear ImGui frame
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL2_NewFrame( window );
  ImGui::NewFrame();
}

void engine::imguiFrameEnd() {
  // get it ready to put on the screen
  ImGui::Render();

  // put imgui data into the framebuffer
  ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );

  // platform windows ( pop out windows )
  ImGuiIO &io = ImGui::GetIO();
  if ( io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable ) {
    SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
    SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
    SDL_GL_MakeCurrent( backup_current_window, backup_current_context );
  }
}
