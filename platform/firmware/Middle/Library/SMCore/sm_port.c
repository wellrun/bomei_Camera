#include "Lib_event.h"
#include "Lib_state.h"

#include "sm_port.h"
#include "smport_cfg.h"
#include "Eng_Debug.h"

static T_U8 gb_StackBuffer[MAX_STACK_BUFFER];
// 因为这个原始buffer最终是转换为事件队列的buffer使用的,即最终转换为M_EVENTENTRY *指针。
// 而作事件队列的buffer必须满足被sizeof(M_EVENTENTRY)和4整除的条件
// 为了即使程序空间改变，事件队列的长度固定，所以gb_EventQueueBuffer要加sizeof(M_EVENTENTRY)
// 而sizeof(M_EVENTENTRY)刚好为12,因此定义gb_EventQueueBuffer时需要加12
static T_U8 gb_EventQueueBuffer[MAX_EVENTQUEUE_BUFFER + 12];


M_STATES SM_GetPreProcID(void)
{
	return eM_preproc;
}

M_STATES SM_GetPostProcID(void)
{
	return eM_postproc;
}

vT_EvtCode SM_GetEvtReturn(void)
{
	return M_EVT_RETURN;
}

vT_EvtCode SM_GetEvtNoNext(void)
{
	return VME_EVT_USER;
}


void *SM_GetStackBuffer(unsigned int *bufSize)
{
	if(bufSize)
	{
		(*bufSize) = MAX_STACK_BUFFER;
	}

	AK_ASSERT_VAL(MAX_STACK_BUFFER > SM_CalcStackBufByMaxDepth(MAX_STACK_DEPTH), 
		"SM_GetStackBuffer warning, add more stack buffer", 
		gb_StackBuffer);

	return gb_StackBuffer;
}

void *SM_GetEventQueueBuffer(unsigned int *bufSize)
{
	if(bufSize)
	{
		(*bufSize) = MAX_EVENTQUEUE_BUFFER;
	}
	
	AK_ASSERT_VAL(MAX_EVENTQUEUE_BUFFER > SM_CalcEventBufferByMaxEntries(MAX_EVENTQUEUE_ENTRIES),
		"SM_GetEventQueueBuffer warning, add more EventQueue buffer", 
		gb_EventQueueBuffer);

	return gb_EventQueueBuffer;
}

