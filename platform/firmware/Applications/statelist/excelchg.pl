#*****************************************************************************
# * Copyright (C) Anyka 2005. All Rights reserved.
# *
# * Transmittal, reproduction and/or dissemination of this document as well
# * as utilization of its contents and communication thereof to others without
# * express authorization are prohibited. Offenders will be held liable for
# * payment of damages. All rights created by patent grant or registration of
# * a utility model or design patent are reserved.
# *****************************************************************************
# *    VME Version: 00.00$
# *****************************************************************************
# * $Workfile: template.c.txt $
# *     $Date: 2003/09/19 06:04:16 $
# *****************************************************************************
# * Requirements:
# * Target:
# *****************************************************************************
# * Description:
# *
# *****************************************************************************

use strict;
use File::Find;
use File::Basename;
use Spreadsheet::ParseExcel::Simple;   # A simple interface to Excel data
############################################################################################
#
#
############################################################################################

my $ExcelFile = "StateDesc.xls";    # default name for the input Excel state description file
my $OutputFile = "StateDesc.txt";   # default name for output text state description file

my %UserEvent  = (
# Standard Keyboard User Events
# M_EVT_KEY_SIDE_UP => 1,
# M_EVT_KEY_SIDE_DOWN => 1,
# M_EVT_KEY_UP => 1,
# M_EVT_KEY_VOICE_UP => 1, # don't remove
# M_EVT_KEY_OK => 1,
# M_EVT_KEY_VOICE_DOWN => 1,
# M_EVT_KEY_NUM => 1,
# M_EVT_KEY_DOWN => 1,
# M_EVT_KEY_RIGHT => 1,
# M_EVT_KEY_LEFT => 1,
# M_EVT_KEY_DONE => 1,
# M_EVT_KEY_CALL => 1,
# M_EVT_KEY_MENUE => 1,
# M_EVT_KEY_CANCEL => 1,
# M_EVT_KEY_START_MODULE => 1,

# M_EVT_KEYLONG_SIDE_UP => 1,
# M_EVT_KEYLONG_SIDE_DOWN => 1,
# M_EVT_KEYLONG_UP => 1,
# M_EVT_KEYLONG_VOICE_UP => 1,
# M_EVT_KEYLONG_OK => 1,
# M_EVT_KEYLONG_VOICE_DOWN => 1,
# M_EVT_KEYLONG_NUM => 1,
# M_EVT_KEYLONG_DOWN => 1,
# M_EVT_KEYLONG_RIGHT => 1,
# M_EVT_KEYLONG_LEFT => 1,
# M_EVT_KEYLONG_DONE => 1,
# M_EVT_KEYLONG_CALL => 1,
# M_EVT_KEYLONG_MENUE => 1,
# M_EVT_KEYLONG_CANCEL => 1,
# M_EVT_KEYLONG_START_MODULE => 1,

#standard vatc line status user event
M_EVT_RING_ON =>1,
M_EVT_RING_OFF =>1,
#M_EVT_CD_ON  =>1,
#M_EVT_CD_OFF  =>1

#all messages for SimpleCC
M_EVT_SCC_RING  =>1, 
M_EVT_SCC_ESTABLISHED  =>1,     
M_EVT_SCC_INCOMING  =>1,       
M_EVT_SCC_HANGUP  =>1,         
M_EVT_SCC_COMMAND_OK  =>1,     
M_EVT_SCC_COMMAND_FAILED  =>1, 
M_EVT_SCC_STATE_CHANGED  =>1,  


M_EVT_CLIP =>1,
M_EVT_NETWORK_REG_CHANGED =>1,

M_EVT_SMS_CMT  =>1,
M_EVT_SMS_CMTI  =>1,

M_EVT_HEADSET_KEY =>1,
M_EVT_FLIP_OFF =>1,
M_EVT_FLIP_ON =>1,

#Event used  for battery
M_EVT_VOLTAGE =>1,

#Event used by Camera
M_EVT_CAM_FAILURE =>1,

);

my %hStates;
my %hStateOrder;
my %TransStates;

my $printInclude = q~

# include external events
~;


sub printHelp
{
    print "Function:\n";
    print "  Translate .xls state description file to .txt for state compiler\n";
    print "Syntax:\n";
    print "  ExcelChag.pl <<--help> | <InputFileName>> <OutputFileName>\n";
    print "  Default InputFileName is StateDesc.xls\n";
    print "  Default OutputFileName is StateDesc.txt\n";
    print "  Examples:\n";
    print "  ExcelChag.pl --help\n";
    print "  ExcelChag.pl InputFileName\n";
    print "  ExcelChag.pl InputFileName OutputFileName\n";
}

