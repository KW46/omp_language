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

#pragma dynamic 10_000

/*-- Includes --*/
#include <open.mp>      //Optional : Included by omp_language
#include <sscanf2>      //Optional : Included by omp_language
#include <FileManager>  //Optional : Included by omp_language

#define LANGUAGE_NO_YSI
#define LANGUAGE_SORT_LANGUAGES
#define LANGUAGE_SORT_BY_NAME
//#define DIALOG_SELECT_LANGUAGE  (1) //1 is the default dialog ID used by Player_SelectLanguage(). Can be changed
#include <omp_language>

/*-- Constants --*/
#define DIALOG_LOGIN        (2) //2, because DIALOG_SELECT_LANGUAGE is already 1
#define MAX_FAILED_LOGINS   (5)

/*-- Variables --*/
//Global

//Local
static
    Text:sServerInfoTextDraw,
    PlayerText:spWelcomeTextDraw[MAX_PLAYERS],
    spFailedLogins[MAX_PLAYERS],
    spDeaths[MAX_PLAYERS],
    PlayerText3D:spDeathsLabel[MAX_PLAYERS]
;

/*-- Functions -- */
//Global
forward HidePlayerTextDraw(playerid, PlayerText:id);
forward HideTextDraw(playerid, Text:id);

main() return;

public HidePlayerTextDraw(playerid, PlayerText:id){
    if (IsPlayerConnected(playerid)){
        PlayerTextDrawHide(playerid, id);
    }
}

public HideTextDraw(playerid, Text:id){
    if (IsPlayerConnected(playerid)){
        TextDrawHideForPlayer(playerid, id);
    }
}

