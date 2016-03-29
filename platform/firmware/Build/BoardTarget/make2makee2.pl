#*****************************************************************************
# Copyright (C) Anyka(GuangZhou) 2005. All Rights reserved.
#
#****************************************************************************
# Requirements: Perl 5.005
#****************************************************************************
# Description: Makefile generator for Anyka MMI
#****************************************************************************
use strict;
use File::Find;
use File::Basename;

my $emake = "Makefile";   # name of target make file, -emake option
my $prjname = "Sword37"; # name of target, -prj option
my $outname = "Sword37"; # name of target file
#update by ljh because ld failed, too long parameter string!
my $buildpath = "bd"; # -buildpath option

my %BuildDirs;
$BuildDirs{"Applications/statelist"} =1;
my %BuildObs;
$BuildObs{"Applications/statelist/Lib_state"} = 1;

my %mmihfilepath;
$mmihfilepath{"Applications/statelist/Lib_event"} = 1;
$mmihfilepath{"Applications/statelist/Lib_state"} = 1;

my %BuildAsmObs;

#for output
my $line;
my $LINE_LENGTH=80;

my %ExceptDirs;

#all except file dirs, guobi still dont release the *.a LIB so we just ignore their files
$ExceptDirs{"../../Drivers/Simulator"} = 1;


sub getObjParamLength
{
	#local($v) = shift ; 
    my(%obs) = @_;
	#local(%obs,@obs);
	my $sob;
	#@sortKeys = sort keys(%$v) ;
	my @laobs = sort keys (%obs);
	my $total = 0;
	foreach $sob (@laobs)
	{
		$total += length($sob);
		#print FMAK length($sob);
		#print FMAK "eeeeeeeeeeeee";

	}
	#print FMAK $total;
	#print FMAK "tttttttttt";
	return $total;
}

sub startLine
{
    my $out = shift;
    $line=0;
    print FMAK $out;
}

sub printLine
{
    my $out = shift;
    $line+= length($out);
    if ($line >$LINE_LENGTH)
    {
	print FMAK "\\\n";
	$line = 0;
    }
    print FMAK $out;
}



