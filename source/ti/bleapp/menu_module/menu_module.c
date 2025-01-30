/******************************************************************************

@file  menu_module.c

@brief This file contains the Menu module implementation

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

/******************************************************************************
 Includes
 *****************************************************************************/
#include <ti/bleapp/menu_module/menu_module.h>
#include <ti/bleapp/ble_app_util/inc/bleapputil_api.h>
#include <ti/drivers/apps/Button.h>
#include <ti/display/Display.h>
#include <string.h>
#include "ti_drivers_config.h"

#if !defined(Display_DISABLE_ALL)
typedef enum
{
    // Row to display the navigation bar header
    MENU_MODULE_NAV_BAR_HEADER,
    // Row to display button 1 options
    MENU_MODULE_NAV_BAR_BTN_1,
    // Row to display button 2 options
    MENU_MODULE_NAV_BAR_BTN_2,
    // Row to display the separator between the navigation bar and the menu
    MENU_MODULE_NAV_BAR_END,
    // Row to display the Title
    MENU_MODULE_TITLE,
    // First row to display the menu items
    MENU_MODULE_FIRST_ITEM,
    // Last row to display the menu items
    MENU_MODULE_LAST_ITEM = MENU_MODULE_FIRST_ITEM + 3,
    // Row to display a separator after the menu
    MENU_MODULE_SEPARATE_ROW,
    // The initial row of the status lines
    MENU_MODULE_INITIAL_STATUS_OFFSET
}MebuModule_menuRows;

typedef enum
{
    MENU_MODULE_FIRST_INDEX,
    MENU_MODULE_SECOND_INDEX
}MebuModule_indexMapping;

#define MENU_MODULE_FIRST_ITEM_INDEX        0
#define MENU_MODULE_MAX_MENUS_DEPT          6

#define MENU_MODULE_LONG_PRESS_DURATION     400
#define MENU_MODULE_DEBOUNCE_DURATION       100

/******************************************************************************
 Globals
 *****************************************************************************/
// Menus related parameters
// Array of menus
static MenuModule_MenuObject_t const *MenuModule_menuArray [MENU_MODULE_MAX_MENUS_DEPT] = {NULL};
// Array of menus sizes
static uint8 MenuModule_menuSize[MENU_MODULE_MAX_MENUS_DEPT] = {0};
// The index of the current menu item to display
static uint8 MenuModule_currentLocation = MENU_MODULE_FIRST_ITEM_INDEX;
// The index of the current used menu in the menus array
static uint8 MenuModule_menuArrayIndex = 0;
// Indicates if the menu was initialized
static uint8 MenuModule_isInitialized = false;
// The row offset for application prints
static uint8 MenuModule_printStartingRow = 0;

// Display Interface handler
Display_Handle dispHandle = NULL;

// Buttons handles
Button_Handle handle_left;
Button_Handle handle_right;

/******************************************************************************
 Internal functions
 *****************************************************************************/
// Menu internal functions
void MenuModule_printMenuNavBar(void);
bStatus_t MenuModule_startMenu(const MenuModule_MenuObject_t *menu, uint8 menuSize);
void MenuModule_displayMenu(const MenuModule_MenuObject_t *menu, uint8 currentItem, uint8 menuSize);
void MenuModule_displayItem(uint32 itemIndex, uint8 row);
// Functions for handling the array of menus
bStatus_t MenuModule_addSubMenuToArray(const MenuModule_MenuObject_t *menu, uint8 menuSize);
void MenuModule_removeSubMenuFromArray(void);
void MenuModule_removeAllSubMenusFromArray(void);
// Buttons related internal functions
bStatus_t MenuModule_initButtons(void);
void MenuModule_buttonsCallback(Button_Handle buttonHandle, Button_EventMask buttonEvents);
void MenuModule_buttonLeft(char *pData);
void MenuModule_buttonLeftLongPress(char *pData);
void MenuModule_buttonRight(char *pData);
void MenuModule_buttonRightLongPress(char *pData);

