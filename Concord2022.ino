/**************************************************************************
    Stars2021 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    See <https://www.gnu.org/licenses/>.
*/

#include "RPU_Config.h"
#include "RPU.h"
#include "AudioHandler.h"
#include "DropTargets.h"
#include "SelfTestAndAudit.h"
#include "Concord.h"
#include "LampAnimations.h"
#include <EEPROM.h>


#define CONCORD_2022_MAJOR_VERSION  2022
#define CONCORD_2022_MINOR_VERSION  1
#define DEBUG_MESSAGES  0

/*********************************************************************

    Game specific code

*********************************************************************/

// MachineState
//  0 - Attract Mode
//  negative - self-test modes
//  positive - game play
char MachineState = 0;
boolean MachineStateChanged = true;
#define MACHINE_STATE_ATTRACT         0
#define MACHINE_STATE_INIT_GAMEPLAY   1
#define MACHINE_STATE_INIT_NEW_BALL   2
#define MACHINE_STATE_NORMAL_GAMEPLAY 4
#define MACHINE_STATE_COUNTDOWN_BONUS 99
#define MACHINE_STATE_BALL_OVER       100
#define MACHINE_STATE_MATCH_MODE      110


#define MACHINE_STATE_ADJUST_FREEPLAY           (MACHINE_STATE_TEST_DONE-1)
#define MACHINE_STATE_ADJUST_BALL_SAVE          (MACHINE_STATE_TEST_DONE-2)
#define MACHINE_STATE_ADJUST_SFX_AND_SOUNDTRACK (MACHINE_STATE_TEST_DONE-3)
#define MACHINE_STATE_ADJUST_MUSIC_VOLUME       (MACHINE_STATE_TEST_DONE-4)
#define MACHINE_STATE_ADJUST_SFX_VOLUME         (MACHINE_STATE_TEST_DONE-5)
#define MACHINE_STATE_ADJUST_CALLOUTS_VOLUME    (MACHINE_STATE_TEST_DONE-6)
#define MACHINE_STATE_ADJUST_CREDIT_RESET_HOLD_TIME   (MACHINE_STATE_TEST_DONE-7)
#define MACHINE_STATE_ADJUST_TOURNAMENT_SCORING (MACHINE_STATE_TEST_DONE-8)
#define MACHINE_STATE_ADJUST_TILT_WARNING       (MACHINE_STATE_TEST_DONE-9)
#define MACHINE_STATE_ADJUST_AWARD_OVERRIDE     (MACHINE_STATE_TEST_DONE-10)
#define MACHINE_STATE_ADJUST_BALLS_OVERRIDE     (MACHINE_STATE_TEST_DONE-11)
#define MACHINE_STATE_ADJUST_SCROLLING_SCORES   (MACHINE_STATE_TEST_DONE-12)
#define MACHINE_STATE_ADJUST_EXTRA_BALL_AWARD   (MACHINE_STATE_TEST_DONE-13)
#define MACHINE_STATE_ADJUST_SPECIAL_AWARD      (MACHINE_STATE_TEST_DONE-14)
#define MACHINE_STATE_ADJUST_GATE_SETTING       (MACHINE_STATE_TEST_DONE-15)
#define MACHINE_STATE_ADJUST_FLIGHT_INSTRUCTION (MACHINE_STATE_TEST_DONE-16)
#define MACHINE_STATE_ADJUST_LEG_DIFFICULTY     (MACHINE_STATE_TEST_DONE-17)
#define MACHINE_STATE_ADJUST_TURBULENCE         (MACHINE_STATE_TEST_DONE-18)
#define MACHINE_STATE_ADJUST_DONE               (MACHINE_STATE_TEST_DONE-19)

byte SelfTestStateToCalloutMap[] = {
  136, 137, 135, 134, 133, 140, 141, 142, 139, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, // <- SelfTestAndAudit modes
  // Starting concord specific modes
  153, 154, 155, 156, 157, 158, // Freeplay through Callouts volume
  209, 159, 160, 161, 162, 163, 164, 165, // through special award
  220, 221, 222, 223,
  0
};

#define SOUND_EFFECT_AP_CRB_OPTION_1    211
#define SOUND_EFFECT_AP_CRB_OPTION_99   210

#define GAME_MODE_SKILL_SHOT_PRE_SWITCH             0
#define GAME_MODE_SKILL_SHOT_BUILD_BONUS            1
#define GAME_MODE_UNSTRUCTURED_PLAY                 2
#define GAME_MODE_BOARDING                          3
#define GAME_MODE_TAKEOFF                           4
#define GAME_MODE_CRUISING                          5
#define GAME_MODE_APPROACH                          6
#define GAME_MODE_LANDING                           7
#define GAME_MODE_FLIGHT_COMPLETE                   8
#define GAME_MODE_COLLECT_MILES                     9
#define GAME_MODE_SPECIAL_TOUR_START                10
#define GAME_MODE_SPECIAL_TOUR_NUMBERS              11
#define GAME_MODE_SPECIAL_TOUR_LEFT_LANE            12
#define GAME_MODE_SPECIAL_TOUR_SPINNER              13
#define GAME_MODE_SPECIAL_TOUR_DROPS                14
#define GAME_MODE_SPECIAL_TOUR_SAUCER               15
#define GAME_MODE_SPECIAL_TOUR_END                  16


#define EEPROM_BALL_SAVE_BYTE           100
#define EEPROM_FREE_PLAY_BYTE           101
#define EEPROM_SOUND_SELECTOR_BYTE      102
#define EEPROM_SKILL_SHOT_BYTE          103
#define EEPROM_TILT_WARNING_BYTE        104
#define EEPROM_AWARD_OVERRIDE_BYTE      105
#define EEPROM_BALLS_OVERRIDE_BYTE      106
#define EEPROM_TOURNAMENT_SCORING_BYTE  107
#define EEPROM_MUSIC_VOLUME_BYTE        108
#define EEPROM_SFX_VOLUME_BYTE          109
#define EEPROM_SCROLLING_SCORES_BYTE    110
#define EEPROM_CALLOUTS_VOLUME_BYTE     111
#define EEPROM_DIM_LEVEL_BYTE           113
#define EEPROM_CRB_HOLD_TIME            119
#define EEPROM_GATE_SETTING             120
#define EEPROM_FLIGHT_INSTRUCTION       121
#define EEPROM_LEG_DIFFICULTY           122
#define EEPROM_TURBULENCE               123
#define EEPROM_EXTRA_BALL_SCORE_BYTE    140
#define EEPROM_SPECIAL_SCORE_BYTE       144

#define SOUND_EFFECT_NONE                     0
#define SOUND_EFFECT_TILT                     1
#define SOUND_EFFECT_TILT_WARNING             2
#define SOUND_EFFECT_OUTLANE_LIT              3
#define SOUND_EFFECT_OUTLANE_UNLIT            4
#define SOUND_EFFECT_INLANE                   5
#define SOUND_EFFECT_SONIC_BOOM               6
#define SOUND_EFFECT_LEG_SCORE                7
#define SOUND_EFFECT_SKILL_SHOT               8
#define SOUND_EFFECT_REPEAT_NUMBER            9
#define SOUND_EFFECT_MATCH_SPIN               10
#define SOUND_EFFECT_DROP_TARGET_HIT          11
#define SOUND_EFFECT_DROP_TARGET_FINISHED     12
#define SOUND_EFFECT_SINGLE_BONG              13
#define SOUND_EFFECT_NEW_NUMBER               14
#define SOUND_EFFECT_JET_FLYBY                15
#define SOUND_EFFECT_ADVANCED_SCORE_1         16
#define SOUND_EFFECT_ADVANCED_SCORE_2         17
#define SOUND_EFFECT_ADVANCED_SCORE_3         18
#define SOUND_EFFECT_ADVANCED_SCORE_4         19
#define SOUND_EFFECT_SPINNER_STD              20
#define SOUND_EFFECT_WIND_GUST                21
#define SOUND_EFFECT_TURBULENCE               22
#define SOUND_EFFECT_SLING_SHOT               34
#define SOUND_EFFECT_ROLLOVER                 35
#define SOUND_EFFECT_ANIMATION_TICK           36
#define SOUND_EFFECT_PLANE_LANDING            37
#define SOUND_EFFECT_TOCK                     38
#define SOUND_EFFECT_DING                     39
#define SOUND_EFFECT_BUMPER_HIT_STD_1         40
#define SOUND_EFFECT_GAME_OVER                60
#define SOUND_EFFECT_CREDIT_ADDED             61
#define SOUND_EFFECT_COIN_DROP_1              100
#define SOUND_EFFECT_COIN_DROP_2              101
#define SOUND_EFFECT_COIN_DROP_3              102
#define SOUND_EFFECT_MACHINE_START            318
#define SOUND_EFFECT_BALL_OVER                319

//#define SOUND_EFFECT_SELF_TEST_MODE_START             133
#define SOUND_EFFECT_SELF_TEST_CPC_START              180
#define SOUND_EFFECT_SELF_TEST_AUDIO_OPTIONS_START    190


#define RALLY_MUSIC_WAITING_FOR_SKILLSHOT         0
#define RALLY_MUSIC_WAITING_FOR_SKILLSHOT_CALM    1
#define RALLY_MUSIC_WAITING_FOR_SKILLSHOT_UPBEAT  3
#define RALLY_MUSIC_WAITING_FOR_SKILLSHOT_MEDIUM  2

// General music
#define SOUND_EFFECT_BACKGROUND_SONG_1        700
#define SOUND_EFFECT_BACKGROUND_SONG_2        701
#define SOUND_EFFECT_BACKGROUND_SONG_3        702
#define SOUND_EFFECT_BACKGROUND_SONG_4        703
#define SOUND_EFFECT_BACKGROUND_SONG_5        704
#define SOUND_EFFECT_TENSE_BACKGROUND_SONG_1       750
#define SOUND_EFFECT_TENSE_BACKGROUND_SONG_2       751
#define SOUND_EFFECT_TENSE_BACKGROUND_SONG_3       752
#define SOUND_EFFECT_WIZARD_BACKGROUND_SONG        760
#define SOUND_EFFECT_BACKGROUND_NOISE         775

// For each music set, there are five types of music
#define MUSIC_TYPE_NO_MISSION         0
#define MUSIC_TYPE_SIDE_QUEST         1
#define MUSIC_TYPE_MISSION            2
#define MUSIC_TYPE_MISSION_COMPLETION 3
#define MUSIC_TYPE_WIZARD             4
#define MUSIC_TYPE_RALLY              5
//

byte CurrentMusicType = 0xFF; // value can be 0-3

#define NUM_DIAGNOSTIC_NOTIFICATIONS  14
unsigned short DiagnosticNotificationDurations[NUM_DIAGNOSTIC_NOTIFICATIONS] = {
  1964, 1309, 1352, 1599, 1298, 2168, 1921, 2769, 2844, 2672,
  2661, 2726, 2693, 2811
};

#define SOUND_EFFECT_DIAG_START                   1900
#define SOUND_EFFECT_DIAG_CREDIT_RESET_BUTTON     1900
#define SOUND_EFFECT_DIAG_SELECTOR_SWITCH_ON      1901
#define SOUND_EFFECT_DIAG_SELECTOR_SWITCH_OFF     1902
#define SOUND_EFFECT_DIAG_STARTING_ORIGINAL_CODE  1903
#define SOUND_EFFECT_DIAG_STARTING_NEW_CODE       1904
#define SOUND_EFFECT_DIAG_ORIGINAL_CPU_DETECTED   1905
#define SOUND_EFFECT_DIAG_ORIGINAL_CPU_RUNNING    1906
#define SOUND_EFFECT_DIAG_PROBLEM_PIA_U10         1907
#define SOUND_EFFECT_DIAG_PROBLEM_PIA_U11         1908
#define SOUND_EFFECT_DIAG_PROBLEM_PIA_1           1909
#define SOUND_EFFECT_DIAG_PROBLEM_PIA_2           1910
#define SOUND_EFFECT_DIAG_PROBLEM_PIA_3           1911
#define SOUND_EFFECT_DIAG_PROBLEM_PIA_4           1912
#define SOUND_EFFECT_DIAG_PROBLEM_PIA_5           1913

// Game play status callouts
#define NUM_VOICE_NOTIFICATIONS                 76
unsigned short VoiceNotificationDurations[NUM_VOICE_NOTIFICATIONS] = {
  4903, 6145, 5462, 5245, 5779, 1903, 1997, 1973, 2137, 1957,
  2042, 2009, 2061, 936, 1392, 1677, 2736, 923, 797, 1189,
  0, 2392, 2422, 1932, 1972, 1969, 1994, 1530, 4334, 3780,
  3601, 4218, 3778, 4396, 3605, 4162, 3626, 4203, 3595, 4223,
  3663, 4183, 3522, 4092, 3240, 3022, 3161, 3017, 3099, 3048,
  3153, 2961, 8151, 2705, 5101, 4751, 4356, 7944, 2997, 1866, 
  1675, 3165, 2088, 2121, 1295, 2646, 2763, 1633, 2817, 1479,
  965, 2144, 1345, 1568, 1868, 1844
};

#define NUM_CAPTAINS      8
#define SOUND_EFFECT_VP_VOICE_NOTIFICATIONS_START  400

#define SOUND_EFFECT_VP_BOARDING_1          400
#define SOUND_EFFECT_VP_BOARDING_2          401
#define SOUND_EFFECT_VP_BOARDING_3          402
#define SOUND_EFFECT_VP_BOARDING_4          403
#define SOUND_EFFECT_VP_BOARDING_5          404
#define SOUND_EFFECT_VP_ADD_PLAYER_1        405
#define SOUND_EFFECT_VP_ADD_PLAYER_2        (SOUND_EFFECT_ADD_PLAYER_1+1)
#define SOUND_EFFECT_VP_ADD_PLAYER_3        (SOUND_EFFECT_ADD_PLAYER_1+2)
#define SOUND_EFFECT_VP_ADD_PLAYER_4        (SOUND_EFFECT_ADD_PLAYER_1+3)
#define SOUND_EFFECT_VP_PLAYER_1_UP         409
#define SOUND_EFFECT_VP_PLAYER_2_UP         (SOUND_EFFECT_PLAYER_1_UP+1)
#define SOUND_EFFECT_VP_PLAYER_3_UP         (SOUND_EFFECT_PLAYER_1_UP+2)
#define SOUND_EFFECT_VP_PLAYER_4_UP         (SOUND_EFFECT_PLAYER_1_UP+3)
#define SOUND_EFFECT_VP_SHOOT_AGAIN         413
#define SOUND_EFFECT_VP_TAKEOFF             414
#define SOUND_EFFECT_VP_CRUISING            415
#define SOUND_EFFECT_VP_APPROACH            416
#define SOUND_EFFECT_VP_EXTRA_BALL          417
#define SOUND_EFFECT_VP_JACKPOT             418
#define SOUND_EFFECT_VP_SUPER_JACKPOT       419

#define SOUND_EFFECT_VP_TURBULENCE          421
#define SOUND_EFFECT_VP_RETURN_TO_1X        422
#define SOUND_EFFECT_VP_2X_PLAYFIELD        423
#define SOUND_EFFECT_VP_3X_PLAYFIELD        424
#define SOUND_EFFECT_VP_4X_PLAYFIELD        425
#define SOUND_EFFECT_VP_5X_PLAYFIELD        426
#define SOUND_EFFECT_VP_FASTEN_SEATBELTS    427
#define SOUND_EFFECT_VP_RETURN_TO_SEATS_1A  428
#define SOUND_EFFECT_VP_RETURN_TO_SEATS_1B  429
#define SOUND_EFFECT_VP_RETURN_TO_SEATS_2A  430
#define SOUND_EFFECT_VP_RETURN_TO_SEATS_2B  431
#define SOUND_EFFECT_VP_RETURN_TO_SEATS_3A  432
#define SOUND_EFFECT_VP_RETURN_TO_SEATS_3B  433
#define SOUND_EFFECT_VP_RETURN_TO_SEATS_4A  434
#define SOUND_EFFECT_VP_RETURN_TO_SEATS_4B  435
#define SOUND_EFFECT_VP_RETURN_TO_SEATS_5A  436
#define SOUND_EFFECT_VP_RETURN_TO_SEATS_5B  437
#define SOUND_EFFECT_VP_RETURN_TO_SEATS_6A  438
#define SOUND_EFFECT_VP_RETURN_TO_SEATS_6B  439
#define SOUND_EFFECT_VP_RETURN_TO_SEATS_7A  440
#define SOUND_EFFECT_VP_RETURN_TO_SEATS_7B  441
#define SOUND_EFFECT_VP_RETURN_TO_SEATS_8A  442
#define SOUND_EFFECT_VP_RETURN_TO_SEATS_8B  443
#define SOUND_EFFECT_VP_CAPTAIN_SEATBELTS_1   444
#define SOUND_EFFECT_VP_CAPTAIN_SEATBELTS_2   445
#define SOUND_EFFECT_VP_CAPTAIN_SEATBELTS_3   446
#define SOUND_EFFECT_VP_CAPTAIN_SEATBELTS_4   447
#define SOUND_EFFECT_VP_CAPTAIN_SEATBELTS_5   448
#define SOUND_EFFECT_VP_CAPTAIN_SEATBELTS_6   449
#define SOUND_EFFECT_VP_CAPTAIN_SEATBELTS_7   450
#define SOUND_EFFECT_VP_CAPTAIN_SEATBELTS_8   451
#define SOUND_EFFECT_VP_SPECIAL_TOUR_1        452
#define SOUND_EFFECT_VP_SPECIAL_TOUR_2        453
#define SOUND_EFFECT_VP_SPECIAL_TOUR_3        454
#define SOUND_EFFECT_VP_SPECIAL_TOUR_4        455
#define SOUND_EFFECT_VP_SPECIAL_TOUR_5        456
#define SOUND_EFFECT_VP_SPECIAL_TOUR_6        457
#define SOUND_EFFECT_VP_RETURN_TO_JFK         458
#define SOUND_EFFECT_VP_TURBULENCE_OVER           459
#define SOUND_EFFECT_VP_ENCOUNTERING_TURBULENCE   460
#define SOUND_EFFECT_VP_BOARDING_TO_JFK           461
#define SOUND_EFFECT_VP_GATE_OPEN_BALL            462
#define SOUND_EFFECT_VP_GATE_OPEN_GAME            463
#define SOUND_EFFECT_VP_USE_JETSTREAM             464
#define SOUND_EFFECT_VP_USE_SPINNER               465
#define SOUND_EFFECT_VP_USE_DROP_TARGETS          466
#define SOUND_EFFECT_VP_USE_SAUCER                467
#define SOUND_EFFECT_VP_COLLECT_FREQUENT          468
#define SOUND_EFFECT_VP_FAILED_TO_COLLECT         469
#define SOUND_EFFECT_VP_FLIGHT_CANCELLED          470
#define SOUND_EFFECT_VP_TOUR_NUMBERS              471
#define SOUND_EFFECT_VP_TOUR_LEFT_LANE            472
#define SOUND_EFFECT_VP_TOUR_SPINNER              473
#define SOUND_EFFECT_VP_TOUR_DROPS                474
#define SOUND_EFFECT_VP_TOUR_SAUCER               475



#define MAX_DISPLAY_BONUS     59
#define TILT_WARNING_DEBOUNCE_TIME      1000


/*********************************************************************

    Machine state and options

*********************************************************************/
byte SoundtrackSelection = 0;
byte Credits = 0;
byte SoundSelector = 2;
byte BallSaveNumSeconds = 0;
byte CurrentBallSaveNumSeconds = 0;
byte MaximumCredits = 40;
byte BallsPerGame = 3;
byte DimLevel = 2;
byte ScoreAwardReplay = 0;
byte MusicVolume = 10;
byte SoundEffectsVolume = 10;
byte CalloutsVolume = 10;
byte ChuteCoinsInProgress[3];
byte TimeRequiredToResetGame = 1;
boolean HighScoreReplay = true;
boolean MatchFeature = true;
boolean TournamentScoring = false;
boolean ScrollingScores = true;
boolean FreePlayMode = false;
unsigned long HighScore = 0;
unsigned long AwardScores[3];
unsigned long ExtraBallValue = 0;
unsigned long SpecialValue = 0;
unsigned long CurrentTime = 0;
unsigned long SoundSettingTimeout = 0;
unsigned long CreditResetPressStarted = 0;

AudioHandler Audio;


/*********************************************************************

    Game State

*********************************************************************/
byte CurrentPlayer = 0;
byte CurrentBallInPlay = 1;
byte CurrentNumPlayers = 0;
byte BonusCountdownProgress;
byte Bonus[4];
byte BonusX[4];
byte GameMode = GAME_MODE_SKILL_SHOT_PRE_SWITCH;
byte MaxTiltWarnings = 2;
byte NumTiltWarnings = 0;
byte CurrentAchievements[4];

boolean SamePlayerShootsAgain = false;
boolean BallSaveUsed = false;
boolean LowerSpecialCollected[4];
boolean UpperExtraBallCollected[4];
boolean UpperSpecialCollected[4];
boolean ShowingModeStats = false;
boolean GateClosed = true;

unsigned long CurrentScores[4];
unsigned long BallFirstSwitchHitTime = 0;
unsigned long BallTimeInTrough = 0;
unsigned long GameModeStartTime = 0;
unsigned long GameModeEndTime = 0;
unsigned long LastTiltWarningTime = 0;
unsigned long ScoreAdditionAnimation;
unsigned long ScoreAdditionAnimationStartTime;
unsigned long LastRemainingAnimatedScoreShown;
unsigned long PlayfieldMultiplier;
unsigned long PlayfieldMultiplierExpiration;
unsigned long BallSaveEndTime;
unsigned long AnimateSkillShotStartTime;
unsigned long PlayScoreAnimationTick = 2500;

#define BALL_SAVE_GRACE_PERIOD  1500


