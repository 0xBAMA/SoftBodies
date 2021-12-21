#ifndef ENGINE
#define ENGINE

#include "includes.h"
#include "model.h"

class engine {
public:
	engine()  { init(); }
	~engine() { quit(); }

  // called from main
  bool mainLoop();

private:
  // application handles + basic data
	SDL_Window * window;
	SDL_GLContext GLcontext;
  int totalScreenWidth, totalScreenHeight;
	ImVec4 clearColor;

  // OpenGL data
  GLuint displayTexture;
  GLuint displayShader;

  // SoftBody Simulation Model
  model simulationModel;

  // initialization
  void init();
  void startMessage();
	void createWindowAndContext();
  void displaySetup();
  void simGeometrySetup();
  void computeShaderCompile();
  void imguiSetup();

  // main loop functions
  void clear();
  void mainDisplay();
  void handleEvents();
  void imguiPass();
  void showConfigWindow();
  void imguiFrameStart();
  void imguiFrameEnd();
  void drawTextEditor();
  void quitConf( bool *open );

  // shutdown procedures
  void imguiQuit();
  void SDLQuit();
	void quit();

  // program flags
	bool quitConfirm = false;
	bool pQuit = false;
};

#endif
