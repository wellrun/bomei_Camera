#the main target

ifdef CHIP_980X
PROJ = producer_980X
endif

ifdef CHIP_37XX
PROJ = producer_37xx
endif


# Pathes and Locations
ifndef SDKPATH
SDKPATH    = .
endif

ifndef BUILDPATH
BUILDPATH   = build
endif

ifndef M3LIBPATH
M3LIBPATH   = c:/cygwin/tools
endif

TD   	= `/bin/date +%Y%m%d_%H%M%S`

DEFINE		= -DOS_ANYKA=1

ifdef CHIP_980X
DEFINE		+= -DCHIP_AK980X=1
SCATTER_FILE = -scatter scatter98.scf
CHIP        = AK980x
endif

ifdef CHIP_37XX
DEFINE		+= -DCHIP_AK37XX=1
SCATTER_FILE = -scatter scatter.scf
CHIP        = AK37xx
endif

INCLUDE     = -I./main -I./fwl -I./library/memory -I./library/burnlib -I./library/utility -I./library/filesystem -I./library/spiflash -I./library/driver

ENDIANELF	    = elf32-little

CFLAGS 	= -g -O2 -apcs /interwork -cpu 4T -W -Ec $(DEFINE) $(INCLUDE)
ASFLAGS = -keep -apcs /interwork -cpu 4T $(INCLUDE)
LDFLAGS	= -info totals,sizes -noremove -map $(SCATTER_FILE) -entry START

LIBS	= ./library/driver/drv_$(CHIP).a
LIBS	+= ./library/memory/memory.a
LIBS	+= ./library/burnlib/fha.a
LIBS	+= ./library/burnlib/fsa.a
LIBS	+= ./library/utility/utility.a
LIBS	+= ./library/filesystem/fat32fs.a
LIBS	+= ./library/filesystem/mtdlib.a
LIBS	+= ./library/filesystem/mount.a
LIBS	+= ./library/spiflash/spi_fs.a


# Tools
CC		    = armcc
AS	     	= armasm
LD	     	= armlink
RM         	= rm -rf
MKDIR      	= mkdir
OBJDUMP     = objdump
OBJCOPY	    = fromelf

SOBJS       = 	$(BUILDPATH)/head.o
COBJS       = 	$(BUILDPATH)/main.o $(BUILDPATH)/transc.o \
                $(BUILDPATH)/Fwl_osMalloc.o $(BUILDPATH)/fwl_osfs.o \
                $(BUILDPATH)/fwl_spiflash.o $(BUILDPATH)/fwl_nandflash.o $(BUILDPATH)/fwl_emmc.o $(BUILDPATH)/fwl_usb_transc.o \

#build rule
.PHONY: clean makedirs maketarget debug

debug:
	$(MAKE) makedirs  BUILDPATH=$(BUILDPATH)/deb
	$(MAKE) maketarget  BUILDPATH=$(BUILDPATH)/deb

release:
	$(MAKE) makedirs  BUILDPATH=$(BUILDPATH)/rel
	$(MAKE) maketarget CFG=Release BUILDPATH=$(BUILDPATH)/rel 

		
makedirs: $(BUILDPATH) 

maketarget: $(BUILDPATH)/$(PROJ).elf $(BUILDPATH)/$(PROJ).bin

$(BUILDPATH):
	-@$(MKDIR) -p $@

	
$(BUILDPATH)/$(PROJ).bin:  $(BUILDPATH)/$(PROJ).elf
	@echo ---------------------[build bin binary]----------------------------------	
	$(OBJCOPY) -bin $(BUILDPATH)/$(PROJ).elf -output $(PROJ).bin
	

$(BUILDPATH)/$(PROJ).elf:  $(SOBJS) $(COBJS) $(LIBS)
	@echo ---------------------[build out]----------------------------------
	$(LD) $(LDFLAGS) -o $(BUILDPATH)/$(PROJ).elf $(SOBJS) $(COBJS) $(LIBS)
	$(OBJCOPY)  -text -c -s $(BUILDPATH)/$(PROJ).elf -output $(PROJ).txt
	
clean : 
	-@$(RM) $(BUILDPATH) *.bin *.txt

help:
	@echo "Usage:   make [TARGET] [VARIABLE=XXX]"
	@echo "TARGET:" 
	@echo "         debug:   Builds a debug version, default target"
	@echo "         release: Builds a release version"
	@echo "         clean:   Remove all created objects "
	@echo "VARIABLES:" 
	@echo "         PLATFORM: like CI2401, AI2301 etc."
	@echo "         CHIP    : like AK3223, AK3224, AK322L"	

vpath %.S ./main
vpath %.c ./main
vpath %.c ./fwl

#Rules ----------------------------- c -> d
$(BUILDPATH)/%.o: %.c
	@echo ---------------------[$<]---------------------
	$(CC) -c $(CFLAGS)  -o $@ $<
	
$(BUILDPATH)/%.o: %.S
	@echo ---------------------[$<]----------------------------------
	$(CC) $(CFLAGS) -E -o $(BUILDPATH)/preproc.s $<	
	$(AS) $(ASFLAGS) -o $@ $(BUILDPATH)/preproc.s
	$(RM) $(BUILDPATH)/preproc.s