/*********************************************************************

    Game Specific State Variables

*********************************************************************/
byte SpinnerHits[4];
byte SkillShotNumber;
byte CaptainNumber[4] = {0xFF, 0x01, 0x02, 0x03};
byte MissionLevel[4];
byte MissionsCompleted[4];
byte CurrentMission[4];
byte CurrentMissionLegs[4];
byte NumbersCompleted[4];
byte NumbersLevel[4];
byte DropTargetsLevel;
byte TurbulenceShift = 0;
byte PopBumperPhase;
byte LowerSpecialPhase;
byte GateSetting = 2;
byte GateLevel[4];
byte LegInstructions = 1; // 0=none, 1=first flight, 2=every flight
byte FlightCompletionDifficulty = 1;
byte AdjustedDifficulty;
byte TurbulenceSolenoids[3];
byte TurbulenceFiresSolenoids = 2;
byte LegProgress[3];

#define GATE_SETTING_ALWAYS_CLOSED  0
#define GATE_SETTING_ALWAYS_TIMED   1
#define GATE_SETTING_LIBERAL        2

boolean TimersPaused = true;
boolean DisplaysNeedResetting;
boolean LegStartScoring = false;
boolean TurbulenceMode = false;

unsigned long LastSpinnerHit;
unsigned long BonusXAnimationStart;
unsigned long BonusAnimationStart;
unsigned long LastSwitchHitTime;
unsigned long LastSaucerHitTime;
unsigned long LastModePrompt;
unsigned long LastAlternatingScore;

unsigned long LastTimeThroughLoop = 0;
unsigned long TimerTicks = 0;
unsigned long LastTimeStatusUpdated = 0;
unsigned long SaucerSwitchStartTime;
unsigned long SkillShotAward;
unsigned long NumbersAnimation;

unsigned long CurrentTurbulenceRotation = 0;
unsigned long TurbulenceRotationTarget = 0;
unsigned long LastTurbulenceRotationChange = 0;
unsigned long GateCloseTime = 0;
unsigned long LastTimeNumberHit[5];
unsigned long TurbulenceModeStartTime = 0;
unsigned long SuperJackpotValue;
unsigned long NextTurbulenceSolenoidChange;
unsigned long LastTurbulenceSolFire[3];

DropTargetBank Drop5Targets(5, 1, DROP_TARGET_TYPE_BLY_1, 12);

#define LANE_AND_TARGET_DEBOUNCE_TIME       100

#define MISSION_LEG_BOARDING_COMPLETE       0x01
#define MISSION_LEG_TAKEOFF_COMPLETE        0x02
#define MISSION_LEG_CRUISING_COMPLETE       0x04
#define MISSION_LEG_APPROACH_COMPLETE       0x08
#define MISSION_LEG_LANDING_COMPLETE        0x10

#define MIN_TURBULENCE_ROTATION_PERDIOD     100
#define MAX_TURBULENCE_ROTATION_PERDIOD     250
#define TURBULENCE_ROTATION_PERDIOD_CHANGE  25

byte PossibleTurbulenceSolenoids[] = {SOL_LEFT_POP, SOL_RIGHT_POP, SOL_BOTTOM_POP, SOL_LEFT_SLING, SOL_RIGHT_SLING, SOL_5_BANK_RESET, SOL_KNOCKER};

void ReadStoredParameters() {
  for (byte count = 0; count < 3; count++) {
    ChuteCoinsInProgress[count] = 0;
  }

  HighScore = RPU_ReadULFromEEProm(RPU_HIGHSCORE_EEPROM_START_BYTE, 10000);
  Credits = RPU_ReadByteFromEEProm(RPU_CREDITS_EEPROM_BYTE);
  if (Credits > MaximumCredits) Credits = MaximumCredits;

  ReadSetting(EEPROM_FREE_PLAY_BYTE, 0);
  FreePlayMode = (EEPROM.read(EEPROM_FREE_PLAY_BYTE)) ? true : false;

  BallSaveNumSeconds = ReadSetting(EEPROM_BALL_SAVE_BYTE, 15);
  if (BallSaveNumSeconds > 20) BallSaveNumSeconds = 20;
  CurrentBallSaveNumSeconds = BallSaveNumSeconds;

  SoundSelector = ReadSetting(EEPROM_SOUND_SELECTOR_BYTE, 2);
  if (SoundSelector > 2) SoundSelector = 2;
  SoundtrackSelection = 0;

  MusicVolume = ReadSetting(EEPROM_MUSIC_VOLUME_BYTE, 10);
  if (MusicVolume == 0 || MusicVolume > 10) MusicVolume = 10;

  SoundEffectsVolume = ReadSetting(EEPROM_SFX_VOLUME_BYTE, 10);
  if (SoundEffectsVolume == 0 || SoundEffectsVolume > 10) SoundEffectsVolume = 10;

  CalloutsVolume = ReadSetting(EEPROM_CALLOUTS_VOLUME_BYTE, 10);
  if (CalloutsVolume == 0 || CalloutsVolume > 10) CalloutsVolume = 10;

  Audio.SetMusicVolume(MusicVolume);
  Audio.SetSoundFXVolume(SoundEffectsVolume);
  Audio.SetNotificationsVolume(CalloutsVolume);

  TournamentScoring = (ReadSetting(EEPROM_TOURNAMENT_SCORING_BYTE, 0)) ? true : false;

  MaxTiltWarnings = ReadSetting(EEPROM_TILT_WARNING_BYTE, 2);
  if (MaxTiltWarnings > 2) MaxTiltWarnings = 2;

  byte awardOverride = ReadSetting(EEPROM_AWARD_OVERRIDE_BYTE, 99);
  if (awardOverride != 99) {
    ScoreAwardReplay = awardOverride;
  }

  byte ballsOverride = ReadSetting(EEPROM_BALLS_OVERRIDE_BYTE, 99);
  if (ballsOverride == 3 || ballsOverride == 5) {
    BallsPerGame = ballsOverride;
  } else {
    if (ballsOverride != 99) EEPROM.write(EEPROM_BALLS_OVERRIDE_BYTE, 99);
  }

  ScrollingScores = (ReadSetting(EEPROM_SCROLLING_SCORES_BYTE, 1)) ? true : false;

  ExtraBallValue = RPU_ReadULFromEEProm(EEPROM_EXTRA_BALL_SCORE_BYTE);
  if (ExtraBallValue % 1000 || ExtraBallValue > 100000) ExtraBallValue = 20000;

  SpecialValue = RPU_ReadULFromEEProm(EEPROM_SPECIAL_SCORE_BYTE);
  if (SpecialValue % 1000 || SpecialValue > 100000) SpecialValue = 40000;

  TimeRequiredToResetGame = ReadSetting(EEPROM_CRB_HOLD_TIME, 1);
  if (TimeRequiredToResetGame > 3 && TimeRequiredToResetGame != 99) TimeRequiredToResetGame = 1;

  GateSetting = ReadSetting(EEPROM_GATE_SETTING, 2);
  if (GateSetting>2) GateSetting = 2;

  LegInstructions = ReadSetting(EEPROM_FLIGHT_INSTRUCTION, 1);
  if (LegInstructions>2) LegInstructions = 1;

  FlightCompletionDifficulty = ReadSetting(EEPROM_LEG_DIFFICULTY, 3);
  if (FlightCompletionDifficulty>15) FlightCompletionDifficulty = 3;

  TurbulenceFiresSolenoids = ReadSetting(EEPROM_TURBULENCE, 2);
  if (TurbulenceFiresSolenoids>2) TurbulenceFiresSolenoids = 2;

  AwardScores[0] = RPU_ReadULFromEEProm(RPU_AWARD_SCORE_1_EEPROM_START_BYTE);
  AwardScores[1] = RPU_ReadULFromEEProm(RPU_AWARD_SCORE_2_EEPROM_START_BYTE);
  AwardScores[2] = RPU_ReadULFromEEProm(RPU_AWARD_SCORE_3_EEPROM_START_BYTE);

}


void QueueDIAGNotification(unsigned short notificationNum) {
  Audio.QueuePrioritizedNotification(notificationNum, DiagnosticNotificationDurations[notificationNum - SOUND_EFFECT_DIAG_START] + 1500, 10, CurrentTime);
}

void setup() {
  if (DEBUG_MESSAGES) {
    Serial.begin(115200);
  }

  CurrentTime = millis();
  Audio.InitDevices(AUDIO_PLAY_TYPE_WAV_TRIGGER | AUDIO_PLAY_TYPE_ORIGINAL_SOUNDS);
  Audio.StopAllAudio();

  // Tell the OS about game-specific lights and switches
  RPU_SetupGameSwitches(NUM_SWITCHES_WITH_TRIGGERS, NUM_PRIORITY_SWITCHES_WITH_TRIGGERS, SolenoidAssociatedSwitches);

  // Set up the chips and interrupts
  unsigned long initResult = 0;
  initResult = RPU_InitializeMPU(RPU_CMD_BOOT_ORIGINAL_IF_CREDIT_RESET | RPU_CMD_INIT_AND_RETURN_EVEN_IF_ORIGINAL_CHOSEN | RPU_CMD_PERFORM_MPU_TEST, SW_CREDIT_RESET);

  if (initResult & RPU_RET_SELECTOR_SWITCH_ON) QueueDIAGNotification(SOUND_EFFECT_DIAG_SELECTOR_SWITCH_ON);
  else QueueDIAGNotification(SOUND_EFFECT_DIAG_SELECTOR_SWITCH_OFF);
  if (initResult & RPU_RET_CREDIT_RESET_BUTTON_HIT) QueueDIAGNotification(SOUND_EFFECT_DIAG_CREDIT_RESET_BUTTON);

  if (initResult & RPU_RET_ORIGINAL_CODE_REQUESTED) {
    QueueDIAGNotification(SOUND_EFFECT_DIAG_STARTING_ORIGINAL_CODE);
    while (Audio.Update(millis()));
    // Arduino should hang if original code is running
    while (1);
  }
  QueueDIAGNotification(SOUND_EFFECT_DIAG_STARTING_NEW_CODE);
  RPU_DisableSolenoidStack();
  RPU_SetDisableFlippers(true);

  // Read parameters from EEProm
  ReadStoredParameters();
  RPU_SetCoinLockout((Credits >= MaximumCredits) ? true : false);

  CurrentScores[0] = CONCORD_2022_MAJOR_VERSION;
  CurrentScores[1] = CONCORD_2022_MINOR_VERSION;
  CurrentScores[2] = RPU_OS_MAJOR_VERSION;
  CurrentScores[3] = RPU_OS_MINOR_VERSION;

  delayMicroseconds(10000);
  Audio.SetMusicDuckingGain(20);
  Audio.PlaySound(SOUND_EFFECT_MACHINE_START, AUDIO_PLAY_TYPE_WAV_TRIGGER);

  Drop5Targets.DefineSwitch(0, SW_DROP_TARGET_5);
  Drop5Targets.DefineSwitch(1, SW_DROP_TARGET_4);
  Drop5Targets.DefineSwitch(2, SW_DROP_TARGET_3);
  Drop5Targets.DefineSwitch(3, SW_DROP_TARGET_2);
  Drop5Targets.DefineSwitch(4, SW_DROP_TARGET_1);
  Drop5Targets.DefineResetSolenoid(0, SOL_5_BANK_RESET);

  MachineState = MACHINE_STATE_ATTRACT;
}

byte ReadSetting(byte setting, byte defaultValue) {
  byte value = EEPROM.read(setting);
  if (value == 0xFF) {
    EEPROM.write(setting, defaultValue);
    return defaultValue;
  }
  return value;
}


// This function is useful for checking the status of drop target switches
byte CheckSequentialSwitches(byte startingSwitch, byte numSwitches) {
  byte returnSwitches = 0;
  for (byte count = 0; count < numSwitches; count++) {
    returnSwitches |= (RPU_ReadSingleSwitchState(startingSwitch + count) << count);
  }
  return returnSwitches;
}



////////////////////////////////////////////////////////////////////////////
//
//  Lamp Management functions
//
////////////////////////////////////////////////////////////////////////////
void SetGatePosition(boolean gateClosed) {

  if (gateClosed != GateClosed) {
    GateClosed = gateClosed;
    RPU_SetContinuousSolenoidBit(GateClosed, SOLCONT_GATE);
    if (gateClosed) GateCloseTime = 0;
  }
}


void ShowBonusXLamps() {

  if (GameMode == GAME_MODE_SKILL_SHOT_PRE_SWITCH || GameMode == GAME_MODE_SKILL_SHOT_BUILD_BONUS) {
    for (byte count = 0; count < 4; count++) RPU_SetLampState(LAMP_5X_BONUS + count, 0);
  } else if (GameMode == GAME_MODE_LANDING || GameMode == GAME_MODE_SPECIAL_TOUR_SAUCER) {
    byte lampPhase = ((CurrentTime / 100) % 4);
    for (byte count = 0; count < 4; count++) RPU_SetLampState(LAMP_2X_BONUS - count, count == lampPhase);
  } else {
    RPU_SetLampState(LAMP_5X_BONUS, BonusX[CurrentPlayer] == 5, 0, BonusXAnimationStart ? 160 : 0);
    RPU_SetLampState(LAMP_3X_5X_BONUS, BonusX[CurrentPlayer] == 3, 0, BonusXAnimationStart ? 160 : 0);
    RPU_SetLampState(LAMP_2X_3X_BONUS, BonusX[CurrentPlayer] == 2, 0, BonusXAnimationStart ? 160 : 0);
    RPU_SetLampState(LAMP_2X_BONUS, BonusX[CurrentPlayer] == 1, 0, BonusXAnimationStart ? 160 : 0);
  }

  if (BonusXAnimationStart && CurrentTime>(BonusXAnimationStart+5000)) {
    BonusXAnimationStart = 0;
  }

}


void ShowSpinnerLamp() {
  if (GameMode == GAME_MODE_SKILL_SHOT_PRE_SWITCH || GameMode == GAME_MODE_SKILL_SHOT_BUILD_BONUS) {
    RPU_SetLampState(LAMP_SPINNER, 0);
  } else if (GameMode == GAME_MODE_CRUISING || GameMode ==  GAME_MODE_SPECIAL_TOUR_SPINNER) {
    RPU_SetLampState(LAMP_SPINNER, 1, 0, LegStartScoring ? 100 : 500);
  } else {
    RPU_SetLampState(LAMP_SPINNER, 0);
  }
}


void ShowDropTargetLamps() {
  if (GameMode == GAME_MODE_SKILL_SHOT_PRE_SWITCH || GameMode == GAME_MODE_SKILL_SHOT_BUILD_BONUS) {
    RPU_SetLampState(LAMP_DT_SPECIAL, 0);
    RPU_SetLampState(LAMP_DT_EXTRA_BALL, 0);
  } else if (GameMode == GAME_MODE_APPROACH || GameMode == GAME_MODE_SPECIAL_TOUR_DROPS) {
    byte lampPhase = (CurrentTime / 250) % 2;
    if (LegStartScoring) lampPhase = (CurrentTime / 100) % 2;
    RPU_SetLampState(LAMP_DT_SPECIAL, lampPhase == 0);
    RPU_SetLampState(LAMP_DT_EXTRA_BALL, lampPhase == 1);
  } else {
    RPU_SetLampState(LAMP_DT_SPECIAL, (DropTargetsLevel==3 && !UpperSpecialCollected[CurrentPlayer])?true:false);
    RPU_SetLampState(LAMP_DT_EXTRA_BALL, (DropTargetsLevel==2 && !UpperExtraBallCollected[CurrentPlayer])?true:false);
  }
}


void ShowBonusLamps() {
  if (GameMode == GAME_MODE_SKILL_SHOT_PRE_SWITCH) {
    for (byte count = 0; count < 11; count++) RPU_SetLampState(LAMP_BONUS_1K + count, 0);
  } else {
    byte bonusToShow = Bonus[CurrentPlayer];
    if (BonusAnimationStart) {
      bonusToShow = 1 + ((CurrentTime - BonusAnimationStart) / 25);
      if (bonusToShow >= Bonus[CurrentPlayer]) {
        BonusAnimationStart = 0;
        bonusToShow = Bonus[CurrentPlayer];
      }
    }

    RPU_SetLampState(LAMP_BONUS_20K, bonusToShow > 19, 0, (bonusToShow > 39) ? 400 : 0);
    RPU_SetLampState(LAMP_BONUS_10K, (bonusToShow % 20) > 9);
    byte effectiveOnes = bonusToShow % 10;
    if (bonusToShow!=0 && effectiveOnes == 0) effectiveOnes = 10;
    for (byte count = 1; count < 10; count++) RPU_SetLampState(LAMP_BONUS_1K + (count - 1), effectiveOnes >= count);
  }
}

byte LaneLampArray[] = {LAMP_1, LAMP_2, LAMP_3, LAMP_4, LAMP_5};

void ShowTopLaneLamps() {
  if (GameMode == GAME_MODE_SKILL_SHOT_PRE_SWITCH || GameMode == GAME_MODE_SKILL_SHOT_BUILD_BONUS) {
    byte bitMask = 0x01;
    for (byte count = 0; count < 5; count++) {
      if (count == SkillShotNumber) RPU_SetLampState(LaneLampArray[count], count == SkillShotNumber, 0, 175);
      else RPU_SetLampState(LaneLampArray[count], (NumbersCompleted[CurrentPlayer]&bitMask) == 0, 1);
      bitMask *= 2;
    }
  } else if (GameMode == GAME_MODE_SPECIAL_TOUR_NUMBERS) {
    byte lampPhase = (CurrentTime/100)%5;
    for (byte count = 0; count < 5; count++) {
      RPU_SetLampState(LaneLampArray[count], count==lampPhase);
    }
  } else if (GameMode == GAME_MODE_COLLECT_MILES) {
    byte lampPhase = (CurrentTime/100)%2;
    RPU_SetLampState(LAMP_1, lampPhase);    
    for (byte count = 1; count < 5; count++) {
      RPU_SetLampState(LaneLampArray[count], 0);
    }
  } else { /*if (GameMode==GAME_MODE_UNSTRUCTURED_PLAY)*/

    if (NumbersAnimation) {
      for (byte count = 0; count < 5; count++) {
        RPU_SetLampState(LaneLampArray[count], 1, 0, 100);
      }
    } else {
      byte bitMask = 0x01;
      for (byte count = 0; count < 5; count++) {
        if (count != CurrentMission[CurrentPlayer]) {
          RPU_SetLampState(LaneLampArray[count], (NumbersCompleted[CurrentPlayer]&bitMask) ? false : true, 1);
        } else {
          if ( (CurrentTime / 500) % 2 ) RPU_SetLampState(LaneLampArray[count], 1);
          else RPU_SetLampState(LaneLampArray[count], (NumbersCompleted[CurrentPlayer]&bitMask) ? false : true, 1);
        }
        bitMask *= 2;
      }
    }

    if (NumbersAnimation && CurrentTime > (NumbersAnimation + 5000)) {
      NumbersAnimation = 0;
    }
  }
}


unsigned long LastCenterLampChange = 0;
unsigned long CenterLampDelta = 200;
byte CenterLampSpeedDirection = 0;
byte CenterLampSelector = 0x01;
byte CenterLampArray[] = {LAMP_1_ROLLOVER, LAMP_2_ROLLOVER, LAMP_3_ROLLOVER, LAMP_4_ROLLOVER, LAMP_5_ROLLOVER};

unsigned short ShiftCenterLamps(unsigned short curVal, byte shiftNum) {
  shiftNum = shiftNum%5;
  unsigned short newVal = 0;

  for (byte count=0; count<shiftNum; count++) {
    newVal = (curVal & 0x03)*2;
    if (curVal&0x08) newVal |= 0x01;
    if (curVal&0x10) newVal |= 0x08;
    if (curVal&0x04) newVal |= 0x10;
    curVal = newVal;
  }

  return curVal;
}

void ShowCenterRolloverLamps() {
  if (GameMode == GAME_MODE_SKILL_SHOT_PRE_SWITCH || GameMode == GAME_MODE_SKILL_SHOT_BUILD_BONUS || GameMode > GAME_MODE_SPECIAL_TOUR_START) {
    if (LastCenterLampChange == 0) LastCenterLampChange = CurrentTime;

    if (CurrentTime > (LastCenterLampChange + CenterLampDelta)) {
      CenterLampSelector = ShiftCenterLamps(CenterLampSelector, 1);
      if (CenterLampSelector==0x01) {
        if (CenterLampSpeedDirection == 0) {
          CenterLampDelta -= 10;
          if (CenterLampDelta < 60) CenterLampSpeedDirection = 1;
        } else {
          CenterLampDelta += 10;
          if (CenterLampDelta > 200) CenterLampSpeedDirection = 0;
        }
      }
      LastCenterLampChange = CurrentTime;
    }

    byte bitMask = 0x01;
    for (byte count = 0; count < 5; count++) {
      RPU_SetLampState(CenterLampArray[count], CenterLampSelector&bitMask);
      bitMask *= 2;
    }

  } else {
    if (LastTurbulenceRotationChange) {
      unsigned short lampsByte = (unsigned short)(MissionsCompleted[CurrentPlayer]);
      if (CurrentMission[CurrentPlayer] != 0xFF) lampsByte |= (0x01 << CurrentMission[CurrentPlayer]);

      // shift with adjustment for 4/5 swap
      lampsByte = ShiftCenterLamps(lampsByte, TurbulenceShift);

      unsigned short bitMask = 0x01;
      for (byte count = 0; count < 5; count++) {
        RPU_SetLampState(CenterLampArray[count], (lampsByte & bitMask) ? true : false);
        bitMask *= 2;
      }

    } else {
      byte bitMask = 0x01;
      for (byte count = 0; count < 5; count++) {
        if (CurrentMission[CurrentPlayer] == count) RPU_SetLampState(CenterLampArray[count], 1, 0, 250);
        else RPU_SetLampState(CenterLampArray[count], (MissionsCompleted[CurrentPlayer] & bitMask) ? true : false);
        bitMask *= 2;
      }
    }
  }
}

