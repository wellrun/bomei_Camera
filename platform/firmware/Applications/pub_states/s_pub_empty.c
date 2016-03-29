
#include "Fwl_public.h"

/*---------------------- BEGIN OF STATE s_pub_empty ------------------------*/
void initpub_empty(void)
{

}

void exitpub_empty(void)
{

}

void paintpub_empty(void)
{

}

unsigned char handlepub_empty(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    if (IsPostProcessEvent(event))
    {
        return 1;
    }

    return 0;
}
