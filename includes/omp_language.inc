//Include version: 1.02

#if defined _INC_omp_language
    #endinput
#endif
#define _INC_omp_language

/*-- Includes --*/
#include <open.mp>
#include <sscanf2>
#include <FileManager>

#if !defined LANGUAGE_NO_YSI
    #if !defined LANGUAGE_NO_YSI_HOOKS
        #tryinclude <YSI_Coding\y_hooks>
    #endif
    #if !defined LANGUAGE_NO_YSI_VA
        #tryinclude <YSI_Coding\y_va>
    #endif
    #if !defined LANGUAGE_NO_YSI_FOREACH
        #tryinclude <YSI_Data\y_foreach>
    #endif
    #if !defined LANGUAGE_NO_YSI_DIALOG
        #tryinclude <YSI_Visual\y_dialog>

        #if defined _INC_y_dialog
            #tryinclude <YSI_Coding\y_inline>
        #endif
    #endif
#endif

#if !defined gLanguageColours
    #tryinclude <omp_language_colours>
#endif
#if !defined gLanguageColours
    #warning gLanguageColours not defined: Colour names used in language files will not work!
#endif

/*-- Macros/Constants --*/
#if !defined SQLITE_INVALID_HANDLE
    #define SQLITE_INVALID_HANDLE (DB:-1)
#endif
#define LANGUAGE_INVALID_ID (-1)

#if !defined LANGUAGE_MAX_CONTENT_LENGTH
    #define LANGUAGE_MAX_CONTENT_LENGTH (1024)
#endif

#if !defined LANGUAGES_MAX
    #define LANGUAGES_MAX (4)
#endif

#if !defined LANGUAGE_DEFAULT
    #define LANGUAGE_DEFAULT "en"
#endif

#if defined LANGUAGE_SQLITE_PERSISTENT_DATA
    #undef LANGUAGE_SQLITE_PERSISTENT_DATA
    #define LANGUAGE_SQLITE_PERSISTENT_DATA (1)

    #if !defined LANGUAGE_SQLITE_PERSISTENT_FILE
        #define LANGUAGE_SQLITE_PERSISTENT_FILE "languages.db"
    #endif
#else
    #define LANGUAGE_SQLITE_PERSISTENT_DATA (0)
#endif

#if !defined _INC_y_dialog || !defined _INC_y_inline
    #if !defined DIALOG_SELECT_LANGUAGE
        #define DIALOG_SELECT_LANGUAGE (1)
    #endif
#endif

#if !defined IsNull
    #define IsNull(%0) ((%0[(%0[0])=='\1'])=='\0'||(%0[0])==ucharmax+1)
#endif

static enum E_LANG_DATA
{
    E_LANG_CODE[3],
    E_LANG_NAME[32]
};

/*-- Variables --*/
static
    DB:sLanguageDBHandle = SQLITE_INVALID_HANDLE,
    sLanguages[LANGUAGES_MAX][E_LANG_DATA],

    sDefaultLanguage[3], //Even though there is LANGUAGE_DEFAULT, still _may_ need this: If language LANGUAGE_DEFAULT doesn't exist, change default language to language id 0 (and store it in sDefaultLanguage)
    spLanguage[MAX_PLAYERS][3]
;

/*-- Functions --*/
//Local functions
static bool:Language_ScanLanguages()
{
    //Scans for languages in `scriptfiles/languages`, adds that language to sLanguages (if dir name is correct: `langCode_langName`)
    //It doesn't check if there are any files within that directory
    //Returns: (bool) true: Language scanned and added | false: Directory "scriptfiles/languages" doesn't exist
    //<!> Function will return true even if no languages exist

    if (!dir_exists("scriptfiles/languages"))
    {
        Print("[ERROR] Language system: Language directory doesn't exist!");
        return false;
    }

    new
        dir:languagesDir,
        dirItem[35],
        dirItemType,
        languagesAdded
    ;
    languagesDir = dir_open("scriptfiles/languages");

    while (dir_list(languagesDir, dirItem, dirItemType))
    {
        if (dirItemType == FM_FILE || !strcmp(dirItem, ".") || !strcmp(dirItem, ".."))
            continue;

        new langCode[3], langName[32];
        if (sscanf(dirItem, "p<_>s[3]s[32]", langCode, langName))
        {
            PrintF("<!> Language system: Invalid directory name '%s' found in 'scriptfiles/languages'!", dirItem);
            continue;
        }
        if (languagesAdded >= LANGUAGES_MAX)
        {
            PrintF("<!> Language system: Cannot add language %s(%s): Maximum languages (%d) reached!", langName, langCode, LANGUAGES_MAX);
            continue; //Print a warning for every language that's not added
        }

        for (new i = 0; i < LANGUAGES_MAX; i++)
        {
            if (IsNull(sLanguages[i][E_LANG_CODE]))
            {
                format(sLanguages[i][E_LANG_CODE], 3, langCode);
                format(sLanguages[i][E_LANG_NAME], 32, langName);
                languagesAdded++;
                break;
            }
        }
    }
    dir_close(languagesDir);
    return true;
}

