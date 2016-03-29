#!/usr/bin/perl -w

my %xlshash;

sub ReadResIniFile
{
    my $buf;
    my $key;
    my $value;
    my %IDTABLE = ();

    my $inifilename = shift(@_);

    open (INIFILE, "<" . $inifilename)
        or die "Can't open $inifilename due to $!.";

    while ($buf = <INIFILE>)
    {
        $buf =~ s/[\r\n]//g;
        ($key, $value) = split("=", $buf);
        $key =~ s/^\s*//g;
        $key =~ s/\s*$//g;
        $value =~ s/^\s*//g;
        $value =~ s/\s*$//g;
        if ($key eq "RES_ID_FILE")
        {
            last;
        }
        print $key . " " . $value . "\n";
    }

    close (INIFILE);

    return $value;
}

sub ReadResIDFile
{
    my $headerfilename = shift(@_);
    open (IDFILE, "<" . $headerfilename)
        or die "Can't open $headerfilename due to $!.";

    my @fbuf;
    while (<IDFILE>)
    {
        push(@fbuf, $_);
    }

    close (IDFILE);

    my $buf = join("", @fbuf);
    $buf =~ s/\/\/.+$//mg;      # 删除//注释
    $buf =~ s/\/\*.+?\*\///sg;  # delete /* */ common
    $buf =~ s/#.+$//mg;         # 删除预定�?
    $buf =~ s/^\s*$//mg;        # delete 空行
    $buf =~ s/\n\n/\n/mg;

    $buf =~ s/\n/ /mg;
    $buf =~ s/\,/\, /mg;
    $buf =~ s/\s\s+/ /mg;

    $_ = $buf;
    my @test = m/\{(.+?)\}(\S+)\;/g;
    my @value;

    my $key = 0;
    my %IDTABLE;

    while ($key < scalar(@test))
    {
        $value = $test[$key];
        $value =~ s/^\s*//g;
        $value =~ s/\s*$//g;
        @value = split(", ", $value);
        @{$IDTABLE{$test[$key + 1]}} = @value;
        $key = $key + 2;
    }

#    for $key (sort keys %IDTABLE) {
#        print "$key:\t[", join(",", @{$IDTABLE{$key}}), "]\n";
#    }

    return %IDTABLE;
}

sub ReadXlsFile
{
    use strict;
    use Spreadsheet::ParseExcel;
    use Encode;

    my $xlsfilename = shift(@_);

    my $excel  = Spreadsheet::ParseExcel::Workbook->Parse("$xlsfilename");

    my %hash = ();
    my $key = 0;
    my @value;

    foreach my $sheet (@{$excel->{Worksheet}}) {
        $sheet->{MaxRow} ||= $sheet->{MinRow};
        foreach my $row ($sheet->{MinRow} .. $sheet->{MaxRow}) {
            $sheet->{MaxCol} ||= $sheet->{MinCol};
            foreach my $col ($sheet->{MinCol} ..  $sheet->{MaxCol}) {
                my $cell = $sheet->{Cells}[$row][$col];
                if ($cell and $row > 0) {
                    my $tmp = "";
                    if (!$cell->{Code}) {
                        if ($col == 0) {
                                if ($key ne 0)
                                {
                                    @{$hash{$key}} = @value;
#                                    push (@xlstable, $key);
                                }
                                $key = $cell->{Val};
                                @value = ();
                        } else {
                            $tmp = encode("UCS-2LE", $cell->{Val});
                            # $tmp =~ s/(.)(.)/$2$1/mg;
                            push (@value, $tmp);
                        }
                    } else {
                        $tmp = encode("UCS-2LE", decode("ucs2", $cell->{Val}));
                        # $tmp =~ s/(.)(.)/$2$1/mg;
                        push (@value, $tmp);
                    }
                }
            }
        }
    }
    @{$hash{$key}} = @value;
    %xlshash = %hash;
#    push (@xlstable, $key);

    return %hash;
}

sub WriteTxtFile ($\@%)
{
    use Encode;
    use warnings;

    my $txtfilename = shift;
    my @idhash = @_;
    my %hash = %xlshash;
#    my ($textfilename, %hash, @idhash) = @_;

    open (OUTPUT, ">" . $txtfilename)
        or die "Can't open $txtfilename due to $!.";
    binmode (OUTPUT);
    print OUTPUT "\xff\xfe";

    my $key;
    my $enter = encode("UCS-2LE", "\n");
#    for $key (sort keys %hash) {
#        print "$key:\t[", join(",", @{$hash{$key}}), "]\n";
#    }

    my $line=0;
    foreach $key (@idhash) {
#        print "$key ";
        if(($key eq "eRES_STR_LANG_NAME") and ($line == 0)){
        #print scalar @{$hash{$key}} ;
            print OUTPUT encode("UCS-2LE", "LANGUAGE :".(scalar @{$hash{$key}} )."\n\n");
        }
        print OUTPUT encode("UCS-2LE", "[" . "$key" . "]\n");
        if (exists($hash{$key})) {
            no warnings;
            print OUTPUT join($enter, @{$hash{$key}});
            use warnings;
        }
        print OUTPUT $enter . $enter;
        $line++;
    }
    print OUTPUT $enter . $enter . $enter . $enter . $enter . $enter;

    close (OUTPUT);
}

1;