byte LeftLaneLampArray[] = {LAMP_1_LEFT_LANE, LAMP_2_LEFT_LANE, LAMP_3_LEFT_LANE, LAMP_4_LEFT_LANE, LAMP_5_LEFT_LANE};

void ShowLeftLaneLamps() {
  if (GameMode == GAME_MODE_SKILL_SHOT_PRE_SWITCH || GameMode == GAME_MODE_SKILL_SHOT_BUILD_BONUS) {
    for (byte count = 0; count < 5; count++) {
      RPU_SetLampState(LeftLaneLampArray[count], 0);
    }
  } else if (GameMode == GAME_MODE_TAKEOFF || GameMode == GAME_MODE_SPECIAL_TOUR_LEFT_LANE) {
    byte lampPhase = (CurrentTime / 200) % 5;
    if (LegStartScoring) lampPhase = (CurrentTime / 100) % 5;
    for (byte count = 0; count < 5; count++) {
      RPU_SetLampState(LeftLaneLampArray[count], lampPhase == count);
    }
  } else {
    byte bitMask = 0x01;
    byte flashSpeed = 0;
    if (NumbersLevel[CurrentPlayer] > 1) flashSpeed = 1000 / ((int)NumbersLevel[CurrentPlayer]);

    for (byte count = 0; count < 5; count++) {
      RPU_SetLampState(LeftLaneLampArray[count], (NumbersLevel[CurrentPlayer] && (NumbersCompleted[CurrentPlayer] & bitMask)) ? true : false, 0, flashSpeed);
      bitMask *= 2;
    }
  }

}


void ShowShootAgainLamp() {

  if (!BallSaveUsed && CurrentBallSaveNumSeconds > 0 && (CurrentTime < BallSaveEndTime)) {
    unsigned long msRemaining = 5000;
    if (BallSaveEndTime != 0) msRemaining = BallSaveEndTime - CurrentTime;
    RPU_SetLampState(LAMP_SHOOT_AGAIN, 1, 0, (msRemaining < 5000) ? 100 : 500);
  } else {
    RPU_SetLampState(LAMP_SHOOT_AGAIN, SamePlayerShootsAgain);
  }
}


void ShowGateLamps() {
  int gateFlash = 0;
  if (((CurrentTime+5000) > GateCloseTime) && GateCloseTime) gateFlash = 250;
  RPU_SetLampState(LAMP_GATE_OPEN, !GateClosed, 0, gateFlash);

  if (GateSetting==GATE_SETTING_ALWAYS_CLOSED) {
    RPU_SetLampState(LAMP_OPENS_GATE, 0);
  } else if (GateSetting==GATE_SETTING_ALWAYS_TIMED) {
    RPU_SetLampState(LAMP_OPENS_GATE, GateClosed);
  } else if (GateSetting==GATE_SETTING_LIBERAL) {
    int gateOpenFlash = 0;
    if (GateLevel[CurrentPlayer]==1) gateOpenFlash = 250;
    else if (GateLevel[CurrentPlayer]==2) gateOpenFlash = 125;
    RPU_SetLampState(LAMP_OPENS_GATE, GateLevel[CurrentPlayer]<3, 0, gateOpenFlash);
  }
}


void ShowOtherScoringLamps() {

  if (GameMode == GAME_MODE_SKILL_SHOT_PRE_SWITCH || GameMode == GAME_MODE_SKILL_SHOT_BUILD_BONUS) {
    RPU_SetLampState(LAMP_TOP_POPS, 0);
    RPU_SetLampState(LAMP_BOTTOM_POP, 0);
    RPU_SetLampState(LAMP_JET_STREAM_AND_CENTER, 0);
    RPU_SetLampState(LAMP_TOP_3K_WHEN_LIT, 0);
    RPU_SetLampState(LAMP_RIGHT_OUTLANE_SPECIAL, (!LowerSpecialCollected[CurrentPlayer] && NumbersLevel[CurrentPlayer]>2 && LowerSpecialPhase==0)?true:false);
    RPU_SetLampState(LAMP_LEFT_OUTLANE_SPECIAL, (!LowerSpecialCollected[CurrentPlayer] && NumbersLevel[CurrentPlayer]>2 && LowerSpecialPhase==1)?true:false);
    RPU_SetLampState(LAMP_RIGHT_TARGETS_3K, 0);
    RPU_SetLampState(LAMP_CENTER_TARGET_3K, 0);
  } else if (GameMode == GAME_MODE_COLLECT_MILES) {
    byte lampPhase = (CurrentTime/100)%2;
    RPU_SetLampState(LAMP_CENTER_TARGET_3K, lampPhase==0);    
    RPU_SetLampState(LAMP_TOP_POPS, 0);
    RPU_SetLampState(LAMP_BOTTOM_POP, 0);
    RPU_SetLampState(LAMP_JET_STREAM_AND_CENTER, 0);
    RPU_SetLampState(LAMP_TOP_3K_WHEN_LIT, 0);
    RPU_SetLampState(LAMP_RIGHT_OUTLANE_SPECIAL, 0);
    RPU_SetLampState(LAMP_LEFT_OUTLANE_SPECIAL, 0);
    RPU_SetLampState(LAMP_RIGHT_TARGETS_3K, 0);
  } else {
    if (TurbulenceMode || (GameMode>GAME_MODE_SPECIAL_TOUR_START)) {
      byte turbulencePhase = (CurrentTime/100)%2;
      RPU_SetLampState(LAMP_TOP_POPS, turbulencePhase);
      RPU_SetLampState(LAMP_BOTTOM_POP, !turbulencePhase);
    } else {
      RPU_SetLampState(LAMP_TOP_POPS, CurrentTurbulenceRotation && (PopBumperPhase==0)?true:false);
      RPU_SetLampState(LAMP_BOTTOM_POP, CurrentTurbulenceRotation && (PopBumperPhase==1)?true:false);
    }
    RPU_SetLampState(LAMP_JET_STREAM_AND_CENTER, NumbersLevel[CurrentPlayer]);
    RPU_SetLampState(LAMP_TOP_3K_WHEN_LIT, CurrentMission[CurrentPlayer] == 0xFF, 0, 220);
    RPU_SetLampState(LAMP_RIGHT_OUTLANE_SPECIAL, 0);
    RPU_SetLampState(LAMP_LEFT_OUTLANE_SPECIAL, 0);
    RPU_SetLampState(LAMP_RIGHT_TARGETS_3K, 0);
    RPU_SetLampState(LAMP_CENTER_TARGET_3K, 0);
  }
  

}


////////////////////////////////////////////////////////////////////////////
//
//  Display Management functions
//
////////////////////////////////////////////////////////////////////////////
unsigned long LastTimeScoreChanged = 0;
unsigned long LastFlashOrDash = 0;
unsigned long ScoreOverrideValue[4] = {0, 0, 0, 0};
byte LastAnimationSeed[4] = {0, 0, 0, 0};
byte AnimationStartSeed[4] = {0, 0, 0, 0};
byte ScoreOverrideStatus = 0;
byte ScoreAnimation[4] = {0, 0, 0, 0};
byte AnimationDisplayOrder[4] = {0, 1, 2, 3};
#define DISPLAY_OVERRIDE_BLANK_SCORE 0xFFFFFFFF
#define DISPLAY_OVERRIDE_ANIMATION_NONE     0
#define DISPLAY_OVERRIDE_ANIMATION_BOUNCE   1
#define DISPLAY_OVERRIDE_ANIMATION_FLUTTER  2
#define DISPLAY_OVERRIDE_ANIMATION_FLYBY    3
#define DISPLAY_OVERRIDE_ANIMATION_CENTER   4
byte LastScrollPhase = 0;

byte MagnitudeOfScore(unsigned long score) {
  if (score == 0) return 0;

  byte retval = 0;
  while (score > 0) {
    score = score / 10;
    retval += 1;
  }
  return retval;
}


void OverrideScoreDisplay(byte displayNum, unsigned long value, byte animationType) {
  if (displayNum > 3) return;

  ScoreOverrideStatus |= (0x01 << displayNum);
  ScoreAnimation[displayNum] = animationType;
  ScoreOverrideValue[displayNum] = value;
  LastAnimationSeed[displayNum] = 255;
}

byte GetDisplayMask(byte numDigits) {
  byte displayMask = 0;
  for (byte digitCount = 0; digitCount < numDigits; digitCount++) {
#ifdef RPU_OS_USE_7_DIGIT_DISPLAYS
    displayMask |= (0x40 >> digitCount);
#else
    displayMask |= (0x20 >> digitCount);
#endif
  }
  return displayMask;
}


void SetAnimationDisplayOrder(byte disp0, byte disp1, byte disp2, byte disp3) {
  AnimationDisplayOrder[0] = disp0;
  AnimationDisplayOrder[1] = disp1;
  AnimationDisplayOrder[2] = disp2;
  AnimationDisplayOrder[3] = disp3;
}


void ShowAnimatedValue(byte displayNum, unsigned long displayScore, byte animationType) {
  byte overrideAnimationSeed;
  byte displayMask = RPU_OS_ALL_DIGITS_MASK;

  byte numDigits = MagnitudeOfScore(displayScore);
  if (numDigits == 0) numDigits = 1;
  if (numDigits < (RPU_OS_NUM_DIGITS - 1) && animationType == DISPLAY_OVERRIDE_ANIMATION_BOUNCE) {
    // This score is going to be animated (back and forth)
    overrideAnimationSeed = (CurrentTime / 250) % (2 * RPU_OS_NUM_DIGITS - 2 * numDigits);
    if (overrideAnimationSeed != LastAnimationSeed[displayNum]) {

      LastAnimationSeed[displayNum] = overrideAnimationSeed;
      byte shiftDigits = (overrideAnimationSeed);
      if (shiftDigits >= ((RPU_OS_NUM_DIGITS + 1) - numDigits)) shiftDigits = (RPU_OS_NUM_DIGITS - numDigits) * 2 - shiftDigits;
      byte digitCount;
      displayMask = GetDisplayMask(numDigits);
      for (digitCount = 0; digitCount < shiftDigits; digitCount++) {
        displayScore *= 10;
        displayMask = displayMask >> 1;
      }
      //RPU_SetDisplayBlank(displayNum, 0x00);
      RPU_SetDisplay(displayNum, displayScore, false);
      RPU_SetDisplayBlank(displayNum, displayMask);
    }
  } else if (animationType == DISPLAY_OVERRIDE_ANIMATION_FLUTTER) {
    overrideAnimationSeed = CurrentTime / 50;
    if (overrideAnimationSeed != LastAnimationSeed[displayNum]) {
      LastAnimationSeed[displayNum] = overrideAnimationSeed;
      displayMask = GetDisplayMask(numDigits);
      if (overrideAnimationSeed % 2) {
        displayMask &= 0x55;
      } else {
        displayMask &= 0xAA;
      }
      RPU_SetDisplay(displayNum, displayScore, false);
      RPU_SetDisplayBlank(displayNum, displayMask);
    }
  } else if (animationType == DISPLAY_OVERRIDE_ANIMATION_FLYBY) {
    overrideAnimationSeed = (CurrentTime / 75) % 256;
    if (overrideAnimationSeed != LastAnimationSeed[displayNum]) {
      if (LastAnimationSeed[displayNum] == 255) {
        AnimationStartSeed[displayNum] = overrideAnimationSeed;
      }
      LastAnimationSeed[displayNum] = overrideAnimationSeed;

      byte realAnimationSeed = overrideAnimationSeed - AnimationStartSeed[displayNum];
      if (overrideAnimationSeed < AnimationStartSeed[displayNum]) realAnimationSeed = (255 - AnimationStartSeed[displayNum]) + overrideAnimationSeed;

      if (realAnimationSeed > 34) {
        RPU_SetDisplayBlank(displayNum, 0x00);
        ScoreOverrideStatus &= ~(0x01 << displayNum);
      } else {
        int shiftDigits = (-6 * ((int)AnimationDisplayOrder[displayNum] + 1)) + realAnimationSeed;
        displayMask = GetDisplayMask(numDigits);
        if (shiftDigits < 0) {
          shiftDigits = 0 - shiftDigits;
          byte digitCount;
          for (digitCount = 0; digitCount < shiftDigits; digitCount++) {
            displayScore /= 10;
            displayMask = displayMask << 1;
          }
        } else if (shiftDigits > 0) {
          byte digitCount;
          for (digitCount = 0; digitCount < shiftDigits; digitCount++) {
            displayScore *= 10;
            displayMask = displayMask >> 1;
          }
        }
        RPU_SetDisplay(displayNum, displayScore, false);
        RPU_SetDisplayBlank(displayNum, displayMask);
      }
    }
  } else if (animationType == DISPLAY_OVERRIDE_ANIMATION_CENTER) {
    overrideAnimationSeed = CurrentTime / 250;
    if (overrideAnimationSeed != LastAnimationSeed[displayNum]) {
      LastAnimationSeed[displayNum] = overrideAnimationSeed;
      byte shiftDigits = (RPU_OS_NUM_DIGITS - numDigits) / 2;

      byte digitCount;
      displayMask = GetDisplayMask(numDigits);
      for (digitCount = 0; digitCount < shiftDigits; digitCount++) {
        displayScore *= 10;
        displayMask = displayMask >> 1;
      }
      //RPU_SetDisplayBlank(displayNum, 0x00);
      RPU_SetDisplay(displayNum, displayScore, false);
      RPU_SetDisplayBlank(displayNum, displayMask);
    }
  } else {
    RPU_SetDisplay(displayNum, displayScore, true, 1);
  }

}

void ShowPlayerScores(byte displayToUpdate, boolean flashCurrent, boolean dashCurrent, unsigned long allScoresShowValue = 0) {

  if (displayToUpdate == 0xFF) ScoreOverrideStatus = 0;
  byte displayMask = RPU_OS_ALL_DIGITS_MASK;
  unsigned long displayScore = 0;
  byte scrollPhaseChanged = false;

  byte scrollPhase = ((CurrentTime - LastTimeScoreChanged) / 125) % 16;
  if (scrollPhase != LastScrollPhase) {
    LastScrollPhase = scrollPhase;
    scrollPhaseChanged = true;
  }

  for (byte scoreCount = 0; scoreCount < 4; scoreCount++) {

    // If this display is currently being overriden, then we should update it
    if (allScoresShowValue == 0 && (ScoreOverrideStatus & (0x01 << scoreCount))) {
      displayScore = ScoreOverrideValue[scoreCount];
      if (displayScore != DISPLAY_OVERRIDE_BLANK_SCORE) {
        ShowAnimatedValue(scoreCount, displayScore, ScoreAnimation[scoreCount]);
      } else {
        RPU_SetDisplayBlank(scoreCount, 0);
      }

    } else {
      boolean showingCurrentAchievement = false;
      // No override, update scores designated by displayToUpdate
      if (allScoresShowValue == 0) {
        displayScore = CurrentScores[scoreCount];
        displayScore += (CurrentAchievements[scoreCount] % 10);
        if (CurrentAchievements[scoreCount]) showingCurrentAchievement = true;
      }
      else displayScore = allScoresShowValue;

      // If we're updating all displays, or the one currently matching the loop, or if we have to scroll
      if (displayToUpdate == 0xFF || displayToUpdate == scoreCount || displayScore > RPU_OS_MAX_DISPLAY_SCORE || showingCurrentAchievement) {

        // Don't show this score if it's not a current player score (even if it's scrollable)
        if (displayToUpdate == 0xFF && (scoreCount >= CurrentNumPlayers && CurrentNumPlayers != 0) && allScoresShowValue == 0) {
          RPU_SetDisplayBlank(scoreCount, 0x00);
          continue;
        }

        if (displayScore > RPU_OS_MAX_DISPLAY_SCORE) {
          // Score needs to be scrolled
          if ((CurrentTime - LastTimeScoreChanged) < 2000) {
            // show score for four seconds after change
            RPU_SetDisplay(scoreCount, displayScore % (RPU_OS_MAX_DISPLAY_SCORE + 1), false);
            byte blank = RPU_OS_ALL_DIGITS_MASK;
            if (showingCurrentAchievement && (CurrentTime / 200) % 2) {
              blank &= ~(0x01 << (RPU_OS_NUM_DIGITS - 1));
            }
            RPU_SetDisplayBlank(scoreCount, blank);
          } else {
            // Scores are scrolled 10 digits and then we wait for 6
            if (scrollPhase < 11 && scrollPhaseChanged) {
              byte numDigits = MagnitudeOfScore(displayScore);

              // Figure out top part of score
              unsigned long tempScore = displayScore;
              if (scrollPhase < RPU_OS_NUM_DIGITS) {
                displayMask = RPU_OS_ALL_DIGITS_MASK;
                for (byte scrollCount = 0; scrollCount < scrollPhase; scrollCount++) {
                  displayScore = (displayScore % (RPU_OS_MAX_DISPLAY_SCORE + 1)) * 10;
                  displayMask = displayMask >> 1;
                }
              } else {
                displayScore = 0;
                displayMask = 0x00;
              }

              // Add in lower part of score
              if ((numDigits + scrollPhase) > 10) {
                byte numDigitsNeeded = (numDigits + scrollPhase) - 10;
                for (byte scrollCount = 0; scrollCount < (numDigits - numDigitsNeeded); scrollCount++) {
                  tempScore /= 10;
                }
                displayMask |= GetDisplayMask(MagnitudeOfScore(tempScore));
                displayScore += tempScore;
              }
              RPU_SetDisplayBlank(scoreCount, displayMask);
              RPU_SetDisplay(scoreCount, displayScore);
            }
          }
        } else {
          if (flashCurrent && displayToUpdate == scoreCount) {
            unsigned long flashSeed = CurrentTime / 250;
            if (flashSeed != LastFlashOrDash) {
              LastFlashOrDash = flashSeed;
              if (((CurrentTime / 250) % 2) == 0) RPU_SetDisplayBlank(scoreCount, 0x00);
              else RPU_SetDisplay(scoreCount, displayScore, true, 2);
            }
          } else if (dashCurrent && displayToUpdate == scoreCount) {
            unsigned long dashSeed = CurrentTime / 50;
            if (dashSeed != LastFlashOrDash) {
              LastFlashOrDash = dashSeed;
              byte dashPhase = (CurrentTime / 60) % (2 * RPU_OS_NUM_DIGITS * 3);
              byte numDigits = MagnitudeOfScore(displayScore);
              if (dashPhase < (2 * RPU_OS_NUM_DIGITS)) {
                displayMask = GetDisplayMask((numDigits == 0) ? 2 : numDigits);
                if (dashPhase < (RPU_OS_NUM_DIGITS + 1)) {
                  for (byte maskCount = 0; maskCount < dashPhase; maskCount++) {
                    displayMask &= ~(0x01 << maskCount);
                  }
                } else {
                  for (byte maskCount = (2 * RPU_OS_NUM_DIGITS); maskCount > dashPhase; maskCount--) {
                    byte firstDigit = (0x20) << (RPU_OS_NUM_DIGITS - 6);
                    displayMask &= ~(firstDigit >> (maskCount - dashPhase - 1));
                  }
                }
                RPU_SetDisplay(scoreCount, displayScore);
                RPU_SetDisplayBlank(scoreCount, displayMask);
              } else {
                RPU_SetDisplay(scoreCount, displayScore, true, 2);
              }
            }
          } else {
            byte blank;
            blank = RPU_SetDisplay(scoreCount, displayScore, false, 2);
            if (showingCurrentAchievement && (CurrentTime / 200) % 2) {
              blank &= ~(0x01 << (RPU_OS_NUM_DIGITS - 1));
            }
            RPU_SetDisplayBlank(scoreCount, blank);
          }
        }
      } // End if this display should be updated
    } // End on non-overridden
  } // End loop on scores

}

void ShowFlybyValue(byte numToShow, unsigned long timeBase) {
  byte shiftDigits = (CurrentTime - timeBase) / 120;
  byte rightSideBlank = 0;

  unsigned long bigVersionOfNum = (unsigned long)numToShow;
  for (byte count = 0; count < shiftDigits; count++) {
    bigVersionOfNum *= 10;
    rightSideBlank /= 2;
    if (count > 2) rightSideBlank |= 0x20;
  }
  bigVersionOfNum /= 1000;

  byte curMask = RPU_SetDisplay(CurrentPlayer, bigVersionOfNum, false, 0);
  if (bigVersionOfNum == 0) curMask = 0;
  RPU_SetDisplayBlank(CurrentPlayer, ~(~curMask | rightSideBlank));
}


void StartScoreAnimation(unsigned long scoreToAnimate, boolean playTick = true) {
  if (ScoreAdditionAnimation != 0) {
    //CurrentScores[CurrentPlayer] += ScoreAdditionAnimation;
  }
  ScoreAdditionAnimation += scoreToAnimate;
  ScoreAdditionAnimationStartTime = CurrentTime;
  LastRemainingAnimatedScoreShown = 0;
  if (playTick) {
    PlayScoreAnimationTick = 10000;
    if (scoreToAnimate<=10000) PlayScoreAnimationTick = 2500;
    else if (scoreToAnimate<=25000) PlayScoreAnimationTick = 5000;
    else if (scoreToAnimate<=50000) PlayScoreAnimationTick = 7500;
  } else {
    PlayScoreAnimationTick = 1;
  }
}



////////////////////////////////////////////////////////////////////////////
//
//  Machine State Helper functions
//
////////////////////////////////////////////////////////////////////////////
boolean AddPlayer(boolean resetNumPlayers = false) {

  if (Credits < 1 && !FreePlayMode) return false;
  if (resetNumPlayers) CurrentNumPlayers = 0;
  if (CurrentNumPlayers >= 4) return false;

  CurrentNumPlayers += 1;
  RPU_SetDisplay(CurrentNumPlayers - 1, 0, true, 2);
  //  RPU_SetDisplayBlank(CurrentNumPlayers - 1, 0x30);

  if (!FreePlayMode) {
    Credits -= 1;
    RPU_WriteByteToEEProm(RPU_CREDITS_EEPROM_BYTE, Credits);
    RPU_SetDisplayCredits(Credits, !FreePlayMode);
    RPU_SetCoinLockout(false);
  }
  if (CurrentNumPlayers>1) QueueNotification(SOUND_EFFECT_VP_ADD_PLAYER_1 + (CurrentNumPlayers - 1), 10);

  RPU_WriteULToEEProm(RPU_TOTAL_PLAYS_EEPROM_START_BYTE, RPU_ReadULFromEEProm(RPU_TOTAL_PLAYS_EEPROM_START_BYTE) + 1);

  return true;
}