static void:Language_AddDataFromFile(const languageId, const string:file[], const string:subDir[] = "")
{
    //Creates a table either named `file` or `dir-file` and adds content of that file.
    //Doesn't check if given languageId is valid, so misusing it may (will) cause OOB RTEs.
    //  `languageId`: Array ID of sLanguages
    //  `file[]`: Name of the file, including file extension (.txt)
    //  `subDir[]`: Sub directory within the language directory, if any
    //Returns: Nothing

    new
        fullPath[128],
        tableName[64],
        File:languageFile,
        languageFileContent[LANGUAGE_MAX_CONTENT_LENGTH + 128],
        languageFileLines,
        languageFileCurrentLine
    ;

    format(fullPath, sizeof(fullPath), "languages/%s_%s%s%s/%s", sLanguages[languageId][E_LANG_CODE], sLanguages[languageId][E_LANG_NAME], (IsNull(subDir) ? "" : "/"), subDir, file);
    format(tableName, sizeof(tableName), "%s%s%s", subDir, (IsNull(subDir) ? "" : "-"), file);

    for (new i = strlen(tableName); i > 0; i--){
        if (tableName[i] == '.'){
            strdel(tableName, i, strlen(tableName));
            break;
        }
    }

    DB_FreeResultSet(DB_ExecuteQuery(sLanguageDBHandle, "CREATE TABLE IF NOT EXISTS `%s` (lang TINYINT NOT NULL, ident VARCHAR(64) NOT NULL, content VARCHAR(%d) NOT NULL, PRIMARY KEY(lang, ident));", tableName, LANGUAGE_MAX_CONTENT_LENGTH));

    languageFile = fopen(fullPath, io_read);

    //Get file lines - used when reading and inserting data; must strip last 2 characters of each line (`\n`) but not from the last file line
    while (fread(languageFile, languageFileContent))
        languageFileLines++;
    fseek(languageFile, 0, seek_start);

    while (fread(languageFile, languageFileContent))
    {
        languageFileCurrentLine++;

        //Ignore comments
        if (languageFileContent[0] == '#')
            continue;

        //Seperate data to variables, ignore invalid lines
        new identifier[64], content[LANGUAGE_MAX_CONTENT_LENGTH], sscanfSpecifier[64];
        format(sscanfSpecifier, sizeof(sscanfSpecifier), "?<SSCANF_QUIET=1>s[64]s[%d]", LANGUAGE_MAX_CONTENT_LENGTH);
        if (sscanf(languageFileContent, sscanfSpecifier, identifier, content))
            continue;

        //Strip "\n" (LF) or "\r\n" (CRLF) at end of line
        if (languageFileCurrentLine < languageFileLines)
        {
            new len = strlen(content);

            if (content[len-1] == '\n' && content[len-2] == '\r'){
                if (languageFileCurrentLine < languageFileLines){
                    content[len-2] = EOS;
                }
            }
            else if (content[len-1] == '\n'){
                if (languageFileCurrentLine < languageFileLines){
                    content[len-1] = EOS;
                }
            }                
            else{
                PrintF("<!>[omp_language] Content length of `%s`:`%s` exceeds LANGUAGE_MAX_CONTENT_LENGTH(%d)", tableName, identifier, LANGUAGE_MAX_CONTENT_LENGTH);
            }
        }

        //Replace colour names with real colours, using variable gLanguageColours
        #if defined gLanguageColours
            for (new i = 0; i < sizeof(gLanguageColours); i++)
            {
                new findPos = 1; //Start at index 1: If content would start with a colour name but without brackets, string appears to becomes empty

                while ((findPos = strfind(content, gLanguageColours[i][0], false, findPos)) != -1)
                {
                    new endPos = findPos + strlen(gLanguageColours[i][0]);
                    if (content[findPos-1] == '{' && content[endPos] == '}')
                    {
                        strdel(content, findPos, endPos);
                        strins(content, gLanguageColours[i][1], findPos);
                    }
                    findPos += endPos;
                }
            }
        #endif
        DB_FreeResultSet(DB_ExecuteQuery(sLanguageDBHandle, "INSERT INTO `%s` VALUES (%d, '%s', '%q');", tableName, languageId, identifier, content));
    }
    fclose(languageFile);
}

