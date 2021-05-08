#ifndef _EMULATOR_H_
#define _EMULATOR_H_

#include "cpu.h"
#include "gpu.h"
#include "sound.h"
#include "network.h"
#include "options.h"

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#endif

#include <string>
#include <thread>
#include <atomic>

#if defined(_WIN32)
	#ifdef EMULATOR_EXPORTS
		#define EMULATOR_API __declspec(dllexport)
	#else
		#define EMULATOR_API __declspec(dllimport)
	#endif
#elif defined(__EMSCRIPTEN__)
	#define EMULATOR_API EMSCRIPTEN_KEEPALIVE
#else
	#define EMULATOR_API
#endif

EMULATOR_API extern emu_options options;

class Debug_Display;
class EMULATOR_API Emulator
{

	friend class Debug_Display;

private:

	enum class emu_state : uint8_t {
		IDLE = 0,
		RUNNING,
		NEED_RELOAD,
		QUIT,
	};

	emu_state state;

	std::string rom_filename;

	Memory memory; //64Ko memory map
	CPU cpu;
	GPU gpu;
	Sound apu;

	uint8_t frame_count;

	uint8_t serial_byte;
	// std::thread serial_thread;
	// std::atomic<bool> serial_stop;
	void step_serial();
	void serial_run();

	bool send_byte(const uint8_t byte, bool blocking);
	bool receive_byte(uint8_t *byte, bool blocking);
	const std::string get_rom_dir() const;

public:
	Emulator();
	~Emulator();

	void init();
	bool load_rom();
	bool load_rom(std::string filename);
	void start();
	uint8_t step(uint8_t inputs);
	void quit();
	void reset();
	void stop();


	void update_rtc();

	const uint32_t* get_pixel_data() const;
	const std::string get_game_name() const;
	const bool is_gameboy_color() const;

	const float* get_audio_data() const;
	void clear_audio();

	void save() const;
	void load_save();

	void set_rom_file(std::string filename);

	//state functions
	bool is_running() const;
	bool is_exiting() const;
	bool needs_reload() const;
	void listen_network(const char* port = nullptr);
	void connect_network(const char* addr = nullptr, const char* port = nullptr);
	void close_network();
	enum network_state is_connected();

	//Some access functions to interact that wraps emu components
	//Memory
	uint8_t read_8bits(uint16_t addr) const;
	uint8_t *const get_data(uint16_t addr) const;
	uint8_t *const get_vram_data(uint16_t addr, int bank) const;
	void set_rtc(uint16_t days, uint8_t hours, uint8_t minutes, uint8_t seconds);

	//GPU
	void draw_full_bg(uint32_t *pixels) const;
	void draw_bg_tiles(uint32_t *pixels, bool bank) const;
	uint32_t get_palette_color(bool bg, uint8_t palette, uint8_t col_id) const;
};


// C API for bindings
#ifdef EMULATOR_EXPORTS
	extern "C" {
		EMULATOR_API Emulator* emu_new() { return new Emulator(); }
		EMULATOR_API void emu_delete(Emulator* emu) { delete emu; }

		EMULATOR_API void emu_init(Emulator* emu){ emu->init(); }
		EMULATOR_API void emu_reset(Emulator* emu){ emu->reset(); }
		EMULATOR_API void emu_start(Emulator* emu){ emu->start(); }
		EMULATOR_API void emu_stop(Emulator* emu){ emu->stop(); }

		EMULATOR_API bool emu_load_rom(Emulator* emu){ return emu->load_rom(); }
		EMULATOR_API bool emu_needs_reload(Emulator* emu){ return emu->needs_reload(); }
		EMULATOR_API bool emu_is_running(Emulator* emu){ return emu->is_running(); }
		EMULATOR_API bool emu_is_exiting(Emulator* emu){ return emu->is_exiting(); }
		EMULATOR_API const char* emu_get_game_name(Emulator* emu) { return emu->get_game_name().c_str();}

		EMULATOR_API void emu_set_rom_file(Emulator *emu, char *filename) { emu->set_rom_file(filename); }

		EMULATOR_API uint8_t emu_step(Emulator* emu, uint8_t inputs) { return emu->step(inputs);}
		EMULATOR_API const uint8_t* emu_get_pixel_data(Emulator* emu) { return (const uint8_t*)emu->get_pixel_data();}
		EMULATOR_API const float* emu_get_audio_data(Emulator* emu) { return emu->get_audio_data();}
		EMULATOR_API void emu_clear_audio(Emulator *emu) { emu->clear_audio(); }
		EMULATOR_API void emu_update_rtc(Emulator *emu) { emu->update_rtc(); }
	}
#endif

#endif