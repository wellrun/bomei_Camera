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
# *     $Date: 2003/09/28 01:43:06 $
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
use Getopt::Std;

my $TRIGGERMODE = 0;

my $resfile;
my %hOptions;



my %hEvent2Id;
my %hId2Event;
my $nextEventId=0;


my $nextStateId=0;
my %hState2Id;
my %hId2State;

my %heFunctions;
my %hFunctions;
my %hStates;
my %hStates;
my %hStateTo;
my %hStateFrom;

my @aStateAttr = ("INIT", "EXIT", "PAINT", "NEXT");

my %hWarnings;
my %hErrors;

my @aIncludes;
my $mstateCdir = "../statelist";
my $mstateHdir = "../../Middle/Include/External_Inc";
my $eventfileH = "Lib_event.h";
my $statefileC = "Lib_state.c";
my $statefileH = "Lib_state.h";
my $genStubs = 0;
my $genErrors = 1;
my $genWarnings = 1;
my $fheader;

my $stack_depth = 21;
my $event_entries = 20;

sub printHelp
{
    print "Syntax:\n";
    print "  statecc.pl -f <input> [-s <0|1>] [-e <0|1]\n";
    print "             -f <input>   : input file in state configuration syntax\n";
    print "             -s <0|1>     : if 1, generate files with stub functions\n";
    print "             -e <0|1>     : if 1, exit with error, if transistions ar inconsistent\n";
    print "             -w <0|1>     : if 1, generate warnings\n";
}


sub handleArgs
{
    my $help;
    
    if (!getopts('f:h:s:e:w:', \%hOptions)) 
    {
        printHelp();
        exit;
    }
    else
      {
        ( $resfile,
          $help,
          $genStubs,
	  $genErrors,
	  $genWarnings
)
            =($hOptions{f},
              $hOptions{h},
              $hOptions{s},
              $hOptions{e},
              $hOptions{w});
        
        if((!$resfile)||($help))
        {
            printHelp();
            exit (1);
        }
    }
}


