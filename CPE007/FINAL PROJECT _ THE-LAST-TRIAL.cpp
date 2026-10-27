// ============================================================
//                    THE LAST TRIAL
//             SECRETS OF BLACKWOOD ACADEMY
// ============================================================
//  COMBINED VERSION
//    - Main game (story, rooms, guardians, Headmaster, items)
//    - File-stream system (save / load with backup + checks)
//
//  Streams used:
//    cin / cout   console input and output
//    ofstream     writes the save file
//    ifstream     reads the save file
//    istringstream validates each saved line
//
//  Compile with:
//    g++ -std=c++11 -Wall -Wextra -pedantic blackwood.cpp -o blackwood
// ============================================================

#include <iostream>                                                             // cout, cin, getline
#include <fstream>                                                              // ofstream, ifstream (save / load)
#include <sstream>                                                              // istringstream (validate saved lines)
#include <string>                                                               // string type
#include <vector>                                                               // lists of lines (map, art, actions)
#include <array>                                                                // fixed-size arrays (questions)
#include <map>                                                                  // text -> enum lookup tables
#include <algorithm>                                                            // transform
#include <cctype>                                                               // tolower, toupper, isspace
#include <cstdio>                                                               // remove, rename (save file handling)

using namespace std;                                                            // lets the code write cout instead of std::cout


// ============================================================
// ENUMERATIONS
// switch cannot test strings, so text is converted to enums.
// ============================================================

enum Room                                                                       // every location in the academy
{                                                                               // start of enum Room
    ROOM_MAIN_HALL,                                                             // central hub
    ROOM_LIBRARY,                                                               // English guardian
    ROOM_HALL_OF_NUMBERS,                                                       // Mathematics guardian
    ROOM_LABORATORY,                                                            // Science guardian
    ROOM_SERVER_ROOM,                                                           // Computer guardian
    ROOM_SEALED_CHAMBER                                                         // leads to the Headmaster
};                                                                              // end of enum Room

enum Direction                                                                  // directions the player can walk
{                                                                               // start of enum Direction
    DIR_NORTH,                                                                  // enum value DIR_NORTH (a named number)
    DIR_SOUTH,                                                                  // enum value DIR_SOUTH (a named number)
    DIR_EAST,                                                                   // enum value DIR_EAST (a named number)
    DIR_WEST                                                                    // enum value DIR_WEST (a named number)
};                                                                              // end of enum Direction

enum Command                                                                    // commands typed in the game loop
{                                                                               // start of enum Command
    CMD_GO_NORTH,                                                               // enum value CMD_GO_NORTH (a named number)
    CMD_GO_SOUTH,                                                               // enum value CMD_GO_SOUTH (a named number)
    CMD_GO_EAST,                                                                // enum value CMD_GO_EAST (a named number)
    CMD_GO_WEST,                                                                // enum value CMD_GO_WEST (a named number)
    CMD_LOOK,                                                                   // enum value CMD_LOOK (a named number)
    CMD_MAP,                                                                    // enum value CMD_MAP (a named number)
    CMD_TAKE_KEY,                                                               // enum value CMD_TAKE_KEY (a named number)
    CMD_OPEN_CHEST,                                                             // enum value CMD_OPEN_CHEST (a named number)
    CMD_CHALLENGE,                                                              // enum value CMD_CHALLENGE (a named number)
    CMD_INVENTORY,                                                              // enum value CMD_INVENTORY (a named number)
    CMD_STATUS,                                                                 // enum value CMD_STATUS (a named number)
    CMD_HELP,                                                                   // enum value CMD_HELP (a named number)
    CMD_SAVE,                                                                   // enum value CMD_SAVE (a named number)
    CMD_LOAD,                                                                   // enum value CMD_LOAD (a named number)
    CMD_QUIT,                                                                   // enum value CMD_QUIT (a named number)
    CMD_INVALID                                                                 // enum value CMD_INVALID (a named number)
};                                                                              // end of enum Command

// The subject number is also the guardian number (0-3).
enum Subject                                                                    // defines the enum Subject (a list of named values)
{                                                                               // start of enum Subject
    SUBJECT_ENGLISH,                                                            // 0 = The Archivist  (Library)
    SUBJECT_MATHEMATICS,                                                        // 1 = The Numeral    (Hall of Numbers)
    SUBJECT_SCIENCE,                                                            // 2 = Dr. Vex        (Laboratory)
    SUBJECT_COMPUTER,                                                           // 3 = The Glitch     (Server Room)
    SUBJECT_NONE                                                                // room without a guardian
};                                                                              // end of enum Subject

// Values 0-3 are also the index of the item in Player::item[].
enum ItemChoice                                                                 // defines the enum ItemChoice (a list of named values)
{                                                                               // start of enum ItemChoice
    ITEM_TIME_CRYSTAL,                                                          // skips a question
    ITEM_RUNE,                                                                  // removes one wrong choice
    ITEM_COIN,                                                                  // 50/50 (removes two wrong choices)
    ITEM_SCROLL,                                                                // shows a hint
    ITEM_CANCEL,                                                                // use nothing
    ITEM_UNKNOWN                                                                // unrecognised text
};                                                                              // end of enum ItemChoice

enum AnswerResult                                                               // what happened to one question
{                                                                               // start of enum AnswerResult
    ANSWER_CORRECT,                                                             // answered right
    ANSWER_FAILED,                                                              // all attempts used (or HP reached 0)
    ANSWER_SKIPPED,                                                             // Time Crystal used
    ANSWER_ABORT                                                                // input closed (end of file)
};                                                                              // end of enum AnswerResult


// ============================================================
// GAME CONSTANTS
// Change the rules here without touching the rest of the code.
// ============================================================

const int LINE_WIDTH              = 60;                                         // width of every banner / border
const int PLAYER_MAX_HP           = 100;                                        // player starting / maximum HP
const int MAX_ATTEMPTS            = 3;                                          // tries allowed per question
const int WRONG_ANSWER_HP_LOSS    = 20;                                         // HP lost for each wrong answer
const int CORRECT_XP              = 20;                                         // XP gained for a correct answer
const int BOSS_DAMAGE             = 20;                                         // boss HP lost per correct answer
const int GUARDIAN_MAX_HP         = 100;                                        // each guardian (5 hits)
const int HEADMASTER_MAX_HP       = 200;                                        // Headmaster before final question
const int HEADMASTER_FINAL_HP     = 20;                                         // HP left for the final question

const int QUESTIONS_PER_GUARDIAN  = 5;                                          // 4 guardians x 5 = 20
const int HEADMASTER_QUESTIONS    = 9;                                          // 200 HP -> 20 HP = 9 hits
const int FINAL_QUESTIONS         = 1;                                          // the last question
const int TOTAL_QUESTIONS         = 4 * QUESTIONS_PER_GUARDIAN                  // creates TOTAL_QUESTIONS (const int) starting as 4 * QUESTIONS_PER_GUARDIAN (continues below)
                                  + HEADMASTER_QUESTIONS                        // joins on: {HEADMASTER_QUESTIONS}
                                  + FINAL_QUESTIONS;                            // = 30

const string SAVE_FILE        = "blackwood_save.txt";                           // current save
const string TEMP_SAVE_FILE   = "blackwood_save.tmp";                           // written first
const string BACKUP_SAVE_FILE = "blackwood_save.bak";                           // previous save


// ============================================================
// NAME TABLES
// ============================================================

const string ROOM_NAMES[6] =                                                    // creates the table ROOM_NAMES (its values are listed below)
{                                                                               // start of list
    "Main Hall", "Library", "Hall of Numbers",                                  // list values: Main Hall, Library, Hall of Numbers
    "Laboratory", "Server Room", "Sealed Chamber"                               // list values: Laboratory, Server Room, Sealed Chamber
};                                                                              // ends the list of values

const string BOSS_NAMES[4] =                                                    // creates the table BOSS_NAMES (its values are listed below)
{                                                                               // start of list
    "THE ARCHIVIST", "THE NUMERAL", "DR. VEX", "THE GLITCH"                     // list values: THE ARCHIVIST, THE NUMERAL, DR. VEX, THE GLITCH
};                                                                              // ends the list of values

const string SEAL_NAMES[4] =                                                    // creates the table SEAL_NAMES (its values are listed below)
{                                                                               // start of list
    "Seal of Language", "Seal of Logic",                                        // list values: Seal of Language, Seal of Logic
    "Seal of Discovery", "Seal of Technology"                                   // list values: Seal of Discovery, Seal of Technology
};                                                                              // ends the list of values

const string ITEM_NAMES[4] =                                                    // creates the table ITEM_NAMES (its values are listed below)
{                                                                               // start of list
    "Time Crystal", "Rune of Elimination", "Oracle's Coin", "Wisdom Scroll"     // list values: Time Crystal, Rune of Elimination, Oracle's Coin, Wisdom Scroll
};                                                                              // ends the list of values

const string ITEM_EFFECTS[4] =                                                  // creates the table ITEM_EFFECTS (its values are listed below)
{                                                                               // start of list
    "Safely skips one question.",                                               // list values: Safely skips one question.
    "Eliminates one wrong choice.",                                             // list values: Eliminates one wrong choice.
    "50/50: eliminates two wrong choices.",                                     // list values: 50/50: eliminates two wrong choices.
    "Reveals a hint."                                                           // list values: Reveals a hint.
};                                                                              // ends the list of values


// ============================================================
// PLAYER STRUCTURE
// ============================================================

struct Player                                                                   // defines the struct Player (a group of related variables)
{                                                                               // start of struct Player
    string name;                                                                // player's chosen name
    Room   currentRoom;                                                         // room the player is in
    string rank;                                                                // Beginner/Apprentice/Scholar/Master

    int hp;                                                                     // health points (0-100)
    int xp;                                                                     // knowledge experience points
    int correct;                                                                // questions answered correctly
    int incorrect;                                                              // wrong answers given

    bool hasKey;                                                                // carrying the Mysterious Key
    bool libraryKeyTaken;                                                       // key already picked up
    bool chestOpened;                                                           // chest already opened

    bool item[4];                                                               // Time Crystal, Rune, Coin, Scroll
    bool defeated[4];                                                           // Archivist, Numeral, Vex, Glitch
};                                                                              // end of struct Player


// ============================================================
// QUESTION STRUCTURE
// ============================================================

struct Question                                                                 // defines the struct Question (a group of related variables)
{                                                                               // start of struct Question
    string text;                                                                // the question itself
    array<string, 4> options;                                                   // choices A-D (no letter prefix)
    int answer;                                                                 // correct choice: 0=A 1=B 2=C 3=D
    string hint;                                                                // shown by the Wisdom Scroll
};                                                                              // end of struct Question


// ============================================================
// FUNCTION PROTOTYPES
// ============================================================

// ---- Part 1: foundation and introduction ----
void initializePlayer(Player &player);                                          // declares initializePlayer: sets every player value to its starting state
void displayTitle();                                                            // declares displayTitle: shows the title screen
void introduction(Player &player);                                              // declares introduction: asks for the name and shows the story prologue
void displayMechanics();                                                        // declares displayMechanics: shows the how-to-play rules

// ---- Text helpers ----
string toLowerCase(string text);                                                // creates toLowerCase (string) from string text
string normalizeSpaces(string text);                                            // creates normalizeSpaces (string) from string text
string padRight(string text, size_t width);                                     // creates padRight (string) from string text, size_t width
string centerIn(const string &text, int width);                                 // creates centerIn (string) from const string &text, int width
string makeBar(int current, int maximum, int width);                            // creates makeBar (string) from int current, int maximum, int width
string roomName(Room room);                                                     // creates roomName (string) from Room room
string directionName(Direction direction);                                      // creates directionName (string) from Direction direction
string directionLetter(Direction direction);                                    // creates directionLetter (string) from Direction direction
Command    parseCommand(const string &text);                                    // creates parseCommand (Command) from const string &text
ItemChoice parseItem(const string &text);                                       // creates parseItem (ItemChoice) from const string &text
bool readCommandLine(string &out);                                              // creates readCommandLine (bool) from string &out

// ---- Visual helpers ----
void printLine(char symbol);                                                    // declares printLine: prints one full-width line made of a symbol
void printCentered(const string &text);                                         // declares printCentered: prints text centered on the screen
void printBanner(const string &title);                                          // declares printBanner: prints a title between two lines
void printField(const string &label, const string &value);                      // declares printField: prints a label and value in aligned columns
void printArt(const string &art);                                               // declares printArt: prints multi-line ASCII art centered
void showBossFigure(Subject subject);                                           // declares showBossFigure: shows the ASCII art of a guardian
void showHeadmasterFigure();                                                    // declares showHeadmasterFigure: shows the ASCII art of the Headmaster

// ---- Part 2: loop and command parser ----
void gameLoop(Player &player);                                                  // declares gameLoop: main loop: runs rooms and commands, calls the boss fight
bool processCommand(Player &player, string command);                            // creates processCommand (bool) from Player &player, string command
void showChoices(const Player &player);                                         // declares showChoices: shows the HUD, exits and actions before each prompt

// ---- Part 3: map and movement ----
void showMap(const Player &player);                                             // declares showMap: draws the academy map
bool getExit(Room from, Direction direction, Room &destination);                // creates getExit (bool) from Room from, Direction direction, Room &d...
bool isExitLocked(const Player &player, Room from, Direction direction);        // creates isExitLocked (bool) from const Player &player, Room from, Direct...
void movePlayer(Player &player, Direction direction);                           // declares movePlayer: walks the player through an exit if it exists and is unlocked

// ---- Part 4: rooms ----
void showLocation(const Player &player);                                        // declares showLocation: describes the current room
void showGuardianBadge(const Player &player, Subject subject);                  // declares showGuardianBadge: shows whether the room guardian is alive or defeated

// ---- Part 5: key, chest, inventory ----
void takeKey(Player &player);                                                   // declares takeKey: picks up the Mysterious Key in the Library
void openChest(Player &player);                                                 // declares openChest: opens the chest in the Main Hall (needs the key)
void showInventory(const Player &player);                                       // declares showInventory: lists the artifacts the player carries
void showStatus(const Player &player);                                          // declares showStatus: shows HP, XP, rank and progress

// ---- Progress ----
int  countDefeated(const Player &player);                                       // creates countDefeated (int) from const Player &player
int  rankLevelOfXp(int xp);                                                     // creates rankLevelOfXp (int) from int xp
string rankNameOfLevel(int level);                                              // creates rankNameOfLevel (string) from int level
void updateRank(Player &player);                                                // declares updateRank: updates the player rank from the current XP

// ---- Questions and battles ----
const Question &getQuestion(Subject subject, int number);                       // creates getQuestion (const Question) from Subject subject, int number
const Question &getHeadmasterQuestion(int number);                              // creates getHeadmasterQuestion (const Question) from int number
AnswerResult askQuestion(Player &player, const Question &question,              // declares askQuestion: asks one question with attempts, items and damage
                         bool finalQuestion, const string &label);              // continues the parameter list: bool finalQuestion, const string &label
Subject getSubjectForRoom(const Player &player);                                // creates getSubjectForRoom (Subject) from const Player &player
bool guardianBattle(Player &player, Subject subject);                           // creates guardianBattle (bool) from Player &player, Subject subject
void discoverJournal(int page);                                                 // declares discoverJournal: shows one journal page found after a guardian falls
void headmasterBattle(Player &player);                                          // declares headmasterBattle: final boss fight; calls victoryEnding when won
void victoryEnding(const Player &player);                                       // declares victoryEnding: victory story and results screen (shown after the final answer)
void waitForEnter(const string &message);                                       // declares waitForEnter: pauses until Enter so the window does not close
bool gameOver(Player &player);                                                  // creates gameOver (bool) from Player &player

// ---- File streams: save / load ----
bool saveFileExists();                                                          // creates saveFileExists (bool) from
bool saveGame(const Player &player, bool announce = true);                      // creates saveGame (bool) from const Player &player, bool announce = t...
bool parseIntegers(const string &line, int *values, int size);                  // creates parseIntegers (bool) from const string &line, int *values, int si...
bool loadGame(Player &player);                                                  // creates loadGame (bool) from Player &player


// ============================================================
// MAIN
// ============================================================

int main()                                                                      // program starts here; when main returns the console closes
{                                                                               // start of block
    Player player;                                                              // create the player object

    initializePlayer(player);                                                   // set starting values
    displayTitle();                                                             // show the title

    bool loaded = false;                                                        // did the player load a save?

    if (saveFileExists())                                                       // a save file is waiting?
    {                                                                           // start of block
        cout << "\nA saved game was found.\n";                                  // tell the player a save exists
        cout << "Load it now? (y/n) > ";                                        // ask whether to load it

        string answer;                                                          // holds the y/n reply

        if (readCommandLine(answer) && (answer == "y" || answer == "yes"))      // player chose to load
        {                                                                       // start of block
            loaded = loadGame(player);                                          // restore progress
        }                                                                       // end of block
    }                                                                           // end of block

    if (!loaded)                                                                // brand-new game
    {                                                                           // start of block
        introduction(player);                                                   // ask name + prologue
        displayMechanics();                                                     // show how to play
    }                                                                           // end of block
    else                                                                        // otherwise
    {                                                                           // start of block
        cout << "\nType \"help\" at any time to see the instructions.\n";       // remind loaded players about help
    }                                                                           // end of block

    showLocation(player);                                                       // describe the current room
    gameLoop(player);                                                           // start playing

    return 0;                                                                   // program ends here, so the console closes (this caused the bug)
}                                                                               // end of block


// ============================================================
// INITIALIZE PLAYER
// ============================================================

void initializePlayer(Player &player)                                           // function initializePlayer: sets every player value to its starting state
{                                                                               // start of function initializePlayer
    player.name = "";                                                           // sets player.name to the text ""
    player.currentRoom = ROOM_MAIN_HALL;                                        // sets player.currentRoom to ROOM_MAIN_HALL
    player.rank = "Beginner";                                                   // sets player.rank to the text "Beginner"

    player.hp = PLAYER_MAX_HP;                                                  // sets player.hp to PLAYER_MAX_HP
    player.xp = 0;                                                              // sets player.xp to 0
    player.correct = 0;                                                         // sets player.correct to 0
    player.incorrect = 0;                                                       // sets player.incorrect to 0

    player.hasKey = false;                                                      // sets player.hasKey to false
    player.libraryKeyTaken = false;                                             // sets player.libraryKeyTaken to false
    player.chestOpened = false;                                                 // sets player.chestOpened to false

    for (int i = 0; i < 4; i++)                                                 // repeats for i = 0 up to 4 - 1
    {                                                                           // start of for loop
        player.item[i] = false;                                                 // no artifacts yet
        player.defeated[i] = false;                                             // every guardian alive
    }                                                                           // end of for loop
}                                                                               // end of function initializePlayer


// ============================================================
// TITLE
// ============================================================

void displayTitle()                                                             // function displayTitle: shows the title screen
{                                                                               // start of function displayTitle
    cout << "\n";                                                               // prints a blank line
    printLine('=');                                                             // draws a full-width line of '='
    cout << "\n";                                                               // prints a blank line
    printCentered("T H E   L A S T   T R I A L");                               // shows centered: T H E L A S T T R I A L
    printCentered("SECRETS OF BLACKWOOD ACADEMY");                              // shows centered: SECRETS OF BLACKWOOD ACADEMY
    cout << "\n";                                                               // prints a blank line
    printLine('=');                                                             // draws a full-width line of '='
    cout << "\n";                                                               // prints a blank line

    printCentered("Every door hides a question.");                              // shows centered: Every door hides a question.
    printCentered("Every wrong answer has a consequence.");                     // shows centered: Every wrong answer has a consequence.
    cout << "\n";                                                               // prints a blank line

    printLine('=');                                                             // draws a full-width line of '='
}                                                                               // end of function displayTitle


// ============================================================
// INTRODUCTION
// ============================================================

void introduction(Player &player)                                               // function introduction: asks for the name and shows the story prologue
{                                                                               // start of function introduction
    cout << "\nEnter player name: ";                                            // shows: Enter player name:
    getline(cin, player.name);                                                  // calls getline: reads a whole line of input

    while (normalizeSpaces(player.name).empty())                                // keeps repeating while normalizeSpaces(player.name).empty()
    {                                                                           // start of while loop
        if (!cin)                                                               // input closed: use a default
        {                                                                       // start of if block
            player.name = "Student";                                            // sets player.name to the text "Student"
            break;                                                              // leaves the switch/loop
        }                                                                       // end of if block

        cout << "Please enter a valid name: ";                                  // shows: Please enter a valid name:
        getline(cin, player.name);                                              // calls getline: reads a whole line of input
    }                                                                           // end of while loop

    player.name = normalizeSpaces(player.name);                                 // sets player.name to normalizeSpaces(player.name)

    cout << "\n";                                                               // prints a blank line
    printBanner("PROLOGUE");                                                    // shows a banner titled: PROLOGUE

    // NOTE: the next 27 lines are ONE raw text block printed exactly as written, so comments cannot go inside it.
    cout << R"TXT(
Twenty years ago, Blackwood Academy was considered
one of the greatest schools ever built.

Its students were famous for their achievements in
language, mathematics, science, and technology.

But one night, at exactly 12:00 midnight...

Every light inside the academy suddenly went out.

The next morning, Blackwood Academy was empty.

No students.
No teachers.
No Headmaster.

Only one message remained, carved into the entrance:

                 KNOWLEDGE HAS A PRICE.

The academy was closed.

For twenty years, nobody dared to enter.

Until now...
)TXT";   // end of the raw text block started above

    cout << "\n";                                                               // prints a blank line
    printBanner("BLACKWOOD ACADEMY");                                           // shows a banner titled: BLACKWOOD ACADEMY

    // NOTE: the next 30 lines are ONE raw text block printed exactly as written, so comments cannot go inside it.
    cout << R"TXT(
You slowly open your eyes.

The floor beneath you is freezing cold.

You sit up and look around.

Broken classroom doors surround you.
Old books cover the floor.
Dead computers sit behind shattered windows.

You cannot remember how you arrived here.

At the end of the hallway stands a massive door.

You run toward it and pull the handle.

                         LOCKED.

Then suddenly...

                     DING... DONG...

An old school bell echoes through the academy.

A damaged speaker crackles to life.

"Another student has arrived."

)TXT";   // end of the raw text block started above

    cout << "\"Welcome to Blackwood Academy, " << player.name << ".\"\n\n";     // shows: "Welcome to Blackwood Academy, {player.name}."

    // NOTE: the next 16 lines are ONE raw text block printed exactly as written, so comments cannot go inside it.
    cout << R"TXT(Four glowing symbols appear on the wall:

                         [ BOOK ]
                        [ NUMBER ]
                         [ ATOM ]
                       [ COMPUTER ]

