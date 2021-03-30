struct emu_options
{
	bool pause;

	bool debug_ui;
	bool display_changed;
	bool cgb_disabled;

	struct sound_options {
		bool channel1;
		bool channel2;
		bool channel3;
		bool channel4;
	} sound;
};