unsigned short ChuteAuditByte[] = {RPU_CHUTE_1_COINS_START_BYTE, RPU_CHUTE_2_COINS_START_BYTE, RPU_CHUTE_3_COINS_START_BYTE};
void AddCoinToAudit(byte chuteNum) {
  if (chuteNum > 2) return;
  unsigned short coinAuditStartByte = ChuteAuditByte[chuteNum];
  RPU_WriteULToEEProm(coinAuditStartByte, RPU_ReadULFromEEProm(coinAuditStartByte) + 1);
}


void AddCredit(boolean playSound = false, byte numToAdd = 1) {
  if (Credits < MaximumCredits) {
    Credits += numToAdd;
    if (Credits > MaximumCredits) Credits = MaximumCredits;
    RPU_WriteByteToEEProm(RPU_CREDITS_EEPROM_BYTE, Credits);
    if (playSound) PlaySoundEffect(SOUND_EFFECT_CREDIT_ADDED);
    RPU_SetDisplayCredits(Credits, !FreePlayMode);
    RPU_SetCoinLockout(false);
  } else {
    RPU_SetDisplayCredits(Credits, !FreePlayMode);
    RPU_SetCoinLockout(true);
  }

}


byte SwitchToChuteNum(byte switchHit) {
  byte chuteNum = 0;
  if (switchHit == SW_COIN_2) chuteNum = 1;
  else if (switchHit == SW_COIN_3) chuteNum = 2;
  return chuteNum;
}


boolean AddCoin(byte chuteNum) {
  boolean creditAdded = false;
  if (chuteNum > 2) return false;
  byte cpcSelection = GetCPCSelection(chuteNum);

  // Find the lowest chute num with the same ratio selection
  // and use that ChuteCoinsInProgress counter
  byte chuteNumToUse;
  for (chuteNumToUse = 0; chuteNumToUse <= chuteNum; chuteNumToUse++) {
    if (GetCPCSelection(chuteNumToUse) == cpcSelection) break;
  }

  PlaySoundEffect(SOUND_EFFECT_COIN_DROP_1 + (CurrentTime % 3));

  byte cpcCoins = GetCPCCoins(cpcSelection);
  byte cpcCredits = GetCPCCredits(cpcSelection);
  byte coinProgressBefore = ChuteCoinsInProgress[chuteNumToUse];
  ChuteCoinsInProgress[chuteNumToUse] += 1;

  if (ChuteCoinsInProgress[chuteNumToUse] == cpcCoins) {
    if (cpcCredits > cpcCoins) AddCredit(cpcCredits - (coinProgressBefore));
    else AddCredit(cpcCredits);
    ChuteCoinsInProgress[chuteNumToUse] = 0;
    creditAdded = true;
  } else {
    if (cpcCredits > cpcCoins) {
      AddCredit(1);
      creditAdded = true;
    } else {
    }
  }

  return creditAdded;
}

void AddSpecialCredit() {
  AddCredit(false, 1);
  RPU_PushToTimedSolenoidStack(SOL_KNOCKER, 3, CurrentTime, true);
  RPU_WriteULToEEProm(RPU_TOTAL_REPLAYS_EEPROM_START_BYTE, RPU_ReadULFromEEProm(RPU_TOTAL_REPLAYS_EEPROM_START_BYTE) + 1);
}

void AwardSpecial(boolean upper) {
  if (upper) {
    if (UpperSpecialCollected[CurrentPlayer]) return;
    UpperSpecialCollected[CurrentPlayer] = true;
  } else {
    if (LowerSpecialCollected[CurrentPlayer]) return;
    LowerSpecialCollected[CurrentPlayer] = true;
  }
  if (TournamentScoring) {
    StartScoreAnimation(PlayfieldMultiplier * SpecialValue);
  } else {
    AddSpecialCredit();
  }
}

void AwardExtraBall() {
  if (UpperExtraBallCollected[CurrentPlayer]) return;
  UpperExtraBallCollected[CurrentPlayer] = true;

  if (TournamentScoring) {
    StartScoreAnimation(PlayfieldMultiplier * ExtraBallValue);
  } else {
    SamePlayerShootsAgain = true;
    RPU_SetLampState(LAMP_SHOOT_AGAIN, SamePlayerShootsAgain);
    QueueNotification(SOUND_EFFECT_VP_EXTRA_BALL, 8);
  }
}

void IncreasePlayfieldMultiplier(unsigned long duration) {
  if (PlayfieldMultiplierExpiration) PlayfieldMultiplierExpiration += duration;
  else PlayfieldMultiplierExpiration = CurrentTime + duration;
  PlayfieldMultiplier += 1;
  if (PlayfieldMultiplier > 5) {
    PlayfieldMultiplier = 5;
  } else {
    QueueNotification(SOUND_EFFECT_VP_RETURN_TO_1X + (PlayfieldMultiplier - 1), 1);
  }
}


void IncreaseBonusX() {
  BonusX[CurrentPlayer] += 1;
  if (BonusX[CurrentPlayer] == 4 || BonusX[CurrentPlayer] > 5) BonusX[CurrentPlayer] = 5;
  BonusXAnimationStart = CurrentTime;
}

void AddToBonus(byte numToAdd) {
  Bonus[CurrentPlayer] += numToAdd;
  if (Bonus[CurrentPlayer] > MAX_DISPLAY_BONUS) Bonus[CurrentPlayer] = MAX_DISPLAY_BONUS;
  BonusAnimationStart = CurrentTime;
}


////////////////////////////////////////////////////////////////////////////
//
//  Audio Output functions
//
////////////////////////////////////////////////////////////////////////////

void QueueNotification(unsigned int soundEffectNum, byte priority) {
  if (CalloutsVolume == 0) return;
  if (SoundSelector < 2) return;
  if (soundEffectNum < SOUND_EFFECT_VP_VOICE_NOTIFICATIONS_START || soundEffectNum >= (SOUND_EFFECT_VP_VOICE_NOTIFICATIONS_START + NUM_VOICE_NOTIFICATIONS)) return;

  if (DEBUG_MESSAGES) {
    char buf[256];
    sprintf(buf, "Notification %d\n", soundEffectNum);
    Serial.write(buf);
  }

  Audio.QueuePrioritizedNotification(soundEffectNum, VoiceNotificationDurations[soundEffectNum - SOUND_EFFECT_VP_VOICE_NOTIFICATIONS_START], priority, CurrentTime);
}

void PlaySoundEffect(unsigned int soundEffectNum) {

  if (SoundSelector == 0) return;

  if (SoundSelector > 1) Audio.PlaySound(soundEffectNum, AUDIO_PLAY_TYPE_WAV_TRIGGER);

  if (DEBUG_MESSAGES) {
    char buf[128];
    sprintf(buf, "Playing %d\n", soundEffectNum);
    Serial.write(buf);
  }

  if (SoundSelector == 1) {
    switch (soundEffectNum) {
      case SOUND_EFFECT_VP_PLAYER_1_UP:
        /*
              AddToSoundQueue(SB300_SOUND_FUNCTION_SQUARE_WAVE, 1, 0x12, CurrentTime);
              AddToSoundQueue(SB300_SOUND_FUNCTION_SQUARE_WAVE, 0, 0x92, CurrentTime);
              AddToSoundQueue(SB300_SOUND_FUNCTION_SQUARE_WAVE, 6, 0x80, CurrentTime);
              AddToSoundQueue(SB300_SOUND_FUNCTION_SQUARE_WAVE, 7, 0x00, CurrentTime);
              AddToSoundQueue(SB300_SOUND_FUNCTION_ANALOG, 0, 0xAA, CurrentTime);
              AddToSoundQueue(SB300_SOUND_FUNCTION_ANALOG, 0, 0x8A, CurrentTime+50);
              AddToSoundQueue(SB300_SOUND_FUNCTION_ANALOG, 0, 0x6A, CurrentTime+100);
              AddToSoundQueue(SB300_SOUND_FUNCTION_ANALOG, 0, 0x4A, CurrentTime+150);
              AddToSoundQueue(SB300_SOUND_FUNCTION_ANALOG, 0, 0x2A, CurrentTime+250);
              AddToSoundQueue(SB300_SOUND_FUNCTION_ANALOG, 0, 0x0A, CurrentTime+300);
        */
        break;
    }
  }

}



////////////////////////////////////////////////////////////////////////////
//
//  Self test, audit, adjustments mode
//
////////////////////////////////////////////////////////////////////////////

#define ADJ_TYPE_LIST                 1
#define ADJ_TYPE_MIN_MAX              2
#define ADJ_TYPE_MIN_MAX_DEFAULT      3
#define ADJ_TYPE_SCORE                4
#define ADJ_TYPE_SCORE_WITH_DEFAULT   5
#define ADJ_TYPE_SCORE_NO_DEFAULT     6
byte AdjustmentType = 0;
byte NumAdjustmentValues = 0;
byte AdjustmentValues[8];
unsigned long AdjustmentScore;
byte *CurrentAdjustmentByte = NULL;
unsigned long *CurrentAdjustmentUL = NULL;
byte CurrentAdjustmentStorageByte = 0;
byte TempValue = 0;

int RunSelfTest(int curState, boolean curStateChanged) {
  int returnState = curState;
  CurrentNumPlayers = 0;

  if (curStateChanged) {
    // Send a stop-all command and reset the sample-rate offset, in case we have
    //  reset while the WAV Trigger was already playing.
    Audio.StopAllAudio();
    int modeMapping = SelfTestStateToCalloutMap[-1 - curState];
    Audio.PlaySound((unsigned short)modeMapping, AUDIO_PLAY_TYPE_WAV_TRIGGER, 10);
    SoundSettingTimeout = 0;
  } else {
    if (SoundSettingTimeout && CurrentTime > SoundSettingTimeout) {
      SoundSettingTimeout = 0;
      Audio.StopAllAudio();
    }
  }

  // Any state that's greater than CHUTE_3 is handled by the Base Self-test code
  // Any that's less, is machine specific, so we handle it here.
  if (curState >= MACHINE_STATE_TEST_DONE) {
    byte cpcSelection = 0xFF;
    byte chuteNum = 0xFF;
    if (curState == MACHINE_STATE_ADJUST_CPC_CHUTE_1) chuteNum = 0;
    if (curState == MACHINE_STATE_ADJUST_CPC_CHUTE_2) chuteNum = 1;
    if (curState == MACHINE_STATE_ADJUST_CPC_CHUTE_3) chuteNum = 2;
    if (chuteNum != 0xFF) cpcSelection = GetCPCSelection(chuteNum);
    returnState = RunBaseSelfTest(returnState, curStateChanged, CurrentTime, SW_CREDIT_RESET, SW_SLAM);
    if (chuteNum != 0xFF) {
      if (cpcSelection != GetCPCSelection(chuteNum)) {
        byte newCPC = GetCPCSelection(chuteNum);
        Audio.StopAllAudio();
        PlaySoundEffect(SOUND_EFFECT_SELF_TEST_CPC_START + newCPC);
      }
    }
  } else {
    byte curSwitch = RPU_PullFirstFromSwitchStack();

    if (curSwitch == SW_SELF_TEST_SWITCH && (CurrentTime - GetLastSelfTestChangedTime()) > 250) {
      SetLastSelfTestChangedTime(CurrentTime);
      returnState -= 1;
    }

    if (curSwitch == SW_SLAM) {
      returnState = MACHINE_STATE_ATTRACT;
    }

    if (curStateChanged) {
      for (int count = 0; count < 4; count++) {
        RPU_SetDisplay(count, 0);
        RPU_SetDisplayBlank(count, 0x00);
      }
      RPU_SetDisplayCredits(MACHINE_STATE_TEST_SOUNDS - curState);
      RPU_SetDisplayBallInPlay(0, false);
      CurrentAdjustmentByte = NULL;
      CurrentAdjustmentUL = NULL;
      CurrentAdjustmentStorageByte = 0;

      AdjustmentType = ADJ_TYPE_MIN_MAX;
      AdjustmentValues[0] = 0;
      AdjustmentValues[1] = 1;
      TempValue = 0;

      switch (curState) {
        case MACHINE_STATE_ADJUST_FREEPLAY:
          CurrentAdjustmentByte = (byte *)&FreePlayMode;
          CurrentAdjustmentStorageByte = EEPROM_FREE_PLAY_BYTE;
          break;
        case MACHINE_STATE_ADJUST_BALL_SAVE:
          AdjustmentType = ADJ_TYPE_LIST;
          NumAdjustmentValues = 5;
          AdjustmentValues[1] = 5;
          AdjustmentValues[2] = 10;
          AdjustmentValues[3] = 15;
          AdjustmentValues[4] = 20;
          CurrentAdjustmentByte = &BallSaveNumSeconds;
          CurrentAdjustmentStorageByte = EEPROM_BALL_SAVE_BYTE;
          break;
        case MACHINE_STATE_ADJUST_SFX_AND_SOUNDTRACK:
          AdjustmentType = ADJ_TYPE_LIST;
          NumAdjustmentValues = 3;
          AdjustmentValues[0] = 0;
          AdjustmentValues[1] = 1;
          AdjustmentValues[2] = 2;
          CurrentAdjustmentByte = &SoundSelector;
          CurrentAdjustmentStorageByte = EEPROM_SOUND_SELECTOR_BYTE;
          break;

        case MACHINE_STATE_ADJUST_MUSIC_VOLUME:
          AdjustmentType = ADJ_TYPE_MIN_MAX;
          AdjustmentValues[0] = 1;
          AdjustmentValues[1] = 10;
          CurrentAdjustmentByte = &MusicVolume;
          CurrentAdjustmentStorageByte = EEPROM_MUSIC_VOLUME_BYTE;
          break;
        case MACHINE_STATE_ADJUST_SFX_VOLUME:
          AdjustmentType = ADJ_TYPE_MIN_MAX;
          AdjustmentValues[0] = 1;
          AdjustmentValues[1] = 10;
          CurrentAdjustmentByte = &SoundEffectsVolume;
          CurrentAdjustmentStorageByte = EEPROM_SFX_VOLUME_BYTE;
          break;
        case MACHINE_STATE_ADJUST_CALLOUTS_VOLUME:
          AdjustmentType = ADJ_TYPE_MIN_MAX;
          AdjustmentValues[0] = 1;
          AdjustmentValues[1] = 10;
          CurrentAdjustmentByte = &CalloutsVolume;
          CurrentAdjustmentStorageByte = EEPROM_CALLOUTS_VOLUME_BYTE;
          break;

        case MACHINE_STATE_ADJUST_TOURNAMENT_SCORING:
          CurrentAdjustmentByte = (byte *)&TournamentScoring;
          CurrentAdjustmentStorageByte = EEPROM_TOURNAMENT_SCORING_BYTE;
          break;
        case MACHINE_STATE_ADJUST_TILT_WARNING:
          AdjustmentValues[1] = 2;
          CurrentAdjustmentByte = &MaxTiltWarnings;
          CurrentAdjustmentStorageByte = EEPROM_TILT_WARNING_BYTE;
          break;
        case MACHINE_STATE_ADJUST_AWARD_OVERRIDE:
          AdjustmentType = ADJ_TYPE_MIN_MAX_DEFAULT;
          AdjustmentValues[1] = 7;
          CurrentAdjustmentByte = &ScoreAwardReplay;
          CurrentAdjustmentStorageByte = EEPROM_AWARD_OVERRIDE_BYTE;
          break;
        case MACHINE_STATE_ADJUST_BALLS_OVERRIDE:
          AdjustmentType = ADJ_TYPE_LIST;
          NumAdjustmentValues = 2;
          AdjustmentValues[0] = 3;
          AdjustmentValues[1] = 5;
          CurrentAdjustmentByte = &BallsPerGame;
          CurrentAdjustmentStorageByte = EEPROM_BALLS_OVERRIDE_BYTE;
          break;
        case MACHINE_STATE_ADJUST_SCROLLING_SCORES:
          CurrentAdjustmentByte = (byte *)&ScrollingScores;
          CurrentAdjustmentStorageByte = EEPROM_SCROLLING_SCORES_BYTE;
          break;

        case MACHINE_STATE_ADJUST_EXTRA_BALL_AWARD:
          AdjustmentType = ADJ_TYPE_SCORE_WITH_DEFAULT;
          CurrentAdjustmentUL = &ExtraBallValue;
          CurrentAdjustmentStorageByte = EEPROM_EXTRA_BALL_SCORE_BYTE;
          break;

        case MACHINE_STATE_ADJUST_SPECIAL_AWARD:
          AdjustmentType = ADJ_TYPE_SCORE_WITH_DEFAULT;
          CurrentAdjustmentUL = &SpecialValue;
          CurrentAdjustmentStorageByte = EEPROM_SPECIAL_SCORE_BYTE;
          break;

        case MACHINE_STATE_ADJUST_CREDIT_RESET_HOLD_TIME:
          AdjustmentType = ADJ_TYPE_LIST;
          NumAdjustmentValues = 5;
          AdjustmentValues[0] = 0;
          AdjustmentValues[1] = 1;
          AdjustmentValues[2] = 2;
          AdjustmentValues[3] = 3;
          AdjustmentValues[4] = 99;
          CurrentAdjustmentByte = &TimeRequiredToResetGame;
          CurrentAdjustmentStorageByte = EEPROM_CRB_HOLD_TIME;
          break;

        case MACHINE_STATE_ADJUST_GATE_SETTING:
          AdjustmentType = ADJ_TYPE_MIN_MAX;
          AdjustmentValues[0] = 0;
          AdjustmentValues[1] = 2;
          CurrentAdjustmentByte = &GateSetting;
          CurrentAdjustmentStorageByte = EEPROM_GATE_SETTING;
          break;        
  
        case MACHINE_STATE_ADJUST_FLIGHT_INSTRUCTION:
          AdjustmentType = ADJ_TYPE_MIN_MAX;
          AdjustmentValues[0] = 0;
          AdjustmentValues[1] = 2;
          CurrentAdjustmentByte = &LegInstructions;
          CurrentAdjustmentStorageByte = EEPROM_FLIGHT_INSTRUCTION;
          break;        

        case MACHINE_STATE_ADJUST_LEG_DIFFICULTY:
          AdjustmentType = ADJ_TYPE_MIN_MAX;
          AdjustmentValues[0] = 0;
          AdjustmentValues[1] = 15;
          CurrentAdjustmentByte = &FlightCompletionDifficulty;
          CurrentAdjustmentStorageByte = EEPROM_LEG_DIFFICULTY;
          break;        

        case MACHINE_STATE_ADJUST_TURBULENCE:
          AdjustmentType = ADJ_TYPE_MIN_MAX;
          AdjustmentValues[0] = 0;
          AdjustmentValues[1] = 2;
          CurrentAdjustmentByte = &TurbulenceFiresSolenoids;
          CurrentAdjustmentStorageByte = EEPROM_TURBULENCE;
          break;        

        case MACHINE_STATE_ADJUST_DONE:
          returnState = MACHINE_STATE_ATTRACT;
          break;
      }

    }

    // Change value, if the switch is hit
    if (curSwitch == SW_CREDIT_RESET) {

      if (CurrentAdjustmentByte && (AdjustmentType == ADJ_TYPE_MIN_MAX || AdjustmentType == ADJ_TYPE_MIN_MAX_DEFAULT)) {
        byte curVal = *CurrentAdjustmentByte;
        curVal += 1;
        if (curVal > AdjustmentValues[1]) {
          if (AdjustmentType == ADJ_TYPE_MIN_MAX) curVal = AdjustmentValues[0];
          else {
            if (curVal > 99) curVal = AdjustmentValues[0];
            else curVal = 99;
          }
        }
        *CurrentAdjustmentByte = curVal;
        if (CurrentAdjustmentStorageByte) EEPROM.write(CurrentAdjustmentStorageByte, curVal);

        if (curState == MACHINE_STATE_ADJUST_MUSIC_VOLUME) {
          if (SoundSettingTimeout) Audio.StopAllAudio();
          Audio.PlaySound(SOUND_EFFECT_BACKGROUND_SONG_1, AUDIO_PLAY_TYPE_WAV_TRIGGER, curVal);
          Audio.SetMusicVolume(curVal);
          SoundSettingTimeout = CurrentTime + 5000;
        } else if (curState == MACHINE_STATE_ADJUST_SFX_VOLUME) {
          if (SoundSettingTimeout) Audio.StopAllAudio();
          Audio.PlaySound(SOUND_EFFECT_DROP_TARGET_HIT, AUDIO_PLAY_TYPE_WAV_TRIGGER, curVal);
          Audio.SetSoundFXVolume(curVal);
          SoundSettingTimeout = CurrentTime + 5000;
        } else if (curState == MACHINE_STATE_ADJUST_CALLOUTS_VOLUME) {
          if (SoundSettingTimeout) Audio.StopAllAudio();
          Audio.PlaySound(SOUND_EFFECT_VP_SHOOT_AGAIN, AUDIO_PLAY_TYPE_WAV_TRIGGER, curVal);
          Audio.SetNotificationsVolume(curVal);
          SoundSettingTimeout = CurrentTime + 3000;
        }
      } else if (CurrentAdjustmentByte && AdjustmentType == ADJ_TYPE_LIST) {
        byte valCount = 0;
        byte curVal = *CurrentAdjustmentByte;
        byte newIndex = 0;
        for (valCount = 0; valCount < (NumAdjustmentValues - 1); valCount++) {
          if (curVal == AdjustmentValues[valCount]) newIndex = valCount + 1;
        }
        *CurrentAdjustmentByte = AdjustmentValues[newIndex];
        if (CurrentAdjustmentStorageByte) EEPROM.write(CurrentAdjustmentStorageByte, AdjustmentValues[newIndex]);
      } else if (CurrentAdjustmentUL && (AdjustmentType == ADJ_TYPE_SCORE_WITH_DEFAULT || AdjustmentType == ADJ_TYPE_SCORE_NO_DEFAULT)) {
        unsigned long curVal = *CurrentAdjustmentUL;
        curVal += 5000;
        if (curVal > 100000) curVal = 0;
        if (AdjustmentType == ADJ_TYPE_SCORE_NO_DEFAULT && curVal == 0) curVal = 5000;
        *CurrentAdjustmentUL = curVal;
        if (CurrentAdjustmentStorageByte) RPU_WriteULToEEProm(CurrentAdjustmentStorageByte, curVal);
      }

      /*
            if (curState == MACHINE_STATE_ADJUST_DIM_LEVEL) {
              RPU_SetDimDivisor(1, DimLevel);
            }
      */
      if (curState == MACHINE_STATE_ADJUST_SFX_AND_SOUNDTRACK) {
        Audio.StopAllAudio();
        PlaySoundEffect(SOUND_EFFECT_SELF_TEST_AUDIO_OPTIONS_START + *CurrentAdjustmentByte);
      } else if (curState == MACHINE_STATE_ADJUST_CREDIT_RESET_HOLD_TIME) {
        Audio.StopAllAudio();
        if (*CurrentAdjustmentByte != 99) PlaySoundEffect(SOUND_EFFECT_AP_CRB_OPTION_1 + *CurrentAdjustmentByte);
        else PlaySoundEffect(SOUND_EFFECT_AP_CRB_OPTION_99);
      }

    }

    // Show current value
    if (CurrentAdjustmentByte != NULL) {
      RPU_SetDisplay(0, (unsigned long)(*CurrentAdjustmentByte), true);
    } else if (CurrentAdjustmentUL != NULL) {
      RPU_SetDisplay(0, (*CurrentAdjustmentUL), true);
    }

  }

  /*
    if (curState == MACHINE_STATE_ADJUST_DIM_LEVEL) {
      //    for (int count = 0; count < 7; count++) RPU_SetLampState(MIDDLE_ROCKET_7K + count, 1, (CurrentTime / 1000) % 2);
    }
  */

  if (returnState == MACHINE_STATE_ATTRACT) {
    // If any variables have been set to non-override (99), return
    // them to dip switch settings
    // Balls Per Game, Player Loses On Ties, Novelty Scoring, Award Score
    //    DecodeDIPSwitchParameters();
    RPU_SetDisplayCredits(Credits, !FreePlayMode);
    ReadStoredParameters();
  }

  return returnState;
}



