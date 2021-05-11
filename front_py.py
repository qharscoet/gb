from ctypes import *
import sys
import pygame
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("filename", help=".gb or .gbc rom file to open")
args = parser.parse_args()

LCD_WIDTH = 160
LCD_HEIGHT = 144

gb_lib = cdll.LoadLibrary("./build/Release/gb_lib")
gb_lib.emu_new.restype = c_void_p
gb_lib.emu_is_exiting.restype = c_bool
gb_lib.emu_is_running.restype = c_bool
gb_lib.emu_needs_reload.restype = c_bool
gb_lib.emu_load_rom.restype = c_bool
gb_lib.emu_get_pixel_data.restype = POINTER(c_uint8)
gb_lib.emu_get_pixel_data.restype = POINTER(c_float)
gb_lib.emu_step.restype = c_uint8



emu = c_void_p(gb_lib.emu_new())
print(type(emu))
# print(gb_lib.emu_get_pixel_data)

gb_lib.emu_set_rom_file(emu,c_char_p(str.encode(args.filename)))

if not gb_lib.emu_load_rom(emu):
	print("Error opening file")
	gb_lib.emu_stop(emu)
else:
	print("starting...")
	gb_lib.emu_start(emu)


SCREEN_WIDTH = 640
SCREEN_HEIGHT = 576
pygame.init()
screen = pygame.display.set_mode((LCD_WIDTH, LCD_HEIGHT))
# buffer = string_at(gb_lib.emu_get_pixel_data(emu),  LCD_WIDTH * LCD_HEIGHT * 4)
# # print(hex(addressof(buffer.contents)))
# # print(len(string_at(buffer, LCD_WIDTH * LCD_HEIGHT * 4)))
# surface = pygame.image.frombuffer(buffer, (LCD_WIDTH, LCD_HEIGHT), 'ARGB')

while 1:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            sys.exit()

    CYCLES_BY_FRAME = 17556 * 4
    AUDIO_BUFFER_SIZE = 1024 * 2 * sizeof(c_float)
    cycles_total = 0

    while cycles_total < CYCLES_BY_FRAME:
        cycles_total += gb_lib.emu_step(emu, 0)
        # samples = gb_lib.emu_get_audio_data(emu)
        # if samples != None:
        #     pygame.mixer.Sound(buffer=string_at(samples, AUDIO_BUFFER_SIZE)).play()
        #     gb_lib.emu_clear_audio(emu)

    buffer = string_at(gb_lib.emu_get_pixel_data(emu),
                       LCD_WIDTH * LCD_HEIGHT * 4)
    # print(hex(addressof(buffer.contents)))
    # print(len(string_at(buffer, LCD_WIDTH * LCD_HEIGHT * 4)))
    surface = pygame.image.frombuffer(buffer, (LCD_WIDTH, LCD_HEIGHT), 'RGBA')
    # surface = pygame.transform.scale(surface,(SCREEN_WIDTH, SCREEN_HEIGHT))

    # screen.fill(black)
    screen.blit(surface, (0,0))
    pygame.display.flip()


gb_lib.emu_delete(emu)