static bool:LanguageDB_Close()
{
    //Closes connection with the SQLite database
    //Returns: (bool) true: closed | false: not closed (there was no connection)

    if (sLanguageDBHandle == SQLITE_INVALID_HANDLE)
        return false;

    DB_Close(sLanguageDBHandle);
    sLanguageDBHandle = SQLITE_INVALID_HANDLE;
    return true;
}

static DB:LanguageDB_Open()
{
    //Opens connection with the SQLite database. If there already was a connection it is closed first.
    //Deletes language database file before opening
    //Returns: (DB) Database handle ID

    if (sLanguageDBHandle != SQLITE_INVALID_HANDLE)
        LanguageDB_Close();

    #if LANGUAGE_SQLITE_PERSISTENT_DATA
        if (fexist(LANGUAGE_SQLITE_PERSISTENT_FILE))
            fremove(LANGUAGE_SQLITE_PERSISTENT_FILE);
        sLanguageDBHandle = DB_Open(LANGUAGE_SQLITE_PERSISTENT_FILE);
    #else
        sLanguageDBHandle = DB_Open("_language_db", SQLITE_OPEN_MEMORY | SQLITE_OPEN_READWRITE);
    #endif

    return sLanguageDBHandle;
}

//Global functions
Language_Count()
{
    //Returns: (int) Amount of available languages

    new count;
    for (new i = 0; i < LANGUAGES_MAX; i++)
    {
        if (!IsNull(sLanguages[i][E_LANG_CODE]))
        {
            count++;
            continue;
        }
        break;
    }
    return count;
}

bool:Language_GetDataFromID(const languageId, string:languageCode[], string:languageName[], const lenLanguageCode = sizeof(languageCode), const lenLanguageName = sizeof(languageName))
{
    //Gets language code and name from language ID.
    //  `languageId`: Language ID
    //  `string:languageCode[]`: (reference) Array to store language code in
    //  `string:languageName[]`: (reference) Array to store language name in
    //  `lenLanguageCode`: Size of languageCode[]
    //  `lenLanguageName`: Size of languageName[]
    //Returns: (bool) true: Data was found | false: Language doesn't exist (invalid array ID, or language simply doesn't exist)

    if (!(0 <= languageId < LANGUAGES_MAX))
        return false;
    if (IsNull(sLanguages[languageId][E_LANG_CODE]))
        return false;

    format(languageCode, lenLanguageCode, sLanguages[languageId][E_LANG_CODE]);
    format(languageName, lenLanguageName, sLanguages[languageId][E_LANG_NAME]);
    return true;
}

Language_GetIDFromData(const string:search[])
{
    //Gets language ID from language code or language name. Must match exactly except for case (case insensitive)
    //  `search[]`: Language code or name to get the ID from
    //Returns: (int) Language ID or LANGUAGE_INVALID_ID if it doesn't exist

    for (new i = 0; i < LANGUAGES_MAX; i++)
    {
        if (!strcmp(sLanguages[i][E_LANG_CODE], search, true) || !strcmp(sLanguages[i][E_LANG_NAME], search, true))
            return i;
    }
    return LANGUAGE_INVALID_ID;
}

stock bool:Language_Exists(const string:search[])
{
    //Checks if a language exists
    //  `search[]`: Language code or name to check
    //Returns: (bool) true: Language exists | false: Language does not exist

    return Language_GetIDFromData(search) != LANGUAGE_INVALID_ID;
}

