#include "bootpack.h"

struct FIFO8 mousefifo;

void inthandler2c(int *esp){
	unsigned char data;
	io_out8(PIC1_OCW2, 0x64);	/* IRQ-12受付完了をPIC1に通知 */
	io_out8(PIC0_OCW2, 0x62);	/* IRQ-02受付完了をPIC0に通知 */
	data = io_in8(PORT_KEYDAT);
	fifo8_put(&mousefifo, data);
	return;
}

#define KEYCMD_SENDTO_MOUSE	0xd4
#define	MOUSECMD_ENABLE	0xf4

void enable_mouse(struct MOUSE_DEC *mouseDec){
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
	mouseDec -> phase = 0;
    return;
}

int mouse_decode(struct MOUSE_DEC *mouseDec, unsigned char dat){
    if (mouseDec->phase == 0){
        if (dat == 0xfa){
            mouseDec->phase = 1;
        }
        return 0;
    }
    if (mouseDec->phase == 1){
        if ((dat & 0xc8) == 0x08){
            mouseDec->buf[0] = dat;
            mouseDec->phase = 2;
        }
        return 0;
    }
    if (mouseDec->phase == 2){
        mouseDec->buf[1] = dat;
        mouseDec->phase = 3;
        return 0;
    }
    if (mouseDec->phase == 3){
        mouseDec->buf[2] = dat;
        mouseDec->phase = 1;
        mouseDec->btn = mouseDec->buf[0] & 0x07;
        mouseDec->x = mouseDec->buf[1];
        mouseDec->y = mouseDec->buf[2];
        if ((mouseDec->buf[0] & 0x10) != 0){
            mouseDec->x |= 0xffffff00;
        }
        if ((mouseDec->buf[0] & 0x20) != 0){
            mouseDec->y |= 0xffffff00;
        }
        mouseDec->y = - mouseDec->y; //y座標は画面と逆
        return 1;
    }
    return -1;
}