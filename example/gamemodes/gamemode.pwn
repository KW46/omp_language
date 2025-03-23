/*
 *                               _                                          
 *   ___  _ __ ___  _ __        | | __ _ _ __   __ _ _   _  __ _  __ _  ___ 
 *  / _ \| '_ ` _ \| '_ \       | |/ _` | '_ \ / _` | | | |/ _` |/ _` |/ _ \
 * | (_) | | | | | | |_) | ___  | | (_| | | | | (_| | |_| | (_| | (_| |  __/
 *  \___/|_| |_| |_| .__/ |___| |_|\__,_|_| |_|\__, |\__,_|\__,_|\__, |\___|
 *                 |_|                         |___/             |___/      
 *  
 !  - omp_language test gamemode created and maintained by KW46 (https://github.com/KW46) (v1.05) 
 !  - This gamemode demonstrates the functionality of the omp_language include.
 */

//==============================================================================
//                       INCLUDES & LIBRARY SETUP
//==============================================================================

#include <open.mp>
#include <sscanf2>
#include <FileManager>

#include <YSI_Coding\y_hooks>
#include <YSI_Coding\y_va>
#include <YSI_Data\y_foreach>
#include <YSI_Visual\y_dialog>

#include <omp_language>

//==============================================================================
//                        CONSTANTS & DEFINITIONS
//==============================================================================

#define MAX_FAILED_LOGINS (3)

//==============================================================================
//                           STATIC VARIABLES
//==============================================================================

static 
    spFailedLogins[MAX_PLAYERS],
    PlayerText:spWelcomeTextDraw[MAX_PLAYERS]
;

//==============================================================================
//                           DIALOG FUNCTIONS
//==============================================================================

static void:ShowLoginDialog(const playerid)
{
    new 
        playerName[MAX_PLAYER_NAME]
    ;

    GetPlayerName(playerid, playerName, MAX_PLAYER_NAME);

    inline OnAttemptLogin(response, listitem, string:inputtext[])
    {
        #pragma unused listitem
        if (response)
        {
            if (!strcmp(inputtext, "1234"))
            {
                spFailedLogins[playerid] = 0;
                SendClientLanguageMessage(playerid, -1, "user-auth", "MESSAGE_LOGIN_OK");
                
                PlayerTextDrawLanguageString(playerid, spWelcomeTextDraw[playerid], "user-auth", "WELCOME_MESSAGE", playerName);
                PlayerTextDrawShow(playerid, spWelcomeTextDraw[playerid]);
            }
            else
            {
                if (++spFailedLogins[playerid] <= MAX_FAILED_LOGINS)
                {
                    SendClientLanguageMessage(playerid, -1, "user-auth", "ERR_WRONG_PASSWORD", spFailedLogins[playerid], MAX_FAILED_LOGINS);
                    ShowLoginDialog(playerid);
                }
                else Kick(playerid);
            }
        }
    }

    Dialog_ShowCallback(
        playerid,
        using inline OnAttemptLogin,
        DIALOG_STYLE_PASSWORD,
        Player_Language_Get(playerid, "user-auth", "DIALOG_LOGIN_TITLE"),
        Player_Language_Get(playerid, "user-auth", "DIALOG_LOGIN_CONTENT", playerName),
        Player_Language_Get(playerid, "user-auth", "DIALOG_BUTTON_LOGIN"),
        Player_Language_Get(playerid, "user-auth", "DIALOG_BUTTON_QUIT")
    );
}

//==============================================================================
//                           PLAYER EVENT HOOKS
//==============================================================================