bool:LanguageDB_Build()
{
    //(Re-)builds the language database
    //Returns: (bool) true: Succes | false: Fail (Directory "scriptfiles/languages" doesn't exist)
    //<!> Function will return true even if no languages exist

    #if !defined LANGUAGE_NO_BUILD_MESSAGE
        Print("===== Building language database =====");
    #endif
    new wipe_sLanguages[LANGUAGES_MAX][E_LANG_DATA];
    sLanguages = wipe_sLanguages;
    LanguageDB_Open(); //Already closes connection if it exists and wipes data.

    if (!Language_ScanLanguages())
    {
        LanguageDB_Close();
        return false;
    }
    new languageCount = Language_Count();
    #if !defined LANGUAGE_NO_BUILD_MESSAGE
        PrintF("Found %d language(s):", languageCount);
        for (new i = 0; i < languageCount; i++)
        {
            new languageCode[3], languageName[32];
            Language_GetDataFromID(i, languageCode, languageName);
            PrintF("\t%d: (%s) %s", i, languageCode, languageName);
        }

        Print("> Adding language files to language database");
    #endif
    for (new i = 0; i < languageCount; i++)
    {
        #if !defined LANGUAGE_NO_BUILD_MESSAGE
            PrintF(">> Adding data for language \"%s\"...", sLanguages[i][E_LANG_NAME]);
        #endif

        new languageCode[3], languageName[32];
        Language_GetDataFromID(i, languageCode, languageName);

        new
            dir:languageDirRoot, dirRoot[64],
            dirItemRoot[64], dirItemTypeRoot
        ;
        format(dirRoot, sizeof(dirRoot), "scriptfiles/languages/%s_%s", languageCode, languageName);
        languageDirRoot = dir_open(dirRoot);
        while (dir_list(languageDirRoot, dirItemRoot, dirItemTypeRoot))
        {
            if (!strcmp(dirItemRoot, ".") || !strcmp(dirItemRoot, ".."))
                continue;

            if (dirItemTypeRoot == FM_DIR)
            {
                new
                    dir:languageDirSub, dirSub[100],
                    dirItemSub[64], dirItemTypeSub
                ;
                format(dirSub, sizeof(dirSub), "%s/%s", dirRoot, dirItemRoot);
                languageDirSub = dir_open(dirSub);
                while (dir_list(languageDirSub, dirItemSub, dirItemTypeSub))
                {
                    if (dirItemTypeSub == FM_FILE)
                        Language_AddDataFromFile(i, dirItemSub, dirItemRoot);
                }
                dir_close(languageDirSub);
            }
            else Language_AddDataFromFile(i, dirItemRoot);
        }
        dir_close(languageDirRoot);
    }

    #if !defined LANGUAGE_NO_BUILD_MESSAGE
        Print("===== Language database built =====");
    #endif

    return true;
}

#if defined _INC_y_va
    stock string:Language_Get(const string:languageCode[], const string:table[], const string:identifier[], OPEN_MP_TAGS:...)
#else
    stock string:Language_Get(const string:languageCode[], const string:table[], const string:identifier[])
#endif
{
    //Retrieves a string from the language database, formats it (if needed) and returns it
    //  `languageCode[]`: Language to get content from, using language code
    //  `table[]`: Table to get the content from
    //  `identifier[]`: Identifier from given table to retrieve
    //  `OPEN_MP_TAGS:...` (optional) Used for formatting format specifiers used in retrieved content <!>ONLY WHEN USING Y_VA(YSI)
    //Returns: (string) Retrieved, formatted language string
    //<!> If the language, table or identifier doesn't exist, it will return the following string: `[languageCode]table:identifier`, eg. `[en]global:SOME_IDENTIFIER`

    new
        output[LANGUAGE_MAX_CONTENT_LENGTH],
        DBResult:result = DB_ExecuteQuery(sLanguageDBHandle, "SELECT content FROM `%s` WHERE ident = '%s' AND lang = %d;", table, identifier, Language_GetIDFromData(languageCode))
    ;
    if (DB_GetRowCount(result))
    {
        DB_GetFieldString(result, 0, output);

        //Replace "\n" and "\t" (they are otherwise string literals)
        new findPos = 0;
        while ((findPos = strfind(output, "\\n", false, findPos)) != -1)
        {
            strdel(output, findPos, findPos + 2);
            strins(output, "\n", findPos);
        }
        findPos = 0;
        while ((findPos = strfind(output, "\\t", false, findPos)) != -1)
        {
            strdel(output, findPos, findPos + 2);
            strins(output, "\t", findPos);
        }

        #if defined _INC_y_va
            format(output, sizeof(output), output, ___(3));
        #endif
    }
    else
        format(output, sizeof(output), "[%s]%s:%s", languageCode, table, identifier);

    DB_FreeResultSet(result);
    return output;
}

