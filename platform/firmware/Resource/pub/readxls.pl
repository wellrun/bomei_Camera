#!/usr/bin/perl -w

my $strFile = "string.xls";
my $binFile = "binary.xls";
my $ResHeaderFile = '..\source_code\include\Gbl_Resource.h';    #default name for the input Header h file
my $ResIniFile = "ResMaker.ini";        #default name for the input ini file
my %idhash = ();


sub printHelp
{
    print "Function:\r\n";
    print "  Translate .xls state description file to .txt for state compiler\r\n";
    print "Syntax:\r\n";
    print "  ExcelChag.pl <<--help> | <InputFileName>> <OutputFileName>\r\n";
    print "  Default InputFileName is StateDesc.xls\r\n";
    print "  Default OutputFileName is StateDesc.txt\r\n";
    print "  Examples:\r\n";
    print "  ExcelChag.pl --help\r\n";
    print "  ExcelChag.pl OutputFileName\r\n";
    print "  ExcelChag.pl InputFileName OutputFileName\r\n";
}


sub HandleArgs
{
    my $argc;

    $argc = @ARGV;
    if ($argc == 0)
    {
        #print "default output and output file names will be used\r\n";    
    }
    elsif ($argc == 1)
    {
        printHelp();
        exit(0);   
    }
    elsif ($argc == 2)
    {
        $strFile = $ARGV[0];
        $binFile = $ARGV[1];
    }
    else
    {
        printHelp();
        exit(0);  
    }
    
    print "\r\n";
    print "string file is $strFile\r\n";
    print "binary file is $binFile\r\n";   
    print "\r\n";

}

HandleArgs();
require ("./pub/controlxls.pl");
if (-f $ResIniFile)
{
    print "$ResIniFile\n";
    $ResHeaderFile = ReadResIniFile($ResIniFile);
}
%idhash = ReadResIDFile($ResHeaderFile);
ReadXlsFile($strFile);
WriteTxtFile("string.txt", @{$idhash{"T_RES_STRING"}});

ReadXlsFile($binFile);
WriteTxtFile("binary.txt", @{$idhash{"T_RES_BINARY"}});
