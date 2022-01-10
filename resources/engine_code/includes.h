#ifndef INCLUDES
#define INCLUDES

#include <stdio.h>

// stl includes
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <deque>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <thread>
#include <atomic>
#include <sstream>
#include <string>
#include <vector>

// iostream stuff
using std::cerr;
using std::cin;
using std::cout;
using std::endl;
using std::flush;

// pi definition - definitely sufficient precision
constexpr double pi = 3.14159265358979323846;

// MSAA count - effects OpenGL geometry evaluation
constexpr int MSAACount = 1;

// vector math library GLM
#define GLM_FORCE_SWIZZLE
#define GLM_SWIZZLE_XYZW
#include "../glm/glm.hpp"                  //general vector types
#include "../glm/gtc/matrix_transform.hpp" // for glm::ortho
#include "../glm/gtc/type_ptr.hpp"         //to send matricies gpu-side
#include "../glm/gtx/rotate_vector.hpp"
#include "../glm/gtx/transform.hpp"

// not sure as to the utility of this
#define GLX_GLEXT_PROTOTYPES

// OpenGL Loader
#include "../ocornut_imgui/gl3w.h"

// GUI library (dear ImGUI)
#include "../ocornut_imgui/TextEditor.h"
#include "../ocornut_imgui/imgui.h"
#include "../ocornut_imgui/imgui_impl_sdl.h"
#include "../ocornut_imgui/imgui_impl_opengl3.h"

// SDL includes - windowing, gl context, system info
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

// png loading library - very powerful
#include "../lodev_lodePNG/lodepng.h"

// wrapper for TinyOBJLoader
#include "../TinyOBJLoader/objLoader.h"

// shader compilation wrapper
#include "shader.h"

// coloring of CLI output
#include "colors.h"

// diamond square heightmap generation
#include "../mafford_diamond_square/diamond_square.h"

// more general noise solution
#include "../FastNoise2/include/FastNoise/FastNoise.h"

// Brent Werness' Voxel Automata Terrain
#include "../VAT/VAT.h"

// Niels Lohmann - JSON for Modern C++
#include "../nlohmann_JSON/json.hpp"
using json = nlohmann::json;

#define WIDTH 640
#define HEIGHT 480


// default colors to use
#define BLACK  glm::vec4( 0.00, 0.00, 0.00, 1.00 )

#define BLUE   glm::vec4( 0.50, 0.00, 0.00, 1.00 )
#define RED    glm::vec4( 0.00, 0.00, 0.50, 1.00 )

#define TAN    glm::vec4( 0.45, 0.35, 0.22, 1.00 )
#define YELLOW glm::vec4( 0.43, 0.36, 0.11, 1.00 )
#define BROWN  glm::vec4( 0.30, 0.20, 0.10, 1.00 )
#define GREEN  glm::vec4( 0.25, 0.28, 0.00, 1.00 )
#define STEEL  glm::vec4( 0.15, 0.24, 0.26, 1.00 )

#define G0     glm::vec4( 0.35, 0.23, 0.04, 1.00 )
#define G1     glm::vec4( 0.42, 0.46, 0.14, 1.00 )
#define BG     glm::vec4( 0.40, 0.30, 0.10, 1.00 )





#endif
