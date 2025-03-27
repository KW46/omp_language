/*
 *                               _                                          
 *   ___  _ __ ___  _ __        | | __ _ _ __   __ _ _   _  __ _  __ _  ___ 
 *  / _ \| '_ ` _ \| '_ \       | |/ _` | '_ \ / _` | | | |/ _` |/ _` |/ _ \
 * | (_) | | | | | | |_) | ___  | | (_| | | | | (_| | |_| | (_| | (_| |  __/
 *  \___/|_| |_| |_| .__/ |___| |_|\__,_|_| |_|\__, |\__,_|\__,_|\__, |\___|
 *                 |_|                         |___/             |___/      
 *  
 !  - omp_language include created and maintained by @KW46 (Kwarde) (https://github.com/KW46) (v1.05) 
 !  - Contributors:
        @NebulaGB (WookieLeaks) (#17)
        @itsneufox (newfox) (#21)
 */


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

#if defined LANGUAGE_SORT_LANGUAGES
    #undef LANGUAGE_SORT_LANGUAGES

    #if !defined _INC_md_sort
        #tryinclude <md-sort\md-sort>
    #endif
    #if !defined _INC_md_sort
        #tryinclude <md-sort/md-sort>
    #endif
    #if !defined _INC_md_sort
        #tryinclude <md-sort>
    #endif
    #if !defined _INC_md_sort
        #warning Failed to include md-sort.inc : Languages will not be sorted!
        #define LANGUAGE_SORT_LANGUAGES(0)
    #else
        #define LANGUAGE_SORT_LANGUAGES (1)
    #endif
#else
    #define LANGUAGE_SORT_LANGUAGES (0)
#endif

