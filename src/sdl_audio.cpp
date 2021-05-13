#include "sdl_audio.h"

#ifndef NDEBUG
#define SDL_Log(fmt,...) SDL_Log(fmt __VA_OPT__(, ) __VA_ARGS__)
#else
#define SDL_Log(fmt,...)
#endif

SDL_Audio::SDL_Audio()
{
}

SDL_Audio::~SDL_Audio()
{
	SDL_CloseAudioDevice(audio_dev);
	SDL_AudioQuit();
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
}


int SDL_Audio::audio_init()
{
	if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0)
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

void SDL_Audio::play_audio(const float *samples)
{
	if (audio_dev != 0)
	{
#ifndef __EMSCRIPTEN__
		while ((SDL_GetQueuedAudioSize(audio_dev)) > BUFFER_SIZE * 2 * (sizeof(float)))
		{
			SDL_Delay(1);
		}
#else
		/* Can't Delay on wasm, so when the playback is getting late we clear the queue.
			Can cause some jiterring so we allow the playback to have a full BUFFER_SIZE of delay */
		if ((SDL_GetQueuedAudioSize(audio_dev)) > BUFFER_SIZE * 2 * (sizeof(float)) *2)
		{
			SDL_ClearQueuedAudio(audio_dev);
		}
#endif

		if (SDL_QueueAudio(audio_dev, samples, BUFFER_SIZE * 2 * sizeof(float)) == -1)
		{
			SDL_Log(SDL_GetError());
		}
	}
}