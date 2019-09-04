#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := jar-garden

include $(IDF_PATH)/make/project.mk

clean_flash:
	$(ESPTOOLPY) --port $(CONFIG_ESPTOOLPY_PORT) --baud $(CONFIG_ESPTOOLPY_BAUD) erase_flash
