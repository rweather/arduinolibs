/*
 * Copyright (C) 2012 Southern Storm Software, Pty Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef RC5_h
#define RC5_h

// http://en.wikipedia.org/wiki/RC-5#System_Number_Allocations
#define RC5_SYS_TV              0   // TV receiver 1
#define RC5_SYS_TV2             1   // TV receiver 2
#define RC5_SYS_TXT             2   // Teletext
#define RC5_SYS_TV_EXT          3   // Extension to TV 1 & 2
#define RC5_SYS_LV              4   // Laservision player
#define RC5_SYS_VCR             5   // VCR 1
#define RC5_SYS_VCR2            6   // VCR 2
#define RC5_SYS_SAT             8   // Satellite receiver 1
#define RC5_SYS_VCR_EXT         9   // Extension to VCR 1 & 2
#define RC5_SYS_SAT2            10  // Satellite receiver 2
#define RC5_SYS_CD_VIDEO        12  // CD video player
#define RC5_SYS_CD_PHOTO        14  // CD photo player
#define RC5_SYS_PREAMP          16  // Audio preamplifier 1
#define RC5_SYS_RADIO           17  // Radio tuner
#define RC5_SYS_REC             18  // Casette recorder 1
#define RC5_SYS_PREAMP2         19  // Audio preamplifier 2
#define RC5_SYS_CD              20  // CD player
#define RC5_SYS_COMBI           21  // Audio stack or record player
#define RC5_SYS_AUDIO_SAT       22  // Audio satellite
#define RC5_SYS_REC2            23  // Casette recorder 2
#define RC5_SYS_CD_R            26  // CD recorder

// http://en.wikipedia.org/wiki/RC-5#Command_Tables
// Table 2, Common commands:
#define RC5_0                   0   // Digit 0
#define RC5_1                   1   // Digit 1
#define RC5_2                   2   // Digit 2
#define RC5_3                   3   // Digit 3
#define RC5_4                   4   // Digit 4
#define RC5_5                   5   // Digit 5
#define RC5_6                   6   // Digit 6
#define RC5_7                   7   // Digit 7
#define RC5_8                   8   // Digit 8
#define RC5_9                   9   // Digit 9
#define RC5_INC_VOLUME          16  // Increase sound volume
#define RC5_DEC_VOLUME          17  // Decrease sound volume
#define RC5_INC_BRIGHTNESS      18  // Increase display brightness
#define RC5_DEC_BRIGHTNESS      19  // Decrease display brightness
#define RC5_INC_BASS            22  // Increase bass response
#define RC5_DEC_BASS            23  // Decrease bass response
#define RC5_INC_TREBLE          24  // Increase treble response
#define RC5_DEC_TREBLE          25  // Decrease treble response
#define RC5_BALANCE_LEFT        26  // Shift sound balance to left
#define RC5_BALANCE_RIGHT       27  // Shift sound balance to right
#define RC5_TRANSMIT_MODE       63  // Select remote transmit mode
#define RC5_DIM                 71  // Dim local display
#define RC5_INC_LINEAR          77  // Increase linear control
#define RC5_DEC_LINEAR          78  // Decrease linear control
#define RC5_UP                  80  // Move cursor up
#define RC5_DOWN                81  // Move cursor down
#define RC5_MENU_ON             82  // Switch display/screen menu on
#define RC5_MENU_OFF            83  // Switch display/screen menu off
#define RC5_AV_STATUS           84  // Display A/V system status
#define RC5_LEFT                85  // Move cursor left
#define RC5_RIGHT               86  // Move cursor right
#define RC5_OK                  87  // Acknowledge function at cursor
#define RC5_SUBMODE             118 // Select sub-mode
#define RC5_OPTIONS             119 // Select options sub-mode
#define RC5_CONNECT_EURO        123 // Connect items via Euroconnector
#define RC5_DISCONNECT_EURO     124 // Disconnect items via Euroconnector

// http://en.wikipedia.org/wiki/RC-5#Command_Tables
// Table 3, Common video system commands:
#define RC5_INC_SATURATION      20  // Increase color saturation
#define RC5_DEC_SATURATION      21  // Decrease color saturation
#define RC5_PIP                 88  // Picture-in-picture on/off
#define RC5_PIP_SHIFT           89  // Picture-in-picture shift
#define RC5_PIP_SWAP            90  // Picture-in-picture swap
#define RC5_PIP_STROBE          91  // Strobe main picture on/off
#define RC5_PIP_MULTI_STROBE    92  // Multi-strobe
#define RC5_PIP_FREEZE_MAIN     93  // Main picture frame frozen
#define RC5_PIP_MULTI_SCAN      94  // 3/9 multi-scan
#define RC5_PIP_SOURCE          95  // Select picture-in-picture source
#define RC5_PIP_MOSAIC          96  // Mosaic/multi-PIP
#define RC5_PIP_NOISE           97  // Digital noise reduction of picture
#define RC5_PIP_STORE           98  // Store main picture
#define RC5_PIP_PHOTO_FINISH    99  // PIP strobe; display photo-finish
#define RC5_PIP_RECALL          100 // Recall main stored picture
#define RC5_PIP_FREEZE          101 // Freeze PIP
#define RC5_PIP_UP              102 // Step up PIP options/source
#define RC5_PIP_DOWN            103 // Step down PIP options/source

// http://en.wikipedia.org/wiki/RC-5#Command_Tables
// Table 4a, TV and VCR commands:
#define RC5_123                 10  // 1/2/3 digit entry
#define RC5_11                  11  // Channel/program/frequency 11
#define RC5_STANDBY             12  // Standby
#define RC5_MUTE                13  // Master mute/de-mute
#define RC5_PREFERENCES         14  // Personal preference settings
#define RC5_DISPLAY_INFO        15  // Display user info on screen
#define RC5_INC_CONTRAST        28  // Increase picture contrast
#define RC5_DEC_CONTRAST        29  // Decrease picture contrast
#define RC5_SEARCH_UP           30  // Search up
#define RC5_DEC_TINT            31  // Decrease tint/hue
#define RC5_CHANNEL_UP          32  // Channel/program up
#define RC5_CHANNEL_DOWN        33  // Channel/program down
#define RC5_CHANNEL_LAST        34  // Last viewed channel/program
#define RC5_STEREO_SELECT       35  // Select stereo channel/language
#define RC5_STEREO_SPATIAL      36  // Spatial stereo
#define RC5_STEREO_TOGGLE       37  // Toggle stereo/mono
#define RC5_SLEEP_TIMER         38  // Sleep timer
#define RC5_INC_TINT            39  // Increase tint/hue
#define RC5_SWITCH_RF           40  // Switch RF inputs
#define RC5_STORE               41  // Store/vote
#define RC5_TIME                42  // Display time
#define RC5_INC_SCAN            43  // Scan forward/increment
#define RC5_DEC_SCAN            44  // Scan backward/decrement
#define RC5_SECONDARY_MENU      46  // Secondary menu
#define RC5_CLOCK               47  // Show clock
#define RC5_PAUSE               48  // Pause
#define RC5_ERASE               49  // Erase/correct entry
#define RC5_REWIND              50  // Rewind
#define RC5_GOTO                51  // Go to
#define RC5_WIND                52  // Wind (fast forward)
#define RC5_PLAY                53  // Play
#define RC5_STOP                54  // Stop
#define RC5_RECORD              55  // Record
#define RC5_EXTERNAL1           56  // External 1
#define RC5_EXTERNAL2           57  // External 2
#define RC5_VIEW_DATA           59  // View data, advance
#define RC5_12                  60  // Channel 12 (or TXT/TV toggle)
#define RC5_SYSTEM_STANDBY      61  // System standby
#define RC5_CRISP               62  // Picture crispener (coutour boost)
#define RC5_AUDIO_RESPONSE      70  // Audio response for speech/music
#define RC5_SOUND_FUNCTIONS     79  // Select sound functions in sequence
#define RC5_PIP_SIZE            104 // Alter PIP size step-by-step
#define RC5_VISION_FUNCTIONS    105 // Select vision functions in sequence
#define RC5_COLOR_KEY           106 // Colored or other special key
#define RC5_RED                 107 // Red button
#define RC5_GREEN               108 // Green button
#define RC5_YELLOW              109 // Yellow button
#define RC5_CYAN                110 // Cyan button
#define RC5_INDEX               111 // Index page/white function
#define RC5_NEXT_OPTION         112 // Next option
#define RC5_PREVIOUS_OPTION     113 // Previous option
#define RC5_STORE_OPEN_CLOSE    122 // Store open/close
#define RC5_PARENTAL_ACCESS     123 // Parental access via PIN code

// http://en.wikipedia.org/wiki/RC-5#Command_Tables
// Table 4b, TV1 and TV2 extension
#define RC5_DEFAULT_VIDEO       10  // Default video settings (TV1)
#define RC5_DEFAULT_AUDIO       11  // Default audio settings (TV1)
#define RC5_PAYTV_CHANNEL_UP    28  // Pay TV channel up (TV1)
#define RC5_PAYTV_CHANNEL_DOWN  29  // Pay TV channel down (TV1)
#define RC5_RADIO_CHANNEL_UP    30  // Radio channel up (TV1)
#define RC5_RADIO_CHANNEL_DOWN  31  // Radio channel down (TV1)
#define RC5_TILT_FORWARD        32  // Tilt cabinet forward (TV1)
#define RC5_TILT_BACKWARD       33  // Tilt cabinet backward (TV1)
#define RC5_EXTERNAL3           56  // External 3 (TV1)
#define RC5_EXTERNAL4           56  // External 4 (TV1)
#define RC5_PICTURE_FORMAT      62  // 4:3 vs 16:9 (TV1)
#define RC5_CHANNEL_10          67  // Channel 10
#define RC5_CHANNEL_11          68  // Channel 11
#define RC5_CHANNEL_12          69  // Channel 12
#define RC5_DEFAULT_VIDEO2      72  // Default video settings (TV2)
#define RC5_DEFAULT_AUDIO2      73  // Default audio settings (TV2)
#define RC5_PAYTV_CHANNEL_UP2   88  // Pay TV channel up (TV2)
#define RC5_PAYTV_CHANNEL_DOWN2 89  // Pay TV channel down (TV2)
#define RC5_RADIO_CHANNEL_UP2   90  // Radio channel up (TV2)
#define RC5_RADIO_CHANNEL_DOWN2 91  // Radio channel down (TV2)
#define RC5_TILT_FORWARD2       104 // Tilt cabinet forward (TV2)
#define RC5_TILT_BACKWARD2      105 // Tilt cabinet backward (TV2)
#define RC5_EXTERNAL3_2         120 // External 3 (TV2)
#define RC5_EXTERNAL4_2         121 // External 4 (TV2)
#define RC5_CHANNEL_MENU        122 // Channel setting menu
#define RC5_PICTURE_FORMAT2     126 // 4:3 vs 16:9 (TV2)

// http://en.wikipedia.org/wiki/RC-5#Command_Tables
// Table 5, Teletext commands
#define RC5_NEXT_PAGE           10  // Next page
#define RC5_PREVIOUS_PAGE       11  // Previous page
//      RC5_STANDBY             12  // Standby
#define RC5_ENTER_PAGE_NUMBER   28  // Enter page number in memory
#define RC5_SEQ_DISPLAY         29  // Sequential display of pages
#define RC5_SEQ_DELETE          30  // Sequential display/deletion of pages
#define RC5_EXCHANGE            32  // Exchange (Antiope function)
#define RC5_MAIN_INDEX          33  // Main index
#define RC5_ROW_ZERO            34  // Row zero (Antiope function)
#define RC5_PRINT               38  // Print displayed page
#define RC5_MIX                 39  // Mix Antiope/TV pictures
#define RC5_HOLD_PAGE           41  // Page hold
//      RC5_TIME                42  // Display time
#define RC5_LARGE               43  // Large top/bottom/normal
#define RC5_REVEAL              44  // Reveal/conceal
#define RC5_TV_TXT              45  // TV/TXT
#define RC5_TV_TXT_SUBTITLE     46  // TV + TXT/subtitle
//      RC5_ERASE               49  // Erase/correct entry
#define RC5_NEWS_FLASH          62  // News flash (Antiope function)

// http://en.wikipedia.org/wiki/RC-5#Command_Tables
// Table 6, LaserVision commands
#define RC5_PICTURE_NUMBER      10  // Display picture number/time
#define RC5_CHAPTER_NUMBER      11  // Display chapter number
//      RC5_STANDBY             12  // Standby
//      RC5_MUTE                13  // Master mute/de-mute
//      RC5_DISPLAY_INFO        15  // Display user info on screen
#define RC5_SHUFFLE             28  // Total shuffle play/repeat once
#define RC5_REPEAT              29  // Repeat continuously
#define RC5_SELECT_NEXT         30  // Select next option
#define RC5_FAST_REVERSE        31  // Fast run reverse
#define RC5_ENTRY               32  // Entry (prepare to program)
#define RC5_AUTO_STOP           33  // Auto-stop at pre-programmed point
#define RC5_SLOW_REVERSE        34  // Slow run reverse
#define RC5_STEREO_CHANNEL1     35  // Select stereo sound channel 1/language 1
#define RC5_STEREO_CHANNEL2     36  // Select stereo sound channel 2/language 2
#define RC5_DEC_STILL           37  // Still increment reverse
#define RC5_INC_SPEED           38  // Increase speed
#define RC5_DEC_SPEED           39  // Decrease speed
#define RC5_SLOW_FORWARD        40  // Slow run forward
#define RC5_INC_STILL           41  // Still increment forward
#define RC5_FAST_FORWARD        42  // Fast run forward
#define RC5_SEARCH_USER_CHOICE  43  // Automatic search for user choice
#define RC5_SEARCH_REVERSE      44  // Search in reverse
#define RC5_TRAY                45  // Open/close tray
#define RC5_SEARCH_FORWARD      46  // Search forward
#define RC5_PLAY_REVERSE        47  // Play reverse/play opposite sound track
//      RC5_PAUSE               48  // Pause
//      RC5_ERASE               49  // Erase/correct entry
//      RC5_PLAY                53  // Play
//      RC5_STOP                54  // Stop
#define RC5_CLEAR_MEMORY        58  // Clear memory all
#define RC5_FREEZE_SEGMENT      59  // Freeze segment(s) indicated by picture numbers.
#define RC5_TV_TXT_ALT          60  // TV/TXT toggle; RF switch (USA only)
#define RC5_CX                  62  // CX 1, 2, 3; toggle for CX noise reduction

// http://en.wikipedia.org/wiki/RC-5#Command_Tables
// Table 11, Preamplifier commands
#define RC5_GEQ_L               10  // Graphic equalizer left
#define RC5_GEQ_R               11  // Graphic equalizer right
//      RC5_STANDBY             12  // Standby
//      RC5_MUTE                13  // Master mute/de-mute
//      RC5_PREFERENCES         14  // Personal preference settings
//      RC5_DISPLAY_INFO        15  // Display user info on screen
#define RC5_GEQ_L_AND_R         28  // Graphic equalizer left and right
#define RC5_SPEAKER_SELECT      29  // Speaker select
#define RC5_SCRATCH_FILTER      30  // Scratch filter on/off
#define RC5_RUMBLE_FILTER       31  // Rumble filter on/off
#define RC5_INC_STEP            32  // Step function +
#define RC5_DEC_STEP            33  // Step function -
#define RC5_SIGNAL_PATH         34  // Signal path options
#define RC5_SPEAKER_A           35  // Speaker A on/off
#define RC5_SURROUND_OPTIONS    37  // Surround sound options
//      RC5_SLEEP_TIMER         38  // Sleep timer
#define RC5_SPEAKER_B           39  // Speaker B on/off
#define RC5_SPEAKER_C           40  // Speaker C on/off
#define RC5_TIMER_PROGRAM       41  // Timer program mode
//      RC5_TIME                42  // Display time
#define RC5_INC_TIMER           43  // Timer +
#define RC5_DEC_TIMER           44  // Timer -
#define RC5_TIMER_MEMORY        45  // Open timer memory
#define RC5_ACOUSTIC_CONTROL    46  // Open acoustic control setting memory
#define RC5_ACOUSTIC_SELECT     47  // Select acoustic settings in memory
//      RC5_ERASE               49  // Erase/correct entry
//      RC5_CLEAR_MEMORY        58  // Clear memory all
#define RC5_DYNAMIC_EXPAND      60  // Dynamic range expand
#define RC5_DYNAMIC_COMPRESS    62  // Dynamic range compress
#define RC5_SURROUND_SOUND      64  // Surround sound on/off
#define RC5_BALANCE_FRONT       65  // Balance front
#define RC5_BALANCE_REAR        66  // Balance rear
#define RC5_LINEAR_SOUND        79  // Scroll linear sound functions
#define RC5_RANDOM_NOISE        88  // Random noise generator on/off
#define RC5_TIMER               89  // Timer on/off
#define RC5_NEWS_TIMER          90  // News timer on/off
#define RC5_INC_CENTER_VOLUME   102 // Increase center channel volume
#define RC5_DEC_CENTER_VOLUME   103 // Decrease center channel volume
#define RC5_INC_DELAY_SURROUND  104 // Increase delay front to surround
#define RC5_DEC_DELAY_SURROUND  105 // Decrease delay front to surround
#define RC5_LINEAR_PHASE        106 // Linear phase
#define RC5_TAPE_MONITOR        122 // Tape monitor

// http://en.wikipedia.org/wiki/RC-5#Command_Tables
// Table 14, Compact disc player commands
#define RC5_LOCAL_CURSOR        10  // Scroll local display cursor
#define RC5_LOCAL_FUNCTION      11  // Scroll local display function
//      RC5_STANDBY             12  // Standby
//      RC5_MUTE                13  // Master mute/de-mute
//      RC5_DISPLAY_INFO        15  // Display user info on screen
//      RC5_SHUFFLE             28  // Total shuffle play/repeat once
//      RC5_REPEAT              29  // Repeat continuously
#define RC5_INC_SELECT          30  // Select increment
#define RC5_DEC_SELECT          31  // Select decrement
#define RC5_NEXT                32  // Next
#define RC5_PREVIOUS            33  // Previous
#define RC5_INDEX_NEXT          34  // Index next
#define RC5_INDEX_PREVIOUS      35  // Index previous
#define RC5_PLAY_PROGRAM        36  // Play/program
#define RC5_NOMINAL_SPEED       37  // Speed nominal
//      RC5_INC_SPEED           38  // Increase speed
//      RC5_DEC_SPEED           39  // Decrease speed
//      RC5_STORE               41  // Store/vote
//      RC5_INC_SCAN            43  // Scan forward/increment
//      RC5_TRAY                45  // Open/close tray
#define RC5_CARTRIDGE           47  // Fast/select disc from catridge
//      RC5_PAUSE               48  // Pause
//      RC5_ERASE               49  // Erase/correct entry
//      RC5_REWIND              50  // Rewind
//      RC5_GOTO                51  // Go to
//      RC5_WIND                52  // Wind (fast forward)
//      RC5_PLAY                53  // Play
//      RC5_STOP                54  // Stop
//      RC5_CLEAR_MEMORY        58  // Clear memory all
#define RC5_REPEAT_AB           59  // Repeat program marked A/B
//      RC5_DYNAMIC_EXPAND      60  // Dynamic range expand
//      RC5_DYNAMIC_COMPRESS    62  // Dynamic range compress
#define RC5_DSP                 91  // Digital signal processing on/off
#define RC5_DSP_MUSIC           92  // Music mode (DSP)
#define RC5_DSP_ACOUSTICS       93  // Select room acoustics (DSP)
#define RC5_DSP_JAZZ            94  // Jazz/s-hall effect (DSP)
#define RC5_DSP_POP             95  // Pop/s-hall effect (DSP)
#define RC5_DSP_CLASSIC         96  // Classic/church music for music/room mode (DSP)
#define RC5_DSP_EASY            97  // Easy/club music for music/room mode (DSP)
#define RC5_DSP_DISCO           98  // Disco/stadium music for music/room mode (DSP)
#define RC5_SECOND_FAVORITE     107 // Second favorite track selection
#define RC5_FAVORITE            108 // Favorite track selection
#define RC5_TITLE_INTO_MEMORY   109 // Title into memory
#define RC5_FADE                120 // Fade in/out audio

#endif
