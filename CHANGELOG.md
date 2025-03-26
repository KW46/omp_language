# Changelog

## v1.05 - 2025/03/26
* Features
    * Allow sorting languages (see gamemode compile option: LANGUAGE_SORT_LANGUAGES) #22
    * PlayerTextDrawLanguageString(), TextDrawLanguageStringForPlayer(), CreatePlayerLanguageTextDraw() #21 (by @itsneufox)
    * SendPlayerLanguageMessageToAll(), SendPlayerLanguageMessageToPlayer() #18
    * CreatePlayer3DLanguageTextLabel(), UpdatePlayer3DLanguageTextLabelText() #18

* Fixes
    * Unreachable code warning in On[FilterScript|GameMode]Init() when not using YSI #22

* Misc
    * Query db for each language instead of for each player in 'to all' functions #22

## v1.04 - 2025/03/22
* Features
    * N/A

* Fixes
    * Wrong variable name #19 (happened in a silent patch - shame on me!)
    * Tag mismatch warning #17 (by @NebulaGB)

* Misc
    * N/A

## v1.03 - 2025/01/04
* Features
    * Prints a warning if language content length exceeds LANGUAGE_MAX_CONTENT_LENGTH #8
    * GameLanguageTextForPlayer() + GameLanguageTextForAll() #9

* Fixes
    * Possibility of having incorrect table names #7

* Misc
    * N/A

## v1.02 - 2024/11/05
* Features
    * N/A

* Fixes
    * N/A

* Misc
    * Allows excluding YSI explicitly and partially (see gamemode compile options: LANGUAGE_NO_YSI[_\*]) #5

## v1.01 - 2024/08/27
* Features
    * N/A

* Fixes
    * Missing characters when using language files using LF #1

* Misc
    * For convenience, add include version on top of include file #2

## v1.00 - 2024/07/08
Initial release