/***************************************************************************
 * gens-sdl: Gens/GS II basic SDL frontend.                                *
 * OsdFont_C64.cpp: C64 font for the Onscreen Display.                     *
 *                                                                         *
 * Copyright (c) 1982 Commodore International.                             *
 ***************************************************************************/

/***************************************************
 * Format: 8 bytes per character, 8 bits per line. *
 * 1 == opaque dot; 0 == transparent dot           *
 * MSB == left-most dot; LSB == right-most dot     *
 ***************************************************/

#include "OsdFont_C64.hpp"

namespace GensSdl {

// ISO-8859-1, with extra cp437 characters in control code positions.
// TODO: Add more symbols.
static const uint8_t C64_charset_ASCII[256][8] = {
	// 0x0000 [cp437]
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x36, 0x7F, 0x7F, 0x7F, 0x3E, 0x1C, 0x08, 0x00},
	{0x08, 0x1C, 0x3E, 0x7F, 0x3E, 0x1C, 0x08, 0x00},
	{0x18, 0x18, 0x66, 0x66, 0x18, 0x18, 0x3C, 0x00},
	{0x08, 0x1C, 0x3E, 0x7F, 0x7F, 0x1C, 0x3E, 0x00},
	{0x00, 0x3C, 0x7E, 0x7E, 0x7E, 0x7E, 0x3C, 0x00},
	{0xFF, 0xC3, 0x81, 0x81, 0x81, 0x81, 0xC3, 0xFF},
	{0x00, 0x3C, 0x7E, 0x66, 0x66, 0x7E, 0x3C, 0x00},
	{0xFF, 0xC3, 0x81, 0x99, 0x99, 0x81, 0xC3, 0xFF},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},

	// 0x0010 [cp437]
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x66, 0x66, 0x66, 0x66, 0x00, 0x00, 0x66, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x18, 0x3C, 0x7E, 0x18, 0x18, 0x18, 0x18},
	{0x00, 0x18, 0x18, 0x18, 0x18, 0x7E, 0x3C, 0x18},
	{0x00, 0x04, 0x06, 0x7F, 0x7F, 0x06, 0x04, 0x00},
	{0x00, 0x10, 0x30, 0x7F, 0x7F, 0x30, 0x10, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},

	// 0x0020
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x18, 0x18, 0x18, 0x18, 0x00, 0x00, 0x18, 0x00},
	{0x66, 0x66, 0x66, 0x00, 0x00, 0x00, 0x00, 0x00},
	// TODO: Verify # (might've been off by one?)
	{0x66, 0x66, 0xFF, 0x66, 0xFF, 0x66, 0x66, 0x00},
	{0x18, 0x3E, 0x60, 0x3C, 0x06, 0x7C, 0x18, 0x00},
	{0x62, 0x66, 0x0C, 0x18, 0x30, 0x66, 0x46, 0x00},
	{0x3C, 0x66, 0x3C, 0x38, 0x67, 0x66, 0x3F, 0x00},
	{0x06, 0x0C, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x0C, 0x18, 0x30, 0x30, 0x30, 0x18, 0x0C, 0x00},
	{0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x18, 0x30, 0x00},
	{0x00, 0x66, 0x3C, 0xFF, 0x3C, 0x66, 0x00, 0x00},
	{0x00, 0x18, 0x18, 0x7E, 0x18, 0x18, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x30},
	{0x00, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00},
	{0x00, 0x03, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x00},

	// 0x0030
	{0x3C, 0x66, 0x6E, 0x76, 0x66, 0x66, 0x3C, 0x00},
	{0x18, 0x18, 0x38, 0x18, 0x18, 0x18, 0x7E, 0x00},
	{0x3C, 0x66, 0x06, 0x0C, 0x30, 0x60, 0x7E, 0x00},
	{0x3C, 0x66, 0x06, 0x1C, 0x06, 0x66, 0x3C, 0x00},
	{0x06, 0x0E, 0x1E, 0x66, 0x7F, 0x06, 0x06, 0x00},
	{0x7E, 0x60, 0x7C, 0x06, 0x06, 0x66, 0x3C, 0x00},
	{0x3C, 0x66, 0x60, 0x7C, 0x66, 0x66, 0x3C, 0x00},
	{0x7E, 0x66, 0x0C, 0x18, 0x18, 0x18, 0x18, 0x00},
	{0x3C, 0x66, 0x66, 0x3C, 0x66, 0x66, 0x3C, 0x00},
	{0x3C, 0x66, 0x66, 0x3E, 0x06, 0x66, 0x3C, 0x00},
	{0x00, 0x00, 0x18, 0x00, 0x00, 0x18, 0x00, 0x00},
	{0x00, 0x00, 0x18, 0x00, 0x00, 0x18, 0x18, 0x30},
	{0x0E, 0x18, 0x30, 0x60, 0x30, 0x18, 0x0E, 0x00},
	{0x00, 0x00, 0x7E, 0x00, 0x7E, 0x00, 0x00, 0x00},
	{0x70, 0x18, 0x0C, 0x06, 0x0C, 0x18, 0x70, 0x00},
	{0x3C, 0x66, 0x06, 0x0C, 0x18, 0x00, 0x18, 0x00},

	// 0x0040
	{0x3C, 0x66, 0x6E, 0x6E, 0x60, 0x62, 0x3C, 0x00},
	{0x18, 0x3C, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x00},
	{0x7C, 0x66, 0x66, 0x7C, 0x66, 0x66, 0x7C, 0x00},
	{0x3C, 0x66, 0x60, 0x60, 0x60, 0x66, 0x3C, 0x00},
	{0x78, 0x6C, 0x66, 0x66, 0x66, 0x6C, 0x78, 0x00},
	{0x7E, 0x60, 0x60, 0x78, 0x60, 0x60, 0x7E, 0x00},
	{0x7E, 0x60, 0x60, 0x78, 0x60, 0x60, 0x60, 0x00},
	{0x3C, 0x66, 0x60, 0x6E, 0x66, 0x66, 0x3C, 0x00},
	{0x66, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x00},
	{0x3C, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00},
	{0x1E, 0x0C, 0x0C, 0x0C, 0x0C, 0x6C, 0x38, 0x00},
	{0x66, 0x6C, 0x78, 0x70, 0x78, 0x6C, 0x66, 0x00},
	{0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x7E, 0x00},
	{0x63, 0x77, 0x7F, 0x6B, 0x63, 0x63, 0x63, 0x00},
	{0x66, 0x76, 0x7E, 0x7E, 0x6E, 0x66, 0x66, 0x00},
	{0x3C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00},

	// 0x0050
	{0x7C, 0x66, 0x66, 0x7C, 0x60, 0x60, 0x60, 0x00},
	{0x3C, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x0E, 0x00},
	{0x7C, 0x66, 0x66, 0x7C, 0x78, 0x6C, 0x66, 0x00},
	{0x3C, 0x66, 0x60, 0x3C, 0x06, 0x66, 0x3C, 0x00},
	{0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00},
	{0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00},
	{0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x00},
	{0x63, 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63, 0x00},
	{0x66, 0x66, 0x3C, 0x18, 0x3C, 0x66, 0x66, 0x00},
	{0x66, 0x66, 0x66, 0x3C, 0x18, 0x18, 0x18, 0x00},
	{0x7E, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x7E, 0x00},
	{0x3C, 0x30, 0x30, 0x30, 0x30, 0x30, 0x3C, 0x00},
	{0x00, 0x60, 0x30, 0x18, 0x0C, 0x06, 0x03, 0x00},
	{0x3C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x3C, 0x00},
	{0x00, 0x18, 0x24, 0x42, 0x00, 0x00, 0x00, 0x00}, // (based on C64 '↑')
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF},

	// 0x0060
	{0x60, 0x30, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00}, // (based on C64 '\'')
	{0x00, 0x00, 0x3C, 0x06, 0x3E, 0x66, 0x3E, 0x00},
	{0x00, 0x60, 0x60, 0x7C, 0x66, 0x66, 0x7C, 0x00},
	{0x00, 0x00, 0x3C, 0x60, 0x60, 0x60, 0x3C, 0x00},
	{0x00, 0x06, 0x06, 0x3E, 0x66, 0x66, 0x3E, 0x00},
	{0x00, 0x00, 0x3C, 0x66, 0x7E, 0x60, 0x3C, 0x00},
	{0x00, 0x0E, 0x18, 0x3E, 0x18, 0x18, 0x18, 0x00},
	{0x00, 0x00, 0x3E, 0x66, 0x66, 0x3E, 0x06, 0x7C},
	{0x00, 0x60, 0x60, 0x7C, 0x66, 0x66, 0x66, 0x00},
	{0x00, 0x18, 0x00, 0x38, 0x18, 0x18, 0x3C, 0x00},
	{0x00, 0x06, 0x00, 0x06, 0x06, 0x06, 0x06, 0x3C},
	{0x00, 0x60, 0x60, 0x6C, 0x78, 0x6C, 0x66, 0x00},
	{0x00, 0x38, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00},
	{0x00, 0x00, 0x66, 0x7F, 0x7F, 0x6B, 0x63, 0x00},
	{0x00, 0x00, 0x7C, 0x66, 0x66, 0x66, 0x66, 0x00},
	{0x00, 0x00, 0x3C, 0x66, 0x66, 0x66, 0x3C, 0x00},

	// 0x0070
	{0x00, 0x00, 0x7C, 0x66, 0x66, 0x7C, 0x60, 0x60},
	{0x00, 0x00, 0x3E, 0x66, 0x66, 0x3E, 0x06, 0x06},
	{0x00, 0x00, 0x7C, 0x66, 0x60, 0x60, 0x60, 0x00},
	{0x00, 0x00, 0x3E, 0x60, 0x3C, 0x06, 0x7C, 0x00},
	{0x00, 0x18, 0x7E, 0x18, 0x18, 0x18, 0x0E, 0x00},
	{0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x3E, 0x00},
	{0x00, 0x00, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x00},
	{0x00, 0x00, 0x63, 0x6B, 0x7F, 0x3E, 0x36, 0x00},
	{0x00, 0x00, 0x66, 0x3C, 0x18, 0x3C, 0x66, 0x00},
	{0x00, 0x00, 0x66, 0x66, 0x66, 0x3E, 0x0C, 0x78},
	{0x00, 0x00, 0x7E, 0x0C, 0x18, 0x30, 0x7E, 0x00},
	{0x1C, 0x30, 0x30, 0x60, 0x30, 0x30, 0x1C, 0x00}, // (based on C64 '[')
	{0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18}, // (custom)
	{0x38, 0x0C, 0x0C, 0x06, 0x0C, 0x0C, 0x38, 0x00}, // (based on C64 ']')
	{0x00, 0x00, 0x20, 0x5A, 0x04, 0x00, 0x00, 0x00}, // (based on C64 '-') [TODO: Needs improvement.]
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // TODO

	// 0x0080 [special OSD characters]
	// TODO
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},

	// 0x0090
	// TODO
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},

	// 0x00A0
	// TODO (except A1, A2, A3)
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x18, 0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x00}, // (based on C64 '!')
	{0x08, 0x3E, 0x68, 0x68, 0x68, 0x3E, 0x08, 0x00}, // (based on C64 'c')
	{0x0C, 0x12, 0x30, 0x7C, 0x30, 0x62, 0xFC, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	
	// 0x00B0
	// TODO (except B6, BF)
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x3E, 0x6A, 0x6A, 0x3E, 0x0A, 0x0A, 0x0A, 0x00}, // (based on C64 'P')
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x18, 0x00, 0x18, 0x30, 0x60, 0x66, 0x3C, 0x00}, // (based on C64 '?')

	// 0x00C0
	// TODO
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},

	// 0x00D0
	// TODO
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},

	// 0x00E0
	{0x60, 0x30, 0x3C, 0x06, 0x3E, 0x66, 0x3E, 0x00}, // (based on C64 'a' and '\'')
	{0x06, 0x0C, 0x3C, 0x06, 0x3E, 0x66, 0x3E, 0x00}, // (based on C64 'a' and '\'')
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // TODO
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // TODO
	{0x24, 0x00, 0x3C, 0x06, 0x3E, 0x66, 0x3E, 0x00}, // (based on C64 'a')
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // TODO
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // TODO
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // TODO
	{0x60, 0x30, 0x3C, 0x66, 0x7E, 0x60, 0x3C, 0x00}, // (based on C64 'e' and '\'')
	{0x06, 0x0C, 0x3C, 0x66, 0x7E, 0x60, 0x3C, 0x00}, // (based on C64 'e' and '\'')
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // TODO
	{0x24, 0x00, 0x3C, 0x66, 0x7E, 0x60, 0x3C, 0x00}, // (based on C64 'e')
	{0x60, 0x30, 0x00, 0x38, 0x18, 0x18, 0x3C, 0x00}, // (based on C64 'i' and '\'')
	{0x06, 0x0C, 0x00, 0x38, 0x18, 0x18, 0x3C, 0x00}, // (based on C64 'i' and '\'')
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // TODO
	{0x24, 0x00, 0x00, 0x38, 0x18, 0x18, 0x3C, 0x00}, // (based on C64 'i')

	// 0x00F0
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // TODO
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // TODO
	{0x60, 0x30, 0x3C, 0x66, 0x66, 0x66, 0x3C, 0x00}, // (based on C64 'o' and '\'')
	{0x06, 0x0C, 0x3C, 0x66, 0x66, 0x66, 0x3C, 0x00}, // (based on C64 'o' and '\'')
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // TODO
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // TODO
	{0x24, 0x00, 0x3C, 0x66, 0x66, 0x66, 0x3C, 0x00}, // (based on C64 'o')
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // TODO
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // TODO
	{0x60, 0x30, 0x66, 0x66, 0x66, 0x66, 0x3E, 0x00}, // (based on C64 'u' and '\'')
	{0x06, 0x0C, 0x66, 0x66, 0x66, 0x66, 0x3E, 0x00}, // (based on C64 'u' and '\'')
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // TODO
	{0x24, 0x00, 0x66, 0x66, 0x66, 0x66, 0x3E, 0x00}, // (based on C64 'y')
	{0x06, 0x0C, 0x66, 0x66, 0x66, 0x3E, 0x0C, 0x78}, // (based on C64 'y' and '\'')
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // TODO
	{0x24, 0x00, 0x66, 0x66, 0x66, 0x3E, 0x0C, 0x78}, // (based on C64 'y')
};

// OsdFont struct.
const OsdFont C64_font = {
	(const uint8_t*)C64_charset_ASCII,
	// Font is 8x8.
	8, 8
};

}
