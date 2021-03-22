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

        printf("Left Analog (X,Y): (%d, %d, %d, %d)\n", leftStickXdirection, leftStickYdirection, data[5], data[6]); // data[3] leftstick X data[4] leftstickY

        report.sThumbLX = leftStickXdirection;
        report.sThumbLY = leftStickYdirection;
        vigem_target_x360_update(client, pad, report);
    }
    vigem_target_remove(client, pad);
    vigem_target_free(pad);
    hid_exit();
}
