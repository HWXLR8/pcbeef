MCU          = atmega32u4
ARCH         = AVR8
BOARD        = MICRO
F_CPU        = 16000000
F_USB        = $(F_CPU)
OPTIMIZATION = s
TARGET       = joystick
SRC          = $(TARGET).c descriptors.c $(LUFA_SRC_USB) $(LUFA_SRC_USBCLASS)
LUFA_PATH    = LUFA
CC_FLAGS     = -DUSE_LUFA_CONFIG_HEADER -Iconfig/
LD_FLAGS     =

# Default target
all:

DMBS_LUFA_PATH ?= $(LUFA_PATH)/Build/LUFA
include $(DMBS_LUFA_PATH)/lufa-sources.mk
include $(DMBS_LUFA_PATH)/lufa-gcc.mk

DMBS_PATH      ?= $(LUFA_PATH)/Build/DMBS/DMBS
include $(DMBS_PATH)/core.mk
include $(DMBS_PATH)/gcc.mk

flash: all
	sudo avrdude -p atmega32u4 -c avr109 -P /dev/ttyACM0 -b 57600 -D -U flash:w:$(TARGET).hex
