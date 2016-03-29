
#include "Fwl_public.h"

/*---------------------- BEGIN OF STATE s_pub_reserved2 ------------------------*/
void initpub_reserved2(void)
{

}

void exitpub_reserved2(void)
{

}

void paintpub_reserved2(void)
{

}

unsigned char handlepub_reserved2(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    if (IsPostProcessEvent(event))
    {
        return 1;
    }

    return 0;
}
