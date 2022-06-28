"""
This module defines the AMK object used to store all the keyboard data,
and module contains the <keyname>:<keycode> mapping used in the keyboard.
"""

import inspect

class AMK: #ArduinoMacroKeyboard:
    def __init__(self, **kwargs):
        """Constructs an AMK object."""
        # self.layout = 
        self.serial_port = kwargs.get('serial_port', None)
        self.num_rows = kwargs.get('num_rows', None)
        self.num_cols = kwargs.get('num_cols', None)
        self.num_sets = kwargs.get('num_sets', None)
        self.bindings = kwargs.get('bindings', None)
        self.orientaion = kwargs.get('orientation', None)
        self.active_set = kwargs.get('active_set', None)
        self.name = kwargs.get('name', None)
        self.arduino_callback = kwargs.get('arduino_callback', None)
        #attribute arduino ready?

#region constructors
    @classmethod
    def init_from_dict(cls, dict, callback):
        """Constructs an AMK object from a filled dictionary."""
        # layout = ( , , )
        rows = dict['num_rows']
        cols = dict['num_cols']
        sets = dict['num_sets']
        bindings = dict['bindings']
        orientaion = dict['orientation']
        active_set = dict['active_set']
        name = dict['name']
        new_keyboard = cls(rows, cols, sets, bindings, orientaion, active_set,
                           name, callback)
        return new_keyboard
#endregion

#region getters
    def __getitem__(self, key):
        """Returns requested attribute."""
        return getattr(self, key)

    def is_full(self):
        """Returns ''True'' if AMK data is full, i.e. all attributes \
            have non-default values"""
        uninitialized_attributes = [k for k, v in vars(self).items()
                                    if (not k.startswith('_')) and v==None]
        if uninitialized_attributes:
            print(f'AMK object has uninitialized attributes: \
                  {uninitialized_attributes}')
        return True if len(uninitialized_attributes) == 0 else False
#endregion

#region setters
    def __setitem__(self, key, value): 
        """Sets requested attribute to the given value."""
        if(key == 'layout'):
            self.num_rows, self.num_cols, self.num_sets = value
        else:
            setattr(self, key, value)
#endregion

#region utility functions
    def to_string(self):
        """Returns a string representing the object."""
        attributes_str = '\n'.join(
            f'{key}: {value}' for key, value in self.__dict__.items()
            if not key.startswith('__')
            and not callable(key)
        )
        self_str = f'AMK {self.name}\'s configuration is:\n{attributes_str}'
        return inspect.cleandoc(self_str)

    def to_dict(self):
        """Returns a dictionary representing the object."""
        attributes_dict = {
            key:value for key, value in self.__dict__.items()
            if not key.startswith('__') and not callable(key)
        } # [k for k, v in dict(self) if not k.startswith('_')]
        # print(attributes_dict)
        return attributes_dict

    def save_to_arduino(self):
        """Sends the callback function all data required to update the \
            AMK device"""
        print(f'Saving profiles to AMK: {self.name}')
        data = self.to_dict()
        self.arduino_callback(data, self)

#endregion


class Keycodes: #consumer keys are not handled yet

