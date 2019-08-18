#include <stdio.h>
#include "bootpack.h"

void HariMain(void){
    struct BOOTINFO *binfo = (struct BOOTINFO *) 0x0ff0;
    char *s[40], mcursor[256], keybuf[32], mousebuf[128];
	int mx, my, i;
	struct MOUSE_DEC mouseDec;

    init_gdtidt();
	init_pic();
	io_sti();
	fifo8_init(&keyfifo, 32, keybuf);
	fifo8_init(&mousefifo, 128, mousebuf);
	io_out8(PIC0_IMR, 0xf9); /* PIC1とキーボードを許可(11111001) */
	io_out8(PIC1_IMR, 0xef); /* マウスを許可(11101111) */

	init_keyboard();

	init_palette();
	init_screen8(binfo->vram, binfo->scrnx, binfo->scrny);
    mx = (binfo->scrnx - 16) / 2; /* 画面中央になるように座標計算 */
	my = (binfo->scrny - 28 - 16) / 2;
	init_mouse_cursor8(mcursor, COL8_008484);
	putfonts8_asc(binfo->vram, binfo->scrnx, 16, 32, COL8_000000, "DemiOS");
	putfonts8_asc(binfo->vram, binfo->scrnx, 15, 31, COL8_FFFFFF, "DemiOS");
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
	sprintf(s, "(%d, %d)", mx, my);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);

	enable_mouse(&mouseDec);

	for (;;) {
		io_cli();
		if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0){
			io_stihlt();
		} else {
			if (fifo8_status(&keyfifo) != 0){
				i = fifo8_get(&keyfifo);
				io_sti();
				sprintf(s, "%02X", i);
				boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 16, 15, 31);
				putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
			} else if (fifo8_status(&mousefifo) != 0){
				i = fifo8_get(&mousefifo);
				io_sti();
				if (mouse_decode(&mouseDec, i) != 0){
				    sprintf(s, "[lcr %4d %4d]", mouseDec.x, mouseDec.y);
				    if ((mouseDec.btn & 0x01) != 0){
				        s[1] = 'L';
				    }
				    if ((mouseDec.btn & 0x02) != 0){
				        s[3] = 'R';
				    }
				    if ((mouseDec.btn & 0x03) != 0){
				        s[2] = 'C';
				    }
                    boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 32, 16, 32 + 15 * 8 - 1, 31);
                    putfonts8_asc(binfo->vram, binfo->scrnx, 32, 16, COL8_FFFFFF, s);
				}
				boxfill8(binfo->vram, binfo->scrnx, COL8_008484, mx, my, mx + 15, my + 15);
				mx += mouseDec.x;
				my += mouseDec.y;
				if (mx < 0){
				    mx = 0;
				}
				if (my < 0){
				    my = 0;
				}
				if (mx > binfo->scrnx - 16){
				    mx = binfo->scrnx - 16;
				}
                if (my > binfo->scrny - 16){
                    my = binfo->scrny - 16;
                }
                sprintf(s, "(%3d, %3d)", mx, my);
                boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 0, 79, 15);
                putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
                putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
			}
		}
	}
}