hook OnPlayerConnect(playerid)
{
    spFailedLogins[playerid] = 0;
    
    spWelcomeTextDraw[playerid] = CreatePlayerTextDraw(playerid, 380.0, 341.15, " ");
    PlayerTextDrawLetterSize(playerid, spWelcomeTextDraw[playerid], 0.58, 2.42);
    PlayerTextDrawAlignment(playerid, spWelcomeTextDraw[playerid], TEXT_DRAW_ALIGN:2);

    PlayerTextDrawColour(playerid, spWelcomeTextDraw[playerid], 0xDDDDDBFF);
    PlayerTextDrawBackgroundColour(playerid, spWelcomeTextDraw[playerid], 0x000000AA);
    PlayerTextDrawBoxColour(playerid, spWelcomeTextDraw[playerid], 0x00000000);

    PlayerTextDrawSetShadow(playerid, spWelcomeTextDraw[playerid], 2);
    PlayerTextDrawSetOutline(playerid, spWelcomeTextDraw[playerid], 0);
    PlayerTextDrawFont(playerid, spWelcomeTextDraw[playerid], TEXT_DRAW_FONT:1);
    PlayerTextDrawSetProportional(playerid, spWelcomeTextDraw[playerid], true);
    PlayerTextDrawUseBox(playerid, spWelcomeTextDraw[playerid], true);
    PlayerTextDrawTextSize(playerid, spWelcomeTextDraw[playerid], 40.0, 460.0);

    new 
        playerName[MAX_PLAYER_NAME]
    ;

    GetPlayerName(playerid, playerName, MAX_PLAYER_NAME);
    SendClientLanguageMessageToAll(-1, "global", "PLAYER_JOIN", playerName, playerid);

    Player_SelectLanguage(playerid);
}

hook OnPlayerDisconnect(playerid, reason)
{
    PlayerTextDrawDestroy(playerid, spWelcomeTextDraw[playerid]);

    return true;
}

forward HideWelcomeTextDraw(playerid);
public HideWelcomeTextDraw(playerid)
{
    if (IsPlayerConnected(playerid))
    {
        PlayerTextDrawHide(playerid, spWelcomeTextDraw[playerid]);
    }
    return true;
}

hook OnPlayerSelectedLanguage(playerid)
{
    new 
        playerName[MAX_PLAYER_NAME]
    ;

    GetPlayerName(playerid, playerName, MAX_PLAYER_NAME);
    PlayerTextDrawLanguageString(playerid, spWelcomeTextDraw[playerid], "user-auth", "WELCOME_MESSAGE", playerName);

    SetTimerEx("HideWelcomeTextDraw", 10000, false, "i", playerid);

    ShowLoginDialog(playerid);
}

//==============================================================================
//                           GAMEMODE EVENT HOOKS
//==============================================================================

new 
    Text:gServerInfoTextDraw
;

hook OnGameModeInit()
{
    gServerInfoTextDraw = TextDrawCreate(320.0, 22.0, " ");
    TextDrawLetterSize(gServerInfoTextDraw, 0.6, 1.8);
    TextDrawAlignment(gServerInfoTextDraw, TEXT_DRAW_ALIGN:2);

    TextDrawColour(gServerInfoTextDraw, 0x906210FF);
    TextDrawBackgroundColour(gServerInfoTextDraw, 0x000000AA);
    TextDrawBoxColour(gServerInfoTextDraw, 0x00000000);

    TextDrawSetShadow(gServerInfoTextDraw, 0);
    TextDrawSetOutline(gServerInfoTextDraw, 1);
    TextDrawFont(gServerInfoTextDraw, TEXT_DRAW_FONT:2);
    TextDrawSetProportional(gServerInfoTextDraw, true);
    TextDrawUseBox(gServerInfoTextDraw, true);
    TextDrawTextSize(gServerInfoTextDraw, 200.0, 620.0);

    return true;
}

forward HideServerInfoTextDraw(playerid);
public HideServerInfoTextDraw(playerid)
{
    if (IsPlayerConnected(playerid))
    {
        TextDrawHideForPlayer(playerid, gServerInfoTextDraw);
    }
    return true;
}

hook OnPlayerSpawn(playerid)
{
    TextDrawShowForPlayer(playerid, gServerInfoTextDraw);

    TextDrawLanguageStringForPlayer(playerid, gServerInfoTextDraw, "global", "SERVER_INFO");

    SetTimerEx("HideServerInfoTextDraw", 10000, false, "i", playerid);

    return true;
}

hook OnGameModeExit()
{
    TextDrawDestroy(gServerInfoTextDraw);

    return true;
}

//==============================================================================
//                              OMP_LANGUAGE
//==============================================================================