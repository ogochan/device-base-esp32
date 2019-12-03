#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

MAJOR_VERSION := 00
MINOR_VERSION := 99
MICRO_VERSION := 01
BUILD_DATE := `date +%Y/%m/%d`
BUILD_TIME := `date +"%H:%M:%S(%Z)"`

PROJECT_NAME := jar-garden
PROJECT_VER := $(MAJOR_VERSION)$(MINOR_VERSION)$(MICRO_VERSION)-$(BUILD_DATE)-$(BUILD_TIME)

include $(IDF_PATH)/make/project.mk

clean_flash:
	$(ESPTOOLPY) --port $(CONFIG_ESPTOOLPY_PORT) --baud $(CONFIG_ESPTOOLPY_BAUD) erase_flash

version.h:
	@echo "#define PROJECT_NAME\t\"$(PROJECT_NAME)\"" > main/version.h
	@echo "#define APPL_VERSION\t\"$(PROJECT_VER)\"" >> main/version.h
	@echo "#define MAJOR_VERSION\t(1$(MAJOR_VERSION) - 100)" >> main/version.h
	@echo "#define MINOR_VERSION\t(1$(MINOR_VERSION) - 100)" >> main/version.h
	@echo "#define MICRO_VERSION\t(1$(MICRO_VERSION) - 100)" >> main/version.h
	@echo "#define BUILD_DATE\t\"$(BUILD_DATE)\"" >> main/version.h
	@echo "#define BUILD_TIME\t\"$(BUILD_TIME)\"" >> main/version.h
