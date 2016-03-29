#!/usr/bin/perl -w

my $ResHeaderFile = '..\source_code\include\Gbl_Resource.h';    #default name for the input Header h file
my $ResIniFile = "ResMaker.ini";        #default name for the input ini file
my $ExcelXlsFile = "string.xls";        #default name for the output xls file

use warnings;

my %idhash;
my %xlshash;
my $flag = "ini";

sub printHelp
{
    print 'Please input command:\n --help\t\tshow this;\n(*.h or *.ini) resource file;\n*.xls Excel file\nExample: createxls.pl (*.h/*.ini) *.xls\n';
}

sub HandleArgs
{
    my $argc;

    $argc = @ARGV;
    if ($argc == 0) 
    {
        print "default input and output file names will be used\n"
    }
    elsif ($argc >= 1) 
    {
        if ($ARGV[0] eq "--help")
        {
            printHelp();
            exit();
        }
        elsif ($ARGV[0] =~ /\.ini/)
        {
            $flag = "ini";
            $ResIniFile = $ARGV[0];
        }
        elsif ($ARGV[0] =~ /\.h/)
        {
            $flag = "h";
            $ResHeaderFile = $ARGV[0];
        }
        if ($argc == 2)
        {
            $ExcelXlsFile = $ARGV[1];
        }
    }
    else
    {
        printHelp();
        exit(0);
    }

    print "\nInput file is $ResIniFile\n\n";
    @ARGV = ();
}


# sub ReadResIniFile
# {
#     my $buf;
#     my $key;
#     my $value;
#     my %IDTABLE = ();
# 
#     my $inifilename = shift(@_);
# 
#     open (INIFILE, "<" . $inifilename)
#         or die "Can't open $inifilename due to $!.";
# 
#     while ($buf = <INIFILE>)
#     {
#         $buf =~ s/[\r\n]//g;
#         ($key, $value) = split("=", $buf);
#         $key =~ s/^\s*//g;
#         $key =~ s/\s*$//g;
#         $value =~ s/^\s*//g;
#         $value =~ s/\s*$//g;
#         if ($key eq "RES_ID_FILE")
#         {
#             last;
#         }
#         print $key . " " . $value . "\n";
#     }
# 
#     close (INIFILE);
# 
#     return $value;
# }
# 
# sub ReadResIDFile
# {
#     my $headerfilename = shift(@_);
#     open (IDFILE, "<" . $headerfilename)
#         or die "Can't open $headerfilename due to $!.";
# 
#     my @fbuf;
#     while (<IDFILE>)
#     {
#         push(@fbuf, $_);
#     }
# 
#     close (IDFILE);
# 
#     my $buf = join("", @fbuf);
#     $buf =~ s/\/\/.+$//mg;      # 删除//注释
#     $buf =~ s/\/\*.+?\*\///sg;  # delete /* */ common
#     $buf =~ s/#.+$//mg;         # 删除预定义
#     $buf =~ s/^\s*$//mg;        # delete 空行
#     $buf =~ s/\n\n/\n/mg;
# 
#     $buf =~ s/\n/ /mg;
#     $buf =~ s/\,/\, /mg;
#     $buf =~ s/\s\s+/ /mg;
# 
#     $_ = $buf;
#     my @test = m/\{(.+?)\}(\S+)\;/g;
#     my @value;
# 
#     my $key = 0;
#     my %IDTABLE;
# 
#     while ($key < scalar(@test))
#     {
#         $value = $test[$key];
#         $value =~ s/^\s*//g;
#         $value =~ s/\s*$//g;
#         @value = split(", ", $value);
#         @{$IDTABLE{$test[$key + 1]}} = @value;
#         $key = $key + 2;
#     }
# 
# #    for $key (sort keys %IDTABLE) {
# #        print "$key:\t[", join(",", @{$IDTABLE{$key}}), "]\n";
# #    }
# 
#     return %IDTABLE;
# }
#
 
sub WriteXlsFile
{
    my $xlsfilename = shift(@_);
    my %idtable = @_;

    print $xlsfilename, "\n";

    use strict;
    use Spreadsheet::WriteExcel;
    use Encode;

    my $workbook  = Spreadsheet::WriteExcel->new("$xlsfilename");
    my $worksheet = $workbook->add_worksheet();
    
    $worksheet->store_formula('=Sheet1!A1');
    
    $workbook->add_format(color => 1);
    $workbook->add_format(color => 2, bold => 1);
    $workbook->add_format(color => 3);
    
    pop(@{$idtable{"T_RES_LANGUAGE"}});
    my $i = 0;
    my $j = 0;
    $worksheet->write($i, $j, "ID");

    $j = 1;

    my @language;
    for my $key (@{$idtable{"T_RES_LANGUAGE"}}) {
        if ($key =~ /eRES_LANG_/) {
            $key =~ s/eRES_LANG_//;
            print "$key ";
            push(@language, $key);
        }
    }

    $worksheet->write_row($i, $j, \@language);

    print pop(@{$idtable{"T_RES_STRING"}}), "\n";

    $i = 1;
    $j = 0;
    $worksheet->write_col($i, $j, \@{$idtable{"T_RES_STRING"}});
}

HandleArgs();
require ("controlxls.pl");
if ($flag eq "ini")
{
    $ResHeaderFile = ReadResIniFile($ResIniFile);
}
%idhash = ReadResIDFile($ResHeaderFile);
WriteXlsFile($ExcelXlsFile, %idhash);