//Local
static ShowLoginDialog(const playerid){
    new 
        playerName[MAX_PLAYER_NAME]
    ;

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

static DestroyDeathLabel(playerid){
    if (spDeathsLabel[playerid] != INVALID_PLAYER_3DTEXT_ID){
        DeletePlayer3DTextLabel(playerid, spDeathsLabel[playerid]);
    }
}

/*-- Callbacks --*/
public OnGameModeInit(){
    sServerInfoTextDraw = TextDrawCreate(320.0, 22.0, " ");
    TextDrawLetterSize(sServerInfoTextDraw, 0.6, 1.8);
    TextDrawAlignment(sServerInfoTextDraw, TEXT_DRAW_ALIGN:2);

    TextDrawColour(sServerInfoTextDraw, 0x906210FF);
    TextDrawBackgroundColour(sServerInfoTextDraw, 0x000000AA);
    TextDrawBoxColour(sServerInfoTextDraw, 0x00000000);

    TextDrawSetShadow(sServerInfoTextDraw, 0);
    TextDrawSetOutline(sServerInfoTextDraw, 1);
    TextDrawFont(sServerInfoTextDraw, TEXT_DRAW_FONT:2);
    TextDrawSetProportional(sServerInfoTextDraw, true);
    TextDrawUseBox(sServerInfoTextDraw, true);
    TextDrawTextSize(sServerInfoTextDraw, 200.0, 620.0);
    return 1;
}

public OnPlayerConnect(playerid){
    new name[MAX_PLAYER_NAME];
    GetPlayerName(playerid, name);

    //SendClientLanguageMessageToAll(-1, "global", "PLAYER_JOIN", playerName, playerid);
    //This one-liner above must be replaced with the following when not using y_va (which is why I highly recommend using YSI, or at least y_va)
    new welcomeMessage[LANGUAGES_MAX][128];
    for (new i = 0, j = Language_Count(); i < j; i++){
        new languageCode[3], languageName[32];
        Language_GetDataFromID(i, languageCode, languageName);
        format(welcomeMessage[i], sizeof(welcomeMessage[]), Language_Get(languageCode, "user-global", "JOIN"), name, playerid);
    }
    for (new i = 0; i < MAX_PLAYERS; i++){
        if (IsPlayerConnected(i)){
            new languageId = Language_GetIDFromData(Player_GetLanguage(i));
            SendClientMessage(i, -1, welcomeMessage[languageId]);
        }
    }
    //Also note that omp_language doesn't reset the player's language by default
    //This example gamemode does (see Player_SetLanguage() in OnPlayerDisconnect)
    //Not doing so will result in showing this message to the joining player in the language of the last player with the same playerid (if any)

    Player_SelectLanguage(playerid);
    return 1;
}

public OnPlayerDisconnect(playerid, reason){
    new name[MAX_PLAYER_NAME], leaveStr[7];
    format(leaveStr, 7, "LEAVE%d", reason);
    GetPlayerName(playerid, name);

    //Note how the next 15 lines can be a one-liner when using y_va: SendClientLanguageMessageToAll(-1, "user-global", leaveStr, name, playerid);
    new leaveMessage[LANGUAGES_MAX][128];
    for (new i = 0, j = Language_Count(); i < j; i++){
        new languageCode[3], languageName[32];
        Language_GetDataFromID(i, languageCode, languageName);
        format(leaveMessage[i], sizeof(leaveMessage[]), "%s", Language_Get(languageCode, "user-global", leaveStr));
    }
    for (new i = 0; i < MAX_PLAYERS; i++){
        if (IsPlayerConnected(i)){
            new languageId, playerLeaveMessage[128];
            languageId = Language_GetIDFromData(Player_GetLanguage(i));
            
            format(playerLeaveMessage, sizeof(playerLeaveMessage), leaveMessage[languageId], name, playerid);
            SendClientMessage(i, -1, playerLeaveMessage);
        }
    }

    PlayerTextDrawDestroy(playerid, spWelcomeTextDraw[playerid]);
    DestroyDeathLabel(playerid);
    Player_SetLanguage(playerid, LANGUAGE_DEFAULT);
    spFailedLogins[playerid] = 0;
    spDeaths[playerid] = 0;
    return 1;
}

public OnDialogResponse(playerid, dialogid, response, listitem, inputtext[]){
    if (dialogid == DIALOG_LOGIN){
        if (response){
            if (!strcmp(inputtext, "1234")){
                spFailedLogins[playerid] = 0;
                SendClientLanguageMessage(playerid, -1, "user-auth", "MESSAGE_LOGIN_OK");
            }
            else{
                if (++spFailedLogins[playerid] <= MAX_FAILED_LOGINS){
                    new msg[128];
                    format(msg, sizeof(msg), Player_Language_Get(playerid, "user-auth", "ERR_WRONG_PASSWORD"), spFailedLogins[playerid], MAX_FAILED_LOGINS);
                    SendClientMessage(playerid, -1, msg);
                    ShowLoginDialog(playerid);
                }
                else Kick(playerid);
            }
        }
        return 1;
    }
    return 0;
}

public OnPlayerSpawn(playerid){
    TextDrawShowForPlayer(playerid, sServerInfoTextDraw);
    TextDrawLanguageStringForPlayer(playerid, sServerInfoTextDraw, "global", "INFO_OMP_LANGUAGE");
    SetTimerEx("HideTextDraw", 7000, false, "dd", playerid, sServerInfoTextDraw);
    return 1;
}

public OnPlayerDeath(playerid, killerid, WEAPON:reason){
    new Float:x, Float:y, Float:z, countStr[20] = "COUNT_APPEND_OTHER";
    GetPlayerPos(playerid, x, y, z);

    DestroyDeathLabel(playerid);

    new tmpDeaths = ++spDeaths[playerid];
    if (tmpDeaths < 10 || tmpDeaths > 20){
        while (tmpDeaths > 10){
            tmpDeaths -= 10;
        }
        switch (tmpDeaths){
            case 1: countStr = "COUNT_APPEND_FIRST";
            case 2: countStr = "COUNT_APPEND_SECOND";
            case 3: countStr = "COUNT_APPEND_THIRD";
        }
    }

    new labelStr[128];
    format(labelStr, sizeof(labelStr), Player_Language_Get(playerid, "user-global", "GRAVE_LABEL"), spDeaths[playerid], Player_Language_Get(playerid, "global", countStr));
    spDeathsLabel[playerid] = CreatePlayer3DTextLabel(playerid, labelStr, 0xFF0000FF, x, y, z, 50.0, .testLOS = true);

    GameLanguageTextForPlayer(playerid, "user-global", "DEATH_TEXT", 5000, 3);

    if (killerid != INVALID_PLAYER_ID){
        GameLanguageTextForPlayer(killerid, "user-global", "KILLER_TEXT", 5000, 3);
        SendDeathMessage(killerid, playerid, _:reason);
        SendDeathMessage(playerid, killerid, _:reason);

        SendPlayerLanguageMessageToPlayer(killerid, playerid, "user-global", "SAD_MESSAGE");
    }
    return 1;
}

public OnPlayerSelectedLanguage(playerid){
    new name[MAX_PLAYER_NAME], tdStr[64];
    GetPlayerName(playerid, name);

    format(tdStr, sizeof(tdStr), Player_Language_Get(playerid, "user-auth", "WELCOME_MESSAGE"), name);
    spWelcomeTextDraw[playerid] = CreatePlayerTextDraw(playerid, 380.0, 341.15, tdStr);
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
    SetTimerEx("HidePlayerTextDraw", 7000, false, "dd", playerid, spWelcomeTextDraw[playerid]);

    ShowLoginDialog(playerid);
    return 1;
}

/*-- Commands --*/
public OnPlayerCommandText(playerid, cmdtext[]){
    if (!strcmp(cmdtext, "/killme")){
        SetPlayerHealth(playerid, 0);
        return 1;
    }
    return 0;
}