////////////////////////////////////////////////////////////////////////////
//
//  Attract Mode
//
////////////////////////////////////////////////////////////////////////////

unsigned long AttractLastLadderTime = 0;
byte AttractLastLadderBonus = 0;
unsigned long AttractDisplayRampStart = 0;
byte AttractLastHeadMode = 255;
byte AttractLastPlayfieldMode = 255;
byte InAttractMode = false;


int RunAttractMode(int curState, boolean curStateChanged) {

  int returnState = curState;

  if (curStateChanged) {
    RPU_DisableSolenoidStack();
    RPU_TurnOffAllLamps();
    RPU_SetDisableFlippers(true);
    if (DEBUG_MESSAGES) {
      Serial.write("Entering Attract Mode\n\r");
    }
    AttractLastHeadMode = 0;
    AttractLastPlayfieldMode = 0;
    RPU_SetDisplayCredits(Credits, !FreePlayMode);
  }

  // Alternate displays between high score and blank
  if (CurrentTime < 16000) {
    if (AttractLastHeadMode != 1) {
      ShowPlayerScores(0xFF, false, false);
      RPU_SetDisplayCredits(Credits, !FreePlayMode);
      RPU_SetDisplayBallInPlay(0, true);
    }
  } else if ((CurrentTime / 8000) % 2 == 0) {

    if (AttractLastHeadMode != 2) {
      RPU_SetLampState(LAMP_HEAD_HIGH_SCORE, 1, 0, 250);
      RPU_SetLampState(LAMP_HEAD_GAME_OVER, 0);
      LastTimeScoreChanged = CurrentTime;
    }
    AttractLastHeadMode = 2;
    ShowPlayerScores(0xFF, false, false, HighScore);
  } else {
    if (AttractLastHeadMode != 3) {
      if (CurrentTime < 32000) {
        for (int count = 0; count < 4; count++) {
          CurrentScores[count] = 0;
        }
        CurrentNumPlayers = 0;
      }
      RPU_SetLampState(LAMP_HEAD_HIGH_SCORE, 0);
      RPU_SetLampState(LAMP_HEAD_GAME_OVER, 1);
      LastTimeScoreChanged = CurrentTime;
    }
    ShowPlayerScores(0xFF, false, false);

    AttractLastHeadMode = 3;
  }

  byte attractPlayfieldPhase = ((CurrentTime / 5000) % 5);

  if (attractPlayfieldPhase != AttractLastPlayfieldMode) {
    RPU_TurnOffAllLamps();
    AttractLastPlayfieldMode = attractPlayfieldPhase;
    if (attractPlayfieldPhase == 2) GameMode = GAME_MODE_SKILL_SHOT_PRE_SWITCH;
    else GameMode = GAME_MODE_UNSTRUCTURED_PLAY;
    AttractLastLadderBonus = 1;
    AttractLastLadderTime = CurrentTime;
  }

  if (attractPlayfieldPhase == 0) {
    ShowLampAnimation(0, 30, CurrentTime, 22, false, false);
  } else if (attractPlayfieldPhase == 1) {
    ShowLampAnimation(1, 30, CurrentTime, 23, false, false);
  } else if (attractPlayfieldPhase == 3) {
    ShowLampAnimation(2, 30, CurrentTime, 23, false, false);
  } else if (attractPlayfieldPhase == 2) {
    ShowLampAnimation(0, 30, CurrentTime, 5, false, false);
  } else {
    ShowLampAnimation(2, 70, CurrentTime, 16, false, true);
  }

  byte switchHit;
  while ( (switchHit = RPU_PullFirstFromSwitchStack()) != SWITCH_STACK_EMPTY ) {
    if (switchHit == SW_CREDIT_RESET) {
      if (AddPlayer(true)) returnState = MACHINE_STATE_INIT_GAMEPLAY;
    }
    if (switchHit == SW_COIN_1 || switchHit == SW_COIN_2 || switchHit == SW_COIN_3) {
      AddCoinToAudit(SwitchToChuteNum(switchHit));
      AddCoin(SwitchToChuteNum(switchHit));
    }
    if (switchHit == SW_SELF_TEST_SWITCH && (CurrentTime - GetLastSelfTestChangedTime()) > 250) {
      returnState = MACHINE_STATE_TEST_LAMPS;
      SetLastSelfTestChangedTime(CurrentTime);
    }
  }

  return returnState;
}





////////////////////////////////////////////////////////////////////////////
//
//  Game Play functions
//
////////////////////////////////////////////////////////////////////////////

byte CountBits(unsigned int intToBeCounted) {
  byte numBits = 0;

  for (byte count = 0; count < 16; count++) {
    numBits += (intToBeCounted & 0x01);
    intToBeCounted = intToBeCounted >> 1;
  }

  return numBits;
}


void SetGameMode(byte newGameMode) {
  //  GameMode = newGameMode | (GameMode & ~GAME_BASE_MODE);
  GameMode = newGameMode;
  GameModeStartTime = 0;
  GameModeEndTime = 0;
  if (DEBUG_MESSAGES) {
    char buf[129];
    sprintf(buf, "Game mode set to %d\n", newGameMode);
    Serial.write(buf);
  }
}



boolean WaitForBallToReturn;

int InitGamePlay(boolean curStateChanged) {

  if (DEBUG_MESSAGES) {
    Serial.write("Starting game\n\r");
  }

  if (curStateChanged) {
    WaitForBallToReturn = false;
    // The start button has been hit only once to get
    // us into this mode, so we assume a 1-player game
    // at the moment
    RPU_EnableSolenoidStack();
    RPU_SetCoinLockout((Credits >= MaximumCredits) ? true : false);
    RPU_TurnOffAllLamps();
    Audio.StopAllAudio();
    SetGatePosition(false);
    SetGatePosition(true);

    if (CaptainNumber[0] == 0xFF) {
      CaptainNumber[0] = CurrentTime % NUM_CAPTAINS;
    } else {
      CaptainNumber[0] = (CaptainNumber[0] + (NUM_CAPTAINS / 2) + 1) % NUM_CAPTAINS;
    }

    // Reset displays & game state variables
    for (int count = 0; count < 4; count++) {
      // Initialize game-specific variables
      SpinnerHits[count] = 0;
      CurrentScores[count] = 0;
      UpperExtraBallCollected[count] = false;
      LowerSpecialCollected[count] = false;
      UpperSpecialCollected[count] = false;
      MissionsCompleted[count] = 0;
      MissionLevel[count] = 0;
      CurrentMission[count] = 0xFF;
      CurrentMissionLegs[count] = 0;
      NumbersCompleted[count] = 0;
      NumbersLevel[count] = 0;
      Bonus[count] = 1;
      BonusX[count] = 1;
      GateLevel[count] = 0;

      if (count) {
        CaptainNumber[count] = (CaptainNumber[count - 1] + 1) % NUM_CAPTAINS;
      }
    }

    SamePlayerShootsAgain = false;
    CurrentBallInPlay = 1;
    CurrentNumPlayers = 1;
    CurrentPlayer = 0;
    ShowPlayerScores(0xFF, false, false);
    if (RPU_ReadSingleSwitchState(SW_SAUCER)) {
      WaitForBallToReturn = true;
      RPU_PushToSolenoidStack(SOL_SAUCER, 5);
    }
  }

  if (WaitForBallToReturn) {
    if (RPU_ReadSingleSwitchState(SW_OUTHOLE) == 0) return MACHINE_STATE_INIT_GAMEPLAY;
    WaitForBallToReturn = false;
  }

  return MACHINE_STATE_INIT_NEW_BALL;
}


int InitNewBall(bool curStateChanged, byte playerNum, int ballNum) {

  // If we're coming into this mode for the first time
  // then we have to do everything to set up the new ball
  if (curStateChanged) {
    RPU_TurnOffAllLamps();
    BallFirstSwitchHitTime = 0;
    BallSaveEndTime = 0;

    if (playerNum == 0) SkillShotNumber = ballNum - 1;
    SkillShotAward = 5000;
    AnimateSkillShotStartTime = 0;

    RPU_SetDisableFlippers(false);
    RPU_EnableSolenoidStack();
    RPU_SetDisplayCredits(Credits, !FreePlayMode);
    SamePlayerShootsAgain = false;

    RPU_SetDisplayBallInPlay(ballNum);
    RPU_SetLampState(LAMP_HEAD_TILT, 0);

    CurrentBallSaveNumSeconds = BallSaveNumSeconds;
    if (BallSaveNumSeconds > 0) {
      RPU_SetLampState(LAMP_SHOOT_AGAIN, 1, 0, 500);
    }

    BallSaveUsed = false;
    BallTimeInTrough = 0;
    NumTiltWarnings = 0;
    LastTiltWarningTime = 0;
    Bonus[playerNum] = 1;
    BonusX[playerNum] = 1;
    AdjustedDifficulty = FlightCompletionDifficulty + MissionLevel[playerNum];
    PopBumperPhase = 0;
    if (GateLevel[playerNum]==3) {
      SetGatePosition(false);
    } else {
      GateLevel[playerNum] = 0;
      SetGatePosition(true);
    }
    GateCloseTime = 0;

    // Initialize game-specific start-of-ball lights & variables
    GameModeStartTime = 0;
    GameModeEndTime = 0;
    GameMode = GAME_MODE_SKILL_SHOT_PRE_SWITCH;

    if (RPU_ReadSingleSwitchState(SW_OUTHOLE)) {
      RPU_PushToTimedSolenoidStack(SOL_OUTHOLE, 4, CurrentTime + 600);
    }

    // Reset progress unless holdover awards
    BonusXAnimationStart = 0;

    PlayfieldMultiplier = 1;
    PlayfieldMultiplierExpiration = 0;
    ScoreAdditionAnimation = 0;
    ScoreAdditionAnimationStartTime = 0;
    LastSpinnerHit = 0;
    LastSaucerHitTime = CurrentTime;
    SaucerSwitchStartTime = 0;
    LegStartScoring = false;
    NumbersAnimation = 0;
    LastCenterLampChange = 0;
    LowerSpecialPhase = 0;
    DropTargetsLevel = 0;

    CurrentTurbulenceRotation = 0;
    TurbulenceRotationTarget = 0;
    LastTurbulenceRotationChange = 0;
    TurbulenceMode = false;
    NextTurbulenceSolenoidChange = 0;
    TurbulenceModeStartTime = 0;
    for (byte count=0; count<3; count++) {
      LastTurbulenceSolFire[count] = 0;
      TurbulenceSolenoids[count] = SOL_NONE;
    }

    Drop5Targets.ResetDropTargets(CurrentTime + 200, true);

    Audio.StopAllAudio();
    CurrentMusicType = 0xFF;

    for (byte count=0; count<5; count++) LastTimeNumberHit[count] = 0;
  }

  // We should only consider the ball initialized when
  // the ball is no longer triggering the SW_OUTHOLE
  if (RPU_ReadSingleSwitchState(SW_OUTHOLE)) {
    return MACHINE_STATE_INIT_NEW_BALL;
  } else {
    return MACHINE_STATE_NORMAL_GAMEPLAY;
  }

}


void StartBackgroundMusic(int musicType) {
  if (musicType != CurrentMusicType) {
    if (musicType == MUSIC_TYPE_NO_MISSION) {
      Audio.PlayBackgroundSong(SOUND_EFFECT_BACKGROUND_NOISE);
    } else if (musicType == MUSIC_TYPE_MISSION) {
      Audio.PlayBackgroundSong(SOUND_EFFECT_BACKGROUND_SONG_1 + (CurrentTime % 5));
    } else if (musicType == MUSIC_TYPE_MISSION_COMPLETION) {
      Audio.PlayBackgroundSong(SOUND_EFFECT_TENSE_BACKGROUND_SONG_1 + (CurrentTime % 3));
    } else if (musicType == MUSIC_TYPE_WIZARD) {
      Audio.PlayBackgroundSong(SOUND_EFFECT_WIZARD_BACKGROUND_SONG);
    }
    CurrentMusicType = musicType;
  }
}


void SetBallSave(unsigned long numberOfMilliseconds) {
  BallSaveEndTime = CurrentTime + numberOfMilliseconds;
  BallSaveUsed = false;
  if (CurrentBallSaveNumSeconds == 0)  CurrentBallSaveNumSeconds = 2;
}



void UpdateDropTargets() {
  Drop5Targets.Update(CurrentTime);
}

