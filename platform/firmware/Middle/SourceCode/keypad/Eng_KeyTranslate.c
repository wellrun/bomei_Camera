#include "anyka_types.h"
#include "Eng_KeyTranslate.h"
#include "gbl_global.h"
#include "Eng_Debug.h"

#define  MODULE_NAME "KeyTranslate"

static T_S32 Eng_DefKeyTranslate(T_MMI_KEYPAD * pKeyParam);
static Eng_T_fKEY_TRANSLATE   m_pKeyTranslate=Eng_DefKeyTranslate;
static T_BOOL longKey = AK_FALSE;

/**
 * @brief Query :default translate function
 *
 * @author 	Liuguodong
 * @param	pKeyParam[in/out], key for translating
 * @return 	0=success, other=failed
 */ 
static T_S32 Eng_DefKeyTranslate(T_MMI_KEYPAD * pKeyParam)
{
	Fwl_Print(C4,MODULE_NAME,"Eng_DefKeyTranslate is vorked!\r\n");
	
	switch (pKeyParam->keyID)
	{
		case kbLEFT:
			if (pKeyParam->pressType == PRESS_LONG)
			{
				longKey = AK_TRUE;
				pKeyParam->keyID = kbVOICE_DOWN;
				pKeyParam->pressType = PRESS_SHORT;
			}
			else if(longKey && pKeyParam->pressType == PRESS_SHORT)
			{
				pKeyParam->keyID = kbVOICE_DOWN;
				pKeyParam->pressType = PRESS_SHORT;
			}
			else if(longKey && pKeyParam->pressType == PRESS_UP)
			{
				longKey = AK_FALSE;
			}
			break;
		case kbRIGHT:
			if (pKeyParam->pressType == PRESS_LONG)
			{
				longKey = AK_TRUE;
				pKeyParam->keyID = kbVOICE_UP;
				pKeyParam->pressType = PRESS_SHORT;
			}
			else if(longKey && pKeyParam->pressType == PRESS_SHORT)
			{
				pKeyParam->keyID = kbVOICE_UP;
				pKeyParam->pressType = PRESS_SHORT;
			}
			else if(longKey && pKeyParam->pressType == PRESS_UP)
			{
				longKey = AK_FALSE;
			}
			break;
		case kbMENU:
			if (pKeyParam->pressType == PRESS_SHORT)
				pKeyParam->keyID = kbOK;
			else if(pKeyParam->pressType == PRESS_LONG)
			{
				pKeyParam->pressType = PRESS_SHORT;
			}
			break;
		case kbOK:
			pKeyParam->keyID = kbMENU;
			break;
			
	}

	//stop menu key's short press from holding key after long press.
	if (pKeyParam->keyID ==kbMENU)
		keypad_keystop();
		
	return 0;
}

T_S32 Eng_KeyTranslate(T_MMI_KEYPAD * pKeyParam)
{
	T_S32 iResult=0 ;

	Fwl_Print(C4,MODULE_NAME,"Eng_KeyTranslate is vorked!\r\n");
	
	//judge if supporting key translating
	if (!Eng_TRANSLATE_SUPPORT)
		return 0;

	
	if (m_pKeyTranslate !=AK_NULL)
	{
		T_MMI_KEYPAD  keyBefore = *pKeyParam;
		iResult =  m_pKeyTranslate(pKeyParam);
		Fwl_Print(C4,MODULE_NAME,"translate keyId:%d->%d ,keytype:%d->%d\r\n"
			,keyBefore.keyID,pKeyParam->keyID,keyBefore.pressType,pKeyParam->pressType);
	}
	return iResult;
}

T_VOID Eng_SetKeyTranslate(Eng_T_fKEY_TRANSLATE  pKeyTranslate )
{
	m_pKeyTranslate = pKeyTranslate ;
}

T_VOID Eng_SetDefKeyTranslate(T_VOID)
{
	m_pKeyTranslate = Eng_DefKeyTranslate ;
}

T_S32 Eng_StandbyTranslate(T_MMI_KEYPAD * pKeyParam)
{
	Fwl_Print(C4,MODULE_NAME,"Eng_StandbyTranslate is vorked!\r\n");

	Eng_DefKeyTranslate(pKeyParam);
	//restore menu short press to long 
	if (pKeyParam->keyID == kbMENU && pKeyParam->pressType ==PRESS_SHORT)
		pKeyParam->pressType = PRESS_LONG;

	return 0;
}
T_S32 Eng_ImgListTranslate(T_MMI_KEYPAD * pKeyParam)
{
	Fwl_Print(C4,MODULE_NAME,"Eng_ImgListTranslate is vorked!\r\n");
	Eng_DefKeyTranslate(pKeyParam);
	
	if (pKeyParam->keyID == kbRIGHT && pKeyParam->pressType ==PRESS_SHORT)
		pKeyParam->keyID= kbSWA;

	return 0;
	
}

T_S32 Eng_ImgThumbnailViewTranslate(T_MMI_KEYPAD * pKeyParam)
{
	Fwl_Print(C4,MODULE_NAME,"Eng_ImgThumbnailViewTranslate is vorked!\r\n");
	Eng_DefKeyTranslate(pKeyParam);

	//translate up long press to  swa long press
	if (pKeyParam->keyID == kbUP && pKeyParam->pressType ==PRESS_LONG)
	{
		pKeyParam->keyID= kbSWA;
		pKeyParam->pressType = PRESS_SHORT;
		keypad_keystop();

	}

	//translate down long press to  ok  long press
	if (pKeyParam->keyID == kbDOWN&&  pKeyParam->pressType ==PRESS_LONG)
	{
		pKeyParam->keyID= kbOK;
		keypad_keystop();
		
	}

	return 0;
	
}



