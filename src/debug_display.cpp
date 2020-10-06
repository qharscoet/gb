#include "debug_display.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_memory_editor.h"
#include "imgui/imfilebrowser.h"

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

static const int BUFFER_SIZE = 1024;

static float fsamples[BUFFER_SIZE * 8] = {0.0f};
static uint16_t sample_offset = 0;

static GLuint bg_full;
static GLuint bg_tiles;
static GLuint screen;

static MemoryEditor mem_edit;
static MemoryEditor mem_edit2;
static ImGui::FileBrowser fileDialog;

static bool memory_editor_open = true;
static bool vram_editor_open = true;
static bool options_open = true;
static bool vram_viewer = true;
static bool sound_viewer = true;

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
{
	bg_full = 0;
	bg_tiles = 0;
	screen = 0;
}

Debug_Display::~Debug_Display()
{

	glDeleteTextures(1, &bg_full);
	glDeleteTextures(1, &bg_tiles);
	glDeleteTextures(1, &screen);

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_GL_DeleteContext(gl_context);
	SDL_DestroyWindow(sdlWindow);

	SDL_CloseAudioDevice(audio_dev);

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
	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
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

	init_audio();

	return 1;
}

void Debug_Display::update(const uint32_t *pixels)
{
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(sdlWindow);
	ImGui::NewFrame();
	ImGuiIO &io = ImGui::GetIO();

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open"))
			{
				fileDialog.Open();
				fileDialog.SetTypeFilters({".gb"});
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Tools"))
		{
			ImGui::MenuItem("VRAM Viewer", NULL, &vram_viewer);
			ImGui::MenuItem("VRAM Editor", NULL, &vram_editor_open);
			ImGui::MenuItem("Sound Viewer", NULL, &sound_viewer);
			ImGui::MenuItem("Memory Editor", NULL, &memory_editor_open);
			ImGui::MenuItem("Options", NULL, &options_open);
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	fileDialog.Display();
	if (fileDialog.HasSelected())
	{
		emu.save();
		emu.set_rom_file(fileDialog.GetSelected().string());
		emu.reset();

		fileDialog.ClearSelected();
	}

	// Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	{

		const float screen_display_size = io.DisplaySize.y / 1.5f;
		ImGui::SetNextWindowPos({io.DisplaySize.x/2.0f, io.DisplaySize.y/2.0f}, 0, {0.5f, 0.5f});
		ImGui::SetNextWindowSize({screen_display_size, screen_display_size});
		ImGui::Begin("Emu screen", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize); // Create a window called "Hello, world!" and append into it.

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

		ImGui::SetCursorPos({screen_display_size * 0.05f, screen_display_size * 0.05f});

		LoadTextureFromPixels(pixels, &screen, LCD_WIDTH, LCD_HEIGHT);
		ImGui::Image((void *)(intptr_t)screen, ImVec2(screen_display_size * 0.90f, screen_display_size * 0.90f));

		ImGui::End();
	}

	// ImGui::ShowDemoWindow();

	if(vram_viewer)
	{
		ImGui::SetNextWindowPos({0, ImGui::GetFrameHeight()}, ImGuiCond_Appearing);
		ImGui::Begin("VRAM Viewer", &vram_viewer);

		ImGui::Text("LCD_CONTROL : %02X", emu.memory.read_8bits(0xFF40));
		uint32_t pixels[256 * 256];
		emu.gpu.draw_full_bg(pixels);
		LoadTextureFromPixels(pixels, &bg_full, 256, 256);

		float size = min(ImGui::GetWindowWidth() * 0.5f, ImGui::GetWindowHeight() * 0.9f);
		ImGui::Image((void *)(intptr_t)bg_full, ImVec2(size, size));
		ImGui::SameLine();

		uint32_t pixels2[128 * 192];
		emu.gpu.display_bg_tiles(pixels2);
		LoadTextureFromPixels(pixels2, &bg_tiles, 128, 192);
		ImGui::Image((void *)(intptr_t)bg_tiles, ImVec2(size * 0.5f, size * 0.75f));

		ImGui::End();
	}

	//  Memory viewer
	if(vram_editor_open)
	{
		ImGui::SetNextWindowPos({io.DisplaySize.x, ImGui::GetFrameHeight()}, ImGuiCond_Appearing, {1.0f, 0.0f});
		ImGui::Begin("VRAM Editor", &vram_editor_open);
		mem_edit.DrawContents(emu.memory.get_data(0x8000), 0x2000, 0x8000);
		ImGui::End();
	}
	//  Memory viewer custom
	if(memory_editor_open)
	{
		ImGui::Begin("Memory Editor", &memory_editor_open);
		ImGui::InputInt("begin", &begin, 1, 100, ImGuiInputTextFlags_CharsHexadecimal);
		ImGui::InputInt("size", &size, 1, 100, ImGuiInputTextFlags_CharsHexadecimal);
		mem_edit2.DrawContents(emu.memory.get_data(begin), size, begin);
		ImGui::End();
	}

	// Debug options
	if(options_open)
	{
		ImGui::Begin("Debug options", &options_open);
		ImGui::Checkbox("Pause", &options.pause);
		if(ImGui::Checkbox("Debug UI", &options.debug_ui)){
			options.display_changed = true;
		}

		if(ImGui::TreeNode("Sound Options"))
		{
			ImGui::Checkbox("Enable Channel 1", &options.sound.channel1);
			ImGui::Checkbox("Enable Channel 2", &options.sound.channel2);
			ImGui::Checkbox("Enable Channel 3", &options.sound.channel3);
			ImGui::Checkbox("Enable Channel 4", &options.sound.channel4);

			ImGui::TreePop();
		}

		if (ImGui::Button("Reset", ImVec2(80, 0)))
		{
			emu.save();
			emu.reset();
		}

		ImGui::End();
	}

	if(sound_viewer)
	{
		ImGui::Begin("Sound viewer", &sound_viewer);

		ImGui::PlotLines("", fsamples + (sample_offset >= BUFFER_SIZE * 4?0:BUFFER_SIZE*4), BUFFER_SIZE * 4, 0, NULL, -1.0f, 1.0f, ImVec2(ImGui::GetWindowWidth() *0.9f, 80.0f)/*, 2 * sizeof(float) /* we use only channel R*/);

		ImGui::End();
	}
}

void Debug_Display::set_title(std::string str)
{
	SDL_SetWindowTitle(sdlWindow, str.c_str());
}

bool Debug_Display::handle_events(Emulator &emu)
{
	SDL_Event event;

	if (SDL_PollEvent(&event))
	{
		if (event.type == SDL_QUIT || (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(sdlWindow)))
		{
			emu.quit();
			return 0;
		}

		if (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_P)
			options.pause = !options.pause;

		if (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_R)
			emu.reset();
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

int Debug_Display::init_audio()
{
	if (SDL_Init(SDL_INIT_AUDIO) < 0)
	{
		SDL_Log("SDL could not initialize! SDL_Error: %s", SDL_GetError());
		return 0;
	}

	SDL_AudioSpec want, have;

	SDL_memset(&want, 0, sizeof(want)); /* or SDL_zero(want) */
	want.freq = 48000;
	want.format = AUDIO_F32;
	want.channels = 2;
	want.samples = BUFFER_SIZE;
	want.callback = nullptr; /* you wrote this function elsewhere -- see SDL_AudioSpec for details */

	audio_dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_CHANNELS_CHANGE);
	if (audio_dev == 0)
	{
		SDL_Log("Failed to open audio: %s", SDL_GetError());

		for (uint8_t i = 0; i < SDL_GetNumAudioDrivers() && audio_dev == 0; ++i)
		{
			const char *driver_name = SDL_GetAudioDriver(i);
			if (SDL_AudioInit(driver_name))
			{
				SDL_Log("Audio driver failed to initialize: %s", driver_name);
				continue;
			}

			SDL_Log("trying to open device with driver : %s", driver_name);
			audio_dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_CHANNELS_CHANGE);
		}

		if (audio_dev != 0)
		{
			SDL_Log("Success");
		}
		else
		{
			SDL_Log("Failed");
			return 0;
		}
	}

	{
		if (have.format != want.format)
		{ /* we let this one thing change. */
			SDL_Log("We didn't get U8 audio format.");
		}
		SDL_PauseAudioDevice(audio_dev, 0); /* start audio playing. */
											//SDL_Delay(5000);			  /* let the audio callback play some sound for 5 seconds. */
	}

	return 1;
}

void Debug_Display::play_audio(const float *samples)
{

	while ((SDL_GetQueuedAudioSize(audio_dev)) > BUFFER_SIZE * sizeof(float) * 2)
	{
		SDL_Delay(1);
	}

	for (int i = 0; i < BUFFER_SIZE; i++){
		fsamples[i + sample_offset] = samples[i * 2];
	}
	sample_offset += BUFFER_SIZE;
	if(sample_offset == BUFFER_SIZE * 8) sample_offset = 0;

	if(SDL_QueueAudio(audio_dev, samples, BUFFER_SIZE * sizeof(float) * 2) == -1)
	{
		std::cout << SDL_GetError() << std::endl;
	}
}