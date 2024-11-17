# omp_language
Yet another multiple languages system.  
This language system is designed for open.mp - Most of it could be made compatible with SA-MP, except using in-memory SQLite database (unless if there's a plugin I don't know about), but highly discouraged since that would be very slow.

## Index
* [Features](#features)
* [Semantics](#semantics)
* [How to use](#how-to-use)
* [Should I use YSI?](#should-i-use-ysi)
* [Functions](#functions-and-callbacks)

## Features
* Uses in-memory SQLite database
* Use embedded colour **names** in language files (eg. `{YELLOW}[NOTICE] {WHITE}Hello world!`)
* Languages can be added/removed/modified without restarting the server
* Uses multiple files - No huge single file please
* Works best with YSI, but works without it (biggest drawback: no variable arguments if not using YSI)

[To main index](#index)

## Semantics
`Language directory`  
A directory inside `scriptfiles/languages`, named as `languageCode_languageName`, that holds all language files of that language.

`Language code`  
A two-letter code of a language. See [ISO 639, Set 1](https://en.wikipedia.org/wiki/List_of_ISO_639_language_codes)

`Language name`  
The full name of a language

`Language file`  
A single file inside a language directory, or language sub-directory

`Table`  
Refers to a single language file from runtime perspective.  
Function descriptions refer to `table` because data is fetched from an SQLite database (which uses tables), and not directly from the files.

`Identifier`  
Entry in a table/language file that identifies a single string.

[To main index](#index)

## How to use
* [Including the language system](#including-the-language-system)
* [Compile options](#compile-options)
* [Embedded colour names](#embedded-colour-names)
* [Adding language data](#adding-language-data)
* [Fetching language data](#fetching-language-data)

[To main index](#index)

### Including the language system
1. Make sure you're using open.mp, the provided 3.10.11 compiler and that you have the dependencies listed in `dependencies.txt`
2. Make sure `omp_language.inc` is in your includes directory
3. Optionally, add `omp_language_colours.inc` to your includes directory (see [Embedded colour names](#embedded-colour-names))
4. Make sure `scriptfiles/languages` is added to your server files
5. Include omp_language: `#include <omp_language>`

That's all! 
<!> Make sure at least one language exist: This language system doesn't support having no languages added and may cause issues. Pointless anyway to include this language system if you're not using any language

[To index](#how-to-use)

### Compile options
**Before including omp_language** you can set several compile options:

`SQLITE_INVALID_HANDLE`  
Since open.mp doesn't have a definition for this, it's added by this include.  
Default value if not defined: `(DB:-1)`

`LANGUAGE_MAX_CONTENT_LENGTH`  
Defines the maximum allowed length of language strings.  
This sets array sizes of retrieved language strings and the size of the VARCHAR column in the language tables.  
Default value if not defined: `(1024)`

`LANGUAGES_MAX`  
Defines the maximum amount of languages that can be added.  
Default value if not defined: `(4)`

`LANGUAGE_DEFAULT`  
Defines the default server language.  
<!> Ultimately the default language is stored to a variable. If this given language does not exist, the default language is changed to the first available language! (language at index 0)  
Default value if not defined: `"en"`

`LANGUAGE_SQLITE_PERSISTENT_DATA`  
Define this to create an on-disk database file.  
<!> Only use this to investigate the language database (since there is no sqlite shell)  
<!> **SIGNIFICANTLY** slower than using in-memory database. Do **NOT** use this in production!  

`LANGUAGE_SQLITE_PERSISTENT_FILE`  
If `LANGUAGE_SQLITE_PERSISTENT_DATA` is defined, this definition is the name of the created language database.  
Default value if not defined: `"languages.db"`

`DIALOG_SELECT_LANGUAGE`  
If YSI is not used (or at least if y_dialog and y_inline are not used), the default `ShowPlayerDialog()` and `OnDialogResponse()` are used  (instead of `Dialog_ShowCallBack()` and an inline on dialog response callback).  
Since those require a dialog ID, this sets that dialog ID. Only effective if y_dialog or y_inline is not included.  
Default value if not defined: `(1)`

`LANGUAGE_NO_BUILD_MESSAGE`  
By default, when the language database is built some messages are printed:
```
[Info] ===== Building language database =====
[Info] Found 2 language(s):
[Info]    0: (en) English
[Info]    1: (nl) Nederlands
[Info] > Adding language files to language database
[Info] >> Adding data for language "English"...
[Info] >> Adding data for language "Nederlands"...
[Info] ===== Language database built =====
```
If you don't want that, simply define LANGUAGE_NO_BUILD_MESSAGE  

`LANGUAGE_NO_YSI`  
Define this to prevent inclusion of any YSI include.

`LANGUAGE_NO_YSI_HOOKS`  
Define this to prevent inclusion of y_hooks.

`LANGUAGE_NO_YSI_VA`  
Define this to prevent inclusion of y_va.

`LANGUAGE_NO_YSI_FOREACH`  
Define this to prevent inclusion of y_foreach.

`LANGUAGE_NO_YSI_DIALOG`  
Define this to prevent inclusion of y_dialog and y_inline.  
If not defined, omp_language will attempt to include both y_dialog and y_inline.

[To index](#how-to-use)

### Embedded colour names
The language files are added in run-time and therefor macros (eg `COL_RED`) can't be used.  
Since it would be very inconvenient to use hexadecimal numbers all the time, this language system allows using colour names.  
There is a default set of language colours, included in file `includes/omp_language_colours.inc`. This creates a constant array called `gLanguageColours`.  
You can simply use these colours, or define the array yourself before including omp_language.  
<!> You can use this language system without using this array, though that will throw a compile warning.
If gLanguageColours exists, all colour names are translated to colour numbers when creating the database.

[To index](#how-to-use)

### Adding language data
First of all, to add a language, create a directory named `languageCode_languageName` (eg `en_English`) inside `scriptfiles/languages`.  
Language name is preferably in its own language (so `nl_Nederlands` and `de_Deutsch`, not `nl_Dutch` and `de_German`).  

Inside a language directory, you can create text files (.txt) or directories.  
When calling a language, you can get content from those files using `fileName` or `directoryName-fileName`.  
Take this directory tree:
```
scriptfiles/
├─ languages/
│  ├─ en_English/
│  │  ├─ command/
│  │  │  ├─ admin.txt
│  │  │  ├─ player.txt
│  │  ├─ global.txt
```
To retrieve a string from `global.txt` you would use `"global"`. To retrieve one from `admin.txt` inside the `command` directory, you would use `"command-admin"`.  
<!> Only one sub-directory is supported, creating another directory inside a sub-directory (in this case, `command`) will not work: Those files will not be added.

To add strings to a language file, simply create an identifier, and then the string attached to that identifier. Use a space (or preferably one or more tabs) to seperate those two.  
<!> Empty lines and lines starting with `#` are ignored. As are invalid lines (single words)
For example:
```
## This line is ignored and thus functions as a comment.
# This is the prefered way of adding entries: Identifier fully capitalized, using words and underscores only, identating all strings on the same column (? did I call that right ?)
MY_IDENTIFIER       This is my identifier!
HELLO_WORLD         Hello {GREEN}world!

# However, this is also valid:
_foo BAR
$myIdentifier0 This is my ugly identifier!

# Even this is valid: 
Hello world!
Lorem ipsum dolor sit amet

# This is invalid:
TEST
```

**NOTE**: Make sure that all language directories have the same sub directories and files, and the same identifiers!  
**NOTE 2**: Since version 1.03, any file extension (or none) is valid. But `.txt` is still preferred and recommended.

[To index](#how-to-use)

### Fetching language data
I'll keep this short, since all available functions are described below.  
To fetch data from the database, use `Language_Get()` or `Player_Language_Get()`  
To set/get language of a player, use `Player_SelectLanguage()` or `Player_SetLanguage()` and `Player_GetLanguage()`  
To get amount of available languages and their language codes and names, use `Language_Count()` and `Language_GetIDFromData()`. For example, to print all available languages to the server log:
```c
for (new i = 0, j = Language_Count(); i < j; i++)
{
    new languageCode[3], languageName[32];
    Language_GetDataFromID(i, languageCode, languageName);
    PrintF("> %s(%s)", languageName, languageCode);
}
```

For some more examples, see the example directory in this repository.

[To index](#how-to-use)
---
## Should I use YSI?
Yes.  
1. Even though support for not using YSI is added, it wasn't tested thoroughly and may not be stable
2. Using YSI makes life much easier thanks to variable arguments, which is a standard in open.mp anyway

[To main index](#index)

## Functions
### Functions and callbacks
* [Language_Count()](#language_count)
* [Language_GetDataFromID()](#language_getdatafromid)
* [Language_GetIDFromData()](#language_getidfromdata)
* [Language_Exists()](#language_exists)
* [LanguageDB_Build()](#languagedb_build)
* [Language_Get()](#language_get)
* [Player_GetLanguage()](#player_getlanguage)
* [Player_SetLanguage()](#player_setlanguage)
* [Player_SelectLanguage()](#player_selectlanguage)
* [Player_Language_Get()](#player_language_get)
* [SendClientLanguageMessage()](#sendclientlanguagemessage)
* [SendClientLanguageMessageToAll()](#sendclientlanguagemessagetoall)
* [OnPlayerSelectLanguage()](#onplayerselectlanguage)

[To main index](#index)

---
#### Language_Count
`Language_Count()`  
Returns the amount of available languages

**Parameters**  
None

**Returns**  
(int) Amount of available languages

**Notes**  
None

**Related functions/callbacks**  
* [Language_GetDataFromID()](#language_getdatafromid)
* [Language_GetIDFromData()](#language_getidfromdata)

[To index](#functions-and-callbacks)

---
#### Language_GetDataFromID
`bool:Language_GetDataFromID(const languageId, string:languageCode[], string:languageName[], const lenLanguageCode = sizeof(languageCode), const lenLanguageName = sizeof(languageName))`  
Retrieves language code and language name from given language ID

**Parameters**  
`const languageId`: Language ID to get data from  
`string:languageCode[]`: (reference) Array to store language code in  
`string:languageName[]`: (reference) Array to store language name in  
`const lenLanguageCode`: (optional) Size of array languageCode[]  
`const lenLanguageName`: (optional) Size of array languageName[]

**Returns**  
(bool) **true**: Language exists, data retrieved. **false**: Given language does not exist

**Notes**  
None

**Related functions/callbacks**  
* [Language_Count()](#language_count)
* [Language_GetIDFromData()](#language_getidfromdata)

[To index](#functions-and-callbacks)

---
#### Language_GetIDFromData
`Language_GetIDFromData(const string:search[])`
Gets the language ID by looking up language code or language name.

**Parameters**  
`const string:search[]`: Language to get the ID of, using language code or language name.

**Returns**  
(int) Language ID, or LANGUAGE_INVALID_ID if given language does not exist

**Notes**  
<!> `search[]` must be the full language code or name  
Case insensitive

**Related functions/callbacks**  
* [Language_Count()](#language_count)
* [Language_GetDataFromID()](#language_getdatafromid)
* [Language_Exists()](#language_exists)

[To index](#functions-and-callbacks)

---
#### Language_Exists
`bool:Language_Exists(const string:search[])`  
Checks if given language exists

**Parameters**  
`const string:search[]`: Language to check, using language code or language name.

**Returns**  
(bool) **true**: Language exists. **false**: Language does not exist

**Notes**  
<!> `search[]` must be the full language code or name  
Case insensitive

**Related functions/callbacks**  
* [Language_GetIDFromData()](#language_getidfromdata)

[To index](#functions-and-callbacks)

---
#### LanguageDB_Build
`bool:LanguageDB_Build()`  
Builds or rebuilds the language database.  
Automatically used on script initialization. Can be used to rebuild the language database during runtime (in an admin command), useful if languages were modified.

**Parameters**  
None

**Returns**  
(bool) **true**: Language database created. **false**: Directory "scriptfiles/languages" does not exist

**Notes**  
<!> Returns true if no languages were added (which happens if directory "scriptfiles/languages" is empty or doesn't have valid language directory names)

**Related functions/callbacks**  
None

[To index](#functions-and-callbacks)

---
#### Language_Get
`string:Language_Get(const string:languageCode[], const string:table[], const string:identifier[], OPEN_MP_TAGS:...)`  
Retrieves a language string. Optionally formats retrieved string (only when using YSI)

**Parameters**  
`const string:languageCode[]`: Language to get content from, using the language code  
`const string:table[]`: Table to get the content from  
`const string:identifier[]`: String to retrieve from given language code and table  
`OPEN_MP_TAGS:...`: (optional) Used for formatting the string if format specifiers were used. Only works when using YSI

**Returns**  
(string) Retrieved (and formatted) string

**Notes**  
<!> If given language, table or identifier doesn't exist, it returns `[languageCode]table:identifier` (eg. `[en]global:SOME_IDENTIFIER`)

**Related functions/callbacks**  
* [Player_Language_Get()](#player_language_get)

[To index](#functions-and-callbacks)

---
#### Player_GetLanguage
`string:Player_GetLanguage(const playerid)`  
Returns the language of a player

**Parameters**  
`const playerid`: Player to get language from

**Returns**  
(string) Language code the player is using

**Notes**  
If no language was selected for the player, or if the player doesn't exist, it returns the server default language

**Related functions/callbacks**  
* [Player_SetLanguage()](#player_setlanguage)
* [Player_SelectLanguage()](#player_selectlanguage)

[To index](#functions-and-callbacks)

---
#### Player_SetLanguage
`bool:Player_SetLanguage(const playerid, const string:language[])`  
Sets the language for a player

**Parameters**  
`const playerid`: Player to set the language for  
`const string:language[]`: Language to set, using language code

**Returns**  
(bool) **true**: Language was set for the player. **false**: Language unchanged: Player isn't connected or given language doesn't exist

**Notes**  
None

**Related functions/callbacks**  
* [Player_GetLanguage()](#player_getlanguage)
* [Player_SelectLanguage()](#player_selectlanguage)

[To index](#functions-and-callbacks)

---
#### Player_SelectLanguage
`void:Player_SelectLanguage(const playerid)`  
Shows a dialog to the player displaying all available languages, allowing them to change their language to one of them.

**Parameters**  
`const playerid`: Player to show the dialog to

**Returns**  
Nothing

**Notes**  
If only one language is available, no dialog is displayed and their language is set to that language.  
After changing their language, `OnPlayerSelectLanguage()` is called.

**Related functions/callbacks**  
* [OnPlayerSelectLanguage()](#onplayerselectlanguage)
* [Player_GetLanguage()](#player_getlanguage)
* [Player_SetLanguage()](#player_setlanguage)

[To index](#functions-and-callbacks)

---
#### Player_Language_Get
`string:Player_Language_Get(const playerid, const string:table[], const string:identifier[], OPEN_MP_TAGS:...)`
Retrieves a language string for a player. Optionally formats retrieved string (only when using YSI)

**Parameters**  
`const playerid`: Player to fetch the string for (or rather, fetch a string, using the selected language of given player)  
`const string:table[]`: Table to get the content from  
`const string:identifier[]`: String to retrieve from given language code and table  
`OPEN_MP_TAGS:...`: (optional) Used for formatting the string if format specifiers were used. Only works when using YSI

**Returns**  
(string) Retrieved (and formatted) string

**Notes**  
<!> If the player's language doesn't exist or if given table or identifier doesn't exist, it returns `[languageCode]table:identifier` (eg. `[en]global:SOME_IDENTIFIER`)

**Related functions/callbacks**  
* [Language_Get()](#language_get)
* [Player_GetLanguage()](#player_getlanguage)

[To index](#functions-and-callbacks)

---
#### SendClientLanguageMessage
`bool:SendClientLanguageMessage(const playerid, const colour, const string:table[], const string:identifier[], OPEN_MP_TAGS:...)`  
Sends a client message to the player using the language system

**Parameters**  
`const playerid`: Player to send the message to  
`const colour`: Colour of the message  
`const string:table[]`: Language table to get content from  
`const string:identifier[]`:  String to retrieve from the language table  
`OPEN_MP_TAGS:...`: (optional) Used for formatting the string if format specifiers were used. Only works when using YSI

**Returns**  
(bool) Output of SendClientMessage(), thus **true**: Message was sent or **false**: Player isn't connected

**Notes**  
None

**Related functions/callbacks**  
* [SendClientLanguageMessageToAll()](#sendclientlanguagemessagetoall)

[To index](#functions-and-callbacks)

---
#### SendClientLanguageMessageToAll
`bool:SendClientLanguageMessageToAll(const colour, const string:table[], const string:identifier[], OPEN_MP_TAGS:...)`  
Sends a client message to all players, displaying the message in the selected language of each individual player

**Parameters**  
`const colour`: Colour of the message  
`const string:table[]`: Language table to get content from  
`const string:identifier[]`:  String to retrieve from the language table  
`OPEN_MP_TAGS:...`: (optional) Used for formatting the string if format specifiers were used. Only works when using YSI

**Returns**  
(bool) Output of SendClientMessage(), thus always returns true

**Notes**  
<!> If you're not using YSI, and the to-be-sent text has format specifiers, you can't use this function. Instead you'll have to loop through each player and use `SendClientLanguageMessage()` for each player.

**Related functions/callbacks**  
* [SendClientLanguageMessage()](#sendclientlanguagemessage)

[To index](#functions-and-callbacks)

---
#### OnPlayerSelectLanguage
`OnPlayerSelectLanguage(playerid)`
Callback that is called after a player changed their language from the dialog

**Parameters**  
`playerid`: Player that changed their language

**Notes**  
<!> Only called when using `Player_SelectLanguage()`, not when `Player_SetLanguage()` was used.

**Related functions/callbacks**  
* [Player_SelectLanguage()](#player_selectlanguage)

[To index](#functions-and-callbacks)
---
[To main index](#index)
