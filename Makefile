#-------------------------------------------------------------------------------
.SUFFIXES:
#-------------------------------------------------------------------------------

export SOH_TOP_DIR := $(CURDIR)

.PHONY: all clean ZAPDUtils StormLib libultraship soh

all: soh
	@echo "\033[92mDone!\033[0m"

ZAPDUtils:
	@echo "\033[92mBuilding $@...\033[0m"
	@$(MAKE) --no-print-directory -C $(CURDIR)/ZAPDTR/ZAPDUtils -f $(CURDIR)/ZAPDTR/ZAPDUtils/Makefile.wiiu

StormLib:
	@echo "\033[92mBuilding $@...\033[0m"
	@/opt/devkitpro/portlibs/wiiu/bin/powerpc-eabi-cmake -DCMAKE_BUILD_TYPE=Release -B $(CURDIR)/StormLib/build $(CURDIR)/StormLib
	@$(MAKE) --no-print-directory -C $(CURDIR)/StormLib/build

libultraship: ZAPDUtils StormLib
	@echo "\033[92mBuilding $@...\033[0m"
	@$(MAKE) --no-print-directory -C $(CURDIR)/libultraship -f $(CURDIR)/libultraship/Makefile.wiiu

soh: libultraship
	@echo "\033[92mBuilding $@...\033[0m"
	@$(MAKE) --no-print-directory -C $(CURDIR)/soh -f $(CURDIR)/soh/Makefile.wiiu

clean:
	@$(MAKE) --no-print-directory -C $(CURDIR)/ZAPDTR/ZAPDUtils -f $(CURDIR)/ZAPDTR/ZAPDUtils/Makefile.wiiu clean
	@rm -rf $(CURDIR)/StormLib/build
	@$(MAKE) --no-print-directory -C $(CURDIR)/libultraship -f $(CURDIR)/libultraship/Makefile.wiiu clean
	@$(MAKE) --no-print-directory -C $(CURDIR)/soh -f $(CURDIR)/soh/Makefile.wiiu clean
