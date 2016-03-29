#*****************************************************************************
# Copyright (C) Anyka(GuangZhou) 2007. All Rights reserved.
#
#****************************************************************************
# Requirements: Perl 5.005
#****************************************************************************
# Description: Makefile generator for Anyka BIOS
#****************************************************************************


use strict;
use File::Find;
use File::Basename;

my $emake = "Makefile";   # name of target make file, -emake option
my $prjname = "bios"; # name of target, -prj option
my $buildpath = "bd"; # -buildpath option

my %BuildDirs;
my %BuildObs;

my %BuildAsmObs;

#for output
my $line;
my $LINE_LENGTH=80;

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
            if ($dir ne ".")
            {
                $fulldir .= "$dir";
                $BuildDirs{$fulldir} = 1;
                $fulldir .= "/";
            }
        }
        
        $BuildObs{"$fulldir"."$1"} = 1;
    }
}

sub getCFiles
{
    find ({ wanted => \&wanted, no_chdir => 0 }, "../driver");
    find ({ wanted => \&wanted, no_chdir => 0 }, "../bios");   
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
            if ($dir ne ".")
            {
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
    find (\&wantedasm, "../driver");
    find (\&wantedasm, "../bios");
}


sub printMake
{
    my $fname = shift;
    my $target = shift;
        
    open(FMAK, ">$fname") or die "ERROR: Cannot open $fname!\n";

	print FMAK "\# Project name\n";
	print FMAK "PROJ = $prjname\n\n";
    print FMAK "\# Pathes and Locations\n\n";	

    print FMAK "ifndef BUILDPATH\n";
    print FMAK "BUILDPATH  = $buildpath\n";
    print FMAK "endif\n";
    
print FMAK q~


PLATFORM = CI37XX
DRVNAME = ak37xx

# Flags

PROJ       := $(PROJ)_$(PLATFORM)
BUILDPATH  := $(BUILDPATH)_$(CHIP)

INCLUDE    = -I../bios/include \
	     -I../bios/fs \
	     -I../bios/mem \
	     -I../bios/utils \
	     -I../bios/engine \
             -I../driver/library/include \
             -I../driver/device/include -I../driver/device/camera/include \
             -I../driver/device/lcd/include -I../driver/device/$(PLATFORM)/include \
             

                  

ENDIANELF	    = elf32-little

CFLAGS 	= -g -O2 -apcs /interwork -cpu 5TEJ -W -Ec $(DEFINE) $(INCLUDE)
ASFLAGS = -keep -apcs /interwork -cpu 5TEJ $(INCLUDE)
ifeq ($(SDRAM), 32)
LDFLAGS	= -info totals,sizes -noremove -map -scatter scatter_32M.scf -entry START
else
LDFLAGS	= -info totals,sizes -noremove -map -scatter scatter.scf -entry START
endif

LIBS	= $(OTHER_LIBS) ../driver/library/drv_$(DRVNAME).a
LIBS	:= $(LIBS) ../bios/fs/fat32fs.a
LIBS	:= $(LIBS) ../bios/fs/mount.a
LIBS	:= $(LIBS) ../bios/fs/mtdlib.a
LIBS	:= $(LIBS) ../bios/fs/fha.a
LIBS	:= $(LIBS) ../bios/mem/memory.a


#define
DEFINE		= -DOS_ANYKA=1

ifdef BOOT_MODE
  DEFINE		:= $(DEFINE) -D$(BOOT_MODE)BOOT=1
ifeq ($(BOOT_MODE), NAND)
    PROJ       := $(PROJ)_NAND
else
    PROJ       := $(PROJ)_SD
endif
endif


DEFINE		:= $(DEFINE) -DSDRAM_MODE=$(SDRAM)

ifdef CHIP
DEFINE      := $(DEFINE) -DCHIP_$(CHIP)=1
endif

DEFINE		:= $(DEFINE) -D$(PLATFORM)_PLATFORM=1



# Tools
CC		    = armcc
AS	     	= armasm
LD	     	= armlink
RM         	= rm -rf
MKDIR      	= mkdir
OBJDUMP     = objdump
OBJCOPY	    = fromelf

#build rule
.PHONY: clean makedirs maketarget debug help msi

debug:
	$(MAKE) makedirs BUILDPATH=$(BUILDPATH)/deb  
	$(MAKE) msi
	$(MAKE) maketarget BUILDPATH=$(BUILDPATH)/deb

maketarget: $(BUILDPATH)/$(PROJ).elf $(BUILDPATH)/$(PROJ).bin

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

startLine("\nSOBJS = ");
my $sob;
my @saObs = sort keys %BuildAsmObs;
foreach $sob (@saObs)
{
    $_ = $sob;
    if (m/ads/)
    {
        printLine(" \$(BUILDPATH)/$sob.o");
    }
}
print FMAK "\n";
startLine("\nCOBJS = ");
my $ob;
my @aObs = sort keys %BuildObs;
foreach $ob (@aObs)
{
    printLine(" \$(BUILDPATH)/$ob.o");
}
print FMAK "\n";

print FMAK "\n\$(BUILDPATH)/\$(PROJ).bin:  \$(BUILDPATH)/\$(PROJ).elf";
print FMAK q~
	@echo ---------------------[build bin binary]----------------------------------
	$(OBJCOPY) -bin $(BUILDPATH)/$(PROJ).elf -output $(PROJ).bin
	
~;

print FMAK "\n\$(BUILDPATH)/\$(PROJ).elf:  \$(SOBJS) \$(COBJS)";
	
print FMAK q~
	@echo ---------------------[build out]----------------------------------
	$(LD) $(LDFLAGS) -o $(BUILDPATH)/$(PROJ).elf $(SOBJS) $(COBJS) $(LIBS)
	$(OBJCOPY)  -text -c -s $(BUILDPATH)/$(PROJ).elf -output $(BUILDPATH)/$(PROJ).txt
	
clean : 
	-@$(RM) $(BUILDPATH)* *.bin *.txt

help:
	@echo "Usage:   make [TARGET] [VARIABLE=XXX]"
	@echo "TARGET:" 
	@echo "         debug:   Builds a debug version, default target"
	@echo "         release: Builds a release version"
	@echo "         clean:   Remove all created objects "
	@echo "VARIABLES:" 
	@echo "         PLATFORM: like CI2401, AI2301 etc."
	@echo "         CHIP    : like AK3223, AK3224, AK322L"

# Rules


# --------------------------- s -> o
$(BUILDPATH)/%.o:%.s
	@echo ---------------------[$<]----------------------------------
	$(CC) $(CFLAGS) -E -o $(BUILDPATH)/preproc.s $<	
	$(AS) $(ASFLAGS) -o $@ $(BUILDPATH)/preproc.s
	$(RM) $(BUILDPATH)/preproc.s

# ----------------------------- c -> d
$(BUILDPATH)/%.o:%.c
	@echo ---------------------[$<]----------------------------------
	$(CC) -c $(CFLAGS)  -o $@ $<	

~;
}



sub printHelp
{
    print "Syntax:\n";
    print "  make2make.pl [-emake <MAKEFILE>] [-prj <PROJECT>] [-taskingpath <PATH>] [-buildpath <PATH>]\n";
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

printMake($emake, $prjname);
