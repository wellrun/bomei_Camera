
#include "Fwl_public.h"

/*---------------------- BEGIN OF STATE s_pub_reserved3 ------------------------*/
void initpub_reserved3(void)
{

}

void exitpub_reserved3(void)
{

}

void paintpub_reserved3(void)
{

}

unsigned char handlepub_reserved3(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    if (IsPostProcessEvent(event))
    {
        return 1;
    }

    return 0;
}