// This function manages all timers, flags, and lights
int ManageGameMode() {
  int returnState = MACHINE_STATE_NORMAL_GAMEPLAY;

  UpdateDropTargets();

  boolean specialAnimationRunning = false;

  if ((CurrentTime - LastSwitchHitTime) > 3000) TimersPaused = true;
  else TimersPaused = false;

  boolean showPlayfieldMultiplier = false;
  if (!TimersPaused && PlayfieldMultiplierExpiration) {
    showPlayfieldMultiplier = true;
    if (CurrentTime > PlayfieldMultiplierExpiration) {
      PlayfieldMultiplierExpiration = 0;
      //      if (PlayfieldMultiplier > 1) QueueNotification(SOUND_EFFECT_VP_RETURN_TO_1X, 1);
      PlayfieldMultiplier = 1;
      ShowPlayerScores(0xFF, false, false);
    }
  }

  if (CurrentTurbulenceRotation == 0) {
    TurbulenceShift = 0;
  }
  if (LastTurbulenceRotationChange && CurrentTime > (LastTurbulenceRotationChange + CurrentTurbulenceRotation)) {
    if (CurrentTurbulenceRotation == 0) CurrentTurbulenceRotation = MAX_TURBULENCE_ROTATION_PERDIOD * 2;
    if (TurbulenceRotationTarget) {
      if (TurbulenceRotationTarget <= CurrentTurbulenceRotation) {
        CurrentTurbulenceRotation -= 20;
        if (TurbulenceRotationTarget >= CurrentTurbulenceRotation) {
          TurbulenceRotationTarget += 10;
          if (TurbulenceRotationTarget > MAX_TURBULENCE_ROTATION_PERDIOD) TurbulenceRotationTarget = 0;
        }
      } else {
        CurrentTurbulenceRotation += 20;
      }
    } else {
      if (CurrentTurbulenceRotation) CurrentTurbulenceRotation += 20;
    }
    TurbulenceShift = (TurbulenceShift + 1) % 5;
    LastTurbulenceRotationChange = CurrentTime;
    if (CurrentTurbulenceRotation > (MAX_TURBULENCE_ROTATION_PERDIOD * 2) && TurbulenceShift == 0) {
      CurrentTurbulenceRotation = 0;
      LastTurbulenceRotationChange = 0;
      TurbulenceRotationTarget = 0;
    }
  }
  if (TurbulenceMode && TurbulenceModeStartTime) {
    if (CurrentTime>(TurbulenceModeStartTime+30000)) {
      TurbulenceMode = false;
      TurbulenceModeStartTime = 0;
      NextTurbulenceSolenoidChange = 0;
      for (byte count=0; count<3; count++) {
        TurbulenceSolenoids[count] = SOL_NONE;
        LastTurbulenceSolFire[count] = 0;
      }
      QueueNotification(SOUND_EFFECT_VP_TURBULENCE_OVER, 7);
      Audio.StopSound(SOUND_EFFECT_TURBULENCE);
    } else if (NextTurbulenceSolenoidChange && (CurrentTime>NextTurbulenceSolenoidChange)) {
      NextTurbulenceSolenoidChange = CurrentTime + 3000 + ((CurrentTime%3)*1000);
      byte numSols = (CurrentTime/343)%3;
      unsigned long solSeed = (CurrentTime%342);
      for (byte count=0; count<3; count++) {
        if (count<=numSols) TurbulenceSolenoids[count] = PossibleTurbulenceSolenoids[solSeed%7];
        else TurbulenceSolenoids[count] = SOL_NONE;
        solSeed /= 7;
      }
    } else {
      unsigned long distanceToPeak;
      if (CurrentTime<(TurbulenceModeStartTime+15000)) {
        distanceToPeak = 15000 - (CurrentTime - TurbulenceModeStartTime);
      } else {
        distanceToPeak = CurrentTime - TurbulenceModeStartTime - 15000;
      }
      distanceToPeak /= 40;
      for (unsigned long count=0; count<3; count++) {
        if ( ((CurrentTime + count)%(100+distanceToPeak))==0 && TurbulenceSolenoids[count]!=SOL_NONE ) {
          if (CurrentTime>(LastTurbulenceSolFire[count] + 85)) {
            LastTurbulenceSolFire[count] = CurrentTime;
            if (TurbulenceFiresSolenoids==2) {
              RPU_PushToSolenoidStack(TurbulenceSolenoids[count], 1);
            } else if (TurbulenceFiresSolenoids==1) {
              RPU_PushToSolenoidStack(SOL_KNOCKER, 1);
            }
          }
        }
      }
    }
  }

  if (GateCloseTime && CurrentTime > GateCloseTime) {
    GateCloseTime = 0;
    SetGatePosition(true);
  }  

  switch ( GameMode ) {
    case GAME_MODE_SKILL_SHOT_PRE_SWITCH:
      if (GameModeStartTime == 0) {
        GameModeStartTime = CurrentTime;
        GameModeEndTime = 0;
        LastModePrompt = 0;
        LastAlternatingScore = CurrentTime;
        SkillShotAward = 5000;
        StartBackgroundMusic(MUSIC_TYPE_NO_MISSION);
      }

      if (LastModePrompt == 0) {
        // Wait a couple of seconds before first prompt
        if ((CurrentTime - GameModeStartTime) > 5000) {
          LastModePrompt = CurrentTime;
          QueueNotification(SOUND_EFFECT_VP_PLAYER_1_UP + CurrentPlayer, 0);
        }
      } else if ((CurrentTime - LastModePrompt) > 35000) {
        LastModePrompt = CurrentTime;
        QueueNotification(SOUND_EFFECT_VP_PLAYER_1_UP + CurrentPlayer, 0);
      }


      if (CurrentTime > (LastAlternatingScore + 2000)) {
        LastAlternatingScore = CurrentTime;
        if (((CurrentTime - GameModeStartTime) / 2000) % 2) {
          OverrideScoreDisplay(CurrentPlayer, SkillShotAward, DISPLAY_OVERRIDE_ANIMATION_FLUTTER);
          DisplaysNeedResetting = true;
        } else {
          DisplaysNeedResetting = false;
          ShowPlayerScores(0xFF, false, false);
        }
      }

      if (BallFirstSwitchHitTime != 0) {
        if (CurrentMission[CurrentPlayer]!=0xFF) StartOrResumeMission(CurrentMission[CurrentPlayer]);
        else SetGameMode(GAME_MODE_UNSTRUCTURED_PLAY);
        ShowPlayerScores(0xFF, false, false);
      }

      if (GameModeEndTime != 0 && CurrentTime > GameModeEndTime) {
        ShowPlayerScores(0xFF, false, false);
      }
      break;

    case GAME_MODE_SKILL_SHOT_BUILD_BONUS:
      if (GameModeStartTime == 0) {
        GameModeStartTime = CurrentTime;
        GameModeEndTime = 0;
        LastAlternatingScore = CurrentTime;
      }

      SkillShotAward = 5000 + 10 * ((CurrentTime - GameModeStartTime) / 10);
      if (SkillShotAward > 10000) SkillShotAward = 10000;

      OverrideScoreDisplay(CurrentPlayer, SkillShotAward, DISPLAY_OVERRIDE_ANIMATION_FLUTTER);
      DisplaysNeedResetting = true;

      if (BallFirstSwitchHitTime != 0) {
        if (CurrentMission[CurrentPlayer]!=0xFF) StartOrResumeMission(CurrentMission[CurrentPlayer]);
        else SetGameMode(GAME_MODE_UNSTRUCTURED_PLAY);
        ShowPlayerScores(0xFF, false, false);
      }

      if (GameModeEndTime != 0 && CurrentTime > GameModeEndTime) {
        ShowPlayerScores(0xFF, false, false);
      }
      break;


    case GAME_MODE_UNSTRUCTURED_PLAY:
      // If this is the first time in this mode
      if (GameModeStartTime == 0) {
        ShowPlayerScores(0xFF, false, false);
        GameModeStartTime = CurrentTime;
        DisplaysNeedResetting = false;
        SaucerSwitchStartTime = 0;
        StartBackgroundMusic(MUSIC_TYPE_NO_MISSION);
        AdjustedDifficulty = FlightCompletionDifficulty + MissionLevel[CurrentPlayer];   
        RPU_SetDisplayCredits(Credits, !FreePlayMode);
      }

      if (RPU_ReadSingleSwitchState(SW_SAUCER)) {
        if (SaucerSwitchStartTime == 0) {
          SaucerSwitchStartTime = CurrentTime;
        } else if (CurrentTime > (SaucerSwitchStartTime + 2000)) {
          RPU_PushToTimedSolenoidStack(SOL_SAUCER, 5, CurrentTime + 500, true);
        }
      } else {
        SaucerSwitchStartTime = 0;
      }
      break;

    case GAME_MODE_BOARDING:
      if (GameModeStartTime == 0) {
        ShowPlayerScores(0xFF, false, false);
        LegStartScoring = true;
        GameModeStartTime = CurrentTime;
        GameModeEndTime = CurrentTime + 2500;
        if ((CurrentMissionLegs[CurrentPlayer]&MISSION_LEG_BOARDING_COMPLETE)==0) QueueNotification(SOUND_EFFECT_VP_BOARDING_1 + CurrentMission[CurrentPlayer], 9);
        CurrentMissionLegs[CurrentPlayer] |= MISSION_LEG_BOARDING_COMPLETE;
      }
      if (CurrentTime > (GameModeStartTime + 2500)) {
        LegStartScoring = false;
      }
      if (GameModeEndTime && (CurrentTime > GameModeEndTime)) {
        AdvanceToNextLeg();
      }
      break;

    case GAME_MODE_TAKEOFF:
      if (GameModeStartTime == 0) {
        ShowPlayerScores(0xFF, false, false);
        LegStartScoring = true;
        GameModeStartTime = CurrentTime;
        GameModeEndTime = 0;
        if (AdjustedDifficulty>11) GameModeEndTime = CurrentTime + 15000;
        else if (AdjustedDifficulty>7) GameModeEndTime = CurrentTime + 30000;
        else if (AdjustedDifficulty>3) GameModeEndTime = CurrentTime + 60000;
        if ((MissionsCompleted[CurrentPlayer]==0 && LegInstructions) || LegInstructions==2) {
          if ((CurrentMissionLegs[CurrentPlayer]&MISSION_LEG_TAKEOFF_COMPLETE)==0) QueueNotification(SOUND_EFFECT_VP_USE_JETSTREAM, 9);
        }
      }
      if (CurrentTime > (GameModeStartTime + 5000)) {
        LegStartScoring = false;
        if (CurrentMissionLegs[CurrentPlayer] & MISSION_LEG_TAKEOFF_COMPLETE) {
          AdvanceToNextLeg();
        }
      }

      if (GameModeEndTime) {
        unsigned long timeLeft = (GameModeEndTime-CurrentTime)/1000;
        boolean timerPhase = (CurrentTime/50)%2;
        RPU_SetDisplayCredits((int)timeLeft+1, timerPhase);
      }
      
      if (GameModeEndTime && (CurrentTime > GameModeEndTime)) {
        QueueNotification(SOUND_EFFECT_VP_FLIGHT_CANCELLED, 9);
        CurrentMissionLegs[CurrentPlayer] = 0;
        CurrentMission[CurrentPlayer] = 0xFF;
        SetGameMode(GAME_MODE_UNSTRUCTURED_PLAY);
      }
      break;

    case GAME_MODE_CRUISING:
      if (GameModeStartTime == 0) {
        ShowPlayerScores(0xFF, false, false);
        LegStartScoring = true;
        GameModeStartTime = CurrentTime;
        GameModeEndTime = 0;
        if (AdjustedDifficulty>11) GameModeEndTime = CurrentTime + 15000;
        else if (AdjustedDifficulty>7) GameModeEndTime = CurrentTime + 30000;
        else if (AdjustedDifficulty>3) GameModeEndTime = CurrentTime + 60000;
        if ((MissionsCompleted[CurrentPlayer]==0 && LegInstructions) || LegInstructions==2) {
          if ((CurrentMissionLegs[CurrentPlayer]&MISSION_LEG_CRUISING_COMPLETE)==0) QueueNotification(SOUND_EFFECT_VP_USE_SPINNER, 9);
        }
      }

      if (CurrentTime > (GameModeStartTime + 10000)) {
        if (LegStartScoring) {
          LegStartScoring = false;
          QueueNotification(SOUND_EFFECT_VP_CRUISING, 5);
        }
        if (CurrentMissionLegs[CurrentPlayer] & MISSION_LEG_CRUISING_COMPLETE) {
          AdvanceToNextLeg();
        }
      }

      if (GameModeEndTime) {
        unsigned long timeLeft = (GameModeEndTime-CurrentTime)/1000;
        boolean timerPhase = (CurrentTime/50)%2;
        RPU_SetDisplayCredits((int)timeLeft+1, timerPhase);
      }

      if (GameModeEndTime && (CurrentTime > GameModeEndTime)) {
        QueueNotification(SOUND_EFFECT_VP_FLIGHT_CANCELLED, 9);
        CurrentMissionLegs[CurrentPlayer] = 0;
        CurrentMission[CurrentPlayer] = 0xFF;
        SetGameMode(GAME_MODE_UNSTRUCTURED_PLAY);
      }
      break;

    case GAME_MODE_APPROACH:
      if (GameModeStartTime == 0) {
        ShowPlayerScores(0xFF, false, false);
        LegStartScoring = true;
        GameModeStartTime = CurrentTime;
        GameModeEndTime = 0;
        if (AdjustedDifficulty>11) GameModeEndTime = CurrentTime + 15000;
        else if (AdjustedDifficulty>7) GameModeEndTime = CurrentTime + 30000;
        else if (AdjustedDifficulty>3) GameModeEndTime = CurrentTime + 60000;
        Drop5Targets.ResetDropTargets(CurrentTime + 500);
        if ((CurrentMissionLegs[CurrentPlayer]&MISSION_LEG_APPROACH_COMPLETE)==0) QueueNotification(SOUND_EFFECT_VP_RETURN_TO_SEATS_1A + (CurrentTime % 2) + (CaptainNumber[CurrentPlayer]) * 2, 5);
        if ((MissionsCompleted[CurrentPlayer]==0 && LegInstructions) || LegInstructions==2) {
          if ((CurrentMissionLegs[CurrentPlayer]&MISSION_LEG_APPROACH_COMPLETE)==0) QueueNotification(SOUND_EFFECT_VP_USE_DROP_TARGETS, 5);
        }
      }

      if (CurrentTime > (GameModeStartTime + 10000)) {
        if (LegStartScoring) {
          LegStartScoring = false;
        }
        if (CurrentMissionLegs[CurrentPlayer] & MISSION_LEG_APPROACH_COMPLETE) {
          SetGameMode(GAME_MODE_LANDING);
        }
      }

      if (GameModeEndTime) {
        unsigned long timeLeft = (GameModeEndTime-CurrentTime)/1000;
        boolean timerPhase = (CurrentTime/50)%2;
        RPU_SetDisplayCredits((int)timeLeft+1, timerPhase);
      }
      
      if (GameModeEndTime && (CurrentTime > GameModeEndTime)) {
        QueueNotification(SOUND_EFFECT_VP_FLIGHT_CANCELLED, 9);
        CurrentMissionLegs[CurrentPlayer] = 0;
        CurrentMission[CurrentPlayer] = 0xFF;
        SetGameMode(GAME_MODE_UNSTRUCTURED_PLAY);
      }
      break;

    case GAME_MODE_LANDING:
      if (GameModeStartTime == 0) {
        ShowPlayerScores(0xFF, false, false);
        LegStartScoring = true;
        GameModeStartTime = CurrentTime;
        GameModeEndTime = 0;
        if (AdjustedDifficulty>11) GameModeEndTime = CurrentTime + 15000;
        else if (AdjustedDifficulty>7) GameModeEndTime = CurrentTime + 30000;
        else if (AdjustedDifficulty>3) GameModeEndTime = CurrentTime + 60000;
        if ((CurrentMissionLegs[CurrentPlayer]&MISSION_LEG_LANDING_COMPLETE)==0) QueueNotification(SOUND_EFFECT_VP_APPROACH, 5);
        if ((MissionsCompleted[CurrentPlayer]==0 && LegInstructions) || LegInstructions==2) {
          if ((CurrentMissionLegs[CurrentPlayer]&MISSION_LEG_LANDING_COMPLETE)==0) QueueNotification(SOUND_EFFECT_VP_USE_SAUCER, 5);
        }
        StartBackgroundMusic(MUSIC_TYPE_MISSION_COMPLETION);
        RPU_SetDisplayCredits(Credits, !FreePlayMode);
      }

      if (CurrentMissionLegs[CurrentPlayer] & MISSION_LEG_LANDING_COMPLETE) {
        SetGameMode(GAME_MODE_FLIGHT_COMPLETE);
        PlaySoundEffect(SOUND_EFFECT_PLANE_LANDING);
      }

      if (GameModeEndTime && (CurrentTime > GameModeEndTime)) {
        QueueNotification(SOUND_EFFECT_VP_FLIGHT_CANCELLED, 9);
        CurrentMissionLegs[CurrentPlayer] = 0;
        CurrentMission[CurrentPlayer] = 0xFF;
        SetGameMode(GAME_MODE_UNSTRUCTURED_PLAY);
      }
      break;

    case GAME_MODE_FLIGHT_COMPLETE:
      if (GameModeStartTime == 0) {
        ShowPlayerScores(0xFF, false, false);
        GameModeStartTime = CurrentTime;
        QueueNotification(SOUND_EFFECT_VP_JACKPOT, 6);
        GameModeEndTime = CurrentTime + 5000;
        MissionsCompleted[CurrentPlayer] |= 0x01 << (CurrentMission[CurrentPlayer]);
        unsigned long jackpotValue = 50000 * PlayfieldMultiplier * ((unsigned long)CountBits(MissionsCompleted[CurrentPlayer]));
        jackpotValue += 250000 * ((unsigned long)MissionLevel[CurrentPlayer]);
        StartScoreAnimation(jackpotValue);
        CurrentMissionLegs[CurrentPlayer] = 0;
        CurrentMission[CurrentPlayer] = 0xFF;
      }
      if (GameModeEndTime && (CurrentTime > GameModeEndTime)) {
        // If we finished all the missions
        if (MissionsCompleted[CurrentPlayer]==0x1F) {
          MissionLevel[CurrentPlayer] += 1;
          MissionsCompleted[CurrentPlayer] = 0;
          SetGameMode(GAME_MODE_SPECIAL_TOUR_START);
        } else {
          SetGameMode(GAME_MODE_COLLECT_MILES);
          RPU_PushToTimedSolenoidStack(SOL_SAUCER, 5, CurrentTime + 100, true);
        }        
      }
      break;
    case GAME_MODE_COLLECT_MILES:
      if (GameModeStartTime == 0) {
        ShowPlayerScores(0xFF, false, false);
        GameModeStartTime = CurrentTime;
        GameModeEndTime = CurrentTime + 10000;
        QueueNotification(SOUND_EFFECT_VP_COLLECT_FREQUENT, 9);
        RPU_TurnOffAllLamps();
      }

      SuperJackpotValue = GameModeEndTime - CurrentTime;
      SuperJackpotValue *= PlayfieldMultiplier;
      SuperJackpotValue *= ((unsigned long)CountBits(MissionsCompleted[CurrentPlayer]));
      SuperJackpotValue /= 1000;
      SuperJackpotValue += 1;
      SuperJackpotValue *= 10000;

      for (byte count = 0; count < 4; count++) {
        if (count != CurrentPlayer) OverrideScoreDisplay(count, ((GameModeEndTime - CurrentTime)/1000)+1, DISPLAY_OVERRIDE_ANIMATION_CENTER);
        else OverrideScoreDisplay(count, SuperJackpotValue, DISPLAY_OVERRIDE_ANIMATION_FLUTTER);
      }
      specialAnimationRunning = true;
      ShowTopLaneLamps();
      ShowOtherScoringLamps();
      ShowShootAgainLamp();

      if (CurrentTime>GameModeEndTime) {
        QueueNotification(SOUND_EFFECT_VP_FAILED_TO_COLLECT, 9);
        ShowPlayerScores(0xFF, false, false);
        SetGameMode(GAME_MODE_UNSTRUCTURED_PLAY);
      }
      break;
    case GAME_MODE_SPECIAL_TOUR_START:
      if (GameModeStartTime == 0) {
        ShowPlayerScores(0xFF, false, false);
        GameModeStartTime = CurrentTime;
        QueueNotification(SOUND_EFFECT_VP_SPECIAL_TOUR_1 + (MissionLevel[CurrentPlayer]-1)%6, 6);
        unsigned long notificationDuraction = VoiceNotificationDurations[(SOUND_EFFECT_VP_SPECIAL_TOUR_1 + (MissionLevel[CurrentPlayer]-1)%6) - SOUND_EFFECT_VP_VOICE_NOTIFICATIONS_START];
        GameModeEndTime = CurrentTime + notificationDuraction;
        StartBackgroundMusic(MUSIC_TYPE_WIZARD);
      }

      specialAnimationRunning = true;
      ShowLampAnimation(1, 40, CurrentTime-GameModeStartTime, LAMP_ANIMATION_STEPS-2, false, false);
      
      if (CurrentTime > GameModeEndTime) {
        SetGameMode(GAME_MODE_SPECIAL_TOUR_NUMBERS);
        RPU_PushToTimedSolenoidStack(SOL_SAUCER, 5, CurrentTime + 100, true);
      }
      break;
    case GAME_MODE_SPECIAL_TOUR_NUMBERS:
    case GAME_MODE_SPECIAL_TOUR_LEFT_LANE:
    case GAME_MODE_SPECIAL_TOUR_SPINNER:
    case GAME_MODE_SPECIAL_TOUR_DROPS:
    case GAME_MODE_SPECIAL_TOUR_SAUCER:
      if (GameModeStartTime == 0) {
        ShowPlayerScores(0xFF, false, false);
        GameModeStartTime = CurrentTime;
        GameModeEndTime = CurrentTime + 15000;
        if (GameMode==GAME_MODE_SPECIAL_TOUR_DROPS) {
          Drop5Targets.ResetDropTargets(CurrentTime + 500);
        }
        QueueNotification(SOUND_EFFECT_VP_TOUR_NUMBERS + (GameMode-GAME_MODE_SPECIAL_TOUR_NUMBERS), 6);
      }

      for (byte count = 0; count < 4; count++) {
        if (count != CurrentPlayer) OverrideScoreDisplay(count, ((GameModeEndTime - CurrentTime)/1000)+1, DISPLAY_OVERRIDE_ANIMATION_CENTER);
      }

      if (CurrentTime<(GameModeStartTime + 1000)) {
        specialAnimationRunning = true;
        ShowLampAnimation(2, 40, CurrentTime-GameModeStartTime, LAMP_ANIMATION_STEPS-3, false, false);
      }

      if (CurrentTime > GameModeEndTime) {        
        SetGameMode(GameMode+1);
      }
      break;
    case GAME_MODE_SPECIAL_TOUR_END:
      if (GameModeStartTime == 0) {
        ShowPlayerScores(0xFF, false, false);
        GameModeStartTime = CurrentTime;
        GameModeEndTime = CurrentTime + 5000;
        QueueNotification(SOUND_EFFECT_VP_RETURN_TO_JFK, 6);
      }

      specialAnimationRunning = true;
      ShowLampAnimation(1, 40, CurrentTime-GameModeStartTime, LAMP_ANIMATION_STEPS-2, false, false);

      if (CurrentTime > GameModeEndTime) {        
        SetGameMode(GAME_MODE_UNSTRUCTURED_PLAY);
      }
      break;
  }


  if (showPlayfieldMultiplier) {
    if (PlayfieldMultiplier > 1) {
      for (byte count = 0; count < 4; count++) {
        if (count != CurrentPlayer) OverrideScoreDisplay(count, PlayfieldMultiplier, DISPLAY_OVERRIDE_ANIMATION_BOUNCE);
      }
    }
  }

  if ( !specialAnimationRunning && NumTiltWarnings <= MaxTiltWarnings ) {
    ShowBonusXLamps();
    ShowShootAgainLamp();
    ShowBonusLamps();
    ShowTopLaneLamps();
    ShowCenterRolloverLamps();
    ShowLeftLaneLamps();
    ShowSpinnerLamp();
    ShowDropTargetLamps();
    ShowGateLamps();
    ShowOtherScoringLamps();
  }


  // Three types of display modes are shown here:
  // 1) score animation
  // 2) fly-bys
  // 3) normal scores
  if (ScoreAdditionAnimationStartTime != 0) {
    // Score animation
    if ((CurrentTime - ScoreAdditionAnimationStartTime) < 2000) {
      byte displayPhase = (CurrentTime - ScoreAdditionAnimationStartTime) / 60;
      byte digitsToShow = 1 + displayPhase / 6;
      if (digitsToShow > 6) digitsToShow = 6;
      unsigned long scoreToShow = ScoreAdditionAnimation;
      for (byte count = 0; count < (6 - digitsToShow); count++) {
        scoreToShow = scoreToShow / 10;
      }
      if (scoreToShow == 0 || displayPhase % 2) scoreToShow = DISPLAY_OVERRIDE_BLANK_SCORE;
      byte countdownDisplay = (1 + CurrentPlayer) % 4;

      for (byte count = 0; count < 4; count++) {
        if (count == countdownDisplay) OverrideScoreDisplay(count, scoreToShow, DISPLAY_OVERRIDE_ANIMATION_NONE);
        else if (count != CurrentPlayer) OverrideScoreDisplay(count, DISPLAY_OVERRIDE_BLANK_SCORE, DISPLAY_OVERRIDE_ANIMATION_NONE);
      }
    } else {
      byte countdownDisplay = (1 + CurrentPlayer) % 4;
      unsigned long remainingScore = 0;
      if ( (CurrentTime - ScoreAdditionAnimationStartTime) < 5000 ) {
        remainingScore = (((CurrentTime - ScoreAdditionAnimationStartTime) - 2000) * ScoreAdditionAnimation) / 3000;
        if (PlayScoreAnimationTick>1 && (remainingScore / PlayScoreAnimationTick) != (LastRemainingAnimatedScoreShown / PlayScoreAnimationTick)) {
          LastRemainingAnimatedScoreShown = remainingScore;
          PlaySoundEffect(SOUND_EFFECT_SLING_SHOT);
        }
      } else {
        CurrentScores[CurrentPlayer] += ScoreAdditionAnimation;
        remainingScore = 0;
        ScoreAdditionAnimationStartTime = 0;
        ScoreAdditionAnimation = 0;
      }

      for (byte count = 0; count < 4; count++) {
        if (count == countdownDisplay) OverrideScoreDisplay(count, ScoreAdditionAnimation - remainingScore, DISPLAY_OVERRIDE_ANIMATION_NONE);
        else if (count != CurrentPlayer) OverrideScoreDisplay(count, DISPLAY_OVERRIDE_BLANK_SCORE, DISPLAY_OVERRIDE_ANIMATION_NONE);
        else OverrideScoreDisplay(count, CurrentScores[CurrentPlayer] + remainingScore, DISPLAY_OVERRIDE_ANIMATION_NONE);
      }
    }
    if (ScoreAdditionAnimationStartTime) ShowPlayerScores(CurrentPlayer, false, false);
    else ShowPlayerScores(0xFF, false, false);
  } else {
    ShowPlayerScores(CurrentPlayer, (BallFirstSwitchHitTime == 0) ? true : false, (BallFirstSwitchHitTime > 0 && ((CurrentTime - LastTimeScoreChanged) > 2000)) ? true : false);
  }

  // Check to see if ball is in the outhole
  if (RPU_ReadSingleSwitchState(SW_OUTHOLE)) {
    if (BallTimeInTrough == 0) {
      BallTimeInTrough = CurrentTime;
    } else {
      // Make sure the ball stays on the sensor for at least
      // 0.5 seconds to be sure that it's not bouncing
      if ((CurrentTime - BallTimeInTrough) > 500) {

        if (BallFirstSwitchHitTime == 0 && NumTiltWarnings <= MaxTiltWarnings) {
          // Nothing hit yet, so return the ball to the player
          RPU_PushToTimedSolenoidStack(SOL_OUTHOLE, 4, CurrentTime);
          BallTimeInTrough = 0;
          returnState = MACHINE_STATE_NORMAL_GAMEPLAY;
        } else {
          CurrentScores[CurrentPlayer] += ScoreAdditionAnimation;
          ScoreAdditionAnimationStartTime = 0;
          ScoreAdditionAnimation = 0;
          ShowPlayerScores(0xFF, false, false);
          // if we haven't used the ball save, and we're under the time limit, then save the ball
          if (!BallSaveUsed && CurrentTime < (BallSaveEndTime + BALL_SAVE_GRACE_PERIOD)) {
            RPU_PushToTimedSolenoidStack(SOL_OUTHOLE, 4, CurrentTime + 100);
            BallSaveUsed = true;
            QueueNotification(SOUND_EFFECT_VP_SHOOT_AGAIN, 8);
            RPU_SetLampState(LAMP_SHOOT_AGAIN, 0);
            BallTimeInTrough = CurrentTime;
            returnState = MACHINE_STATE_NORMAL_GAMEPLAY;
          } else {
            ShowPlayerScores(0xFF, false, false);
            Audio.StopAllAudio();

            if (CurrentBallInPlay < BallsPerGame) PlaySoundEffect(SOUND_EFFECT_BALL_OVER);
            returnState = MACHINE_STATE_COUNTDOWN_BONUS;
          }
        }
      }
    }
  } else {
    BallTimeInTrough = 0;
  }

  LastTimeThroughLoop = CurrentTime;
  return returnState;
}



