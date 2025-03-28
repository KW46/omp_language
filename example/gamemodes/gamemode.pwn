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

/*-- Includes --*/
#include <open.mp>      //Optional : Included by omp_language
#include <sscanf2>      //Optional : Included by omp_language
#include <FileManager>  //Optional : Included by omp_language

#define LANGUAGE_SORT_LANGUAGES
#define LANGUAGE_SORT_BY_NAME
#include <omp_language>

#include <YSI_Coding\y_hooks>
#include <YSI_Visual\y_commands>

/*-- Constants --*/
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
static void:ShowLoginDialog(const playerid){
    new 
        playerName[MAX_PLAYER_NAME]
    ;

    GetPlayerName(playerid, playerName, MAX_PLAYER_NAME);

    inline OnAttemptLogin(response, listitem, string:inputtext[]){
        #pragma unused listitem
        if (response){
            if (!strcmp(inputtext, "1234")){
                spFailedLogins[playerid] = 0;
                SendClientLanguageMessage(playerid, -1, "user-auth", "MESSAGE_LOGIN_OK");
            }
            else{
                if (++spFailedLogins[playerid] <= MAX_FAILED_LOGINS){
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

static void:DestroyDeathLabel(playerid){
    if (spDeathsLabel[playerid] != INVALID_PLAYER_3DTEXT_ID){
        DeletePlayer3DTextLabel(playerid, spDeathsLabel[playerid]);
    }
}

/*-- Callbacks/hooks --*/
hook OnGameModeInit(){
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

hook OnPlayerConnect(playerid){
    new name[MAX_PLAYER_NAME];
    GetPlayerName(playerid, name);

    //Note that omp_language doesn't reset the player's language by default
    //This example gamemode does (see Player_SetLanguage() in OnPlayerDisconnect)
    //Not doing so will result in showing this message to the joining player in the language of the last player with the same playerid (if any)
    SendClientLanguageMessageToAll(-1, "user-global", "JOIN", name, playerid);

    Player_SelectLanguage(playerid);
    return 1;
}

hook OnPlayerDisconnect(playerid, reason){
    new name[MAX_PLAYER_NAME], leaveStr[7];
    format(leaveStr, 7, "LEAVE%d", reason);
    GetPlayerName(playerid, name);

    SendClientLanguageMessageToAll(-1, "user-global", leaveStr, name, playerid);

    PlayerTextDrawDestroy(playerid, spWelcomeTextDraw[playerid]);
    DestroyDeathLabel(playerid);
    Player_SetLanguage(playerid, LANGUAGE_DEFAULT);
    spFailedLogins[playerid] = 0;
    spDeaths[playerid] = 0;
    return 1;
}

hook OnPlayerSpawn(playerid){
    TextDrawShowForPlayer(playerid, sServerInfoTextDraw);
    TextDrawLanguageStringForPlayer(playerid, sServerInfoTextDraw, "global", "INFO_OMP_LANGUAGE");
    SetTimerEx("HideTextDraw", 7000, false, "dd", playerid, sServerInfoTextDraw);
    return 1;
}

hook OnPlayerDeath(playerid, killerid, WEAPON:reason){
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
    spDeathsLabel[playerid] = CreatePlayer3DLanguageTextLabel(playerid, "user-global", "GRAVE_LABEL", 0xFF0000FF, x, y, z, 50.0, INVALID_PLAYER_ID, INVALID_VEHICLE_ID, true, spDeaths[playerid], Player_Language_Get(playerid, "global", countStr));

    GameLanguageTextForPlayer(playerid, "user-global", "DEATH_TEXT", 5000, 3);

    if (killerid != INVALID_PLAYER_ID){
        GameLanguageTextForPlayer(killerid, "user-global", "KILLER_TEXT", 5000, 3);
        SendDeathMessage(killerid, playerid, _:reason);
        SendDeathMessage(playerid, killerid, _:reason);

        SendPlayerLanguageMessageToPlayer(killerid, playerid, "user-global", "SAD_MESSAGE");
    }
    return 1;
}

hook OnPlayerSelectedLanguage(playerid){
    new name[MAX_PLAYER_NAME];
    GetPlayerName(playerid, name);

    spWelcomeTextDraw[playerid] = CreatePlayerLanguageTextDraw(playerid, 380.0, 341.15, "user-auth", "WELCOME_MESSAGE", name);
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
@cmd() killme(const playerid, const string:params[], const help){
    SetPlayerHealth(playerid, 0);
    return 1;
}