#if LANGUAGE_SORT_LANGUAGES
    #if !defined LANGUAGE_SORT_COUNTRY_NAME && !defined LANGUAGE_SORT_COUNTRY_CODE
        #error LANGUAGE_SORT_LANGUAGES defined, please also define either LANGUAGE_SORT_COUNTRY_NAME or LANGUAGE_SORT_COUNTRY_CODE
    #elseif defined LANGUAGE_SORT_COUNTRY_NAME && defined LANGUAGE_SORT_COUNTRY_CODE
        #error Define either LANGUAGE_SORT_COUNTRY_NAME or LANGUAGE_SORT_COUNTRY_CODE, not both
    #else
        #if defined LANGUAGE_SORT_COUNTRY_NAME
            #define _LANGUAGE_SORT_BY E_LANG_NAME
        #else
            #define _LANGUAGE_SORT_BY E_LANG_CODE
        #endif
    #endif

    #if !defined LANGUAGE_SORT_METHOD
        #define LANGUAGE_SORT_METHOD SORT_ASC
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
        #if LANGUAGE_SORT_LANGUAGES
            languages[LANGUAGES_MAX][E_LANG_DATA],
        #endif
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
            #if LANGUAGE_SORT_LANGUAGES
                if (IsNull(languages[i][E_LANG_CODE]))
                {
                    format(languages[i][E_LANG_CODE], 3, langCode);
                    format(languages[i][E_LANG_NAME], 32, langName);
                    languagesAdded++;
                    break;
                }
            #else
                if (IsNull(sLanguages[i][E_LANG_CODE]))
                {
                    format(sLanguages[i][E_LANG_CODE], 3, langCode);
                    format(sLanguages[i][E_LANG_NAME], 32, langName);
                    languagesAdded++;
                    break;
                }
            #endif
        }
    }
    dir_close(languagesDir);

    #if LANGUAGE_SORT_LANGUAGES
        SortDeepArray(languages, _LANGUAGE_SORT_BY, .order = LANGUAGE_SORT_METHOD);

        for (new i = 0, j = -1; i < LANGUAGES_MAX; i++)
        {
            if (!IsNull(languages[i][E_LANG_CODE]))
            {
                format(sLanguages[++j][E_LANG_CODE], 3, languages[i][E_LANG_CODE]);
                format(sLanguages[j][E_LANG_NAME], 32, languages[i][E_LANG_NAME]);
            }
        }
    #endif
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

    for (new i = strlen(tableName); i > 0; i--)
    {
        if (tableName[i] == '.')
        {
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
        new identifier[64], content[LANGUAGE_MAX_CONTENT_LENGTH+2], sscanfSpecifier[64];
        format(sscanfSpecifier, sizeof(sscanfSpecifier), "?<SSCANF_QUIET=1>s[64]s[%d]", LANGUAGE_MAX_CONTENT_LENGTH+2);
        if (sscanf(languageFileContent, sscanfSpecifier, identifier, content))
            continue;

        if (strlen(content) > LANGUAGE_MAX_CONTENT_LENGTH)
            PrintF("<!>[omp_language] Content length of `%s`:`%s` exceeds LANGUAGE_MAX_CONTENT_LENGTH(%d)", tableName, identifier, LANGUAGE_MAX_CONTENT_LENGTH);

        //Strip "\n" (LF) or "\r\n" (CRLF) at end of line
        if (languageFileCurrentLine < languageFileLines)
        {
            new len = strlen(content);

            if (content[len-1] == '\n' && content[len-2] == '\r')
                content[len-2] = EOS;
            else
                content[len-1] = EOS;
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

    new fetchedData[LANGUAGES_MAX][LANGUAGE_MAX_CONTENT_LENGTH];
    //Note that I didn't use array size 128 (which is limit for text output in open.mp)
    //Some people may have a script that breaks longer messages in multiple messages, so that's why

    for (new i = 0; i < LANGUAGES_MAX; i++)
    {
        if (IsNull(sLanguages[i][E_LANG_NAME]))
            break;

        #if defined _INC_y_va
            format(fetchedData[i], LANGUAGE_MAX_CONTENT_LENGTH, Language_Get(sLanguages[i][E_LANG_CODE], table, identifier, ___(3)));
        #else
            format(fetchedData[i], LANGUAGE_MAX_CONTENT_LENGTH, Language_Get(sLanguages[i][E_LANG_CODE], table, identifier));
        #endif
    }

    #if defined _INC_y_iterate
        foreach (new i : Player)
    #else
        for (new i = 0; i < MAX_PLAYERS; i++) if (IsPlayerConnected(i))
    #endif
            SendClientMessage(i, colour, fetchedData[Language_GetIDFromData(Player_GetLanguage(i))]);
    return true;
}

#if defined _INC_y_va
    stock PlayerTextDrawLanguageString(const playerid, PlayerText:textid, const string:table[], const string:identifier[], OPEN_MP_TAGS:...)
#else
    stock PlayerTextDrawLanguageString(const playerid, PlayerText:textid, const string:table[], const string:identifier[])
#endif
{
    //Sets a player-textdraw string using the language system
    //  `playerid`: Player who owns the textdraw
    //  `textid`: The PlayerTextDraw to set the string for
    //  `table[]`: Table to get the content from
    //  `identifier[]`: Identifier from given table to retrieve
    //  `OPEN_MP_TAGS:...` (optional) Used for formatting format specifiers used in retrieved content <!>ONLY WHEN USING Y_VA(YSI)
    //Returns: Always returns 1
    
    #if defined _INC_y_va
        PlayerTextDrawSetString(playerid, textid, Player_Language_Get(playerid, table, identifier, ___(4)));
    #else
        PlayerTextDrawSetString(playerid, textid, Player_Language_Get(playerid, table, identifier));
    #endif
    return 1;
}

#if defined _INC_y_va
    stock TextDrawLanguageStringForPlayer(const playerid, Text:textid, const string:table[], const string:identifier[], OPEN_MP_TAGS:...)
#else
    stock TextDrawLanguageStringForPlayer(const playerid, Text:textid, const string:table[], const string:identifier[])
#endif
{
    //Sets a textdraw string for a specific player using the language system
    //  `playerid`: Player to show the textdraw to
    //  `textid`: The TextDraw to set the string for
    //  `table[]`: Table to get the content from
    //  `identifier[]`: Identifier from given table to retrieve
    //  `OPEN_MP_TAGS:...` (optional) Used for formatting format specifiers used in retrieved content <!>ONLY WHEN USING Y_VA(YSI)
    //Returns: Always returns 1
    
    #if defined _INC_y_va
        TextDrawSetStringForPlayer(textid, playerid, Player_Language_Get(playerid, table, identifier, ___(4)));
    #else
        TextDrawSetStringForPlayer(textid, playerid, Player_Language_Get(playerid, table, identifier));
    #endif
    return 1;
}

#if defined _INC_y_va
    stock PlayerText:CreatePlayerLanguageTextDraw(const playerid, Float:x, Float:y, const string:table[], const string:identifier[], OPEN_MP_TAGS:...)
#else
    stock PlayerText:CreatePlayerLanguageTextDraw(const playerid, Float:x, Float:y, const string:table[], const string:identifier[])
#endif
{
    //Creates a player-textdraw using the language system
    //  `playerid`: Player to create the textdraw for
    //  `Float:x`: X-Coordinate
    //  `Float:y`: Y-Coordinate
    //  `table[]`: Table to get the content from
    //  `identifier[]`: Identifier from given table to retrieve
    //  `OPEN_MP_TAGS:...` (optional) Used for formatting format specifiers used in retrieved content <!>ONLY WHEN USING Y_VA(YSI)
    //Returns: PlayerText:The ID of the created textdraw
    
    #if defined _INC_y_va
        return CreatePlayerTextDraw(playerid, x, y, Player_Language_Get(playerid, table, identifier, ___(5)));
    #else
        return CreatePlayerTextDraw(playerid, x, y, Player_Language_Get(playerid, table, identifier));
    #endif
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
        return GameTextForPlayer(playerid, Player_Language_Get(playerid, table, identifier), time, style, ___(5));
    #else
        return GameTextForPlayer(playerid, Player_Language_Get(playerid, table, identifier), time, style);
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

    new fetchedData[LANGUAGES_MAX][LANGUAGE_MAX_CONTENT_LENGTH];
    //Default LANGUAGE_MAX_CONTENT_LENGTH may be too much for a gametext.
    //Since I couldn't find limits on gametext lengths (except using colour codes beyond the 255th character may crash a client), just going with LANGUAGE_MAX_CONTENT_LENGTH

    for (new i = 0; i < LANGUAGES_MAX; i++)
    {
        if (IsNull(sLanguages[i][E_LANG_NAME]))
            break;

        #if defined _INC_y_va
            format(fetchedData[i], LANGUAGE_MAX_CONTENT_LENGTH, Language_Get(sLanguages[i][E_LANG_CODE], table, identifier, ___(4)));
        #else
            format(fetchedData[i], LANGUAGE_MAX_CONTENT_LENGTH, Language_Get(sLanguages[i][E_LANG_CODE], table, identifier));
        #endif
    }

    #if defined _INC_y_iterate
        foreach (new i : Player)
    #else
        for (new i = 0; i < MAX_PLAYERS; i++) if (IsPlayerConnected(i))
    #endif
            GameTextForPlayer(i, fetchedData[Language_GetIDFromData(Player_GetLanguage(i))], time, style);
}

#if defined _INC_y_va
    stock bool:SendPlayerLanguageMessageToAll(const senderid, const string:table[], const string:identifier[], OPEN_MP_TAGS:...)
#else
    stock bool:SendPlayerLanguageMessageToAll(const senderid, const string:table[], const string:identifier[])
#endif
{
    //Sends a message in the name of a player to all other players on the server using the language system
    //  `senderid`: The ID of the sender. If invalid, the message will not be sent
    //  `table[]`: Table to get the content from
    //  `identifier[]`: Identifier from given table to retrieve
    //  `OPEN_MP_TAGS:...` (optional) Used for formatting format specifiers used in retrieved content <!>ONLY WHEN USING Y_VA(YSI)
    //Returns: Always returns true

    new fetchedData[LANGUAGES_MAX][LANGUAGE_MAX_CONTENT_LENGTH];

    for (new i = 0; i < LANGUAGES_MAX; i++)
    {
        if (IsNull(sLanguages[i][E_LANG_NAME]))
            break;

        #if defined _INC_y_va
            format(fetchedData[i], LANGUAGE_MAX_CONTENT_LENGTH, Language_Get(sLanguages[i][E_LANG_CODE], table, identifier, ___(3)));
        #else
            format(fetchedData[i], LANGUAGE_MAX_CONTENT_LENGTH, Language_Get(sLanguages[i][E_LANG_CODE], table, identifier));
        #endif
    }

    #if defined _INC_y_iterate
        foreach (new i : Player)
    #else
        for (new i = 0; i < MAX_PLAYERS; i++) if (IsPlayerConnected(i))
    #endif
           SendPlayerMessageToPlayer(i, senderid, fetchedData[Language_GetIDFromData(Player_GetLanguage(i))]);    
    return true;
}

#if defined _INC_y_va
    stock bool:SendPlayerLanguageMessageToPlayer(const playerid, const senderid, const string:table[], const string:identifier[], OPEN_MP_TAGS:...)
#else
    stock bool:SendPlayerLanguageMessageToPlayer(const playerid, const senderid, const string:table[], const string:identifier[])
#endif
{
    //Sends a message in the name of a player to another player on the server using the language system
    //  `playerid`: The ID of the player who will receive the message.
    //  `senderid`: The ID of the sender. If invalid, the message will not be sent
    //  `table[]`: Table to get the content from
    //  `identifier[]`: Identifier from given table to retrieve
    //  `OPEN_MP_TAGS:...` (optional) Used for formatting format specifiers used in retrieved content <!>ONLY WHEN USING Y_VA(YSI)
    //Returns: Output of SendPlayerMessageToPlayer(), thus true: function executed successfully or false: Failed ot execute (specified player does not exist)

    #if defined _INC_y_va
        return SendPlayerMessageToPlayer(playerid, senderid, Player_Language_Get(playerid, table, identifier, ___(4)));
    #else
        return SendPlayerMessageToPlayer(playerid, senderid, Player_Language_Get(playerid, table, identifier));
    #endif
}

//native PlayerText3D:CreatePlayer3DTextLabel(playerid, const text[], colour, Float:x, Float:y, Float:z, Float:drawDistance, parentPlayerid = INVALID_PLAYER_ID, parentVehicleid = INVALID_VEHICLE_ID, bool:testLOS = false, OPEN_MP_TAGS:...);
//native bool:UpdatePlayer3DTextLabelText(playerid, PlayerText3D:textid, colour, const text[], OPEN_MP_TAGS:...);

#if defined _INC_y_va
    stock PlayerText3D:CreatePlayer3DLanguageTextLabel(const playerid, const string:table[], const string:identifier[], const colour, const Float:x, const Float:y, const Float:z, const Float:drawDistance, const parentPlayerid = INVALID_PLAYER_ID, const parentVehicleid = INVALID_VEHICLE_ID, const bool:testLOS = false, OPEN_MP_TAGS:...)
#else
    stock PlayerText3D:CreatePlayer3DLanguageTextLabel(const playerid, const string:table[], const string:identifier[], const colour, const Float:x, const Float:y, const Float:z, const Float:drawDistance, const parentPlayerid = INVALID_PLAYER_ID, const parentVehicleid = INVALID_VEHICLE_ID, const bool:testLOS = false)
#endif
{
    //Creates a 3D text label for a specific player using the language system
    //  `playerid`: The ID of the player to create the 3D text label for
    //  `table[]`: Table to get the content from
    //  `identifier[]`: Identifier from given table to retrieve
    //  `colour`: The text colour
    //  `Float:x`: X coordinate (or offset if attached)
    //  `Float:y`: Y coordinate (or offset if attached)
    //  `Float:z`: Z coordinate (or offest if attached)
    //  `Float:drawDistance`: The distance where you are able to see the 3D Text Label
    //  `parentPlayerid`: The player you want to attach the 3D Text Label to (None: INVALID_PLAYER_ID)
    //  `parentVehicleid`: The vehicle you want to attach the 3D Text Label to (None: INVALID_VEHICLE_ID)
    //  `testLOS`: Test the line of sight (true) so this text can't be seen through walls - or not (false)
    //  `OPEN_MP_TAGS:...` (optional) Used for formatting format specifiers used in retrieved content <!>ONLY WHEN USING Y_VA(YSI)
    //Returns: Output of CreatePlayer3DTextLabel(): ID of the newly created label or INVALID_3DTEXT_ID if the Player 3D Text Label limit (MAX_3DTEXT_PLAYER) was reached
    
    #if defined _INC_y_va
        return CreatePlayer3DTextLabel(playerid, Player_Language_Get(playerid, table, identifier, ___(11)), colour, x, y, z, drawDistance, parentPlayerid, parentVehicleid, testLOS);
    #else
        return CreatePlayer3DTextLabel(playerid, Player_Language_Get(playerid, table, identifier), colour, x, y, z, drawDistance, parentPlayerid, parentVehicleid, testLOS);
    #endif
}

#if defined _INC_y_va
    stock bool:UpdatePlayer3DLanguageTextLabelText(const playerid, const PlayerText3D:textid, const colour, const string:table[], const string:identifier[], OPEN_MP_TAGS:...)
#else
    stock bool:UpdatePlayer3DLanguageTextLabelText(const playerid, const PlayerText3D:textid, const colour, const string:table[], const string:identifier[])
#endif
{
    //Updates a player 3D Text Label's text and colour using the language system
    //  `playerid`: The ID of the player for which the 3D Text Label was created
    //  `textid`: The 3D Text Label you want to update
    //  `colour`: The color the 3D Text Label should have from now on
    //  `table[]`: Table to get the content from
    //  `identifier[]`: Identifier from given table to retrieve
    //  `OPEN_MP_TAGS:...` (optional) Used for formatting format specifiers used in retrieved content <!>ONLY WHEN USING Y_VA(YSI)
    //Returns: Always return true

    #if defined _INC_y_va
        UpdatePlayer3DTextLabelText(playerid, textid, colour, Player_Language_Get(playerid, table, identifier, ___(5)));
    #else
        UpdatePlayer3DTextLabelText(playerid, textid, colour, Player_Language_Get(playerid, table, identifier));
    #endif
    return true;
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
    #else
        return 1;
    #endif
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