stock string:Player_GetLanguage(const playerid)
{
    //Retrieves the language of a player
    //  `playerid`: Player to get the language from
    //Returns: (string) Language code of player, or the server's default language.

    if (!IsPlayerConnected(playerid))
        return sDefaultLanguage;

    return IsNull(spLanguage[playerid]) ? sDefaultLanguage : spLanguage[playerid];
}

stock bool:Player_SetLanguage(const playerid, const string:language[])
{
    //Sets the language of a player
    //  `playerid`: Player to set the language for
    //  `language[]`: Language to set, using either language code or language name
    //Returns: (bool) true: Language was changed | false: Language wasn't changed: Language doesn't exist

    if (!IsPlayerConnected(playerid))
        return false;

    new languageId = Language_GetIDFromData(language);
    if (languageId == LANGUAGE_INVALID_ID)
        return false;

    new languageCode[3], languageName[1];
    Language_GetDataFromID(languageId, languageCode, languageName);
    format(spLanguage[playerid], sizeof(spLanguage[]), languageCode);
    return true;
}

stock void:Player_SelectLanguage(const playerid)
{
    //Shows dialog with available languages. When selected, change player's language to that language.
    //If only one language is available, language is set to that language.
    //   `playerid`: Player to show the dialog to
    //Returns: Nothing

    if (!IsPlayerConnected(playerid))
        return;

    new languageCount = Language_Count();
    if (languageCount > 1)
    {
        new dialogStr[64 * LANGUAGES_MAX];
        for (new i = 0; i < languageCount; i++)
        {
            new languageCode[3], languageName[32];
            if (Language_GetDataFromID(i, languageCode, languageName))
                format(dialogStr, sizeof(dialogStr), "%s- %s (%s)%s", dialogStr, languageCode, languageName, ((i == languageCount - 1) ? "" : "\n"));
        }

        #if defined _INC_y_dialog && defined _INC_y_inline
            inline OnSelectLanguage(response, listitem, string:inputtext[])
            {
                #pragma unused inputtext
                if (response)
                {
                    new languageCode[3], languageName[32];
                    Language_GetDataFromID(listitem, languageCode, languageName);
                    Player_SetLanguage(playerid, languageCode);
                    SendClientLanguageMessage(playerid, -1, "_omp_language", "MSG_SELECTED_LANGUAGE", languageName);
                    CallLocalFunction("OnPlayerSelectedLanguage", "d", playerid);
                }
            }

            Dialog_ShowCallback(
                playerid,
                using inline OnSelectLanguage,
                DIALOG_STYLE_LIST,
                Player_Language_Get(playerid, "_omp_language", "DIALOG_TITLE"),
                dialogStr,
                Player_Language_Get(playerid, "_omp_language", "DIALOG_BUTTON_SELECT"),
                Player_Language_Get(playerid, "_omp_language", "DIALOG_BUTTON_CANCEL")
            );
        #else
            ShowPlayerDialog(
                playerid,
                DIALOG_SELECT_LANGUAGE,
                DIALOG_STYLE_LIST,
                Player_Language_Get(playerid, "_omp_language", "DIALOG_TITLE"),
                dialogStr,
                Player_Language_Get(playerid, "_omp_language", "DIALOG_BUTTON_SELECT"),
                Player_Language_Get(playerid, "_omp_language", "DIALOG_BUTTON_CANCEL")
            );
        #endif
    }
    else Player_SetLanguage(playerid, sDefaultLanguage);
}

#if defined _INC_y_va
    stock string:Player_Language_Get(const playerid, const string:table[], const string:identifier[], OPEN_MP_TAGS:...)
#else
    stock string:Player_Language_Get(const playerid, const string:table[], const string:identifier[])
#endif
{
    //Retrieves a string from the language database, formats it (if needed) and returns it
    //  `playerid`: Player to retrieve the string for (used to determine what language to fetch)
    //  `table[]`: Table to get the content from
    //  `identifier[]`: Identifier from given table to retrieve
    //  `OPEN_MP_TAGS:...` (optional) Used for formatting format specifiers used in retrieved content <!>ONLY WHEN USING Y_VA(YSI)
    //Returns: (string) Retrieved, formatted language string
    //         If the language, table or identifier doesn't exist, it will return the following string: `[languageCode]table:identifier`, eg. `[en]global:SOME_IDENTIFIER`

    #if defined _INC_y_va
        return Language_Get(Player_GetLanguage(playerid), table, identifier, ___(3));
    #else
        return Language_Get(Player_GetLanguage(playerid), table, identifier);
    #endif
}