sub wanted
{
    if (-f && /^([^ ]+?)\.c$/)
    {
        my @aDirs = split(/\//, $File::Find::dir);
        my $dir;
        my $fulldir;
        foreach $dir (@aDirs)
        {
            if ($dir ne "." and $dir ne "..")
            {
                $fulldir .= "$dir";
                $BuildDirs{$fulldir} = 1;
                $fulldir .= "/";
            }
        }

        $BuildObs{"$fulldir"."$1"} = 1;
    }
}

sub wantedExcept
{
	my $dir;
	my $fulldir;
	if (-f && (/^([^ ]+?)\.c$/ || /^([^ ]+?)\.s$/ || /^([^ ]+?)\.a$/ ))
	{
        my @aDirs = split(/\//, $File::Find::dir);

        foreach $dir (@aDirs)
        {
            if ($dir ne "." and $dir ne "..")
            {
                $fulldir .= "$dir";
                $fulldir .= "/";
            }
        }
		delete ($BuildObs{"$fulldir"."$1"});
		delete ($BuildAsmObs{"$fulldir"."$1"});
	}

}


sub wantedhfiles
{
    if (-f && (/^([^ ]+?)\.h$/ || /^([^ ]+?)\.H$/))
    {
        my @aDirs = split(/\//, $File::Find::dir);
        my $dir;
        my $fulldir;
        foreach $dir (@aDirs)
        {
            if ($dir ne "." and $dir ne "..")
            {
				#$dir = s/(..\/)+//g;
                $fulldir .= "$dir";
                $BuildDirs{$fulldir} = 1;
                $fulldir .= "/";
            }
        }

        $mmihfilepath{"$fulldir"."$1"} = 1;
    }
}


sub delExceptFiles
{
	my $dir;

	my @aDirs = sort keys %ExceptDirs;

	foreach $dir (@aDirs)
	{
		find (\&wantedExcept, $dir);
	}
}

sub getCFiles
{
    find ({ wanted => \&wanted, no_chdir => 0 }, "../../Applications");
    find ({ wanted => \&wanted, no_chdir => 0 }, "../../Middle");
    find ({ wanted => \&wanted, no_chdir => 0 }, "../../AKOS");
    find ({ wanted => \&wanted, no_chdir => 0 }, "../../Drivers");
}

sub wantedasm
{
    if (-f && /^([^ ]+?)\.s$/)
    {
        my @aDirs = split(/\//, $File::Find::dir);
        my $dir;
        my $fulldir;
        foreach $dir (@aDirs)
        {
            if ($dir ne "." and $dir ne "..")
            {
				#$dir = s/(..\/)+//g;
                $fulldir .= "$dir";
                $BuildDirs{$fulldir} = 1;
                $fulldir .= "/";
            }
        }

        $BuildAsmObs{"$fulldir"."$1"} = 1;
    }
}

sub getAsmFiles
{
	find (\&wantedasm, "../../Drivers/");
}


sub printMake
{
    my $fname = shift;
    my $prjname = shift;
    my $outname = shift;

    open(FMAK, ">$fname") or die "ERROR: Cannot open $fname!\n";

	print FMAK "\# Project name\n\n";
	print FMAK "PROJ = $prjname \n\n";
	print FMAK "TARGET= $outname\n\n";

    print FMAK "\# Pathes and Locations\n\n";

    print FMAK "ifndef BUILDPATH\n";
    print FMAK "BUILDPATH  = $buildpath\n";
    print FMAK "endif\n";

    print FMAK "ifndef M3LIBPATH\n";
    print FMAK "M3LIBPATH  = c:/cygwin/tools\n";
    print FMAK "endif\n";

print FMAK q~
# Flags

include system.cfg

BUILDPATH  := $(BUILDPATH)_$(CHIP)

APPPATH		= Applications
MIDPATH		= Middle
AKOSPATH	= AKOS
DRVPATH		= Drivers
		
MIDHPATH		= -I../../$(MIDPATH)/Include/External_Inc -I../../PubInclude
MIDINPATH		= -I../../$(MIDPATH)/Include/Internal_Inc
AKOSHPATH		= -I../../$(AKOSPATH)/Include/External_Inc -I../../PubInclude
AKOSINPATH		= -I../../$(AKOSPATH)/Include/Internal_Inc
DRVHPATH		= -I../../$(DRVPATH)/Include/External_Inc -I../../PubInclude
DRVINPATH		= -I../../$(DRVPATH)/Include/Internal_Inc
			  
ENDIANELF	= elf32-little
INCLUDE = -I../../$(DRVPATH)/Include/External_Inc

DEFINE		:= $(DEFINE) -DOS_ANYKA=1 -DDEPENDS=1

STATEPATH		:= ../../Applications/statelist


DEFINE		:= $(DEFINE) -D_UNICODE=1 



ifeq ($(SUPPORT_DEBUG_OUTPUT), Y)
DEFINE      := $(DEFINE) -DDEBUG_OUTPUT=1
endif

ifeq ($(BUILD_TYPE),TEST)
DEFINE		:= $(DEFINE) -DDEBUG_OUTPUT=1 -DASSERT_DEATH_LOOP=1 
ASFLAGS   = -keep -apcs /interwork -cpu 5TEJ
endif

ifeq ($(BUILD_TYPE),TRYOUT)
DEFINE		:= $(DEFINE) -DDEBUG_OUTPUT=1 -DJAVA_NOLIMIT=1 -DASSERT_REBOOT=1 
DEFINE		:= $(DEFINE) -DSUPPORT_PANNIC_REBOOT=1 
DEFINE		:= $(DEFINE) -DSUPPORT_NAND_TRACE=1
ASFLAGS   = -keep -apcs /interwork -cpu 5TEJ
endif

ifeq ($(BUILD_TYPE),PRODUCT)
DEFINE		:= $(DEFINE) -DJAVA_NOLIMIT=1  -DASSERT_REBOOT=1
ASFLAGS   = -keep -apcs /interwork -cpu 5TEJ -c
endif


CFLAGS		= -O2 -g -apcs /interwork -cpu ARM926EJ-S -Wb -Ec $(DEFINE)
LCCFLAGS	= -O2 -apcs /interwork -cpu ARM926EJ-S -Wb -Ec $(DEFINE) 

LDFLAGS     = -info totals,sizes -noremove -map -scatter scatter.scf -entry START

#define operation system
DEFINE		:= $(DEFINE) -DAKOS=1
ifdef NO_NORFLASH
DEFINE		:= $(DEFINE) -DNO_NORFLASH=1
endif

ifdef MEMLEAK
BUILDPATH	:= $(BUILDPATH)ml
TARGET		:= $(TARGET)_ml
DEFINE		:= $(DEFINE) -DDEBUG_TRACE_MEMORY_LEAK=1
else
endif

ifdef CHIP
DEFINE      := $(DEFINE) -DCHIP_$(CHIP)=1
endif

DEFINE		:= $(DEFINE) -D$(PLATFORM)_PLATFORM=1


ifdef SUPPORT_NAND_TRACE
DEFINE		:= $(DEFINE) -DSUPPORT_NAND_TRACE=1
endif

ifdef SUPPORT_PANNIC_REBOOT
DEFINE		:= $(DEFINE) -DSUPPORT_PANNIC_REBOOT=1
ifdef SUPPORT_PANNIC_REBOOT_TEST
DEFINE		:= $(DEFINE) -DSUPPORT_PANNIC_REBOOT_TEST=1
endif
endif

# Camera Flash Light
DEFINE      := $(DEFINE) -DCAMERA_FLASHLIGHT

# Tools
CC		=armcc
AS	     	= armasm
LD	     	= armlink
RM         	= rm -rf
MKDIR      	= mkdir
OBJDUMP      = objdump
OBJCOPY	     = fromelf
COMPRESS_BIN    = ./cpstool.exe

LIBS		:= $(LIBS) ../../AKOS/Library/fileSystem/fat32fs.a \
				../../AKOS/Library/fileSystem/mount.a \
				../../AKOS/Library/fileSystem/mtdlib.a \
				../../AKOS/Library/filesystem/fha.a \
				../../AKOS/Library/AKOS/akos.a \
				../../AKOS/Library/memory/memory.a \
				../../AKOS/Library/devManage/drv_ak37xx.a \
				../../Middle/Library/SMCore/smcore.a \
				../../Middle/Library/unicode/unilib.a \
				../../Middle/Library/media/media_lib_aspen.a \
				../../Middle/Library/media/video_lib_aspen.a



ifeq ($(SUPPORT_NETWORK), Y)
DEFINE      += -DSUPPORT_NETWORK
LIBS		:= $(LIBS) \
				../../Middle/Library/lwip/LwipLib.a 
endif

ifeq ($(SUPPORT_FLASH), Y)
DEFINE      += -DSUPPORT_FLASH
LIBS		:= $(LIBS) \
                ../../Middle/Library/flash/Flash_lib.a \
                ../../Middle/Library/freetype/libfreetype.a \
				../../Middle/Library/flash/libiconv.a
endif

ifeq ($(SUPPORT_VFONT), Y)
DEFINE      += -DSUPPORT_VFONT
ifneq ($(SUPPORT_FLASH), Y)
LIBS		:= $(LIBS) \
                ../../Middle/Library/freetype/libfreetype.a
endif
endif

ifeq ($(RAM_SIZE), 8)
	LIBS		:= $(LIBS) ../../Middle/Library/media/sdcodec.a
else
	LIBS		:= $(LIBS) ../../Middle/Library/media/sdcodec_midi.a
endif

ifdef BOOT_MODE
  DEFINE		:= $(DEFINE) -D$(BOOT_MODE)BOOT=1
				LIBS		:= $(LIBS) ../../Middle/Library/dynamicload/dynamicload.a

  ifeq ($(BOOT_MODE), SPI)
        LIBS		:= $(LIBS) ../../Middle/Library/dynamicfont/DynamicFont_v4_spi.a
  else
        LIBS		:= $(LIBS) ../../Middle/Library/dynamicfont/DynamicFont_v4.a
  endif
endif

#Locations
#ifndef NOSOUNDLIB

ifeq ($(PLATFORM), CI37XX)
LIBS		:= $(LIBS) ../../Middle/Library/media/sdfilter.a
LIBS		:= $(LIBS) ../../Middle/Library/image/image_37xx.a
LIBS		:= $(LIBS) ../../Middle/Library/media/motion_detector_lib.a
endif

#endif

ifeq ($(SUPPORT_EBK), Y)
    DEFINE      += -DSUPPORT_EBK
	LIBS		:= $(LIBS) ../../Middle/Library/ebook/ebookcore.a
endif


ifeq ($(SUPPORT_AUTOTEST), Y)
    DEFINE      += -DSUPPORT_AUTOTEST
endif
ifneq ($(RAM_SIZE), 8)
ifeq ($(SUPPORT_GESHADE), Y)
    DEFINE      += -DSUPPORT_GE_SHADE
    LIBS		:= $(LIBS) ../../Middle/Library/ge/libge.a
endif

ifeq ($(SUPPORT_AUDIOREC), Y)
		DEFINE      += -DSUPPORT_AUDIOREC_DENOICE
endif
endif


ifeq ($(SUPPORT_GAME), Y)
ifdef SUPPORT_GAME_NES
ifeq ($(SUPPORT_GAME_NES),Y)
	DEFINE      += -DINSTALL_GAME_NES
	LIBS		:= $(LIBS) ../../Middle/Library/game/fcsim_lib/fcsim.a 
endif
endif

ifdef SUPPORT_GAME_SFC
ifeq ($(SUPPORT_GAME_SFC),Y)
	DEFINE      += -DINSTALL_GAME_SFC
	LIBS		:= $(LIBS) ../../Middle/Library/game/snes_lib/snes.a 
endif
endif

ifdef SUPPORT_GAME_GBA
ifeq ($(SUPPORT_GAME_GBA),Y)
	DEFINE      += -DINSTALL_GAME_GBA
	LIBS		:= $(LIBS) ../../Middle/Library/game/gba_lib/gba.a
endif
endif

ifdef SUPPORT_GAME_MD
ifeq ($(SUPPORT_GAME_MD),Y)
	DEFINE      += -DINSTALL_GAME_MD
	LIBS		:= $(LIBS) ../../Middle/Library/game/mdsim_lib/mdsim_lib.a
endif
endif
endif


ifeq ($(SUPPORT_VISUAL_AUDIO), Y)
LIBS		:= $(LIBS) ../../Middle/Library/media/libva.a
endif

#build rule
.PHONY: clean makedirs maketarget debug release help msi compresscode

debug:
	@echo ---------------------[build debug version]---------------------
	@echo build start datetime:
	date
	$(MAKE) makedirs CFG=Debug
	$(MAKE) msi CFG=Debug
	$(MAKE) ctohdepends CFG=Debug >$(BUILDPATH)/depends.txt 2>&1
	$(MAKE) maketarget CFG=Debug
	$(MAKE) compresscode CFG=Debug
	@echo build end datetime:
	date
release:
	@echo ---------------------[build release version]---------------------
	$(MAKE) makedirs CFG=Release
	$(MAKE) msi CFG=Release
	$(MAKE) ctohdepends CFG=Release >$(BUILDPATH)/depends.txt 2>&1
	$(MAKE) maketarget CFG=Release
	$(MAKE) compresscode CFG=Release


maketarget: $(BUILDPATH)/$(TARGET).elf $(BUILDPATH)/$(TARGET).bin
compresscode:
ifeq ($(ENABLE_COMPRESS_BIN), Y)
	$(COMPRESS_BIN)  $(BUILDPATH)/../$(TARGET).bin
	@echo compress bin ok
else
	@echo bin neednot compress
endif

$(BUILDPATH):
	-@$(MKDIR) -p $@

~;

startLine("BUILDDIRS = ");

my $path;
my @aPath = sort keys %BuildDirs;
foreach $path (@aPath)
{
    printLine(" \$(BUILDPATH)/$path");
}
print FMAK "\n\$(BUILDDIRS):\n";
print FMAK "\t-@\$(MKDIR) \$@\n";
print FMAK "makedirs: \$(BUILDPATH) \$(BUILDDIRS)\n";


print FMAK "\n\$(BUILDPATH)/\$(TARGET).bin:  \$(BUILDPATH)/\$(TARGET).elf";
print FMAK q~
	@echo ---------------------[build bin binary]----------------------------------
	@$(OBJCOPY) -bin $(BUILDPATH)/$(TARGET).elf  -output $(BUILDPATH)/../$(TARGET).bin
	#$(OBJCOPY)  -text -c -s $(BUILDPATH)/$(TARGET).elf -output $(BUILDPATH)/$(TARGET).txt
~;

#if LD arguments not exceed the max length, then make ELF file directly
#print FMAK "aaaaaaaa";
#print FMAK getObjParamLength(%BuildObs);
#print FMAK "bbbbbbb";
#print FMAK getObjParamLength(%BuildAsmObs);
#getObjParamLength(\%BuildObs);
#print FMAK "bbbbbbbbbbbbbbbbb";

startLine("\nSOBJS = ");
my $sob;
my @sObs = sort keys %BuildAsmObs;
foreach $sob (@sObs)
{
    printLine(" \$(BUILDPATH)/$sob.o");
}
print FMAK "\n";

startLine("\nCOBJS = ");
my $ob;
my @aObs = sort keys %BuildObs;
foreach $ob (@aObs)
{
		if("Drivers/SourceCode" eq substr($ob,0,18))
    {
        printLine(" \$(BUILDPATH)/$ob.o");
    }
}
print FMAK "\n";

	#otherwise, build n.ELF (n=1,2,3...) and combine them
	my $ob;
	my $libidx = 1;
	my $elfparamlen = 0;
	my $obsstr = "";
	my $libstr = "";
	my @laobs;
	my $i;

	for($i=0; $i<1; $i++)
	{
		if(0 == $i)
		{
			@laobs = sort keys (%BuildObs);
		}
		else
		{
			@laobs = sort keys (%BuildAsmObs);
		}
		
		$obsstr = "";
		foreach $ob (@laobs)
		{
			if("Applications" eq substr($ob,0,12))
			{
				$obsstr .= (" \$(BUILDPATH)/$ob.o");
			}
		}
		print FMAK "\n\$(BUILDPATH)/\$(APPPATH).a: $obsstr Makefile\n";
		print FMAK "\t\@echo ---------------------[build out \$(APPPATH).a:]----------------------------------\n";
		print FMAK "\t-\$(AR) -rsv \$(BUILDPATH)/\$(APPPATH).a $obsstr\n";
		$libstr .= " \$(BUILDPATH)/\$(APPPATH).a";
		
		$obsstr = "";
		foreach $ob (@laobs)
		{
			if("Middle" eq substr($ob,0,6))
			{
				$obsstr .= (" \$(BUILDPATH)/$ob.o");
			}
		}
		print FMAK "\n\$(BUILDPATH)/\$(MIDPATH).a: $obsstr Makefile\n";
		print FMAK "\t\@echo ---------------------[build out \$(MIDPATH).a:]----------------------------------\n";
		print FMAK "\t-\$(AR) -rsv \$(BUILDPATH)/\$(MIDPATH).a $obsstr\n";
		$libstr .= " \$(BUILDPATH)/\$(MIDPATH).a";
		
		$obsstr = "";
		foreach $ob (@laobs)
		{
			if("AKOS" eq substr($ob,0,4))
			{
				$obsstr .= (" \$(BUILDPATH)/$ob.o");
			}
		}
		print FMAK "\n\$(BUILDPATH)/\$(AKOSPATH).a: $obsstr Makefile\n";
		print FMAK "\t\@echo ---------------------[build out \$(AKOSPATH).a:]----------------------------------\n";
		print FMAK "\t-\$(AR) -rsv \$(BUILDPATH)/\$(AKOSPATH).a $obsstr\n";
		$libstr .= " \$(BUILDPATH)/\$(AKOSPATH).a";
	}

	#print .elf and n.elf(n=1,2,3...) dependence and make command
	print FMAK "\n\$(BUILDPATH)/\$(TARGET).elf: \$(SOBJS) \$(COBJS) \$(LIBS)  $libstr\n";
	print FMAK "\t\@echo ---------------------[build out \$(TARGET).elf:]----------------------------------\n";
	print FMAK "\t\$(LD) \$(LDFLAGS) -o \$(BUILDPATH)/\$(TARGET).elf $libstr \$(SOBJS) \$(COBJS) \$(LIBS)";

print FMAK q~
	@$(OBJCOPY)  -text -c -s $(BUILDPATH)/$(TARGET).elf -output $(BUILDPATH)/$(TARGET).txt
~;

print FMAK q~
msi: $(STATEPATH)/Lib_event.h  $(STATEPATH)/Lib_state.h  $(STATEPATH)/Lib_state.c

$(STATEPATH)/Lib_event.h  $(STATEPATH)/Lib_state.h  $(STATEPATH)/Lib_state.c: $(STATEPATH)/statelist.xls
	perl $(STATEPATH)/excelchg.pl $(STATEPATH)/statelist.xls $(STATEPATH)/states.cfg
	cd $(STATEPATH);  perl statecc.pl -f states.cfg

clean :
	-@$(RM) $(BUILDPATH)* *.bin
	-@$(RM) $(STATEPATH)/Lib_event.h  $(STATEPATH)/Lib_state.h  $(STATEPATH)/Lib_state.c $(STATEPATH)/states.cfg
	-@$(RM) ../../allincfiles

help:
	@echo "Usage:   make [TARGET] [VARIABLE=XXX]"
	@echo "TARGET:"
	@echo "	debug:   Builds a debug version, default target"
	@echo "		release: Builds a release version"
	@echo "		clean:   Remove all created objects "
	@echo "VARIABLES:"
	@echo "The VARIABLES values are case sensitive."
	@echo "	BUILDPATH: Directory of all build objects,"
	@echo "	e.g. BUILDPATH = c:/build, default: BUILDPATH=build"
	@echo "	Don't use a backslash!"
	@echo "	MMI_PDA:   build PDA version MMI, build in BUILDPATH/pda directory"
	@echo "		MMI_PDA=1"
	@echo "	MEMLEAK:   build memory leak version, build in BUILDPATHml directory"
	@echo "		MEMLEAK=1"
	@echo "	NOSOUNDLIB:  no link sound library?"
	@echo "		NOSOUNDLIB=1"
	@echo "	SUPPORT_EMAIL:  support email function?"
	@echo "		SUPPORT_EMAIL=1"
	@echo "	SUPPORT_MUX:  support multiplexer protocol?"
	@echo "		SUPPORT_MUX=1"
	@echo " INSTALL_GAME_NES: install nes game simulator?"
	@echo "		INSTALL_GAME_NES=1"
	@echo " INSTALL_GAME_SFC: install sfc game simulator?"
	@echo "		INSTALL_GAME_SFC=1"
	@echo " INSTALL_GAME_GBA: install gba game simulator?"
	@echo "		INSTALL_GAME_GBA=1"	
	@echo "	SUPPORT_BCR:  support BCR?"
	@echo "		SUPPORT_BCR=1"
	@echo "	SUPPORT_OFFICEVIEWER:  support OFFICEVIEWER?"
	@echo "		SUPPORT_OFFICEVIEWER=1"
	@echo "	SUPPORT_CARELAND_EMAP:  support careland emap?"
	@echo "		SUPPORT_CARELAND_EMAP=1"
	@echo "	SUPPORT_GPS:  use gps?"
	@echo "		SUPPORT_GPS=1"
	@echo "	SUPPORT_GLONAV_GPS:  use GLONAV gps?"
	@echo "		SUPPORT_GLONAV_GPS=1"
	@echo "	GLONAV_GPS_DEBUG:  use GLONAV gps debug mode?"
	@echo "		GLONAV_GPS_DEBUG=1"
	@echo "	SUPPORT_JAVA:  support JAVA?"
	@echo "		SUPPORT_JAVA=1"
	@echo "	SDRAM_8M:  support 8M SDRAM?"
	@echo "		SDRAM_8M=1"
	@echo "	SDRAM_32M:  support 32M SDRAM?"
	@echo "		SDRAM_32M=1"
	@echo "	NANDBOOT:  build nandboot version, build in BUILDPATH/nandboot directory"
	@echo "		NANDBOOT=1"
	@echo "	NO_NORFLASH:  build no nor-flash version"
	@echo "		NO_NORFLASH=1, use NANDBOOT=1 and NO_NORFLASH=1 to demo nandboot"
	@echo "	DEPENDS:  build and generate dependence information(.d) file"
	@echo "		DEPENDS=1 to open, if this flag opened, when .h file changes, related .c file would be recompiled automatically"
	@echo "	IGNOREERR:  ignore compile error when building"
	@echo "		IGNOREERR=1 to open, if this flag opened, ignore compile error when building"
	@echo ""
	@echo "Build folder example:"
	@echo ""
	@echo "	build/debug build debug nor-boot version of board mobile"
	@echo "	build/release build release nor-boot version of board mobile"
	@echo "	build/nandboot/debug build debug nand-boot version of board mobile"
	@echo "	build/nandboot/release build release nand-boot version of board mobile"
	@echo ""
	@echo "	build/pda/debug build debug nor-boot version of PDA mobile"
	@echo "	build/pda/release build release nor-boot version of PDA mobile"
	@echo "	build/pda/nandboot/debug build debug nand-boot version of PDA mobile"
	@echo "	build/pda/nandboot/release build release nand-boot version of PDA mobile"
	

# Rules


# ----------------------------- .s -> .o
ifdef IGNOREERR
$(BUILDPATH)/%.o:../../%.s
	@echo ---------------------build out .s obj file[$@]----------------------------------
	-$(AS) $(ASFLAGS) -o $@ $<
else
$(BUILDPATH)/%.o:../../%.s
	@echo ---------------------build out .s obj file[$@]----------------------------------
	$(CC) $(CFLAGS) $(INCLUDE) -E -o $(BUILDPATH)/preproc.s $<	
	$(AS) $(ASFLAGS) $(INCLUDE) -o $@ $(BUILDPATH)/preproc.s
	$(RM) $(BUILDPATH)/preproc.s

endif

# ----------------------------- .c -> .o
$(BUILDPATH)/$(APPPATH)/%.o:../../$(APPPATH)/%.c
	@echo ---------------------build out .c obj file[$@]----------------------------------
	$(CC) -c $(CFLAGS) $(MIDHPATH) -o $@ $<

$(BUILDPATH)/$(MIDPATH)/%.o:../../$(MIDPATH)/%.c
	@echo ---------------------build out .c obj file[$@]----------------------------------
	$(CC) -c $(CFLAGS) $(AKOSHPATH) $(MIDHPATH) $(MIDINPATH) $(DRVHPATH) -o $@ $<
	
$(BUILDPATH)/$(AKOSPATH)/%.o:../../$(AKOSPATH)/%.c
	@echo ---------------------build out .c obj file[$@]----------------------------------
	$(CC) -c $(CFLAGS) $(DRVHPATH) $(AKOSHPATH) $(AKOSINPATH) -o $@ $<
	
$(BUILDPATH)/$(DRVPATH)/%.o:../../$(DRVPATH)/%.c
	@echo ---------------------build out .c obj file[$@]----------------------------------
	$(CC) -c $(CFLAGS) $(DRVHPATH) $(DRVINPATH)  $(AKOSHPATH)  -o $@ $<

ifdef DEPENDS
# ----------------------------- .c,.s -> d
ifdef IGNOREERR
$(BUILDPATH)/%.d: ../../%.c
	@echo ---------------------build out depend file[$@]----------------------------------
	-$(CC) -MD -E $(CFLAGS) $(INCLUDE) -o $@ $< \
	| sed 's/\($(notdir $*)\)\.o[ :]*/$(subst /,\/,$(join $(BUILDPATH)/,$(dir $*)))\1.o $(subst /,\/,$@) : /g' > $@;
else
$(BUILDPATH)/%.d: ../../%.c
	@echo ---------------------build out depend file[$@]----------------------------------
	$(CC) -MD -E $(CFLAGS) $(INCLUDE) -o $@ $< \
	| sed 's/\($(notdir $*)\)\.o[ :]*/$(subst /,\/,$(join $(BUILDPATH)/,$(dir $*)))\1.o $(subst /,\/,$@) : /g' > $@;
endif

endif

CTOHDEPS    := $(patsubst %.o, %.d, $(COBJS))
# $(SOBJS))	$(CC) -MD xxx.s can't generate the .d file

ifdef DEPENDS
ctohdepends: $(CTOHDEPS)
	@echo ---------------------[build depend files]---------------------
else
ctohdepends:

endif


ifeq ($(MAKECMDGOALS),maketarget)
ifdef DEPENDS
include $(CTOHDEPS)
endif
endif
~;
}


sub printHelp
{
    print "Syntax:\n";
    print "  make2make.pl [-emake <MAKEFILE>] [-prj <PROJECT>] [-out <OUTNAME>]  [-taskingpath <PATH>] [-buildpath <PATH>]\n";
    print "            -emake <MAKEFILE>: target makefile for elise, default: Makefile\n";
    print "            -prj <PROJECT>: project name, default: elise\n";
    print "            -taskingpath <PATH>: Path of tasking compiler, default: c:/c166\n";
    print "            -buildpath <PATH>: Path for all build objects, default: build\n";

}

sub handleArgs
{
    my $argc;

    for ($argc=0; $argc<@ARGV; $argc++)
    {
	if ($ARGV[$argc] eq "--help")
	{
	    printHelp();
	    exit(0);
	}
	elsif ($ARGV[$argc] eq "-emake")
	{
            $argc++;
	    if ($argc<@ARGV)
	    {
		$emake = $ARGV[$argc];
	    }
	}
	elsif ($ARGV[$argc] eq "-prj")
	{
            $argc++;
	    if ($argc<@ARGV)
	    {
		$prjname = $ARGV[$argc];
	    }
	}
	elsif ($ARGV[$argc] eq "-out")
	{
            $argc++;
	    if ($argc<@ARGV)
	    {
		$outname = $ARGV[$argc];
	    }
	}
	elsif ($ARGV[$argc] eq "-buildpath")
	{
            $argc++;
	    if ($argc<@ARGV)
	    {
		$buildpath = $ARGV[$argc];
		$buildpath =~ tr/\\/\//;
	    }
	}
	else
	{
	    print "ERROR: Unknown option $ARGV[$argc] !\n";
	    printHelp();
	    exit(-1);
	}
    }
}

# ----------------------------------------------------------------- main
# ---------------------------------------------------------- handle arguments
handleArgs();
getCFiles();
getAsmFiles();
delExceptFiles();

printMake($emake, $prjname, $outname);
