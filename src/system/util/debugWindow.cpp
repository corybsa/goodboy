#include "includes/debugWindow.h"
#include "includes/texture.h"
#include <iostream>
#include <string>

DebugWindow::DebugWindow(GameBoy* gameBoy) {
    gb = gameBoy;
}

bool DebugWindow::init(char* title, int w, int h) {
    count = 0;
    bool success = Window::init(title, w, h);

    if(success == false) {
        return false;
    }

    if(TTF_Init() < 0) {
        printf("TTF failed to initialize! TTF Error: %s\n", TTF_GetError());
        success = false;
    } else {
        TTF_Font* font = TTF_OpenFont("./resources/fonts/consolasb.ttf", 18);
        texture = new Texture(font, renderer);
    }

    return success;
}

void DebugWindow::free() {
    Window::free();

    delete texture;
    texture = NULL;
}

void DebugWindow::askForBreakpoint() {
    if(isKeyboardFocused) {
        printf("testing\n");
    }
}

void DebugWindow::render() {
    if(!isMinimized) {
        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderClear(renderer);

        /*
        
        this.interruptFlags = new Label("IF: 0x00");
        this.interruptEnable = new Label("IE: 0x00");
        this.ime = new Label("IME: off");
        this.lcdc = new Label("LCDC: 0x00");
        this.ly = new Label("LY: 0x00");
        this.lcdStat = new Label("STAT: 0x00");

        */

        texture->renderText(wordToHex("AF: ", gb->cpu->registers.AF), 5, 5);
        texture->renderText(wordToHex("BC: ", gb->cpu->registers.BC), 5, 25);
        texture->renderText(wordToHex("DE: ", gb->cpu->registers.DE), 5, 45);
        texture->renderText(wordToHex("HL: ", gb->cpu->registers.HL), 5, 65);
        texture->renderText(wordToHex("SP: ", gb->cpu->registers.SP), 5, 85);
        texture->renderText(wordToHex("PC: ", gb->cpu->registers.PC), 5, 105);

        texture->renderText(byteToHex("IF: ", gb->memory->readIO(IO_INTERRUPT_FLAGS)), 120, 5);
        texture->renderText(byteToHex("IE: ", gb->memory->readIO(IO_INTERRUPT_ENABLE)), 120, 25);
        texture->renderText(boolToHex("IME: ", gb->cpu->ime), 120, 45);
        texture->renderText(byteToHex("LCDC: ", gb->memory->readIO(IO_LCDC)), 120, 65);
        texture->renderText(byteToHex("LY: ", gb->memory->readIO(IO_LY_COORDINATE)), 120, 85);
        texture->renderText(byteToHex("STAT: ", gb->memory->readIO(IO_LCD_STATUS)), 120, 105);

        texture->renderText("Add breakpoint: B", 5, 440);
        texture->renderText("Add memory watch: M", 5, 460);

        SDL_RenderPresent(renderer);
        count++;
    }
}

char* DebugWindow::wordToHex(char* info, word value) {
    int length = strlen(info);
    char* result = new char[length + 6];
    char* digits = "0123456789ABCDEF";
    auto hex_len = (sizeof(value) << 1) + length + 2;
    std::string rc(hex_len, '0');
    rc[length + 1] = 'x';
    
    for(int i = (length + 2), j = (hex_len - (length + 3)) * 4; i < hex_len; ++i, j -= 4) {
        rc[i] = digits[(value >> j) & 0x0f];
    }

    strncpy(result, rc.c_str(), hex_len + 1);
    strncpy(result, info, length);

    return result;
}

char* DebugWindow::byteToHex(char* info, byte value) {
    int length = strlen(info);
    char* result = new char[length + 6];
    char* digits = "0123456789ABCDEF";
    auto hex_len = (sizeof(value) << 1) + length + 2;
    std::string rc(hex_len, '0');
    rc[length + 1] = 'x';
    
    for(int i = (length + 2), j = (hex_len - (length + 3)) * 4; i < hex_len; ++i, j -= 4) {
        rc[i] = digits[(value >> j) & 0x0f];
    }

    strncpy(result, rc.c_str(), hex_len + 1);
    strncpy(result, info, length);

    return result;
}

char* DebugWindow::boolToHex(char* info, bool value) {
    int len = sizeof(info) + sizeof(value);
    char* chars = new char[len + 1];
    chars[len + 1] = (value ? '1' : '0');

    printf("before: %s\n", chars);

    strncpy(chars, info, sizeof(info));
    printf("after: %s\n", chars);
    return chars;
}