"Find the Four Seals of Knowledge."

"Defeat those who guard them."

"And the final door will open."

A skull-shaped crest appears beneath the symbols.

)TXT";   // end of the raw text block started above

    cout << "\"But remember, " << player.name << "...\"\n\n";                   // shows: "But remember, {player.name}..."

    // NOTE: the next 11 lines are ONE raw text block printed exactly as written, so comments cannot go inside it.
    cout << R"TXT("Every wrong answer brings you closer
to your final lesson."

The speaker goes silent.

The lights return.

Ahead of you lies the Main Hall.

Your trial begins now.
)TXT";   // end of the raw text block started above

    printLine('=');                                                             // draws a full-width line of '='
}                                                                               // end of function introduction


// ============================================================
// MECHANICS (also used by the "help" command)
// ============================================================

void displayMechanics()                                                         // function displayMechanics: shows the how-to-play rules
{                                                                               // start of function displayMechanics
    cout << "\n";                                                               // prints a blank line
    printBanner("HOW TO PLAY");                                                 // shows a banner titled: HOW TO PLAY

    cout << "\nGOAL\n";                                                         // shows: GOAL
    printLine('-');                                                             // draws a full-width line of '-'
    cout << "Defeat the 4 guardians, collect the 4 Seals of Knowledge,\n"       // shows: Defeat the 4 guardians, collect the 4 Seals of Knowledge,
         << "then enter the Sealed Chamber and face the Headmaster.\n\n";       // continues the output: then enter the Sealed Chamber and face the Headmaster.

    cout << "MOVEMENT COMMANDS\n";                                              // shows: MOVEMENT COMMANDS
    printLine('-');                                                             // draws a full-width line of '-'
    cout << "go north   go south   go east   go west\n\n";                      // shows: go north go south go east go west

    cout << "OTHER COMMANDS\n";                                                 // shows: OTHER COMMANDS
    printLine('-');                                                             // draws a full-width line of '-'
    cout << "look         Examine the current room\n";                          // shows: look Examine the current room
    cout << "map          Show the academy floor plan\n";                       // shows: map Show the academy floor plan
    cout << "take key     Pick up a key\n";                                     // shows: take key Pick up a key
    cout << "open chest   Open a chest (needs the key)\n";                      // shows: open chest Open a chest (needs the key)
    cout << "challenge    Fight the guardian in this room\n";                   // shows: challenge Fight the guardian in this room
    cout << "inventory    View items and Seals\n";                              // shows: inventory View items and Seals
    cout << "status       View HP, XP, rank and progress\n";                    // shows: status View HP, XP, rank and progress
    cout << "save         Save your game to a file\n";                          // shows: save Save your game to a file
    cout << "load         Load your saved game\n";                              // shows: load Load your saved game
    cout << "Progress is also saved automatically after each command.\n";       // shows: Progress is also saved automatically after each command.
    cout << "help         Show these instructions\n";                           // shows: help Show these instructions
    cout << "quit         Exit the game\n\n";                                   // shows: quit Exit the game
    cout << "Your available exits and actions are shown before\n";              // shows: Your available exits and actions are shown before
    cout << "every prompt, so you always know your choices.\n\n";               // shows: every prompt, so you always know your choices.

    cout << "QUESTIONS\n";                                                      // shows: QUESTIONS
    printLine('-');                                                             // draws a full-width line of '-'
    cout << "Total questions in the game : " << TOTAL_QUESTIONS << "\n";        // shows: Total questions in the game : {TOTAL_QUESTIONS}
    cout << "  Each guardian             : " << QUESTIONS_PER_GUARDIAN          // shows: Each guardian : {QUESTIONS_PER_GUARDIAN}
         << " questions (" << 4 * QUESTIONS_PER_GUARDIAN << " in all)\n";       // continues the output: questions ({4 * QUESTIONS_PER_GUARDIAN} in all)
    cout << "  The Headmaster            : " << HEADMASTER_QUESTIONS            // shows: The Headmaster : {HEADMASTER_QUESTIONS}
         << " questions + " << FINAL_QUESTIONS << " final question\n";          // continues the output: questions + {FINAL_QUESTIONS} final question
    cout << "Every question has choices A, B, C and D.\n";                      // shows: Every question has choices A, B, C and D.
    cout << "A question you fail comes back later in the fight.\n\n";           // shows: A question you fail comes back later in the fight.

    cout << "ATTEMPTS AND DAMAGE\n";                                            // shows: ATTEMPTS AND DAMAGE
    printLine('-');                                                             // draws a full-width line of '-'
    cout << "You have " << MAX_ATTEMPTS << " attempts for each question.\n";    // shows: You have {MAX_ATTEMPTS} attempts for each question.
    cout << "Correct answer : +" << CORRECT_XP << " XP and -" << BOSS_DAMAGE    // shows: Correct answer : +{CORRECT_XP} XP and -{BOSS_DAMAGE}
         << " boss HP\n";                                                       // continues the output: boss HP
    cout << "Wrong answer   : -" << WRONG_ANSWER_HP_LOSS                        // shows: Wrong answer : -{WRONG_ANSWER_HP_LOSS}
         << " HP (that choice is crossed out)\n";                               // continues the output: HP (that choice is crossed out)
    cout << "Type ITEM during a question to use an artifact.\n";                // shows: Type ITEM during a question to use an artifact.
    cout << "You begin with " << PLAYER_MAX_HP << " HP.\n\n";                   // shows: You begin with {PLAYER_MAX_HP} HP.

    cout << "RANKS (kept by your XP, they never go down)\n";                    // shows: RANKS (kept by your XP, they never go down)
    printLine('-');                                                             // draws a full-width line of '-'
    cout << "Beginner    0 - 99 XP\n";                                          // shows: Beginner 0 - 99 XP
    cout << "Apprentice  100 - 199 XP\n";                                       // shows: Apprentice 100 - 199 XP
    cout << "Scholar     200 - 299 XP\n";                                       // shows: Scholar 200 - 299 XP
    cout << "Master      300+ XP\n\n";                                          // shows: Master 300+ XP

    printLine('=');                                                             // draws a full-width line of '='
}                                                                               // end of function displayMechanics


// ============================================================
// STRING HELPERS
// ============================================================

string toLowerCase(string text)                                                 // function toLowerCase: returns the text converted to lower case
{                                                                               // start of function toLowerCase
    transform(text.begin(), text.end(), text.begin(),                           // calls transform: converts every character of the text (arguments continue below)
        [](unsigned char c) { return static_cast<char>(tolower(c)); });         // picks the item at this index

    return text;                                                                // returns text
}                                                                               // end of function toLowerCase

// Trims both ends and collapses repeated spaces.
string normalizeSpaces(string text)                                             // function normalizeSpaces: returns the text trimmed with repeated spaces collapsed
{                                                                               // start of function normalizeSpaces
    string result = "";                                                         // creates result (string) starting as the text ""
    bool previousWasSpace = true;                                               // skips leading spaces

    for (char c : text)                                                         // repeats once for every item (c) in text
    {                                                                           // start of for loop
        if (isspace(static_cast<unsigned char>(c)))                             // checks whether isspace(static_cast<unsigned char>(c))
        {                                                                       // start of if block
            if (!previousWasSpace)                                              // checks whether not previousWasSpace
            {                                                                   // start of if block
                result += ' ';                                                  // adds ' ' to result
                previousWasSpace = true;                                        // sets previousWasSpace to true
            }                                                                   // end of if block
        }                                                                       // end of if block
        else                                                                    // otherwise (none of the conditions above were true)
        {                                                                       // start of else block
            result += c;                                                        // adds c to result
            previousWasSpace = false;                                           // sets previousWasSpace to false
        }                                                                       // end of else block
    }                                                                           // end of for loop

    if (!result.empty() && result.back() == ' ')                                // checks whether not result.empty() and result.back() == ' '
    {                                                                           // start of if block
        result.pop_back();                                                      // calls result.pop_back()
    }                                                                           // end of if block

    return result;                                                              // returns result
}                                                                               // end of function normalizeSpaces

// Pads text with spaces on the right so columns line up.
string padRight(string text, size_t width)                                      // function padRight: returns the text padded with spaces (or cut) to a fixed width
{                                                                               // start of function padRight
    if (text.size() < width)                                                    // checks whether text.size() < width
    {                                                                           // start of if block
        text += string(width - text.size(), ' ');                               // adds string(width - text.size(), ' ') to text
    }                                                                           // end of if block

    return text;                                                                // returns text
}                                                                               // end of function padRight

// Centers text inside a field of the given width.
string centerIn(const string &text, int width)                                  // function centerIn: returns the text centered inside a given width
{                                                                               // start of function centerIn
    int length = static_cast<int>(text.size());                                 // creates length (int) starting as static_cast<int>(text.size())

    if (length >= width)                                                        // checks whether length >= width
    {                                                                           // start of if block
        return text.substr(0, static_cast<size_t>(width));                      // returns text.substr(0, static_cast<size_t>(width))
    }                                                                           // end of if block

    int left = (width - length) / 2;                                            // creates left (int) starting as (width - length) / 2
    int right = width - length - left;                                          // creates right (int) starting as width - length - left

    return string(static_cast<size_t>(left), ' ') + text                        // returns string(static_cast<size_t>(left), ' ') + text (continues below)
         + string(static_cast<size_t>(right), ' ');                             // joins on: {string(static_cast<size_t>(right), ' ')}
}                                                                               // end of function centerIn

// Builds a bar such as [#########---------].
string makeBar(int current, int maximum, int width)                             // function makeBar: builds a text progress bar such as [####----]
{                                                                               // start of function makeBar
    if (current < 0) current = 0;                                               // checks whether current < 0; if so: current = 0
    if (current > maximum) current = maximum;                                   // checks whether current > maximum; if so: current = maximum

    int filled = (maximum > 0) ? (current * width) / maximum : 0;               // creates filled (int) starting as (maximum > 0) ? (current * width) / maximum : 0

    return "[" + string(static_cast<size_t>(filled), '#')                       // returns "[" + string(static_cast<size_t>(filled), '#') (continues below)
              + string(static_cast<size_t>(width - filled), '-') + "]";         // joins on: {string(static_cast<size_t>(width - filled), '-')}]
}                                                                               // end of function makeBar

string roomName(Room room)                                                      // function roomName: returns the display name of a room
{                                                                               // start of function roomName
    return ROOM_NAMES[room];                                                    // returns ROOM_NAMES[room]
}                                                                               // end of function roomName

string directionName(Direction direction)                                       // function directionName: returns the direction as a word (north, south...)
{                                                                               // start of function directionName
    switch (direction)                                                          // chooses what to do based on direction
    {                                                                           // start of switch
        case DIR_NORTH: return "north";                                         // when the value is DIR_NORTH: returns the text "north"
        case DIR_SOUTH: return "south";                                         // when the value is DIR_SOUTH: returns the text "south"
        case DIR_EAST:  return "east";                                          // when the value is DIR_EAST: returns the text "east"
        case DIR_WEST:  return "west";                                          // when the value is DIR_WEST: returns the text "west"
    }                                                                           // end of switch

    return "";                                                                  // returns the text ""
}                                                                               // end of function directionName

string directionLetter(Direction direction)                                     // function directionLetter: returns the direction as a letter (N, S, E, W)
{                                                                               // start of function directionLetter
    switch (direction)                                                          // chooses what to do based on direction
    {                                                                           // start of switch
        case DIR_NORTH: return "N";                                             // when the value is DIR_NORTH: returns the text "N"
        case DIR_SOUTH: return "S";                                             // when the value is DIR_SOUTH: returns the text "S"
        case DIR_EAST:  return "E";                                             // when the value is DIR_EAST: returns the text "E"
        case DIR_WEST:  return "W";                                             // when the value is DIR_WEST: returns the text "W"
    }                                                                           // end of switch

    return "";                                                                  // returns the text ""
}                                                                               // end of function directionLetter

// Reads one line and cleans it. Returns false if input was closed.
bool readCommandLine(string &out)                                               // function readCommandLine: reads one line and cleans it; false when input is closed
{                                                                               // start of function readCommandLine
    if (!getline(cin, out))                                                     // checks whether not getline(cin, out)
    {                                                                           // start of if block
        return false;                                                           // returns false
    }                                                                           // end of if block

    out = toLowerCase(normalizeSpaces(out));                                    // sets out to toLowerCase(normalizeSpaces(out))
    return true;                                                                // returns true
}                                                                               // end of function readCommandLine

Command parseCommand(const string &text)                                        // function parseCommand: turns typed text into a Command value
{                                                                               // start of function parseCommand
    static const map<string, Command> table =                                   // creates the table table (its values are listed below)
    {                                                                           // start of list
        { "go north",   CMD_GO_NORTH   },                                       // list values: go north
        { "go south",   CMD_GO_SOUTH   },                                       // list values: go south
        { "go east",    CMD_GO_EAST    },                                       // list values: go east
        { "go west",    CMD_GO_WEST    },                                       // list values: go west
        { "look",       CMD_LOOK       },                                       // list values: look
        { "map",        CMD_MAP        },                                       // list values: map
        { "take key",   CMD_TAKE_KEY   },                                       // list values: take key
        { "open chest", CMD_OPEN_CHEST },                                       // list values: open chest
        { "challenge",  CMD_CHALLENGE  },                                       // list values: challenge
        { "inventory",  CMD_INVENTORY  },                                       // list values: inventory
        { "status",     CMD_STATUS     },                                       // list values: status
        { "help",       CMD_HELP       },                                       // list values: help
        { "save",       CMD_SAVE       },                                       // list values: save
        { "load",       CMD_LOAD       },                                       // list values: load
        { "quit",       CMD_QUIT       }                                        // list values: quit
    };                                                                          // ends the list of values

    map<string, Command>::const_iterator it = table.find(text);                 // creates it (map<string, Command>::const_iterator) starting as table.find(text)

    if (it == table.end())                                                      // checks whether it == table.end()
    {                                                                           // start of if block
        return CMD_INVALID;                                                     // returns CMD_INVALID
    }                                                                           // end of if block

    return it->second;                                                          // returns it->second
}                                                                               // end of function parseCommand

ItemChoice parseItem(const string &text)                                        // function parseItem: turns typed text into an ItemChoice value
{                                                                               // start of function parseItem
    static const map<string, ItemChoice> table =                                // creates the table table (its values are listed below)
    {                                                                           // start of list
        { "time crystal", ITEM_TIME_CRYSTAL },                                  // list values: time crystal
        { "rune",         ITEM_RUNE         },                                  // list values: rune
        { "coin",         ITEM_COIN         },                                  // list values: coin
        { "scroll",       ITEM_SCROLL       },                                  // list values: scroll
        { "cancel",       ITEM_CANCEL       }                                   // list values: cancel
    };                                                                          // ends the list of values

    map<string, ItemChoice>::const_iterator it = table.find(text);              // creates it (map<string, ItemChoice>::const_iterator) starting as table.find(text)

    if (it == table.end())                                                      // checks whether it == table.end()
    {                                                                           // start of if block
        return ITEM_UNKNOWN;                                                    // returns ITEM_UNKNOWN
    }                                                                           // end of if block

    return it->second;                                                          // returns it->second
}                                                                               // end of function parseItem


// ============================================================
// VISUAL HELPERS
// ============================================================

void printLine(char symbol)                                                     // function printLine: prints one full-width line made of a symbol
{                                                                               // start of function printLine
    cout << string(static_cast<size_t>(LINE_WIDTH), symbol) << "\n";            // shows: {string(static_cast<size_t>(LINE_WIDTH), symbol)}
}                                                                               // end of function printLine

void printCentered(const string &text)                                          // function printCentered: prints text centered on the screen
{                                                                               // start of function printCentered
    int padding = (LINE_WIDTH - static_cast<int>(text.size())) / 2;             // creates padding (int) starting as (LINE_WIDTH - static_cast<int>(text.size())) / 2

    if (padding < 0)                                                            // checks whether padding < 0
    {                                                                           // start of if block
        padding = 0;                                                            // sets padding to 0
    }                                                                           // end of if block

    cout << string(static_cast<size_t>(padding), ' ') << text << "\n";          // shows: {string(static_cast<size_t>(padding), ' ')}{text}
}                                                                               // end of function printCentered

void printBanner(const string &title)                                           // function printBanner: prints a title between two lines
{                                                                               // start of function printBanner
    printLine('=');                                                             // draws a full-width line of '='
    printCentered(title);                                                       // shows centered: {title}
    printLine('=');                                                             // draws a full-width line of '='
}                                                                               // end of function printBanner

// Prints "Label            : value" with the colon always in column 21.
void printField(const string &label, const string &value)                       // function printField: prints a label and value in aligned columns
{                                                                               // start of function printField
    cout << padRight(label, 20) << ": " << value << "\n";                       // shows: {padRight(label, 20)}: {value}
}                                                                               // end of function printField

// Prints multi-line ASCII art as one block centered on the screen.
void printArt(const string &art)                                                // function printArt: prints multi-line ASCII art centered
{                                                                               // start of function printArt
    vector<string> lines;                                                       // creates the variable lines (vector<string>)
    istringstream input(art);                                                   // creates the file stream input for art
    string line;                                                                // creates the variable line (string)
    size_t widest = 0;                                                          // creates widest (size_t) starting as 0

    while (getline(input, line))                                                // keeps repeating while getline(input, line)
    {                                                                           // start of while loop
        lines.push_back(line);                                                  // calls push_back: adds an item to the end of the list

        if (line.size() > widest)                                               // checks whether line.size() > widest
        {                                                                       // start of if block
            widest = line.size();                                               // sets widest to line.size()
        }                                                                       // end of if block
    }                                                                           // end of while loop

    int padding = (LINE_WIDTH - static_cast<int>(widest)) / 2;                  // creates padding (int) starting as (LINE_WIDTH - static_cast<int>(widest)) / 2

    if (padding < 0)                                                            // checks whether padding < 0
    {                                                                           // start of if block
        padding = 0;                                                            // sets padding to 0
    }                                                                           // end of if block

    for (const string &row : lines)                                             // repeats once for every item (row) in lines
    {                                                                           // start of for loop
        cout << string(static_cast<size_t>(padding), ' ') << row << "\n";       // shows: {string(static_cast<size_t>(padding), ' ')}{row}
    }                                                                           // end of for loop
}                                                                               // end of function printArt


// ============================================================
// BOSS FIGURES (ASCII art)
// ============================================================

