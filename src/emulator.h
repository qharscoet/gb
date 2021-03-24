#ifndef _EMULATOR_H_
#define _EMULATOR_H_

#include "cpu.h"
#include "gpu.h"
#include "sound.h"

#include <string>
#include <thread>
#include <atomic>

class Emulator
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
	std::thread serial_thread;
	std::atomic<bool> serial_stop;
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
	void listen_network();
	void connect_network();
	void close_network();
	enum network_state is_connected();
};

#endif