sub HandleArgs
{
    my $argc;    
    
    $argc = @ARGV;
 
    if ($argc == 0)
    {
        #print "default input and output file names will be used\n";	
    }
    elsif ($argc == 1)
    {
	if ($ARGV[0] eq "--help")
	{
	    printHelp();
	    exit(0);   
	}
	else
	{
	    $ExcelFile = $ARGV[0];
	}
    }
    elsif ($argc == 2)
    {
	$ExcelFile = $ARGV[0];
	$OutputFile = $ARGV[1];
    }
    else
    {
	printHelp();
	exit(0);  
    }
    
    print "\n";
    print "Input  file Excel file is $ExcelFile\n";
    print "Output file Text  file is $OutputFile\n";   
    print "\n";
}


sub CollectUserEvent
{
    my( $EventName ) = @_;

    # User defined event looks like M_EVT_XXX
    if ( $EventName =~ /^M_EVT_/ )
    {
	  $UserEvent{$EventName} = 1;
    }
}


sub checkTrans
  {
    my $trans;
    my @aTrans = keys %TransStates;
    foreach $trans (@aTrans)
      {
	if (!($trans =~ /_return/))
	  {
	    if (!exists $hStates{$trans})
	      {
		print "STATE WARNING: $trans not defined, but used in transition!\n";
	      }
	  }
      }
    
  }

sub OuputUserEvent
{
    my @UserEvent = sort keys %UserEvent;

    print OUTPUT "\n";
    print OUTPUT "# user event definition\n";
    print OUTPUT "\n";

    foreach my $Event ( @UserEvent ) {
        print OUTPUT "event $Event;\n";
   }

   print OUTPUT "\n"; 
}

sub checkStateName
  {
    my $state = shift;
    if (!($state =~ /(\w+)|(_return\s*(\(\s*(STACKROOT)|(\d+)\s*\))*\s*)/))
      {
	die "ERROR: Wrong state name: $state!\n";
      }
  }

sub checkEventName
  {
    my $event = shift;
    my $state = shift;
    $event =~ s/maincode *\((.+?)\)/$1/;
    if (($event =~ /[\W]+/))
      {
	die "ERROR: Wrong event name: $event in $state!\n";
      }
  }


sub checkFuncName
{
    my $func = shift;
    my $state= shift;
    if (($func =~ /[\W()]+/))
    {
	die "ERROR: Wrong function name: $func in $state!\n";
    }
}

sub getPackedName
  {
    my $state = shift;
    $state =~ s/^s_//;
    return $state;
  }

sub ProcExcelFile
{
    # File check
    my $xls = Spreadsheet::ParseExcel::Simple->read($ExcelFile) or die "Cannot read the specified excel file -- $ExcelFile\n";


    my $statecount=0;
    foreach my $sheet ($xls->sheets) {
      $sheet->next_row; #eat first row;
      my ($state, $mpath);
        while ($sheet->has_data) {
       	    # each sheet corresponds to a module
            my ($newmpath, $id, $newstate, $desc, $event, $trans, $func) = $sheet->next_row;

	    # del \n space and tab
	    $newmpath =~ tr/\n\t //d;
	    $id =~ tr/\n\t //d;
	    $desc =~ tr/\n\t //d;
	    $event =~ tr/\n\t //d;
	    $trans =~ tr/\n\t //d;
            $func =~ tr/\n\t //d;
            if ($newstate=~/\:(\w+) (\w+)/)
	      {
		my $control = $1;
		my $param = $2;
		if ($control =~/^(INIT|EXIT|PAINT|HANDLE)$/)
		  {
		    my $PackedName = getPackedName($param);
		    my $func = $control;
		    $func =~ tr/A-Z/a-z/;	
		    $hStates{$state}->{$control} =$func.$PackedName;
		  }
		  
		elsif ($control eq "USES")
		  {
		    my $PackedName = getPackedName($param);
		    $hStates{$state}->{"INIT"} = "init".$PackedName;
		    $hStates{$state}->{"EXIT"} = "exit".$PackedName;
		    $hStates{$state}->{"PAINT"} = "paint".$PackedName;
		    $hStates{$state}->{"HANDLE"} = "handle".$PackedName;
		  }
		else
		  {
		    die "ERROR: Don't know control :$control!\n";
		  }
	      }  
	    else
	      {
	      	$newstate =~ tr/\n\t //d;
	      	if ($newstate=~/(\w+)/)
	      	{
		  $state = $newstate;
		  checkStateName($state);
		  if (exists $hStates{$state})
		  {
		     die "ERROR: State $state already defined!\n";
		  }
		  $hStateOrder{$state} = $statecount++;

		  my $PackedName = getPackedName($state);
		  $hStates{$state}->{"INIT"} = "init".$PackedName;
		  $hStates{$state}->{"EXIT"} = "exit".$PackedName;
		  $hStates{$state}->{"PAINT"} = "paint".$PackedName;
		  $hStates{$state}->{"HANDLE"} = "handle".$PackedName;

                }
	      } 



	    if ($newmpath=~/\w/)
	      {
		$mpath = $newmpath;
	      }



	    if (!exists $hStates{$state}->{MPATH} && ($mpath ne "") )
	      {
		$hStates{$state}->{MPATH} =$mpath;
	      }

	    if (!exists $hStates{$state}->{ID} && ($id ne ""))
	      {
		$hStates{$state}->{ID} =$id;
	      }
	    if (!exists $hStates{$state}->{DESC} && ($desc ne "") )
	      {
		$hStates{$state}->{DESC} =$desc;
	      }

	    checkEventName($event, $state);
	    if (!exists $hStates{$state}->{EVENT}->{$event} && ($event ne ""))
            {
                if ($trans)
                {
                    if ($trans =~ /^\s*_return/)
                    {
                        $trans = "->".$trans;
                    }
                    if (!($trans =~ /(\+|\-)>(.+)/))
                    {
                        die "ERROR: MISSING +> or -> for $event(-+>??)$trans in state $state!\n";
                    }
                    else
                    {
                        my $transstate = $2;
                        checkStateName($transstate); 
                        $TransStates{$transstate} =1;
                    }		
                    $hStates{$state}->{EVENT}->{$event} =$trans;
                    $hStates{$state}->{EVENT_TYPE}->{$event} = "TRANS";
                }
                elsif ($func)
                {
                    $func =~ /(\**)(.+)$/;
                    checkFuncName($2);
                    $hStates{$state}->{EVENT}->{$event} =$2;
                    if ($1)
                    {
                        $hStates{$state}->{EVENT_TYPE}->{$event} = "EFUNC";
                    }
                    else
                    {
                        $hStates{$state}->{EVENT_TYPE}->{$event} = "FUNC";
                    }
                }
                else
                {
                    die "ERROR: No transitions or function call for event  in state $state!n";
                }
            }
            CollectUserEvent( $event );
        }
    }
}


