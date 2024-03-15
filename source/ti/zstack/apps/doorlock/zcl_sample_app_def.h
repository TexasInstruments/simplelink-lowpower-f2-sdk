/*
 * zcl_sampleAppDef.h
 *
 *  Created on: May 18, 2019
 *      Author: luis
 */

#include "zcl_sampledoorlock.h"

#ifndef APPLICATION_UI_ZCL_SAMPLE_APP_DEF_H_
#define APPLICATION_UI_ZCL_SAMPLE_APP_DEF_H_


#ifndef CUI_DISABLE

#define SAMPLE_APP_MENUS  4

#define CUI_APP_MENU    CUI_MENU_ITEM_INT_ACTION("<   CHANGE PIN   >", (CUI_pFnIntercept_t) zclSampleDoorLock_UiActionEnterPin) \
                            CUI_MENU_ITEM_ACTION("<   LOCK DOOR    >", (CUI_pFnAction_t) zclSampleDoorLock_UiActionLock)  \
                            CUI_MENU_ITEM_ACTION("<  UNLOCK DOOR   >", (CUI_pFnAction_t) zclSampleDoorLock_UiActionUnlock)  \
                            CUI_MENU_ITEM_ACTION("<    DISCOVER    >", (CUI_pFnAction_t) zclSampleDoorLock_UiActionDoorLockDiscoverable)

#define APP_TITLE_STR "TI Sample Doorlock"

#endif  // #ifndef CUI_DISABLE


#endif /* APPLICATION_UI_ZCL_SAMPLEAPPDEF_H_ */
