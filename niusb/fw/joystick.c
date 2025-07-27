#include "joystick.h"

static uint8_t PrevJoystickHIDReportBuffer[sizeof(USB_JoystickReport_Data_t)];

USB_ClassInfo_HID_Device_t Joystick_HID_Interface = {
    .Config = {
        .InterfaceNumber = INTERFACE_ID_Joystick,
        .ReportINEndpoint = {
            .Address = JOYSTICK_EPADDR,
            .Size = JOYSTICK_EPSIZE,
            .Banks = 1,
        },
        .PrevReportINBuffer = PrevJoystickHIDReportBuffer,
        .PrevReportINBufferSize = sizeof(PrevJoystickHIDReportBuffer),
    },
};

#define NES_CLOCK PF7
#define NES_LATCH PF6
#define NES_DATA PF4
#define LED_1 PB0
#define LED_2 PD5

typedef struct {
    uint8_t a : 1;
    uint8_t b : 1;
    uint8_t select : 1;
    uint8_t start : 1;
    uint8_t up : 1;
    uint8_t down : 1;
    uint8_t left : 1;
    uint8_t right : 1;
} Buttons;

Buttons read_nes_controller(void) {
    uint8_t button_states = 0;

    // data collection pulse
    PORTF |= (1 << NES_LATCH);
    _delay_us(12);
    PORTF &= ~(1 << NES_LATCH);
    // button state captured

    // button order: A, B, Select, Start, Up, Down, Left, Right
    for (int i = 0; i < 8; i++) {
        button_states |= ((PINF & (1 << NES_DATA)) == 0) << (7 - i);

        // clock pulse
        PORTF |= (1 << NES_CLOCK);
        _delay_us(6);
        PORTF &= ~(1 << NES_CLOCK);
        _delay_us(6);
    }

    Buttons buttons = {0};
    buttons.a = (button_states & 0x80) != 0;
    buttons.b = (button_states & 0x40) != 0;
    buttons.select = (button_states & 0x20) != 0;
    buttons.start = (button_states & 0x10) != 0;
    buttons.up = (button_states & 0x08) != 0;
    buttons.down = (button_states & 0x04) != 0;
    buttons.left = (button_states & 0x02) != 0;
    buttons.right = (button_states & 0x01) != 0;
    return buttons;
}


int main(void) {
	SetupHardware();
	GlobalInterruptEnable();

	for (;;) {
		HID_Device_USBTask(&Joystick_HID_Interface);
		USB_USBTask();
	}
}

void SetupHardware(void) {
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	clock_prescale_set(clock_div_1);

    // set LATCH and CLOCK as output
    DDRF |= (1 << NES_LATCH) | (1 << NES_CLOCK);
    // set DATA as input
    DDRF &= ~(1 << NES_DATA);
    // disable LEDs
    DDRB &= ~(1 << PB0);
    DDRD &= ~(1 << PD5);

	USB_Init();
}

void EVENT_USB_Device_Connect(void) {}

void EVENT_USB_Device_Disconnect(void) {}

void EVENT_USB_Device_ConfigurationChanged(void) {
	bool ConfigSuccess = true;
	ConfigSuccess &= HID_Device_ConfigureEndpoints(&Joystick_HID_Interface);
	USB_Device_EnableSOFEvents();
}

void EVENT_USB_Device_ControlRequest(void) {
	HID_Device_ProcessControlRequest(&Joystick_HID_Interface);
}

void EVENT_USB_Device_StartOfFrame(void) {
	HID_Device_MillisecondElapsed(&Joystick_HID_Interface);
}

bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                         uint8_t* const ReportID,
                                         const uint8_t ReportType,
                                         void* ReportData,
                                         uint16_t* const ReportSize) {
	USB_JoystickReport_Data_t* JoystickReport = (USB_JoystickReport_Data_t*)ReportData;
    Buttons b = read_nes_controller();

    if (b.a) (JoystickReport->buttons |= 0x01);
    if (b.b) (JoystickReport->buttons |= 0x02);
    if (b.select) (JoystickReport->buttons |= 0x04);
    if (b.start) (JoystickReport->buttons |= 0x08);
    if (b.up) (JoystickReport->buttons |= 0x10);
    if (b.down) (JoystickReport->buttons |= 0x20);
    if (b.left) (JoystickReport->buttons |= 0x40);
    if (b.right) (JoystickReport->buttons |= 0x80);

    *ReportSize = sizeof(USB_JoystickReport_Data_t);
	return false;
}

void CALLBACK_HID_Device_ProcessHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                          const uint8_t ReportID,
                                          const uint8_t ReportType,
                                          const void* ReportData,
                                          const uint16_t ReportSize) {
	// Unused (but mandatory for the HID class driver) in this demo,
	// since there are no Host->Device reports
}
