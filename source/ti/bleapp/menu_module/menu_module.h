/******************************************************************************

@file  menu_module.h

@brief This file contains the Menu module APIs

Group: WCS, BTS
Target Device: cc13xx_cc26xx

******************************************************************************

 Copyright (c) 2023-2025, Texas Instruments Incorporated
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 *  Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

 *  Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

 *  Neither the name of Texas Instruments Incorporated nor the names of
    its contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

******************************************************************************


*****************************************************************************/

#ifndef MENU_MODULE_H_
#define MENU_MODULE_H_

/******************************************************************************
 Includes
 *****************************************************************************/
#include <bcomdef.h>

// Font Colors
#define MENU_MODULE_COLOR_RED               "\033[31m"
#define MENU_MODULE_COLOR_GREEN             "\033[32m"
#define MENU_MODULE_COLOR_YELLOW            "\033[33m"
#define MENU_MODULE_COLOR_BLUE              "\033[34m"
#define MENU_MODULE_COLOR_MAGENTA           "\033[35m"
#define MENU_MODULE_COLOR_CYAN              "\033[36m"
#define MENU_MODULE_COLOR_WHITE             "\033[37m"

// Border Colors
#define MENU_MODULE_BORDER_COLOR_RED        "\033[41m"
#define MENU_MODULE_BORDER_COLOR_GREEN      "\033[42m"
#define MENU_MODULE_BORDER_COLOR_YELLOW     "\033[43m"
#define MENU_MODULE_BORDER_COLOR_BLUE       "\033[44m"
#define MENU_MODULE_BORDER_COLOR_MAGENTA    "\033[45m"
#define MENU_MODULE_BORDER_COLOR_CYAN       "\033[46m"
#define MENU_MODULE_BORDER_COLOR_WHITE      "\033[47m"

// Additions
#define MENU_MODULE_COLOR_RESET             "\033[0m"
#define MENU_MODULE_COLOR_BOLD              "\033[1m"
#define MENU_MODULE_COLOR_UNDERLINE         "\033[4m"
#define MENU_MODULE_COLOR_BLINK             "\033[5m"

/******************************************************************************
 Typedefs
 *****************************************************************************/
// Menu items callback type
typedef void (*MenuModule_pItemCB_t)(uint8 index);

typedef enum
{
    MenuModule_Mode_PRINTS_ONLY,
    MenuModule_Mode_MENU_WITH_BUTTONS
} MenuModule_Mode;

PACKED_ALIGNED_TYPEDEF_STRUCT
{
    char *itemName;
    MenuModule_pItemCB_t itemCallback;
    char *itemHelp;
}MenuModule_Menu_t;

PACKED_ALIGNED_TYPEDEF_STRUCT
{
    uint8 menuNumItems;
    char *menuTitle;
    const MenuModule_Menu_t *menuArray;
}MenuModule_MenuObject_t;

PACKED_ALIGNED_TYPEDEF_STRUCT
{
    MenuModule_Mode mode;
}MenuModule_params_t;

/******************************************************************************
 Function Prototypes
 *****************************************************************************/

/*********************************************************************
 * @fn      MenuModule_doInit
 *
 * @brief   Initialize the menu module according to the selected
 *          parameters provided to the function
 *
 * @param   mainMenu - the main menu pointer
 * @param   pParams - the parameters needed for menu initialization
 *
 * @return  SUCCESS, FAILURE
 */
bStatus_t MenuModule_doInit(const MenuModule_MenuObject_t *mainMenu, MenuModule_params_t* pParams);

/*********************************************************************
 * @fn      MenuModule_paramsInit
 *
 * @brief   Initialize the menu module parametrs with default parameters
 *
 * @param   pParams - the parameters to initialize
 *
 * @return  None
 */
void MenuModule_paramsInit(MenuModule_params_t* pParams);

/*********************************************************************
 * @fn      MenuModule_startSubMenu
 *
 * @brief   Display a sub menu
 *
 * @param   subMenu - a pointer to a sub menu
 *
 * @return  SUCCESS, FAILURE
 */
bStatus_t MenuModule_startSubMenu(const MenuModule_MenuObject_t *subMenu);

/*********************************************************************
 * @fn      MenuModule_goToRoot
 *
 * @brief   Go to the main menu and display it
 *
 * @return  None
 */
void MenuModule_goToRoot(void);

/*********************************************************************
 * @fn      MenuModule_goBack
 *
 * @brief   Go to the last menu that was used and display it
 *
 * @return  None
 */
void MenuModule_goBack(void);

/*********************************************************************
 * @fn      MenuModule_printStringList
 *
 * @brief   Display a dynamic menu
 *
 * @param   stringList - a pointer to a dynamic menu
 * @param   listSize - the size of the menu
 *
 * @return  SUCCESS, FAILURE
 */
bStatus_t MenuModule_printStringList(const MenuModule_MenuObject_t *stringList, uint8 listSize);

/*********************************************************************
 * @fn      MenuModule_vprintf
 *
 * @brief   Calls the Display_doVprintf function of the Display module
 *
 * @param   line - line index (0..)
 * @param   column - column index (0..)
 * @param   fmt - format string
 * @param   ... - optional arguments
 *
 * @return  None
 */
void MenuModule_vprintf(uint8_t line, uint8_t column, const char *fmt, ...);

/*********************************************************************
 * @fn      MenuModule_doClearLines
 *
 * @brief   Calls the Display_clearLines function of the Display module
 *
 * @param   fromLine - line index (0 .. )
 * @param   toLine - line index (0 .. )
 *
 * @return  None
 */
void MenuModule_doClearLines(uint8_t fromLine, uint8_t toLine);

/******************************************************************************
 Macros
 *****************************************************************************/

/*!
 *  @brief Creates a menu object.
 *
 *  Initialize a menu object struct from the provided _title and _menu
 */
#define MENU_MODULE_MENU_OBJECT(_title, _menu)                      \
static const MenuModule_MenuObject_t _menu##Object =                \
{                                                                   \
    .menuArray = _menu,                                             \
    .menuNumItems = (sizeof(_menu)/sizeof(_menu[0])),               \
    .menuTitle = (char *)_title                                     \
}

#if !defined(Display_DISABLE_ALL)
/*! Initializes the Menu module. \see MenuModule_doInit() */
#define MenuModule_init(mainMenu, pParams) MenuModule_doInit(mainMenu, pParams)
/*! Output string fmt with variable arguments to print \see MenuModule_vprintf() */
#define MenuModule_printf MenuModule_vprintf
#define MenuModule_clearLines(fromLine, toLine) MenuModule_doClearLines(fromLine, toLine)
#else
#define MenuModule_init(mainMenu,pParams) SUCCESS
#define MenuModule_printf(line, col, fmt, ...)
#define MenuModule_clearLines(fromLine, toLine)
#endif

#endif /* MENU_MODULE_H_ */