#if defined _INC_y_va
    stock bool:SendClientLanguageMessage(const playerid, const colour, const string:table[], const string:identifier[], OPEN_MP_TAGS:...)
#else
    stock bool:SendClientLanguageMessage(const playerid, const colour, const string:table[], const string:identifier[])
#endif
{
    //Sends a client message to a player using the language system
    //  `playerid`: Player to send the message to
    //  `colour`: Colour of the message
    //  `table[]`: Table to get the content from
    //  `identifier[]`: Identifier from given table to retrieve
    //  `OPEN_MP_TAGS:...` (optional) Used for formatting format specifiers used in retrieved content <!>ONLY WHEN USING Y_VA(YSI)
    //Returns: (bool) Output of SendClientMessage (thus true: message sent, false: message not sent, player not connected)    

    #if defined _INC_y_va
        return SendClientMessage(playerid, colour, Player_Language_Get(playerid, table, identifier, ___(4)));
    #else
        return SendClientMessage(playerid, colour, Player_Language_Get(playerid, table, identifier));
    #endif
}

#if defined _INC_y_va
    stock bool:SendClientLanguageMessageToAll(const colour, const string:table[], const string:identifier[], OPEN_MP_TAGS:...)
#else
    stock bool:SendClientLanguageMessageToAll(const colour, const string:table[], const string:identifier[])
#endif
{
    //Sends a client message to all players using the language system
    //  `colour`: Colour of the message
    //  `table[]`: Table to get the content from
    //  `identifier[]`: Identifier from given table to retrieve
    //  `OPEN_MP_TAGS:...` (optional) Used for formatting format specifiers used in retrieved content <!>ONLY WHEN USING Y_VA(YSI)
    //Returns: (bool) Always returns true

    #if defined _INC_y_iterate
        foreach (new i : Player)
    #else
        for (new i = 0; i < MAX_PLAYERS; i++) if (IsPlayerConnected(i))
    #endif
    {
        #if defined _INC_y_va
            SendClientMessage(i, colour, Player_Language_Get(i, table, identifier, ___(3)));
        #else
            SendClientMessage(i, colour, Player_Language_Get(i, table, identifier));
        #endif
    }
    return 1;
}

#if defined _INC_y_va
    stock bool:GameLanguageTextForPlayer(const playerid, const string:table[], const string:identifier[], const time, const style, OPEN_MP_TAGS:...)
#else
    stock bool:GameLanguageTextForPlayer(const playerid, const string:table[], const string:identifier[], const time, const style)
#endif
{
    //Sends a game text to a player using the language system
    //  `playerid`: Player to to show the gametext for
    //  `table[]`: Table to get the content from
    //  `identifier[]`: Identifier from given table to retrieve
    //  `time`: The duration of the text being shown in milliseconds
    //  `style`: The style of text to be displayed
    //  `OPEN_MP_TAGS:...` (optional) Used for formatting format specifiers used in retrieved content <!>ONLY WHEN USING Y_VA(YSI)
    //Returns: (int) output of GameTextForPlayer(), thus 1: Function executed or 0: Player not connected or string (retrieved from table:identifier) is null

    #if defined _INC_y_va
        return GameTextForPlayer(playerid, Player_Language_Get(i, table, identifier), time, style, ___(5));
    #else
        return GameTextForPlayer(playerid, Player_Language_Get(i, table, identifier), time, style);
    #endif
}

#if defined _INC_y_va
    stock void:GameLanguageTextForAll(const string:table[], const string:identifier[], const time, const style, OPEN_MP_TAGS:...)
#else
    stock void:GameLanguageTextForAll(const string:table[], const string:identifier[], const time, const style)
#endif
{
    //Shows a game text to all players using the language system
    //  `table[]`: Table to get the content from
    //  `identifier[]`: Identifier from given table to retrieve
    //  `time`: The duration of the text being shown in milliseconds
    //  `style`: The style of text to be displayed
    //  `OPEN_MP_TAGS:...` (optional) Used for formatting format specifiers used in retrieved content <!>ONLY WHEN USING Y_VA(YSI)
    //Returns: Nothing

    #if defined _INC_y_iterate
        foreach (new i : Player)
    #else
        for (new i = 0; i < MAX_PLAYERS; i++) if (IsPlayerConnected(i))
    #endif
    {
        #if defined _INC_y_va
            GameTextForPlayer(playerid, Player_Language_Get(i, table, identifier), time, style, ___(4));
        #else
            GameTextForPlayer(playerid, Player_Language_Get(i, table, identifier), time, style);
        #endif
    }
}

