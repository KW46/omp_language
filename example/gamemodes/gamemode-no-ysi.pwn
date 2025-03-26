/*
 *                               _                                          
 *   ___  _ __ ___  _ __        | | __ _ _ __   __ _ _   _  __ _  __ _  ___ 
 *  / _ \| '_ ` _ \| '_ \       | |/ _` | '_ \ / _` | | | |/ _` |/ _` |/ _ \
 * | (_) | | | | | | |_) | ___  | | (_| | | | | (_| | |_| | (_| | (_| |  __/
 *  \___/|_| |_| |_| .__/ |___| |_|\__,_|_| |_|\__, |\__,_|\__,_|\__, |\___|
 *                 |_|                         |___/             |___/      
 *  
 !  - omp_language test gamemode created and maintained by KW46 (https://github.com/KW46) (v1.05) 
 !  - This gamemode demonstrates the functionality of the omp_language include without YSI
 */

//==============================================================================
//                       INCLUDES & LIBRARY SETUP
//==============================================================================

#pragma dynamic 7000
#pragma option -v2

#include <open.mp>
#include <sscanf2>
#include <FileManager>

#define DIALOG_SELECT_LANGUAGE (2) //<!> If not defined, defaults to (1)
#define LANGUAGE_NO_YSI //Only required if the YSI library is present in includes path
#include <omp_language>

//==============================================================================
//                        CONSTANTS & DEFINITIONS
//==============================================================================

#define MAX_FAILED_LOGINS (3)

enum
{
    DIALOG_LOGIN = 1
};

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
    new playerName[MAX_PLAYER_NAME];
    GetPlayerName(playerid, playerName, MAX_PLAYER_NAME);

    new dialogStr[128];
    format(dialogStr, sizeof(dialogStr), Player_Language_Get(playerid, "user-auth", "DIALOG_LOGIN_CONTENT"), playerName);
    ShowPlayerDialog(
        playerid,
        DIALOG_LOGIN,
        DIALOG_STYLE_PASSWORD,
        Player_Language_Get(playerid, "user-auth", "DIALOG_LOGIN_TITLE"),
        dialogStr,
        Player_Language_Get(playerid, "user-auth", "DIALOG_BUTTON_LOGIN"),
        Player_Language_Get(playerid, "user-auth", "DIALOG_BUTTON_QUIT")
    );
}

main() return;

//==============================================================================
//                           PLAYER EVENT CALLBACKS
//==============================================================================

public OnPlayerConnect(playerid)
{
    spFailedLogins[playerid] = 0;

    new
        playerName[MAX_PLAYER_NAME],
        textDrawStr[100]
    ;
    GetPlayerName(playerid, playerName, MAX_PLAYER_NAME);

    format(textDrawStr, sizeof(textDrawStr), Player_Language_Get(playerid, "user-auth", "WELCOME_MESSAGE"), playerName);

    spWelcomeTextDraw[playerid] = CreatePlayerTextDraw(playerid, 380.0, 341.15, textDrawStr);
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

    PlayerTextDrawShow(playerid, spWelcomeTextDraw[playerid]);

    //SendClientLanguageMessageToAll(-1, "global", "PLAYER_JOIN", playerName, playerid);
    //This one-liner above must be replaced with the following when not using y_va (which is why I highly recommend using YSI, or at least y_va)
    new welcomeMessage[LANGUAGES_MAX][144];
    for (new i = 0, j = Language_Count(); i < j; i++)
    {
        new languageCode[3], languageName[32];
        Language_GetDataFromID(i, languageCode, languageName);
        format(welcomeMessage[i], sizeof(welcomeMessage[]), Language_Get(languageCode, "global", "PLAYER_JOIN"), playerName, playerid);
    }
    for (new i = 0; i < MAX_PLAYERS; i++)
        if (IsPlayerConnected(i))
            SendClientMessage(i, -1, welcomeMessage[Language_GetIDFromData(Player_GetLanguage(i))]);

    Player_SelectLanguage(playerid);
    return 1;
}

public OnPlayerDisconnect(playerid, reason)
{
    PlayerTextDrawDestroy(playerid, spWelcomeTextDraw[playerid]);
    return 1;
}

public OnPlayerSelectedLanguage(playerid)
{
    SetTimerEx("HideWelcomeTextDraw", 10000, false, "i", playerid);

    ShowLoginDialog(playerid);
}

public OnDialogResponse(playerid, dialogid, response, listitem, inputtext[])
{
    if (dialogid == DIALOG_LOGIN)
    {
        if (response)
        {
            if (!strcmp(inputtext, "1234"))
            {
                spFailedLogins[playerid] = 0;
                SendClientLanguageMessage(playerid, -1, "user-auth", "MESSAGE_LOGIN_OK");
            }
            else
            {
                if (++spFailedLogins[playerid] <= MAX_FAILED_LOGINS)
                {
                    new str[144];
                    format(str, sizeof(str), Player_Language_Get(playerid, "user-auth", "ERR_WRONG_PASSWORD"), spFailedLogins[playerid], MAX_FAILED_LOGINS);
                    SendClientMessage(playerid, -1, str);
                    ShowLoginDialog(playerid);
                }
                else Kick(playerid);
            }
        }
        return 1;
    }
    return 0;
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

//==============================================================================
//                           GAMEMODE EVENT HOOKS
//==============================================================================
new 
    Text:gServerInfoTextDraw
;

public OnGameModeInit()
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

    return 1;
}

forward HideServerInfoTextDraw(playerid);
public HideServerInfoTextDraw(playerid)
{
    if (IsPlayerConnected(playerid))
    {
        TextDrawHideForPlayer(playerid, gServerInfoTextDraw);
    }
    return 1;
}

public OnPlayerSpawn(playerid)
{
    TextDrawShowForPlayer(playerid, gServerInfoTextDraw);

    TextDrawLanguageStringForPlayer(playerid, gServerInfoTextDraw, "global", "SERVER_INFO");

    SetTimerEx("HideServerInfoTextDraw", 10000, false, "i", playerid);

    return 1;
}

public OnGameModeExit()
{
    TextDrawDestroy(gServerInfoTextDraw);
    return 1;
}