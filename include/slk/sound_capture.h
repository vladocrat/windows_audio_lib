// windows_audio_lib - Windows audio library
// Copyright (C) 2026  Vladislav Milovanov
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

// Core types and enumerations
#include <slk/general.h>
#include <slk/types.h>

// Audio data structures
#include <slk/audiobuffer.h>
#include <slk/audiodata.h>
#include <slk/audioformat.h>
#include <slk/audiofile.h>
#include <slk/ringbuffer.h>

// Device abstraction
#include <slk/device.h>
#include <slk/inputdevice.h>
#include <slk/outputdevice.h>
#include <slk/deviceexplorer.h>
#include <slk/devicemanager.h>

// DSP
#include <slk/dsp/complex.h>
#include <slk/dsp/dsp.h>
#include <slk/dsp/filter.h>
#include <slk/dsp/noise.h>
#include <slk/dsp/window.h>