unsigned long CountdownStartTime = 0;
unsigned long LastCountdownReportTime = 0;
unsigned long NextCountdownReportTime = 0;
unsigned long BonusCountDownEndTime = 0;

int CountdownBonus(boolean curStateChanged) {

  BonusAnimationStart = 0;
  // If this is the first time through the countdown loop
  if (curStateChanged) {
    RPU_SetLampState(LAMP_HEAD_BIP, 1, 0, 250);

    CountdownStartTime = CurrentTime;
    ShowBonusLamps();

    LastCountdownReportTime = CountdownStartTime;
    BonusCountDownEndTime = 0xFFFFFFFF;
  }

  unsigned long countdownDelayTime = 200 - Bonus[CurrentPlayer] * 3;

  if ((CurrentTime - LastCountdownReportTime) > countdownDelayTime) {

    if (Bonus[CurrentPlayer] > 0) {
      // Only give sound & score if this isn't a tilt
      if (NumTiltWarnings <= MaxTiltWarnings) {
        PlaySoundEffect(SOUND_EFFECT_ADVANCED_SCORE_1);
        CurrentScores[CurrentPlayer] += (unsigned long)1000 * ((unsigned long)BonusX[CurrentPlayer]);
      }

      Bonus[CurrentPlayer] -= 1;
      ShowBonusLamps();
    } else if (BonusCountDownEndTime == 0xFFFFFFFF) {
      PlaySoundEffect(SOUND_EFFECT_BALL_OVER);
      RPU_SetLampState(LAMP_BONUS_1K, 0);
      BonusCountDownEndTime = CurrentTime + 1000;
    }
    LastCountdownReportTime = CurrentTime;
  }

  if (CurrentTime > BonusCountDownEndTime) {
    // Reset any lights & variables of goals that weren't completed
    BonusCountDownEndTime = 0xFFFFFFFF;
    return MACHINE_STATE_BALL_OVER;
  }

  return MACHINE_STATE_COUNTDOWN_BONUS;
}



void CheckHighScores() {
  unsigned long highestScore = 0;
  int highScorePlayerNum = 0;
  for (int count = 0; count < CurrentNumPlayers; count++) {
    if (CurrentScores[count] > highestScore) highestScore = CurrentScores[count];
    highScorePlayerNum = count;
  }

  if (highestScore > HighScore) {
    HighScore = highestScore;
    if (HighScoreReplay) {
      AddCredit(false, 3);
      RPU_WriteULToEEProm(RPU_TOTAL_REPLAYS_EEPROM_START_BYTE, RPU_ReadULFromEEProm(RPU_TOTAL_REPLAYS_EEPROM_START_BYTE) + 3);
    }
    RPU_WriteULToEEProm(RPU_HIGHSCORE_EEPROM_START_BYTE, highestScore);
    RPU_WriteULToEEProm(RPU_TOTAL_HISCORE_BEATEN_START_BYTE, RPU_ReadULFromEEProm(RPU_TOTAL_HISCORE_BEATEN_START_BYTE) + 1);

    for (int count = 0; count < 4; count++) {
      if (count == highScorePlayerNum) {
        RPU_SetDisplay(count, CurrentScores[count], true, 2);
      } else {
        RPU_SetDisplayBlank(count, 0x00);
      }
    }

    RPU_PushToTimedSolenoidStack(SOL_KNOCKER, 3, CurrentTime, true);
    RPU_PushToTimedSolenoidStack(SOL_KNOCKER, 3, CurrentTime + 300, true);
    RPU_PushToTimedSolenoidStack(SOL_KNOCKER, 3, CurrentTime + 600, true);
  }
}


unsigned long MatchSequenceStartTime = 0;
unsigned long MatchDelay = 150;
byte MatchDigit = 0;
byte NumMatchSpins = 0;
byte ScoreMatches = 0;

int ShowMatchSequence(boolean curStateChanged) {
  if (!MatchFeature) return MACHINE_STATE_ATTRACT;

  if (curStateChanged) {
    MatchSequenceStartTime = CurrentTime;
    MatchDelay = 1500;
    MatchDigit = CurrentTime % 10;
    NumMatchSpins = 0;
    RPU_SetLampState(LAMP_HEAD_MATCH, 1, 0);
    RPU_SetDisableFlippers();
    ScoreMatches = 0;
  }

  if (NumMatchSpins < 40) {
    if (CurrentTime > (MatchSequenceStartTime + MatchDelay)) {
      MatchDigit += 1;
      if (MatchDigit > 9) MatchDigit = 0;
      PlaySoundEffect(SOUND_EFFECT_MATCH_SPIN);
      RPU_SetDisplayBallInPlay((int)MatchDigit * 10);
      MatchDelay += 50 + 4 * NumMatchSpins;
      NumMatchSpins += 1;
      RPU_SetLampState(LAMP_HEAD_MATCH, NumMatchSpins % 2, 0);

      if (NumMatchSpins == 40) {
        RPU_SetLampState(LAMP_HEAD_MATCH, 0);
        MatchDelay = CurrentTime - MatchSequenceStartTime;
      }
    }
  }

  if (NumMatchSpins >= 40 && NumMatchSpins <= 43) {
    if (CurrentTime > (MatchSequenceStartTime + MatchDelay)) {
      if ( (CurrentNumPlayers > (NumMatchSpins - 40)) && ((CurrentScores[NumMatchSpins - 40] / 10) % 10) == MatchDigit) {
        ScoreMatches |= (1 << (NumMatchSpins - 40));
        AddSpecialCredit();
        MatchDelay += 1000;
        NumMatchSpins += 1;
        RPU_SetLampState(LAMP_HEAD_MATCH, 1);
      } else {
        NumMatchSpins += 1;
      }
      if (NumMatchSpins == 44) {
        MatchDelay += 5000;
      }
    }
  }

  if (NumMatchSpins > 43) {
    if (CurrentTime > (MatchSequenceStartTime + MatchDelay)) {
      return MACHINE_STATE_ATTRACT;
    }
  }

  for (int count = 0; count < 4; count++) {
    if ((ScoreMatches >> count) & 0x01) {
      // If this score matches, we're going to flash the last two digits
#ifdef RPU_OS_USE_7_DIGIT_DISPLAYS
      if ( (CurrentTime / 200) % 2 ) {
        RPU_SetDisplayBlank(count, RPU_GetDisplayBlank(count) & 0x1F);
      } else {
        RPU_SetDisplayBlank(count, RPU_GetDisplayBlank(count) | 0x60);
      }
#else
      if ( (CurrentTime / 200) % 2 ) {
        RPU_SetDisplayBlank(count, RPU_GetDisplayBlank(count) & 0x0F);
      } else {
        RPU_SetDisplayBlank(count, RPU_GetDisplayBlank(count) | 0x30);
      }
#endif
    }
  }

  return MACHINE_STATE_MATCH_MODE;
}


void AdvanceToNextLeg() {
  // We're resuming a mission - so back up to last completed leg
  if (CurrentMissionLegs[CurrentPlayer]&MISSION_LEG_BOARDING_COMPLETE) {
    SetGameMode(GAME_MODE_TAKEOFF);
    StartBackgroundMusic(MUSIC_TYPE_MISSION);
    if (CurrentMissionLegs[CurrentPlayer]&MISSION_LEG_TAKEOFF_COMPLETE) {
      SetGameMode(GAME_MODE_CRUISING);
      if (CurrentMissionLegs[CurrentPlayer]&MISSION_LEG_CRUISING_COMPLETE) {
        SetGameMode(GAME_MODE_APPROACH);
        if (CurrentMissionLegs[CurrentPlayer]&MISSION_LEG_APPROACH_COMPLETE) {
          SetGameMode(GAME_MODE_LANDING);
          StartBackgroundMusic(MUSIC_TYPE_MISSION_COMPLETION);

          // This one should never happen.
          if (CurrentMissionLegs[CurrentPlayer]&MISSION_LEG_LANDING_COMPLETE) {
            SetGameMode(GAME_MODE_FLIGHT_COMPLETE);
          }
        }
      }
    }
  }
}


void StartOrResumeMission(byte missionNumber) {
  for (byte count=0; count<3; count++) LegProgress[count] = 0;
  if (CurrentMission[CurrentPlayer] != 0xFF) {
    AdvanceToNextLeg();
  } else {
    CurrentMission[CurrentPlayer] = missionNumber;
    CurrentMissionLegs[CurrentPlayer] = 0;
    SetGameMode(GAME_MODE_BOARDING);
    StartBackgroundMusic(MUSIC_TYPE_MISSION);
  }
}


void SpotMissionLeg() {
  if (GameMode==GAME_MODE_LANDING) {
    
  } else if (GameMode==GAME_MODE_APPROACH) {
    CurrentMissionLegs[CurrentPlayer] |= MISSION_LEG_APPROACH_COMPLETE;
    StartBackgroundMusic(MUSIC_TYPE_MISSION_COMPLETION);
    SetGameMode(GAME_MODE_LANDING);
  } else if (GameMode==GAME_MODE_CRUISING) {
    CurrentMissionLegs[CurrentPlayer] |= MISSION_LEG_CRUISING_COMPLETE;
    SetGameMode(GAME_MODE_APPROACH);
  } else if (GameMode==GAME_MODE_TAKEOFF) {
    CurrentMissionLegs[CurrentPlayer] |= MISSION_LEG_TAKEOFF_COMPLETE;
    SetGameMode(GAME_MODE_CRUISING);
  }
}



void HandleLaneAndTarget(byte switchHit) {

  byte laneNumber = 4 - (switchHit - SW_5_LANE_AND_TARGET); // zero index

  // debounce quick hits
  if (LastTimeNumberHit[laneNumber]!=0 && CurrentTime<(LastTimeNumberHit[laneNumber]+LANE_AND_TARGET_DEBOUNCE_TIME)) return;

  LastTimeNumberHit[laneNumber] = CurrentTime;
  
  byte oldNumbersCompleted = NumbersCompleted[CurrentPlayer];
  NumbersCompleted[CurrentPlayer] |= 0x01 << laneNumber;

  if (switchHit == SW_2_LANE_AND_TARGET) {

    if (GateSetting==GATE_SETTING_ALWAYS_CLOSED) {
      // Nothing to do here - gate always closed
      SetGatePosition(true);
    } else if (GateSetting==GATE_SETTING_ALWAYS_TIMED) {
      SetGatePosition(false);
      GateCloseTime = CurrentTime + 30000;
    } else if (GateSetting==GATE_SETTING_LIBERAL) {
      GateLevel[CurrentPlayer] += 1;
      if (GateLevel[CurrentPlayer]==1) {
        SetGatePosition(false);
        GateCloseTime = CurrentTime + 30000;
      } else if (GateLevel[CurrentPlayer]==2) {
        QueueNotification(SOUND_EFFECT_VP_GATE_OPEN_BALL, 8);
        SetGatePosition(false);
        GateCloseTime = 0;
      } else if (GateLevel[CurrentPlayer]==3) {
        QueueNotification(SOUND_EFFECT_VP_GATE_OPEN_GAME, 8);
        SetGatePosition(false);
        GateCloseTime = 0;
      }
      if (GateLevel[CurrentPlayer]>3) GateLevel[CurrentPlayer] = 3;
    }
    
  }

  boolean soundPlayed = false;
  if (GameMode == GAME_MODE_SKILL_SHOT_PRE_SWITCH || GameMode == GAME_MODE_SKILL_SHOT_BUILD_BONUS) {
    if (laneNumber == SkillShotNumber) {
      StartScoreAnimation(SkillShotAward);
    } else {
      CurrentScores[CurrentPlayer] += 100 * PlayfieldMultiplier;
    }
    StartOrResumeMission(laneNumber);
  } else if (GameMode == GAME_MODE_COLLECT_MILES) {
    if (switchHit==SW_1_LANE_AND_TARGET) {
      PlaySoundEffect(SOUND_EFFECT_SKILL_SHOT);
      QueueNotification(SOUND_EFFECT_VP_SUPER_JACKPOT, 9);
      StartScoreAnimation(SuperJackpotValue);
      SetGameMode(GAME_MODE_UNSTRUCTURED_PLAY);

      // Figure out next available flight
      byte nextFlight = 0;
      byte bitMask = 0x01;
      for (nextFlight = 0; nextFlight<5; nextFlight++) {
        if (!(MissionsCompleted[CurrentPlayer]&bitMask)) { 
          StartOrResumeMission(nextFlight);
        }
        bitMask *= 2;
      }
    }
  } else if (GameMode==GAME_MODE_SPECIAL_TOUR_NUMBERS) {
    PlaySoundEffect(SOUND_EFFECT_DING);
    CurrentScores[CurrentPlayer] += PlayfieldMultiplier * 10000;
  } else {
    if (CurrentMission[CurrentPlayer] == 0xFF && GameMode<GAME_MODE_SPECIAL_TOUR_START) {
      StartOrResumeMission(laneNumber);
    }
    if (GameMode == GAME_MODE_BOARDING && LegStartScoring) {
      CurrentScores[CurrentPlayer] += 10000 * PlayfieldMultiplier;
      PlaySoundEffect(SOUND_EFFECT_LEG_SCORE);
      soundPlayed = true;
    } else if (NumbersLevel[CurrentPlayer]) {
      CurrentScores[CurrentPlayer] += 3000 * PlayfieldMultiplier;
    } else {
      CurrentScores[CurrentPlayer] += 100 * PlayfieldMultiplier;
    }
  }

  if (NumbersCompleted[CurrentPlayer] == 0x1F) {
    NumbersLevel[CurrentPlayer] += 1;
    StartScoreAnimation(5000 * PlayfieldMultiplier * ((unsigned long)NumbersLevel[CurrentPlayer]));
    NumbersAnimation = CurrentTime;
    NumbersCompleted[CurrentPlayer] = 0;
    PlaySoundEffect(SOUND_EFFECT_SKILL_SHOT);
    soundPlayed = true;
    IncreasePlayfieldMultiplier(30000);

    // Completing numbers will take you directly to landing
    if (CurrentMission[CurrentPlayer] != 0xFF) {
      SpotMissionLeg();
    }
  }

  if (!soundPlayed) {
    if (oldNumbersCompleted != NumbersCompleted[CurrentPlayer]) {
      Audio.StopSound(SOUND_EFFECT_NEW_NUMBER);
      PlaySoundEffect(SOUND_EFFECT_NEW_NUMBER);
    } else {
      PlaySoundEffect(SOUND_EFFECT_REPEAT_NUMBER);
    }
  }

  PopBumperPhase = PopBumperPhase ^ 0x01;
}


void HandleLeftLane(byte switchHit) {
  byte leftLaneNum = 4 - (switchHit - SW_5_LEFT_LANE); // zero index
  byte legGoal = 1;
  if (AdjustedDifficulty%2) legGoal = CountBits(MissionsCompleted[CurrentPlayer]) + 1;
    
  if (GameMode == GAME_MODE_TAKEOFF) {
    LegProgress[0] += 1;
    if (LegProgress[0]>=legGoal) {
      LegProgress[0] = legGoal;
      if (!(CurrentMissionLegs[CurrentPlayer]&MISSION_LEG_TAKEOFF_COMPLETE)) {
        PlaySoundEffect(SOUND_EFFECT_JET_FLYBY);
      }
      CurrentMissionLegs[CurrentPlayer] |= MISSION_LEG_TAKEOFF_COMPLETE;
    }
    
    if (LegStartScoring) {
      CurrentScores[CurrentPlayer] += 10000 * PlayfieldMultiplier;
      PlaySoundEffect(SOUND_EFFECT_LEG_SCORE);
    } else {
      CurrentScores[CurrentPlayer] += 1000 * PlayfieldMultiplier;
      Audio.StopSound(SOUND_EFFECT_SINGLE_BONG);
      PlaySoundEffect(SOUND_EFFECT_SINGLE_BONG);
    }
  } else if (GameMode==GAME_MODE_SPECIAL_TOUR_LEFT_LANE) {
    PlaySoundEffect(SOUND_EFFECT_DING);
    CurrentScores[CurrentPlayer] += PlayfieldMultiplier * 10000;
  } else {
    if (!(CurrentMissionLegs[CurrentPlayer]&MISSION_LEG_TAKEOFF_COMPLETE)) {
      if ((AdjustedDifficulty%10)<2) {
        // if the AdjustedDifficulty is 0, 1, 10, or 11, we can complete legs out of order
        LegProgress[0] += 1;
        if (LegProgress[0]>=legGoal) {
          LegProgress[0] = legGoal;
          CurrentMissionLegs[CurrentPlayer] |= MISSION_LEG_TAKEOFF_COMPLETE;
        }
      }
    }
    
    if (NumbersLevel[CurrentPlayer]) {
      unsigned long buttonScore = 1000 * ((unsigned long)NumbersLevel[CurrentPlayer]);
      if (NumbersCompleted[CurrentPlayer] & (0x01 << leftLaneNum)) {
        AddToBonus(NumbersLevel[CurrentPlayer]);
        buttonScore += 1000;
      }
      CurrentScores[CurrentPlayer] += buttonScore;
      PlaySoundEffect(SOUND_EFFECT_ADVANCED_SCORE_1);
    } else {
      CurrentScores[CurrentPlayer] += 100 * PlayfieldMultiplier;
      PlaySoundEffect(SOUND_EFFECT_ROLLOVER);
    }
  }

}



int HandleSystemSwitches(int curState, byte switchHit) {
  int returnState = curState;
  switch (switchHit) {
    case SW_SELF_TEST_SWITCH:
      returnState = MACHINE_STATE_TEST_LAMPS;
      SetLastSelfTestChangedTime(CurrentTime);
      break;
    case SW_COIN_1:
    case SW_COIN_2:
    case SW_COIN_3:
      AddCoinToAudit(SwitchToChuteNum(switchHit));
      AddCoin(SwitchToChuteNum(switchHit));
      break;
    case SW_CREDIT_RESET:
      if (MachineState == MACHINE_STATE_MATCH_MODE) {
        // If the first ball is over, pressing start again resets the game
        if (Credits >= 1 || FreePlayMode) {
          if (!FreePlayMode) {
            Credits -= 1;
            RPU_WriteByteToEEProm(RPU_CREDITS_EEPROM_BYTE, Credits);
            RPU_SetDisplayCredits(Credits, !FreePlayMode);
          }
          returnState = MACHINE_STATE_INIT_GAMEPLAY;
        }
      } else {
        CreditResetPressStarted = CurrentTime;
      }
      break;
    case SW_TILT:
      // This should be debounced
      if ((CurrentTime - LastTiltWarningTime) > TILT_WARNING_DEBOUNCE_TIME) {
        LastTiltWarningTime = CurrentTime;
        NumTiltWarnings += 1;
        if (NumTiltWarnings > MaxTiltWarnings) {
          RPU_DisableSolenoidStack();
          RPU_SetDisableFlippers(true);
          RPU_TurnOffAllLamps();
          RPU_SetLampState(LAMP_HEAD_TILT, 1);
          SetGatePosition(true);
          Audio.StopAllAudio();
        }
        PlaySoundEffect(SOUND_EFFECT_TILT_WARNING);
      }
      break;
  }

  if (CreditResetPressStarted) {
    if (CurrentBallInPlay < 2) {
      // If we haven't finished the first ball, we can add players
      AddPlayer();
      if (DEBUG_MESSAGES) {
        Serial.write("Start game button pressed\n\r");
      }
      CreditResetPressStarted = 0;
    } else {
      if (RPU_ReadSingleSwitchState(SW_CREDIT_RESET)) {
        if (TimeRequiredToResetGame != 99 && (CurrentTime - CreditResetPressStarted) >= ((unsigned long)TimeRequiredToResetGame * 1000)) {
          // If the first ball is over, pressing start again resets the game
          if (Credits >= 1 || FreePlayMode) {
            if (!FreePlayMode) {
              Credits -= 1;
              RPU_WriteByteToEEProm(RPU_CREDITS_EEPROM_BYTE, Credits);
              RPU_SetDisplayCredits(Credits, !FreePlayMode);
            }
            returnState = MACHINE_STATE_INIT_GAMEPLAY;
            CreditResetPressStarted = 0;
          }
        }
      } else {
        CreditResetPressStarted = 0;
      }
    }

  }

  return returnState;
}