/******************************************************************************
 Functions
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
bStatus_t MenuModule_doInit(const MenuModule_MenuObject_t *mainMenu, MenuModule_params_t* pParams)
{
    // and return failure in this case
    if(MenuModule_isInitialized == true || pParams == NULL ||
       (mainMenu == NULL && pParams->mode != MenuModule_Mode_PRINTS_ONLY))
    {
        return FAILURE;
    }

    // Indicate that the menu module is initialized
    MenuModule_isInitialized = true;

    // Initialize and open the display driver
    Display_init();
    dispHandle = Display_open(Display_Type_ANY, NULL);
    if (dispHandle == NULL) {
        return FAILURE;
    }

    // Do the following only if menu is used
    if(pParams->mode != MenuModule_Mode_PRINTS_ONLY)
    {
        // Since the menu is used, start the prints from the following line
        MenuModule_printStartingRow = MENU_MODULE_INITIAL_STATUS_OFFSET;
        // Save the main menu pointer
        MenuModule_menuArray[MenuModule_menuArrayIndex] = mainMenu;
        MenuModule_menuSize[MenuModule_menuArrayIndex] = mainMenu->menuNumItems;

        // Init the buttons
        if (MenuModule_initButtons() != SUCCESS) {
            return FAILURE;
        }

        // Print the navigation bar
        MenuModule_printMenuNavBar();
        // Print the main menu
        MenuModule_goToRoot();
    }

    return SUCCESS;
}

/*********************************************************************
 * @fn      MenuModule_paramsInit
 *
 * @brief   Initialize the menu module parametrs with default parameters
 *
 * @param   pParams - the parameters to initialize
 *
 * @return  None
 */
void MenuModule_paramsInit(MenuModule_params_t* pParams)
{
    pParams->mode = MenuModule_Mode_MENU_WITH_BUTTONS;
}

/*********************************************************************
 * @fn      MenuModule_printMenuNavBar
 *
 * @brief   Prints the menu navigation bar and separator lines
 *
 * @return  None
 */
void MenuModule_printMenuNavBar(void)
{
    // Print the menu navigation bar and separators rows
    char menuNavigation[] = "========================= Menu Navigation =========================";
    char menuSeparators[] = "===================================================================";
    Display_printf(dispHandle, MENU_MODULE_NAV_BAR_HEADER, 0, "%s", menuNavigation);
    Display_printf(dispHandle, MENU_MODULE_NAV_BAR_BTN_1, 0,
                   "|   BTN-1(Left) : Press ---> Next   | Long Press ---> Back        |");
    Display_printf(dispHandle, MENU_MODULE_NAV_BAR_BTN_2, 0,
                   "|   BTN-2(Right): Press ---> Select | Long Press ---> Main Menu   |");
    Display_printf(dispHandle, MENU_MODULE_NAV_BAR_END, 0, "%s", menuSeparators);
    Display_printf(dispHandle, MENU_MODULE_SEPARATE_ROW, 0, "%s", menuSeparators);
}

/*********************************************************************
 * @fn      MenuModule_goToRoot
 *
 * @brief   Go to the main menu and display it
 *
 * @return  None
 */
void MenuModule_goToRoot(void)
{
    // Remove all sub menus before returning to root
    MenuModule_removeAllSubMenusFromArray();
    // Start the root menu
    MenuModule_startMenu(MenuModule_menuArray[0], MenuModule_menuArray[0]->menuNumItems);
}

/*********************************************************************
 * @fn      MenuModule_goBack
 *
 * @brief   Go to the last menu that was used and display it
 *
 * @return  None
 */
void MenuModule_goBack(void)
{
    // Go back to the previous menu
    MenuModule_removeSubMenuFromArray();
    // Display the menu starting from the first menu item
    MenuModule_currentLocation = MENU_MODULE_FIRST_ITEM_INDEX;
    MenuModule_displayMenu(MenuModule_menuArray[MenuModule_menuArrayIndex], MenuModule_currentLocation,
                           MenuModule_menuSize[MenuModule_menuArrayIndex]);
}

