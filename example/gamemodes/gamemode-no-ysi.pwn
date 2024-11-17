#pragma dynamic 7000
#pragma option -v2

#include <open.mp>
#include <sscanf2>
#include <FileManager>

#define DIALOG_SELECT_LANGUAGE (2) //<!> If not defined, defaults to (1)
#define LANGUAGE_NO_YSI //Only required if the YSI library is present in includes path
#include <omp_language>

#define MAX_FAILED_LOGINS (3)

enum
{
    DIALOG_LOGIN = 1
};

static spFailedLogins[MAX_PLAYERS];

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

public OnPlayerConnect(playerid)
{
    spFailedLogins[playerid] = 0;

    new playerName[MAX_PLAYER_NAME];
    GetPlayerName(playerid, playerName, MAX_PLAYER_NAME);

    //Yes, you're seeing it correctly, you can't use SendClientLanguageMessageToAll() when using format specifiers in selected string.
    //You could technically but you can't format one string for all players. Again, you can, but then some players may see a message in the wrong language.
    //As seen in example file `gamemode.pwn`, the next 9 lines of code could simply be a simple  `SendClientLanguageMessageToAll(-1, "global", "PLAYER_JOIN", playerName, playerid);` if you're using YSI.
    //Yes, I could make variable arguments without needing YSI, but I won't, cba, if you want that, just use YSI.
    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (IsPlayerConnected(i))
        {
            new str[144];
            format(str, sizeof(str), Player_Language_Get(i, "global", "PLAYER_JOIN"), playerName, playerid);
            SendClientMessage(i, -1, str);
        }
    }
    Player_SelectLanguage(playerid);
    return 1;
}

public OnPlayerSelectedLanguage(playerid)
{
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