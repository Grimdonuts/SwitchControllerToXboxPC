// SwitchControllerToXboxPC.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "hidapi.h"
#include "ViGEm/ViGEmClient.h"

short targetVendorId = 8406;
short productId = 42769; // can be 42771 on a newer one
hid_device* switch_controller = NULL;
uint8_t data[65];
XUSB_REPORT report;
int16_t gamepadButtons = 0;

int main()
{
	int res = hid_init();
	hid_device_info* controllerptr = hid_enumerate(targetVendorId, productId);
	switch_controller = hid_open(targetVendorId, productId, controllerptr->serial_number);

	const auto client = vigem_alloc();

	if (client == nullptr)
	{
		std::cerr << "Uh, not enough memory to do that?!" << std::endl;
		return -1;
	}
	const auto retval = vigem_connect(client);

	if (!VIGEM_SUCCESS(retval))
	{
		std::cerr << "ViGEm Bus connection failed with error code: 0x" << std::hex << retval << std::endl;
		return -1;
	}

	const auto pad = vigem_target_x360_alloc();

	const auto pir = vigem_target_add(client, pad);

	if (!VIGEM_SUCCESS(pir))
	{
		std::cerr << "Target plugin failed with error code: 0x" << std::hex << pir << std::endl;
		return -1;
	}
	XUSB_REPORT_INIT(&report);

	while (hid_read(switch_controller, data, 65) != -1) {
		int16_t leftStickXdirection = (int16_t)((data[3] - 128) << 8);
		int16_t leftStickYdirection = (int16_t)((data[4] - 128) << 8);
		int16_t rightStickXdirection = (int16_t)((data[5] - 128) << 8);
		int16_t rightStickYdirection = (int16_t)((data[6] - 128) << 8);
		if (leftStickXdirection > 0) {
			leftStickXdirection = leftStickXdirection + 255;
		}

		if (leftStickYdirection < 0) {
			leftStickYdirection = abs(leftStickYdirection);
			if (leftStickYdirection == -32768) {
				leftStickYdirection = leftStickYdirection - 1;
			}
		}
		else if (leftStickYdirection > 0) {
			leftStickYdirection = (leftStickYdirection + 255) * (-1);
		}

		if (rightStickXdirection > 0) {
			rightStickXdirection = rightStickXdirection + 255;
		}

		if (rightStickYdirection < 0) {
			rightStickYdirection = abs(rightStickYdirection);
			if (rightStickYdirection == -32768) {
				rightStickYdirection = rightStickYdirection - 1;
			}
		}
		else if (rightStickYdirection > 0) {
			rightStickYdirection = (rightStickYdirection + 255) * (-1);
		}

		gamepadButtons = gamepadButtons | data[0] << 12;
		gamepadButtons = gamepadButtons | data[1] << 4;
		printf("%d\n", data[0]);
		
		//switch (data[1]) {
		//case 1:
		//	gamepadButtons = gamepadButtons | XUSB_GAMEPAD_BACK;
		//	//back
		//	break;
		//case 2:
		//	gamepadButtons = gamepadButtons | XUSB_GAMEPAD_START;
		//	//start
		//	break;
		//case 4:
		//	gamepadButtons = gamepadButtons | XUSB_GAMEPAD_LEFT_THUMB;
		//	//left stick
		//	break;
		//case 8:
		//	gamepadButtons = gamepadButtons | XUSB_GAMEPAD_RIGHT_THUMB;
		//	//right stick
		//	break;
		//case 16:
		//	gamepadButtons = gamepadButtons | XUSB_GAMEPAD_GUIDE;
		//	//home
		//	break;
		//case 32:
		//	//share
		//	break;
		//}

		report.sThumbLX = leftStickXdirection;
		report.sThumbLY = leftStickYdirection;
		report.sThumbRX = rightStickXdirection;
		report.sThumbRY = rightStickYdirection;
		report.wButtons = gamepadButtons;

		vigem_target_x360_update(client, pad, report);
	}
	vigem_target_remove(client, pad);
	vigem_target_free(pad);
	hid_exit();
}
