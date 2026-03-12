#include "../include/sdl.h"
#include "../include/ppu.h"
#include <string.h>
#include <stdio.h>

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;

void sdl_init() {
    // 1. 初始化 SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("[SDL Error] SDL_Init failed: %s\n", SDL_GetError());
        return;
    }
    printf("[SDL Info] SDL initialized successfully\n");
    
    // 2. 创建窗口
    window = SDL_CreateWindow("NESemulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 256, 240, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("[SDL Error] SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return;
    }
    printf("[SDL Info] Window created successfully (256x240)\n");
    
    // 3. 创建渲染器（移除 VSYNC 防止卡死）
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("[SDL Error] SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return;
    }
    printf("[SDL Info] Renderer created successfully\n");
    
    // 4. 创建纹理用于显示帧缓冲 (256x240, RGB888 格式)
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, 256, 240);
    if (!texture) {
        printf("[SDL Error] SDL_CreateTexture failed: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return;
    }
    printf("[SDL Info] Texture created successfully\n");
    printf("[SDL Info] SDL initialization complete\n");
}

bool sdl_poll_quit() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            return true;
        }
    }
    return false;
}

// 将 PPU 帧缓冲渲染到 SDL 窗口
void sdl_render_frame() {
    u16 *frame = ppu_frame_buffer();
    if (frame == NULL) {
        return; // 没有新帧，跳过渲染
    }
    
    // 将 RGB565 帧缓冲转换为 RGB888 像素数据
    u32 pixels[256 * 240];
    for (int i = 0; i < 256 * 240; i++) {
        u16 color = frame[i];
        // RGB565 格式转换：R(5bit) G(6bit) B(5bit) -> RGB888
        u8 r = ((color >> 11) & 0x1F) << 3;  // 5bit -> 8bit
        u8 g = ((color >> 5) & 0x3F) << 2;   // 6bit -> 8bit
        u8 b = (color & 0x1F) << 3;          // 5bit -> 8bit
        pixels[i] = (r << 16) | (g << 8) | b;
    }
    
    // 更新纹理
    SDL_UpdateTexture(texture, NULL, pixels, 256 * sizeof(u32));
    // 清空渲染器
    SDL_RenderClear(renderer);
    // 复制纹理到渲染器
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    // 呈现
    SDL_RenderPresent(renderer);
}

void sdl_shutdown() {
    if (texture) {
        SDL_DestroyTexture(texture);
        texture = NULL;
    }
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = NULL;
    }
    if (window) {
        SDL_DestroyWindow(window);
        window = NULL;
    }
    SDL_Quit();
}
