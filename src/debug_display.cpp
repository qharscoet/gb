#include "debug_display.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_memory_editor.h"

#include "options.h"

extern emu_options options;

#define IMGUI_IMPL_OPENGL_LOADER_GLEW

// About Desktop OpenGL function loaders:
//  Modern desktop OpenGL doesn't have a standard portable header file to load OpenGL function pointers.
//  Helper libraries are often used for this purpose! Here we are supporting a few common ones (gl3w, glew, glad).
//  You may use another loader/header of your choice (glext, glLoadGen, etc.), or chose to manually implement your own.
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h> // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h> // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h> // Initialize with gladLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD2)
#include <glad/gl.h> // Initialize with gladLoadGL(...) or gladLoaderLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
#define GLFW_INCLUDE_NONE	   // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/Binding.h> // Initialize with glbinding::Binding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
#define GLFW_INCLUDE_NONE		 // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/glbinding.h> // Initialize with glbinding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif

static GLuint bg_full = 0;
static GLuint bg_tiles = 0;
static GLuint screen = 0;

static MemoryEditor mem_edit;
static MemoryEditor mem_edit2;

static int begin, size;

// Simple helper function to load an image into a OpenGL texture with common settings
bool LoadTextureFromPixels(const uint32_t* pixels, GLuint *out_texture, int w, int h)
{
	// Create a OpenGL texture identifier
	GLuint image_texture = *out_texture;
	bool update = image_texture != 0;

	if(!update)
		glGenTextures(1, &image_texture);

	glBindTexture(GL_TEXTURE_2D, image_texture);

	// Setup filtering parameters for display
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Upload pixels into texture
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

	if(update)
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA,
						GL_UNSIGNED_BYTE, pixels);
	else
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	*out_texture = image_texture;
	return true;
}

Debug_Display::Debug_Display(Emulator &emu)
:emu(emu)
{}

Debug_Display::~Debug_Display()
{

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	glDeleteTextures(1, &bg_full);
	glDeleteTextures(1, &bg_tiles);
	glDeleteTextures(1, &screen);

	SDL_GL_DeleteContext(gl_context);
	SDL_DestroyWindow(sdlWindow);

	SDL_Quit();
}

int Debug_Display::init()
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
#if __APPLE__
	// GL 3.2 Core + GLSL 150
	const char *glsl_version = "#version 150";
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
	// GL 3.0 + GLSL 130
	const char *glsl_version = "#version 130";
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

	// Create window with graphics context
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL |  SDL_WINDOW_ALLOW_HIGHDPI);
	sdlWindow = SDL_CreateWindow("Dear ImGui SDL2+OpenGL3 example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
	gl_context = SDL_GL_CreateContext(sdlWindow);
	SDL_GL_MakeCurrent(sdlWindow, gl_context);
	SDL_GL_SetSwapInterval(1); // Enable vsync

	// Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
	bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
	bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
	bool err = gladLoadGL() == 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD2)
	bool err = gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress) == 0; // glad2 recommend using the windowing library loader instead of the (optionally) bundled one.
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
	bool err = false;
	glbinding::Binding::initialize();
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
	bool err = false;
	glbinding::initialize([](const char *name) { return (glbinding::ProcAddress)SDL_GL_GetProcAddress(name); });
#else
	bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif

	if (err)
	{
		fprintf(stderr, "Failed to initialize OpenGL loader!\n");
		return 0;
	}


	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer bindings
	ImGui_ImplSDL2_InitForOpenGL(sdlWindow, gl_context);
	ImGui_ImplOpenGL3_Init(glsl_version);

	return 1;
}

void Debug_Display::update(const uint32_t *pixels)
{
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(sdlWindow);
	ImGui::NewFrame();
	ImGuiIO &io = ImGui::GetIO();


	// Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	{

		ImGui::SetNextWindowPos({640, 360}, 0, {0.5f, 0.5f});
		ImGui::SetNextWindowSize({480, 480});
		ImGui::Begin("Emu screen", NULL, ImGuiWindowFlags_NoTitleBar  | ImGuiWindowFlags_NoMove); // Create a window called "Hello, world!" and append into it.

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

		LoadTextureFromPixels(pixels, &screen, LCD_WIDTH, LCD_HEIGHT);
		ImGui::Image((void *)(intptr_t)screen, ImVec2(LCD_WIDTH * 3, LCD_HEIGHT * 3));

		ImGui::End();
	}

	{
		ImGui::Begin("VRAM Viewer");

		ImGui::Text("LCD_CONTROL : %02X", emu.memory.read_8bits(0xFF40));
		uint32_t pixels[256 * 256];
		emu.gpu.draw_full_bg(pixels);
		LoadTextureFromPixels(pixels, &bg_full, 256, 256);
		ImGui::Image((void *)(intptr_t)bg_full, ImVec2(256, 256));
		ImGui::SameLine();

		uint32_t pixels2[128 * 192];
		emu.gpu.display_bg_tiles(pixels2);
		LoadTextureFromPixels(pixels2, &bg_tiles, 128, 192);
		ImGui::Image((void *)(intptr_t)bg_tiles, ImVec2(128, 192));

		ImGui::End();
	}

	//  Memory viewer
	{
		ImGui::Begin("Memory Editor");
		mem_edit.DrawContents(emu.memory.get_data(0x8000), 0x2000, 0x8000);
		ImGui::SameLine();
		mem_edit.DrawContents(emu.memory.get_data(0xFE00), 0x100, 0xFE00);
		ImGui::SameLine();
		mem_edit.DrawContents(emu.memory.get_data(0xFF00), 0x100, 0xFF00);
		ImGui::End();
	}
	//  Memory viewer custom
	{
		ImGui::Begin("Memory Editor 2");
		ImGui::InputInt("begin", &begin);
		ImGui::InputInt("size", &size);
		mem_edit2.DrawContents(emu.memory.get_data(begin), size, begin);
		ImGui::End();
	}

	// Debug options
	{
		ImGui::Begin("Debug options");
		ImGui::Checkbox("Pause", &options.pause);

		ImGui::End();
	}
}

void Debug_Display::set_title(std::string str)
{
	SDL_SetWindowTitle(sdlWindow, str.c_str());
}

bool Debug_Display::handle_events()
{
	SDL_Event event;

	if (SDL_PollEvent(&event))
	{
		if (event.type == SDL_QUIT || (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(sdlWindow)))
		{
			return 0;
		}

		if (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_P)
			options.pause = !options.pause;
	}

	update_keystate();
	ImGui_ImplSDL2_ProcessEvent(&event);

	return 1;
}

void Debug_Display::update_keystate()
{
	const Uint8 *state = SDL_GetKeyboardState(NULL);

	const uint8_t keys[8] = {SDL_SCANCODE_DOWN, SDL_SCANCODE_UP, SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT,
							 SDL_SCANCODE_RETURN, SDL_SCANCODE_BACKSPACE, SDL_SCANCODE_X, SDL_SCANCODE_C};
	keystate = 0;

	for (int i = 0; i < 8; i++)
	{
		if (state[keys[i]])
			keystate |= (1 << (7 - i));
	}
}

uint8_t Debug_Display::get_keystate()
{
	return keystate;
}

void Debug_Display::clear()
{

	// Our state
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
	glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
	glClear(GL_COLOR_BUFFER_BIT);
}

void Debug_Display::render()
{
	ImGui::Render();

	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	SDL_GL_SwapWindow(sdlWindow);
}