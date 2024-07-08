#include <open.mp>
#include <sscanf2>
#include <FileManager>
#include <YSI_Coding\y_hooks>
#include <YSI_Coding\y_va>
#include <YSI_Data\y_foreach>
#include <YSI_Visual\y_dialog>
#include <omp_language>

#define MAX_FAILED_LOGINS (3)

static spFailedLogins[MAX_PLAYERS];

static void:ShowLoginDialog(const playerid)
{
    new playerName[MAX_PLAYER_NAME];
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

hook OnPlayerConnect(playerid)
{
    spFailedLogins[playerid] = 0;

    new playerName[MAX_PLAYER_NAME];
    GetPlayerName(playerid, playerName, MAX_PLAYER_NAME);

    SendClientLanguageMessageToAll(-1, "global", "PLAYER_JOIN", playerName, playerid);
    Player_SelectLanguage(playerid);
}

hook OnPlayerSelectedLanguage(playerid)
{
    ShowLoginDialog(playerid);
}