sub printState
  {
    my $state = shift;
    my $desc = $hStates{$state}->{DESC};
    my ($init, $exit, $paint, $handle);
    my $PackedName;

    # state description line
    print OUTPUT  "# $desc\n";
    
    # state name line
    print OUTPUT  "state $state =\n";
    print OUTPUT  "\{\n";
    
    if (($state ne "preproc") && ($state ne "postproc"))
    {
        # module name line
        my $ModuleName;
        if (exists $hStates{$state}->{MPATH})
            
        {
            $ModuleName = $hStates{$state}->{MPATH}
        }
        else
        {
            $ModuleName = ".";
        }
        print OUTPUT "  module = $ModuleName/$state;\n";
        print OUTPUT "\n";

        # standard function lines

	$init = $hStates{$state}->{"INIT"};
	$exit = $hStates{$state}->{"EXIT"};
	$paint = $hStates{$state}->{"PAINT"};
	$handle = $hStates{$state}->{"HANDLE"};

        print OUTPUT "# standard functions:\n";
        print OUTPUT "  init = $init;\n";
        print OUTPUT "  exit = $exit;\n";
        print OUTPUT "  paint = $paint;\n";
        print OUTPUT "\n";

        # event handle functions
        print OUTPUT "# event handle functions\n";
        print OUTPUT "  function $handle;\n";
        print OUTPUT "\n";
    }
    my $event;
    my @aEvents = sort keys %{$hStates{$state}->{EVENT}};
    foreach $event (@aEvents) 
    {
        if ($hStates{$state}->{EVENT_TYPE}->{$event} eq "EFUNC")
        {
            print OUTPUT "  efunction ".$hStates{$state}->{EVENT}->{$event}.";\n";
        }
        elsif ($hStates{$state}->{EVENT_TYPE}->{$event} eq "FUNC")
        {
            print OUTPUT "  function ".$hStates{$state}->{EVENT}->{$event}.";\n";            
        }

    }
    
    print OUTPUT "# transitions\n";
    @aEvents = sort keys %{$hStates{$state}->{EVENT}};
    foreach $event (@aEvents) 
    {        
        if ($hStates{$state}->{EVENT_TYPE}->{$event} eq "TRANS")
        {
            print OUTPUT "  $event ".$hStates{$state}->{EVENT}->{$event}.";\n";
        }
        else
        {
            print OUTPUT "  $event ->".$hStates{$state}->{EVENT}->{$event}.";\n";
        }
    }
    print OUTPUT "\n";
    if (($state ne "preproc") && ($state ne "postproc"))
    {
        print OUTPUT "# call event functions\n";
        print OUTPUT "  _default      -> $handle;\n";
    }
    print OUTPUT "\}\n";
    print OUTPUT "\n";
  }

sub OutputStates
  {

    my $state;

    my @aStates = sort {
                    $hStateOrder{$a} <=> $hStateOrder{$b}
            } keys %hStateOrder;       

    my $firststate = shift(@aStates);
    print OUTPUT "# This is the initial state after startup\n";
    printState($firststate);

    foreach $state (@aStates)
      {
	printState($state);
      }
  }


sub createCfg
  {
    open (OUTPUT, ">$OutputFile") or die "Cannot open the specified output file -- $OutputFile\n";
    print OUTPUT $printInclude;
    OuputUserEvent();
    OutputStates();
    close OUTPUT;
  }


# Main function
HandleArgs();
ProcExcelFile();
checkTrans();
createCfg();