/*********************************************************************
 * @fn      MenuModule_startSubMenu
 *
 * @brief   Display a sub menu
 *
 * @param   subMenu - a pointer to a sub menu
 *
 * @return  SUCCESS, FAILURE
 */
bStatus_t MenuModule_startSubMenu(const MenuModule_MenuObject_t *subMenu)
{
    // If the menu doesn't exist, return error
    if(subMenu == NULL)
    {
       return FAILURE;
    }

    // Start the sub menu
    return MenuModule_startMenu(subMenu, subMenu->menuNumItems);
}

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
bStatus_t MenuModule_printStringList(const MenuModule_MenuObject_t *stringList, uint8 listSize)
{
    // If the menu doesn't exist, return error
    if(stringList == NULL || listSize == 0)
    {
        return FAILURE;
    }

    // Start the stringList
    return MenuModule_startMenu(stringList, listSize);
}

/*********************************************************************
 * @fn      MenuModule_startMenu
 *
 * @brief   Adds the menu to the menus array and calls
 *          MenuModule_displayMenu to display it
 *
 * @param   menu - a pointer to the menu
 * @param   menuSize - the size of the menu
 *
 * @return  SUCCESS, FAILURE
 */
bStatus_t MenuModule_startMenu(const MenuModule_MenuObject_t *menu, uint8 menuSize)
{
    bStatus_t status = SUCCESS;

    // Set the current menu item to the first one
    MenuModule_currentLocation = MENU_MODULE_FIRST_ITEM_INDEX;

    // If this is a sub menu, add it to the array
    if(menu != MenuModule_menuArray[0])
    {
        status = MenuModule_addSubMenuToArray(menu, menuSize);
    }

    // Display the menu
    MenuModule_displayMenu(MenuModule_menuArray[MenuModule_menuArrayIndex], MenuModule_currentLocation,
                           MenuModule_menuSize[MenuModule_menuArrayIndex]);

    return status;
}

/*********************************************************************
 * @fn      MenuModule_displayMenu
 *
 * @brief   Displays the menu in the provided space for the menu items.
 *          Starting from the currentItem until the last item that will
 *          fit in the provided space
 *
 * @param   menu - a pointer to the menu
 * @param   currentItem - the current item to display
 * @param   menuSize - the size of the menu
 *
 * @return  None
 */
void MenuModule_displayMenu(const MenuModule_MenuObject_t *menu, uint8 currentItem, uint8 menuSize)
{
    // The first line to print the menu in
    uint8 itemLine = MENU_MODULE_FIRST_ITEM;

    // Print the menu title
    Display_printf(dispHandle, MENU_MODULE_TITLE, 0,
                   MENU_MODULE_COLOR_BOLD MENU_MODULE_COLOR_CYAN "%s" MENU_MODULE_COLOR_RESET, menu->menuTitle);

    // Go over the menu and display all items that enters in the menu space
    while (currentItem < menuSize && itemLine <= MENU_MODULE_LAST_ITEM)
    {
      // Display a specific item
      MenuModule_displayItem(currentItem++, itemLine++);
    };

    // Clear the menu lines that are unused
    if (itemLine <= MENU_MODULE_LAST_ITEM)
    {
      Display_clearLines(dispHandle, itemLine, MENU_MODULE_LAST_ITEM);
    }
}

/*********************************************************************
 * @fn      MenuModule_displayItem
 *
 * @brief   Displays a specific item
 *
 * @param   itemIndex - the index of the item in the menu
 * @param   row - the row to print to
 *
 * @return  None
 */
