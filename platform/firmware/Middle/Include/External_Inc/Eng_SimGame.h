#ifndef __ENG_SIMGAME_H__
#define __ENG_SIMGAME_H__

#include "Ctl_Msgbox.h"
#include "Gbl_Global.h"
#include "Fwl_osFs.h"

#define SAVE_FILE_SIZE  (500*1024)

typedef struct {
        T_MSGBOX            msgbox;
        T_USTR_FILE         file_path_name;
        T_USTR_FILE         file_path;
        T_hFILE             fhandle;
        T_TIMER             game_timer;
        T_U16               volume;
        T_BOOL              game_audio_switch_on;
    } T_GAME_PLAY_PARM;

#endif