sub matchingbrace_without_comments 
{
    my $rIn = shift;
    # pos($rIn) is after the opening brace now
    my $n = 0;
    while ($$rIn=~/(\#)|([\{\[\(])|([\]\)\}])/g) 
    {
        if ($1)
        {
             $$rIn =~ /\n/gs;
        }
        elsif ($2)
        {
            $n++;
        }
        else
        {
            $n--;
        }
	return 1 if $n < 0;
    }
    # pos($rIn) is after the closing brace now
    return;				# false
}

sub get2Brace
{
    my $rIn =shift;
    my $start = pos($$rIn);
    matchingbrace_without_comments($rIn);
    my $end = pos($$rIn);
    my $content = substr($$rIn, $start, $end-$start-1);
    #print "---------------------------\n$content\n";
    return $content;
}

sub get2Colon
{
    my $rIn =shift;
    my $start = pos($$rIn);
    $$rIn =~ /;/gs;
    my $end = pos($$rIn);
    my $content = substr($$rIn, $start, $end-$start-1);
    #print "---------------------------\n$content\n";
    while ($content =~ s/[ \n\t]//gs){};
    return $content;
}


sub parseEvent
{
    my $in = shift;
    $in =~ s/[ \t\n]+//;
    $hEvent2Id{$in} = $nextEventId;
    $hId2Event{$nextEventId} = $in;
    $nextEventId++;
}

sub getAssignment
{
    my $in = shift;
    $in =~ s/[ \t\n]+//;
    my @aIn = split(/[=\s\t\n]+/, $in);
    return $aIn[1];
            
}


sub parseState
{
    my $state =shift;    
    my $in = shift;
    $hStates{$state}->{NEXT} = "m_$state"."GetNext";

    $hState2Id{$state} = $nextStateId;
    $hId2State{$nextStateId} = $state;
    $nextStateId++;

    my $transid = 0;
    #print "\nstate $state\n-----------------\n";
    
    while ($in =~ /(\#)|(module)|(init)|(exit)|(handle)|(paint)|(function)|(efunction)|(([^;\#\n]+)[\s\t\n]*(-|\+)>)|([^\t\s\n]+)/gs)
    {
        my (
            $p_comment,
            $p_module,
            $p_init,
            $p_exit,
            $p_handle,
            $p_paint,
            $p_function,
            $p_efunction,
            $p_transition, $p_event, $p_type,
            $p_error                        
            ) = ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12);

        if ($p_comment)
        {
            $in =~ /\n/gs;
        }
        elsif ($p_module)
        {
            my $module = getAssignment(get2Colon(\$in));
            #print "module = $module\n";
            $hStates{$state}->{MODULE} = $module;
            
        }
        elsif ($p_init)
        {
            my $init =  getAssignment(get2Colon(\$in));
            #print "init = $init\n";
            $hStates{$state}->{INIT} = $init;
        }
        elsif ($p_exit,)
        {
            my $exit =  getAssignment(get2Colon(\$in));
            #print "exit = $exit\n";
            $hStates{$state}->{EXIT} = $exit;
        }
        elsif ($p_handle)
        {
            my $handle =  getAssignment(get2Colon(\$in));
            #print "handle = $handle\n";
            $hStates{$state}->{HANDLE} = $handle;
        }
        elsif ($p_paint)
        {
            my $paint =  getAssignment(get2Colon(\$in));
            #print "paint = $paint\n";
            $hStates{$state}->{PAINT} = $paint;
        }
        elsif ($p_function)
        {
            my $function =  get2Colon(\$in);
            #print "function = $function\n";
            $hStates{$state}->{FUNCTIONS}->{$function}="state";
            $hFunctions{$function} = $state;
        }
        elsif ($p_efunction)
        {
            my $function =  get2Colon(\$in);
            #print "efunction = $function\n";
            $hStates{$state}->{FUNCTIONS}->{$function}="event";
            $heFunctions{$function} = $state;
        }
        elsif ($p_transition)
        {
            my $transition = get2Colon(\$in);
            my $event = $p_event;
            my $type = $p_type;
#            print "$event\n";
            while($event =~ s/[ \t\n]+//gs){};
            while($transition =~ s/[ \t\n]+//gs){};
 #           print "$event -> $transition\n";

            $hStates{$state}->{hTRANSITIONS}->{$transid}->{TRANS} = $transition;
            $hStates{$state}->{hTRANSITIONS}->{$transid}->{EVENT} = $event;
            $hStates{$state}->{hTRANSITIONS}->{$transid}->{TYPE} = $type;
            $transid++;
        }
        else
        {
            die "PARSE ERROR in $resfile: $p_error!\n";
        }
        
    }
}


sub parseRes
{
    my $fname = shift;
    my $in;
    undef $/;
    open(FRES, "<$fname") or die "ERROR: Cannot open $fname!\n";
    $in = <FRES>;

    
    while ($in =~ /(\#)|(include)|(event)|(state)|([^\t\s\n]+)/gs)
    {
        my (
            $p_comment,
            $p_include,
            $p_event,
            $p_state,
            $p_error
            ) = ($1, $2, $3, $4, $5);

        if ($p_comment)
        {
            $in =~ /\n/gs;
        }
        elsif ($p_include)
        {
            my $include = get2Colon(\$in);
            if (grep(/^$include$/,@aIncludes)==0)
            {
                push (@aIncludes, $include);
            }
        }
        elsif ($p_event)
        {
            my $inputEvent = get2Colon(\$in);
            parseEvent($inputEvent);            
        }
        elsif ($p_state)
        {
            $in =~ /([^\s\t\n]+)[\s\t\n]*=[\s\t\n]*\{/gs;   
            my $state = $1;
            my $inputState = get2Brace(\$in);
            parseState($state, $inputState);
        }
        else
        {
            die "PARSE ERROR in $resfile: $p_error!\n";
        }
    }
    




    close(FRES);
}


sub createUserEventName
{
    my $event = shift;
    #if (exists $hEvent2Id{$event})
    #{
    #    $event = "M_EVT_".$event; 
    #}
    return $event;
}


sub createStateName
{
    my $state = shift;
    $state = "eM_".$state;
    return $state;
}


sub createFunctionName
{
    my $function = shift;
    $function = "eM_".$function;
    return $function;
}



sub createEventFileH
{
    my $eventfile = shift;
    
    if (!-d $mstateHdir)
    {
        mkdir($mstateHdir);
    }
    open(EFILE, ">$mstateHdir/$eventfile") or die "ERROR: Cannot open $eventfile\n";
    print EFILE "#ifndef _M_EVENT_H\n";
    print EFILE "#define _M_EVENT_H\n";
    print EFILE $fheader;

    print EFILE "#include <fwl_vme.h>\n";
    
    print EFILE "#define MAX_EVENTQUEUE_ENTRIES $event_entries\n\n";


    #print EFILE "typdef enum {\n";
    
    my @aEvents = sort keys %hEvent2Id;
    my $id;
    for ($id=1; $id<=@aEvents; $id++)
    {
        my $event = createUserEventName($hId2Event{$id-1});
        print EFILE "  #define $event (VME_EVT_USER + $id)\n";
    }    
#    print EFILE "  " . $hId2Event{$id-1} . " = VME_EVT_USER + $id\n";
    
    #print EFILE "} M_EVENTS;\n";
    print EFILE "#endif\n";

    close(EFILE);
}




sub createStateFileH
{
    my $statefile = shift;
    if (!-d $mstateHdir)
    {
        mkdir($mstateHdir);
    }
    open(SFILE, ">$mstateHdir/$statefile") or die "ERROR: Cannot open $statefile\n";
    print SFILE "#ifndef _M_STATE_H\n";
    print SFILE "#define _M_STATE_H\n";
    
    print SFILE $fheader;
    print SFILE "#include \"$eventfileH\"\n\n\n";
    
    print SFILE "#define MAX_STACK_DEPTH $stack_depth\n\n";
    
# print states enum
    my $id;
    my @aStates = sort keys %hStates;
    print SFILE "typedef enum \n{\n";
    for ($id=0; $id<@aStates-1; $id++)
    {
        my $name =  createStateName($hId2State{$id});
        print SFILE "  $name = $id,\n";
    }
    my $name =  createStateName($hId2State{$id});
    print SFILE "  $name = $id\n";
    print SFILE "} M_STATES;\n";

# print state function enum    

    my $id;
    my @aFuncs = sort keys %hFunctions;
    if (@aFuncs)
    {
        print SFILE "typedef enum \n{\n";
        for ($id=0; $id<@aFuncs-1; $id++)
        {
            my $name = createFunctionName($aFuncs[$id]);
            print SFILE "  $name = $id,\n";
        }
        my $name = createFunctionName($aFuncs[$id]);
        print SFILE "  $name = $id\n";
        print SFILE "} M_FUNCTIONS;\n";
    }
    

# print event funcction enum


   my $id;
    my @aFuncs = sort keys %heFunctions;
    if (@aFuncs)
    {
        print SFILE "typedef enum \n{\n";
        for ($id=0; $id<@aFuncs-1; $id++)
        {
            my $name = createFunctionName($aFuncs[$id]);
            print SFILE "  $name = $id,\n";
        }
        my $name = createFunctionName($aFuncs[$id]);
        print SFILE "  $name = $id\n";
        print SFILE "} M_EFUNCTIONS;\n";
    }
        
  print SFILE "#endif\n";
  close(SFILE);
}


sub printNextTrans
{
    my $state = shift;
    my $transition = shift;
    my $type = shift;

    if ($transition =~ /_return\s*(\(\s*((STACKROOT)|(\d+))\s*\))*\s*/)
    {
      if ($3)
	{
	  print SFILE "      *pTrans = eM_TYPE_RETURN_ROOT;\n";
	}
      elsif($4)
	{
	  print SFILE "      *pTrans = eM_TYPE_RETURN + $4;\n";
	}
      else
	{
	  print SFILE "      *pTrans = eM_TYPE_RETURN;\n";
	}
    }
    elsif (exists $hStates{$state}->{FUNCTIONS}->{$transition})
    {
        my $name = createFunctionName($transition);
        print SFILE "      nextstate = $name;\n";
        if ($hStates{$state}->{FUNCTIONS}->{$transition} eq "state")
        {
            print SFILE "      *pTrans = eM_TYPE_SCALL;\n";
        }
        else
        {
            print SFILE "      *pTrans = eM_TYPE_ECALL;\n";
        }
    }
    elsif ($type eq "-")                
    {
        my $name = createStateName($transition);
        print SFILE "      nextstate = $name;\n";
        print SFILE "      *pTrans = eM_TYPE_EXIT;\n";
    }
    else
    {
        my $name = createStateName($transition);
        print SFILE "      nextstate = $name;\n";
        print SFILE "      *pTrans = eM_TYPE_IRPT;\n";
    }
}

sub createStateFileC
{
    

    my $statefile = shift;
    my %hFuncs;
    my $state;
    my @aStates = sort keys %hStates;
    
    foreach $state (@aStates)
    {
        my @aFuncs = ("INIT", "EXIT", "PAINT");
        my $func;
        foreach $func (@aFuncs)
        {
            if (exists $hStates{$state}->{$func})
            {
#                if ($func eq "HANDLE")
#                {
#                    $hFuncs{$hStates{$state}->{$func}} = "unsigned char $hStates{$state}->{$func}(T_EVT_CODE event, T_EVT_PARAM *pEventParm)";
 #               }
 #               else
 #               {
                    $hFuncs{$hStates{$state}->{$func}} = "void $hStates{$state}->{$func}(void)";
  #              }
            }
            #else
            #{
            #    die "ERROR!\n";
            #}
        }
    }

    if (!-d $mstateCdir)
    {
        mkdir($mstateCdir);
    }
    open(SFILE, ">$mstateCdir/$statefile") or die "ERROR: Cannot open $statefile\n";
    print SFILE $fheader;
    
    my $include;
    foreach $include (@aIncludes)
    {
        print SFILE "#include $include\n";
    }

    print SFILE "#include \"$eventfileH\"\n";
    print SFILE "#include \"$statefileH\"\n";
    print SFILE "#include \"Lib_state_api.h\"\n";

    print SFILE q~

#include <stdio.h>
#include <string.h>


/* ---------------------------------------------------------------------------------------*/
/*                     state functions: init, exit, paint                                 */
/* ---------------------------------------------------------------------------------------*/
~;
    my @aFuncs = sort keys %hFuncs;
    my $func;
    foreach $func (@aFuncs)
    {
        print SFILE "extern $hFuncs{$func};\n"
    }


   print SFILE q~/* ---------------------------------------------------------------------------------------*/
/*                      functions                                                        */
/* ---------------------------------------------------------------------------------------*/
~;


    my @aFuncs = sort keys %hFunctions;
    my $func;
    foreach $func (@aFuncs)
    {
        print SFILE "extern unsigned char $func(T_EVT_CODE event, T_EVT_PARAM *pEventParm);\n";
    }

   print SFILE q~/* ---------------------------------------------------------------------------------------*/
/*                     event functions                                                   */
/* ---------------------------------------------------------------------------------------*/
~;
    my @aFuncs = sort keys %heFunctions;
    my $func;
    foreach $func (@aFuncs)
    {
        print SFILE "extern unsigned char $func(T_EVT_CODE* event, T_EVT_PARAM** pEventParm);\n";
    }



   print SFILE q~/* ---------------------------------------------------------------------------------------*/
/*                     state functions:    getNext                                        */
/* ---------------------------------------------------------------------------------------*/
~;

    foreach $state (@aStates)
    {
        my $nextfunc = $hStates{$state}->{NEXT};
        print SFILE "static int $nextfunc(T_EVT_CODE event, M_TRANSTYPE *pTrans);\n";
    }
   
    
   print SFILE q~/* ---------------------------------------------------------------------------------------*/
/*                     state structs                                                      */
/* ---------------------------------------------------------------------------------------*/
~;

    # print state arrays
    foreach $state (@aStates)
    {
        
        print SFILE "static const M_STATESTRUCT m_$state =\n{\n";

        my $id;
        for ($id=0; $id <@aStateAttr-1; $id++)
        {
            if (exists $hStates{$state}->{$aStateAttr[$id]})
            {
                print SFILE "  $hStates{$state}->{$aStateAttr[$id]},\n";
            }
            else
            {
                print SFILE "  (void *)0,\n";
            }
        }
        if (exists $hStates{$state}->{$aStateAttr[$id]})
        {
            print SFILE "  $hStates{$state}->{$aStateAttr[$id]}\n";
        }
        else
        {
            print SFILE "  (void *)0\n";
        }
        print SFILE "};\n";
    }  
     

   print SFILE q~/* ---------------------------------------------------------------------------------------*/
/*                     next function implementations                                      */
/* ---------------------------------------------------------------------------------------*/
~;
    # print next function implementations
    foreach $state (@aStates)
    {
        my $nextfunc = $hStates{$state}->{NEXT};
        print SFILE "static int $nextfunc(T_EVT_CODE event, M_TRANSTYPE *pTrans)\n{\n";
        print SFILE "  M_STATES nextstate = SM_GetCurrentSM();\n";

        my $transid;
        my @aEvents = sort keys %{$hStates{$state}->{hTRANSITIONS}};
	my @aNormalEvents = @aEvents;
        my $defaultTrans = "";
        my $defaultType  = "";
        
        my $printMainCodesStart=1;
        foreach $transid (@aEvents)
        { 
            my $transition = $hStates{$state}->{hTRANSITIONS}->{$transid}->{TRANS};
            my $event = $hStates{$state}->{hTRANSITIONS}->{$transid}->{EVENT};
            my $type = $hStates{$state}->{hTRANSITIONS}->{$transid}->{TYPE};
            if ($event =~ "maincode\((.+)\)")
            {
                if ($printMainCodesStart)                    
                {
                    $printMainCodesStart=0;
                    print SFILE "  switch(VME_EVT_MAINCODE(event))\n  {\n";
                }
                my $event = $1;
                $event =~ s/\(|\)//gs;

                my $name = createUserEventName($event);
                print SFILE "    case $name:\n";
                printNextTrans($state, $transition, $type);                
                print SFILE "      break;\n";
                @aNormalEvents = grep(!/^$transid/, @aNormalEvents);    
            }
        }
	@aEvents = @aNormalEvents;

        if ($printMainCodesStart==0)
        {
            print SFILE "    default:\n";
            print SFILE "      break;\n";
            print SFILE "  }\n";
        }


        print SFILE "  switch(event)\n  {\n";
        foreach $transid (@aEvents)
        { 
            my $transition = $hStates{$state}->{hTRANSITIONS}->{$transid}->{TRANS};
            my $event = $hStates{$state}->{hTRANSITIONS}->{$transid}->{EVENT};
            my $type = $hStates{$state}->{hTRANSITIONS}->{$transid}->{TYPE};
            if ($event ne "_default")
            {            
                if ($event =~ /\:/)
                {
                    my ($start, $stop) = split(/:/, $event);
                    my $startid = $hEvent2Id{$start}; 
                    my $stopid = $hEvent2Id{$stop}; 
                    my $id;
                    for ($id=$startid; $id<=$stopid; $id++)
                    {
                        if (!exists $hId2Event{$id})
                        {
                            die "ERROR: Cannot use : operator for external events in $event\n";
                        }
                        my $name = createUserEventName($hId2Event{$id});
                        print SFILE "    case $name:\n";
                    }

                }
                elsif ($event =~ /\,/)
                {
                    my @aEvents = split(/,/, $event);
                    foreach $event (@aEvents)
                    {
                        my $name = createUserEventName($event);
                        print SFILE "    case $name:\n";
                    }
                }
                else
                {
                    my $name = createUserEventName($event);
                    print SFILE "    case $name:\n";
                }

                printNextTrans($state, $transition, $type);                
                print SFILE "      break;\n";
            }
            else
            {
                  $defaultTrans = $transition;
                  $defaultType  = $type;
            }
            
        }
        print SFILE "    default:\n";
        if ($defaultTrans ne "")
        {
            printNextTrans($state, $defaultTrans, $defaultType);
        }
        print SFILE "      break;\n";
        print SFILE "  }\n";
        print SFILE "  return(nextstate);\n";
        print SFILE "}\n";
    }
    

   print SFILE q~/* ---------------------------------------------------------------------------------------*/
/*                    state array                                                         */
/* ---------------------------------------------------------------------------------------*/
~;
    # print state Array
    
    my $id;
    my @aStates = sort keys %hStates;
    my $state;
    print SFILE "static const M_STATESTRUCT* m_statearray[] = \n{\n";
    for ($id=0; $id<@aStates-1; $id++)
    {
        $state = $hId2State{$id};
        print SFILE "  &m_$state,\n";
    }
    $state = $hId2State{$id};
    print SFILE "  &m_$state\n";
    print SFILE "};\n";


   print SFILE q~/* ---------------------------------------------------------------------------------------*/
/*                    efunction array                                                      */
/* ---------------------------------------------------------------------------------------*/
~;

    print SFILE "static const _feHandle m_efuncarray[] = \n{\n";;

    my @aFuncs = sort keys %heFunctions;
    if (@aFuncs)
    {
        my $func;        
        for ($id=0; $id<@aFuncs-1; $id++)
        {            
            print SFILE "  $aFuncs[$id],\n";
        }        
        print SFILE "  $aFuncs[$id]\n";
    }
    else
    {
        print SFILE "(void *)0\n"   
    } 

    print SFILE "};\n";
  print SFILE q~/* ---------------------------------------------------------------------------------------*/
/*                    function array                                                      */
/* ---------------------------------------------------------------------------------------*/
~;

    print SFILE "static const _fHandle m_funcarray[] = \n{\n";

    my @aFuncs = sort keys %hFunctions;

    if (@aFuncs)
    {
        my $func;        
        for ($id=0; $id<@aFuncs-1; $id++)
        {            
            print SFILE "  $aFuncs[$id],\n";
        }        
        print SFILE "  $aFuncs[$id]\n";

    }
    else
    {
        print SFILE "(void *)0\n"
    }
    print SFILE "};\n";

   print SFILE q~/* ---------------------------------------------------------------------------------------*/
/*                    porting lib functions                                                   */
/* ---------------------------------------------------------------------------------------*/
~;

    print SFILE q~const _fHandle* SM_GetfHande(void)
{
	return m_funcarray;
}

const _feHandle* SM_GeteHandle(void)
{
	return m_efuncarray;
}

const M_STATESTRUCT** SM_GetStateArray(void)
{
	return m_statearray;
}

~;
    close(SFILE);
}

sub mkdir_f
  {
    my $dir = shift;
    if (!(-d $dir))
      {
	$dir =~ tr/\\/\//;
	my @aDirs = split(/\//, $dir);
	my $createdir;
	while (my $subdir = shift(@aDirs))
	  {
	    $createdir .= $subdir . "/";
	    if (!(-d $createdir))
	      {
		mkdir($createdir) or die "ERROR: Cannot create directory  $createdir!\n";
	      }
	  }
      }
  }

sub createStubs
  {
    my %hModules;

    my $header = q~
#include "fwl_vme.h"
#include "Lib_event.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
~;
    my $state;
    my @aStates = sort keys %hStates;

    foreach $state (@aStates)
    {

        if (exists $hStates{$state}->{MODULE})
        {

	  my $module = $hStates{$state}->{MODULE};

	  $hModules{$module}->{OUTPUT} .= "/*---------------------- BEGIN OF STATE $state ------------------------*/\n";
	  my @aFuncs = ("INIT", "EXIT", "PAINT");
	  my $func;
	  foreach $func (@aFuncs)
	    {
	      if (exists $hStates{$state}->{$func})
		{
		  $hModules{$module}->{OUTPUT} .= "void $hStates{$state}->{$func}(void)\n\{\n\n\}\n";
		}
	      else
		{
		  die "ERROR!\n";
		}
	    }

	  my @aFuncs = sort keys %hFunctions;
	  my $func;
	  foreach $func (@aFuncs)
	    {
	      if ($hFunctions{$func} eq $state)
		{
		   $hModules{$module}->{OUTPUT} .= "unsigned char $func(T_EVT_CODE event, T_EVT_PARAM *pEventParm)\n";
		   $hModules{$module}->{OUTPUT} .= "{\n	if (IsPostProcessEvent(event))\n	{\n		return 1;\n	}\n\n	return 0; \n}\n";
		}
	    }

	  my @aFuncs = sort keys %heFunctions;
	  my $func;
	  foreach $func (@aFuncs)
	    {
	      if ($heFunctions{$func} eq $state)
		{
		   $hModules{$module}->{OUTPUT} .= "unsigned char $func(T_EVT_CODE *event, T_EVT_PARAM **pEventParm)\n";
		   $hModules{$module}->{OUTPUT} .= "{\n   return(1); \n}\n";
		}
	    }
	}
      }

    my @aModules = sort keys %hModules;
    my $module;
    foreach $module (@aModules)
      {
	my ($name,$dir,$type) = fileparse($module);

	mkdir_f($dir);

	if (open (MFILE, ">$module.c"))
	  {
	    print MFILE $fheader;
	    print MFILE $header;
	    print MFILE $hModules{$module}->{OUTPUT};
	  }
	else
	  {
	    print "WARNING: Cannot Open $module!\n";
	  }
      }
  }

sub buildTransHashes
  {
    my $state;
    my @aStates = sort keys %hStates;
    foreach $state (@aStates)
      {
	my @aEvents = sort keys %{$hStates{$state}->{hTRANSITIONS}};
	my $transid;
	foreach $transid (@aEvents)
	  {
	    my $transition = $hStates{$state}->{hTRANSITIONS}->{$transid}->{TRANS};
            my $event = $hStates{$state}->{hTRANSITIONS}->{$transid}->{EVENT};
            my $type = $hStates{$state}->{hTRANSITIONS}->{$transid}->{TYPE};

	    if (!exists $hStates{$state}->{FUNCTIONS}->{$transition})
	      {
		$hStateTo{$state}->{$transition}->{"$type.$event"} = 1;
		$hStateFrom{$transition}->{$state}->{"$type.$event"} = 1;
	      }
	  }
      }
  }


sub printWarning
  {
    my $text = shift;
    if ($genWarnings == 1)
      {
	printOnce($text, \%hWarnings);
      }
  }

sub printOnce
  {
    my $text = shift;
    my $rhAll = shift;

    if (!exists $$rhAll{$text})
      {
	print $text;
	$$rhAll{$text} = 1;
      }
  }

sub checkTransitions
{
    my $state = shift;

    my $rStateHash = \%hStateTo;
    my @aStack;
    my $maxStack=0;
    my %hDeadEnds;
    my %hRecRoutes;

sub diveF
{
    my $state = shift;
    my $raBack = shift;
    my $raBackTrans;
    my $popstack = 0;
    my $stacked = 0;
    my @stackedEvents;
    if ($state =~ /_return/)
      {
	if (@aStack == 0)
	  {
	    my $prev = $$raBack[@$raBack-1];
	    printWarning( "WARNING: Transistion to $state in state $prev has no effect!\n");
	  }
	$popstack = 1;
      }
    if (@$raBack)
    {
	my $previous = $$raBack[@$raBack-1];

	my @aEvents = sort keys %{$$rStateHash{$previous}->{$state}};
	if ((@stackedEvents = grep(/\+/, @aEvents))!=0)
	  {
	    $stacked = 1;
	    push (@aStack, $previous);
	    if (@aStack > $maxStack)
	      {
		$maxStack = @aStack;
	      }
	  }
    }

    push(@$raBack,$state);
    push(@$raBackTrans,$stacked);

    if (exists $$rStateHash{$state})
      {
	my $nextState;
	my @aNextStates = keys %{$$rStateHash{$state}};
	foreach $nextState (@aNextStates)
	  {
	    if (grep(/^$nextState$/, @$raBack)!=0)
	      {
		if (grep(/^1$/, @$raBackTrans)!=0)
		  {
		    my @aRoute = @$raBack;
		    push(@aRoute, $nextState); 
		    my $prev;
		    while (($prev = shift(@aRoute)) ne $nextState){}
		    my $cur;
		    my $route = $prev;
		    while ($cur = shift(@aRoute))
		      {
			my @aEvents = sort keys %{$$rStateHash{$prev}->{$cur}};
			if (grep(/\+/, @aEvents)!=0)
			  {
			    $route .= "+>$cur";
			  }
			else
			  {
			    $route .= "->$cur";
			  }
			$prev = $cur;
		      }
		    printOnce("WARNING: Recursive routes using stacked transitions are not allowed:\n   $route!\n",
			      \%hErrors);
		  }
	      }
	    else
	      {
		if ($nextState)
		  {
		    if (diveF($nextState,\@$raBack, \@$raBackTrans))
		      {
			$popstack = 1;
		      }
		  }
	      }
	  }
      }
    else
      {
	if (!($state =~ /_return/) && !exists $hDeadEnds{$state})
	  {
	    $hDeadEnds{$state} = 1;
	    printWarning("WARNING: There are no transistions for $state!\n");
	  }
      }

    if ($stacked==1)
      {
	if ($popstack==0)
	  {
	    printWarning("WARNING: Stacked transistion  to state $state never returns\n");
	  }
	pop(@aStack);
      }


   # my $x = @aStack;
   # my $stack = join(" ", @aStack);
   # my $route = join(" ", @$raBack);
   # print "$x State :($state)\nRoute: $route\nStack: $stack\n\n";

   
    pop(@$raBack);
    pop(@$raBackTrans);
    return $popstack;
  }
    

my @aRoute;
my @aTrans;
diveF($state, \@aRoute, \@aTrans);
print "\n\nMaximal stack size is $maxStack!\n";
  if ($maxStack > $stack_depth)
  {
    $stack_depth = $maxStack;
    print "Increased stack to $maxStack\n";
  }
}


# ----------------------------------------------------------------- main
# ---------------------------------------------------------- handle arguments
my $Version = ' $Revision: 1.11 $ ';
my $VerString = $Version;
$VerString =~ s/\$|\s//g;
$VerString =~ s/.*?\\(\d+)$/$1/;
print "statecc.pl Version $VerString written by F.Klamroth ICM WM SW7\n";
print "Copyright Anyka 2005\n";


handleArgs();
print "$resfile\n";
parseRes($resfile);
buildTransHashes();
checkTransitions($hId2State{"0"});

my @aErrors = keys %hErrors;
if(@aErrors)
  {
    if ($genErrors  ==1 )
      {
	die;
      }
  }



createEventFileH($eventfileH);
createStateFileC($statefileC);
createStateFileH($statefileH);

if ($genStubs==1)
{
    createStubs();
}