void HandleCenterRollover(byte switchHit) {

  byte rolloverNum = (SW_1_ROLLOVER - switchHit);

  if (GameMode==GAME_MODE_SPECIAL_TOUR_SPINNER) {
    PlaySoundEffect(SOUND_EFFECT_DING);
    CurrentScores[CurrentPlayer] += PlayfieldMultiplier * 2000;
  }

  if (CurrentTurbulenceRotation == 0) {
    if (CurrentMission[CurrentPlayer] == rolloverNum) {
      AddToBonus(2);
    } else if (MissionsCompleted[CurrentPlayer] & (0x01 << rolloverNum)) {
      AddToBonus(1);
    }

    if (NumbersLevel[CurrentPlayer]) {
      CurrentScores[CurrentPlayer] += 1000 * ((unsigned long)NumbersLevel[CurrentPlayer]) * PlayfieldMultiplier;
      PlaySoundEffect(SOUND_EFFECT_ADVANCED_SCORE_3);
    } else {
      CurrentScores[CurrentPlayer] += 100 * PlayfieldMultiplier;
      PlaySoundEffect(SOUND_EFFECT_ROLLOVER);
    }
  } else {
    CurrentScores[CurrentPlayer] += 1000 * PlayfieldMultiplier;
    if (!TurbulenceMode) PlaySoundEffect(SOUND_EFFECT_WIND_GUST);
  }
  
  if (TurbulenceRotationTarget == 0) {
    TurbulenceRotationTarget = MAX_TURBULENCE_ROTATION_PERDIOD;
    if (LastTurbulenceRotationChange == 0) LastTurbulenceRotationChange = CurrentTime;
    //TurbulenceMode = false;
  } else {
    TurbulenceRotationTarget -= TURBULENCE_ROTATION_PERDIOD_CHANGE;
    if (TurbulenceRotationTarget < MIN_TURBULENCE_ROTATION_PERDIOD) {
      TurbulenceRotationTarget = MIN_TURBULENCE_ROTATION_PERDIOD;
      if (!TurbulenceMode) {
        TurbulenceMode = true;
        TurbulenceModeStartTime = CurrentTime;
        NextTurbulenceSolenoidChange = CurrentTime + 3000 + ((CurrentTime%3)*1000);
        PlaySoundEffect(SOUND_EFFECT_TURBULENCE);
        IncreasePlayfieldMultiplier(30000);
        Audio.StopSound(SOUND_EFFECT_SINGLE_BONG);
        PlaySoundEffect(SOUND_EFFECT_SINGLE_BONG);
        QueueNotification(SOUND_EFFECT_VP_CAPTAIN_SEATBELTS_1 + CaptainNumber[CurrentPlayer], 7);
      }
    }
  }
}


void HandleGamePlaySwitches(byte switchHit) {

  byte dropTargetResult;
  //RPU_SetDisplayCredits(switchHit);

  byte legGoal = 1;
  if (AdjustedDifficulty%2) legGoal = CountBits(MissionsCompleted[CurrentPlayer]) + 1;

  switch (switchHit) {
    case SW_1_LANE_AND_TARGET:
    case SW_2_LANE_AND_TARGET:
    case SW_3_LANE_AND_TARGET:
    case SW_4_LANE_AND_TARGET:
    case SW_5_LANE_AND_TARGET:
      HandleLaneAndTarget(switchHit);
      if (BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
      LastSwitchHitTime = CurrentTime;
      break;
    case SW_1_LEFT_LANE:
    case SW_2_LEFT_LANE:
    case SW_3_LEFT_LANE:
    case SW_4_LEFT_LANE:
    case SW_5_LEFT_LANE:
      HandleLeftLane(switchHit);
      if (BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
      LastSwitchHitTime = CurrentTime;
      break;
    case SW_1_ROLLOVER:
    case SW_2_ROLLOVER:
    case SW_3_ROLLOVER:
    case SW_4_ROLLOVER:
    case SW_5_ROLLOVER:
      HandleCenterRollover(switchHit);
      if (BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
      LastSwitchHitTime = CurrentTime;
      break;
    case SW_DROP_TARGET_10_PT:
      if (TurbulenceMode) {
        CurrentScores[CurrentPlayer] += PlayfieldMultiplier * 2500;
        PlaySoundEffect(SOUND_EFFECT_DING);
      } else if (GameMode == GAME_MODE_SPECIAL_TOUR_DROPS) {
        StartScoreAnimation(PlayfieldMultiplier * 15000);
        PlaySoundEffect(SOUND_EFFECT_DING);
      } else {
        CurrentScores[CurrentPlayer] += PlayfieldMultiplier * 100;
        PlaySoundEffect(SOUND_EFFECT_ADVANCED_SCORE_1);
      }
      if (BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
      LastSwitchHitTime = CurrentTime;
      break;
    case SW_DROP_TARGET_5:
    case SW_DROP_TARGET_4:
    case SW_DROP_TARGET_3:
    case SW_DROP_TARGET_2:
    case SW_DROP_TARGET_1:
      dropTargetResult = Drop5Targets.HandleDropTargetHit(switchHit);
      if (dropTargetResult) {
        unsigned long numTargets = CountBits(dropTargetResult);
        AddToBonus(numTargets);
        if (GameMode == GAME_MODE_APPROACH) {
          LegProgress[2] += numTargets;
          if (LegProgress[2]>=legGoal) {
            LegProgress[2] = legGoal;          
            CurrentMissionLegs[CurrentPlayer] |= MISSION_LEG_APPROACH_COMPLETE;
          }
          if (LegStartScoring) {
            CurrentScores[CurrentPlayer] += PlayfieldMultiplier * numTargets * 10000;
            PlaySoundEffect(SOUND_EFFECT_LEG_SCORE);
          } else {
            CurrentScores[CurrentPlayer] += PlayfieldMultiplier * numTargets * 3000;
            PlaySoundEffect(SOUND_EFFECT_DROP_TARGET_HIT);
          }
        } else if (GameMode==GAME_MODE_SPECIAL_TOUR_DROPS) {
          PlaySoundEffect(SOUND_EFFECT_DING);
          CurrentScores[CurrentPlayer] += 10000;
        } else {

          if (!(CurrentMissionLegs[CurrentPlayer]&MISSION_LEG_APPROACH_COMPLETE)) {
            if ((AdjustedDifficulty%10)<2) {
              // if the AdjustedDifficulty is 0, 1, 10, or 11, we can complete legs out of order
              LegProgress[2] += numTargets;
              if (LegProgress[2]>=legGoal) {
                LegProgress[2] = legGoal;
                CurrentMissionLegs[CurrentPlayer] |= MISSION_LEG_APPROACH_COMPLETE;
              }
            }
          }
          
          CurrentScores[CurrentPlayer] += PlayfieldMultiplier * numTargets * 500;
          PlaySoundEffect(SOUND_EFFECT_DROP_TARGET_HIT);
        }
        if (BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
        LastSwitchHitTime = CurrentTime;
      }
      dropTargetResult = Drop5Targets.CheckIfBankCleared();
      if (dropTargetResult == DROP_TARGET_BANK_CLEARED) {
        PlaySoundEffect(SOUND_EFFECT_DROP_TARGET_FINISHED);
        Drop5Targets.ResetDropTargets(CurrentTime + 500);
        IncreasePlayfieldMultiplier(30000);
        // Completing targets will advance the leg
        SpotMissionLeg();
        if (DropTargetsLevel==3) AwardSpecial(true);
        if (DropTargetsLevel==2) AwardExtraBall();
        DropTargetsLevel += 1;
      } else if (dropTargetResult == DROP_TARGET_BANK_CLEARED_IN_ORDER) {
        PlaySoundEffect(SOUND_EFFECT_DROP_TARGET_FINISHED);
        Drop5Targets.ResetDropTargets(CurrentTime + 500);
        IncreasePlayfieldMultiplier(60000);
        // Completing targets will advance the leg
        SpotMissionLeg();
        if (DropTargetsLevel==3) AwardSpecial(true);
        if (DropTargetsLevel==2) AwardExtraBall();
        DropTargetsLevel += 1;
      }
      break;

    case SW_LEFT_OUTLANE:
      if (NumbersLevel[CurrentPlayer]>2 && LowerSpecialPhase==1) AwardSpecial(false);
      CurrentScores[CurrentPlayer] += 1000 * PlayfieldMultiplier;
      PlaySoundEffect(SOUND_EFFECT_OUTLANE_UNLIT);
      if (BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
      LastSwitchHitTime = CurrentTime;      
      if (BallSaveEndTime != 0) {
        BallSaveEndTime += 3000;
      }
      break;

    case SW_RIGHT_OUTLANE:
      if (NumbersLevel[CurrentPlayer]>2 && LowerSpecialPhase==0) AwardSpecial(false);
      CurrentScores[CurrentPlayer] += 1000 * PlayfieldMultiplier;
      
      if (GateLevel[CurrentPlayer] || GateCloseTime) {
        GateCloseTime = CurrentTime + 5000;
        GateLevel[CurrentPlayer] = 0;
        PlaySoundEffect(SOUND_EFFECT_OUTLANE_LIT);
      } else {
        PlaySoundEffect(SOUND_EFFECT_OUTLANE_UNLIT);
      }
      if (BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
      LastSwitchHitTime = CurrentTime;
      if (BallSaveEndTime != 0) {
        BallSaveEndTime += 3000;
      }
      break;

    case SW_LEFT_SLING:
    case SW_RIGHT_SLING:
      CurrentScores[CurrentPlayer] += PlayfieldMultiplier * 10;
      PlaySoundEffect(SOUND_EFFECT_SLING_SHOT);
      if (BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
      LastSwitchHitTime = CurrentTime;
      break;

    case SW_TOP_ROLLOVER:
      if (GameMode == GAME_MODE_SKILL_SHOT_PRE_SWITCH || GameMode == GAME_MODE_SKILL_SHOT_BUILD_BONUS) {
        SkillShotAward += 1000;
        if (GameMode == GAME_MODE_SKILL_SHOT_PRE_SWITCH) SetGameMode(GAME_MODE_SKILL_SHOT_BUILD_BONUS);
      } else {
        CurrentScores[CurrentPlayer] += PlayfieldMultiplier * 10;
      }
      AddToBonus(3);
      break;

    case SW_BOTTOM_POP:
      if (TurbulenceMode) {
        CurrentScores[CurrentPlayer] += PlayfieldMultiplier * 1000;
        PlaySoundEffect(SOUND_EFFECT_DING);
      } else {
        if (PopBumperPhase==1) {
          CurrentScores[CurrentPlayer] += PlayfieldMultiplier * 100;
          PlaySoundEffect(SOUND_EFFECT_DROP_TARGET_HIT);
        } else {
          CurrentScores[CurrentPlayer] += PlayfieldMultiplier * 10;
          PlaySoundEffect(SOUND_EFFECT_BUMPER_HIT_STD_1);
        }
      }
      if (BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
      LastSwitchHitTime = CurrentTime;
      break;

    case SW_RIGHT_POP:
      if (TurbulenceMode) {
        CurrentScores[CurrentPlayer] += PlayfieldMultiplier * 1000;
        PlaySoundEffect(SOUND_EFFECT_DING);
      } else {
        if (PopBumperPhase==0) {
          CurrentScores[CurrentPlayer] += PlayfieldMultiplier * 100;
          PlaySoundEffect(SOUND_EFFECT_DROP_TARGET_HIT);
        } else {
          CurrentScores[CurrentPlayer] += PlayfieldMultiplier * 10;
          PlaySoundEffect(SOUND_EFFECT_BUMPER_HIT_STD_1);
        }
      }
      if (BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
      LastSwitchHitTime = CurrentTime;
      break;

    case SW_LEFT_POP:
      LowerSpecialPhase ^= 1;
      if (TurbulenceMode) {
        CurrentScores[CurrentPlayer] += PlayfieldMultiplier * 1000;
        PlaySoundEffect(SOUND_EFFECT_DING);
      } else {
        if (PopBumperPhase==0) {
          CurrentScores[CurrentPlayer] += PlayfieldMultiplier * 100;
          PlaySoundEffect(SOUND_EFFECT_DROP_TARGET_HIT);
        } else {
          CurrentScores[CurrentPlayer] += PlayfieldMultiplier * 10;
          PlaySoundEffect(SOUND_EFFECT_BUMPER_HIT_STD_1);
        }
      }
      if (BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
      LastSwitchHitTime = CurrentTime;
      break;
    case SW_SPINNER:
      SpinnerHits[CurrentPlayer] += 1;
      if (SpinnerHits[CurrentPlayer]>250) SpinnerHits[CurrentPlayer] = 248;
      PlaySoundEffect(SOUND_EFFECT_SPINNER_STD);
      if (GameMode == GAME_MODE_CRUISING) {
        LegProgress[1] += 1;
        if (LegProgress[1]>=legGoal) {
          LegProgress[1] = legGoal;        
          CurrentMissionLegs[CurrentPlayer] |= MISSION_LEG_CRUISING_COMPLETE;
        }
        if (LegStartScoring) {
          CurrentScores[CurrentPlayer] += PlayfieldMultiplier * 2000;
        } else {
          CurrentScores[CurrentPlayer] += PlayfieldMultiplier * 1000;
        }
      } else if (GameMode==GAME_MODE_SPECIAL_TOUR_SPINNER) {
        CurrentScores[CurrentPlayer] += PlayfieldMultiplier * 2500;
      } else {
        if (!(CurrentMissionLegs[CurrentPlayer]&MISSION_LEG_CRUISING_COMPLETE)) {
          if ((AdjustedDifficulty%10)<2) {
            // if the AdjustedDifficulty is 0, 1, 10, or 11, we can complete legs out of order
            LegProgress[1] += 1;
            if (LegProgress[1]>=legGoal) {
              LegProgress[1] = legGoal;
              CurrentMissionLegs[CurrentPlayer] |= MISSION_LEG_CRUISING_COMPLETE;
            }
          }
        }
        if (TurbulenceMode) {
          CurrentScores[CurrentPlayer] += PlayfieldMultiplier * 500;
        } else {
          CurrentScores[CurrentPlayer] += PlayfieldMultiplier * 100;
        }
      }

      // Increase turbulence speed if it's already started
      if (TurbulenceRotationTarget && !TurbulenceMode && (SpinnerHits[CurrentPlayer]%2)==0) {
        TurbulenceRotationTarget -= TURBULENCE_ROTATION_PERDIOD_CHANGE;
        if (TurbulenceRotationTarget < MIN_TURBULENCE_ROTATION_PERDIOD) {
          TurbulenceRotationTarget = MIN_TURBULENCE_ROTATION_PERDIOD;
          if (!TurbulenceMode) {
            PlaySoundEffect(SOUND_EFFECT_TURBULENCE);
            TurbulenceMode = true;
            NextTurbulenceSolenoidChange = CurrentTime + 3000 + ((CurrentTime%3)*1000);
            TurbulenceModeStartTime = CurrentTime;
            IncreasePlayfieldMultiplier(30000);
            Audio.StopSound(SOUND_EFFECT_SINGLE_BONG);
            PlaySoundEffect(SOUND_EFFECT_SINGLE_BONG);
            QueueNotification(SOUND_EFFECT_VP_CAPTAIN_SEATBELTS_1 + CaptainNumber[CurrentPlayer], 7);
          }
        }
      }      

      if (BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
      LastSwitchHitTime = CurrentTime;
      break;

    case SW_INLANES_AND_10_PT:
      if (GameMode == GAME_MODE_SKILL_SHOT_PRE_SWITCH || GameMode == GAME_MODE_SKILL_SHOT_BUILD_BONUS) {
        SkillShotAward += 1000;
        if (GameMode == GAME_MODE_SKILL_SHOT_PRE_SWITCH) SetGameMode(GAME_MODE_SKILL_SHOT_BUILD_BONUS);
      } else {
        Audio.StopSound(SOUND_EFFECT_SINGLE_BONG);
        PlaySoundEffect(SOUND_EFFECT_SINGLE_BONG);
        CurrentScores[CurrentPlayer] += PlayfieldMultiplier * 300;
        LastSwitchHitTime = CurrentTime;
      }
      //      if (BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
      break;

    case SW_SAUCER:
      // Debounce the saucer switch
      if ((CurrentTime - LastSaucerHitTime) > 500) {
        if (GameMode == GAME_MODE_LANDING) {
          CurrentMissionLegs[CurrentPlayer] |= MISSION_LEG_LANDING_COMPLETE;
        } else if (GameMode==GAME_MODE_SPECIAL_TOUR_SAUCER) {
          PlaySoundEffect(SOUND_EFFECT_DING);
          QueueNotification(SOUND_EFFECT_VP_SUPER_JACKPOT, 4);
          StartScoreAnimation(PlayfieldMultiplier * 50000);          
          RPU_PushToTimedSolenoidStack(SOL_SAUCER, 5, CurrentTime + 100, true);
        } else {
          PlaySoundEffect(SOUND_EFFECT_ADVANCED_SCORE_2);
          RPU_PushToTimedSolenoidStack(SOL_SAUCER, 5, CurrentTime + 100, true);
        }
        LastSaucerHitTime = CurrentTime;
        CurrentScores[CurrentPlayer] += 3000;
        IncreaseBonusX();
        AddToBonus(1);
      }
      break;
  }

}

int RunGamePlayMode(int curState, boolean curStateChanged) {
  int returnState = curState;
  unsigned long scoreAtTop = CurrentScores[CurrentPlayer];

  // Very first time into gameplay loop
  if (curState == MACHINE_STATE_INIT_GAMEPLAY) {
    returnState = InitGamePlay(curStateChanged);
  } else if (curState == MACHINE_STATE_INIT_NEW_BALL) {
    returnState = InitNewBall(curStateChanged, CurrentPlayer, CurrentBallInPlay);
  } else if (curState == MACHINE_STATE_NORMAL_GAMEPLAY) {
    returnState = ManageGameMode();
  } else if (curState == MACHINE_STATE_COUNTDOWN_BONUS) {
    // Reset lamps to reasonable state (in case they were in an animation)
    ShowBonusXLamps();
    ShowShootAgainLamp();
    returnState = CountdownBonus(curStateChanged);
    ShowPlayerScores(0xFF, false, false);
  } else if (curState == MACHINE_STATE_BALL_OVER) {
    RPU_SetDisplayCredits(Credits, !FreePlayMode);

    if (SamePlayerShootsAgain) {
      QueueNotification(SOUND_EFFECT_VP_SHOOT_AGAIN, 8);
      returnState = MACHINE_STATE_INIT_NEW_BALL;
    } else {

      CurrentPlayer += 1;
      if (CurrentPlayer >= CurrentNumPlayers) {
        CurrentPlayer = 0;
        CurrentBallInPlay += 1;
      }

      scoreAtTop = CurrentScores[CurrentPlayer];

      if (CurrentBallInPlay > BallsPerGame) {
        CheckHighScores();
        PlaySoundEffect(SOUND_EFFECT_GAME_OVER);
        for (int count = 0; count < CurrentNumPlayers; count++) {
          RPU_SetDisplay(count, CurrentScores[count], true, 2);
        }

        returnState = MACHINE_STATE_MATCH_MODE;
      }
      else returnState = MACHINE_STATE_INIT_NEW_BALL;
    }
  } else if (curState == MACHINE_STATE_MATCH_MODE) {
    returnState = ShowMatchSequence(curStateChanged);
  }

  unsigned long lastBallFirstSwitchHitTime = BallFirstSwitchHitTime;
  byte switchHit;

  while ( (switchHit = RPU_PullFirstFromSwitchStack()) != SWITCH_STACK_EMPTY ) {
    returnState = HandleSystemSwitches(curState, switchHit);
    if (NumTiltWarnings <= MaxTiltWarnings) HandleGamePlaySwitches(switchHit);
  }

  if (lastBallFirstSwitchHitTime == 0 && BallFirstSwitchHitTime != 0) {
    BallSaveEndTime = BallFirstSwitchHitTime + ((unsigned long)CurrentBallSaveNumSeconds) * 1000;
  }
  if (CurrentTime > (BallSaveEndTime + BALL_SAVE_GRACE_PERIOD)) {
    BallSaveEndTime = 0;
  }

  if (!ScrollingScores && CurrentScores[CurrentPlayer] > RPU_OS_MAX_DISPLAY_SCORE) {
    CurrentScores[CurrentPlayer] -= RPU_OS_MAX_DISPLAY_SCORE;
  }

  if (CreditResetPressStarted) {

    if (CurrentBallInPlay < 2) {
      // If we haven't finished the first ball, we can add players
      AddPlayer();
      if (DEBUG_MESSAGES) {
        Serial.write("Start game button pressed\n\r");
      }
      CreditResetPressStarted = 0;
    } else {

      if (RPU_ReadSingleSwitchState(SW_CREDIT_RESET)) {
        if (TimeRequiredToResetGame != 99 && (CurrentTime - CreditResetPressStarted) >= ((unsigned long)TimeRequiredToResetGame * 1000)) {
          // If the first ball is over, pressing start again resets the game
          if (Credits >= 1 || FreePlayMode) {
            if (!FreePlayMode) {
              Credits -= 1;
              RPU_WriteByteToEEProm(RPU_CREDITS_EEPROM_BYTE, Credits);
              RPU_SetDisplayCredits(Credits, !FreePlayMode);
            }
            returnState = MACHINE_STATE_INIT_GAMEPLAY;
            CreditResetPressStarted = 0;
          }
        }
      } else {
        CreditResetPressStarted = 0;
      }
    }

  }

  if (scoreAtTop != CurrentScores[CurrentPlayer]) {
    LastTimeScoreChanged = CurrentTime;
    if (!TournamentScoring) {
      for (int awardCount = 0; awardCount < 3; awardCount++) {
        if (AwardScores[awardCount] != 0 && scoreAtTop < AwardScores[awardCount] && CurrentScores[CurrentPlayer] >= AwardScores[awardCount]) {
          // Player has just passed an award score, so we need to award it
          if (((ScoreAwardReplay >> awardCount) & 0x01)) {
            AddSpecialCredit();
          } else {
            AwardExtraBall();
          }
        }
      }
    }

  }

  return returnState;
}


void loop() {

  RPU_DataRead(0);

  CurrentTime = millis();
  int newMachineState = MachineState;

  if (MachineState < 0) {
    newMachineState = RunSelfTest(MachineState, MachineStateChanged);
  } else if (MachineState == MACHINE_STATE_ATTRACT) {
    newMachineState = RunAttractMode(MachineState, MachineStateChanged);
  } else {
    newMachineState = RunGamePlayMode(MachineState, MachineStateChanged);
  }

  if (newMachineState != MachineState) {
    MachineState = newMachineState;
    MachineStateChanged = true;
  } else {
    MachineStateChanged = false;
  }

  RPU_Update(CurrentTime);
  Audio.Update(CurrentTime);
}