#region keycodes
    dict_keycodes = {
        'KEY_RESERVED'        : 0x00,
        'KEY_ERROR_ROLLOVER'  : 0x01,
        'KEY_POST_FAIL'       : 0x02,
        'KEY_ERROR_UNDEFINED' : 0x03,
        'KEY_A'               : 0x04,
        'KEY_B'               : 0x05,
        'KEY_C'               : 0x06,
        'KEY_D'               : 0x07,
        'KEY_E'               : 0x08,
        'KEY_F'               : 0x09,
        'KEY_G'               : 0x0A,
        'KEY_H'               : 0x0B,
        'KEY_I'               : 0x0C,
        'KEY_J'               : 0x0D,
        'KEY_K'               : 0x0E,
        'KEY_L'               : 0x0F,
        'KEY_M'               : 0x10,
        'KEY_N'               : 0x11,
        'KEY_O'               : 0x12,
        'KEY_P'               : 0x13,
        'KEY_Q'               : 0x14,
        'KEY_R'               : 0x15,
        'KEY_S'               : 0x16,
        'KEY_T'               : 0x17,
        'KEY_U'               : 0x18,
        'KEY_V'               : 0x19,
        'KEY_W'               : 0x1A,
        'KEY_X'               : 0x1B,
        'KEY_Y'               : 0x1C,
        'KEY_Z'               : 0x1D,
        'KEY_1'               : 0x1E,
        'KEY_2'               : 0x1F,
        'KEY_3'               : 0x20,
        'KEY_4'               : 0x21,
        'KEY_5'               : 0x22,
        'KEY_6'               : 0x23,
        'KEY_7'               : 0x24,
        'KEY_8'               : 0x25,
        'KEY_9'               : 0x26,
        'KEY_0'               : 0x27,
        'KEY_ENTER'           : 0x28,
        'KEY_RETURN'          : 0x28, # Alias
        'KEY_ESC'             : 0x29,
        'KEY_BACKSPACE'       : 0x2A,
        'KEY_TAB'             : 0x2B,
        'KEY_SPACE'           : 0x2C,
        'KEY_MINUS'           : 0x2D,
        'KEY_EQUAL'           : 0x2E,
        'KEY_LEFT_BRACE'      : 0x2F,
        'KEY_RIGHT_BRACE'     : 0x30,
        'KEY_BACKSLASH'       : 0x31,
        'KEY_NON_US_NUM'      : 0x32,
        'KEY_SEMICOLON'       : 0x33,
        'KEY_QUOTE'           : 0x34,
        'KEY_TILDE'           : 0x35,
        'KEY_COMMA'           : 0x36,
        'KEY_PERIOD'          : 0x37,
        'KEY_SLASH'           : 0x39,
        'KEY_CAPS_LOCK'       : 0x39,
        'KEY_F1'              : 0x3A,
        'KEY_F2'              : 0x3B,
        'KEY_F3'              : 0x3C,
        'KEY_F4'              : 0x3D,
        'KEY_F5'              : 0x3E,
        'KEY_F6'              : 0x3F,
        'KEY_F7'              : 0x40,
        'KEY_F8'              : 0x41,
        'KEY_F9'              : 0x42,
        'KEY_F10'             : 0x43,
        'KEY_F11'             : 0x44,
        'KEY_F12'             : 0x45,
        'KEY_PRINT'           : 0x46,
        'KEY_PRINTSCREEN'     : 0x46, # Alias
        'KEY_SCROLL_LOCK'     : 0x47,
        'KEY_PAUSE'           : 0x48,
        'KEY_INSERT'          : 0x49,
        'KEY_HOME'            : 0x4A,
        'KEY_PAGE_UP'         : 0x4B,
        'KEY_DELETE'          : 0x4C,
        'KEY_END'             : 0x4D,
        'KEY_PAGE_DOWN'       : 0x4E,
        'KEY_RIGHT_ARROW'     : 0x4F,
        'KEY_LEFT_ARROW'      : 0x50,
        'KEY_DOWN_ARROW'      : 0x51,
        'KEY_UP_ARROW'        : 0x52,
        'KEY_RIGHT'           : 0x4F, # Alias
        'KEY_LEFT'            : 0x50, # Alias
        'KEY_DOWN'            : 0x51, # Alias
        'KEY_UP'              : 0x52, # Alias
        'KEY_NUM_LOCK'        : 0x53,
        'KEYPAD_DIVIDE'       : 0x54,
        'KEYPAD_MULTIPLY'     : 0x55,
        'KEYPAD_SUBTRACT'     : 0x56,
        'KEYPAD_ADD'          : 0x57,
        'KEYPAD_ENTER'        : 0x58,
        'KEYPAD_1'            : 0x59,
        'KEYPAD_2'            : 0x5A,
        'KEYPAD_3'            : 0x5B,
        'KEYPAD_4'            : 0x5C,
        'KEYPAD_5'            : 0x5D,
        'KEYPAD_6'            : 0x5E,
        'KEYPAD_7'            : 0x5F,
        'KEYPAD_8'            : 0x60,
        'KEYPAD_9'            : 0x61,
        'KEYPAD_0'            : 0x62,
        'KEYPAD_DOT'          : 0x63,
        'KEY_NON_US'          : 0x64,
        'KEY_APPLICATION'     : 0x65, # Context menu/right click
        'KEY_MENU'            : 0x65, # ''Alias''

        # Most of the following keys will only work with Linux or not at all.
        # F13+ keys are mostly used for laptop functions like ECO key.
        'KEY_POWER'           : 0x66, # PowerOff (Ubuntu)
        'KEY_PAD_EQUALS'      : 0x67, # Dont confuse with KEYPAD_EQUAL_SIGN
        'KEY_F13'             : 0x68, # Tools (Ubunutu)
        'KEY_F14'             : 0x69, # Launch5 (Ubuntu)
        'KEY_F15'             : 0x6A, # Launch6 (Ubuntu)
        'KEY_F16'             : 0x6B, # Launch7 (Ubuntu)
        'KEY_F17'             : 0x6C, # Launch8 (Ubuntu)
        'KEY_F18'             : 0x6D, # Launch9 (Ubuntu)
        'KEY_F19'             : 0x6E, # Disabled (Ubuntu)
        'KEY_F20'             : 0x6F, # AudioMicMute (Ubuntu)
        'KEY_F21'             : 0x70, # Touchpad toggle (Ubuntu)
        'KEY_F22'             : 0x71, # TouchpadOn (Ubuntu)
        'KEY_F23'             : 0x72, # TouchpadOff Ubuntu)
        'KEY_F24'             : 0x73, # Disabled (Ubuntu)
        'KEY_EXECUTE'         : 0x74, # Open (Ubuntu)
        'KEY_HELP'            : 0x75, # Help (Ubuntu)
        'KEY_MENU2'           : 0x76, # Disabled (Ubuntu)
        'KEY_SELECT'          : 0x77, # Disabled (Ubuntu)
        'KEY_STOP'            : 0x78, # Cancel (Ubuntu)
        'KEY_AGAIN'           : 0x79, # Redo (Ubuntu)
        'KEY_UNDO'            : 0x7A, # Undo (Ubuntu)
        'KEY_CUT'             : 0x7B, # Cut (Ubuntu)
        'KEY_COPY'            : 0x7C, # Copy (Ubuntu)
        'KEY_PASTE'           : 0x7D, # Paste (Ubuntu)
        'KEY_FIND'            : 0x7E, # Find (Ubuntu)
        'KEY_MUTE'            : 0x7F,
        'KEY_VOLUME_MUTE'     : 0x7F, # Alias
        'KEY_VOLUME_UP'       : 0x80,
        'KEY_VOLUME_DOWN'     : 0x81,
        'KEY_LOCKING_CAPS_LOCK'   : 0x82, # Disabled (Ubuntu)
        'KEY_LOCKING_NUM_LOCK'    : 0x83, # Disabled (Ubuntu)
        'KEY_LOCKING_SCROLL_LOCK' : 0x84, # Disabled (Ubuntu)
        'KEYPAD_COMMA'            : 0x85, # .
        'KEYPAD_EQUAL_SIGN'       : 0x86, # Disabled (Ubuntu), Dont confuse with KEYPAD_EQUAL
        'KEY_INTERNATIONAL1'      : 0x87, # Disabled (Ubuntu)
        'KEY_INTERNATIONAL2'      : 0x88, # Hiragana Katakana (Ubuntu)
        'KEY_INTERNATIONAL3'      : 0x89, # Disabled (Ubuntu)
        'KEY_INTERNATIONAL4'      : 0x8A, # Henkan (Ubuntu)
        'KEY_INTERNATIONAL5'      : 0x8B, # Muhenkan (Ubuntu)
        'KEY_INTERNATIONAL6'      : 0x8C, # Disabled (Ubuntu)
        'KEY_INTERNATIONAL7'      : 0x8D, # Disabled (Ubuntu)
        'KEY_INTERNATIONAL8'      : 0x8E, # Disabled (Ubuntu)
        'KEY_INTERNATIONAL9'      : 0x8F, # Disabled (Ubuntu)
        'KEY_LANG1'               : 0x90, # Disabled (Ubuntu)
        'KEY_LANG2'               : 0x91, # Disabled (Ubuntu)
        'KEY_LANG3'               : 0x92, # Katana (Ubuntu)
        'KEY_LANG4'               : 0x93, # Hiragana (Ubuntu)
        'KEY_LANG5'               : 0x94, # Disabled (Ubuntu)
        'KEY_LANG6'               : 0x95, # Disabled (Ubuntu)
        'KEY_LANG7'               : 0x96, # Disabled (Ubuntu)
        'KEY_LANG8'               : 0x97, # Disabled (Ubuntu)
        'KEY_LANG9'               : 0x98, # Disabled (Ubuntu)
        'KEY_ALTERNATE_ERASE'     : 0x99, # Disabled (Ubuntu)
        'KEY_SYSREQ_ATTENTION'    : 0x9A, # Disabled (Ubuntu)
        'KEY_CANCEL'              : 0x9B, # Disabled (Ubuntu)
        'KEY_CLEAR'               : 0x9C, # Delete (Ubuntu)
        'KEY_PRIOR'               : 0x9D, # Disabled (Ubuntu)
        'KEY_RETURN2'             : 0x9E, # Disabled (Ubuntu), Do not confuse this with KEY_ENTER
        'KEY_SEPARATOR'           : 0x9F, # Disabled (Ubuntu)
        'KEY_OUT'                 : 0xA0, # Disabled (Ubuntu)
        'KEY_OPER'                : 0xA1, # Disabled (Ubuntu)
        'KEY_CLEAR_AGAIN'         : 0xA2, # Disabled (Ubuntu)
        'KEY_CRSEL_PROPS'         : 0xA3, # Disabled (Ubuntu)
        'KEY_EXSEL'               : 0xA4, # Disabled (Ubuntu)

        'KEY_PAD_00'                  : 0xB0, # Disabled (Ubuntu)
        'KEY_PAD_000'                 : 0xB1, # Disabled (Ubuntu)
        'KEY_THOUSANDS_SEPARATOR'     : 0xB2, # Disabled (Ubuntu)
        'KEY_DECIMAL_SEPARATOR'       : 0xB3, # Disabled (Ubuntu)
        'KEY_CURRENCY_UNIT'           : 0xB4, # Disabled (Ubuntu)
        'KEY_CURRENCY_SUB_UNIT'       : 0xB5, # Disabled (Ubuntu)
        'KEYPAD_LEFT_BRACE'           : 0xB6, # (
        'KEYPAD_RIGHT_BRACE'          : 0xB7, # )
        'KEYPAD_LEFT_CURLY_BRACE'     : 0xB8, # Disabled (Ubuntu)
        'KEYPAD_RIGHT_CURLY_BRACE'    : 0xB9, # Disabled (Ubuntu)
        'KEYPAD_TAB'                  : 0xBA, # Disabled (Ubuntu)
        'KEYPAD_BACKSPACE'            : 0xBB, # Disabled (Ubuntu)
        'KEYPAD_A'                    : 0xBC, # Disabled (Ubuntu)
        'KEYPAD_B'                    : 0xBD, # Disabled (Ubuntu)
        'KEYPAD_C'                    : 0xBE, # Disabled (Ubuntu)
        'KEYPAD_D'                    : 0xBF, # Disabled (Ubuntu)
        'KEYPAD_E'                    : 0xC0, # Disabled (Ubuntu)
        'KEYPAD_F'                    : 0xC1, # Disabled (Ubuntu)
        'KEYPAD_XOR'                  : 0xC2, # Disabled (Ubuntu)
        'KEYPAD_CARET'                : 0xC3, # Disabled (Ubuntu)
        'KEYPAD_PERCENT'              : 0xC4, # Disabled (Ubuntu)
        'KEYPAD_LESS_THAN'            : 0xC5, # Disabled (Ubuntu)
        'KEYPAD_GREATER_THAN'         : 0xC6, # Disabled (Ubuntu)
        'KEYPAD_AMPERSAND'            : 0xC7, # Disabled (Ubuntu)
        'KEYPAD_DOUBLEAMPERSAND'      : 0xC8, # Disabled (Ubuntu)
        'KEYPAD_PIPE'                 : 0xC9, # Disabled (Ubuntu)
        'KEYPAD_DOUBLEPIPE'           : 0xCA, # Disabled (Ubuntu)
        'KEYPAD_COLON'                : 0xCB, # Disabled (Ubuntu)
        'KEYPAD_POUND_SIGN'           : 0xCC, # Disabled (Ubuntu)
        'KEYPAD_SPACE'                : 0xCD, # Disabled (Ubuntu)
        'KEYPAD_AT_SIGN'              : 0xCE, # Disabled (Ubuntu)
        'KEYPAD_EXCLAMATION_POINT'    : 0xCF, # Disabled (Ubuntu)
        'KEYPAD_MEMORY_STORE'         : 0xD0, # Disabled (Ubuntu)
        'KEYPAD_MEMORY_RECALL'        : 0xD1, # Disabled (Ubuntu)
        'KEYPAD_MEMORY_CLEAR'         : 0xD2, # Disabled (Ubuntu)
        'KEYPAD_MEMORY_ADD'           : 0xD3, # Disabled (Ubuntu)
        'KEYPAD_MEMORY_SUBTRACT'      : 0xD4, # Disabled (Ubuntu)
        'KEYPAD_MEMORY_MULTIPLY'      : 0xD5, # Disabled (Ubuntu)
        'KEYPAD_MEMORY_DIVIDE'        : 0xD6, # Disabled (Ubuntu)
        'KEYPAD_PLUS_MINUS'           : 0xD7, # Disabled (Ubuntu)
        'KEYPAD_CLEAR'                : 0xD8, # Delete (Ubuntu)
        'KEYPAD_CLEAR_ENTRY'          : 0xD9, # Disabled (Ubuntu)
        'KEYPAD_BINARY'               : 0xDA, # Disabled (Ubuntu)
        'KEYPAD_OCTAL'                : 0xDB, # Disabled (Ubuntu)
        'KEYPAD_DECIMAL'              : 0xDC, # Disabled (Ubuntu)
        'KEYPAD_HEXADECIMAL'          : 0xDD, # Disabled (Ubuntu)

        'KEY_LEFT_CTRL'           : 0xE0,
        'KEY_LEFT_SHIFT'          : 0xE1,
        'KEY_LEFT_ALT'            : 0xE2,
        'KEY_LEFT_GUI'            : 0xE3,
        'KEY_LEFT_WINDOWS'        : 0xE3, # Alias
        'KEY_RIGHT_CTRL'          : 0xE4,
        'KEY_RIGHT_SHIFT'         : 0xE5,
        'KEY_RIGHT_ALT'           : 0xE6,
        'KEY_RIGHT_GUI'           : 0xE7,
        'KEY_RIGHT_WINDOWS'       : 0xE7, # Alias
    }

    consumer_keycode = {
        # Some keys might only work with linux
        'CONSUMER_POWER'	: 0x30,
        'CONSUMER_SLEEP' : 0x32,
        #
        'MEDIA_RECORD' : 0xB2,
        'MEDIA_FAST_FORWARD'	: 0xB3,
        'MEDIA_REWIND'	: 0xB4,
        'MEDIA_NEXT'	: 0xB5,
        'MEDIA_PREVIOUS'	: 0xB6,
        'MEDIA_PREV'	: 0xB6, # Alias
        'MEDIA_STOP'	: 0xB7,
        'MEDIA_PLAY_PAUSE'	: 0xCD,
        'MEDIA_PAUSE'	: 0xB0,
        #
        'MEDIA_VOLUME_MUTE'	: 0xE2,
        'MEDIA_VOL_MUTE' : 0xE2, # Alias
        'MEDIA_VOLUME_UP'	: 0xE9,
        'MEDIA_VOL_UP'	: 0xE9, # Alias
        'MEDIA_VOLUME_DOWN'	: 0xEA,
        'MEDIA_VOL_DOWN'	: 0xEA, # Alias
        #
        'CONSUMER_BRIGHTNESS_UP' : 0x006F,
        'CONSUMER_BRIGHTNESS_DOWN' : 0x0070,
        #
        'CONSUMER_SCREENSAVER' : 0x19e,
        #
        'CONSUMER_PROGRAMMABLE_BUTTON_CONFIGURATION' : 0x182,
        'CONSUMER_CONTROL_CONFIGURATION' : 0x183,
        'CONSUMER_EMAIL_READER'	: 0x18A,
        'CONSUMER_CALCULATOR'	: 0x192,
        'CONSUMER_EXPLORER'	: 0x194,
        #
        'CONSUMER_BROWSER_HOME'	: 0x223,
        'CONSUMER_BROWSER_BACK'	: 0x224,
        'CONSUMER_BROWSER_FORWARD'	: 0x225,
        'CONSUMER_BROWSER_REFRESH'	: 0x227,
        'CONSUMER_BROWSER_BOOKMARKS'	: 0x22A,
        #
        #
        # Consumer_Page_(0x0C)	0x15
        'HID_CONSUMER_UNASSIGNED'		: 0x00,
        'HID_CONSUMER_NUMERIC_KEY_PAD'	: 0x02,	# HID type NARY
        'HID_CONSUMER_PROGRAMMABLE_BUTTONS'	: 0x03,	# HID type NARY
        'HID_CONSUMER_MICROPHONE_CA'	: 0x04,
        'HID_CONSUMER_HEADPHONE_CA'	: 0x05,
        'HID_CONSUMER_GRAPHIC_EQUALIZER_CA'	: 0x06,
        # Reserved	: 0x07-1F
        'HID_CONSUMER_PLUS_10'	: 0x20,	# HID type OSC
        'HID_CONSUMER_PLUS_100'	: 0x21,	# HID type OSC
        'HID_CONSUMER_AM_SLASH_PM'	: 0x22,	# HID type OSC
        # Reserved	: 0x23-3F
        'HID_CONSUMER_POWER'	: 0x30,	# HID type OOC
        'HID_CONSUMER_RESET'	: 0x31,	# HID type OSC
        'HID_CONSUMER_SLEEP'	: 0x32,	# HID type OSC
        'HID_CONSUMER_SLEEP_AFTER'	: 0x33,	# HID type OSC
        'HID_CONSUMER_SLEEP_MODE'	: 0x34,	# HID type RTC
        'HID_CONSUMER_ILLUMINATION'	: 0x35,	# HID type OOC
        'HID_CONSUMER_FUNCTION_BUTTONS'	: 0x36,	# HID type NARY
        # Reserved	: 0x37-3F
        'HID_CONSUMER_MENU'	: 0x40,	# HID type OOC
        'HID_CONSUMER_MENU_PICK'	: 0x41,	# HID type OSC
        'HID_CONSUMER_MENU_UP'	: 0x42,	# HID type OSC
        'HID_CONSUMER_MENU_DOWN'	: 0x43,	# HID type OSC
        'HID_CONSUMER_MENU_LEFT'	: 0x44,	# HID type OSC
        'HID_CONSUMER_MENU_RIGHT'	: 0x45,	# HID type OSC
        'HID_CONSUMER_MENU_ESCAPE'	: 0x46,	# HID type OSC
        'HID_CONSUMER_MENU_VALUE_INCREASE'	: 0x47,	# HID type OSC
        'HID_CONSUMER_MENU_VALUE_DECREASE'	: 0x48,	# HID type OSC
        # Reserved 0x49-5F
        'HID_CONSUMER_DATA_ON_SCREEN'	: 0x60,	# HID type OOC
        'HID_CONSUMER_CLOSED_CAPTION'	: 0x61,	# HID type OOC
        'HID_CONSUMER_CLOSED_CAPTION_SELECT'	: 0x62,	# HID type OSC
        'HID_CONSUMER_VCR_SLASH_TV'	: 0x63,	# HID type OOC
        'HID_CONSUMER_BROADCAST_MODE'	: 0x64,	# HID type OSC
        'HID_CONSUMER_SNAPSHOT'	: 0x65,	# HID type OSC
        'HID_CONSUMER_STILL'	: 0x66,	# HID type OSC
        # Reserved 0x67-7F
        'HID_CONSUMER_SELECTION'	: 0x80,	# HID type NARY
        'HID_CONSUMER_ASSIGN_SELECTION'	: 0x81,	# HID type OSC
        'HID_CONSUMER_MODE_STEP'	: 0x82,	# HID type OSC
        'HID_CONSUMER_RECALL_LAST'	: 0x83,	# HID type OSC
        'HID_CONSUMER_ENTER_CHANNEL'	: 0x84,	# HID type OSC
        'HID_CONSUMER_ORDER_MOVIE'	: 0x85,	# HID type OSC
        'HID_CONSUMER_CHANNEL'	: 0x86,	# HID type LC
        'HID_CONSUMER_MEDIA_SELECTION'	: 0x87,	# HID type NARY
        'HID_CONSUMER_MEDIA_SELECT_COMPUTER'	: 0x88,	# HID type SEL
        'HID_CONSUMER_MEDIA_SELECT_TV'	: 0x89,	# HID type SEL
        'HID_CONSUMER_MEDIA_SELECT_WWW'	: 0x8A,	# HID type SEL
        'HID_CONSUMER_MEDIA_SELECT_DVD'	: 0x8B,	# HID type SEL
        'HID_CONSUMER_MEDIA_SELECT_TELEPHONE'	: 0x8C,	# HID type SEL
        'HID_CONSUMER_MEDIA_SELECT_PROGRAM_GUIDE'	: 0x8D,	# HID type SEL
        'HID_CONSUMER_MEDIA_SELECT_VIDEO_PHONE'	: 0x8E,	# HID type SEL
        'HID_CONSUMER_MEDIA_SELECT_GAMES'	: 0x8F,	# HID type SEL
        'HID_CONSUMER_MEDIA_SELECT_MESSAGES'	: 0x90,	# HID type SEL
        'HID_CONSUMER_MEDIA_SELECT_CD'	: 0x91,	# HID type SEL
        'HID_CONSUMER_MEDIA_SELECT_VCR'	: 0x92,	# HID type SEL
        'HID_CONSUMER_MEDIA_SELECT_TUNER'	: 0x93,	# HID type SEL
        'HID_CONSUMER_QUIT'	: 0x94,	# HID type OSC
        'HID_CONSUMER_HELP'	: 0x95,	# HID type OOC
        'HID_CONSUMER_MEDIA_SELECT_TAPE'	: 0x96,	# HID type SEL
        'HID_CONSUMER_MEDIA_SELECT_CABLE'	: 0x97,	# HID type SEL
        'HID_CONSUMER_MEDIA_SELECT_SATELLITE'	: 0x98,	# HID type SEL
        'HID_CONSUMER_MEDIA_SELECT_SECURITY'	: 0x99,	# HID type SEL
        'HID_CONSUMER_MEDIA_SELECT_HOME'	: 0x9A,	# HID type SEL
        'HID_CONSUMER_MEDIA_SELECT_CALL'	: 0x9B,	# HID type SEL
        'HID_CONSUMER_CHANNEL_INCREMENT'	: 0x9C,	# HID type OSC
        'HID_CONSUMER_CHANNEL_DECREMENT'	: 0x9D,	# HID type OSC
        'HID_CONSUMER_MEDIA_SELECT_SAP'	: 0x9E,	# HID type SEL
        # Reserved 0x9F
        'HID_CONSUMER_VCR_PLUS'	: 0xA0,	# HID type OSC
        'HID_CONSUMER_ONCE'	: 0xA1,	# HID type OSC
        'HID_CONSUMER_DAILY'	: 0xA2,	# HID type OSC
        'HID_CONSUMER_WEEKLY'	: 0xA3,	# HID type OSC
        'HID_CONSUMER_MONTHLY'	: 0xA4,	# HID type OSC
        # Reserved 0xA5-AF
        'HID_CONSUMER_PLAY'	: 0xB0,	# HID type OOC
        'HID_CONSUMER_PAUSE'	: 0xB1,	# HID type OOC
        'HID_CONSUMER_RECORD'	: 0xB2,	# HID type OOC
        'HID_CONSUMER_FAST_FORWARD'	: 0xB3,	# HID type OOC
        'HID_CONSUMER_REWIND'	: 0xB4,	# HID type OOC
        'HID_CONSUMER_SCAN_NEXT_TRACK'	: 0xB5,	# HID type OSC
        'HID_CONSUMER_SCAN_PREVIOUS_TRACK'	: 0xB6,	# HID type OSC
        'HID_CONSUMER_STOP'	: 0xB7,	# HID type OSC
        'HID_CONSUMER_EJECT'	: 0xB8,	# HID type OSC
        'HID_CONSUMER_RANDOM_PLAY'	: 0xB9,	# HID type OOC
        'HID_CONSUMER_SELECT_DISC'	: 0xBA,	# HID type NARY
        'HID_CONSUMER_ENTER_DISC_MC'	: 0xBB,
        'HID_CONSUMER_REPEAT'	: 0xBC,	# HID type OSC
        'HID_CONSUMER_TRACKING'	: 0xBD,	# HID type LC
        'HID_CONSUMER_TRACK_NORMAL'	: 0xBE,	# HID type OSC
        'HID_CONSUMER_SLOW_TRACKING'	: 0xBF,	# HID type LC
        'HID_CONSUMER_FRAME_FORWARD'	: 0xC0,	# HID type RTC
        'HID_CONSUMER_FRAME_BACK'	: 0xC1,	# HID type RTC
        'HID_CONSUMER_MARK'	: 0xC2,	# HID type OSC
        'HID_CONSUMER_CLEAR_MARK'	: 0xC3,	# HID type OSC
        'HID_CONSUMER_REPEAT_FROM_MARK'	: 0xC4,	# HID type OOC
        'HID_CONSUMER_RETURN_TO_MARK'	: 0xC5,	# HID type OSC
        'HID_CONSUMER_SEARCH_MARK_FORWARD'	: 0xC6,	# HID type OSC
        'HID_CONSUMER_SEARCH_MARK_BACKWARDS'	: 0xC7,	# HID type OSC
        'HID_CONSUMER_COUNTER_RESET'	: 0xC8,	# HID type OSC
        'HID_CONSUMER_SHOW_COUNTER'	: 0xC9,	# HID type OSC
        'HID_CONSUMER_TRACKING_INCREMENT'	: 0xCA,	# HID type RTC
        'HID_CONSUMER_TRACKING_DECREMENT'	: 0xCB,	# HID type RTC
        'HID_CONSUMER_STOP_SLASH_EJECT'	: 0xCC,	# HID type OSC
        'HID_CONSUMER_PLAY_SLASH_PAUSE'	: 0xCD,	# HID type OSC
        'HID_CONSUMER_PLAY_SLASH_SKIP'	: 0xCE,	# HID type OSC
        # Reserved 0xCF-DF
        'HID_CONSUMER_VOLUME'	: 0xE0,	# HID type LC
        'HID_CONSUMER_BALANCE'	: 0xE1,	# HID type LC
        'HID_CONSUMER_MUTE'	: 0xE2,	# HID type OOC
        'HID_CONSUMER_BASS'	: 0xE3,	# HID type LC
        'HID_CONSUMER_TREBLE'	: 0xE4,	# HID type LC
        'HID_CONSUMER_BASS_BOOST'	: 0xE5,	# HID type OOC
        'HID_CONSUMER_SURROUND_MODE'	: 0xE6,	# HID type OSC
        'HID_CONSUMER_LOUDNESS'	: 0xE7,	# HID type OOC
        'HID_CONSUMER_MPX'	: 0xE8,	# HID type OOC
        'HID_CONSUMER_VOLUME_INCREMENT'	: 0xE9,	# HID type RTC
        'HID_CONSUMER_VOLUME_DECREMENT'	: 0xEA,	# HID type RTC
        # Reserved 0xEB-EF
        'HID_CONSUMER_SPEED_SELECT'	: 0xF0,	# HID type OSC
        'HID_CONSUMER_PLAYBACK_SPEED'	: 0xF1,	# HID type NARY
        'HID_CONSUMER_STANDARD_PLAY'	: 0xF2,	# HID type SEL
        'HID_CONSUMER_LONG_PLAY'	: 0xF3,	# HID type SEL
        'HID_CONSUMER_EXTENDED_PLAY'	: 0xF4,	# HID type SEL
        'HID_CONSUMER_SLOW'	: 0xF5,	# HID type OSC
    }
