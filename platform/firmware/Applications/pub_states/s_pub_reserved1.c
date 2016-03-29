
#include "Fwl_public.h"

/*---------------------- BEGIN OF STATE s_pub_reserved1 ------------------------*/
void initpub_reserved1(void)
{

}

void exitpub_reserved1(void)
{

}

void paintpub_reserved1(void)
{

}

unsigned char handlepub_reserved1(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    if (IsPostProcessEvent(event))
    {
        return 1;
    }

    return 0;
}
