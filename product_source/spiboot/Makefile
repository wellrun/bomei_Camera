##########################################################################
# complie and linker
##########################################################################
	CC			= arm-elf-gcc
	AS	     	= arm-elf-as
	LD 		 	= arm-elf-ld
	OBJDUMP      = arm-elf-objdump
	OBJCOPY	     = arm-elf-objcopy
#TEXT         = 0x2FFFFE00
    TEXT         = 0x30770000

##########################################################################

    DEFINE		= -DOS_ANYKA=1

    ifdef MTD_NFTL
    DEFINE := $(DEFINE) -DMTD_NFTL=1
    endif
    
	  DEFINE		:= $(DEFINE) -DFREE_MEM_START_ADDR=$(FREE_MEM_START_ADDR)

    ifdef DEBUG_OUTPUT
    ifeq ($(DEBUG_OUTPUT), Y)
    DEFINE		:= $(DEFINE) -DDEBUG_OUTPUT=1
    endif
    endif

   
    
    ifdef CHIP_980X
    DEFINE := $(DEFINE) -DCHIP_980X=1
    PROJ	= Nandboot_980X
#   SCATTER_FILE = -scatter scatter98.scf
    endif
    
    ifdef CHIP_37XX
    DEFINE := $(DEFINE) -DCHIP_37XX=1
    PROJ	= spiboot_37XX
#    SCATTER_FILE = -scatter scatter37.scf
    endif
    
##########################################################################
# FILES
##########################################################################
	SRC 	    = .
	SOBJS		= rom.o mmulib.o
	COBJS	    = main.o uart.o spi.o spiflash.o l2.o fwl_spiflash.o freq.o memapi.o mmu.o
	
	CFLAGS		= -fno-builtin -mlittle-endian -mlong-calls -nostdlib -g -O2 $(DEFINE)
	ASFLAGS     =  -x assembler-with-cpp $(DEFINE)
	LDFLAGS     = -e_start -EL -N -p -X -Ttext $(TEXT) \
			$(TOOLLDFLAGS) -n -nostartfiles -L. -T boot.ld
	LIBS		= /tools/H-i686-pc-cygwin/arm-elf/lib/libc.a  \
		 /tools/H-i686-pc-cygwin/lib/gcc-lib/arm-elf/3.0/libgcc.a

  ifdef COMPRESS
  ifeq ($(COMPRESS), Y)
  DEFINE		:= $(DEFINE) -DCOMPRESS=1
  LIBS		:= $(LIBS)	 ./inflate.a
  COBJS		:= $(COBJS)	 Fwl_compress.o
  endif
  endif
	ENDIANELF	= elf32-little
##########################################################################
TARGET	=  $(PROJ).elf  $(PROJ).bin
build:$(TARGET)
 
$(PROJ).bin: $(PROJ).elf
	$(OBJCOPY) -O binary $(PROJ).elf $(PROJ).bin

$(PROJ).elf: $(SOBJS) $(COBJS)
	$(LD) $(LDFLAGS) -o $(PROJ).elf $(SOBJS) $(COBJS) $(LIBS)
	$(OBJDUMP) -h -S -l -D -Mreg-names-raw --show-raw-insn $(PROJ).elf > $(PROJ)_elf.txt

.s.o:
	$(CC) -c $(ASFLAGS) -o $@  $<
.c.o:
	$(CC) -c $(CFLAGS)  $<	

clean:
	-rm *.o  *.elf *.bin *.txt *.bak -rf