#endregion

    @classmethod
    def get_name_of(cls, code):
        """Returns the name of the key whose code was given."""
        for k, v in cls.dict_keycodes.items():
            if (v == code):
                return k

        return 'unidentified key'

    # @classmethod
    # def get_code_of(cls, name) -> str:
    #     return cls.dict_keycodes[name]

    @classmethod
    def get_code_of(cls, name) -> str: #without the prefix
        """Returns the code of the key whose name was given."""
        for key, value in cls.dict_keycodes.items():
            if (name == key.split("_",1)[1]):
                return value
        return None

    @classmethod
    def get_key_names(cls) -> list:
        """Returns the names of all keys supported by AMK."""
        names_list = []
        for name in cls.dict_keycodes.keys():
            names_list.append(name.split("_",1)[1])
        return names_list

if __name__ == "__main__":

    # AMK
    amk_1 = AMK(name='amk_1')
    example_keypad_dict = {
        'num_rows': 3,
        'num_cols': 3,
        'num_sets': 2,
        'bindings': [[104, 105, 106, 107, 108, 109, 110, 111, 112],
                     [30, 20, 9, 7, 33, 26, 5, 8, 21]],
        'orientation': 3,
        'active_set': 0,
    }
    amk_2 = AMK.init_from_dict(example_keypad_dict)
    amk_2.name = 'amk_2'
    print(amk_1.to_string())
    print(amk_2.to_string())
    print(amk_1.is_full())
    print(amk_2.is_full())
    
    # Keycodes
    # print(Keycodes.get_code_of('KEY_M'))
    # print(Keycodes.get_name_of(0x05))
    # print(Keycodes.get_key_names())