/*-- Callbacks --*/
forward OnPlayerSelectedLanguage(playerid);

#if defined _INC_y_hooks
    hook OnScriptInit()
#else
    #if defined FITERSCRIPT
        public OnFilterScriptInit()
    #else
        public OnGameModeInit()
    #endif
#endif
{
    LanguageDB_Build();

    if (Language_Count() == 0)
    {
        PrintF("\n<!> <!> <!> <!> <!> <!> <!> No languages have been added!\n");
    }
    else
    {
        if (!Language_Exists(LANGUAGE_DEFAULT))
        {
            PrintF("\n<!> <!> <!> <!> <!> <!> <!> The default language \"%s\" (LANGUAGE_DEFAULT) does not exist! Setting default language to \"%s (%s)\"!", LANGUAGE_DEFAULT, sLanguages[0][E_LANG_NAME], sLanguages[0][E_LANG_CODE]);
            format(sDefaultLanguage, sizeof(sDefaultLanguage), sLanguages[0][E_LANG_CODE]);
        }
        else format(sDefaultLanguage, sizeof(sDefaultLanguage), LANGUAGE_DEFAULT);
    }

    #if !defined _INC_y_hooks
        #if defined FILTERSCRIPT
            #if defined OMPLanguage_OnFilterScriptInit
                return OMPLanguage_OnFilterScriptInit();
            #else
                return 1;
            #endif
        #else
            #if defined OMPLanguage_OnGameModeInit
                return OMPLanguage_OnGameModeInit();
            #else
                return 1;
            #endif        
        #endif
    #endif
    return 1;
}
#if !defined _INC_y_hooks
    #if defined FILTERSCRIPT
        #if defined _ALS_OnFilterScriptInit
            #undef OnFilterScriptInit
        #else
            #define _ALS_OnFilterScriptInit
        #endif
        #define OnFilterScriptInit OMPLanguage_OnFilterScriptInit
        #if defined OMPLanguage_OnFilterScriptInit
            forward OMPLanguage_OnFilterScriptInit();
        #endif
    #else
        #if defined _ALS_OnGameModeInit
            #undef OnGameModeInit
        #else
            #define _ALS_OnGameModeInit
        #endif
        #define OnGameModeInit OMPLanguage_OnGameModeInit
        #if defined OMPLanguage_OnGameModeInit
            forward OMPLanguage_OnGameModeInit();
        #endif
    #endif
#endif

#if !defined _INC_y_dialog || !defined _INC_y_inline
    public OnDialogResponse(playerid, dialogid, response, listitem, inputtext[])
    {
        if (dialogid == DIALOG_SELECT_LANGUAGE)
        {
            if (response)
            {
                new languageCode[3], languageName[32];
                Language_GetDataFromID(listitem, languageCode, languageName);
                Player_SetLanguage(playerid, languageCode);
                #if defined _INC_y_va
                    SendClientLanguageMessage(playerid, -1, "_omp_language", "MSG_SELECTED_LANGUAGE", languageName);
                #else
                    new str[144];
                    format(str, sizeof(str), Player_Language_Get(playerid, "_omp_language", "MSG_SELECTED_LANGUAGE"), languageName);
                    SendClientMessage(playerid, -1, str);
                #endif
                CallLocalFunction("OnPlayerSelectedLanguage", "d", playerid);
            }
        }

        #if !defined _INC_y_dialog || !defined _INC_y_inline
            #if defined OMPLanguage_OnDialogResponse
                return OMPLanguage_OnDialogResponse(playerid, dialogid, response, listitem, inputtext);
            #else
                return 0;
            #endif
        #endif
    }
    #if defined _ALS_OnDialogResponse
        #undef OnDialogResponse
    #else
        #define _ALS_OnDialogResponse
    #endif
    #define OnDialogResponse OMPLanguage_OnDialogResponse
    #if defined OMPLanguage_OnDialogResponse
        forward OMPLanguage_OnDialogResponse(playerid, dialogid, response, listitem, inputtext[]);
    #endif
#endif