void MenuModule_displayItem(uint32 itemIndex, uint8 row)
{
  uint8* pPostfix =(void*)"";

  // Add ">" to the item printed in the first menu line
  if (row == MENU_MODULE_FIRST_ITEM)
  {
    // If the current item is the first one, postfix with " >"
    pPostfix = (void*)">";
  }

  // Display the item with the postfix and the help description
  Display_printf(dispHandle, row, 0, "%s %s    %s",
                 MenuModule_menuArray[MenuModule_menuArrayIndex]->menuArray[itemIndex].itemName,
                 pPostfix,
                 MenuModule_menuArray[MenuModule_menuArrayIndex]->menuArray[itemIndex].itemHelp);
}

/*********************************************************************
 * @fn      MenuModule_addSubMenuToArray
 *
 * @brief   Adds a menu to the menu array and its size to the size
 *          array. Since the space is limited, the menu will not
 *          be added if there is no space and FAILURE will be returned
 *
 * @param   menu - the menu to add to the array
 * @param   menuSize - the size of the menu
 *
 * @return  SUCCESS, FAILURE
 */
bStatus_t MenuModule_addSubMenuToArray(const MenuModule_MenuObject_t *menu, uint8 menuSize)
{
    // Add the menu to the array if there is space
    if(MenuModule_menuArrayIndex < MENU_MODULE_MAX_MENUS_DEPT - 1)
    {
        MenuModule_menuArray[++MenuModule_menuArrayIndex] = menu;
        MenuModule_menuSize[MenuModule_menuArrayIndex] = menuSize;
    }
    else
    {
        // No space left in the array for this sub-menu
        return FAILURE;
    }

    return SUCCESS;
}

/*********************************************************************
 * @fn      MenuModule_removeSubMenuFromArray
 *
 * @brief   Removes a menu from the menu array
 *
 * @return  None
 */
void MenuModule_removeSubMenuFromArray(void)
{
    // Don't remove the root menu
    if(MenuModule_menuArrayIndex == 0)
    {
        return;
    }

    // Remove the sub-menu from the array and set the index value to NULL
    MenuModule_menuSize[MenuModule_menuArrayIndex] = 0;
    MenuModule_menuArray[MenuModule_menuArrayIndex--] = NULL;
}

/*********************************************************************
 * @fn      MenuModule_removeAllSubMenusFromArray
 *
 * @brief   Removes all the sub menus from the menu array
 *
 * @return  None
 */
void MenuModule_removeAllSubMenusFromArray(void)
{
    // Add a local variable to make the code MISRA-C compliant.
    uint8 menuArrayIndex = MenuModule_menuArrayIndex;

    // Remove all sub-menus except to the root menu
    while(menuArrayIndex > 0)
    {
        MenuModule_removeSubMenuFromArray();
        menuArrayIndex = MenuModule_menuArrayIndex;
    }
}

/*********************************************************************
 * @fn      MenuModule_vprintf
 *
 * @brief   Calls the Display_vprintf function of the Display module
 *
 * @param   line - line index (0..)
 * @param   column - column index (0..)
 * @param   fmt - format string
 * @param   ... - optional arguments
 *
 * @return  None
 */
void MenuModule_vprintf(uint8_t line, uint8_t column, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);

    Display_vprintf(dispHandle, MenuModule_printStartingRow + line, column, fmt, va);

    va_end(va);
}

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
void MenuModule_doClearLines(uint8_t fromLine, uint8_t toLine)
{
    Display_clearLines(dispHandle, MenuModule_printStartingRow + fromLine, MenuModule_printStartingRow + toLine);
}

/*********************************************************************
 * @fn      MenuModule_initButtons
 *
 * @brief   Initialize the buttons using the buttons module APIs
 *
 * @return  SUCCESS, FAILURE
 */
bStatus_t MenuModule_initButtons(void)
{
    Button_Params params;
    Button_Params_init(&params);

    // Set the buttons parameters
    params.buttonEventMask = Button_EV_CLICKED | Button_EV_LONGPRESSED;
    params.buttonCallback = MenuModule_buttonsCallback;
    params.longPressDuration = MENU_MODULE_LONG_PRESS_DURATION;
    params.debounceDuration = MENU_MODULE_DEBOUNCE_DURATION;

    // Open the buttons
    handle_left = Button_open(CONFIG_BUTTON_0, &params);
    handle_right = Button_open(CONFIG_BUTTON_1, &params);

    if(handle_left == NULL || handle_right == NULL)
    {
        return FAILURE;
    }

    return SUCCESS;
}