void showBossFigure(Subject subject)                                            // function showBossFigure: shows the ASCII art of a guardian
{                                                                               // start of function showBossFigure
    switch (subject)                                                            // chooses what to do based on subject
    {                                                                           // start of switch
        case SUBJECT_ENGLISH:                                                   // THE ARCHIVIST
            // NOTE: the next 12 lines are ONE raw text block printed exactly as written, so comments cannot go inside it.
            printArt(R"ART(          .-""""""-.
         /  _    _  \
        |  (o)  (o)  |
        |      /\    |
         \    '--'  /
      .---'-.____.-'---.
     /|  ___      ___  |\
    / | |   |    |   | | \
   |  | |BOOK|  |BOOK| |  |
   |  | |___ |  |___ | |  |
   |__|________________|__|
        |  W O R D S  |)ART");   // end of the raw text block started above
            break;                                                              // leaves the switch/loop

        case SUBJECT_MATHEMATICS:                                               // THE NUMERAL
            // NOTE: the next 11 lines are ONE raw text block printed exactly as written, so comments cannot go inside it.
            printArt(R"ART(  7     +     3     -     9
    .---------------------.
    |   .-------------.   |
    | 4 |  (0)   (0)  | 2 |
    |   |             |   |
    | 1 |    \___/    | 6 |
    |   '-------------'   |
    '-----+---------+-----'
         /|  1+1=?  |\
        / |_________| \
       /_/           \_\)ART");   // end of the raw text block started above
            break;                                                              // leaves the switch/loop

        case SUBJECT_SCIENCE:                                                   // DR. VEX
            // NOTE: the next 10 lines are ONE raw text block printed exactly as written, so comments cannot go inside it.
            printArt(R"ART(      \ \ | | /  /
      \ \|_|/  /         o  O
     .-"""""""-.       .-------.
    / [O]___[O] \      |  ~~~  |
   |    \___/    |     | ~~~~~ |
    \   '---'   /      |~~~~~~~|
   __'-._____.-'__     '-------'
  /  \  |   |  /  \        FLASK
 /VEX / |   | \ LAB\
/____/__|___|__\____\)ART");   // end of the raw text block started above
            break;                                                              // leaves the switch/loop

        case SUBJECT_COMPUTER:                                                  // THE GLITCH
        default:                                                                // any other value ends up here
            // NOTE: the next 10 lines are ONE raw text block printed exactly as written, so comments cannot go inside it.
            printArt(R"ART(.--------------------------------.
|    ####  !! ERROR !!  ####     |
|   .----------. .----------.    |
|   | [X]  [X] | | 0101 1010|    |
|   |   ____   | | 1100 0011|    |
|   |  /\/\/\  | | 0110 1001|    |
|   '----------' '----------'    |
|  S Y S T E M   F A I L U R E   |
'--------------------------------'
      ||||||          ||||||)ART");   // end of the raw text block started above
            break;                                                              // leaves the switch/loop
    }                                                                           // end of switch
}                                                                               // end of function showBossFigure

void showHeadmasterFigure()                                                     // function showHeadmasterFigure: shows the ASCII art of the Headmaster
{                                                                               // start of function showHeadmasterFigure
    // NOTE: the next 16 lines are ONE raw text block printed exactly as written, so comments cannot go inside it.
    printArt(R"ART(          _____________
         /_____________\
              \   /
           .-'-._.-'-.
          /  _     _  \
         |  (@)   (@)  |
         |      V      |
          \   .---.   /
           \  \___/  /
        .---'-.___.-'---.
       /  |    | |    |  \
      /   |    | |    |   \
     |____|____|_|____|____|
.-------------------------------.
|  T H E   H E A D M A S T E R  |
'-------------------------------')ART");   // end of the raw text block started above
}                                                                               // end of function showHeadmasterFigure


// ============================================================
// MAIN GAME LOOP
// Shows the HUD + available choices, reads one command, runs it.
// ============================================================

void gameLoop(Player &player)                                                   // main loop: runs rooms and commands, calls the boss fight
{                                                                               // start of block
    string command;                                                             // holds the command the player types

    while (true)                                                                // keep playing until a return leaves the loop
    {                                                                           // start of block
        if (player.hp <= 0)                                                     // is the player dead?
        {                                                                       // start of block
            if (gameOver(player))                                               // same player chose to continue
            {                                                                   // start of block
                continue;                                                       // go back to the top of the loop
            }                                                                   // end of block
            return;                                                             // leave this function (after victory this ends the game loop)
        }                                                                       // end of block

        if (player.currentRoom == ROOM_SEALED_CHAMBER)                          // final area?
        {                                                                       // start of block
            headmasterBattle(player);                                           // final boss fight

            if (player.hp <= 0 && gameOver(player))                             // died in the fight and chose to continue
            {                                                                   // start of block
                continue;                                                       // go back to the top of the loop
            }                                                                   // end of block
            return;                                                             // victory, quit, or input closed
        }                                                                       // end of block

        showChoices(player);                                                    // HUD, exits and actions

        cout << "What will you do? > ";                                         // prompt for a command

        if (!readCommandLine(command))                                          // input closed?
        {                                                                       // start of block
            return;                                                             // leave this function (after victory this ends the game loop)
        }                                                                       // end of block

        if (command.empty())                                                    // player typed nothing
        {                                                                       // start of block
            cout << "Please enter a command.\n";                                // ask again
            continue;                                                           // go back to the top of the loop
        }                                                                       // end of block

        if (!processCommand(player, command))                                   // quit chosen / input closed
        {                                                                       // start of block
            return;                                                             // leave this function (after victory this ends the game loop)
        }                                                                       // end of block

        // Keep a checkpoint after each completed command so progress survives
        // even if the player forgets to type "save".
        saveGame(player, false);                                                // silent checkpoint after each command
    }                                                                           // end of block
}                                                                               // end of block


// ============================================================
// COMMAND PROCESSOR
// Returns false when the game should end (quit or input closed).
// ============================================================

bool processCommand(Player &player, string command)                             // function processCommand: runs one typed command; returns false when the game should end
{                                                                               // start of function processCommand
    switch (parseCommand(command))                                              // chooses what to do based on parseCommand(command)
    {                                                                           // start of switch
        case CMD_GO_NORTH:  movePlayer(player, DIR_NORTH);  break;              // when the value is CMD_GO_NORTH: calls movePlayer: walks the player through an exit if it exists and...
        case CMD_GO_SOUTH:  movePlayer(player, DIR_SOUTH);  break;              // when the value is CMD_GO_SOUTH: calls movePlayer: walks the player through an exit if it exists and...
        case CMD_GO_EAST:   movePlayer(player, DIR_EAST);   break;              // when the value is CMD_GO_EAST: calls movePlayer: walks the player through an exit if it exists and...
        case CMD_GO_WEST:   movePlayer(player, DIR_WEST);   break;              // when the value is CMD_GO_WEST: calls movePlayer: walks the player through an exit if it exists and...

        case CMD_LOOK:      showLocation(player);           break;              // when the value is CMD_LOOK: calls showLocation: describes the current room; then leaves the switch
        case CMD_MAP:       showMap(player);                break;              // when the value is CMD_MAP: calls showMap: draws the map; then leaves the switch
        case CMD_TAKE_KEY:  takeKey(player);                break;              // when the value is CMD_TAKE_KEY: calls takeKey: picks up the Mysterious Key in the Library; then lea...
        case CMD_OPEN_CHEST:openChest(player);              break;              // when the value is CMD_OPEN_CHEST: calls openChest: opens the chest in the Main Hall (needs the key)...
        case CMD_INVENTORY: showInventory(player);          break;              // when the value is CMD_INVENTORY: calls showInventory: lists the artifacts the player carries; then...
        case CMD_STATUS:    showStatus(player);             break;              // when the value is CMD_STATUS: calls showStatus: shows HP, XP, rank and progress; then leaves the sw...
        case CMD_HELP:      displayMechanics();             break;              // when the value is CMD_HELP: calls displayMechanics: shows the how-to-play rules; then leaves the sw...
        case CMD_SAVE:      saveGame(player);               break;              // when the value is CMD_SAVE: calls saveGame: saves the game; then leaves the switch

        case CMD_LOAD:                                                          // when the value is CMD_LOAD
            if (loadGame(player))                                               // restored successfully?
            {                                                                   // start of if block
                showStatus(player);                                             // calls showStatus: shows HP, XP, rank and progress
                showLocation(player);                                           // calls showLocation: describes the current room
            }                                                                   // end of if block
            break;                                                              // leaves the switch/loop

        case CMD_CHALLENGE:                                                     // when the value is CMD_CHALLENGE
            return guardianBattle(player, getSubjectForRoom(player));           // returns guardianBattle(player, getSubjectForRoom(player))

        case CMD_QUIT:                                                          // when the value is CMD_QUIT
        {                                                                       // start of case CMD_QUIT
            cout << "\nSave your progress before leaving? (y/n) > ";            // shows: Save your progress before leaving? (y/n) >

            string answer;                                                      // creates the variable answer (string)

            if (readCommandLine(answer) && (answer == "y" || answer == "yes"))  // checks whether readCommandLine(answer) and (answer == "y" or answer == "yes")
            {                                                                   // start of if block
                saveGame(player);                                               // calls saveGame: saves the game
            }                                                                   // end of if block

            cout << "\nYou decide to leave Blackwood Academy.\n";               // shows: You decide to leave Blackwood Academy.
            cout << "The mystery will remain for another day...\n";             // shows: The mystery will remain for another day...
            cout << "Thank you for playing, " << player.name << ".\n";          // shows: Thank you for playing, {player.name}.
            return false;                                                       // returns false
        }                                                                       // end of case CMD_QUIT

        case CMD_INVALID:                                                       // when the value is CMD_INVALID
        default:                                                                // any other value ends up here
            cout << "\nInvalid command.\n";                                     // shows: Invalid command.
            cout << "Type \"help\" to see the available commands.\n";           // shows: Type "help" to see the available commands.
            break;                                                              // leaves the switch/loop
    }                                                                           // end of switch

    return true;                                                                // returns true
}                                                                               // end of function processCommand


// ============================================================
// AVAILABLE CHOICES
// Prints a status bar, the exits and the actions that make
// sense in the current room before every prompt.
// ============================================================

void showChoices(const Player &player)                                          // function showChoices: shows the HUD, exits and actions before each prompt
{                                                                               // start of function showChoices
    cout << "\n";                                                               // prints a blank line
    printLine('-');                                                             // draws a full-width line of '-'

    cout << " " << padRight(roomName(player.currentRoom), 17)                   // shows: {padRight(roomName(player.currentRoom), 17)}
         << "HP " << makeBar(player.hp, PLAYER_MAX_HP, 20) << " "               // continues the output: HP {makeBar(player.hp, PLAYER_MAX_HP, 20)}
         << player.hp << "/" << PLAYER_MAX_HP << "\n";                          // continues the output: {player.hp}/{PLAYER_MAX_HP}

    cout << " XP " << padRight(to_string(player.xp), 6)                         // shows: XP {padRight(to_string(player.xp), 6)}
         << "| Rank: " << padRight(player.rank, 12)                             // continues the output: | Rank: {padRight(player.rank, 12)}
         << "| Guardians: " << countDefeated(player) << "/4\n";                 // continues the output: | Guardians: {countDefeated(player)}/4

    printLine('-');                                                             // draws a full-width line of '-'

    // ---- exits (two per line so the text stays aligned) ----
    vector<string> exits;                                                       // creates the variable exits (vector<string>)
    const Direction all[4] = { DIR_NORTH, DIR_SOUTH, DIR_EAST, DIR_WEST };      // creates the table all

    for (int i = 0; i < 4; i++)                                                 // repeats for i = 0 up to 4 - 1
    {                                                                           // start of for loop
        Room destination;                                                       // creates the variable destination (Room)

        if (getExit(player.currentRoom, all[i], destination))                   // checks whether getExit(player.currentRoom, all[i], destination)
        {                                                                       // start of if block
            string entry = "go " + directionName(all[i]) + " > "                // creates entry (string) starting as "go " + directionName(all[i]) + " > " (continues below)
                         + roomName(destination);                               // joins on: {roomName(destination)}

            if (isExitLocked(player, player.currentRoom, all[i]))               // checks whether isExitLocked(player, player.currentRoom, all[i])
            {                                                                   // start of if block
                entry += " (LOCKED)";                                           // adds " (LOCKED)" to entry
            }                                                                   // end of if block

            exits.push_back(entry);                                             // calls push_back: adds an item to the end of the list
        }                                                                       // end of if block
    }                                                                           // end of for loop

    cout << " EXITS   :\n";                                                     // shows: EXITS :

    for (size_t i = 0; i < exits.size(); i++)                                   // repeats for i = 0 up to exits.size() - 1
    {                                                                           // start of for loop
        cout << "   " << exits[i] << "\n";                                      // shows: {exits[i]}
    }                                                                           // end of for loop

    // ---- actions that are useful right now ----
    vector<string> actions;                                                     // creates the variable actions (vector<string>)

    if (player.currentRoom == ROOM_LIBRARY && !player.libraryKeyTaken)          // checks whether player.currentRoom == ROOM_LIBRARY and not player.libraryKeyTaken
    {                                                                           // start of if block
        actions.push_back("take key");                                          // calls push_back: adds an item to the end of the list
    }                                                                           // end of if block

    if (player.currentRoom == ROOM_HALL_OF_NUMBERS && !player.chestOpened)      // checks whether player.currentRoom == ROOM_HALL_OF_NUMBERS and not player.chestOpened
    {                                                                           // start of if block
        actions.push_back("open chest");                                        // calls push_back: adds an item to the end of the list
    }                                                                           // end of if block

    Subject subject = getSubjectForRoom(player);                                // creates subject (Subject) starting as getSubjectForRoom(player)

    if (subject != SUBJECT_NONE && !player.defeated[subject])                   // checks whether subject != SUBJECT_NONE and not player.defeated[subject]
    {                                                                           // start of if block
        actions.push_back("challenge");                                         // calls push_back: adds an item to the end of the list
    }                                                                           // end of if block

    actions.push_back("look");                                                  // calls push_back: adds an item to the end of the list
    actions.push_back("map");                                                   // calls push_back: adds an item to the end of the list
    actions.push_back("inventory");                                             // calls push_back: adds an item to the end of the list
    actions.push_back("status");                                                // calls push_back: adds an item to the end of the list
    actions.push_back("save");                                                  // calls push_back: adds an item to the end of the list
    actions.push_back("load");                                                  // calls push_back: adds an item to the end of the list
    actions.push_back("help");                                                  // calls push_back: adds an item to the end of the list
    actions.push_back("quit");                                                  // calls push_back: adds an item to the end of the list

    cout << " ACTIONS :";                                                       // shows: ACTIONS :

    size_t used = 10;                                                           // width of " ACTIONS :"

    for (size_t i = 0; i < actions.size(); i++)                                 // repeats for i = 0 up to actions.size() - 1
    {                                                                           // start of for loop
        string piece = " " + actions[i] + (i + 1 < actions.size() ? "," : "");  // creates piece (string) starting as " " + actions[i] + (i + 1 < actions.size() ? ","...

        if (used + piece.size() > static_cast<size_t>(LINE_WIDTH))              // checks whether used + piece.size() > static_cast<size_t>(LINE_WIDTH)
        {                                                                       // start of if block
            cout << "\n          ";                                             // wrap, keep the column
            used = 10;                                                          // sets used to 10
        }                                                                       // end of if block

        cout << piece;                                                          // shows: {piece}
        used += piece.size();                                                   // adds piece.size() to used
    }                                                                           // end of for loop

    cout << "\n";                                                               // prints a blank line

    if (subject != SUBJECT_NONE)                                                // show boss state in guardian rooms
    {                                                                           // start of if block
        cout << " GUARDIAN :";                                                  // shows: GUARDIAN :
        cout << " " << BOSS_NAMES[subject] << "  "                              // shows: {BOSS_NAMES[subject]}
             << (player.defeated[subject] ? "[ DEFEATED ]" : "[ ALIVE ]")       // continues the output: [ DEFEATED ][ ALIVE ]
             << "\n";                                                           // also outputs:
    }                                                                           // end of if block

    printLine('-');                                                             // draws a full-width line of '-'
}                                                                               // end of function showChoices


// ============================================================
// MAP
// Draws a box for every room with the guardian's state, the
// key / chest markers, locked door and the player's position.
// ============================================================

// Which guardian lives in a room (no Player needed).
static Subject subjectOfRoom(Room room)                                         // function subjectOfRoom: returns which guardian subject belongs to a room
{                                                                               // start of function subjectOfRoom
    switch (room)                                                               // chooses what to do based on room
    {                                                                           // start of switch
        case ROOM_LIBRARY:         return SUBJECT_ENGLISH;                      // when the value is ROOM_LIBRARY: returns SUBJECT_ENGLISH
        case ROOM_HALL_OF_NUMBERS: return SUBJECT_MATHEMATICS;                  // when the value is ROOM_HALL_OF_NUMBERS: returns SUBJECT_MATHEMATICS
        case ROOM_LABORATORY:      return SUBJECT_SCIENCE;                      // when the value is ROOM_LABORATORY: returns SUBJECT_SCIENCE
        case ROOM_SERVER_ROOM:     return SUBJECT_COMPUTER;                     // when the value is ROOM_SERVER_ROOM: returns SUBJECT_COMPUTER
        default:                   return SUBJECT_NONE;                         // any other value ends up here
    }                                                                           // end of switch
}                                                                               // end of function subjectOfRoom

// Builds the 7 text rows of one room box (each exactly 18 wide).
static vector<string> roomBox(const Player &player, Room room)                  // function roomBox: builds the little text box drawn for one room on the map
{                                                                               // start of function roomBox
    const int inner = 16;                                                       // width between the borders
    Subject subject = subjectOfRoom(room);                                      // creates subject (Subject) starting as subjectOfRoom(room)

    string title = ROOM_NAMES[room];                                            // creates title (string) starting as ROOM_NAMES[room]

    for (size_t i = 0; i < title.size(); i++)                                   // map titles in capitals
    {                                                                           // start of for loop
        title[i] = static_cast<char>(toupper(static_cast<unsigned char>(title[i])));  // sets title[i] to static_cast<char>(toupper(static_cast<unsigned ch...
    }                                                                           // end of for loop
    string row2, row3, row4;                                                    // creates the variables row2, row3, row4 (string)

    if (room == ROOM_MAIN_HALL)                                                 // checks whether room == ROOM_MAIN_HALL
    {                                                                           // start of if block
        row2 = "(central hub)";                                                 // sets row2 to the text "(central hub)"
        row3 = "4 exits";                                                       // sets row3 to the text "4 exits"
    }                                                                           // end of if block
    else if (room == ROOM_SEALED_CHAMBER)                                       // otherwise, checks whether room == ROOM_SEALED_CHAMBER
    {                                                                           // start of else-if block
        row2 = "The Headmaster";                                                // sets row2 to the text "The Headmaster"
        row3 = (countDefeated(player) == 4) ? "[ ] UNLOCKED" : "[#] LOCKED";    // sets row3 to (countDefeated(player) == 4) ? "[ ] UNLOCKED" : "...
    }                                                                           // end of else-if block
    else                                                                        // otherwise (none of the conditions above were true)
    {                                                                           // start of else block
        row2 = BOSS_NAMES[subject];                                             // sets row2 to BOSS_NAMES[subject]
        row3 = player.defeated[subject] ? "[X] DEFEATED" : "[B] ALIVE";         // sets row3 to player.defeated[subject] ? "[X] DEFEATED" : "[B]...

        if (room == ROOM_LIBRARY && !player.libraryKeyTaken)                    // checks whether room == ROOM_LIBRARY and not player.libraryKeyTaken
        {                                                                       // start of if block
            row4 = "[K] key here";                                              // sets row4 to the text "[K] key here"
        }                                                                       // end of if block
        else if (room == ROOM_HALL_OF_NUMBERS && !player.chestOpened)           // otherwise, checks whether room == ROOM_HALL_OF_NUMBERS and not player.chestOpened
        {                                                                       // start of else-if block
            row4 = "[C] chest here";                                            // sets row4 to the text "[C] chest here"
        }                                                                       // end of else-if block
    }                                                                           // end of else block

    string marker = (player.currentRoom == room) ? "<< YOU >>" : "";            // creates marker (string) starting as (player.currentRoom == room) ? "<< YOU >>" : ""

    vector<string> rows;                                                        // creates the variable rows (vector<string>)
    string border = "+" + string(static_cast<size_t>(inner), '-') + "+";        // creates border (string) starting as "+" + string(static_cast<size_t>(inner), '-') + "...

    rows.push_back(border);                                                     // calls push_back: adds an item to the end of the list
    rows.push_back("|" + centerIn(title, inner) + "|");                         // calls push_back: adds an item to the end of the list
    rows.push_back("|" + centerIn(row2,  inner) + "|");                         // calls push_back: adds an item to the end of the list
    rows.push_back("|" + centerIn(row3,  inner) + "|");                         // calls push_back: adds an item to the end of the list
    rows.push_back("|" + centerIn(row4,  inner) + "|");                         // calls push_back: adds an item to the end of the list
    rows.push_back("|" + centerIn(marker, inner) + "|");                        // calls push_back: adds an item to the end of the list
    rows.push_back(border);                                                     // calls push_back: adds an item to the end of the list

    return rows;                                                                // returns rows
}                                                                               // end of function roomBox

void showMap(const Player &player)                                              // function showMap: draws the academy map
{                                                                               // start of function showMap
    const string indent(21, ' ');                                               // 18 (box) + 3 (corridor)
    const string vertical = indent + string(9, ' ') + "|";                      // corridor piece

    vector<string> lab     = roomBox(player, ROOM_LABORATORY);                  // creates lab (vector<string>) starting as roomBox(player, ROOM_LABORATORY)
    vector<string> library = roomBox(player, ROOM_LIBRARY);                     // creates library (vector<string>) starting as roomBox(player, ROOM_LIBRARY)
    vector<string> hall    = roomBox(player, ROOM_MAIN_HALL);                   // creates hall (vector<string>) starting as roomBox(player, ROOM_MAIN_HALL)
    vector<string> numbers = roomBox(player, ROOM_HALL_OF_NUMBERS);             // creates numbers (vector<string>) starting as roomBox(player, ROOM_HALL_OF_NUMBERS)
    vector<string> server  = roomBox(player, ROOM_SERVER_ROOM);                 // creates server (vector<string>) starting as roomBox(player, ROOM_SERVER_ROOM)
    vector<string> chamber = roomBox(player, ROOM_SEALED_CHAMBER);              // creates chamber (vector<string>) starting as roomBox(player, ROOM_SEALED_CHAMBER)

    cout << "\n";                                                               // prints a blank line
    printBanner("BLACKWOOD ACADEMY - FLOOR PLAN");                              // shows a banner titled: BLACKWOOD ACADEMY - FLOOR PLAN
    cout << "\n";                                                               // prints a blank line

    cout << indent << string(9, ' ') << "N\n";                                  // shows: {indent}{string(9, ' ')}N
    cout << indent << string(7, ' ') << "W + E\n";                              // shows: {indent}{string(7, ' ')}W + E
    cout << indent << string(9, ' ') << "S\n\n";                                // shows: {indent}{string(9, ' ')}S

    for (size_t i = 0; i < lab.size(); i++)                                     // laboratory (north)
    {                                                                           // start of for loop
        cout << indent << lab[i] << "\n";                                       // shows: {indent}{lab[i]}
    }                                                                           // end of for loop

    cout << vertical << "\n" << vertical << "\n";                               // shows: {vertical}{vertical}

    for (size_t i = 0; i < hall.size(); i++)                                    // library - hall - numbers
    {                                                                           // start of for loop
        string link = (i == 3) ? "---" : "   ";                                 // doorway at row 3
        cout << library[i] << link << hall[i] << link << numbers[i] << "\n";    // shows: {library[i]}{link}{hall[i]}{link}{numbers[i]}
    }                                                                           // end of for loop

    cout << vertical << "\n" << vertical << "\n";                               // shows: {vertical}{vertical}

    for (size_t i = 0; i < server.size(); i++)                                  // server room (south)
    {                                                                           // start of for loop
        cout << indent << server[i] << "\n";                                    // shows: {indent}{server[i]}
    }                                                                           // end of for loop

    if (countDefeated(player) == 4)                                             // door to the chamber
    {                                                                           // start of if block
        cout << vertical << "\n" << vertical << "\n";                           // shows: {vertical}{vertical}
    }                                                                           // end of if block
    else                                                                        // otherwise (none of the conditions above were true)
    {                                                                           // start of else block
        cout << vertical << "\n";                                               // shows: {vertical}
        cout << indent << string(7, ' ') << "[#] LOCKED DOOR\n";                // shows: {indent}{string(7, ' ')}[#] LOCKED DOOR
        cout << vertical << "\n";                                               // shows: {vertical}
    }                                                                           // end of else block

    for (size_t i = 0; i < chamber.size(); i++)                                 // sealed chamber
    {                                                                           // start of for loop
        cout << indent << chamber[i] << "\n";                                   // shows: {indent}{chamber[i]}
    }                                                                           // end of for loop

    cout << "\n";                                                               // prints a blank line
    printLine('-');                                                             // draws a full-width line of '-'
    cout << " LEGEND\n";                                                        // shows: LEGEND
    cout << " << YOU >>  Your position      [B] Guardian alive\n";              // shows: << YOU >> Your position [B] Guardian alive
    cout << " [X]  Guardian defeated        [#] Locked\n";                      // shows: [X] Guardian defeated [#] Locked
    cout << " [K]  Key waiting              [C] Chest waiting\n";               // shows: [K] Key waiting [C] Chest waiting
    printLine('-');                                                             // draws a full-width line of '-'

    printField("You are in", roomName(player.currentRoom));                     // shows the row "You are in" = {roomName(player.currentRoom)}
    printField("Guardians defeated",                                            // calls printField: prints a label and value in aligned columns (arguments continue below)
               to_string(countDefeated(player)) + "/4  "                        // argument / continuation: {to_string(countDefeated(player))}/4
               + makeBar(countDefeated(player), 4, 12));                        // joins on: {makeBar(countDefeated(player), 4, 12)}
    printField("Sealed Chamber",                                                // calls printField: prints a label and value in aligned columns (arguments continue below)
               countDefeated(player) == 4 ? "UNLOCKED" : "LOCKED");             // picks UNLOCKED or LOCKED depending on: countDefeated(player) == 4
    printLine('=');                                                             // draws a full-width line of '='
}                                                                               // end of function showMap


// ============================================================
// MOVEMENT
// ============================================================

// Fills "destination" and returns true if a path exists.
bool getExit(Room from, Direction direction, Room &destination)                 // function getExit: finds where an exit leads; false if there is no path
{                                                                               // start of function getExit
    switch (from)                                                               // chooses what to do based on from
    {                                                                           // start of switch
        case ROOM_MAIN_HALL:                                                    // when the value is ROOM_MAIN_HALL
            if (direction == DIR_NORTH) { destination = ROOM_LABORATORY;      return true; }  // checks whether direction == DIR_NORTH; if so: { destination = ROOM_LABORATO...
            if (direction == DIR_SOUTH) { destination = ROOM_SERVER_ROOM;     return true; }  // checks whether direction == DIR_SOUTH; if so: { destination = ROOM_SERVER_R...
            if (direction == DIR_EAST)  { destination = ROOM_HALL_OF_NUMBERS; return true; }  // checks whether direction == DIR_EAST; if so: { destination = ROOM_HALL_OF_...
            if (direction == DIR_WEST)  { destination = ROOM_LIBRARY;         return true; }  // checks whether direction == DIR_WEST; if so: { destination = ROOM_LIBRARY;...
            break;                                                              // leaves the switch/loop

        case ROOM_LIBRARY:                                                      // when the value is ROOM_LIBRARY
            if (direction == DIR_EAST)  { destination = ROOM_MAIN_HALL;       return true; }  // checks whether direction == DIR_EAST; if so: { destination = ROOM_MAIN_HAL...
            break;                                                              // leaves the switch/loop

        case ROOM_HALL_OF_NUMBERS:                                              // when the value is ROOM_HALL_OF_NUMBERS
            if (direction == DIR_WEST)  { destination = ROOM_MAIN_HALL;       return true; }  // checks whether direction == DIR_WEST; if so: { destination = ROOM_MAIN_HAL...
            break;                                                              // leaves the switch/loop

        case ROOM_LABORATORY:                                                   // when the value is ROOM_LABORATORY
            if (direction == DIR_SOUTH) { destination = ROOM_MAIN_HALL;       return true; }  // checks whether direction == DIR_SOUTH; if so: { destination = ROOM_MAIN_HAL...
            break;                                                              // leaves the switch/loop

        case ROOM_SERVER_ROOM:                                                  // when the value is ROOM_SERVER_ROOM
            if (direction == DIR_NORTH) { destination = ROOM_MAIN_HALL;       return true; }  // checks whether direction == DIR_NORTH; if so: { destination = ROOM_MAIN_HAL...
            if (direction == DIR_SOUTH) { destination = ROOM_SEALED_CHAMBER;  return true; }  // checks whether direction == DIR_SOUTH; if so: { destination = ROOM_SEALED_C...
            break;                                                              // leaves the switch/loop

        case ROOM_SEALED_CHAMBER:                                               // when the value is ROOM_SEALED_CHAMBER
            if (direction == DIR_NORTH) { destination = ROOM_SERVER_ROOM;     return true; }  // checks whether direction == DIR_NORTH; if so: { destination = ROOM_SERVER_R...
            break;                                                              // leaves the switch/loop
    }                                                                           // end of switch

    return false;                                                               // returns false
}                                                                               // end of function getExit

// Only the door between the Server Room and the Chamber can be locked.
bool isExitLocked(const Player &player, Room from, Direction direction)         // function isExitLocked: true when the exit is the locked Sealed Chamber door
{                                                                               // start of function isExitLocked
    return from == ROOM_SERVER_ROOM                                             // returns from == ROOM_SERVER_ROOM (continues below)
        && direction == DIR_SOUTH                                               // and also direction == DIR_SOUTH
        && countDefeated(player) < 4;                                           // and also countDefeated(player) < 4;
}                                                                               // end of function isExitLocked

void movePlayer(Player &player, Direction direction)                            // function movePlayer: walks the player through an exit if it exists and is unlocked
{                                                                               // start of function movePlayer
    Room nextRoom = player.currentRoom;                                         // creates nextRoom (Room) starting as player.currentRoom

    if (!getExit(player.currentRoom, direction, nextRoom))                      // checks whether not getExit(player.currentRoom, direction, nextRoom)
    {                                                                           // start of if block
        cout << "\nYou cannot go " << directionName(direction)                  // shows: You cannot go {directionName(direction)}
             << " from the " << roomName(player.currentRoom) << ".\n";          // continues the output: from the {roomName(player.currentRoom)}.
        cout << "Check the EXITS list for the paths you can take.\n";           // shows: Check the EXITS list for the paths you can take.
        return;                                                                 // leaves the function
    }                                                                           // end of if block

    if (isExitLocked(player, player.currentRoom, direction))                    // checks whether isExitLocked(player, player.currentRoom, direction)
    {                                                                           // start of if block
        cout << "\nYou approach the enormous sealed door.\n\n";                 // shows: You approach the enormous sealed door.
        cout << "Four symbols are carved into its surface.\n";                  // shows: Four symbols are carved into its surface.
        cout << "Only " << countDefeated(player)                                // shows: Only {countDefeated(player)}
             << " of the 4 symbols are glowing.\n\n";                           // continues the output: of the 4 symbols are glowing.
        cout << "The Sealed Chamber remains LOCKED.\n";                         // shows: The Sealed Chamber remains LOCKED.
        cout << "Defeat all four guardians first.\n";                           // shows: Defeat all four guardians first.
        return;                                                                 // leaves the function
    }                                                                           // end of if block

    player.currentRoom = nextRoom;                                              // sets player.currentRoom to nextRoom

    cout << "\nYou walk " << directionName(direction) << "...\n";               // shows: You walk {directionName(direction)}...
    cout << "You enter the " << roomName(player.currentRoom) << ".\n";          // shows: You enter the {roomName(player.currentRoom)}.

    showLocation(player);                                                       // calls showLocation: describes the current room
}                                                                               // end of function movePlayer


// ============================================================
// ROOM DESCRIPTIONS
// The exits are listed by showChoices() before every prompt.
// ============================================================

// One clear line saying if the room's guardian is alive or defeated.
void showGuardianBadge(const Player &player, Subject subject)                   // function showGuardianBadge: shows whether the room guardian is alive or defeated
{                                                                               // start of function showGuardianBadge
    if (subject == SUBJECT_NONE)                                                // checks whether subject == SUBJECT_NONE
    {                                                                           // start of if block
        return;                                                                 // leaves the function
    }                                                                           // end of if block

    cout << "\n";                                                               // prints a blank line
    printLine('-');                                                             // draws a full-width line of '-'

    if (player.defeated[subject])                                               // checks whether player.defeated[subject]
    {                                                                           // start of if block
        cout << " GUARDIAN : " << padRight(BOSS_NAMES[subject], 15)             // shows: GUARDIAN : {padRight(BOSS_NAMES[subject], 15)}
             << "[ DEFEATED ]\n";                                               // continues the output: [ DEFEATED ]
        cout << " " << SEAL_NAMES[subject] << " has been collected.\n";         // shows: {SEAL_NAMES[subject]} has been collected.
    }                                                                           // end of if block
    else                                                                        // otherwise (none of the conditions above were true)
    {                                                                           // start of else block
        cout << " GUARDIAN : " << padRight(BOSS_NAMES[subject], 15)             // shows: GUARDIAN : {padRight(BOSS_NAMES[subject], 15)}
             << "[ ALIVE ]\n";                                                  // continues the output: [ ALIVE ]
        cout << " Type \"challenge\" when you are ready to fight.\n";           // shows: Type "challenge" when you are ready to fight.
    }                                                                           // end of else block

    printLine('-');                                                             // draws a full-width line of '-'
}                                                                               // end of function showGuardianBadge

void showLocation(const Player &player)                                         // function showLocation: describes the current room
{                                                                               // start of function showLocation
    cout << "\n";                                                               // prints a blank line
    printLine('=');                                                             // draws a full-width line of '='

    switch (player.currentRoom)                                                 // chooses what to do based on player.currentRoom
    {                                                                           // start of switch
        case ROOM_MAIN_HALL:                                                    // when the value is ROOM_MAIN_HALL
            printCentered("MAIN HALL");                                         // shows centered: MAIN HALL
            printLine('=');                                                     // draws a full-width line of '='

            cout << "\nYou stand in the grand center of Blackwood Academy.\n\n";  // shows: You stand in the grand center of Blackwood Academy.
            cout << "A massive broken chandelier hangs above you.\n";           // shows: A massive broken chandelier hangs above you.
            cout << "Four dark archways lead into different wings:\n\n";        // shows: Four dark archways lead into different wings:
            cout << "  NORTH : Laboratory      EAST : Hall of Numbers\n";       // shows: NORTH : Laboratory EAST : Hall of Numbers
            cout << "  SOUTH : Server Room     WEST : Library\n\n";             // shows: SOUTH : Server Room WEST : Library
            cout << "The academy is completely silent.\n";                      // shows: The academy is completely silent.

            if (countDefeated(player) == 0)                                     // checks whether countDefeated(player) == 0
            {                                                                   // start of if block
                cout << "\nObjective: defeat the 4 guardians.\n";               // shows: Objective: defeat the 4 guardians.
            }                                                                   // end of if block
            else if (countDefeated(player) < 4)                                 // otherwise, checks whether countDefeated(player) < 4
            {                                                                   // start of else-if block
                cout << "\nGuardians defeated: " << countDefeated(player)       // shows: Guardians defeated: {countDefeated(player)}
                     << "/4. Keep going!\n";                                    // continues the output: /4. Keep going!
            }                                                                   // end of else-if block
            else                                                                // otherwise (none of the conditions above were true)
            {                                                                   // start of else block
                cout << "\nAll guardians are defeated!\n";                      // shows: All guardians are defeated!
                cout << "Go to the Server Room, then SOUTH.\n";                 // shows: Go to the Server Room, then SOUTH.
            }                                                                   // end of else block
            break;                                                              // leaves the switch/loop

        case ROOM_LIBRARY:                                                      // when the value is ROOM_LIBRARY
            printCentered("LIBRARY");                                           // shows centered: LIBRARY
            printLine('=');                                                     // draws a full-width line of '='

            cout << "\nTowering bookshelves disappear into darkness.\n\n";      // shows: Towering bookshelves disappear into darkness.
            cout << "WHISPER...\nWHISPER...\nWHISPER...\n\n";                   // shows: WHISPER...WHISPER...WHISPER...
            cout << "Words crawl across the walls before disappearing.\n\n";    // shows: Words crawl across the walls before disappearing.

            if (!player.libraryKeyTaken)                                        // checks whether not player.libraryKeyTaken
            {                                                                   // start of if block
                cout << "Something metallic glints beneath a fallen book.\n";   // shows: Something metallic glints beneath a fallen book.
                cout << "Type \"take key\" to investigate it.\n";               // shows: Type "take key" to investigate it.
            }                                                                   // end of if block
            else                                                                // otherwise (none of the conditions above were true)
            {                                                                   // start of else block
                cout << "The table where you discovered the key is empty.\n";   // shows: The table where you discovered the key is empty.
            }                                                                   // end of else block

            if (!player.defeated[SUBJECT_ENGLISH])                              // checks whether not player.defeated[SUBJECT_ENGLISH]
            {                                                                   // start of if block
                cout << "\nEvery book suddenly SLAMS shut.\n";                  // shows: Every book suddenly SLAMS shut.
                cout << "A tall shadow appears between the shelves.\n\n";       // shows: A tall shadow appears between the shelves.
                cout << "\"Words created this academy.\"\n";                    // shows: "Words created this academy."
                cout << "\"Let us see if yours can save you.\"\n";              // shows: "Let us see if yours can save you."
            }                                                                   // end of if block
            else                                                                // otherwise (none of the conditions above were true)
            {                                                                   // start of else block
                cout << "\nThe Library is finally quiet.\n";                    // shows: The Library is finally quiet.
            }                                                                   // end of else block

            showGuardianBadge(player, SUBJECT_ENGLISH);                         // calls showGuardianBadge: shows whether the room guardian is alive or defeated
            return;                                                             // leaves the function

        case ROOM_HALL_OF_NUMBERS:                                              // when the value is ROOM_HALL_OF_NUMBERS
            printCentered("HALL OF NUMBERS");                                   // shows centered: HALL OF NUMBERS
            printLine('=');                                                     // draws a full-width line of '='

            cout << "\nClocks along the walls are running backward.\n";         // shows: Clocks along the walls are running backward.
            cout << "Numbers change every time you blink.\n";                   // shows: Numbers change every time you blink.
            cout << "Equations rearrange themselves across the floor.\n\n";     // shows: Equations rearrange themselves across the floor.

            if (!player.chestOpened)                                            // checks whether not player.chestOpened
            {                                                                   // start of if block
                cout << "A heavy iron chest sits in the center of the room.\n";  // shows: A heavy iron chest sits in the center of the room.
                cout << "An ancient lock keeps it closed.\n";                   // shows: An ancient lock keeps it closed.
                cout << "Type \"open chest\" to investigate it.\n";             // shows: Type "open chest" to investigate it.
            }                                                                   // end of if block
            else                                                                // otherwise (none of the conditions above were true)
            {                                                                   // start of else block
                cout << "The ancient chest stands open and empty.\n";           // shows: The ancient chest stands open and empty.
            }                                                                   // end of else block

            if (!player.defeated[SUBJECT_MATHEMATICS])                          // checks whether not player.defeated[SUBJECT_MATHEMATICS]
            {                                                                   // start of if block
                cout << "\nA voice counts down from the darkness...\n";         // shows: A voice counts down from the darkness...
                cout << "10... 9... 8...\n";                                    // shows: 10... 9... 8...
            }                                                                   // end of if block
            else                                                                // otherwise (none of the conditions above were true)
            {                                                                   // start of else block
                cout << "\nThe impossible clocks have finally stopped.\n";      // shows: The impossible clocks have finally stopped.
            }                                                                   // end of else block

            showGuardianBadge(player, SUBJECT_MATHEMATICS);                     // calls showGuardianBadge: shows whether the room guardian is alive or defeated
            return;                                                             // leaves the function

        case ROOM_LABORATORY:                                                   // when the value is ROOM_LABORATORY
            printCentered("ABANDONED LABORATORY");                              // shows centered: ABANDONED LABORATORY
            printLine('=');                                                     // draws a full-width line of '='

            cout << "\nBroken experiments fill the laboratory.\n";              // shows: Broken experiments fill the laboratory.
            cout << "Green liquid still bubbles inside cracked containers.\n";  // shows: Green liquid still bubbles inside cracked containers.
            cout << "Something moves behind one of the glass chambers.\n";      // shows: Something moves behind one of the glass chambers.

            if (!player.defeated[SUBJECT_SCIENCE])                              // checks whether not player.defeated[SUBJECT_SCIENCE]
            {                                                                   // start of if block
                cout << "\nCRASH!\n\nOne of the chambers shatters.\n";          // shows: CRASH!One of the chambers shatters.
                cout << "A man in a ruined laboratory coat steps forward.\n\n";  // shows: A man in a ruined laboratory coat steps forward.
                cout << "\"Knowledge without experimentation is useless.\"\n";  // shows: "Knowledge without experimentation is useless."
                cout << "\"So tonight... you are the experiment.\"\n";          // shows: "So tonight... you are the experiment."
            }                                                                   // end of if block
            else                                                                // otherwise (none of the conditions above were true)
            {                                                                   // start of else block
                cout << "\nThe laboratory has finally fallen silent.\n";        // shows: The laboratory has finally fallen silent.
            }                                                                   // end of else block

            showGuardianBadge(player, SUBJECT_SCIENCE);                         // calls showGuardianBadge: shows whether the room guardian is alive or defeated
            return;                                                             // leaves the function

        case ROOM_SERVER_ROOM:                                                  // when the value is ROOM_SERVER_ROOM
            printCentered("SERVER ROOM");                                       // shows centered: SERVER ROOM
            printLine('=');                                                     // draws a full-width line of '='

            cout << "\nRows of dead computers fill the dark room.\n\n";         // shows: Rows of dead computers fill the dark room.
            cout << "Then...  BEEP.\n\n";                                       // shows: Then... BEEP.
            cout << "Every computer turns on at the same time.\n";              // shows: Every computer turns on at the same time.
            cout << "Each monitor displays:\n\n";                               // shows: Each monitor displays:
            printCentered("USER DETECTED");                                     // shows centered: USER DETECTED
            printCentered(player.name);                                         // shows centered: {player.name}

            if (!player.defeated[SUBJECT_COMPUTER])                             // checks whether not player.defeated[SUBJECT_COMPUTER]
            {                                                                   // start of if block
                cout << "\nThe doors suddenly lock.\n\n";                       // shows: The doors suddenly lock.
                cout << "\"HUMAN KNOWLEDGE DETECTED.\"\n";                      // shows: "HUMAN KNOWLEDGE DETECTED."
                cout << "\"TESTING REQUIRED.\"\n";                              // shows: "TESTING REQUIRED."
            }                                                                   // end of if block
            else                                                                // otherwise (none of the conditions above were true)
            {                                                                   // start of else block
                cout << "\nThe infected computers have gone dark.\n";           // shows: The infected computers have gone dark.
            }                                                                   // end of else block

            cout << "\nSouth of this room: the Sealed Chamber ("                // shows: South of this room: the Sealed Chamber (
                 << (countDefeated(player) < 4 ? "LOCKED" : "UNLOCKED")         // continues the output: {(countDefeated(player) < 4 ? "LOCKED" : "UNLOCKED")}
                 << ").\n";                                                     // continues the output: ).

            showGuardianBadge(player, SUBJECT_COMPUTER);                        // calls showGuardianBadge: shows whether the room guardian is alive or defeated
            return;                                                             // leaves the function

        case ROOM_SEALED_CHAMBER:                                               // when the value is ROOM_SEALED_CHAMBER
            printCentered("SEALED CHAMBER");                                    // shows centered: SEALED CHAMBER
            printLine('=');                                                     // draws a full-width line of '='

            cout << "\nThe four Seals of Knowledge begin to glow.\n";           // shows: The four Seals of Knowledge begin to glow.
            cout << "The floor trembles beneath your feet.\n\n";                // shows: The floor trembles beneath your feet.
            cout << "A hidden staircase slowly appears.\n";                     // shows: A hidden staircase slowly appears.
            cout << "It leads beneath Blackwood Academy...\n\n";                // shows: It leads beneath Blackwood Academy...
            cout << "To a room that does not exist on the map.\n\n";            // shows: To a room that does not exist on the map.
            printCentered("THE HEADMASTER'S OFFICE");                           // shows centered: THE HEADMASTER'S OFFICE
            break;                                                              // leaves the switch/loop
    }                                                                           // end of switch

    printLine('=');                                                             // draws a full-width line of '='
}                                                                               // end of function showLocation


// ============================================================
// TAKE KEY
// ============================================================

void takeKey(Player &player)                                                    // function takeKey: picks up the Mysterious Key in the Library
{                                                                               // start of function takeKey
    if (player.currentRoom != ROOM_LIBRARY)                                     // checks whether player.currentRoom != ROOM_LIBRARY
    {                                                                           // start of if block
        cout << "\nThere is no key here.\n";                                    // shows: There is no key here.
        return;                                                                 // leaves the function
    }                                                                           // end of if block

    if (player.libraryKeyTaken)                                                 // checks whether player.libraryKeyTaken
    {                                                                           // start of if block
        cout << "\nYou already collected the key.\n";                           // shows: You already collected the key.
        return;                                                                 // leaves the function
    }                                                                           // end of if block

    player.libraryKeyTaken = true;                                              // sets player.libraryKeyTaken to true
    player.hasKey = true;                                                       // sets player.hasKey to true

    cout << "\nYou carefully lift the fallen book...\n\n";                      // shows: You carefully lift the fallen book...
    cout << "A cold metal key lies underneath it.\n\n";                         // shows: A cold metal key lies underneath it.
    printCentered("[ MYSTERIOUS KEY ACQUIRED ]");                               // shows centered: [ MYSTERIOUS KEY ACQUIRED ]
    cout << "\nThe key has been added to your inventory.\n";                    // shows: The key has been added to your inventory.
    cout << "Maybe it opens the chest in the Hall of Numbers.\n";               // shows: Maybe it opens the chest in the Hall of Numbers.
}                                                                               // end of function takeKey


// ============================================================
// OPEN CHEST
// ============================================================

void openChest(Player &player)                                                  // function openChest: opens the chest in the Main Hall (needs the key)
{                                                                               // start of function openChest
    if (player.currentRoom != ROOM_HALL_OF_NUMBERS)                             // checks whether player.currentRoom != ROOM_HALL_OF_NUMBERS
    {                                                                           // start of if block
        cout << "\nThere is no chest here.\n";                                  // shows: There is no chest here.
        return;                                                                 // leaves the function
    }                                                                           // end of if block

    if (player.chestOpened)                                                     // checks whether player.chestOpened
    {                                                                           // start of if block
        cout << "\nThe chest is already open and empty.\n";                     // shows: The chest is already open and empty.
        return;                                                                 // leaves the function
    }                                                                           // end of if block

    if (!player.hasKey)                                                         // checks whether not player.hasKey
    {                                                                           // start of if block
        cout << "\nYou pull on the chest lid.\n\nCLANK!\n\n";                   // shows: You pull on the chest lid.CLANK!
        cout << "It is locked.\n";                                              // shows: It is locked.
        cout << "You need a key to open it.\n";                                 // shows: You need a key to open it.
        return;                                                                 // leaves the function
    }                                                                           // end of if block

    cout << "\nYou insert the Mysterious Key into the lock...\n\n";             // shows: You insert the Mysterious Key into the lock...
    cout << "CLICK!\n\n";                                                       // shows: CLICK!
    cout << "The ancient lock falls to the floor.\n";                           // shows: The ancient lock falls to the floor.
    cout << "The chest slowly opens...\n";                                      // shows: The chest slowly opens...
    cout << "A strange golden coin floats inside.\n\n";                         // shows: A strange golden coin floats inside.

    player.hasKey = false;                                                      // the key is used up
    player.chestOpened = true;                                                  // sets player.chestOpened to true
    player.item[ITEM_COIN] = true;                                              // sets player.item[ITEM_COIN] to true

    printCentered("[ ORACLE'S COIN ACQUIRED ]");                                // shows centered: [ ORACLE'S COIN ACQUIRED ]
    cout << "\nThe Mysterious Key has been used.\n";                            // shows: The Mysterious Key has been used.
    cout << "Oracle's Coin can activate 50/50 during a question.\n";            // shows: Oracle's Coin can activate 50/50 during a question.
}                                                                               // end of function openChest


// ============================================================
// INVENTORY
// ============================================================

void showInventory(const Player &player)                                        // function showInventory: lists the artifacts the player carries
{                                                                               // start of function showInventory
    cout << "\n";                                                               // prints a blank line
    printBanner("INVENTORY");                                                   // shows a banner titled: INVENTORY
    cout << "\n";                                                               // prints a blank line

    bool empty = true;                                                          // creates empty (bool) starting as true

    if (player.hasKey)                                                          // checks whether player.hasKey
    {                                                                           // start of if block
        cout << "- Mysterious Key\n";                                           // shows: - Mysterious Key
        cout << "  Effect: Opens the chest in the Hall of Numbers.\n\n";        // shows: Effect: Opens the chest in the Hall of Numbers.
        empty = false;                                                          // sets empty to false
    }                                                                           // end of if block

    for (int i = 0; i < 4; i++)                                                 // repeats for i = 0 up to 4 - 1
    {                                                                           // start of for loop
        if (player.item[i])                                                     // checks whether player.item[i]
        {                                                                       // start of if block
            cout << "- " << ITEM_NAMES[i] << "\n";                              // shows: - {ITEM_NAMES[i]}
            cout << "  Effect: " << ITEM_EFFECTS[i] << "\n\n";                  // shows: Effect: {ITEM_EFFECTS[i]}
            empty = false;                                                      // sets empty to false
        }                                                                       // end of if block
    }                                                                           // end of for loop

    if (empty)                                                                  // checks whether empty
    {                                                                           // start of if block
        cout << "Your inventory is empty.\n\n";                                 // shows: Your inventory is empty.
    }                                                                           // end of if block

    cout << "SEALS OF KNOWLEDGE\n";                                             // shows: SEALS OF KNOWLEDGE
    printLine('-');                                                             // draws a full-width line of '-'

    for (int i = 0; i < 4; i++)                                                 // repeats for i = 0 up to 4 - 1
    {                                                                           // start of for loop
        printField(SEAL_NAMES[i],                                               // calls printField: prints a label and value in aligned columns (arguments continue below)
                   player.defeated[i] ? "COLLECTED" : "MISSING");               // picks COLLECTED or MISSING depending on: player.defeated[i]
    }                                                                           // end of for loop

    cout << "\n";                                                               // prints a blank line
    printLine('=');                                                             // draws a full-width line of '='
}                                                                               // end of function showInventory


// ============================================================
// STATUS
// ============================================================

void showStatus(const Player &player)                                           // function showStatus: shows HP, XP, rank and progress
{                                                                               // start of function showStatus
    cout << "\n";                                                               // prints a blank line
    printBanner("PLAYER STATUS");                                               // shows a banner titled: PLAYER STATUS
    cout << "\n";                                                               // prints a blank line

    printField("Player", player.name);                                          // shows the row "Player" = {player.name}
    printField("Current Location", roomName(player.currentRoom));               // shows the row "Current Location" = {roomName(player.currentRoom)}
    printField("HP", makeBar(player.hp, PLAYER_MAX_HP, 20) + " "                // calls printField: prints a label and value in aligned columns (arguments continue below)
                     + to_string(player.hp) + "/" + to_string(PLAYER_MAX_HP));  // joins on: {to_string(player.hp)}/{to_string(PLAYER_MAX_HP)}
    printField("Knowledge XP", to_string(player.xp));                           // shows the row "Knowledge XP" = {to_string(player.xp)}
    printField("Rank", player.rank);                                            // shows the row "Rank" = {player.rank}
    printField("Correct Answers", to_string(player.correct) + " / "             // calls printField: prints a label and value in aligned columns (arguments continue below)
                                  + to_string(TOTAL_QUESTIONS));                // joins on: {to_string(TOTAL_QUESTIONS)}
    printField("Incorrect Answers", to_string(player.incorrect));               // shows the row "Incorrect Answers" = {to_string(player.incorrect)}
    printField("Guardians Defeated", to_string(countDefeated(player)) + "/4");  // shows the row "Guardians Defeated" = {to_string(countDefeated(player))}/4

    cout << "\nGUARDIANS\n";                                                    // shows: GUARDIANS
    printLine('-');                                                             // draws a full-width line of '-'

    const char *places[4] = { "Library", "Hall of Numbers",                     // creates places (const char) starting as { "Library", "Hall of Numbers", (continues below)
                              "Laboratory", "Server Room" };                    // list values: Laboratory, Server Room

    for (int i = 0; i < 4; i++)                                                 // repeats for i = 0 up to 4 - 1
    {                                                                           // start of for loop
        cout << padRight(BOSS_NAMES[i], 15)                                     // shows: {padRight(BOSS_NAMES[i], 15)}
             << padRight(string("(") + places[i] + ")", 19)                     // continues the output: {padRight(string("(") + places[i] + ")", 19)}
             << (player.defeated[i] ? "[ DEFEATED ]" : "[ ALIVE ]") << "\n";    // continues the output: [ DEFEATED ][ ALIVE ]
    }                                                                           // end of for loop

    cout << "\n";                                                               // prints a blank line
    printLine('=');                                                             // draws a full-width line of '='
}                                                                               // end of function showStatus


// ============================================================
// PROGRESS HELPERS
// ============================================================

int countDefeated(const Player &player)                                         // function countDefeated: counts how many guardians are defeated
{                                                                               // start of function countDefeated
    int total = 0;                                                              // creates total (int) starting as 0

    for (int i = 0; i < 4; i++)                                                 // repeats for i = 0 up to 4 - 1
    {                                                                           // start of for loop
        if (player.defeated[i])                                                 // checks whether player.defeated[i]
        {                                                                       // start of if block
            total++;                                                            // adds 1 to total
        }                                                                       // end of if block
    }                                                                           // end of for loop

    return total;                                                               // returns total
}                                                                               // end of function countDefeated

// 0-99 XP -> 0 (Beginner), 100-199 -> 1, 200-299 -> 2, 300+ -> 3.
int rankLevelOfXp(int xp)                                                       // function rankLevelOfXp: converts XP into a rank level number
{                                                                               // start of function rankLevelOfXp
    int level = xp / 100;                                                       // creates level (int) starting as xp / 100

    if (level > 3)                                                              // checks whether level > 3
    {                                                                           // start of if block
        level = 3;                                                              // sets level to 3
    }                                                                           // end of if block

    if (level < 0)                                                              // checks whether level < 0
    {                                                                           // start of if block
        level = 0;                                                              // sets level to 0
    }                                                                           // end of if block

    return level;                                                               // returns level
}                                                                               // end of function rankLevelOfXp

string rankNameOfLevel(int level)                                               // function rankNameOfLevel: converts a rank level into its name
{                                                                               // start of function rankNameOfLevel
    switch (level)                                                              // chooses what to do based on level
    {                                                                           // start of switch
        case 3:  return "Master";                                               // when the value is 3: returns the text "Master"
        case 2:  return "Scholar";                                              // when the value is 2: returns the text "Scholar"
        case 1:  return "Apprentice";                                           // when the value is 1: returns the text "Apprentice"
        default: return "Beginner";                                             // any other value ends up here
    }                                                                           // end of switch
}                                                                               // end of function rankNameOfLevel

// Rank always follows XP and can only go UP, so Apprentice (100 XP)
// is kept for good once earned, including after loading a save.
void updateRank(Player &player)                                                 // function updateRank: updates the player rank from the current XP
{                                                                               // start of function updateRank
    int oldLevel = 0;                                                           // creates oldLevel (int) starting as 0

    for (int level = 0; level <= 3; level++)                                    // level of the current rank
    {                                                                           // start of for loop
        if (rankNameOfLevel(level) == player.rank)                              // checks whether rankNameOfLevel(level) == player.rank
        {                                                                       // start of if block
            oldLevel = level;                                                   // sets oldLevel to level
        }                                                                       // end of if block
    }                                                                           // end of for loop

    int newLevel = rankLevelOfXp(player.xp);                                    // creates newLevel (int) starting as rankLevelOfXp(player.xp)

    if (newLevel < oldLevel)                                                    // never lose a rank
    {                                                                           // start of if block
        newLevel = oldLevel;                                                    // sets newLevel to oldLevel
    }                                                                           // end of if block

    player.rank = rankNameOfLevel(newLevel);                                    // sets player.rank to rankNameOfLevel(newLevel)

    if (newLevel > oldLevel)                                                    // checks whether newLevel > oldLevel
    {                                                                           // start of if block
        cout << "\n";                                                           // prints a blank line
        printLine('*');                                                         // draws a full-width line of '*'
        printCentered("RANK EVOLUTION!");                                       // shows centered: RANK EVOLUTION!
        printCentered(rankNameOfLevel(oldLevel) + "  --->  " + player.rank);    // shows centered: {rankNameOfLevel(oldLevel)} ---> {player.rank}
        printLine('*');                                                         // draws a full-width line of '*'
    }                                                                           // end of if block
}                                                                               // end of function updateRank


// ============================================================
// QUESTION BANK
// 20 guardian questions (5 English, 5 Math, 5 Science,
// 5 Computer) + 9 Headmaster questions + 1 final question
// = 30 questions in the whole game.
// ============================================================

static const array<array<Question, QUESTIONS_PER_GUARDIAN>, 4> &questionBank()  // function questionBank: returns the table of 5 questions for each of the 4 guardians
{                                                                               // start of function questionBank
    static const array<array<Question, QUESTIONS_PER_GUARDIAN>, 4> bank{{       // creates the question table (filled in below)
        // ENGLISH
        {{                                                                      // starts this subject's list of questions
            {                                                                   // starts one question entry
                "Which word means 'brave'?",                                    // question text: Which word means 'brave'?
                {{"Cowardly", "Courageous", "Silent", "Weak"}},                 // answer choices A-D: Cowardly / Courageous / Silent / Weak
                1,                                                              // correct answer is B (index 1)
                "Showing courage."                                              // hint shown by the Wisdom Scroll: Showing courage.
            },                                                                  // ends this question entry
            {                                                                   // starts one question entry
                "Choose the correct sentence.",                                 // question text: Choose the correct sentence.
                {{                                                              // starts the four answer choices
                    "She don't like books.",                                    // choice A: She don't like books.
                    "She doesn't likes books.",                                 // choice B: She doesn't likes books.
                    "She doesn't like books.",                                  // choice C: She doesn't like books.
                    "She not like books."                                       // choice D: She not like books.
                }},                                                             // ends the four answer choices
                2,                                                              // correct answer is C (index 2)
                "Use the base verb after doesn't."                              // hint shown by the Wisdom Scroll: Use the base verb after doesn't.
            },                                                                  // ends this question entry
            {                                                                   // starts one question entry
                "What is the opposite of 'ancient'?",                           // question text: What is the opposite of 'ancient'?
                {{"Old", "Historic", "Modern", "Broken"}},                      // answer choices A-D: Old / Historic / Modern / Broken
                2,                                                              // correct answer is C (index 2)
                "Think of something new."                                       // hint shown by the Wisdom Scroll: Think of something new.
            },                                                                  // ends this question entry
            {                                                                   // starts one question entry
                "Which is a noun?",                                             // question text: Which is a noun?
                {{"Quickly", "Beautiful", "Knowledge", "Carefully"}},           // answer choices A-D: Quickly / Beautiful / Knowledge / Carefully
                2,                                                              // correct answer is C (index 2)
                "A noun can name an idea."                                      // hint shown by the Wisdom Scroll: A noun can name an idea.
            },                                                                  // ends this question entry
            {                                                                   // starts one question entry
                "What does 'uncover' mean?",                                    // question text: What does 'uncover' mean?
                {{"Hide", "Reveal", "Destroy", "Forget"}},                      // answer choices A-D: Hide / Reveal / Destroy / Forget
                1,                                                              // correct answer is B (index 1)
                "Make something hidden known."                                  // hint shown by the Wisdom Scroll: Make something hidden known.
            }                                                                   // ends this question entry
        }},                                                                     // ends this subject's list of questions

        // MATHEMATICS
        {{                                                                      // starts this subject's list of questions
            {                                                                   // starts one question entry
                "What is 12 x 8?",                                              // question text: What is 12 x 8?
                {{"86", "92", "96", "108"}},                                    // answer choices A-D: 86 / 92 / 96 / 108
                2,                                                              // correct answer is C (index 2)
                "12 multiplied by 8."                                           // hint shown by the Wisdom Scroll: 12 multiplied by 8.
            },                                                                  // ends this question entry
            {                                                                   // starts one question entry
                "What is 45 + 37?",                                             // question text: What is 45 + 37?
                {{"72", "82", "92", "102"}},                                    // answer choices A-D: 72 / 82 / 92 / 102
                1,                                                              // correct answer is B (index 1)
                "Add the ones and tens."                                        // hint shown by the Wisdom Scroll: Add the ones and tens.
            },                                                                  // ends this question entry
            {                                                                   // starts one question entry
                "What is 144 / 12?",                                            // question text: What is 144 / 12?
                {{"10", "11", "12", "14"}},                                     // answer choices A-D: 10 / 11 / 12 / 14
                2,                                                              // correct answer is C (index 2)
                "12 times what equals 144?"                                     // hint shown by the Wisdom Scroll: 12 times what equals 144?
            },                                                                  // ends this question entry
            {                                                                   // starts one question entry
                "If x + 7 = 19, what is x?",                                    // question text: If x + 7 = 19, what is x?
                {{"10", "11", "12", "13"}},                                     // answer choices A-D: 10 / 11 / 12 / 13
                2,                                                              // correct answer is C (index 2)
                "Subtract 7."                                                   // hint shown by the Wisdom Scroll: Subtract 7.
            },                                                                  // ends this question entry
            {                                                                   // starts one question entry
                "What is 25% of 80?",                                           // question text: What is 25% of 80?
                {{"15", "20", "25", "30"}},                                     // answer choices A-D: 15 / 20 / 25 / 30
                1,                                                              // correct answer is B (index 1)
                "One quarter of 80."                                            // hint shown by the Wisdom Scroll: One quarter of 80.
            }                                                                   // ends this question entry
        }},                                                                     // ends this subject's list of questions

        // SCIENCE
        {{                                                                      // starts this subject's list of questions
            {                                                                   // starts one question entry
                "Which organ pumps blood?",                                     // question text: Which organ pumps blood?
                {{"Brain", "Lungs", "Heart", "Liver"}},                         // answer choices A-D: Brain / Lungs / Heart / Liver
                2,                                                              // correct answer is C (index 2)
                "It beats continuously."                                        // hint shown by the Wisdom Scroll: It beats continuously.
            },                                                                  // ends this question entry
            {                                                                   // starts one question entry
                "Which is the Red Planet?",                                     // question text: Which is the Red Planet?
                {{"Venus", "Mars", "Jupiter", "Mercury"}},                      // answer choices A-D: Venus / Mars / Jupiter / Mercury
                1,                                                              // correct answer is B (index 1)
                "Its soil contains iron oxide."                                 // hint shown by the Wisdom Scroll: Its soil contains iron oxide.
            },                                                                  // ends this question entry
            {                                                                   // starts one question entry
                "Water freezes at what Celsius temperature (standard pressure)?",  // question text: Water freezes at what Celsius temperature (standard pressure)?
                {{"0", "10", "32", "100"}},                                     // answer choices A-D: 0 / 10 / 32 / 100
                0,                                                              // correct answer is A (index 0)
                "The freezing point of water."                                  // hint shown by the Wisdom Scroll: The freezing point of water.
            },                                                                  // ends this question entry
            {                                                                   // starts one question entry
                "Plants absorb which gas for photosynthesis?",                  // question text: Plants absorb which gas for photosynthesis?
                {{"Oxygen", "Nitrogen", "Carbon dioxide", "Hydrogen"}},         // answer choices A-D: Oxygen / Nitrogen / Carbon dioxide / Hydrogen
                2,                                                              // correct answer is C (index 2)
                "Humans exhale it."                                             // hint shown by the Wisdom Scroll: Humans exhale it.
            },                                                                  // ends this question entry
            {                                                                   // starts one question entry
                "What pulls objects toward Earth?",                             // question text: What pulls objects toward Earth?
                {{"Magnetism", "Gravity", "Friction", "Electricity"}},          // answer choices A-D: Magnetism / Gravity / Friction / Electricity
                1,                                                              // correct answer is B (index 1)
                "It keeps your feet on the ground."                             // hint shown by the Wisdom Scroll: It keeps your feet on the ground.
            }                                                                   // ends this question entry
        }},                                                                     // ends this subject's list of questions

        // COMPUTER
        {{                                                                      // starts this subject's list of questions
            {                                                                   // starts one question entry
                "What ends most C++ statements?",                               // question text: What ends most C++ statements?
                {{":", ";", "#", ","}},                                         // answer choices A-D: : / ; / # / ,
                1,                                                              // correct answer is B (index 1)
                "A semicolon."                                                  // hint shown by the Wisdom Scroll: A semicolon.
            },                                                                  // ends this question entry
            {                                                                   // starts one question entry
                "Which C++ type stores whole numbers?",                         // question text: Which C++ type stores whole numbers?
                {{"int", "string", "bool", "char"}},                            // answer choices A-D: int / string / bool / char
                0,                                                              // correct answer is A (index 0)
                "Short for integer."                                            // hint shown by the Wisdom Scroll: Short for integer.
            },                                                                  // ends this question entry
            {                                                                   // starts one question entry
                "Which C++ statement makes a decision?",                        // question text: Which C++ statement makes a decision?
                {{"cout", "if", "include", "using"}},                           // answer choices A-D: cout / if / include / using
                1,                                                              // correct answer is B (index 1)
                "It checks a condition."                                        // hint shown by the Wisdom Scroll: It checks a condition.
            },                                                                  // ends this question entry
            {                                                                   // starts one question entry
                "What does && mean in C++?",                                    // question text: What does && mean in C++?
                {{"OR", "NOT", "AND", "EQUAL"}},                                // answer choices A-D: OR / NOT / AND / EQUAL
                2,                                                              // correct answer is C (index 2)
                "Both conditions must be true."                                 // hint shown by the Wisdom Scroll: Both conditions must be true.
            },                                                                  // ends this question entry
            {                                                                   // starts one question entry
                "What reads a full line with spaces?",                          // question text: What reads a full line with spaces?
                {{"getline", "cout", "sizeof", "return"}},                      // answer choices A-D: getline / cout / sizeof / return
                0,                                                              // correct answer is A (index 0)
                "You used it to enter your name."                               // hint shown by the Wisdom Scroll: You used it to enter your name.
            }                                                                   // ends this question entry
        }}                                                                      // ends this subject's list of questions
    }};                                                                         // ends the question table

    return bank;                                                                // returns bank
}                                                                               // end of function questionBank

// Nine different questions used only by the Headmaster.
static const array<Question, HEADMASTER_QUESTIONS> &headmasterBank()            // function headmasterBank: returns the table of the Headmaster questions
{                                                                               // start of function headmasterBank
    static const array<Question, HEADMASTER_QUESTIONS> bank{{                   // creates the question table (filled in below)
        {                                                                       // starts one question entry
            "Which word is a verb?",                                            // question text: Which word is a verb?
            {{"Table", "Run", "Green", "Under"}},                               // answer choices A-D: Table / Run / Green / Under
            1,                                                                  // correct answer is B (index 1)
            "A verb shows action."                                              // hint shown by the Wisdom Scroll: A verb shows action.
        },                                                                      // ends this question entry
        {                                                                       // starts one question entry
            "What is 15 + 27?",                                                 // question text: What is 15 + 27?
            {{"32", "42", "52", "41"}},                                         // answer choices A-D: 32 / 42 / 52 / 41
            1,                                                                  // correct answer is B (index 1)
            "Add the ones first: 5 + 7 = 12."                                   // hint shown by the Wisdom Scroll: Add the ones first: 5 + 7 = 12.
        },                                                                      // ends this question entry
        {                                                                       // starts one question entry
            "What is the center of an atom called?",                            // question text: What is the center of an atom called?
            {{"Electron", "Nucleus", "Orbit", "Cell"}},                         // answer choices A-D: Electron / Nucleus / Orbit / Cell
            1,                                                                  // correct answer is B (index 1)
            "Protons and neutrons live here."                                   // hint shown by the Wisdom Scroll: Protons and neutrons live here.
        },                                                                      // ends this question entry
        {                                                                       // starts one question entry
            "Which loop repeats while a condition is true?",                    // question text: Which loop repeats while a condition is true?
            {{"if", "switch", "while", "return"}},                              // answer choices A-D: if / switch / while / return
            2,                                                                  // correct answer is C (index 2)
            "Its name is also an English word for 'during the time that'."      // hint shown by the Wisdom Scroll: Its name is also an English word for 'during the time that'.
        },                                                                      // ends this question entry
        {                                                                       // starts one question entry
            "What is the plural of 'child'?",                                   // question text: What is the plural of 'child'?
            {{"Childs", "Childes", "Children", "Childrens"}},                   // answer choices A-D: Childs / Childes / Children / Childrens
            2,                                                                  // correct answer is C (index 2)
            "It is an irregular plural."                                        // hint shown by the Wisdom Scroll: It is an irregular plural.
        },                                                                      // ends this question entry
        {                                                                       // starts one question entry
            "What is the square root of 81?",                                   // question text: What is the square root of 81?
            {{"7", "8", "9", "10"}},                                            // answer choices A-D: 7 / 8 / 9 / 10
            2,                                                                  // correct answer is C (index 2)
            "What number times itself equals 81?"                               // hint shown by the Wisdom Scroll: What number times itself equals 81?
        },                                                                      // ends this question entry
        {                                                                       // starts one question entry
            "Which state of matter has a fixed shape?",                         // question text: Which state of matter has a fixed shape?
            {{"Solid", "Liquid", "Gas", "Plasma"}},                             // answer choices A-D: Solid / Liquid / Gas / Plasma
            0,                                                                  // correct answer is A (index 0)
            "Its particles are tightly packed."                                 // hint shown by the Wisdom Scroll: Its particles are tightly packed.
        },                                                                      // ends this question entry
        {                                                                       // starts one question entry
            "What does CPU stand for?",                                         // question text: What does CPU stand for?
            {{"Central Processing Unit", "Computer Personal Unit",              // starts the answer choices A-B: Central Processing Unit / Computer Personal Unit
              "Central Program Utility", "Control Processing User"}},           // answer choices C-D: Central Program Utility / Control Processing User
            0,                                                                  // correct answer is A (index 0)
            "It is the 'brain' of the computer."                                // hint shown by the Wisdom Scroll: It is the 'brain' of the computer.
        },                                                                      // ends this question entry
        {                                                                       // starts one question entry
            "Which word is an adjective?",                                      // question text: Which word is an adjective?
            {{"Slowly", "Bright", "Run", "Teacher"}},                           // answer choices A-D: Slowly / Bright / Run / Teacher
            1,                                                                  // correct answer is B (index 1)
            "An adjective describes a noun."                                    // hint shown by the Wisdom Scroll: An adjective describes a noun.
        }                                                                       // ends this question entry
    }};                                                                         // ends the question table

    return bank;                                                                // returns bank
}                                                                               // end of function headmasterBank

const Question &getQuestion(Subject subject, int number)                        // function getQuestion: picks a guardian question by subject and number
{                                                                               // start of function getQuestion
    int index = subject;                                                        // creates index (int) starting as subject

    if (subject == SUBJECT_NONE)                                                // safety net
    {                                                                           // start of if block
        index = SUBJECT_COMPUTER;                                               // sets index to SUBJECT_COMPUTER
    }                                                                           // end of if block

    return questionBank()[static_cast<size_t>(index)]                           // returns questionBank()[static_cast<size_t>(index)] (continues below)
                         [static_cast<size_t>(number % QUESTIONS_PER_GUARDIAN)];  // picks the item at this index
}                                                                               // end of function getQuestion

const Question &getHeadmasterQuestion(int number)                               // function getHeadmasterQuestion: picks a Headmaster question by number
{                                                                               // start of function getHeadmasterQuestion
    return headmasterBank()[static_cast<size_t>(number % HEADMASTER_QUESTIONS)];  // returns headmasterBank()[static_cast<size_t>(number % HEA...
}                                                                               // end of function getHeadmasterQuestion


// ============================================================
// ASK QUESTION
// Shows one question with up to MAX_ATTEMPTS tries.
//   Correct answer -> +XP, returns ANSWER_CORRECT
//   Wrong answer   -> -HP, that choice is crossed out, try again
//   No tries left  -> shows the answer, returns ANSWER_FAILED
//   Time Crystal   -> returns ANSWER_SKIPPED (no XP, no damage)
// ============================================================

AnswerResult askQuestion(Player &player, const Question &question,              // function askQuestion: asks one question with attempts, items and damage
                         bool finalQuestion, const string &label)               // continues the parameter list: bool finalQuestion, const string &label
{                                                                               // start of function askQuestion
    bool hidden[4] = { false, false, false, false };                            // choices removed
    string note[4];                                                             // why it was removed
    bool hintShown = false;                                                     // Wisdom Scroll used?
    int attemptsLeft = MAX_ATTEMPTS;                                            // tries remaining

    while (true)                                                                // loops forever until a return or break leaves it
    {                                                                           // start of while loop
        // ---------- draw the question ----------
        cout << "\n";                                                           // prints a blank line
        printLine('-');                                                         // draws a full-width line of '-'
        cout << " " << padRight(label, 38)                                      // shows: {padRight(label, 38)}
             << "Attempts left: " << attemptsLeft << "/" << MAX_ATTEMPTS << "\n";  // continues the output: Attempts left: {attemptsLeft}/{MAX_ATTEMPTS}
        printLine('-');                                                         // draws a full-width line of '-'

        cout << " " << question.text << "\n\n";                                 // shows: {question.text}

        for (int i = 0; i < 4; i++)                                             // repeats for i = 0 up to 4 - 1
        {                                                                       // start of for loop
            cout << "   " << static_cast<char>('A' + i) << ". ";                // shows: {static_cast<char>('A' + i)}.

            if (hidden[i])                                                      // checks whether hidden[i]
            {                                                                   // start of if block
                cout << "[" << note[i] << "]\n";                                // shows: [{note[i]}]
            }                                                                   // end of if block
            else                                                                // otherwise (none of the conditions above were true)
            {                                                                   // start of else block
                cout << question.options[static_cast<size_t>(i)] << "\n";       // shows: {question.options[static_cast<size_t>(i)]}
            }                                                                   // end of else block
        }                                                                       // end of for loop

        if (hintShown)                                                          // checks whether hintShown
        {                                                                       // start of if block
            cout << "\n HINT: " << question.hint << "\n";                       // shows: HINT: {question.hint}
        }                                                                       // end of if block

        printLine('-');                                                         // draws a full-width line of '-'

        if (finalQuestion)                                                      // checks whether finalQuestion
        {                                                                       // start of if block
            cout << "Answer A/B/C/D > ";                                        // shows: Answer A/B/C/D >
        }                                                                       // end of if block
        else                                                                    // otherwise (none of the conditions above were true)
        {                                                                       // start of else block
            cout << "Answer A/B/C/D or type ITEM > ";                           // shows: Answer A/B/C/D or type ITEM >
        }                                                                       // end of else block

        string answer;                                                          // creates the variable answer (string)

        if (!readCommandLine(answer))                                           // checks whether not readCommandLine(answer)
        {                                                                       // start of if block
            return ANSWER_ABORT;                                                // returns ANSWER_ABORT
        }                                                                       // end of if block


        // ---------- artifacts ----------
        if (answer == "item" && !finalQuestion)                                 // checks whether answer == "item" and not finalQuestion
        {                                                                       // start of if block
            showInventory(player);                                              // calls showInventory: lists the artifacts the player carries

            cout << "\nChoose an artifact:\n";                                  // shows: Choose an artifact:
            cout << "time crystal / rune / coin / scroll / cancel\n> ";         // shows: time crystal / rune / coin / scroll / cancel>

            string itemText;                                                    // creates the variable itemText (string)

            if (!readCommandLine(itemText))                                     // checks whether not readCommandLine(itemText)
            {                                                                   // start of if block
                return ANSWER_ABORT;                                            // returns ANSWER_ABORT
            }                                                                   // end of if block

            ItemChoice choice = parseItem(itemText);                            // creates choice (ItemChoice) starting as parseItem(itemText)

            if (choice == ITEM_CANCEL)                                          // checks whether choice == ITEM_CANCEL
            {                                                                   // start of if block
                cout << "\nYou decide not to use an artifact.\n";               // shows: You decide not to use an artifact.
                continue;                                                       // skips to the next round of the loop
            }                                                                   // end of if block

            if (choice == ITEM_UNKNOWN)                                         // checks whether choice == ITEM_UNKNOWN
            {                                                                   // start of if block
                cout << "\nUnknown artifact.\n";                                // shows: Unknown artifact.
                continue;                                                       // skips to the next round of the loop
            }                                                                   // end of if block

            if (!player.item[choice])                                           // checks whether not player.item[choice]
            {                                                                   // start of if block
                cout << "\nYou do not have that artifact.\n";                   // shows: You do not have that artifact.
                continue;                                                       // skips to the next round of the loop
            }                                                                   // end of if block

            switch (choice)                                                     // chooses what to do based on choice
            {                                                                   // start of switch
                case ITEM_TIME_CRYSTAL:                                         // when the value is ITEM_TIME_CRYSTAL
                    player.item[choice] = false;                                // sets player.item[choice] to false

                    cout << "\nThe Time Crystal begins to crack...\n\n";        // shows: The Time Crystal begins to crack...
                    cout << "TIME FREEZES.\n\n";                                // shows: TIME FREEZES.
                    cout << "[TIME CRYSTAL USED]\n";                            // shows: [TIME CRYSTAL USED]
                    cout << "The question was safely skipped.\n";               // shows: The question was safely skipped.
                    cout << "(No XP is awarded and the boss takes no damage.)\n";  // shows: (No XP is awarded and the boss takes no damage.)
                    return ANSWER_SKIPPED;                                      // returns ANSWER_SKIPPED

                case ITEM_RUNE:                                                 // when the value is ITEM_RUNE
                case ITEM_COIN:                                                 // when the value is ITEM_COIN
                {                                                               // start of case ITEM_COIN
                    int toRemove = (choice == ITEM_RUNE) ? 1 : 2;               // creates toRemove (int) starting as (choice == ITEM_RUNE) ? 1 : 2
                    int removed = 0;                                            // creates removed (int) starting as 0

                    for (int i = 0; i < 4 && removed < toRemove; i++)           // repeats for i = 0 up to 4 && removed < toRemove - 1
                    {                                                           // start of for loop
                        if (i != question.answer && !hidden[i])                 // checks whether i != question.answer and not hidden[i]
                        {                                                       // start of if block
                            hidden[i] = true;                                   // sets hidden[i] to true
                            note[i] = "ELIMINATED";                             // sets note[i] to the text "ELIMINATED"
                            removed++;                                          // adds 1 to removed
                        }                                                       // end of if block
                    }                                                           // end of for loop

                    if (removed == 0)                                           // checks whether removed == 0
                    {                                                           // start of if block
                        cout << "\nAll wrong choices are already removed.\n";   // shows: All wrong choices are already removed.
                        cout << "The artifact was not used.\n";                 // shows: The artifact was not used.
                        continue;                                               // skips to the next round of the loop
                    }                                                           // end of if block

                    player.item[choice] = false;                                // sets player.item[choice] to false

                    if (choice == ITEM_RUNE)                                    // checks whether choice == ITEM_RUNE
                    {                                                           // start of if block
                        cout << "\nThe Rune of Elimination begins to glow.\n";  // shows: The Rune of Elimination begins to glow.
                        cout << "One incorrect choice has been eliminated!\n";  // shows: One incorrect choice has been eliminated!
                    }                                                           // end of if block
                    else                                                        // otherwise (none of the conditions above were true)
                    {                                                           // start of else block
                        cout << "\nThe Oracle's Coin spins in the air...\n";    // shows: The Oracle's Coin spins in the air...
                        cout << "50/50 ACTIVATED! Wrong choices removed.\n";    // shows: 50/50 ACTIVATED! Wrong choices removed.
                    }                                                           // end of else block
                    continue;                                                   // skips to the next round of the loop
                }                                                               // end of case ITEM_COIN

                case ITEM_SCROLL:                                               // when the value is ITEM_SCROLL
                    if (hintShown)                                              // checks whether hintShown
                    {                                                           // start of if block
                        cout << "\nThe hint is already visible.\n";             // shows: The hint is already visible.
                        continue;                                               // skips to the next round of the loop
                    }                                                           // end of if block

                    player.item[choice] = false;                                // sets player.item[choice] to false
                    hintShown = true;                                           // sets hintShown to true

                    cout << "\nThe Wisdom Scroll opens by itself...\n";         // shows: The Wisdom Scroll opens by itself...
                    continue;                                                   // skips to the next round of the loop

                default:                                                        // any other value ends up here
                    continue;                                                   // skips to the next round of the loop
            }                                                                   // end of switch
        }                                                                       // end of if block


        // ---------- normal answer ----------
        if (answer.size() != 1 || answer[0] < 'a' || answer[0] > 'd')           // checks whether answer.size() != 1 or answer[0] < 'a' or answer[0] > 'd'
        {                                                                       // start of if block
            cout << "\nInvalid answer.\n";                                      // shows: Invalid answer.
            cout << "Please enter A, B, C or D";                                // shows: Please enter A, B, C or D

            if (!finalQuestion)                                                 // checks whether not finalQuestion
            {                                                                   // start of if block
                cout << ", or ITEM";                                            // shows: , or ITEM
            }                                                                   // end of if block

            cout << ".\n";                                                      // shows: .
            continue;                                                           // skips to the next round of the loop
        }                                                                       // end of if block

        int selected = answer[0] - 'a';                                         // creates selected (int) starting as answer[0] - 'a'

        if (hidden[selected])                                                   // checks whether hidden[selected]
        {                                                                       // start of if block
            cout << "\nThat choice is crossed out. Choose another.\n";          // shows: That choice is crossed out. Choose another.
            continue;                                                           // skips to the next round of the loop
        }                                                                       // end of if block

        if (selected == question.answer)                                        // ---- correct ----
        {                                                                       // start of if block
            player.correct++;                                                   // adds 1 to player.correct
            player.xp += CORRECT_XP;                                            // adds CORRECT_XP to player.xp

            cout << "\nCORRECT!\n";                                             // shows: CORRECT!
            cout << "+" << CORRECT_XP << " KNOWLEDGE XP\n";                     // shows: +{CORRECT_XP} KNOWLEDGE XP

            if (attemptsLeft < MAX_ATTEMPTS)                                    // checks whether attemptsLeft < MAX_ATTEMPTS
            {                                                                   // start of if block
                cout << "(Solved on attempt " << (MAX_ATTEMPTS - attemptsLeft + 1)  // shows: (Solved on attempt {(MAX_ATTEMPTS - attemptsLeft + 1)}
                     << " of " << MAX_ATTEMPTS << ")\n";                        // continues the output: of {MAX_ATTEMPTS})
            }                                                                   // end of if block

            updateRank(player);                                                 // rank follows XP

            return ANSWER_CORRECT;                                              // returns ANSWER_CORRECT
        }                                                                       // end of if block

        // ---- wrong ----
        player.incorrect++;                                                     // adds 1 to player.incorrect
        player.hp -= WRONG_ANSWER_HP_LOSS;                                      // subtracts WRONG_ANSWER_HP_LOSS from player.hp

        if (player.hp < 0)                                                      // checks whether HP went below zero
        {                                                                       // start of if block
            player.hp = 0;                                                      // sets player.hp to 0
        }                                                                       // end of if block

        hidden[selected] = true;                                                // sets hidden[selected] to true
        note[selected] = "WRONG";                                               // sets note[selected] to the text "WRONG"
        attemptsLeft--;                                                         // subtracts 1 from attemptsLeft

        cout << "\nWRONG ANSWER!\n";                                            // shows: WRONG ANSWER!
        cout << "-" << WRONG_ANSWER_HP_LOSS << " HP\n";                         // shows: -{WRONG_ANSWER_HP_LOSS} HP
        cout << "Your HP: " << makeBar(player.hp, PLAYER_MAX_HP, 20) << " "     // shows: Your HP: {makeBar(player.hp, PLAYER_MAX_HP, 20)}
             << player.hp << "/" << PLAYER_MAX_HP << "\n";                      // continues the output: {player.hp}/{PLAYER_MAX_HP}

        if (player.hp <= 0)                                                     // checks whether the player has no HP left
        {                                                                       // start of if block
            return ANSWER_FAILED;                                               // defeated
        }                                                                       // end of if block

        if (attemptsLeft <= 0)                                                  // checks whether attemptsLeft <= 0
        {                                                                       // start of if block
            cout << "\nNo attempts left.\n";                                    // shows: No attempts left.
            cout << "The correct answer was "                                   // shows: The correct answer was
                 << static_cast<char>('A' + question.answer) << ". "            // continues the output: {static_cast<char>('A' + question.answer)}.
                 << question.options[static_cast<size_t>(question.answer)] << "\n";  // continues the output: {question.options[static_cast<size_t>(question.answer)]}

            return ANSWER_FAILED;                                               // returns ANSWER_FAILED
        }                                                                       // end of if block

        cout << "\nTry again! You have " << attemptsLeft                        // shows: Try again! You have {attemptsLeft}
             << (attemptsLeft == 1 ? " attempt" : " attempts") << " left.\n";   // continues the output: attempt attempts left.
    }                                                                           // end of while loop
}                                                                               // end of function askQuestion


// ============================================================
// SUBJECT FOR THE CURRENT ROOM
// ============================================================

Subject getSubjectForRoom(const Player &player)                                 // function getSubjectForRoom: returns the guardian subject of the current room
{                                                                               // start of function getSubjectForRoom
    return subjectOfRoom(player.currentRoom);                                   // returns subjectOfRoom(player.currentRoom)
}                                                                               // end of function getSubjectForRoom


// ============================================================
// JOURNAL PAGES (found automatically when a guardian falls)
// ============================================================

void discoverJournal(int page)                                                  // function discoverJournal: shows one journal page found after a guardian falls
{                                                                               // start of function discoverJournal
    cout << "\n";                                                               // prints a blank line
    printBanner("JOURNAL PAGE FOUND");                                          // shows a banner titled: JOURNAL PAGE FOUND
    cout << "\n";                                                               // prints a blank line

    switch (page)                                                               // chooses what to do based on page
    {                                                                           // start of switch
        case 1:                                                                 // when the value is 1
            cout << "\"The guardians were not always monsters.\n";              // shows: "The guardians were not always monsters.
            cout << "They were once Blackwood Academy's greatest teachers.\"\n";  // shows: They were once Blackwood Academy's greatest teachers."
            break;                                                              // leaves the switch/loop

        case 2:                                                                 // when the value is 2
            cout << "\"The Headmaster created something called\n";              // shows: "The Headmaster created something called
            cout << "The Last Trial. He believed ordinary examinations\n";      // shows: The Last Trial. He believed ordinary examinations
            cout << "could never reveal a student's true potential.\"\n";       // shows: could never reveal a student's true potential."
            break;                                                              // leaves the switch/loop

        case 3:                                                                 // when the value is 3
            cout << "\"Fear. Uncertainty. Limited resources.\n";                // shows: "Fear. Uncertainty. Limited resources.
            cout << "The trial was designed to discover who could continue\n";  // shows: The trial was designed to discover who could continue
            cout << "learning even when everything became difficult.\"\n";      // shows: learning even when everything became difficult."
            break;                                                              // leaves the switch/loop

        case 4:                                                                 // when the value is 4
            cout << "\"Something has gone wrong.\n";                            // shows: "Something has gone wrong.
            cout << "The trial will not stop.\n";                               // shows: The trial will not stop.
            cout << "The academy has sealed itself.\n";                         // shows: The academy has sealed itself.
            cout << "We cannot get out.\"\n";                                   // shows: We cannot get out."
            break;                                                              // leaves the switch/loop

        default:                                                                // any other value ends up here
            break;                                                              // leaves the switch/loop
    }                                                                           // end of switch

    cout << "\n";                                                               // prints a blank line
    printLine('=');                                                             // draws a full-width line of '='
}                                                                               // end of function discoverJournal


// ============================================================
// GUARDIAN BATTLE
// Returns false only if the input stream was closed.
// ============================================================

// "QUESTION 2 of 5", or "EXTRA QUESTION 6" after failed questions return.
static string questionLabel(const string &prefix, int number, int total)        // function questionLabel: builds the "QUESTION n of N" label text
{                                                                               // start of function questionLabel
    if (number <= total)                                                        // checks whether number <= total
    {                                                                           // start of if block
        return prefix + "QUESTION " + to_string(number)                         // returns prefix + "QUESTION " + to_string(number) (continues below)
               + " of " + to_string(total);                                     // joins on: of {to_string(total)}
    }                                                                           // end of if block

    return prefix + "RETURNING QUESTION (#" + to_string(number) + ")";          // returns prefix + "RETURNING QUESTION (#" + to_string(numb...
}                                                                               // end of function questionLabel

// Player name shortened so the HP rows stay aligned.
static string shortName(const string &name)                                     // function shortName: shortens the player name so HP rows stay aligned
{                                                                               // start of function shortName
    return (name.size() > 14) ? name.substr(0, 14) : name;                      // returns (name.size() > 14) ? name.substr(0, 14) : name
}                                                                               // end of function shortName

bool guardianBattle(Player &player, Subject subject)                            // function guardianBattle: runs the fight against the guardian of this room
{                                                                               // start of function guardianBattle
    if (subject == SUBJECT_NONE)                                                // checks whether subject == SUBJECT_NONE
    {                                                                           // start of if block
        cout << "\nThere is no guardian in this room.\n";                       // shows: There is no guardian in this room.
        return true;                                                            // returns true
    }                                                                           // end of if block

    if (player.defeated[subject])                                               // already beaten
    {                                                                           // start of if block
        cout << "\n";                                                           // prints a blank line
        printLine('=');                                                         // draws a full-width line of '='
        printCentered("[ DEFEATED ]");                                          // shows centered: [ DEFEATED ]
        printCentered(BOSS_NAMES[subject] + " has already been defeated.");     // shows centered: {BOSS_NAMES[subject]} has already been defeated.
        printCentered(SEAL_NAMES[subject] + " is yours.");                      // shows centered: {SEAL_NAMES[subject]} is yours.
        printLine('=');                                                         // draws a full-width line of '='
        return true;                                                            // returns true
    }                                                                           // end of if block

    cout << "\n";                                                               // prints a blank line
    printBanner("GUARDIAN ENCOUNTER");                                          // shows a banner titled: GUARDIAN ENCOUNTER
    cout << "\n";                                                               // prints a blank line

    switch (subject)                                                            // chooses what to do based on subject
    {                                                                           // start of switch
        case SUBJECT_ENGLISH:                                                   // when the value is SUBJECT_ENGLISH
            cout << "Every book in the Library suddenly opens.\n";              // shows: Every book in the Library suddenly opens.
            cout << "Thousands of pages begin turning by themselves.\n";        // shows: Thousands of pages begin turning by themselves.
            cout << "A tall figure emerges from between the shelves.\n\n";      // shows: A tall figure emerges from between the shelves.
            cout << "\"Words created this academy.\"\n";                        // shows: "Words created this academy."
            cout << "\"Let us see if yours can save you.\"\n";                  // shows: "Let us see if yours can save you."
            break;                                                              // leaves the switch/loop

        case SUBJECT_MATHEMATICS:                                               // when the value is SUBJECT_MATHEMATICS
            cout << "Every clock suddenly stops.\n";                            // shows: Every clock suddenly stops.
            cout << "Numbers pour from the walls like falling rain.\n";         // shows: Numbers pour from the walls like falling rain.
            cout << "A figure made of shifting equations appears.\n\n";         // shows: A figure made of shifting equations appears.
            cout << "\"Everything has an answer...\"\n";                        // shows: "Everything has an answer..."
            cout << "\"Including how long you have left.\"\n";                  // shows: "Including how long you have left."
            break;                                                              // leaves the switch/loop

        case SUBJECT_SCIENCE:                                                   // when the value is SUBJECT_SCIENCE
            cout << "The glass chambers begin shaking.\n\n";                    // shows: The glass chambers begin shaking.
            cout << "CRASH!\n\n";                                               // shows: CRASH!
            cout << "Dr. Vex steps through a cloud of green smoke.\n\n";        // shows: Dr. Vex steps through a cloud of green smoke.
            cout << "\"Knowledge without experimentation is useless.\"\n";      // shows: "Knowledge without experimentation is useless."
            cout << "\"Tonight... you are the experiment.\"\n";                 // shows: "Tonight... you are the experiment."
            break;                                                              // leaves the switch/loop

        case SUBJECT_COMPUTER:                                                  // when the value is SUBJECT_COMPUTER
        default:                                                                // any other value ends up here
            cout << "Every monitor begins flashing red.\n\n";                   // shows: Every monitor begins flashing red.
            cout << "WARNING.\nWARNING.\nWARNING.\n\n";                         // shows: WARNING.WARNING.WARNING.
            cout << "A distorted face appears on every screen.\n\n";            // shows: A distorted face appears on every screen.
            cout << "\"HUMAN KNOWLEDGE DETECTED.\"\n";                          // shows: "HUMAN KNOWLEDGE DETECTED."
            cout << "\"TESTING REQUIRED.\"\n";                                  // shows: "TESTING REQUIRED."
            break;                                                              // leaves the switch/loop
    }                                                                           // end of switch

    cout << "\n";                                                               // prints a blank line
    showBossFigure(subject);                                                    // the boss figure
    cout << "\n";                                                               // prints a blank line
    printBanner(BOSS_NAMES[subject]);                                           // shows a banner titled: {BOSS_NAMES[subject]}
    printCentered("Defeat it with " + to_string(QUESTIONS_PER_GUARDIAN)         // shows centered: Defeat it with {to_string(QUESTIONS_PER_GUARDIAN}
                  + " correct answers.");                                       // joins on: correct answers.

    int bossHP = GUARDIAN_MAX_HP;                                               // creates bossHP (int) starting as GUARDIAN_MAX_HP
    int questionNumber = 0;                                                     // creates questionNumber (int) starting as 0

    while (bossHP > 0 && player.hp > 0)                                         // keeps repeating while bossHP > 0 and player.hp > 0
    {                                                                           // start of while loop
        cout << "\n";                                                           // prints a blank line
        cout << " " << padRight(BOSS_NAMES[subject], 15)                        // shows: {padRight(BOSS_NAMES[subject], 15)}
             << makeBar(bossHP, GUARDIAN_MAX_HP, 20) << " "                     // continues the output: {makeBar(bossHP, GUARDIAN_MAX_HP, 20)}
             << bossHP << "/" << GUARDIAN_MAX_HP << "\n";                       // continues the output: {bossHP}/{GUARDIAN_MAX_HP}
        cout << " " << padRight(shortName(player.name), 15)                     // shows: {padRight(shortName(player.name), 15)}
             << makeBar(player.hp, PLAYER_MAX_HP, 20) << " "                    // continues the output: {makeBar(player.hp, PLAYER_MAX_HP, 20)}
             << player.hp << "/" << PLAYER_MAX_HP << "\n";                      // continues the output: {player.hp}/{PLAYER_MAX_HP}

        AnswerResult result = askQuestion(                                      // creates result (AnswerResult) from the result of askQuestion (arguments listed below)
            player,                                                             // argument / continuation: {player}
            getQuestion(subject, questionNumber),                               // argument / continuation: {getQuestion(subject, questionNumber)}
            false,                                                              // argument / continuation: {false}
            questionLabel("", questionNumber + 1, QUESTIONS_PER_GUARDIAN));     // argument / continuation: {questionLabel("", questionNumber + 1, QUESTIONS_PER_GUARDIAN)}

        questionNumber++;                                                       // adds 1 to questionNumber

        if (result == ANSWER_ABORT)                                             // checks whether result == ANSWER_ABORT
        {                                                                       // start of if block
            return false;                                                       // returns false
        }                                                                       // end of if block

        if (result == ANSWER_CORRECT)                                           // checks whether result == ANSWER_CORRECT
        {                                                                       // start of if block
            bossHP -= BOSS_DAMAGE;                                              // subtracts BOSS_DAMAGE from bossHP

            if (bossHP < 0)                                                     // checks whether bossHP < 0
            {                                                                   // start of if block
                bossHP = 0;                                                     // sets bossHP to 0
            }                                                                   // end of if block

            cout << "\nYour knowledge strikes the guardian!\n";                 // shows: Your knowledge strikes the guardian!
            cout << BOSS_NAMES[subject] << " takes " << BOSS_DAMAGE             // shows: {BOSS_NAMES[subject]} takes {BOSS_DAMAGE}
                 << " damage.\n";                                               // continues the output: damage.
        }                                                                       // end of if block
        else if (result == ANSWER_SKIPPED)                                      // otherwise, checks whether result == ANSWER_SKIPPED
        {                                                                       // start of else-if block
            cout << "\nQuestion skipped. The guardian takes no damage.\n";      // shows: Question skipped. The guardian takes no damage.
        }                                                                       // end of else-if block
        else if (player.hp > 0)                                                 // failed all attempts
        {                                                                       // start of else-if block
            cout << "\n" << BOSS_NAMES[subject]                                 // shows: {BOSS_NAMES[subject]}
                 << " resists! That question will return later.\n";             // continues the output: resists! That question will return later.
        }                                                                       // end of else-if block
    }                                                                           // end of while loop

    if (player.hp <= 0)                                                         // checks whether the player has no HP left
    {                                                                           // start of if block
        return true;                                                            // gameLoop shows Game Over
    }                                                                           // end of if block

    cout << "\n";                                                               // prints a blank line
    printBanner(BOSS_NAMES[subject] + " DEFEATED!");                            // shows a banner titled: {BOSS_NAMES[subject]} DEFEATED!
    cout << "\n";                                                               // prints a blank line

    player.defeated[subject] = true;                                            // [ DEFEATED ]

    switch (subject)                                                            // chooses what to do based on subject
    {                                                                           // start of switch
        case SUBJECT_ENGLISH:                                                   // when the value is SUBJECT_ENGLISH
            player.item[ITEM_SCROLL] = true;                                    // sets player.item[ITEM_SCROLL] to true
            discoverJournal(1);                                                 // calls discoverJournal: shows one journal page found after a guardian falls
            cout << "\n[" << SEAL_NAMES[subject] << " COLLECTED]\n";            // shows: [{SEAL_NAMES[subject]} COLLECTED]
            cout << "[WISDOM SCROLL ACQUIRED]\n";                               // shows: [WISDOM SCROLL ACQUIRED]
            break;                                                              // leaves the switch/loop

        case SUBJECT_MATHEMATICS:                                               // when the value is SUBJECT_MATHEMATICS
            player.item[ITEM_RUNE] = true;                                      // sets player.item[ITEM_RUNE] to true
            discoverJournal(2);                                                 // calls discoverJournal: shows one journal page found after a guardian falls
            cout << "\n[" << SEAL_NAMES[subject] << " COLLECTED]\n";            // shows: [{SEAL_NAMES[subject]} COLLECTED]
            cout << "[RUNE OF ELIMINATION ACQUIRED]\n";                         // shows: [RUNE OF ELIMINATION ACQUIRED]
            break;                                                              // leaves the switch/loop

        case SUBJECT_SCIENCE:                                                   // when the value is SUBJECT_SCIENCE
            player.item[ITEM_TIME_CRYSTAL] = true;                              // sets player.item[ITEM_TIME_CRYSTAL] to true
            discoverJournal(3);                                                 // calls discoverJournal: shows one journal page found after a guardian falls
            cout << "\n[" << SEAL_NAMES[subject] << " COLLECTED]\n";            // shows: [{SEAL_NAMES[subject]} COLLECTED]
            cout << "[TIME CRYSTAL ACQUIRED]\n";                                // shows: [TIME CRYSTAL ACQUIRED]
            break;                                                              // leaves the switch/loop

        case SUBJECT_COMPUTER:                                                  // when the value is SUBJECT_COMPUTER
        default:                                                                // any other value ends up here
            discoverJournal(4);                                                 // calls discoverJournal: shows one journal page found after a guardian falls
            cout << "\n[" << SEAL_NAMES[subject] << " COLLECTED]\n";            // shows: [{SEAL_NAMES[subject]} COLLECTED]

            if (!player.chestOpened)                                            // chest was skipped
            {                                                                   // start of if block
                player.item[ITEM_COIN] = true;                                  // sets player.item[ITEM_COIN] to true
                cout << "[ORACLE'S COIN ACQUIRED]\n";                           // shows: [ORACLE'S COIN ACQUIRED]
            }                                                                   // end of if block
            break;                                                              // leaves the switch/loop
    }                                                                           // end of switch

    cout << "\nGUARDIANS DEFEATED: " << countDefeated(player) << "/4  "         // shows: GUARDIANS DEFEATED: {countDefeated(player)}/4
         << makeBar(countDefeated(player), 4, 12) << "\n";                      // continues the output: {makeBar(countDefeated(player), 4, 12)}

    if (countDefeated(player) == 4)                                             // checks whether countDefeated(player) == 4
    {                                                                           // start of if block
        cout << "\nSuddenly...\n\n";                                            // shows: Suddenly...
        cout << "The entire academy begins to shake.\n";                        // shows: The entire academy begins to shake.
        cout << "The four Seals of Knowledge glow at once.\n\n";                // shows: The four Seals of Knowledge glow at once.
        cout << "DING... DONG...\n\n";                                          // shows: DING... DONG...
        cout << "A voice echoes through every hallway:\n\n";                    // shows: A voice echoes through every hallway:
        cout << "\"THE FINAL EXAMINATION IS READY.\"\n\n";                      // shows: "THE FINAL EXAMINATION IS READY."
        cout << "The Sealed Chamber is now UNLOCKED.\n";                        // shows: The Sealed Chamber is now UNLOCKED.
        cout << "Travel to the Server Room and go SOUTH.\n";                    // shows: Travel to the Server Room and go SOUTH.
    }                                                                           // end of if block

    return true;                                                                // returns true
}                                                                               // end of function guardianBattle


// ============================================================
// FINAL BOSS: THE HEADMASTER
// ============================================================

void headmasterBattle(Player &player)                                           // final boss fight; calls victoryEnding when won
{                                                                               // start of block
    cout << "\nThe four seals burn with white light.\n\n";                      // shows: The four seals burn with white light.
    cout << "The Sealed Chamber closes behind you.\n";                          // shows: The Sealed Chamber closes behind you.
    cout << "A hidden staircase descends beneath the academy.\n\n";             // shows: A hidden staircase descends beneath the academy.
    cout << "You walk down...\n";                                               // shows: You walk down...
    cout << "Step after step...\n\n";                                           // shows: Step after step...
    cout << "Until you reach a room that should not exist.\n\n";                // shows: Until you reach a room that should not exist.
    cout << "A single man sits behind an enormous wooden desk.\n";              // shows: A single man sits behind an enormous wooden desk.
    cout << "He slowly raises his head.\n\n";                                   // shows: He slowly raises his head.

    showHeadmasterFigure();                                                     // boss figure

    cout << "\n";                                                               // prints a blank line
    printLine('=');                                                             // draw a divider line
    printCentered("THE HEADMASTER");                                            // shows centered: THE HEADMASTER
    printCentered("FINAL EXAMINER");                                            // shows centered: FINAL EXAMINER
    printCentered("HP: " + to_string(HEADMASTER_MAX_HP));                       // shows centered: HP:
    printLine('=');                                                             // draw a divider line

    cout << "\n\"English. Mathematics. Science. Technology.\"\n\n";             // shows: "English. Mathematics. Science. Technology."
    cout << "\"You defeated every teacher I placed before you.\"\n\n";          // shows: "You defeated every teacher I placed before you."
    cout << "\"But knowing facts does not make you knowledgeable.\"\n\n";       // shows: "But knowing facts does not make you knowledgeable."
    cout << "\"Let us begin your final examination.\"\n";                       // shows: "Let us begin your final examination."

    int headmasterHP = HEADMASTER_MAX_HP;                                       // boss starts at full HP
    int questionNumber = 0;                                                     // which Headmaster question is next

    bool phaseTwoShown = false;                                                 // has the phase II scene been shown?
    bool phaseThreeShown = false;                                               // has the phase III scene been shown?

    while (headmasterHP > HEADMASTER_FINAL_HP && player.hp > 0)                 // fight until the boss is at 20 HP or the player dies
    {                                                                           // start of block
        if (headmasterHP <= 100 && !phaseTwoShown)                              // ---- PHASE II ----
        {                                                                       // start of block
            phaseTwoShown = true;                                               // sets phaseTwoShown to true

            cout << "\n";                                                       // prints a blank line
            printBanner("PHASE II - NO MERCY");                                 // print a banner
            cout << "\nThe lights turn blood-red.\n\n";                         // shows: The lights turn blood-red.
            cout << "The Headmaster rises from his chair.\n\n";                 // shows: The Headmaster rises from his chair.
            cout << "\"Enough warm-up questions.\"\n";                          // shows: "Enough warm-up questions."
            cout << "\"Now show me what you truly know.\"\n";                   // shows: "Now show me what you truly know."
        }                                                                       // end of block

        if (headmasterHP <= 50 && !phaseThreeShown)                             // ---- PHASE III ----
        {                                                                       // start of block
            phaseThreeShown = true;                                             // sets phaseThreeShown to true

            cout << "\n";                                                       // prints a blank line
            printBanner("PHASE III - THE FINAL TRIAL");                         // print a banner
            cout << "\nYour head suddenly begins to hurt.\n\n";                 // shows: Your head suddenly begins to hurt.
            cout << "Memories return.\n\n";                                     // shows: Memories return.
            cout << "You remember reading about Blackwood Academy.\n";          // shows: You remember reading about Blackwood Academy.
            cout << "You remember the legend of The Last Trial.\n\n";           // shows: You remember the legend of The Last Trial.
            cout << "And then...\n\n";                                          // shows: And then...
            cout << "You remember the truth.\n\n";                              // shows: You remember the truth.
            cout << "You were never kidnapped.\n";                              // shows: You were never kidnapped.
            cout << "You entered Blackwood Academy willingly.\n\n";             // shows: You entered Blackwood Academy willingly.
            cout << "You wanted to prove that you could complete\n";            // shows: You wanted to prove that you could complete
            cout << "the challenge nobody else had finished.\n\n";              // shows: the challenge nobody else had finished.
            cout << "The Headmaster smiles.\n\n";                               // shows: The Headmaster smiles.
            cout << "\"You asked for the greatest test Blackwood\n";            // shows: "You asked for the greatest test Blackwood
            cout << "could offer.\"\n\n";                                       // shows: could offer."
            cout << "\"I simply gave it to you.\"\n";                           // shows: "I simply gave it to you."
        }                                                                       // end of block

        cout << "\n";                                                           // prints a blank line
        cout << " " << padRight("HEADMASTER", 15)                               // shows: HEADMASTER
             << makeBar(headmasterHP, HEADMASTER_MAX_HP, 20) << " "             // continues the output: {makeBar(headmasterHP, HEADMASTER_MAX_HP, 20)}
             << headmasterHP << "/" << HEADMASTER_MAX_HP << "\n";               // continues the output: {headmasterHP}/{HEADMASTER_MAX_HP}
        cout << " " << padRight(shortName(player.name), 15)                     // prints a blank line
             << makeBar(player.hp, PLAYER_MAX_HP, 20) << " "                    // continues the output: {makeBar(player.hp, PLAYER_MAX_HP, 20)}
             << player.hp << "/" << PLAYER_MAX_HP << "\n";                      // continues the output: {player.hp}/{PLAYER_MAX_HP}

        AnswerResult result = askQuestion(                                      // ask a question and get the outcome
            player,                                                             // argument / continuation: {player}
            getHeadmasterQuestion(questionNumber),                              // argument / continuation: {getHeadmasterQuestion(questionNumber)}
            false,                                                              // argument / continuation: {false}
            questionLabel("HEADMASTER ", questionNumber + 1,                    // argument / continuation: {questionLabel("HEADMASTER ", questionNumber + 1}
                          HEADMASTER_QUESTIONS));                               // argument / continuation: {HEADMASTER_QUESTIONS}

        questionNumber++;                                                       // move to the next question

        if (result == ANSWER_ABORT)                                             // input closed during the question
        {                                                                       // start of block
            return;                                                             // leave this function (after victory this ends the game loop)
        }                                                                       // end of block

        if (result == ANSWER_CORRECT)                                           // answered correctly
        {                                                                       // start of block
            headmasterHP -= BOSS_DAMAGE;                                        // boss takes damage

            if (headmasterHP < HEADMASTER_FINAL_HP)                             // would the boss drop below 20 HP?
            {                                                                   // start of block
                headmasterHP = HEADMASTER_FINAL_HP;                             // keep 20 HP for the end
            }                                                                   // end of block

            cout << "\nThe Headmaster staggers backward.\n";                    // shows: The Headmaster staggers backward.
            cout << "HEADMASTER -" << BOSS_DAMAGE << " HP\n";                   // shows: HEADMASTER - HP
        }                                                                       // end of block
        else if (result == ANSWER_SKIPPED)                                      // Time Crystal used
        {                                                                       // start of block
            cout << "\nQuestion skipped. The Headmaster takes no damage.\n";    // shows: Question skipped. The Headmaster takes no damage.
        }                                                                       // end of block
        else if (player.hp > 0)                                                 // wrong answer but still alive
        {                                                                       // start of block
            cout << "\nThe Headmaster shakes his head.\n";                      // shows: The Headmaster shakes his head.
            cout << "\"Not enough.\" That question will return later.\n";       // shows: "Not enough." That question will return later.
        }                                                                       // end of block
    }                                                                           // end of block

    if (player.hp <= 0)                                                         // is the player dead?
    {                                                                           // start of block
        return;                                                                 // gameLoop handles restart
    }                                                                           // end of block

    // ---------------- FINAL QUESTION ----------------
    cout << "\n";                                                               // prints a blank line
    printLine('=');                                                             // draw a divider line
    printCentered("HEADMASTER HP: " + to_string(HEADMASTER_FINAL_HP) + "/"      // shows centered: HEADMASTER HP: 20/200 (continues next line)
                  + to_string(HEADMASTER_MAX_HP));                              // joins on: {to_string(HEADMASTER_MAX_HP)}
    cout << "\n";                                                               // prints a blank line
    printCentered("FINAL PHASE ACTIVATED");                                     // shows centered: FINAL PHASE ACTIVATED
    printCentered("NO ITEMS ALLOWED");                                          // shows centered: NO ITEMS ALLOWED
    printCentered("FINAL QUESTION");                                            // shows centered: FINAL QUESTION
    printLine('=');                                                             // draw a divider line

    for (int i = 0; i < 4; i++)                                                 // loop over the four artifacts
    {                                                                           // start of block
        player.item[i] = false;                                                 // artifacts disappear
    }                                                                           // end of block

    Question finalQuestion;                                                     // the last question
    finalQuestion.text = "What does true knowledge require?";                   // question text
    finalQuestion.options = {{                                                  // answer options
        "Never making a mistake",                                               // answer options
        "Memorizing every fact",                                                // answer options
        "Continuing to learn even when you do not know",                        // answer options
        "Avoiding difficult questions"                                          // answer options
    }};                                                                         // answer options
    finalQuestion.answer = 2;                                                   // correct answer is C
    finalQuestion.hint = "";                                                    // no hint allowed

    while (player.hp > 0)                                                       // repeat the final question until solved or dead
    {                                                                           // start of block
        AnswerResult result = askQuestion(                                      // ask a question and get the outcome
            player, finalQuestion, true,                                        // argument / continuation: {player, finalQuestion, true}
            "FINAL QUESTION " + to_string(HEADMASTER_QUESTIONS + FINAL_QUESTIONS)  // label text for the question header
            + " of " + to_string(HEADMASTER_QUESTIONS + FINAL_QUESTIONS));      // joins on: of {to_string(HEADMASTER_QUESTIONS + FINAL_QUESTIONS)}

        if (result == ANSWER_ABORT)                                             // input closed during the question
        {                                                                       // start of block
            return;                                                             // leave this function (after victory this ends the game loop)
        }                                                                       // end of block

        if (result == ANSWER_CORRECT)                                           // answered correctly
        {                                                                       // start of block
            victoryEnding(player);                                              // CORRECT: show the victory ending
            return;                                                             // leave this function (after victory this ends the game loop)
        }                                                                       // end of block

        if (player.hp > 0)                                                      // wrong answer but still alive, so repeat the question
        {                                                                       // start of block
            cout << "\nThe room becomes dark.\n\n";                             // shows: The room becomes dark.
            cout << "The final question appears again.\n\n";                    // shows: The final question appears again.
            cout << "\"There is no escape from the final lesson.\"\n";          // shows: "There is no escape from the final lesson."
        }                                                                       // end of block
    }                                                                           // end of block

    return;                                                                     // gameLoop handles death
}                                                                               // end of block


// ============================================================
// VICTORY ENDING
// ============================================================

void victoryEnding(const Player &player)                                        // victory story and results screen (shown after the final answer)
{                                                                               // start of block
    cout << "\nThe Headmaster's final light disappears.\n\n";                   // shows: The Headmaster's final light disappears.
    cout << "For several seconds...\n\n";                                       // shows: For several seconds...
    cout << "There is complete silence.\n\n";                                   // shows: There is complete silence.
    cout << "Then...\n\n";                                                      // shows: Then...
    printCentered("DING... DONG...");                                           // shows centered: DING... DONG...
    cout << "\nThe school bell rings again.\n\n";                               // shows: The school bell rings again.
    cout << "But this time...\n\n";                                             // shows: But this time...
    cout << "Sunlight begins pouring through the windows.\n";                   // shows: Sunlight begins pouring through the windows.
    cout << "The darkness inside Blackwood Academy disappears.\n\n";            // shows: The darkness inside Blackwood Academy disappears.
    cout << "The Headmaster looks toward you and smiles.\n\n";                  // shows: The Headmaster looks toward you and smiles.
    cout << "\"Congratulations.\"\n";                                           // shows: "Congratulations."
    cout << "\"The Last Trial is over.\"\n\n";                                  // shows: "The Last Trial is over."
    cout << "For the first time in twenty years...\n";                          // shows: For the first time in twenty years...
    cout << "The academy entrance slowly opens.\n\n";                           // shows: The academy entrance slowly opens.
    cout << "Before you leave, the Headmaster asks:\n\n";                       // shows: Before you leave, the Headmaster asks:
    cout << "\"Why do you think you passed?\"\n\n";                             // shows: "Why do you think you passed?"
    cout << "You answer quietly...\n\n";                                        // shows: You answer quietly...
    cout << "\"Because I knew the answers?\"\n\n";                              // shows: "Because I knew the answers?"
    cout << "The Headmaster shakes his head.\n\n";                              // shows: The Headmaster shakes his head.
    cout << "\"No.\"\n\n";                                                      // shows: "No."
    cout << "\"Because you continued when you did not.\"\n\n";                  // shows: "Because you continued when you did not."
    cout << "You step outside Blackwood Academy.\n";                            // shows: You step outside Blackwood Academy.
    cout << "Behind you, the ancient doors slowly close.\n\n";                  // shows: Behind you, the ancient doors slowly close.

    waitForEnter("Press ENTER to see your final results...");                   // pause so the story can be read

    printBanner("THE LAST TRIAL - COMPLETED");                                  // show the completed banner
    cout << "\n";                                                               // prints a blank line

    printField("PLAYER", player.name);                                          // print player name
    printField("FINAL RANK", player.rank);                                      // print final rank
    printField("KNOWLEDGE XP", to_string(player.xp));                           // print total XP
    printField("QUESTIONS CORRECT", to_string(player.correct));                 // print correct answers
    printField("QUESTIONS INCORRECT", to_string(player.incorrect));             // print wrong answers
    printField("GUARDIANS DEFEATED", to_string(countDefeated(player)) + "/4");  // print guardians beaten
    printField("FINAL BOSS", "DEFEATED");                                       // print boss status

    cout << "\n";                                                               // prints a blank line
    printLine('=');                                                             // draw a divider line
    cout << "\n";                                                               // prints a blank line
    printCentered("\"Knowledge isn't knowing every answer.");                   // shows centered: "Knowledge isn't knowing every answer.
    printCentered("It's having the courage to keep searching for one.\"");      // shows centered: It's having the courage to keep searching for one."
    cout << "\n";                                                               // prints a blank line
    printLine('=');                                                             // draw a divider line

    waitForEnter("Press ENTER to exit the game...");                            // keep the window open after the ending
}                                                                               // end of block


// Pauses so the console window stays open (Dev C++ closes it on exit).
void waitForEnter(const string &message)                                        // pauses until Enter so the window does not close
{                                                                               // start of block
    cout << "\n" << message;                                                    // blank line, then the prompt text
    cout.flush();                                                               // make sure the prompt shows before waiting

    string ignored;                                                             // holds whatever the player types (not used)
    getline(cin, ignored);                                                      // wait here until Enter is pressed
}                                                                               // end of block


// ============================================================
// GAME OVER
// ============================================================

bool gameOver(Player &player)                                                   // function gameOver: shows GAME OVER; true if the player restarts
{                                                                               // start of function gameOver
    saveGame(player, false);                                                    // retain the death checkpoint

    cout << "\n";                                                               // prints a blank line
    printBanner("GAME OVER");                                                   // shows a banner titled: GAME OVER
    cout << "\n";                                                               // prints a blank line

    cout << player.name << ", your strength fades.\n\n";                        // shows: {player.name}, your strength fades.
    cout << "The academy bell rings once more.\n\n";                            // shows: The academy bell rings once more.
    printCentered("DING... DONG...");                                           // shows centered: DING... DONG...
    cout << "\nThe darkness closes around you.\n\n";                            // shows: The darkness closes around you.
    cout << "The Last Trial remains unfinished.\n\n";                           // shows: The Last Trial remains unfinished.

    printField("FINAL XP", to_string(player.xp));                               // shows the row "FINAL XP" = {to_string(player.xp)}
    printField("FINAL RANK", player.rank);                                      // shows the row "FINAL RANK" = {player.rank}
    printField("GUARDIANS DEFEATED", to_string(countDefeated(player)) + "/4");  // shows the row "GUARDIANS DEFEATED" = {to_string(countDefeated(player))}/4

    while (true)                                                                // loops forever until a return or break leaves it
    {                                                                           // start of while loop
        cout << "\nChoose START OVER or QUIT > ";                               // shows: Choose START OVER or QUIT >
        string choice;                                                          // creates the variable choice (string)

        if (!readCommandLine(choice))                                           // checks whether not readCommandLine(choice)
        {                                                                       // start of if block
            return false;                                                       // returns false
        }                                                                       // end of if block

        if (choice == "quit")                                                   // checks whether choice == "quit"
        {                                                                       // start of if block
            cout << "\nThe Last Trial ends here.\n";                            // shows: The Last Trial ends here.
            return false;                                                       // returns false
        }                                                                       // end of if block

        if (choice != "start over" && choice != "startover")                    // checks whether choice != "start over" and choice != "startover"
        {                                                                       // start of if block
            cout << "Please choose START OVER or QUIT.\n";                      // shows: Please choose START OVER or QUIT.
            continue;                                                           // skips to the next round of the loop
        }                                                                       // end of if block

        cout << "Enter player name: ";                                          // shows: Enter player name:
        string enteredName;                                                     // creates the variable enteredName (string)

        if (!getline(cin, enteredName))                                         // checks whether not getline(cin, enteredName)
        {                                                                       // start of if block
            return false;                                                       // returns false
        }                                                                       // end of if block
        enteredName = normalizeSpaces(enteredName);                             // sets enteredName to normalizeSpaces(enteredName)

        if (toLowerCase(enteredName) != toLowerCase(player.name))               // checks whether toLowerCase(enteredName) != toLowerCase(player.name)
        {                                                                       // start of if block
            cout << "Sorry! You're Not Registered in This Class.\n";            // shows: Sorry! You're Not Registered in This Class.
            continue;                                                           // skips to the next round of the loop
        }                                                                       // end of if block

        // Restore the checkpoint with fresh health; keep XP, rank,
        // artifacts, keys, chest state and defeated guardians intact.
        player.hp = PLAYER_MAX_HP;                                              // sets player.hp to PLAYER_MAX_HP
        updateRank(player);                                                     // calls updateRank: updates the player rank from the current XP
        saveGame(player, false);                                                // calls saveGame: saves the game

        cout << "\nWelcome back, " << player.name << ". Here is your progress recap.\n";  // shows: Welcome back, {player.name}. Here is your progress recap.
        showStatus(player);                                                     // calls showStatus: shows HP, XP, rank and progress
        showInventory(player);                                                  // calls showInventory: lists the artifacts the player carries
        showLocation(player);                                                   // calls showLocation: describes the current room
        return true;                                                            // returns true
    }                                                                           // end of while loop
}                                                                               // end of function gameOver


// ============================================================
// FILE STREAMS: SAVE AND LOAD
//
//   ofstream      writes the save data
//   ifstream      reads the save data
//   istringstream checks each saved line
//
// Save format (BLACKWOOD_SAVE_V2, same as the stream version):
//   line 1  version name
//   line 2  player name
//   line 3  room number
//   line 4  hp xp correct incorrect hasKey keyTaken chestOpened
//   line 5  4 artifact flags
//   line 6  4 defeated-guardian flags
// The rank is not stored: it is rebuilt from XP when loading,
// so a player who reached Apprentice always keeps Apprentice.
// ============================================================

bool saveFileExists()                                                           // function saveFileExists: checks whether a save file is on disk
{                                                                               // start of function saveFileExists
    ifstream test(SAVE_FILE.c_str());                                           // creates the file stream test for SAVE_FILE.c_str(
    return test.good();                                                         // returns test.good()
}                                                                               // end of function saveFileExists

bool saveGame(const Player &player, bool announce)                              // function saveGame: writes the progress to the save file (with temp + backup)
{                                                                               // start of function saveGame
    // Write to a temporary file first so a crash cannot ruin the old save.
    ofstream out(TEMP_SAVE_FILE.c_str(), ios::out | ios::trunc);                // creates the file stream out for TEMP_SAVE_FILE.c_str(), ios::out | ios:...

    if (!out.is_open())                                                         // checks whether not out.is_open()
    {                                                                           // start of if block
        cout << "\nERROR: Cannot open the temporary save file.\n";              // shows: ERROR: Cannot open the temporary save file.
        return false;                                                           // returns false
    }                                                                           // end of if block

    out << "BLACKWOOD_SAVE_V2\n";                                               // writes to the file stream out: BLACKWOOD_SAVE_V2
    out << player.name << '\n';                                                 // writes to the file stream out: {player.name}
    out << static_cast<int>(player.currentRoom) << '\n';                        // writes to the file stream out: {static_cast<int>(player.currentRoom)}

    out << player.hp << ' '                                                     // writes to the file stream out: {player.hp}
        << player.xp << ' '                                                     // also writes: {player.xp}
        << player.correct << ' '                                                // also writes: {player.correct}
        << player.incorrect << ' '                                              // also writes: {player.incorrect}
        << static_cast<int>(player.hasKey) << ' '                               // also writes: {static_cast<int>(player.hasKey)}
        << static_cast<int>(player.libraryKeyTaken) << ' '                      // also writes: {static_cast<int>(player.libraryKeyTaken)}
        << static_cast<int>(player.chestOpened) << '\n';                        // also writes: {static_cast<int>(player.chestOpened)}

    for (int i = 0; i < 4; i++)                                                 // repeats for i = 0 up to 4 - 1
    {                                                                           // start of for loop
        out << static_cast<int>(player.item[i]) << ' ';                         // writes to the file stream out: {static_cast<int>(player.item[i])}
    }                                                                           // end of for loop
    out << '\n';                                                                // writes to the file stream out:

    for (int i = 0; i < 4; i++)                                                 // repeats for i = 0 up to 4 - 1
    {                                                                           // start of for loop
        out << static_cast<int>(player.defeated[i]) << ' ';                     // writes to the file stream out: {static_cast<int>(player.defeated[i])}
    }                                                                           // end of for loop
    out << '\n';                                                                // writes to the file stream out:

    out.close();                                                                // calls close: closes the file

    if (!out)                                                                   // write failed?
    {                                                                           // start of if block
        remove(TEMP_SAVE_FILE.c_str());                                         // calls remove: deletes a file from disk
        cout << "\nERROR: Writing the save file failed.\n";                     // shows: ERROR: Writing the save file failed.
        cout << "Your previous save was kept.\n";                               // shows: Your previous save was kept.
        return false;                                                           // returns false
    }                                                                           // end of if block

    // Back up the existing save (if any).
    bool hadPrevious = saveFileExists();                                        // creates hadPrevious (bool) starting as saveFileExists()

    if (hadPrevious)                                                            // checks whether hadPrevious
    {                                                                           // start of if block
        remove(BACKUP_SAVE_FILE.c_str());                                       // calls remove: deletes a file from disk

        if (rename(SAVE_FILE.c_str(), BACKUP_SAVE_FILE.c_str()) != 0)           // checks whether rename(SAVE_FILE.c_str(), BACKUP_SAVE_FILE.c_str()) != 0
        {                                                                       // start of if block
            remove(TEMP_SAVE_FILE.c_str());                                     // calls remove: deletes a file from disk
            cout << "\nERROR: Cannot back up the existing save.\n";             // shows: ERROR: Cannot back up the existing save.
            cout << "Your previous save was not changed.\n";                    // shows: Your previous save was not changed.
            return false;                                                       // returns false
        }                                                                       // end of if block
    }                                                                           // end of if block

    // Install the new save.
    if (rename(TEMP_SAVE_FILE.c_str(), SAVE_FILE.c_str()) != 0)                 // checks whether rename(TEMP_SAVE_FILE.c_str(), SAVE_FILE.c_str()) != 0
    {                                                                           // start of if block
        if (hadPrevious &&                                                      // checks whether hadPrevious and (condition continues below)
            rename(BACKUP_SAVE_FILE.c_str(), SAVE_FILE.c_str()) != 0)           // argument / continuation: {rename(BACKUP_SAVE_FILE.c_str(), SAVE_FILE.c_str()) != 0}
        {                                                                       // start of block
            cout << "\nWARNING: Automatic restore failed.\n";                   // shows: WARNING: Automatic restore failed.
            cout << "Your old progress is in " << BACKUP_SAVE_FILE << ".\n";    // shows: Your old progress is in {BACKUP_SAVE_FILE}.
        }                                                                       // end of block

        remove(TEMP_SAVE_FILE.c_str());                                         // calls remove: deletes a file from disk
        cout << "\nERROR: Cannot install the new save file.\n";                 // shows: ERROR: Cannot install the new save file.
        return false;                                                           // returns false
    }                                                                           // end of if block

    if (announce)                                                               // checks whether announce
    {                                                                           // start of if block
        cout << "\n";                                                           // prints a blank line
        printLine('-');                                                         // draws a full-width line of '-'
        printCentered("GAME SAVED SUCCESSFULLY!");                              // shows centered: GAME SAVED SUCCESSFULLY!
        printLine('-');                                                         // draws a full-width line of '-'
        printField("File", SAVE_FILE);                                          // shows the row "File" = {SAVE_FILE}
        printField("Player", player.name);                                      // shows the row "Player" = {player.name}
        printField("Room", roomName(player.currentRoom));                       // shows the row "Room" = {roomName(player.currentRoom)}
        printField("HP / XP / Rank", to_string(player.hp) + " / "               // calls printField: prints a label and value in aligned columns (arguments continue below)
                                     + to_string(player.xp) + " / " + player.rank);  // joins on: {to_string(player.xp)} / {player.rank}
        printLine('-');                                                         // draws a full-width line of '-'
    }                                                                           // end of if block

    return true;                                                                // returns true
}                                                                               // end of function saveGame

// Reads exactly "size" integers from one line (nothing extra allowed).
bool parseIntegers(const string &line, int *values, int size)                   // function parseIntegers: checks a saved line holds the expected whole numbers
{                                                                               // start of function parseIntegers
    istringstream input(line);                                                  // creates the file stream input for line

    for (int i = 0; i < size; i++)                                              // repeats for i = 0 up to size - 1
    {                                                                           // start of for loop
        if (!(input >> values[i]))                                              // checks whether not (input >> values[i])
        {                                                                       // start of if block
            return false;                                                       // returns false
        }                                                                       // end of if block
    }                                                                           // end of for loop

    string extra;                                                               // creates the variable extra (string)

    return !(input >> extra);                                                   // returns !(input >> extra)
}                                                                               // end of function parseIntegers

bool loadGame(Player &player)                                                   // function loadGame: reads and validates the save file, then restores progress
{                                                                               // start of function loadGame
    ifstream in(SAVE_FILE.c_str());                                             // creates the file stream in for SAVE_FILE.c_str(

    if (!in.is_open())                                                          // checks whether not in.is_open()
    {                                                                           // start of if block
        cout << "\nNo save file found.\n";                                      // shows: No save file found.
        cout << "Type \"save\" to create one.\n";                               // shows: Type "save" to create one.
        return false;                                                           // returns false
    }                                                                           // end of if block

    string version;                                                             // creates the variable version (string)
    string name;                                                                // creates the variable name (string)
    string line;                                                                // creates the variable line (string)

    if (!getline(in, version) ||                                                // checks whether not getline(in, version; if so: ||
        (version != "BLACKWOOD_SAVE_V1" && version != "BLACKWOOD_SAVE_V2") ||   // argument / continuation: BLACKWOOD_SAVE_V1BLACKWOOD_SAVE_V2
        !getline(in, name) ||                                                   // argument / continuation: {!getline(in, name) ||}
        name.empty() ||                                                         // argument / continuation: {name.empty() ||}
        name.find_first_not_of(" \t\r") == string::npos)                        // argument / continuation: {name.find_first_not_of(" \t\r") == string::npos}
    {                                                                           // start of block
        cout << "\nERROR: Invalid save header or player name.\n";               // shows: ERROR: Invalid save header or player name.
        return false;                                                           // returns false
    }                                                                           // end of block

    while (!name.empty() && (name.back() == '\r' || name.back() == '\n'))       // keeps repeating while not name.empty() and (name.back() == '\r' or name.back() == '\n')
    {                                                                           // start of while loop
        name.pop_back();                                                        // Windows line endings
    }                                                                           // end of while loop

    Player temp;                                                                // load here first
    initializePlayer(temp);                                                     // calls initializePlayer: sets every player value to its starting state
    temp.name = name;                                                           // sets temp.name to name

    int roomData[1];                                                            // creates the variable roomData (int)
    int stats[7];                                                               // creates the variable stats (int)
    int itemData[4];                                                            // creates the variable itemData (int)
    int defeatedData[4];                                                        // creates the variable defeatedData (int)

    if (!getline(in, line) || !parseIntegers(line, roomData, 1) ||              // checks whether not getline(in, line) or not parseIntegers(line, roomData, 1; if so: ||
        roomData[0] < ROOM_MAIN_HALL || roomData[0] > ROOM_SEALED_CHAMBER)      // argument / continuation: {roomData[0] < ROOM_MAIN_HALL || roomData[0] > ROOM_SEALED_CHAMBER}
    {                                                                           // start of block
        cout << "\nERROR: Invalid saved room.\n";                               // shows: ERROR: Invalid saved room.
        return false;                                                           // returns false
    }                                                                           // end of block

    temp.currentRoom = static_cast<Room>(roomData[0]);                          // sets temp.currentRoom to static_cast<Room>(roomData[0])

    if (!getline(in, line) || !parseIntegers(line, stats, 7))                   // checks whether not getline(in, line) or not parseIntegers(line, stats, 7)
    {                                                                           // start of if block
        cout << "\nERROR: Missing or invalid player statistics.\n";             // shows: ERROR: Missing or invalid player statistics.
        return false;                                                           // returns false
    }                                                                           // end of if block

    temp.hp = stats[0];                                                         // sets temp.hp to stats[0]
    temp.xp = stats[1];                                                         // sets temp.xp to stats[1]
    temp.correct = stats[2];                                                    // sets temp.correct to stats[2]
    temp.incorrect = stats[3];                                                  // sets temp.incorrect to stats[3]
    temp.hasKey = (stats[4] == 1);                                              // sets temp.hasKey to (stats[4] == 1)
    temp.libraryKeyTaken = (stats[5] == 1);                                     // sets temp.libraryKeyTaken to (stats[5] == 1)
    temp.chestOpened = (stats[6] == 1);                                         // sets temp.chestOpened to (stats[6] == 1)

    if (temp.hp < 0 || temp.hp > PLAYER_MAX_HP ||                               // checks whether temp.hp < 0 or temp.hp > PLAYER_MAX_HP or (condition continues below)
        temp.xp < 0 || temp.correct < 0 || temp.incorrect < 0 ||                // argument / continuation: {temp.xp < 0 || temp.correct < 0 || temp.incorrect < 0 ||}
        temp.correct > 1000000 ||                                               // argument / continuation: {temp.correct > 1000000 ||}
        temp.xp != temp.correct * CORRECT_XP ||                                 // argument / continuation: {temp.xp != temp.correct * CORRECT_XP ||}
        stats[4] < 0 || stats[4] > 1 ||                                         // argument / continuation: {stats[4] < 0 || stats[4] > 1 ||}
        stats[5] < 0 || stats[5] > 1 ||                                         // argument / continuation: {stats[5] < 0 || stats[5] > 1 ||}
        stats[6] < 0 || stats[6] > 1 ||                                         // argument / continuation: {stats[6] < 0 || stats[6] > 1 ||}
        (temp.hasKey && !temp.libraryKeyTaken) ||                               // argument / continuation: {(temp.hasKey && !temp.libraryKeyTaken) ||}
        (temp.chestOpened && (!temp.libraryKeyTaken || temp.hasKey)))           // argument / continuation: {(temp.chestOpened && (!temp.libraryKeyTaken || temp.hasKey))}
    {                                                                           // start of block
        cout << "\nERROR: Saved statistics are inconsistent.\n";                // shows: ERROR: Saved statistics are inconsistent.
        return false;                                                           // returns false
    }                                                                           // end of block

    if (!getline(in, line) || !parseIntegers(line, itemData, 4) ||              // checks whether not getline(in, line) or not parseIntegers(line, itemData, 4; if so: ||
        !getline(in, line) || !parseIntegers(line, defeatedData, 4))            // argument / continuation: {!getline(in, line) || !parseIntegers(line, defeatedData, 4)}
    {                                                                           // start of block
        cout << "\nERROR: Missing or invalid inventory/guardian data.\n";       // shows: ERROR: Missing or invalid inventory/guardian data.
        return false;                                                           // returns false
    }                                                                           // end of block

    for (int i = 0; i < 4; i++)                                                 // repeats for i = 0 up to 4 - 1
    {                                                                           // start of for loop
        if (itemData[i] < 0 || itemData[i] > 1 ||                               // checks whether itemData[i] < 0 or itemData[i] > 1 or (condition continues below)
            defeatedData[i] < 0 || defeatedData[i] > 1)                         // argument / continuation: {defeatedData[i] < 0 || defeatedData[i] > 1}
        {                                                                       // start of block
            cout << "\nERROR: Saved flags must be 0 or 1.\n";                   // shows: ERROR: Saved flags must be 0 or 1.
            return false;                                                       // returns false
        }                                                                       // end of block

        temp.item[i] = (itemData[i] == 1);                                      // sets temp.item[i] to (itemData[i] == 1)
        temp.defeated[i] = (defeatedData[i] == 1);                              // sets temp.defeated[i] to (defeatedData[i] == 1)
    }                                                                           // end of for loop

    string extra;                                                               // creates the variable extra (string)

    while (getline(in, line))                                                   // no unexpected data
    {                                                                           // start of while loop
        istringstream remaining(line);                                          // creates the file stream remaining for line

        if (remaining >> extra)                                                 // checks whether remaining >> extra
        {                                                                       // start of if block
            cout << "\nERROR: Unexpected extra save data.\n";                   // shows: ERROR: Unexpected extra save data.
            return false;                                                       // returns false
        }                                                                       // end of if block
    }                                                                           // end of while loop

    if (in.bad())                                                               // checks whether in.bad()
    {                                                                           // start of if block
        cout << "\nERROR: The save file could not be read.\n";                  // shows: ERROR: The save file could not be read.
        return false;                                                           // returns false
    }                                                                           // end of if block

    // Progress must make sense (items need their source).
    if ((temp.currentRoom == ROOM_SEALED_CHAMBER && countDefeated(temp) != 4) ||  // checks whether (temp.currentRoom == ROOM_SEALED_CHAMBER and countDefeated(temp) != 4; if so: ||
        (temp.item[ITEM_TIME_CRYSTAL] && !temp.defeated[SUBJECT_SCIENCE]) ||    // argument / continuation: {(temp.item[ITEM_TIME_CRYSTAL] && !temp.defeated[SUBJECT_SCIENCE]) ||}
        (temp.item[ITEM_RUNE]         && !temp.defeated[SUBJECT_MATHEMATICS]) ||  // argument / continuation: {(temp.item[ITEM_RUNE] && !temp.defeated[SUBJECT_MATHEMATICS]) ||}
        (temp.item[ITEM_SCROLL]       && !temp.defeated[SUBJECT_ENGLISH]) ||    // argument / continuation: {(temp.item[ITEM_SCROLL] && !temp.defeated[SUBJECT_ENGLISH]) ||}
        (temp.item[ITEM_COIN] && !temp.chestOpened && !temp.defeated[SUBJECT_COMPUTER]))  // argument / continuation: {(temp.item[ITEM_COIN] && !temp.chestOpened && !temp.defeated[SUBJECT_COMP...
    {                                                                           // start of block
        cout << "\nERROR: Saved game progress is inconsistent.\n";              // shows: ERROR: Saved game progress is inconsistent.
        return false;                                                           // returns false
    }                                                                           // end of block

    // Rank is rebuilt from XP, so Apprentice (100+ XP) is always kept.
    temp.rank = rankNameOfLevel(rankLevelOfXp(temp.xp));                        // sets temp.rank to rankNameOfLevel(rankLevelOfXp(temp.xp))

    player = temp;                                                              // only now replace it

    cout << "\n";                                                               // prints a blank line
    printLine('-');                                                             // draws a full-width line of '-'
    printCentered("GAME LOADED SUCCESSFULLY!");                                 // shows centered: GAME LOADED SUCCESSFULLY!
    printLine('-');                                                             // draws a full-width line of '-'
    printField("Save version", version);                                        // shows the row "Save version" = {version}
    printCentered("Welcome back, " + player.name + "!");                        // shows centered: Welcome back, {player.name}!
    printLine('-');                                                             // draws a full-width line of '-'

    return true;                                                                // returns true
}                                                                               // end of function loadGame
