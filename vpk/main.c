#include <stdio.h>
#include <psp2/ctrl.h>
#include <psp2/touch.h>
#include <psp2/power.h>
#include "vitastick_uapi.h"
#include "debugScreen.h"

#define printf(...) psvDebugScreenPrintf(__VA_ARGS__)

static void wait_key_press();

int main(int argc, char *argv[])
{
	int ret;

	sceTouchSetSamplingState(SCE_TOUCH_PORT_BACK, SCE_TOUCH_SAMPLING_STATE_START);
	sceTouchEnableTouchForce(SCE_TOUCH_PORT_BACK);

	psvDebugScreenInit();

	printf("vitastick by xerpi\n");

	ret = vitastick_start();
	if (ret >= 0) {
		printf("vitastick started successfully!\n");
	} else if (ret == VITASTICK_ERROR_DRIVER_ALREADY_ACTIVATED) {
		printf("vitastick is already active!\n");
	} else if (ret < 0) {
		printf("Error vitastick_start(): 0x%08X\n", ret);
		wait_key_press("X", SCE_CTRL_CROSS);
		return -1;
	}



	//scePowerSetArmClockFrequency(111);
	//scePowerSetBusClockFrequency(111);
	//scePowerSetGpuClockFrequency(111);
	//scePowerSetGpuXbarClockFrequency(111);

	wait_key_press("START + SELECT", SCE_CTRL_START | SCE_CTRL_SELECT);

	scePowerSetArmClockFrequency(266);
	scePowerSetBusClockFrequency(166);
	scePowerSetGpuClockFrequency(166);
	scePowerSetGpuXbarClockFrequency(111);

	vitastick_stop();

	return 0;
}

void wait_key_press(const char *key_desc, unsigned int key_mask)
{
	SceCtrlData pad;
	SceTouchData touch;
	int8_t ltrig, rtrig;
	SceTouchPanelInfo back_info;

	sceTouchGetPanelInfo(SCE_TOUCH_PORT_BACK, &back_info);
	uint16_t x_mid =
		(back_info.maxAaX - back_info.minAaX) / 2 + back_info.minAaX;
	uint16_t y_range = (back_info.maxAaY - back_info.minAaY);
	uint16_t y_mid = y_range + back_info.minAaY;

	printf("Press %s to exit.\n", key_desc);

	while (1) {
				sceTouchPeek(SCE_TOUCH_PORT_BACK, &touch, 1);
				ltrig = -127;
				rtrig = -127;
				for (int i = 0; i < touch.reportNum; i++)
				{
					int8_t trig_val = 0;
					if (touch.report[i].y < y_range + back_info.minAaY)
					{
						trig_val =
							(int8_t)((uint32_t)((touch.report[i].y - back_info.minAaY)) * 255 / y_range);
					}else{
						trig_val = 255;
					}

					if (touch.report[i].x < x_mid)
						ltrig += trig_val;
					if (touch.report[i].x >= x_mid)
						rtrig += trig_val;
				}
				vitastick_uploadtouchdata(ltrig, rtrig);
                //printf("%d, %d\n", ltrig, rtrig);

                sceCtrlReadBufferPositive(0, &pad, 1);
		if ((pad.buttons & key_mask) == key_mask)
			break;
		sceKernelDelayThreadCB(8 * 1000);
	}
}
