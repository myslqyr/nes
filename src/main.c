#include <stdio.h>
#include <stdlib.h>

#include "../include/cpu.h"
#include "../include/cartridge.h"
#include "../include/disassembly.h"
#include "../include/ppu.h"
#include "../include/bus.h"
#include "../include/sdl.h"
#include <string.h>

int main() {
    init_op_table();
    debug_log_init();

    printf("输入ROM文件路径运行游戏:\n");

    char input[100];
    //scanf("%s", input);
    strcpy(input, "/home/myslqyr/czh/NES/rom/donkeykong.nes");
    bool ret = cartridge_load(input);
    if(!ret) {
        printf("failed to load rom: %s\n", input);
        return 1;
    }

    // 在ROM加载后初始化CPU，这样reset()才能读取正确的向量
    cpu_init();
    ppu_init();
    sdl_init();
    printf("开始运行游戏...\n");
    printf("按 Ctrl+C 停止\n\n");
    while(1) {
        if (sdl_poll_quit()) {
            break;
        }
        bus_clock();
        sdl_render_frame();  // 添加：每帧渲染 PPU 输出到 SDL 窗口
    }
    sdl_shutdown();
    debug_log_close();
    return 0;
}
