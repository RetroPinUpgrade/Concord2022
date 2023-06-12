#ifndef SUPERSONIC_H



boolean GeneralIlluminationOn = true;

#define LAMP_BONUS_1K                  0
#define LAMP_BONUS_2K                  1
#define LAMP_BONUS_3K                  2
#define LAMP_BONUS_4K                  3
#define LAMP_BONUS_5K                  4
#define LAMP_BONUS_6K                  5
#define LAMP_BONUS_7K                  6
#define LAMP_BONUS_8K                  7
#define LAMP_BONUS_9K                  8
#define LAMP_BONUS_10K                 9
#define LAMP_BONUS_20K                 10

#define LAMP_1                          12
#define LAMP_3                          13
#define LAMP_4                          14
#define LAMP_5                          15
#define LAMP_2                          16
#define LAMP_2_ROLLOVER                 17
#define LAMP_2_LEFT_LANE                18
#define LAMP_TOP_3K_WHEN_LIT            19
#define LAMP_JET_STREAM_AND_CENTER      20
#define LAMP_SPINNER                    21
#define LAMP_GATE_OPEN                  22
#define LAMP_OPENS_GATE                 23
#define LAMP_1_ROLLOVER                 24
#define LAMP_3_ROLLOVER                 25
#define LAMP_4_ROLLOVER                 26
#define LAMP_5_ROLLOVER                 27
#define LAMP_1_LEFT_LANE                28
#define LAMP_3_LEFT_LANE                29
#define LAMP_4_LEFT_LANE                30
#define LAMP_5_LEFT_LANE                31
#define LAMP_5X_BONUS                   32
#define LAMP_3X_5X_BONUS                33
#define LAMP_2X_3X_BONUS                34
#define LAMP_2X_BONUS                   35
#define LAMP_TOP_POPS                   36
#define LAMP_BOTTOM_POP                 37
#define LAMP_RIGHT_OUTLANE_SPECIAL      38
#define LAMP_LEFT_OUTLANE_SPECIAL       39
#define LAMP_HEAD_SAME_PLAYER           40
#define LAMP_HEAD_MATCH                 41
#define LAMP_SHOOT_AGAIN                42
#define LAMP_APRON_CREDIT               43
#define LAMP_RIGHT_TARGETS_3K           44
#define LAMP_CENTER_TARGET_3K           45
#define LAMP_DT_EXTRA_BALL              46
#define LAMP_DT_SPECIAL                 47
#define LAMP_HEAD_BIP                   48
#define LAMP_HEAD_HIGH_SCORE            49
#define LAMP_HEAD_GAME_OVER             50
#define LAMP_HEAD_TILT                  51
#define LAMP_HEAD_1_PLAYER              52
#define LAMP_HEAD_2_PLAYER              53
#define LAMP_HEAD_3_PLAYER              54
#define LAMP_HEAD_4_PLAYER              55
#define LAMP_HEAD_PLAYER_1_UP           56
#define LAMP_HEAD_PLAYER_2_UP           57
#define LAMP_HEAD_PLAYER_3_UP           58
#define LAMP_HEAD_PLAYER_4_UP           59



#define SW_DROP_TARGET_5            0
#define SW_DROP_TARGET_4            1
#define SW_DROP_TARGET_3            2
#define SW_DROP_TARGET_2            3
#define SW_DROP_TARGET_1            4
#define SW_CREDIT_RESET             5
#define SW_TILT                     6
#define SW_OUTHOLE                  7
#define SW_COIN_3                   8
#define SW_COIN_1                   9
#define SW_COIN_2                   10
#define SW_TOP_ROLLOVER             11
#define SW_DROP_TARGET_10_PT        12
#define SW_RIGHT_OUTLANE            13
#define SW_LEFT_OUTLANE             14
#define SW_SLAM                     15
#define SW_SPINNER                  16
#define SW_5_LANE_AND_TARGET        17
#define SW_4_LANE_AND_TARGET        18
#define SW_3_LANE_AND_TARGET        19
#define SW_2_LANE_AND_TARGET        20
#define SW_1_LANE_AND_TARGET        21
#define SW_INLANES_AND_10_PT        22
#define SW_SAUCER                   23

#define SW_5_ROLLOVER               25
#define SW_4_ROLLOVER               26
#define SW_3_ROLLOVER               27
#define SW_2_ROLLOVER               28
#define SW_1_ROLLOVER               29
#define SW_RIGHT_SLING              30
#define SW_LEFT_SLING               31
#define SW_BOTTOM_POP               32
#define SW_5_LEFT_LANE              33
#define SW_4_LEFT_LANE              34
#define SW_3_LEFT_LANE              35
#define SW_2_LEFT_LANE              36
#define SW_1_LEFT_LANE              37
#define SW_RIGHT_POP                38
#define SW_LEFT_POP                 39

#define SOL_KNOCKER               5
#define SOL_OUTHOLE               6
#define SOL_SAUCER                7
#define SOL_LEFT_POP              8
#define SOL_RIGHT_POP             9
#define SOL_BOTTOM_POP            10
#define SOL_LEFT_SLING            11
#define SOL_RIGHT_SLING           12
#define SOL_5_BANK_RESET          13

#define SOLCONT_GATE              0x10

//#define SOL_NONE          15


#define NUM_SWITCHES_WITH_TRIGGERS          5
#define NUM_PRIORITY_SWITCHES_WITH_TRIGGERS 5

struct PlayfieldAndCabinetSwitch SolenoidAssociatedSwitches[] = {
  { SW_RIGHT_SLING, SOL_RIGHT_SLING, 4},
  { SW_LEFT_SLING, SOL_LEFT_SLING, 4},
  { SW_BOTTOM_POP, SOL_BOTTOM_POP, 6},
  { SW_LEFT_POP, SOL_LEFT_POP, 6},
  { SW_RIGHT_POP, SOL_RIGHT_POP, 6}
};


#define SUPERSONIC_H
#endif
