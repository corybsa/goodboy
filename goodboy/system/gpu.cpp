#include "includes/gpu.h"
#include "includes/globals.h"
#include <chrono>

GPU::GPU(Memory* memory, LCD* lcd) {
    this->memory = memory;
    this->lcd = lcd;
    mode = GPU_MODE_HBLANK;
}

void GPU::reset() {
    mode = GPU_MODE_HBLANK;
    scanline = 0;
    ticks = 0;
    previousCycles = 0;
    memory->writeIO(IO_LY_COORDINATE, 0);
}

void GPU::tick(unsigned long long cycles) {
    ticks += cycles - previousCycles;
    previousCycles = cycles;

    switch(mode) {
        case GPU_MODE_HBLANK:
            if(ticks >= GPU_TIMING_HBLANK) {
                // lcd->render(backgroundMap);
                scanline++;

                if(scanline == VBLANK_START) {
                    changeMode(GPU_MODE_VBLANK);
                } else {
                    changeMode(GPU_MODE_OAM);
                }

                ticks -= GPU_TIMING_HBLANK;
                memory->writeIO(IO_LY_COORDINATE, scanline);
            }

            break;
        case GPU_MODE_VBLANK:
            if(ticks >= GPU_TIMING_VBLANK) {
                scanline++;

                if(scanline > VBLANK_END) {
                    scanline = 0;
                    changeMode(GPU_MODE_OAM);

                    // calculate frame rate
                    ++frameCount;

                    unsigned long long nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

                    // update fps 15 times per second
                    if((frameCount % 4) == 0) {
                        frameRate = (float)1000000000 / (float)(nanoseconds - lastFrameTime);
                        frameTime = (float)(nanoseconds - lastFrameTime) / (float)1000000;
                    }

                    lastFrameTime = nanoseconds;
                }

                ticks -= GPU_TIMING_VBLANK;
                memory->writeIO(IO_LY_COORDINATE, scanline);
            }

            break;
        case GPU_MODE_OAM:
            if(ticks >= GPU_TIMING_OAM) {
                changeMode(GPU_MODE_VRAM);
                ticks -= GPU_TIMING_OAM;
            }

            break;
        case GPU_MODE_VRAM:
            if(ticks >= GPU_TIMING_VRAM) {
                changeMode(GPU_MODE_HBLANK);
                ticks -= GPU_TIMING_VRAM;
            }

            break;
    }
}

void GPU::updateTiles(word address, byte byte1, byte byte2) {
    if(address >= 0x1800) {
        // updateBackgroundMap();
    }

    int tileIndex = address / 16;
    int rowIndex = (address % 16) / 2;
    int pixelIndex = 0;
    int pixelValue;

    for(; pixelIndex < 8; pixelIndex++) {
        int mask = 1 << (7 - pixelIndex);
        int msb = (byte1 & mask) >> (7 - pixelIndex);
        int lsb = (byte2 & mask) >> (7 - pixelIndex);
        pixelValue = (lsb << 1) | msb;

        // if(lsb == 1 && msb == 1) {
        //     pixelValue = PIXEL_BLACK; // 11
        // } else if(lsb == 1 && msb == 0) {
        //     pixelValue = PIXEL_DARK_GRAY; // 10
        // } else if(lsb == 0 && msb == 1) {
        //     pixelValue = PIXEL_LIGHT_GRAY; // 01
        // } else {
        //     pixelValue = PIXEL_WHITE; // 00
        // }

        tiles[tileIndex][rowIndex][pixelIndex] = pixelValue;
    }
}

void GPU::changeMode(int newMode) {
    if(newMode != mode) {
        int status = memory->readIO(IO_LCD_STATUS);

        // set the mode bits in LCD_STATUS
        status = (status & ~(newMode)) | newMode;

        // set the mode interrupts bit (bits 3 through 5)
        status |= (1 << (newMode + 3));

        /*

        This signal is set to 1 if:
        ( (LY = LYC) AND (STAT.ENABLE_LYC_COMPARE = 1) ) OR
        ( (ScreenMode = 0) AND (STAT.ENABLE_HBL = 1) ) OR
        ( (ScreenMode = 2) AND (STAT.ENABLE_OAM = 1) ) OR
        ( (ScreenMode = 1) AND (STAT.ENABLE_VBL || STAT.ENABLE_OAM) )

        Bit 6 - LYC=LY Coincidence Interrupt (1=Enable) (Read/Write)
        Bit 5 - Mode 2 OAM Interrupt         (1=Enable) (Read/Write)
        Bit 4 - Mode 1 V-Blank Interrupt     (1=Enable) (Read/Write)
        Bit 3 - Mode 0 H-Blank Interrupt     (1=Enable) (Read/Write)
        Bit 2 - Coincidence Flag  (0:LYC<>LY, 1:LYC=LY) (Read Only)
        Bit 1-0 - Mode Flag       (Mode 0-3, see below) (Read Only)
            0: During H-Blank
            1: During V-Blank
            2: During Searching OAM
            3: During Transferring Data to LCD Driver

        */
        switch(newMode) {
            case GPU_MODE_HBLANK:
                if((status & 0x08) == 0x08) {
                    requestInterrupt(INTERRUPT_LCD_STAT);
                }

                break;
            case GPU_MODE_VBLANK:
                if((status & 0x10) == 0x10 || (status & 0x20) == 0x20) {
                    requestInterrupt(INTERRUPT_LCD_STAT);
                }

                break;
            case GPU_MODE_OAM:
                if((status & 0x20) == 0x20) {
                    requestInterrupt(INTERRUPT_LCD_STAT);
                }

                break;
            case GPU_MODE_VRAM:

                break;
        }

        memory->writeIO(IO_LCD_STATUS, status);
        mode = newMode;
    }
}

void GPU::requestInterrupt(int interrupt) {
    byte flags = memory->readIO(IO_INTERRUPT_FLAGS) | interrupt;
    memory->writeIO(IO_INTERRUPT_FLAGS, flags);
}

/*
    PinoBatch
    My best guess for the DMG color values to RGB is %00=#CC4, %01=#8A4, %10=#464, %11=#044
*/