/*********************************************************************
 * @fn      MenuModule_buttonsCallback
 *
 * @brief   Buttons callback, it will be called once one of the events
 *          provided to the button module will occur
 *
 * @param   buttonHandle - the button handle
 * @param   buttonEvents - the button event
 *
 * @return  None
 */
void MenuModule_buttonsCallback(Button_Handle buttonHandle, Button_EventMask buttonEvents)
{
    InvokeFromBLEAppUtilContext_t callbackSelection[2][2] =
    {
     {MenuModule_buttonLeft, MenuModule_buttonLeftLongPress},
     {MenuModule_buttonRight, MenuModule_buttonRightLongPress}
    };

    uint8 whichButton = (buttonHandle == handle_left) ? MENU_MODULE_FIRST_INDEX : MENU_MODULE_SECOND_INDEX;
    uint8 whichPress = (buttonEvents == Button_EV_CLICKED) ? MENU_MODULE_FIRST_INDEX : MENU_MODULE_SECOND_INDEX;

    // Switch the context to the BLE App Util task context
    BLEAppUtil_invokeFunctionNoData(callbackSelection[whichButton][whichPress]);
}

/*********************************************************************
 * @fn      MenuModule_buttonLeft
 *
 * @brief   The Left button (button 1) is pressed.
 *          Advence to the next menu item and display the menu starting
 *          from this item
 *
 * @param   pData - is ignored since pData is allways NULL
 *
 * @return  None
 */
void MenuModule_buttonLeft(char *pData)
{
    (MenuModule_currentLocation == MenuModule_menuSize[MenuModule_menuArrayIndex] - 1)?
            // If current item is the last one, move to the first item
            MenuModule_currentLocation = 0:
            // Move to the next item
            MenuModule_currentLocation++;

  // Display the menu starting from the MenuModule_currentLocation
  MenuModule_displayMenu(MenuModule_menuArray[MenuModule_menuArrayIndex], MenuModule_currentLocation,
                         MenuModule_menuSize[MenuModule_menuArrayIndex]);
}

/*********************************************************************
 * @fn      MenuModule_buttonLeftLongPress
 *
 * @brief   The Left button (button 1) is pressed for at least
 *          MENU_MODULE_LONG_PRESS_DURATION.
 *          Go back to the last menu
 *
 * @param   pData - is ignored since pData is allways NULL
 *
 * @return  None
 */
void MenuModule_buttonLeftLongPress(char *pData)
{
    MenuModule_goBack();
}

/*********************************************************************
 * @fn      MenuModule_buttonRight
 *
 * @brief   The Right button (button 2) is pressed.
 *          Call the callback assigned to the current menu item
 *
 * @param   pData - is ignored since pData is allways NULL
 *
 * @return  None
 */
void MenuModule_buttonRight(char *pData)
{
    if(MenuModule_menuArray[MenuModule_menuArrayIndex]->menuArray[MenuModule_currentLocation].itemCallback != NULL)
    {
        MenuModule_menuArray[MenuModule_menuArrayIndex]->menuArray[MenuModule_currentLocation].itemCallback(MenuModule_currentLocation);
    }
}

/*********************************************************************
 * @fn      MenuModule_buttonRightLongPress
 *
 * @brief   The Right button (button 2) is pressed for at least
 *          MENU_MODULE_LONG_PRESS_DURATION.
 *          Go to the main menu
 *
 * @param   pData - is ignored since pData is allways NULL
 *
 * @return  None
 */
void MenuModule_buttonRightLongPress(char *pData)
{
    MenuModule_goToRoot();
}

#endif // #if !defined(Display_DISABLE_ALL)
