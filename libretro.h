/* Copyright (C) 2010-2020 The RetroArch team
 * --------------------------------------------------------------------------------------- 
 * 以下のライセンスステートメントは、このlibretro APIヘッダー（libretro.h）にのみ適用されます。
 * ---------------------------------------------------------------------------------------  
 * 本ソフトウェアおよび関連文書ファイル（以下「本ソフトウェア」といいます）のコピーを取得する者に対して、
 * 本ソフトウェアを無制限に取り扱うことを無償で許可します。
 * これには、本ソフトウェアのコピーを使用、コピー、変更、結合、公開、配布、サブライセンス、
 * および/または販売する権利、および本ソフトウェアを提供する相手にこれを許可する権利が無制限に含まれます。 
 * 上記の著作権表示および本許諾表示は、本ソフトウェアのすべてのコピーまたは実質的な部分に含まれるものとします。
 * 本ソフトウェアは、「現状有姿」で提供され、明示的または黙示的を問わず、商品性、特定目的への適合性、
 * および非侵害の保証を含むがこれに限定されない、いかなる種類の保証もありません。
 * いかなる場合においても、著者または著作権者は、本ソフトウェアまたは本ソフトウェアの使用または
 * その他の取引に起因する、または関連する、契約、不法行為またはその他の行為であるかを問わず、
 * いかなる請求、損害またはその他の責任も負いません。
 */

#ifndef LIBRETRO_H__
#define LIBRETRO_H__

#include <stdint.h>
#include <stddef.h>
#include <limits.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __cplusplus
#if defined(_MSC_VER) && _MSC_VER < 1800 && !defined(SN_TARGET_PS3)
/* C89モードでコンパイルする際にMSVCに適用されるハック
 * C99に準拠していないため、MSVCに適用されるハックです。 */
#define bool unsigned char
#define true 1
#define false 0
#else
#include <stdbool.h>
#endif
#endif

#ifndef RETRO_CALLCONV
#  if defined(__GNUC__) && defined(__i386__) && !defined(__x86_64__)
#    define RETRO_CALLCONV __attribute__((cdecl))
#  elif defined(_MSC_VER) && defined(_M_X86) && !defined(_M_X64)
#    define RETRO_CALLCONV __cdecl
#  else
#    define RETRO_CALLCONV /* 他のすべてのプラットフォームでは、それぞれ1つの呼び出し規則しかありません。 */
#  endif
#endif

#ifndef RETRO_API
#  if defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
#    ifdef RETRO_IMPORT_SYMBOLS
#      ifdef __GNUC__
#        define RETRO_API RETRO_CALLCONV __attribute__((__dllimport__))
#      else
#        define RETRO_API RETRO_CALLCONV __declspec(dllimport)
#      endif
#    else
#      ifdef __GNUC__
#        define RETRO_API RETRO_CALLCONV __attribute__((__dllexport__))
#      else
#        define RETRO_API RETRO_CALLCONV __declspec(dllexport)
#      endif
#    endif
#  else
#      if defined(__GNUC__) && __GNUC__ >= 4
#        define RETRO_API RETRO_CALLCONV __attribute__((__visibility__("default")))
#      else
#        define RETRO_API RETRO_CALLCONV
#      endif
#  endif
#endif

/* libretroの実装を壊す可能性のあるAPI/ABIの不一致をチェックするために使用されます。
 * APIに互換性のある変更があってもインクリメントされません。
 */
#define RETRO_API_VERSION         1

/*
 * Libretroの基本的なデバイスの抽象化。
 *
 * Libretroの入力システムは、ジョイパッド(アナログあり/なし)、マウス、キーボード、
 * ライトガン、ポインターなど、いくつかの標準化されたデバイスタイプで構成されています。
 *
 * これらのデバイスの機能は固定されており、
 * 各コアは独自のコントローラの概念をlibretroの抽象化にマッピングしています。
 * これにより、フロントエンドは抽象的なタイプを実際の入力デバイスにマッピングすることが可能になり、
 * 任意のコントローラレイアウトに入力を正しくバインドすることを心配する必要がなくなります。
 */

#define RETRO_DEVICE_TYPE_SHIFT         8
#define RETRO_DEVICE_MASK               ((1 << RETRO_DEVICE_TYPE_SHIFT) - 1)
#define RETRO_DEVICE_SUBCLASS(base, id) (((id + 1) << RETRO_DEVICE_TYPE_SHIFT) | base)

/* 入力無効 */
#define RETRO_DEVICE_NONE         0

/* JOYPADはRetroPadと呼ばれています。
 * 基本的にはスーパーファミコンのコントローラーですが、
 * PS1のDualShockのようにL2/R2/L3/R3ボタンが追加されています。*/
#define RETRO_DEVICE_JOYPAD       1

/* マウスは、スーパーファミコンのマウスに似たシンプルなマウスです。
 * XとYの座標は、最後のポーリング(pollコールバック)に相対的に報告されます。
 * マウスポインタが画面上のどこにあるかを追跡するのはlibretroの実装に任されています。
 * フロントエンドは、自身のハードウェアマウスポインタと干渉しないようにしなければなりません。
 */
#define RETRO_DEVICE_MOUSE        2

/* KEYBOARDデバイスは、押された生のキーをポーリングします。
 * これはポーリングベースなので、入力コールバックは現在の押された状態を返します。
 * イベント/テキストベースのキーボード入力については
 * RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK を参照してください。
 */
#define RETRO_DEVICE_KEYBOARD     3

/* LIGHTGUNデバイスは、PlayStation 2のGuncon-2に似ています。
 * LIGHTGUNデバイスは、スクリーン空間におけるX/Y座標（ポインタに似ている）を、
 * 両軸とも[-0x8000, 0x7fff]の範囲で報告します。
 * また、画面のオン/オフ状態も報告します。
 * トリガー、スタート／セレクトボタン、補助アクションボタン、方向パッドを搭載しています。
 * 一部のゲームではオートリロード機能のために強制的なオフスクリーンショットを要求することができる。
 */
#define RETRO_DEVICE_LIGHTGUN     4

/* ANALOGデバイスは、JOYPAD（RetroPad）の拡張デバイスです。
 * DualShock2と同様に、2つのアナログスティックを追加し、すべてのボタンをアナログにすることができます。
 * これは、軸の値をアナログの全範囲である[-0x7fff, 0x7fff]で返すため、
 * 別のデバイスタイプとして扱われますが、デバイスによっては-0x8000を返す場合もあります。
 * 正のX軸は右。正のY軸は下になります。
 * ボタンは、[0, 0x7fff]の範囲で返されます。
 * アナログ値をポーリングする場合は、ANALOGタイプのみを使用してください。
 */
#define RETRO_DEVICE_ANALOG       5

/* タッチなどのポインティングメカニズムの概念を抽象化します。
 * これにより、libretroは画面上のどこにマウス(または同様のもの)が置かれているかを
 * 絶対座標で問い合わせることができます。
 * タッチ中心のデバイスの場合、報告される座標は押したときの座標です。
 *
 * XとYの座標は次のように報告されます。0x7fffは画面の左端/上端に、0x7fffは画面の右端/下端に相当する。
 * ここでいう「画面」とは、フロントエンドに渡され、後にモニターに表示される領域を指す。
 * 
 * フロントエンドはこの画面を自由に拡大・縮小することができますが、
 * (X, Y) = (-0x7fff, -0x7fff)はゲーム画像の左上のピクセルなどに対応します。
 *
 * ポインタの座標が有効であるかどうか（例えば、タッチディスプレイが実際にタッチされているかどうか）
 * を確認するために、PRESSEDは1または0を返します。
 *
 * デスクトップでマウスを使用している場合、
 * PRESSED は通常マウスの左ボタンに対応しますが、これはフロントエンドの判断です。
 * PRESSEDは、ポインターがゲーム画面内にある場合にのみ1を返します。
 *
 * マルチタッチの場合、index変数を使って連続してより多くのプレスを問い合わせることができます。
 * index = 0で_PRESSEDがtrueを返した場合、index = 0の_X, _Yで座標を抽出することができます。
 * 次にindex = 1で_PRESSED, _X, _Yを照会することができ、以下同様です。
 * 最終的には、_PRESSEDはインデックスに対してfalseを返します。
 * この時点では、それ以上の押下は登録されません。*/
#define RETRO_DEVICE_POINTER      6

/* レトロパッド（JOYPAD）用のボタンです。
 * スーパーファミコンのコントローラーと同じ配置になっています。
 * L2/R2/L3/R3ボタンは、PS1のDUALSHOCKに対応しています。
 * RETRO_DEVICE_INDEX_ANALOG_BUTTONのID値としても使用されています。 */
#define RETRO_DEVICE_ID_JOYPAD_B        0
#define RETRO_DEVICE_ID_JOYPAD_Y        1
#define RETRO_DEVICE_ID_JOYPAD_SELECT   2
#define RETRO_DEVICE_ID_JOYPAD_START    3
#define RETRO_DEVICE_ID_JOYPAD_UP       4
#define RETRO_DEVICE_ID_JOYPAD_DOWN     5
#define RETRO_DEVICE_ID_JOYPAD_LEFT     6
#define RETRO_DEVICE_ID_JOYPAD_RIGHT    7
#define RETRO_DEVICE_ID_JOYPAD_A        8
#define RETRO_DEVICE_ID_JOYPAD_X        9
#define RETRO_DEVICE_ID_JOYPAD_L       10
#define RETRO_DEVICE_ID_JOYPAD_R       11
#define RETRO_DEVICE_ID_JOYPAD_L2      12
#define RETRO_DEVICE_ID_JOYPAD_R2      13
#define RETRO_DEVICE_ID_JOYPAD_L3      14
#define RETRO_DEVICE_ID_JOYPAD_R3      15

#define RETRO_DEVICE_ID_JOYPAD_MASK    256

/* ANALOGデバイスのIndex / Id値 */
#define RETRO_DEVICE_INDEX_ANALOG_LEFT       0
#define RETRO_DEVICE_INDEX_ANALOG_RIGHT      1
#define RETRO_DEVICE_INDEX_ANALOG_BUTTON     2
#define RETRO_DEVICE_ID_ANALOG_X             0
#define RETRO_DEVICE_ID_ANALOG_Y             1

/* MOUSEのID値 */
#define RETRO_DEVICE_ID_MOUSE_X                0
#define RETRO_DEVICE_ID_MOUSE_Y                1
#define RETRO_DEVICE_ID_MOUSE_LEFT             2
#define RETRO_DEVICE_ID_MOUSE_RIGHT            3
#define RETRO_DEVICE_ID_MOUSE_WHEELUP          4
#define RETRO_DEVICE_ID_MOUSE_WHEELDOWN        5
#define RETRO_DEVICE_ID_MOUSE_MIDDLE           6
#define RETRO_DEVICE_ID_MOUSE_HORIZ_WHEELUP    7
#define RETRO_DEVICE_ID_MOUSE_HORIZ_WHEELDOWN  8
#define RETRO_DEVICE_ID_MOUSE_BUTTON_4         9
#define RETRO_DEVICE_ID_MOUSE_BUTTON_5         10

/* LIGHTGUNのID値 */
#define RETRO_DEVICE_ID_LIGHTGUN_SCREEN_X        13 /*絶対位置*/
#define RETRO_DEVICE_ID_LIGHTGUN_SCREEN_Y        14 /*絶対*/
#define RETRO_DEVICE_ID_LIGHTGUN_IS_OFFSCREEN    15 /*ステータスチェック*/
#define RETRO_DEVICE_ID_LIGHTGUN_TRIGGER          2
#define RETRO_DEVICE_ID_LIGHTGUN_RELOAD          16 /*Forced off-screen shot*/
#define RETRO_DEVICE_ID_LIGHTGUN_AUX_A            3
#define RETRO_DEVICE_ID_LIGHTGUN_AUX_B            4
#define RETRO_DEVICE_ID_LIGHTGUN_START            6
#define RETRO_DEVICE_ID_LIGHTGUN_SELECT           7
#define RETRO_DEVICE_ID_LIGHTGUN_AUX_C            8
#define RETRO_DEVICE_ID_LIGHTGUN_DPAD_UP          9
#define RETRO_DEVICE_ID_LIGHTGUN_DPAD_DOWN       10
#define RETRO_DEVICE_ID_LIGHTGUN_DPAD_LEFT       11
#define RETRO_DEVICE_ID_LIGHTGUN_DPAD_RIGHT      12
/* 非推奨 */
#define RETRO_DEVICE_ID_LIGHTGUN_X                0 /*相対位置*/
#define RETRO_DEVICE_ID_LIGHTGUN_Y                1 /*相対*/
#define RETRO_DEVICE_ID_LIGHTGUN_CURSOR           3 /*Use Aux:A*/
#define RETRO_DEVICE_ID_LIGHTGUN_TURBO            4 /*Use Aux:B*/
#define RETRO_DEVICE_ID_LIGHTGUN_PAUSE            5 /*Use Start*/

/* POINTERのID値 */
#define RETRO_DEVICE_ID_POINTER_X         0
#define RETRO_DEVICE_ID_POINTER_Y         1
#define RETRO_DEVICE_ID_POINTER_PRESSED   2
#define RETRO_DEVICE_ID_POINTER_COUNT     3

/* Returned from retro_get_region(). */
#define RETRO_REGION_NTSC  0
#define RETRO_REGION_PAL   1

/* LANGUAGEのID値 */
enum retro_language
{
   RETRO_LANGUAGE_ENGLISH             = 0,
   RETRO_LANGUAGE_JAPANESE            = 1,
   RETRO_LANGUAGE_FRENCH              = 2,
   RETRO_LANGUAGE_SPANISH             = 3,
   RETRO_LANGUAGE_GERMAN              = 4,
   RETRO_LANGUAGE_ITALIAN             = 5,
   RETRO_LANGUAGE_DUTCH               = 6,
   RETRO_LANGUAGE_PORTUGUESE_BRAZIL   = 7,
   RETRO_LANGUAGE_PORTUGUESE_PORTUGAL = 8,
   RETRO_LANGUAGE_RUSSIAN             = 9,
   RETRO_LANGUAGE_KOREAN              = 10,
   RETRO_LANGUAGE_CHINESE_TRADITIONAL = 11,
   RETRO_LANGUAGE_CHINESE_SIMPLIFIED  = 12,
   RETRO_LANGUAGE_ESPERANTO           = 13,
   RETRO_LANGUAGE_POLISH              = 14,
   RETRO_LANGUAGE_VIETNAMESE          = 15,
   RETRO_LANGUAGE_ARABIC              = 16,
   RETRO_LANGUAGE_GREEK               = 17,
   RETRO_LANGUAGE_TURKISH             = 18,
   RETRO_LANGUAGE_SLOVAK              = 19,
   RETRO_LANGUAGE_PERSIAN             = 20,
   RETRO_LANGUAGE_HEBREW              = 21,
   RETRO_LANGUAGE_ASTURIAN            = 22,
   RETRO_LANGUAGE_FINNISH             = 23,
   RETRO_LANGUAGE_LAST,

   /* Ensure sizeof(enum) == sizeof(int) */
   RETRO_LANGUAGE_DUMMY          = INT_MAX
};

/* retro_get_memory_data/size()に渡されます。
 * メモリタイプが実装に当てはまらない場合は、NULL/0を返すことができます。
 */
#define RETRO_MEMORY_MASK        0xff

/* 通常のセーブRAM。
 * このRAMは通常、ゲームカートリッジに搭載されており、バッテリーでバックアップされています。
 * ゲームのセーブデータが単一のメモリバッファでは複雑すぎる場合は、
 * SAVE_DIRECTORY（できれば）またはSYSTEM_DIRECTORY環境コールバックを使用できます。*/
#define RETRO_MEMORY_SAVE_RAM    0

/* ゲームの中には、時間を管理するための時計が内蔵されているものがあります。
 * このメモリは通常、時間を把握するための数バイトのものです。
 */
#define RETRO_MEMORY_RTC         1

/* System RAMは、ゲームシステムのメインRAMをフロントエンドが覗くことができます。 */
#define RETRO_MEMORY_SYSTEM_RAM  2

/* Video RAMは、ゲームシステムのビデオRAM（VRAM）をフロントエンドから覗くことができます。 */
#define RETRO_MEMORY_VIDEO_RAM   3

/* RETRO_KEYBOARDをポーリングする際の入力状態コールバックでIDに使用される Keysyms*/
enum retro_key
{
   RETROK_UNKNOWN        = 0,
   RETROK_FIRST          = 0,
   RETROK_BACKSPACE      = 8,
   RETROK_TAB            = 9,
   RETROK_CLEAR          = 12,
   RETROK_RETURN         = 13,
   RETROK_PAUSE          = 19,
   RETROK_ESCAPE         = 27,
   RETROK_SPACE          = 32,
   RETROK_EXCLAIM        = 33,
   RETROK_QUOTEDBL       = 34,
   RETROK_HASH           = 35,
   RETROK_DOLLAR         = 36,
   RETROK_AMPERSAND      = 38,
   RETROK_QUOTE          = 39,
   RETROK_LEFTPAREN      = 40,
   RETROK_RIGHTPAREN     = 41,
   RETROK_ASTERISK       = 42,
   RETROK_PLUS           = 43,
   RETROK_COMMA          = 44,
   RETROK_MINUS          = 45,
   RETROK_PERIOD         = 46,
   RETROK_SLASH          = 47,
   RETROK_0              = 48,
   RETROK_1              = 49,
   RETROK_2              = 50,
   RETROK_3              = 51,
   RETROK_4              = 52,
   RETROK_5              = 53,
   RETROK_6              = 54,
   RETROK_7              = 55,
   RETROK_8              = 56,
   RETROK_9              = 57,
   RETROK_COLON          = 58,
   RETROK_SEMICOLON      = 59,
   RETROK_LESS           = 60,
   RETROK_EQUALS         = 61,
   RETROK_GREATER        = 62,
   RETROK_QUESTION       = 63,
   RETROK_AT             = 64,
   RETROK_LEFTBRACKET    = 91,
   RETROK_BACKSLASH      = 92,
   RETROK_RIGHTBRACKET   = 93,
   RETROK_CARET          = 94,
   RETROK_UNDERSCORE     = 95,
   RETROK_BACKQUOTE      = 96,
   RETROK_a              = 97,
   RETROK_b              = 98,
   RETROK_c              = 99,
   RETROK_d              = 100,
   RETROK_e              = 101,
   RETROK_f              = 102,
   RETROK_g              = 103,
   RETROK_h              = 104,
   RETROK_i              = 105,
   RETROK_j              = 106,
   RETROK_k              = 107,
   RETROK_l              = 108,
   RETROK_m              = 109,
   RETROK_n              = 110,
   RETROK_o              = 111,
   RETROK_p              = 112,
   RETROK_q              = 113,
   RETROK_r              = 114,
   RETROK_s              = 115,
   RETROK_t              = 116,
   RETROK_u              = 117,
   RETROK_v              = 118,
   RETROK_w              = 119,
   RETROK_x              = 120,
   RETROK_y              = 121,
   RETROK_z              = 122,
   RETROK_LEFTBRACE      = 123,
   RETROK_BAR            = 124,
   RETROK_RIGHTBRACE     = 125,
   RETROK_TILDE          = 126,
   RETROK_DELETE         = 127,

   RETROK_KP0            = 256,
   RETROK_KP1            = 257,
   RETROK_KP2            = 258,
   RETROK_KP3            = 259,
   RETROK_KP4            = 260,
   RETROK_KP5            = 261,
   RETROK_KP6            = 262,
   RETROK_KP7            = 263,
   RETROK_KP8            = 264,
   RETROK_KP9            = 265,
   RETROK_KP_PERIOD      = 266,
   RETROK_KP_DIVIDE      = 267,
   RETROK_KP_MULTIPLY    = 268,
   RETROK_KP_MINUS       = 269,
   RETROK_KP_PLUS        = 270,
   RETROK_KP_ENTER       = 271,
   RETROK_KP_EQUALS      = 272,

   RETROK_UP             = 273,
   RETROK_DOWN           = 274,
   RETROK_RIGHT          = 275,
   RETROK_LEFT           = 276,
   RETROK_INSERT         = 277,
   RETROK_HOME           = 278,
   RETROK_END            = 279,
   RETROK_PAGEUP         = 280,
   RETROK_PAGEDOWN       = 281,

   RETROK_F1             = 282,
   RETROK_F2             = 283,
   RETROK_F3             = 284,
   RETROK_F4             = 285,
   RETROK_F5             = 286,
   RETROK_F6             = 287,
   RETROK_F7             = 288,
   RETROK_F8             = 289,
   RETROK_F9             = 290,
   RETROK_F10            = 291,
   RETROK_F11            = 292,
   RETROK_F12            = 293,
   RETROK_F13            = 294,
   RETROK_F14            = 295,
   RETROK_F15            = 296,

   RETROK_NUMLOCK        = 300,
   RETROK_CAPSLOCK       = 301,
   RETROK_SCROLLOCK      = 302,
   RETROK_RSHIFT         = 303,
   RETROK_LSHIFT         = 304,
   RETROK_RCTRL          = 305,
   RETROK_LCTRL          = 306,
   RETROK_RALT           = 307,
   RETROK_LALT           = 308,
   RETROK_RMETA          = 309,
   RETROK_LMETA          = 310,
   RETROK_LSUPER         = 311,
   RETROK_RSUPER         = 312,
   RETROK_MODE           = 313,
   RETROK_COMPOSE        = 314,

   RETROK_HELP           = 315,
   RETROK_PRINT          = 316,
   RETROK_SYSREQ         = 317,
   RETROK_BREAK          = 318,
   RETROK_MENU           = 319,
   RETROK_POWER          = 320,
   RETROK_EURO           = 321,
   RETROK_UNDO           = 322,
   RETROK_OEM_102        = 323,

   RETROK_LAST,

   RETROK_DUMMY          = INT_MAX /* Ensure sizeof(enum) == sizeof(int) */
};

enum retro_mod
{
   RETROKMOD_NONE       = 0x0000,

   RETROKMOD_SHIFT      = 0x01,
   RETROKMOD_CTRL       = 0x02,
   RETROKMOD_ALT        = 0x04,
   RETROKMOD_META       = 0x08,

   RETROKMOD_NUMLOCK    = 0x10,
   RETROKMOD_CAPSLOCK   = 0x20,
   RETROKMOD_SCROLLOCK  = 0x40,

   RETROKMOD_DUMMY = INT_MAX /* Ensure sizeof(enum) == sizeof(int) */
};

/* 設定されている場合、このコールはまだパブリックなlibretro APIの一部ではありません。
 * いつでも変更や削除が可能です。*/
#define RETRO_ENVIRONMENT_EXPERIMENTAL 0x10000
/* フロントエンドの内部で使用される環境コールバック。 */
#define RETRO_ENVIRONMENT_PRIVATE 0x20000

/* 環境コマンド。 */
#define RETRO_ENVIRONMENT_SET_ROTATION  1  /* const unsigned * --
                                            * グラフィックスの画面回転を設定する。
                                            * 有効な値は0, 1, 2, 3で、
                                            * それぞれ0, 90, 180, 270度反時計回りに
                                            * 画面が回転します。
                                            */
#define RETRO_ENVIRONMENT_GET_OVERSCAN  2  /* bool * --
                                            * 注：2019年現在、このコールバックは非推奨とされており、
                                            * コアオプションを使用して、より微妙な、
                                            * コア特有の方法でオーバースキャンを管理することが推奨されています。
                                            * 実装がオーバースキャンを使用するか、
                                            * オーバースキャンを切り取るかどうかを示すブール値。
                                            */
#define RETRO_ENVIRONMENT_GET_CAN_DUPE  3  /* bool * --
                                            * フロントエンドがフレームの二重化を
                                            * サポートしているかどうかを示すブール値で、
                                            * video frame callbackにはNULLを渡します。
                                            */

                                           /* Environ 4、5は、もはやサポートされておらず
                                            *（GET_VARIABLE / SET_VARIABLES）、
                                            * ABIの衝突の可能性を避けるために予約されています。
                                            */

#define RETRO_ENVIRONMENT_SET_MESSAGE   6  /* const struct retro_message * --
                                            * 一定の 'frames' 間、実装特有の方法で
                                            * メッセージを表示するように設定します。
                                            * 単純にRETRO_ENVIRONMENT_GET_LOG_INTERFACE 
                                            * (またはフォールバックとしてstderr)を介して
                                            * ロギングされるべき些細なメッセージには使用すべきではありません。
                                            */
#define RETRO_ENVIRONMENT_SHUTDOWN      7  /* N/A (NULL) --
                                            * フロントエンドにシャットダウンを要求します。
                                            * ゲーム中のメニューアイテムなどから
                                            * シャットダウンする方法がある場合にのみ使用してください。
                                            */
#define RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL 8
                                           /* const unsigned * --
                                            * この実装がシステム上でどの程度要求されるか、
                                            * フロントエンドにヒントを与える。
                                            * 例えば、レベルを2とすると、
                                            * この実装はレベル2以上のすべてのフロントエンドで
                                            * 適切に動作することを意味します。
                                            *
                                            * これは、フロントエンドがあまりにも負荷の高い実装を
                                            * 警告するために使用することができます。
                                            *
                                            * レベルは「floating」です。
                                            * 
                                            * この関数は、実装がプレイできる特定のゲームが
                                            * 特に厳しいものであるかもしれないので、
                                            * ゲームごとに呼び出すことができます。
                                            * 呼び出される場合は、
                                            * retro_load_game()の中で呼び出されなければなりません。
                                            */
#define RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY 9
                                           /* const char ** --
                                            * フロントエンドの「system」ディレクトリを返します。
                                            * このディレクトリは、BIOSや設定データなどの
                                            * システム固有のコンテンツを保存するために使用できます。
                                            * 返される値がNULLの場合もあります。
                                            * NULLの場合、そのようなディレクトリは定義されていませんので、
                                            * 適切なディレクトリを見つけるのは実装次第です。
                                            *
                                            * 注：一部のコアでは、
                                            * メモリカードなどの「保存」データのためにこのフォルダを使用していました。
                                            * 可能であれば、新しい GET_SAVE_DIRECTORY を使用するようにしてください。
                                            */
#define RETRO_ENVIRONMENT_SET_PIXEL_FORMAT 10
                                           /* const enum retro_pixel_format * --
                                            * 実装で使用される内部ピクセルフォーマットを設定します。
                                            * デフォルトのピクセルフォーマットは RETRO_PIXEL_FORMAT_0RGB1555 です。
                                            * しかし、このピクセルフォーマットは非推奨です 
                                            * (enum retro_pixel_format を参照してください)。
                                            * 呼び出しが false を返す場合、
                                            * フロントエンドはこのピクセルフォーマットをサポートしていません。
                                            * 
                                            * この関数は retro_load_game() 
                                            * または retro_get_system_av_info() の中で呼び出されるべきです。
                                            */
#define RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS 11
                                           /* const struct retro_input_descriptor * --
                                            * retro_input_descriptors の配列を設定します。
                                            * これを使用可能な方法で表示するのはフロントエンド次第です。
                                            * 配列は retro_input_descriptor::description が
                                            * NULL に設定されることで終了します。
                                            * この関数はいつでも呼び出すことができますが、
                                            * できるだけ早い段階で呼び出すことをお勧めします。
                                            */
#define RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK 12
                                           /* const struct retro_keyboard_callback * --
                                            * キーボードイベントをコアに通知するためのコールバック関数を設定します。
                                            */
#define RETRO_ENVIRONMENT_SET_DISK_CONTROL_INTERFACE 13
                                           /* const struct retro_disk_control_callback * --
                                            * フロントエンドがディスクイメージを取り出したり挿入したりする際に
                                            * 使用するインターフェースを設定します。
                                            * これは、複数のイメージで構成され、
                                            * ユーザーが手動で交換しなければならないゲーム（PSXなど）に使用されます。
                                            */
#define RETRO_ENVIRONMENT_SET_HW_RENDER 14
                                           /* struct retro_hw_render_callback * --
                                            * libretroコアがハードウェアアクセラレーションで
                                            * レンダリングするためのインターフェースを設定します。
                                            * retro_load_game()の中で呼び出されなければなりません。
                                            * 成功すると、libretro コアは、フロントエンドが提供する
                                            * フレームバッファにレンダリングできるようになります。
                                            * このフレームバッファのサイズは、
                                            * 少なくとも get_av_info() で提供される
                                            * max_width/max_height と同じ大きさになります。
                                            * HWレンダリングが使用される場合、
                                            * RETRO_HW_FRAME_BUFFER_VALIDまたはNULLのみを
                                            * retro_video_refresh_tに渡す。
                                            */
#define RETRO_ENVIRONMENT_GET_VARIABLE 15
                                           /* struct retro_variable * --
                                            * マルチシステムに対応できない環境から、
                                            * ユーザー定義の情報を取得するインターフェースです。
                                            * 'key'には、SET_VARIABLESですでに設定されているキーを設定します。
                                            * data'には値またはNULLが設定されます。
                                            */
#define RETRO_ENVIRONMENT_SET_VARIABLES 16
                                           /* const struct retro_variable * --
                                            * 実装は、GET_VARIABLEを使用して、
                                            * 後で確認したい変数を環境に通知することができます。
                                            * これにより、フロントエンドはこれらの変数を
                                            * ユーザーに動的に提示することができます。
                                            * これは可能な限り早い段階で最初に呼び出されるべきです
                                            * （理想的には retro_set_environment の中で）。
                                            * その後、更新されたオプションをフロントエンドに伝えるために、
                                            * コアのために再度呼び出されるかもしれませんが、
                                            * コアのオプションの数は最初の呼び出しの数から変更してはいけません。
                                            * 
                                            * 「data」は、{ NULL, NULL } 要素で終了する 
                                            * retro_variable 構造体の配列を指します。
                                            * retro_variable::key は、
                                            * 他の実装のキーと衝突しないように名前を付けなければならない。
                                            * 例：'foo'と呼ばれるコアは、'foo_option'という名前のキーを使用する。
                                            * retro_variable::value は、
                                            * 期待される値の'|'で区切られたリストと同様に、
                                            * キーの人間が読める説明を含むべきです。
                                            * 
                                            * 可能なオプションの数は非常に限られているべきで、
                                            * キーボードなしでオプションを循環させることが可能であるべきです。
                                            * 
                                            * 最初の入力はデフォルトとして扱われるべきである。
                                            * 
                                            * エントリーの例
                                            * { "foo_option", "Speed hack coprocessor X; false|true" }。
                                            * 最初の ';' の前のテキストは説明です。
                                            * この';'の後にはスペースを入れ、
                                            * さらに'|'で分割された可能な値のリストが続く必要があります。
                                            * 
                                            * 文字列のみが処理されます。
                                            * 可能な値は通常、フロントエンドでそのまま表示・保存されます。
                                            */
#define RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE 17
                                           /* bool * --
                                            * RETRO_ENVIRONMENT_GET_VARIABLEが最後に呼ばれてから、
                                            * フロントエンドによっていくつかの変数が更新された場合、
                                            * 結果はtrueに設定されます。
                                            * 変数は GET_VARIABLE で照会しなければなりません。
                                            */
#define RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME 18
                                           /* const bool * --
                                            * true の場合、libretro の実装は、
                                            * 引数に NULL を指定した retro_load_game() の呼び出しをサポートします。
                                            * 特定のゲームデータなしで実行できるコアによって使用されます。
                                            * これは retro_set_environment() の中でのみ呼び出されるべきである。
                                            */
#define RETRO_ENVIRONMENT_GET_LIBRETRO_PATH 19
                                           /* const char ** --
                                            * この libretro の実装がロードされた絶対パスを取得します。
                                            * libretro が静的にロードされた 
                                            * (すなわち、フロントエンドに静的にリンクされた) 場合や、
                                            * パスがわからない場合は NULL を返します。
                                            * SET_SUPPORT_NO_GAMEと連携すると、
                                            * 醜いハックをせずにアセットをロードすることができるので、非常に便利です。
                                            */

                                           /* 環境20は、SET_AUDIO_CALLBACKの廃止されたバージョンでした。
                                            * これは当時の既知のコアでは使用されておらず、APIから削除されました。*/
#define RETRO_ENVIRONMENT_SET_FRAME_TIME_CALLBACK 21
                                           /* const struct retro_frame_time_callback * --
                                            * retro_run()を最後に起動してから
                                            * どれだけの時間が経過したかをコアに知らせる。
                                            * フロントエンドは、早送り、スローモーション、フレームステッピングなどを
                                            * 装うために、タイミングを改ざんすることができます。
                                            * この場合、delta time は frame_time_callback の参照値を使用します。
                                            */
#define RETRO_ENVIRONMENT_SET_AUDIO_CALLBACK 22
                                           /* const struct retro_audio_callback * --
                                            * オーディオが書き込み可能になったことを
                                            * libretroコアに通知するためのインターフェイスを設定します。
                                            * このコールバックはどのスレッドからでも呼び出すことができるので、
                                            * これを使うコアはスレッドセーフなオーディオ実装を
                                            * 持っていなければなりません。
                                            * オーディオとビデオが完全に非同期で、
                                            * オーディオがその場で生成されるようなゲームを想定しています。
                                            * このインターフェースは、
                                            * 高度に同期したオーディオを持つエミュレータでの使用は推奨されません。
                                            * 
                                            * このコールバックは、書き込み可能かどうかを通知するだけで、
                                            * オーディオを書き込むためには、
                                            * libretroコアが通常のオーディオコールバックを呼び出す必要があります。
                                            * オーディオコールバックは、
                                            * 通知コールバックの中から呼び出す必要があります。
                                            * 書き込みを行うオーディオデータの量は、実装に依存します。
                                            * 一般的には、オーディオコールバックはループ内で継続的に呼び出されます。
                                            * 
                                            * スレッドの安全性が保証されていることと、
                                            * オーディオとビデオの同期が取れていないことから、
                                            * フロントエンドは内部設定に基づいて
                                            * このインターフェースを選択的に無効にすることができます。
                                            * このインターフェイスを使用するコアは、
                                            * 「通常の」オーディオインターフェイスも実装しなければなりません。
                                            * 
                                            * SET_AUDIO_CALLBACKを使用するlibretroコアは、
                                            * SET_FRAME_TIME_CALLBACKも使用しなければなりません。
                                            */
#define RETRO_ENVIRONMENT_GET_RUMBLE_INTERFACE 23
                                           /* struct retro_rumble_interface * --
                                            * libretro coreがコントローラの振動モーターの状態を
                                            * 設定するために使用するインターフェースを取得します。
                                            * 強いモーターと弱いモーターがサポートされており、
                                            * それぞれ独立して制御することができます。
                                            * retro_init()またはretro_load_game()の
                                            * いずれかから呼び出されなければなりません。
                                            * retro_set_environment()からは呼び出さないでください。
                                            * 振動機能が利用できない場合、falseを返します。
                                            */
#define RETRO_ENVIRONMENT_GET_INPUT_DEVICE_CAPABILITIES 24
                                           /* uint64_t * --
                                            * retro_input_state_t の呼び出しにおいて、
                                            * どのデバイスタイプが適切に処理されることが
                                            * 期待されるかを示すビットマスクを取得する。
                                            * 処理されない、または認識されないデバイスは、
                                            * 常にretro_input_state_tで0を返す。
                                            * ビットマスクの例：
                                            * caps = (1 << RETRO_DEVICE_JOYPAD) | (1 << RETRO_DEVICE_ANALOG).
                                            * retro_run()の中でのみ呼び出されるべきである。
                                            */
#define RETRO_ENVIRONMENT_GET_SENSOR_INTERFACE (25 | RETRO_ENVIRONMENT_EXPERIMENTAL)
                                           /* struct retro_sensor_interface * --
                                            * センサーインターフェースへのアクセスを取得します。
                                            * このインターフェースの目的は、
                                            * ポーリングレート、完全な有効化/無効化など、
                                            * センサーに関連する状態を設定できるようにすることです。
                                            * センサーの状態を読み取るには、
                                            * 通常の input_state_callback API を使用します。
                                            */
#define RETRO_ENVIRONMENT_GET_CAMERA_INTERFACE (26 | RETRO_ENVIRONMENT_EXPERIMENTAL)
                                           /* struct retro_camera_callback * --
                                            * ビデオカメラドライバへのインターフェースを取得します。
                                            * libretroコアは、ビデオカメラにアクセスするために
                                            * このインターフェイスを利用できます。
                                            * 新しいビデオフレームは、retro_run()と同じスレッドの
                                            * コールバックで配信されます。
                                            * 
                                            * GET_CAMERA_INTERFACE は 
                                            * retro_load_game() で呼び出されなければなりません。
                                            * 
                                            * 使用されるカメラの実装に応じて、
                                            * カメラフレームは生のフレームバッファとして、
                                            * あるいはOpenGLテクスチャとして直接配信されます。
                                            * 
                                            * コアはここで、どのタイプのバッファが適切に処理できるかを
                                            * フロントエンドに伝えなければなりません。
                                            * OpenGLテクスチャは、
                                            * libretro GLコア(SET_HW_RENDER)を使用している場合のみ処理できます。
                                            * カメラインターフェースを使用する場合は、
                                            * libretro GLコアの使用を推奨します。
                                            * 
                                            * カメラの自動起動ができません。
                                            * 取得したstart/stop関数を使って、
                                            * 明示的にカメラドライバを起動・停止する必要があります。
                                            */
#define RETRO_ENVIRONMENT_GET_LOG_INTERFACE 27
                                           /* struct retro_log_callback * --
                                            * ロギング用のインターフェースを取得します。
                                            * これは、ロギングに標準エラーを使用できないプラットフォームがあるため、
                                            * クロスプラットフォームの方法でロギングを行うのに便利です。
                                            * また、フロントエンドがロギング情報を
                                            * より適切な方法で表示できるようになります。
                                            * このインターフェイスを使用しない場合、
                                            * libretro のコアは必要に応じて stderr にログを記録します。
                                            */
#define RETRO_ENVIRONMENT_GET_PERF_INTERFACE 28
                                           /* struct retro_perf_callback * --
                                            * パフォーマンスカウンタのインターフェースを取得します。
                                            * これは、クロスプラットフォームでのパフォーマンスロギングや、
                                            * SIMDサポートなどのアーキテクチャ固有の機能を検出するのに便利です。
                                            */
#define RETRO_ENVIRONMENT_GET_LOCATION_INTERFACE 29
                                           /* struct retro_location_callback * --
                                            * 位置情報インターフェースへのアクセスを取得します。
                                            * このインターフェースの目的は、
                                            * ホストデバイスから現在の緯度や経度などの位置情報を取得することです。
                                            */
#define RETRO_ENVIRONMENT_GET_CONTENT_DIRECTORY 30 /* 旧名称、互換性のために残してあります。 */
#define RETRO_ENVIRONMENT_GET_CORE_ASSETS_DIRECTORY 30
                                           /* const char ** --
                                            * フロントエンドの "core assets "ディレクトリを返します。
                                            * このディレクトリには、アートアセットや入力データなど、
                                            * コアが必要とする特定のアセットを格納することができます。
                                            * 返された値がNULLの場合もあります。
                                            * NULL の場合は、そのようなディレクトリは定義されておらず、
                                            * 適切なディレクトリを見つけるのは実装側の役目となります。
                                            */
#define RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY 31
                                           /* const char ** --
                                            * 利用可能な保存ディレクトリがない場合を除き、
                                            * フロントエンドの "save" ディレクトリを返します。
                                            * libretroコアが通常のメモリインターフェイス(retro_get_memory_data())
                                            * を使用できない場合、SRAM、メモリカード、ハイスコアなどを保存するために、
                                            * saveディレクトリを使用する必要があります。
                                            * 
                                            * フロントエンドが保存ディレクトリを指定できない場合は、
                                            * 保存ディレクトリを設定せずにコアの動作を試みることを示すために
                                            * NULLを返します。
                                            * 
                                            * NOTE: 初期の libretro コアは、
                                            * 保存ファイルにシステムディレクトリを使用していました。
                                            * 下位互換性が必要なコアでは、
                                            * GET_SYSTEM_DIRECTORY をチェックすることができます。
                                            */
#define RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO 32
                                           /* const struct retro_system_av_info * --
                                            * 新しい av_info 構造体を設定します。
                                            * これは retro_run() の中からのみ呼び出すことができます。
                                            * これは、コアが内部の解像度、アスペクト比、タイミング、
                                            * サンプリングレートなどを完全に変更する場合に*のみ*使用されるべきです。
                                            * これを呼び出すと、フロントエンドのビデオ／オーディオドライバの
                                            * 完全な再初期化が必要になることがあるので、非常に控えめに、
                                            * そして通常はユーザーの明示的な同意を得てのみ呼び出すことが重要です。
                                            * 最終的にはドライバーの再初期化が行われるので、
                                            * 同じ retro_run()の中でこの呼び出しの後に行われる
                                            * ビデオやオーディオのコールバックは、
                                            * 新しく初期化されたドライバーをターゲットにします。
                                            * 
                                            * このコールバックにより、
                                            * ゲームで設定可能な解像度をサポートすることが可能になり、
                                            * max_width/max_heightで
                                            * 「最悪のケース」を設定しないようにするのに役立ちます。
                                            * 
                                            * ***強く推奨*** 
                                            * ドライバーの再初期化の可能性を考慮して、
                                            * エミュレータコアで解像度が一時的に変更されることが予想される場合は、
                                            * 毎回このコールバックを呼び出さないでください。
                                            * このコールは、retro_get_system_av_info()で
                                            * 正しい値を提供しようとしないことへのフリーパスではありません。
                                            * アスペクト比や公称の幅/高さのようなものを変更する必要がある場合は、
                                            * SET_SYSTEM_AV_INFO のよりソフトなバリエーションである 
                                            * RETRO_ENVIRONMENT_SET_GEOMETRY を使ってください。
                                            * 
                                            * これが false を返すと、
                                            * フロントエンドは av_info 構造体が変更されたことを認識しません。
                                            */
#define RETRO_ENVIRONMENT_SET_PROC_ADDRESS_CALLBACK 33
                                           /* const struct retro_get_proc_address_interface * --
                                            * libretro コアが get_proc_address() インターフェースの
                                            * サポートを表明できるようにします。
                                            * このインターフェイスは、環境コールの使用が間接的すぎる場合、
                                            * 例えばフロントエンドがコアに直接コールしたい場合などに、
                                            * libretroを拡張する標準的な方法を可能にします。
                                            * 
                                            * コアがこのインターフェースを公開したい場合、
                                            * SET_PROC_ADDRESS_CALLBACK は **必ず** retro_set_environment() の
                                            * 中から呼び出されなければなりません。
                                            */
#define RETRO_ENVIRONMENT_SET_SUBSYSTEM_INFO 34
                                           /* const struct retro_subsystem_info * --
                                            * この環境コールでは、libretroの「サブシステム」の概念を導入しています。
                                            * サブシステムとは、
                                            * 様々な種類のゲームをサポートするlibretro コアの一種です。
                                            * この目的は、Super Nintendo の Super GameBoy やSufami Turbo など、
                                            * 特別なニーズを持つエミュレータをサポートすることです。
                                            * また、libretroの実装がマルチシステムエミュレータである場合、
                                            * 明示的にサブシステムを選択するために使用することもできます。
                                            * 
                                            * サブシステム経由でのゲームのロードは
                                            * retro_load_game_special() で行われますが、
                                            * この環境コールは libretro コアが 
                                            * retro_load_game_special() で使用するために
                                            * どのサブシステムがサポートされているかを公開することを可能にします。
                                            * コアは、ゼロ化された retro_game_special_info 構造体で終了される
                                            * retro_game_special_info の配列を渡す。
                                            * 
                                            * コアがこの機能を使いたい場合、
                                            * SET_SUBSYSTEM_INFO は **必ず** retro_set_environment() の
                                            * 中から呼ばれなければなりません。
                                            */
#define RETRO_ENVIRONMENT_SET_CONTROLLER_INFO 35
                                           /* const struct retro_controller_info * --
                                            * この環境コールにより、libretro コアは
                                            * retro_set_controller_port_device()のコールで、
                                            * どのコントローラサブクラスが認識されるかを
                                            * フロントエンドに伝えることができます。
                                            * 
                                            * Super Nintendo のようないくつかのエミュレータでは、
                                            * 複数のライトガンタイプをサポートしており、
                                            * それらを特別に選択する必要があります。
                                            * そのため、Libretro API で提供されていない
                                            * 特別な種類の入力デバイスについて、
                                            * フロントエンドがコアに伝えることが必要になることがあります。
                                            * 
                                            * フロントエンドがそれらのデバイスの動作を理解するためには、
                                            * libretro APIで既に定義されているジェネリックなデバイスタイプの
                                            * 特殊なサブクラスとして定義されている必要があります。
                                            * 
                                            * コアは、空白の構造体で終端された
                                            * const struct retro_controller_info の配列を渡さなければなりません。
                                            * retro_controller_info 構造体の各要素は、
                                            * フロントエンドがアクティブなデバイスサブクラスを変更したことを
                                            * コアに示すために retro_set_controller_port_device() 関数が
                                            * 呼び出されたときに、 retro_set_controller_port_device() に
                                            * 渡される昇順のポートインデックスに対応します。
                                            * SEE ALSO: retro_set_controller_port_device() 
                                            * 構造体の中でコアによって提供される昇順の入力ポートインデックスは、
                                            * 一般的にフロントエンドによって、
                                            * Player 1、Player 2、Player 3などのように、
                                            * User #またはPlayer #の昇順で示される。
                                            * サポートされるデバイスサブクラスは、入力ポートごとに異なります。
                                            * 
                                            * retro_controller_info 配列の各エントリの最初の内部要素は
                                            * retro_controller_description 構造体であり、
                                            * サブクラスが派生する汎用 Libretro デバイスで始まる、
                                            * 対応するユーザーまたはプレーヤーで利用可能な
                                            * すべてのデバイスサブクラスの名前とコードを指定します。
                                            * 各エントリの 2 番目の内部要素は、retro_controller_description に
                                            * リストアップされているサブクラスの総数である。
                                            * 
                                            * 注意：特別なデバイスタイプが libretro core で設定されていても
                                            * libretro は基本の入力デバイスタイプに基づいて
                                            * 入力をポーリングするだけです。
                                            */
#define RETRO_ENVIRONMENT_SET_MEMORY_MAPS (36 | RETRO_ENVIRONMENT_EXPERIMENTAL)
                                           /* const struct retro_memory_map * --
                                            * この環境コールは、libretroコアがフロントエンドに、
                                            * そのコアがエミュレートするメモリマップを伝えるためのものです。
                                            * これを利用して、コアに依存しない方法で
                                            * チートなどを実装することができます。
                                            * 
                                            * エミュレータでのみ使用されるべきで、
                                            * 他のものにはあまり意味がありません。
                                            * retro_get_memory_* を通じて
                                            * 関連するすべてのポインタを公開することを推奨します。
                                            * 
                                            * retro_init や retro_load_game から呼び出すことができます。
                                            */
#define RETRO_ENVIRONMENT_SET_GEOMETRY 37
                                           /* const struct retro_game_geometry * --
                                            * この環境コールは、ビデオパラメーターを変更するための
                                            * SET_SYSTEM_AV_INFO に似ていますが、
                                            * ドライバーが再初期化されないことを保証するものです。
                                            * これは retro_run()内からのみ呼び出すことができる。
                                            * 
                                            * この呼び出しの目的は、
                                            * コアがアスペクト比と同様に公称幅/高さをオンザフライで
                                            * 変更できるようにすることであり、
                                            * これはいくつかのエミュレータがランタイムで変更する場合に有用である。
                                            * 
                                            * max_width/max_height引数は無視され、
                                            * このコールで変更することはできません。
                                            * これは、再初期化や非定常時間の操作が必要になる可能性があるからです。
                                            * max_width/max_heightを変更する場合は、
                                            * SET_SYSTEM_AV_INFOが必要である。
                                            * 
                                            * フロントエンドは、
                                            * この環境コールが一定時間で完了することを保証しなければなりません。
                                            */
#define RETRO_ENVIRONMENT_GET_USERNAME 38
                                           /* const char **
                                            * ユーザがフロントエンドのユーザ名を指定している場合に、
                                            * その指定されたユーザ名を返します。
                                            * このユーザ名は、
                                            * オンライン機能を持つコアのニックネームとして使用したり、
                                            * ユーザのパーソナライズが望ましいその他のモードで
                                            * 使用したりすることができます。
                                            * 戻り値は NULL になる可能性があります。
                                            * 有効なユーザ名を必要とするコアが
                                            * この environ コールバックを使用する場合は、
                                            * そのコアがデフォルトのユーザ名を指定する必要があります。
                                            */
#define RETRO_ENVIRONMENT_GET_LANGUAGE 39
                                           /* unsigned * --
                                            * ユーザがフロントエンドの言語を指定した場合に、
                                            * その指定された言語を返します。
                                            * これは、コアがローカライズのために使用することができます。
                                            */
#define RETRO_ENVIRONMENT_GET_CURRENT_SOFTWARE_FRAMEBUFFER (40 | RETRO_ENVIRONMENT_EXPERIMENTAL)
                                           /* struct retro_framebuffer * --
                                            * SET_HW_RENDERを使用していないときに、
                                            * コアがフレームのレンダリングに使用できる、
                                            * 事前に割り当てられたフレームバッファを返します。
                                            * このコールから返されたフレームバッファは、
                                            * retro_run()の現在のコールが戻った後に使用してはならない。
                                            * 
                                            * このコールの目的は、
                                            * コアがビデオメモリに直接レンダリングできる
                                            * ゼロコピー動作を可能にすることで、
                                            * コアからビデオメモリにメモリをコピーすることによる
                                            * 余分な帯域幅コストを回避することです。
                                            * 
                                            * このコールが成功し、コアがその中にレンダリングする場合、
                                            * フレームバッファのポインタとピッチは
                                            * retro_video_refresh_t に渡すことができる。
                                            * GET_CURRENT_SOFTWARE_FRAMEBUFFERからのバッファを使用する場合、
                                            * コアは、GET_CURRENT_SOFTWARE_FRAMEBUFFERから返されたものと
                                            * 全く同じポインタを渡さなければならない；
                                            * すなわち、バッファからオフセットされたポインタを渡すことは定義されていない。
                                            * また、幅、高さ、ピッチの各パラメータは、
                                            * GET_CURRENT_SOFTWARE_FRAMEBUFFERから取得した値と
                                            * 正確に一致しなければなりません。
                                            * 
                                            * フロントエンドが、SET_PIXEL_FORMAT で使用したものとは異なる
                                            * ピクセルフォーマットを返すことがあります。
                                            * これは、フロントエンドが変換を行う必要がある場合に起こります。
                                            * 
                                            * GET_CURRENT_SOFTWARE_FRAMEBUFFER が成功しても、
                                            * コアが別のバッファにレンダリングすることは有効です。
                                            * 
                                            * フロントエンドは、この関数で取得したポインターが
                                            * 書き込み可能（および読み取り可能）であることを確認する必要があります。
                                            */
#define RETRO_ENVIRONMENT_GET_HW_RENDER_INTERFACE (41 | RETRO_ENVIRONMENT_EXPERIMENTAL)
                                           /* const struct retro_hw_render_interface ** --
                                            * API固有のデータにアクセスするための、
                                            * API固有のレンダリングインターフェースを返します。
                                            * すべてのHWレンダリングAPIがこれをサポートしているわけでも、
                                            * 必要としているわけでもありません。
                                            * 返されるポインターの内容は、使用しているレンダリングAPIに固有のものです。
                                            * libretro_vulkan.hなどの各種ヘッダーを参照してください。
                                            * 
                                            * GET_HW_RENDER_INTERFACEは、context_resetが呼ばれる前には呼べません。
                                            * 同様に、context_destroyedコールバックが戻ってきた後では、
                                            * HW_RENDER_INTERFACEの内容は無効になります。
                                            */
#define RETRO_ENVIRONMENT_SET_SUPPORT_ACHIEVEMENTS (42 | RETRO_ENVIRONMENT_EXPERIMENTAL)
                                           /* const bool * --
                                            * true の場合、libretro の実装は
                                            * RETRO_ENVIRONMENT_SET_MEMORY_MAPS で設定されたメモリディスクリプタを介して、
                                            * あるいは retro_get_memory_data/retro_get_memory_size を介して、
                                            * 達成をサポートします。
                                            * 
                                            * これは retro_run の最初の呼び出しの前に呼ばれなければなりません。
                                            */
#define RETRO_ENVIRONMENT_SET_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE (43 | RETRO_ENVIRONMENT_EXPERIMENTAL)
                                           /* const struct retro_hw_render_context_negotiation_interface * --
                                            * コンテキストがどのように生成されるかを、
                                            * libretro core が frontend と交渉するためのインターフェイスを設定します。
                                            * このインターフェースのセマンティクスは、
                                            * 以前の SET_HW_RENDER でどの API が使われたかに依存します。
                                            * このインターフェイスは、
                                            * フロントエンドが HW レンダリングコンテキストを作成しようとしているときに
                                            * 使用されるので、SET_HW_RENDER の後、
                                            * context_reset コールバックの前に使用されます。
                                            */
#define RETRO_ENVIRONMENT_SET_SERIALIZATION_QUIRKS 44
                                           /* uint64_t * --
                                            * シリアル化に関連する quirk フラグを設定します。
                                            * フロントエンドは、
                                            * 認識できない、あるいはサポートしていないフラグをゼロにします。
                                            * retro_initまたはretro_load_gameのいずれかで設定する必要がありますが、
                                            * 両方ではありません。
                                            */
#define RETRO_ENVIRONMENT_SET_HW_SHARED_CONTEXT (44 | RETRO_ENVIRONMENT_EXPERIMENTAL)
                                           /* N/A (null) * --
                                            * ハードウェアコンテキストが設定されているときに、
                                            * フロントエンドは
                                            * 「共有」ハードウェアコンテキスト（主にOpenGLに適用されます）の使用を試みます。
                                            * 
                                            * フロントエンドが共有ハードウェアコンテキストを
                                            * サポートしている場合は true を、
                                            * サポートしていない場合は false を返します。
                                            * 
                                            * これは、SET_HW_RENDER envコールバックが使用されるまでは、何もしません。
                                            */
#define RETRO_ENVIRONMENT_GET_VFS_INTERFACE (45 | RETRO_ENVIRONMENT_EXPERIMENTAL)
                                           /* struct retro_vfs_interface_info * --
                                            * VFSインターフェースへのアクセスを取得します。
                                            * VFS の存在は、load_game や get_system/save/other_directory が呼ばれる前に、
                                            * フロントエンドがパスの配布を開始する前にコアが
                                            * VFS をサポートしていることを知らせるために、質問される必要があります。
                                            * retro_set_environment でそうすることが推奨されます。
                                            */
#define RETRO_ENVIRONMENT_GET_LED_INTERFACE (46 | RETRO_ENVIRONMENT_EXPERIMENTAL)
                                           /* struct retro_led_interface * --
                                            * libretro core がLEDの状態を設定するために
                                            * 使用するインターフェースを取得します。
                                            */
#define RETRO_ENVIRONMENT_GET_AUDIO_VIDEO_ENABLE (47 | RETRO_ENVIRONMENT_EXPERIMENTAL)
                                           /* int * --
                                            * フロントエンドがオーディオやビデオを必要としているかどうかをコアに伝えます。
                                            * 無効にした場合、フロントエンドはオーディオやビデオを破棄するので、
                                            * コアはフレームの生成やオーディオの生成を省略することができます。
                                            * これは、主にパフォーマンスを向上させるために使用されます。
                                            * Bit 0 (value 1):Enable Video
                                            * Bit 1 (value 2):Enable Audio
                                            * Bit 2 (value 4):Use Fast Savestates.
                                            * Bit 3 (value 8):Hard Disable Audio 
                                            * その他のビットは将来の使用のために予約されており、デフォルトでは0になります。
                                            * video が disable の場合:
                                            * * フロントエンドは、コアがハードウェアアクセラレーションによるフレームの提示を含め、
                                            *   いかなるビデオも生成しないことを望んでいます。
                                            * * フロントエンドのビデオフレームコールバックは何もしません。
                                            * * フレームを実行した後、次のフレームのビデオ出力は、
                                            *   ビデオが有効な場合と変わらないはずで、ステートの保存と読み込みにも問題はありません。
                                            * audio が disable の場合:
                                            * * フロントエンドは、コアがオーディオを生成しないことを望んでいます。
                                            * * フロントエンドのオーディオコールバックは何もしません。
                                            * * フレームを実行した後、次のフレームのオーディオ出力は、
                                            *   オーディオが有効な場合と変わらないはずで、ステートの保存と読み込みに問題はありません。
                                            * Fast Savestates:
                                            * * ロードするのと同じバイナリで作成されることが保証されています。
                                            * * ディスクに書き込まれたり、ディスクから読み込まれたりすることはありません。
                                            * * コアが状態のロードが成功することを想定していることを示唆しています。
                                            * * 可能であれば、コアがそのメモリバッファをインプレースで更新することを提案します。
                                            * * コアがメモリのクリアをスキップすることを提案します。
                                            * * コアがシステムのリセットをスキップすることを提案します。
                                            * * コアが検証ステップをスキップすることを提案します。
                                            * Hard Disable Audio:
                                            * * 先行して実行しているときのセカンダリコアに使用されます。
                                            * * フロントエンドがコアからのオーディオを必要としないことを示します。
                                            * * コアがオーディオの合成を停止することを示唆しますが、
                                            *   エミュレーションの精度を損なうものではありません。
                                            * * 次のフレームのオーディオ出力は重要ではなく、
                                            *   フロントエンドが将来的に正確なオーディオの状態を必要とすることはありません。
                                            * * Hard Disable Audioを使用する場合、ステートは決して保存されません。
                                            */
#define RETRO_ENVIRONMENT_GET_MIDI_INTERFACE (48 | RETRO_ENVIRONMENT_EXPERIMENTAL)
                                           /* struct retro_midi_interface ** --
                                            * 生データの入出力に使用可能なMIDIインターフェースを返します。
                                            */

#define RETRO_ENVIRONMENT_GET_FASTFORWARDING (49 | RETRO_ENVIRONMENT_EXPERIMENTAL)
                                            /* bool * --
                                            * フロントエンドが fastforwarding mode になっているかどうかを示す Boolean値。
                                            */

#define RETRO_ENVIRONMENT_GET_TARGET_REFRESH_RATE (50 | RETRO_ENVIRONMENT_EXPERIMENTAL)
                                            /* float * --
                                            * フロントエンドで現在使用されているターゲットリフレッシュレートを知ることができるフロート値。
                                            * 
                                            * コアはこの値を使って、理想的なリフレッシュレートやフレームレートを設定することができます。
                                            */

#define RETRO_ENVIRONMENT_GET_INPUT_BITMASKS (51 | RETRO_ENVIRONMENT_EXPERIMENTAL)
                                            /* bool * --
                                            * retro_input_state_t によって返される入力ビットマスクをフロントエンドが
                                            * サポートするかどうかを示すブール値です。
                                            * この利点は、すべてのボタンの状態を取得するために、
                                            * retro_input_state_t を複数回ではなく一度だけ呼び出す必要があることです。
                                            * 
                                            * これが true を返した場合、RETRO_DEVICE_ID_JOYPAD_MASK を
                                            *  'id' として retro_input_state_t に渡すことができます
                                            * （'device' が RETRO_DEVICE_JOYPAD に設定されていることを確認してください）。
                                            * これは全てのデジタルボタンのビットマスクを返します。
                                            */

#define RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION 52
                                           /* unsigned * --
                                            * 符号なしの値は、フロントエンドがサポートする
                                            * コア・オプション・インターフェースのAPIバージョン番号です。
                                            * コールバックが false を返した場合、API バージョンは 0 とみなされます。
                                            * 
                                            * レガシーコードでは、コアオプションは
                                            * retro_variable 構造体の配列を RETRO_ENVIRONMENT_SET_VARIABLES に渡すことで設定されます。
                                            * これは、コアオプションインターフェースのバージョンに関係なく、まだ行われるかもしれません。
                                            * 
                                            * しかしバージョンが >= 1 の場合、コアのオプションは代わりに
                                            * RETRO_ENVIRONMENT_SET_CORE_OPTIONS に
                                            * retro_core_option_definition 構造体の配列を渡すか、
                                            * RETRO_ENVIRONMENT_SET_CORE_OPTIONS_INTL に
                                            * retro_core_option_definition 構造体の 2 次元配列を渡すことで設定することができます。
                                            * これにより、コアはオプションのサブラベル情報を追加設定したり、
                                            * ローカリゼーションサポートを提供することができる。
                                            */

#define RETRO_ENVIRONMENT_SET_CORE_OPTIONS 53
                                           /* const struct retro_core_option_definition ** --
                                            * 実装は、GET_VARIABLEを使用して、後で確認したい変数を環境に通知することができます。
                                            * これにより、フロントエンドはこれらの変数をユーザーに動的に提示することができます。
                                            * これは、RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION が
                                            * API バージョンを >= 1 で返した場合にのみ呼び出されるべきです。
                                            * これは RETRO_ENVIRONMENT_SET_VARIABLES の代わりに呼ばれるべきです。
                                            * これはできるだけ早い段階で最初に呼ばれるべきです（理想的には retro_set_environment の中で）。
                                            * その後、更新されたオプションをフロントエンドに伝えるために、
                                            * コアのために再び呼ばれるかもしれませんが、
                                            * コアのオプションの数は最初の呼び出しの数から変更してはいけません。
                                            * 
                                            * 「data」は、 { NULL, NULL, NULL, {{0}}, NULL } 要素で終了する
                                            * retro_core_option_definition 構造体の配列を指します。
                                            * retro_core_option_definition::key は、
                                            * 他の実装のキーと衝突しないように名前を付けなければなりません。 
                                            * 例えば、'foo' と呼ばれるコアは、'foo_option' という名前のキーを使用しなければなりません。
                                            * retro_core_option_definition::desc は、キーの人間が読める説明を含むべきです。
                                            * retro_core_option_definition::info は、
                                            * 典型的なユーザがオプションの機能を理解するために必要な、
                                            * 人間が読める追加の情報テキストを含むべきです。
                                            * retro_core_option_definition::values は、 
                                            * { NULL, NULL } 要素で終了する retro_core_option_value 構造体の配列です。
                                            * > retro_core_option_definition::values[index].value は、
                                            *   期待されるオプション値である。
                                            * > retro_core_option_definition::values[index].label は、
                                            *   画面上に値を表示するときに使われる、人間が読めるラベルです。
                                            *   NULL の場合は、値そのものが使われます。
                                            * retro_core_option_definition::default_value は、デフォルトのコアオプション設定です。
                                            * retro_core_option_definition::values 配列の中の、
                                            * 期待されるオプション値の一つにマッチしなければなりません。
                                            * そうでない場合、あるいはデフォルト値がNULLの場合は、
                                            * retro_core_option_definition::values 配列の最初のエントリがデフォルトとして扱われます。

                                            * 可能なオプションの数は非常に限られたものでなければならず、
                                            * RETRO_NUM_CORE_OPTION_VALUES_MAX よりも小さくなければなりません。
                                            * つまり、キーボードを使わずにオプションを循環させることが可能でなければなりません。
                                            *
                                            * エントリーの例:
                                            * {
                                            *     "foo_option",
                                            *     "Speed hack coprocessor X",
                                            *     "Provides increased performance at the expense of reduced accuracy",
                                            * 	  {
                                            *         { "false",    NULL },
                                            *         { "true",     NULL },
                                            *         { "unstable", "Turbo (Unstable)" },
                                            *         { NULL, NULL },
                                            *     },
                                            *     "false"
                                            * }
                                            *
                                            * 文字列のみが操作されます。可能な値は通常、フロントエンドによってそのまま表示・保存されます。
                                            */

#define RETRO_ENVIRONMENT_SET_CORE_OPTIONS_INTL 54
                                           /* const struct retro_core_options_intl * --
                                            * 実装は、GET_VARIABLEを使用して、後で確認したい変数を環境に通知することができます。
                                            * これにより、フロントエンドはこれらの変数をユーザーに動的に提示することができます。
                                            * これは、RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION が
                                            * API バージョンを >= 1 で返した場合にのみ呼び出されるべきです。
                                            * これは RETRO_ENVIRONMENT_SET_VARIABLES の代わりに呼ばれるべきです。
                                            * これはできるだけ早い段階で最初に呼ばれるべきです
                                            * （理想的には retro_set_environment の中で）。
                                            * その後、更新されたオプションをフロントエンドに伝えるために、
                                            * コアのために再び呼ばれるかもしれませんが、
                                            * コアのオプションの数は最初の呼び出しの数から変わってはいけません。
                                            * 
                                            * これは基本的にRETRO_ENVIRONMENT_SET_CORE_OPTIONSと同じですが、
                                            * ローカリゼーションのサポートが追加されています。
                                            * 詳細については、RETRO_ENVIRONMENT_SET_CORE_OPTIONSコールバックの説明を参照してください。
                                            * 
                                            * data' は retro_core_options_intl 構造体を指します。
                                            * 
                                            * retro_core_options_intl::us は、アメリカ英語のコアオプションの実装を定義する
                                            * retro_core_option_definition 構造体の配列へのポインタです。
                                            * それは有効な配列を指していなければならない。
                                            * 
                                            * retro_core_options_intl::local は、
                                            * 現在のフロントエンド言語のコアオプションを定義する
                                            * retro_core_option_definition 構造体の配列へのポインタです。
                                            * これは NULL かもしれません
                                            * （その場合、 retro_core_options_intl::us がフロントエンドで使われます）。
                                            * この配列にない項目は、代わりに retro_core_options_intl::us から読み込まれます。
                                            * 
                                            * 注意：コアオプションのデフォルト値は、
                                            * 常に retro_core_options_intl::us 配列から取得されます。
                                            * retro_core_options_intl::local 配列内のいかなるデフォルト値も無視されます。
                                            */

#define RETRO_ENVIRONMENT_SET_CORE_OPTIONS_DISPLAY 55
                                           /* struct retro_core_option_display * --
                                            *
                                            * コアオプションを表示する際に、変数の表示・非表示を環境に通知することができます。
                                            * これは *suggestion* と見なされます。
                                            * フロントエンドがこのコールバックを無視することは自由であり、
                                            * その実装は必須とはみなされません。
                                            * 
                                            * 'data' は retro_core_option_display 構造体を指します。
                                            * 
                                            * retro_core_option_display::key は、
                                            * SET_VARIABLES/SET_CORE_OPTIONS によって既に設定されている変数識別子である。
                                            * 
                                            * retro_core_option_display::visible は boolean で、
                                            * 変数を表示するかどうかを指定します。
                                            * 
                                            * SET_VARIABLES/SET_CORE_OPTIONS を呼び出すと、
                                            * すべてのコアオプション変数はデフォルトで可視に設定されることに注意してください。
                                            */

#define RETRO_ENVIRONMENT_GET_PREFERRED_HW_RENDER 56
                                           /* unsigned * --
                                            *
                                            * フロントエンドに使用するハードウェアコンテキストを問い合わせることができます。
                                            * コアはこの情報を使って、SET_HW_RENDER で要求する特定のコンテキストを処理します。
                                            * 
                                            * 「data」は、符号なしの変数を指します。
                                            */

#define RETRO_ENVIRONMENT_GET_DISK_CONTROL_INTERFACE_VERSION 57
                                           /* unsigned * --
                                            * 符号なしの値は、フロントエンドがサポートする
                                            * ディスク制御インターフェースのAPIバージョン番号です。
                                            * callback が false を返した場合、API バージョンは 0 とみなされます。
                                            * 
                                            * レガシーコードでは、ディスク制御インターフェースは
                                            * retro_disk_control_callback 型の構造体を
                                            * RETRO_ENVIRONMENT_SET_DISK_CONTROL_INTERFACE に渡すことで定義されます。
                                            * これは、ディスク制御インターフェースのバージョンに関係なく行われるかもしれません。
                                            * 
                                            * しかしバージョンが >= 1 の場合、
                                            * ディスク制御インターフェースは代わりに
                                            * retro_disk_control_ext_callback 型の構造体を
                                            * RETRO_ENVIRONMENT_SET_DISK_CONTROL_EXT_INTERFACE に渡すことで定義されるかもしれません。
                                            * これにより、コアがフロントエンドにディスクイメージに関する追加情報を提供したり、
                                            * フロントエンドによる追加のディスクコントロール機能を有効にすることができます。
                                            */

#define RETRO_ENVIRONMENT_SET_DISK_CONTROL_EXT_INTERFACE 58
                                           /* const struct retro_disk_control_ext_callback * --
                                            * フロントエンドがディスクイメージを取り出したり挿入したり、
                                            * コアに登録されている個々のディスクイメージファイルの
                                            * 情報を取得したりするためのインターフェースを設定します。
                                            * これは、複数のイメージで構成され、ユーザーが手動で交換しなければならないゲーム
                                            * （PSXやフロッピーディスクベースのシステムなど）に使用されます。
                                            */

#define RETRO_ENVIRONMENT_GET_MESSAGE_INTERFACE_VERSION 59
                                           /* unsigned * --
                                            * 符号なしの値は、フロントエンドがサポートする
                                            * メッセージインターフェースのAPIバージョン番号です。
                                            * コールバックが false を返した場合、API バージョンは 0 とみなされます。
                                            * 
                                            * レガシーコードでは、retro_message 型の構造体を
                                            * RETRO_ENVIRONMENT_SET_MESSAGE に渡すことで、
                                            * 実装固有の方法でメッセージを表示することができます。
                                            * これは、メッセージインタフェースのバージョンに関係なく行うことができます。
                                            * 
                                            * しかし、バージョンが >= 1 の場合は、
                                            * RETRO_ENVIRONMENT_SET_MESSAGE_EXT に
                                            * retro_message_ext 型の構造体を渡すことで、代わりにメッセージを表示することができます。
                                            * これにより、コアはメッセージのロギングレベル、優先度、および宛先
                                            * （OSD、ロギングインターフェース、またはその両方）を指定することができます。
                                            */

#define RETRO_ENVIRONMENT_SET_MESSAGE_EXT 60
                                           /* const struct retro_message_ext * --
                                            * 一定の「フレーム」数の間、実装特有の方法でメッセージを表示するように設定します。
                                            * さらに、コアはメッセージのロギングレベル、優先度、宛先
                                            * （OSD、ロギングインターフェース、またはその両方）を指定することができます。
                                            * 単純に RETRO_ENVIRONMENT_GET_LOG_INTERFACE 
                                            * (または予備として stderr) を通してログに記録されるべき
                                            * 些細なメッセージには使用すべきではありません。
                                            */

#define RETRO_ENVIRONMENT_GET_INPUT_MAX_USERS 61
                                           /* unsigned * --
                                            * 符号なしの値は、フロントエンドが提供するアクティブな入力デバイスの数です。
                                            * この値は、フレーム間で変化することがありますが、各フレームの間は一定です。
                                            * コールバックがtrueを返した場合、コアはアクティブなデバイスの数以上の
                                            * インデックスを持つ入力デバイスをポーリングする必要はありません。
                                            * コールバックがfalseを返した場合、アクティブな入力デバイスの数は不明です。
                                            * この場合は、すべての入力デバイスがアクティブであると考えてください。
                                            */

#define RETRO_ENVIRONMENT_SET_AUDIO_BUFFER_STATUS_CALLBACK 62
                                           /* const struct retro_audio_buffer_status_callback * --
                                            * フロントエンドのオーディオバッファの占有率をコアに知らせます。
                                            * コアは，バッファのアンダーランを避けるために
                                            * フレームスキップを試みるために使用することができます。
                                            * コアは，NULLを渡すことで，フロントエンドのバッファの状態報告を無効にすることができます。
                                            */

#define RETRO_ENVIRONMENT_SET_MINIMUM_AUDIO_LATENCY 63
                                           /* const unsigned * --
                                            * フロントエンドのオーディオレイテンシーの最小値をミリ秒単位で設定します。
                                            * 結果として得られるオーディオレイテンシーは設定値よりも大きくなることがありますが、
                                            * ハードウェアの制限がある場合は小さくなります。
                                            * フロントエンドは、512ミリ秒までの要求に応えることが期待されます。
                                            *
                                            * - 値が現在のフロントエンドのオーディオレイテンシーよりも小さい場合、
                                            *   コールバックは何の影響も受けません。
                                            * - 値がゼロの場合、デフォルトのフロントエンドオーディオレイテンシーが設定される。
                                            *
                                            * オーディオレイテンシーを増加させ、
                                            * 集中的な操作を行う際のバッファアンダーラン(クラックリング)の可能性を減少させるために、
                                            * コアによって使用されるかもしれません。
                                            * RETRO_ENVIRONMENT_SET_AUDIO_BUFFER_STATUS_CALLBACKを利用して
                                            * オーディオバッファベースのフレームスキップを実装するコアは、
                                            * オーディオレイテンシを予想されるフレーム時間の「高」（通常は6倍または8倍）の
                                            * 整数倍に設定することで最適な結果を得ることができる。
                                            *
                                            * 警告：これは retro_run() の中からのみ呼び出すことができます。
                                            * これを呼び出すと、フロントエンドのオーディオドライバの完全な再初期化が必要になるので、
                                            * 非常に控えめに、そして通常はユーザーの明示的な同意を得てのみ呼び出すことが重要です。
                                            * 最終的なドライバーの再初期化が行われるので、
                                            * 同じ retro_run()の中でこの呼び出しの後に行われるオーディオコールバックは、
                                            * 新しく初期化されたドライバーをターゲットにします。
                                            */

#define RETRO_ENVIRONMENT_SET_FASTFORWARDING_OVERRIDE 64
                                           /* const struct retro_fastforwarding_override * --
                                            * libretro コアが、フロントエンドの現在の高速転送モードを上書きするために使用します。
                                            * この関数に NULL を渡した場合、
                                            * フロントエンドはfastforwarding override 機能がサポートされていれば true を返します 
                                            * (この場合、fastforwarding の状態は変化しません)。
                                            */

#define RETRO_ENVIRONMENT_SET_CONTENT_INFO_OVERRIDE 65
                                           /* const struct retro_system_content_info_override * --
                                            * retro_get_system_info() によって報告された 
                                            * 'global' content info パラメータをオーバーライドすることを可能にします。
                                            * オーバーライドは、 RETRO_ENVIRONMENT_SET_SUBSYSTEM_INFOで
                                            * 設定されたサブシステムのコンテンツ情報パラメータにも影響します。
                                            * この関数は retro_set_environment() の中で呼ばれなければなりません。
                                            * コールバックが false を返す場合、
                                            * コンテンツ情報の上書きはフロントエンドではサポートされておらず、無視されます。
                                            * コールバックが true を返した場合、 
                                            * retro_load_game() や retro_load_game_special() で
                                            * RETRO_ENVIRONMENT_GET_GAME_INFO_EXT を呼び出すことで、
                                            * 拡張されたゲーム情報を取得することができます。
                                            * 
                                            * 'data' は、{ NULL, false, false } 要素で終了する
                                            * retro_system_content_info_override 構造体の配列を指します。
                                            * コアは、RETRO_ENVIRONMENT_SET_CONTENT_INFO_OVERRIDE および
                                            * RETRO_ENVIRONMENT_GET_GAME_INFO_EXT コールバックが
                                            * フロントエンドでサポートされているかどうかをテストするために、NULL を渡すことができます。
                                            * 
                                            * 構造体メンバーの説明については、
                                            * struct retro_system_content_info_override の定義を参照してください。
                                            *
                                            * 例:
                                            *
                                            * - struct retro_system_info:
                                            * {
                                            *    "My Core",                      // library_name
                                            *    "v1.0",                         // library_version
                                            *    "m3u|md|cue|iso|chd|sms|gg|sg", // valid_extensions
                                            *    true,                           // need_fullpath
                                            *    false                           // block_extract
                                            * }
                                            *
                                            * - Array of struct retro_system_content_info_override:
                                            * {
                                            *    {
                                            *       "md|sms|gg", // extensions
                                            *       false,       // need_fullpath
                                            *       true         // persistent_data
                                            *    },
                                            *    {
                                            *       "sg",        // extensions
                                            *       false,       // need_fullpath
                                            *       false        // persistent_data
                                            *    },
                                            *    { NULL, false, false }
                                            * }
                                            *
                                            * 結果:
                                            * - m3u, cue, iso, chd タイプのファイルは、フロントエンドでは読み込まれません。
                                            *   フロントエンドはコアに有効なパスを渡し、コアは内部で読み込みを処理します。
                                            * - m3u, cue, iso, chd タイプのファイルは、フロントエンドでは読み込まれません。
                                            *   フロントエンドはコアに有効なパスを渡し、コアは内部で読み込みを処理します。
                                            * - sg 型のファイルは、フロントエンドによって読み込まれます。
                                            *   有効なメモリバッファがコアに渡されます。
                                            *   このメモリバッファは、retro_load_game() 
                                            *   (または retro_load_game_special())が以下を返すまで有効です。
                                            *
                                            * 注： retro_system_content_info_override 構造体の配列に
                                            * 拡張機能が複数回リストアップされている場合は、最初のインスタンスのみが登録されます。
                                            */

#define RETRO_ENVIRONMENT_GET_GAME_INFO_EXT 66
                                           /* const struct retro_game_info_ext ** --
                                            * 追加のコンテンツパスやメモリバッファの状態の詳細を提供する、
                                            * 拡張されたゲーム情報を取得することを許可する。
                                            * この関数は retro_load_game() または 
                                            * retro_load_game_special() の内部でのみ呼び出されます。
                                            * コールバックが false を返す場合、
                                            * 拡張ゲーム情報はフロントエンドではサポートされていません。
                                            * この場合、通常の retro_game_info だけが利用できます。
                                            * RETRO_ENVIRONMENT_GET_GAME_INFO_EXTは、
                                            * RETRO_ENVIRONMENT_SET_CONTENT_INFO_OVERRIDEがtrueを返した場合に
                                            * trueを返すことが保証されています。
                                            * 
                                            * 'data' は retro_game_info_ext 構造体の配列を指します。
                                            * 
                                            * 構造体メンバーの説明については、 struct retro_game_info_ext の定義を参照してください。
                                            *
                                            * - この関数が retro_load_game() の中で呼ばれた場合、 
                                            *  retro_game_info_ext 配列のサイズが 1 であることが保証されます。
                                            *  つまり、返されたポインターは、最初の retro_game_info_ext 構造体のメンバーに
                                            *  直接アクセスするために使用されるかもしれないということです, 例として:
                                            *
                                            *      struct retro_game_info_ext *game_info_ext;
                                            *      if (environ_cb(RETRO_ENVIRONMENT_GET_GAME_INFO_EXT, &game_info_ext))
                                            *         printf("Content Directory: %s\n", game_info_ext->dir);
                                            *
                                            * - この関数が retro_load_game_special() の中で呼び出された場合、
                                            *   retro_game_info_ext 配列は retro_load_game_special() に渡された
                                            *   num_info 引数に等しいサイズを持つことが保証されます。
                                            */

/* VFS機能 */

/* File paths:
 * 本APIを使用する際にパラメータとして渡されるファイルパスは、UNIXスタイルの整ったものでなければならず、
 * プラットフォームのネイティブセパレータにかかわらず、ディレクトリセパレータとして「/」（引用符のないフォワードスラッシュ）を使用します。
 * また、パスには少なくとも1つのフォワードスラッシュが含まれていなければなりません
 * （「game.bin」は無効なパスなので、「./game.bin」を使用してください）。
 * ディレクトリセパレータ以外のパスの形式については、コアは仮定してはならない。
 * "C:/path/game.bin", "http://example.com/game.bin", "#game/game.bin", "./game.bin" (引用符なし)はすべて有効なパスです。
 * しかし、コアはフロントエンドに要求するパスに、"./"、"../"、複数の連続したフォワードスラッシュ("//")を付加してはなりません。
 * フロントエンドは、そのようなパスをできるだけうまく機能させることが推奨されますが、コアがパスを変更しすぎた場合には、あきらめることができます。
 * フロントエンドはネイティブなファイルシステムのパスをサポートすることが推奨されますが、必須ではありません 
 * (ディレクトリセパレータを置き換える場合はその限りではありません)。
 * コアはそれらの使用を試みることができますが、フロントエンドがそのような要求を拒否した場合、機能を維持しなければなりません。
 * コアは、ファイル I/O に libretro-common filestream 関数を使用することが推奨されます。
 * これらの関数は、VFS とシームレスに統合され、 ディレクトリセパレータの置き換えを適切に処理し、 
 * フロントエンドが VFS をサポートしていない場合にプラットフォーム固有のフォールバックを提供します。*/

/* 不透明なファイル ハンドル
 * VFS API v1 で導入 */
struct retro_vfs_file_handle;

/* 不透明なディレクトリハンドル
 * VFS API v3 で導入 */
struct retro_vfs_dir_handle;

/* ファイル オープン フラグ
 * VFS API v1 で導入 */
#define RETRO_VFS_FILE_ACCESS_READ            (1 << 0) /* 読み取り専用モード  */
#define RETRO_VFS_FILE_ACCESS_WRITE           (1 << 1) /* RETRO_VFS_FILE_ACCESS_UPDATEが指定されていない限り、書き込み専用モードで、内容は破棄され、既存のファイルが上書きされる */
#define RETRO_VFS_FILE_ACCESS_READ_WRITE      (RETRO_VFS_FILE_ACCESS_READ | RETRO_VFS_FILE_ACCESS_WRITE) /* RETRO_VFS_FILE_ACCESS_UPDATEが指定されていない限り、読み書き両用モードで、内容は破棄され、既存のファイルは上書きされます。*/
#define RETRO_VFS_FILE_ACCESS_UPDATE_EXISTING (1 << 2) /* 書き込み用に開かれた既存のファイルの内容が破棄されるのを防ぐ */

/* これらはヒントに過ぎません。フロントエンドはこれらを無視しても構いません。
  RAMやCPUなどの使用状況や、外部からの予期せぬ干渉（例えば、他の人がそのファイルに書き込んだり、そのファイルのサーバーがダウンしたり）
  に対する反応以外は、動作は変わりません。 */
#define RETRO_VFS_FILE_ACCESS_HINT_NONE              (0)
/* そのファイルが何度もアクセスされることを示します。フロントエンドはすべてを積極的にキャッシュします。 */
#define RETRO_VFS_FILE_ACCESS_HINT_FREQUENT_ACCESS   (1 << 0)

/* Seek positions */
#define RETRO_VFS_SEEK_POSITION_START    0
#define RETRO_VFS_SEEK_POSITION_CURRENT  1
#define RETRO_VFS_SEEK_POSITION_END      2

/* stat() 結果 フラグ
 * VFS API v3 で導入 */
#define RETRO_VFS_STAT_IS_VALID               (1 << 0)
#define RETRO_VFS_STAT_IS_DIRECTORY           (1 << 1)
#define RETRO_VFS_STAT_IS_CHARACTER_SPECIAL   (1 << 2)

/* 不透明なハンドルからパスを取得します。ハンドルを取得する際に file_open に渡されたものと全く同じパスを返します。
 * VFS API v1 で導入 */
typedef const char *(RETRO_CALLCONV *retro_vfs_get_path_t)(struct retro_vfs_file_handle *stream);

/* ファイルを読み込みまたは書き込みのために開きます。pathがディレクトリを指している場合は失敗します。
 * 不透明なファイルハンドルを返すか、エラーの場合は NULL を返します。
 * VFS API v1 で導入 */
typedef struct retro_vfs_file_handle *(RETRO_CALLCONV *retro_vfs_open_t)(const char *path, unsigned mode, unsigned hints);

/* ファイルを閉じてリソースを解放します。open_fileがNULL以外を返した場合に呼び出さなければなりません。
   成功すれば0，失敗すれば-1を返します。呼び出しが成功してもしなくても、
   パラメータとして渡されたハンドルは無効になるので、今後は使用しないでください。
 * VFS API v1 で導入 */
typedef int (RETRO_CALLCONV *retro_vfs_close_t)(struct retro_vfs_file_handle *stream);

/* ファイルのサイズをバイト数で返し、エラーの場合は-1を返します。
 * VFS API v1 で導入 */
typedef int64_t (RETRO_CALLCONV *retro_vfs_size_t)(struct retro_vfs_file_handle *stream);

/* ファイルを指定したサイズに切り詰めます。成功した場合は0、エラーの場合は-1を返します。
 * VFS API v2 で導入 */
typedef int64_t (RETRO_CALLCONV *retro_vfs_truncate_t)(struct retro_vfs_file_handle *stream, int64_t length);

/* ファイルの現在の読み取り/書き込み位置を取得します。エラーの場合は-1を返します。
 * VFS API v1 で導入 */
typedef int64_t (RETRO_CALLCONV *retro_vfs_tell_t)(struct retro_vfs_file_handle *stream);

/* ファイルの現在の読み取り/書き込み位置を設定します。新しい位置を返し、エラーの場合は-1を返します。
 * VFS API v1 で導入 */
typedef int64_t (RETRO_CALLCONV *retro_vfs_seek_t)(struct retro_vfs_file_handle *stream, int64_t offset, int seek_position);

/* ファイルからデータを読み込みます。読み込んだバイト数を返し、エラーの場合は-1を返します。
 * VFS API v1 で導入 */
typedef int64_t (RETRO_CALLCONV *retro_vfs_read_t)(struct retro_vfs_file_handle *stream, void *s, uint64_t len);

/* ファイルにデータを書き込みます。書き込んだバイト数を返し、エラーの場合は-1を返します。
 * VFS API v1 で導入 */
typedef int64_t (RETRO_CALLCONV *retro_vfs_write_t)(struct retro_vfs_file_handle *stream, const void *s, uint64_t len);

/* バッファードIOを使用している場合、ファイルへの保留中の書き込みをフラッシュします。成功した場合は0、失敗した場合は-1を返します。
 * VFS API v1 で導入 */
typedef int (RETRO_CALLCONV *retro_vfs_flush_t)(struct retro_vfs_file_handle *stream);

/* 指定されたファイルを削除します。成功すると0、失敗すると-1を返します。
 * VFS API v1 で導入 */
typedef int (RETRO_CALLCONV *retro_vfs_remove_t)(const char *path);

/* 指定されたファイルの名前を変更します。成功すると0、失敗すると-1を返します。
 * VFS API v1 で導入 */
typedef int (RETRO_CALLCONV *retro_vfs_rename_t)(const char *old_path, const char *new_path);

/* 指定されたファイルを統計します。RETRO_VFS_STAT_*フラグのビットマスクを再実行し、パスが有効でなかった場合は何も設定されません。
 * さらに、NULLが与えられない限り、与えられた変数にファイル・サイズを格納します。
 * VFS API v3 で導入 */
typedef int (RETRO_CALLCONV *retro_vfs_stat_t)(const char *path, int32_t *size);

/* 指定されたディレクトリを作成します。成功した場合は0、失敗した場合は-1、すでに存在する場合は-2を返します。
 * VFS API v3 で導入 */
typedef int (RETRO_CALLCONV *retro_vfs_mkdir_t)(const char *dir);

/* 指定されたディレクトリをリストアップするために開きます。不透明な dir ハンドルを返すか，エラーの場合は NULL を返します。
 * include_hidden引数のサポートは、プラットフォームによって異なる場合があります。
 * VFS API v3 で導入 */
typedef struct retro_vfs_dir_handle *(RETRO_CALLCONV *retro_vfs_opendir_t)(const char *dir, bool include_hidden);

/* 現在の位置にあるディレクトリエントリを読み込み、読み込みポインタを次の位置に移動させます。
 * 成功した場合は true を、すでに最後のエントリにいる場合は false を返します。
 * VFS API v3 で導入 */
typedef bool (RETRO_CALLCONV *retro_vfs_readdir_t)(struct retro_vfs_dir_handle *dirstream);

/* 最後に読み込んだエントリーの名前を取得します。成功した場合は文字列を、エラーの場合は NULL を返します。
 * 返された文字列ポインタは、次に readdir または closedir を呼び出すまで有効です。
 * VFS API v3 で導入 */
typedef const char *(RETRO_CALLCONV *retro_vfs_dirent_get_name_t)(struct retro_vfs_dir_handle *dirstream);

/* 最後に読み込んだエントリがディレクトリかどうかをチェックします。ディレクトリであればtrueを、そうでなければfalseを返します（またはエラー時）。
 * VFS API v3 で導入 */
typedef bool (RETRO_CALLCONV *retro_vfs_dirent_is_dir_t)(struct retro_vfs_dir_handle *dirstream);

/* ディレクトリを閉じて、そのリソースを解放します。opendirがNULL以外を返した場合に呼び出さなければなりません。成功すれば0，失敗すれば-1を返します。
 * 呼び出しが成功してもしなくても，パラメータとして渡されたハンドルは無効となり，今後は使用しないでください。
 * VFS API v3 で導入 */
typedef int (RETRO_CALLCONV *retro_vfs_closedir_t)(struct retro_vfs_dir_handle *dirstream);

struct retro_vfs_interface
{
   /* VFS API v1 */
	retro_vfs_get_path_t get_path;
	retro_vfs_open_t open;
	retro_vfs_close_t close;
	retro_vfs_size_t size;
	retro_vfs_tell_t tell;
	retro_vfs_seek_t seek;
	retro_vfs_read_t read;
	retro_vfs_write_t write;
	retro_vfs_flush_t flush;
	retro_vfs_remove_t remove;
	retro_vfs_rename_t rename;
   /* VFS API v2 */
   retro_vfs_truncate_t truncate;
   /* VFS API v3 */
   retro_vfs_stat_t stat;
   retro_vfs_mkdir_t mkdir;
   retro_vfs_opendir_t opendir;
   retro_vfs_readdir_t readdir;
   retro_vfs_dirent_get_name_t dirent_get_name;
   retro_vfs_dirent_is_dir_t dirent_is_dir;
   retro_vfs_closedir_t closedir;
};

struct retro_vfs_interface_info
{
   /* コアによって設定されます。フロントエンドがサポートするバージョンよりも高い場合、
    * フロントエンドはRETRO_ENVIRONMENT_GET_VFS_INTERFACEコールでfalseを返します。
    * VFS API v1 で導入 */
   uint32_t required_interface_version;

   /* フロントエンドはここにインターフェースポインタを書き込みます。フロントエンドは実際のバージョンも設定します。
    * これは少なくとも required_interface_version でなければなりません。
    * Introduced in VFS API v1 */
   struct retro_vfs_interface *iface;
};

enum retro_hw_render_interface_type
{
	RETRO_HW_RENDER_INTERFACE_VULKAN = 0,
	RETRO_HW_RENDER_INTERFACE_D3D9   = 1,
	RETRO_HW_RENDER_INTERFACE_D3D10  = 2,
	RETRO_HW_RENDER_INTERFACE_D3D11  = 3,
	RETRO_HW_RENDER_INTERFACE_D3D12  = 4,
   RETRO_HW_RENDER_INTERFACE_GSKIT_PS2  = 5,
   RETRO_HW_RENDER_INTERFACE_DUMMY  = INT_MAX
};

/* 基本構造体。
 * すべての retro_hw_render_interface_* 型は、少なくともこれらのフィールドを含みます。*/
struct retro_hw_render_interface
{
   enum retro_hw_render_interface_type interface_type;
   unsigned interface_version;
};

typedef void (RETRO_CALLCONV *retro_set_led_state_t)(int led, int state);
struct retro_led_interface
{
    retro_set_led_state_t set_led_state;
};

/* MIDI入力の現在の状態を取得します。
 * 有効な場合は true、そうでない場合は false を返します。 */
typedef bool (RETRO_CALLCONV *retro_midi_input_enabled_t)(void);

/* MIDI出力の現在の状態を取得します。
 * 有効な場合はtrue、そうでない場合はfalseを返します */
typedef bool (RETRO_CALLCONV *retro_midi_output_enabled_t)(void);

/* 入力ストリームから次のバイトを読み込みます。
 * バイトが読み込まれた場合は true、そうでない場合は false を返します。 */
typedef bool (RETRO_CALLCONV *retro_midi_read_t)(uint8_t *byte);

/* 出力ストリームにバイトを書き込みます。
 * 'delta_time' はマイクロ秒単位で、前回の書き込みからの経過時間を表します。
 * バイトが書き込まれた場合は true、そうでない場合は false を返します。*/
typedef bool (RETRO_CALLCONV *retro_midi_write_t)(uint8_t byte, uint32_t delta_time);

/* 以前に書き込まれたデータをフラッシュします。
 * 成功すればtrue、そうでなければfalseを返します。 */
typedef bool (RETRO_CALLCONV *retro_midi_flush_t)(void);

struct retro_midi_interface
{
   retro_midi_input_enabled_t input_enabled;
   retro_midi_output_enabled_t output_enabled;
   retro_midi_read_t read;
   retro_midi_write_t write;
   retro_midi_flush_t flush;
};

enum retro_hw_render_context_negotiation_interface_type
{
   RETRO_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE_VULKAN = 0,
   RETRO_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE_DUMMY = INT_MAX
};

/* 基本構造体。
 * すべての retro_hw_render_context_negotiation_interface_* 型は、少なくともこれらのフィールドを含む。 */
struct retro_hw_render_context_negotiation_interface
{
   enum retro_hw_render_context_negotiation_interface_type interface_type;
   unsigned interface_version;
};

/* シリアル化された状態が何らかの形で不完全である。
 * シリアライズが一般的なエンドユーザーのケースで使用可能であるが、
 * ネットプレイや再録画などのフレームセンシティブなフロントエンド機能の実装に依存してはならない場合に設定します。*/
#define RETRO_SERIALIZATION_QUIRK_INCOMPLETE (1 << 0)
/* retro_serialize()は最初は失敗します；
 * retro_unserialize()やretro_serialize_size()も正しく動作しないかもしれません。*/
#define RETRO_SERIALIZATION_QUIRK_MUST_INITIALIZE (1 << 1)
/* シリアライズサイズは、セッション内で変更されることがあります。 */
#define RETRO_SERIALIZATION_QUIRK_CORE_VARIABLE_SIZE (1 << 2)
/* フロントエンドが可変サイズの状態をサポートしていることを確認するために設定します。 */
#define RETRO_SERIALIZATION_QUIRK_FRONT_VARIABLE_SIZE (1 << 3)
/* シリアル化された状態は、同一セッション内でのみ読み込むことができます。 */
#define RETRO_SERIALIZATION_QUIRK_SINGLE_SESSION (1 << 4)
/* シリアル化された状態は、保存されたものとは異なるエンディアンのアーキテクチャではロードできません。 */
#define RETRO_SERIALIZATION_QUIRK_ENDIAN_DEPENDENT (1 << 5)
/* シリアライズされた状態は、ワードサイズの依存性などエンディアン以外の理由で、保存されたものとは異なるプラットフォームではロードできません。 */
#define RETRO_SERIALIZATION_QUIRK_PLATFORM_DEPENDENT (1 << 6)

#define RETRO_MEMDESC_CONST      (1 << 0)   /* retro_load_game が戻ってくると、フロントエンドはこのメモリ領域を決して変更しません。 */
#define RETRO_MEMDESC_BIGENDIAN  (1 << 1)   /* ビッグエンディアンのデータを格納するメモリ領域。デフォルトはリトルエンディアン。 */
#define RETRO_MEMDESC_SYSTEM_RAM (1 << 2)   /* システムRAMのメモリ領域。 ゲームシステムのメインRAMです。 */
#define RETRO_MEMDESC_SAVE_RAM   (1 << 3)   /* Save RAMのメモリ領域。このRAMは通常、ゲームカートリッジに搭載されており、バッテリーでバックアップされている。 */
#define RETRO_MEMDESC_VIDEO_RAM  (1 << 4)   /* ビデオRAM（VRAM）のメモリ領域  */
#define RETRO_MEMDESC_ALIGN_2    (1 << 16)  /* このエリアのすべてのメモリアクセスは、自身のサイズか2のいずれか小さい方にアラインされます。 */
#define RETRO_MEMDESC_ALIGN_4    (2 << 16)
#define RETRO_MEMDESC_ALIGN_8    (3 << 16)
#define RETRO_MEMDESC_MINSIZE_2  (1 << 24)  /* この領域のすべてのメモリは、少なくとも2バイトずつアクセスされます。 */
#define RETRO_MEMDESC_MINSIZE_4  (2 << 24)
#define RETRO_MEMDESC_MINSIZE_8  (3 << 24)
struct retro_memory_descriptor
{
   uint64_t flags;

   /* 該当するROMまたはRAMチップの先頭へのポインタ。
    * ポインタで計算するよりも、可能な限り「オフセット」を使用することを強く推奨します。
    * 
    * 同じバイトが複数の記述子にマッピングされている場合、それらの記述子は同じポインタを持たなければならない。
    * もし 'start' がポインタの最初のバイトを指していない場合は、代わりに 'offset' にその差を入れます。
    * 
    * ここに使用できるものがない場合は、NULLでもよい(例：ハードウェアレジスタやオープンバス)。ポインタがNULLの場合はフラグを立ててはいけません。
    * 可能であれば，ディスクリプターの数を最小限にすることを推奨しますが，必須ではありません．*/
   void *ptr;
   size_t offset;

   /* これは、エミュレートされたアドレス空間の中で、マッピングを開始する位置です。 */
   size_t start;

   /* このマッピングを適用するには、どのビットが「start」と同じでなければならないか。
    * あるバイトを主張する最初のメモリ記述子が適用されます。
    * 「start」で設定されているビットは、「start」でも設定されていなければなりません。
    * 0を指定することもでき、その場合は各バイトが1回だけマッピングされたとみなされます。
    * この場合、'len'は2の累乗でなければなりません。 */
   size_t select;

   /* この値が0でない場合、セットされたビットはメモリチップのアドレス端子に接続されていないと見なされます。*/
   size_t disconnect;

   /* これは、現在のメモリ領域のサイズを示します。
    * スタート＋ディスコネクトが適用された後、アドレスがこれよりも高い場合は、アドレスの最上位ビットがクリアされます。
    * 
    * それでもアドレスが高すぎる場合は、次に高いビットがクリアされます。
    * 0でも構いませんが、その場合は無限大とみなされます（「select」と「disconnect」によって制限されます）。*/
   size_t len;

   /* エミュレートされたアドレスから物理的なアドレスに移動するには、以下の順序が適用されます。
    * start "を引き、"disconnect "を取り、"len "を適用し、"offset "を加える。*/

   /* アドレス空間名は、a-zA-Z0-9_-のみで構成され、可能な限り短くし（最大長は8＋NUL）、
    * 他のアドレス空間と最後に1つ以上の0-9A-Fを加えてはならない。
    * ただし、同一アドレス空間に複数のメモリ記述子を置くことは可能で、
    * アドレス空間名は空でもよいとされています。NULLは空として扱われます。
    *
    * アドレス空間名は大文字と小文字を区別しますが、可能な限り小文字を避けてください。
    * 同一のポインタが複数のアドレス空間に存在する場合があります。
    *
    * 例:
    * blank+blank - 有効 (複数のものが同じ名前空間にマップされる可能性があります) 
    * 'Sp'+'Sp' - 有効 (複数のものが同じ名前空間にマップされる可能性があります) 
    * 'A'+'B' - 有効 (お互いの prefix ではありません) 
    * 'S'+blank - 有効 ('S' は 0-9A-F にありません) 
    * 'a'+blank - 有効 ('a' は 0-9A-F にありません) 
    * 'a'+'A' - 有効 (お互いの prefix ではありません) 
    * 'AR'+blank - 有効 ('R' は 0-9A-F にありません) 
    * 'ARB'+blank - 有効（名前空間「AR」が存在しないため、Bはアドレスの一部にもなり得ない）。
    * blank+'B' - 無効　B1234 がどのアドレス空間を参照するかは曖昧
    * 長さはその目的には使用できません。
    * フロントエンドは、区切り文字なしで任意のデータをアドレスに追加したい場合があります。 */
   const char *addrspace;

   /* TODO: When finalizing this one, add a description field, which should be
    * "WRAM" or something roughly equally long. */

   /* TODO: When finalizing this one, replace 'select' with 'limit', which tells
    * which bits can vary and still refer to the same address (limit = ~select).
    * TODO: limit? range? vary? something else? */

   /* TODO: When finalizing this one, if 'len' is above what 'select' (or
    * 'limit') allows, it's bankswitched. Bankswitched data must have both 'len'
    * and 'select' != 0, and the mappings don't tell how the system switches the
    * banks. */

   /* TODO: When finalizing this one, fix the 'len' bit removal order.
    * For len=0x1800, pointer 0x1C00 should go to 0x1400, not 0x0C00.
    * Algorithm: Take bits highest to lowest, but if it goes above len, clear
    * the most recent addition and continue on the next bit.
    * TODO: Can the above be optimized? Is "remove the lowest bit set in both
    * pointer and 'len'" equivalent? */

   /* TODO: Some emulators (MAME?) emulate big endian systems by only accessing
    * the emulated memory in 32-bit chunks, native endian. But that's nothing
    * compared to Darek Mihocka <http://www.emulators.com/docs/nx07_vm101.htm>
    * (section Emulation 103 - Nearly Free Byte Reversal) - he flips the ENTIRE
    * RAM backwards! I'll want to represent both of those, via some flags.
    *
    * I suspect MAME either didn't think of that idea, or don't want the #ifdef.
    * Not sure which, nor do I really care. */

   /* TODO: Some of those flags are unused and/or don't really make sense. Clean
    * them up. */
};

/* フロントエンドは、ある名前空間における'start'+'select'の最大値を使って、アドレス空間の大きさを推測することがあります。
 *
 * アドレス空間がそれよりも大きい場合は、.ptr=NULLのマッピングを配列の最後に置き、
 * アドレス空間が大きい限り、.selectをすべて1に設定する必要があります。
 *
 * サンプル記述子（.ptrとフラグのRETRO_MEMFLAG_を除いたもの:
 * SNES WRAM:
 * .start=0x7E0000, .len=0x20000
 * (ほとんどの場合、これはROMの前にマッピングされなければならないことに注意してください。
 * ROMマッパーの中には、$7E0000、または少なくとも$7E8000を要求しようとするものがあります。)
 * SNES SPC700 RAM:
 * .addrspace="S", .len=0x10000
 * SNES WRAM mirrors:
 * .flags=MIRROR, .start=0x000000, .select=0xC0E000, .len=0x2000
 * .flags=MIRROR, .start=0x800000, .select=0xC0E000, .len=0x2000
 * SNES WRAM mirrors, alternate equivalent descriptor:
 * .flags=MIRROR, .select=0x40E000, .disconnect=~0x1FFF
 * (Various similar constructions can be created by combining parts of
 * the above two.)
 * SNES LoROM (512KB, mirrored a couple of times):
 * .flags=CONST, .start=0x008000, .select=0x408000, .disconnect=0x8000, .len=512*1024
 * .flags=CONST, .start=0x400000, .select=0x400000, .disconnect=0x8000, .len=512*1024
 * SNES HiROM (4MB):
 * .flags=CONST,                 .start=0x400000, .select=0x400000, .len=4*1024*1024
 * .flags=CONST, .offset=0x8000, .start=0x008000, .select=0x408000, .len=4*1024*1024
 * SNES ExHiROM (8MB):
 * .flags=CONST, .offset=0,                  .start=0xC00000, .select=0xC00000, .len=4*1024*1024
 * .flags=CONST, .offset=4*1024*1024,        .start=0x400000, .select=0xC00000, .len=4*1024*1024
 * .flags=CONST, .offset=0x8000,             .start=0x808000, .select=0xC08000, .len=4*1024*1024
 * .flags=CONST, .offset=4*1024*1024+0x8000, .start=0x008000, .select=0xC08000, .len=4*1024*1024
 * Clarify the size of the address space:
 * .ptr=NULL, .select=0xFFFFFF
 * .len can be implied by .select in many of them, but was included for clarity.
 */

struct retro_memory_map
{
   const struct retro_memory_descriptor *descriptors;
   unsigned num_descriptors;
};

struct retro_controller_description
{
   /* コントローラの人間が読める説明。
    *汎用の入力デバイスタイプを使用している場合でも、コアが使用する特定のデバイスタイプに設定することができます。 */
   const char *desc;

   /* retro_set_controller_port_device()に渡されるデバイスタイプです。
    * デバイスタイプが一般的な入力デバイスタイプのサブクラスの場合、RETRO_DEVICE_SUBCLASS マクロを使って ID を作成します。

    * 例：RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_JOYPAD, 1)。 */
   unsigned id;
};

struct retro_controller_info
{
   const struct retro_controller_description *types;
   unsigned num_types;
};

struct retro_subsystem_memory_info
{
   /* "psram" など、メモリタイプに関連する拡張子。 */
   const char *extension;

   /* retro_get_memory()のためのメモリタイプです。
    * 標準化されたlibretroのメモリタイプとの衝突を避けるため、少なくとも0x100でなければなりません。 */
   unsigned type;
};

struct retro_subsystem_rom_info
{
   /* どのような内容なのかを記述します（SGB BIOS、GB ROMなど）。 */
   const char *desc;

   /* retro_get_system_info()と同じ定義です。 */
   const char *valid_extensions;

   /* retro_get_system_info()と同じ定義です。 */
   bool need_fullpath;

   /* retro_get_system_info()と同じ定義です。 */
   bool block_extract;

   /* これは、ゲームをロードするためにコンテンツが必要な場合に設定されます。
    * これが false に設定されている場合は、ゼロになった retro_game_info を渡すことができます。 */
   bool required;

   /* コンテンツには、複数の永続メモリ タイプを関連付けることができます (retro_get_memory())。 */
   const struct retro_subsystem_memory_info *memory;
   unsigned num_memory;
};

struct retro_subsystem_info
{
   /* サブシステムのタイプを表す人間が読める文字列 例："Super GameBoy" */
   const char *desc;

   /* サブシステム・タイプのためのコンピュータに適した短い文字列の識別子です。
    * この名前は[a-z]でなければなりません。
    * 例：Descが "Super GameBoy "の場合、これは "sgb "となります。
    * この識別子は、コマンドライン・インターフェースなどに使用できます。
    */
   const char *ident;

   /* 各コンテンツファイルの情報です。最初のエントリは、フロントエンドの目的上、「最も重要な」コンテンツであると想定されます。
    * 例：Super GameBoyの場合、最初のコンテンツはGameBoy ROMであるべきです。これはユーザーにとって最も「重要な」コンテンツだからです。
    * フロントエンドが使用するコンテンツに基づいて新しいファイルパスを作成する場合（例：savestates）、
    * そのために最初のROMのパスを使用する必要があります。 */
   const struct retro_subsystem_rom_info *roms;

   /* サブシステムに関連するコンテンツファイルの数。 */
   unsigned num_roms;

   /* retro_load_game_special()に渡される型。 */
   unsigned id;
};

typedef void (RETRO_CALLCONV *retro_proc_address_t)(void);

/* libretro APIの拡張関数です。
 * (今のところありません)。
 *
 * libretro coreからシンボルを取得します。
 * コアは libretro API の実際の拡張機能であるシンボルのみを返すべきです。
 * 
 * フロントエンドは、標準的な libretro エントリーポイント (スタティックリンクや dlsym) の
 * シンボルを取得するために、この機能を使ってはいけません。
 * 
 * 例えば、void retro_foo(void); が存在する場合、シンボルは "retro_foo" と呼ばれなければなりません。
 * 返された関数ポインタは、対応する型にキャストされなければなりません。
 */
typedef retro_proc_address_t (RETRO_CALLCONV *retro_get_proc_address_t)(const char *sym);

struct retro_get_proc_address_interface
{
   retro_get_proc_address_t get_proc_address;
};

enum retro_log_level
{
   RETRO_LOG_DEBUG = 0,
   RETRO_LOG_INFO,
   RETRO_LOG_WARN,
   RETRO_LOG_ERROR,

   RETRO_LOG_DUMMY = INT_MAX
};

/* ロギング機能。ログレベルの引数も取ります。 */
typedef void (RETRO_CALLCONV *retro_log_printf_t)(enum retro_log_level level,
      const char *fmt, ...);

struct retro_log_callback
{
   retro_log_printf_t log;
};

/* パフォーマンスに関する機能 */

/* SIMD CPU機能のID値 */
#define RETRO_SIMD_SSE      (1 << 0)
#define RETRO_SIMD_SSE2     (1 << 1)
#define RETRO_SIMD_VMX      (1 << 2)
#define RETRO_SIMD_VMX128   (1 << 3)
#define RETRO_SIMD_AVX      (1 << 4)
#define RETRO_SIMD_NEON     (1 << 5)
#define RETRO_SIMD_SSE3     (1 << 6)
#define RETRO_SIMD_SSSE3    (1 << 7)
#define RETRO_SIMD_MMX      (1 << 8)
#define RETRO_SIMD_MMXEXT   (1 << 9)
#define RETRO_SIMD_SSE4     (1 << 10)
#define RETRO_SIMD_SSE42    (1 << 11)
#define RETRO_SIMD_AVX2     (1 << 12)
#define RETRO_SIMD_VFPU     (1 << 13)
#define RETRO_SIMD_PS       (1 << 14)
#define RETRO_SIMD_AES      (1 << 15)
#define RETRO_SIMD_VFPV3    (1 << 16)
#define RETRO_SIMD_VFPV4    (1 << 17)
#define RETRO_SIMD_POPCNT   (1 << 18)
#define RETRO_SIMD_MOVBE    (1 << 19)
#define RETRO_SIMD_CMOV     (1 << 20)
#define RETRO_SIMD_ASIMD    (1 << 21)

typedef uint64_t retro_perf_tick_t;
typedef int64_t retro_time_t;

struct retro_perf_counter
{
   const char *ident;
   retro_perf_tick_t start;
   retro_perf_tick_t total;
   retro_perf_tick_t call_cnt;

   bool registered;
};

/* 現在の時刻をマイクロ秒単位で返します。
 * 利用可能な最も正確なタイマーを使用するようにしています。
 */
typedef retro_time_t (RETRO_CALLCONV *retro_perf_get_time_usec_t)(void);

/* 単純なカウンタ。通常はナノ秒ですが、CPUサイクルの場合もあります。
 * 必要に応じて直接使用することができます（より洗練されたパフォーマンスカウンタシステムを作成する場合）。
 * */
typedef retro_perf_tick_t (RETRO_CALLCONV *retro_perf_get_counter_t)(void);

/* 検出されたCPU機能（RETRO_SIMD_*）のビットマスクを返します。 */
typedef uint64_t (RETRO_CALLCONV *retro_get_cpu_features_t)(void);

/* フロントエンドにパフォーマンスカウンターの状態を記録・表示するよう求めます。
 * パフォーマンスカウンターはいつでも手動で覗くことができます。
 */
typedef void (RETRO_CALLCONV *retro_perf_log_t)(void);

/* パフォーマンスカウンターを登録します。
 * identフィールドには離散的な値が設定され、retro_perf_counterの他の値は0である必要があります。
 * 登録は複数回呼び出すことができます。
 * フロントエンドへの重複した呼び出しを避けるために、登録されたフィールドを最初にチェックすることができます。 */
typedef void (RETRO_CALLCONV *retro_perf_register_t)(struct retro_perf_counter *counter);

/* 登録されているカウンタを起動します。 */
typedef void (RETRO_CALLCONV *retro_perf_start_t)(struct retro_perf_counter *counter);

/* 登録されているカウンターを停止します。 */
typedef void (RETRO_CALLCONV *retro_perf_stop_t)(struct retro_perf_counter *counter);

/* 利便性を高めるために、register、start、stopをマクロで囲むと便利です。
 * 例:
 * #ifdef LOG_PERFORMANCE
 * #define RETRO_PERFORMANCE_INIT(perf_cb, name) static struct retro_perf_counter name = {#name}; if (!name.registered) perf_cb.perf_register(&(name))
 * #define RETRO_PERFORMANCE_START(perf_cb, name) perf_cb.perf_start(&(name))
 * #define RETRO_PERFORMANCE_STOP(perf_cb, name) perf_cb.perf_stop(&(name))
 * #else
 * ... Blank macros ...
 * #endif
 *
 * これらは、コードスニペットを中心とした機能の途中で使用することができます。
 *
 * extern struct retro_perf_callback perf_cb;  * Somewhere in the core.
 *
 * void do_some_heavy_work(void)
 * {
 *    RETRO_PERFORMANCE_INIT(cb, work_1;
 *    RETRO_PERFORMANCE_START(cb, work_1);
 *    heavy_work_1();
 *    RETRO_PERFORMANCE_STOP(cb, work_1);
 *
 *    RETRO_PERFORMANCE_INIT(cb, work_2);
 *    RETRO_PERFORMANCE_START(cb, work_2);
 *    heavy_work_2();
 *    RETRO_PERFORMANCE_STOP(cb, work_2);
 * }
 *
 * void retro_deinit(void)
 * {
 *    perf_cb.perf_log();  * Log all perf counters here for example.
 * }
 */

struct retro_perf_callback
{
   retro_perf_get_time_usec_t    get_time_usec;
   retro_get_cpu_features_t      get_cpu_features;

   retro_perf_get_counter_t      get_perf_counter;
   retro_perf_register_t         perf_register;
   retro_perf_start_t            perf_start;
   retro_perf_stop_t             perf_stop;
   retro_perf_log_t              perf_log;
};

/* FIXME: センサー API を文書化し、動作を解決します。
 * それまでは試験運用となります。 
 */
enum retro_sensor_action
{
   RETRO_SENSOR_ACCELEROMETER_ENABLE = 0,
   RETRO_SENSOR_ACCELEROMETER_DISABLE,
   RETRO_SENSOR_GYROSCOPE_ENABLE,
   RETRO_SENSOR_GYROSCOPE_DISABLE,
   RETRO_SENSOR_ILLUMINANCE_ENABLE,
   RETRO_SENSOR_ILLUMINANCE_DISABLE,

   RETRO_SENSOR_DUMMY = INT_MAX
};

/* SENSOR タイプの ID 値。 */
#define RETRO_SENSOR_ACCELEROMETER_X 0
#define RETRO_SENSOR_ACCELEROMETER_Y 1
#define RETRO_SENSOR_ACCELEROMETER_Z 2
#define RETRO_SENSOR_GYROSCOPE_X 3
#define RETRO_SENSOR_GYROSCOPE_Y 4
#define RETRO_SENSOR_GYROSCOPE_Z 5
#define RETRO_SENSOR_ILLUMINANCE 6

typedef bool (RETRO_CALLCONV *retro_set_sensor_state_t)(unsigned port,
      enum retro_sensor_action action, unsigned rate);

typedef float (RETRO_CALLCONV *retro_sensor_get_input_t)(unsigned port, unsigned id);

struct retro_sensor_interface
{
   retro_set_sensor_state_t set_sensor_state;
   retro_sensor_get_input_t get_sensor_input;
};

enum retro_camera_buffer
{
   RETRO_CAMERA_BUFFER_OPENGL_TEXTURE = 0,
   RETRO_CAMERA_BUFFER_RAW_FRAMEBUFFER,

   RETRO_CAMERA_BUFFER_DUMMY = INT_MAX
};

/* カメラドライバを起動します。 retro_run() でのみ呼び出すことができます。  */
typedef bool (RETRO_CALLCONV *retro_camera_start_t)(void);

/* カメラドライバを停止します。retro_run()内でのみ呼び出すことができます。 */
typedef void (RETRO_CALLCONV *retro_camera_stop_t)(void);

/* カメラドライバが初期化および/または非初期化されたときにシグナルを送るコールバックです。
 * retro_camera_start_t は、初期化されたコールバックの中で呼び出されます。
 */
typedef void (RETRO_CALLCONV *retro_camera_lifetime_status_t)(void);

/* 生のフレームバッファデータのコールバックです。 bufferはXRGB8888のバッファを指します。
 * 幅、高さ、ピッチは retro_video_refresh_t と同様です。
 * 最初のピクセルは左上の原点に相当します。
 */
typedef void (RETRO_CALLCONV *retro_camera_frame_raw_framebuffer_t)(const uint32_t *buffer,
      unsigned width, unsigned height, size_t pitch);

/* OpenGLテクスチャを使用する際のコールバックです。
 * 
 * texture_idは、カメラドライバが所有するテクスチャです。
 * テクスチャのフィルタリングやクランプなどを除き、その状態や内容は不変と考えてください。
 * 
 * texture_target は、GLテクスチャのテクスチャターゲットです。
 * GL_TEXTURE_2D、GL_TEXTURE_RECTANGLEなどのほか、拡張機能によってはそれ以上のものもあります。
 * 
 * affine テクスチャ座標にアフィン変換を適用するために使用される、パックされた3x3列長のマトリクスを指します。
 * (affine_matrix * vec3(coord_x, coord_y, 1.0)) 変換後、
 * 正規化されたテクスチャの座標(0, 0)は左下、(1, 1)は右上(RECTANGLEの場合は(width, height))になるはずです。
 * 
 * API定義でgl.hに依存しないように、ここではGL固有のtypedefは避けています。
 */
typedef void (RETRO_CALLCONV *retro_camera_frame_opengl_texture_t)(unsigned texture_id,
      unsigned texture_target, const float *affine);

struct retro_camera_callback
{
   /* libretro coreで設定します。
    * ビットマスクの例： caps = (1 << RETRO_CAMERA_BUFFER_OPENGL_TEXTURE) | (1 << RETRO_CAMERA_BUFFER_RAW_FRAMEBUFFER).
    */
   uint64_t caps;

   /* カメラに要求される解像度。あくまでもヒントとして使用します。 */
   unsigned width;
   unsigned height;

   /* フロントエンドで設定します。 */
   retro_camera_start_t start;
   retro_camera_stop_t stop;

   /* 生のフレームバッファ・コールバックが使用される場合、libretro coreによって設定されます。. */
   retro_camera_frame_raw_framebuffer_t frame_raw_framebuffer;

   /* OpenGLテクスチャコールバックを使用する場合、libretro coreによって設定されます。 */
   retro_camera_frame_opengl_texture_t frame_opengl_texture;

   /* libretro coreにより設定されます。カメラドライバが初期化され、起動可能な状態になった後に呼び出されます。
    * NULLを指定することもでき、その場合はこのコールバックは呼ばれません。
    */
   retro_camera_lifetime_status_t initialized;

   /* libretro coreにより設定されます。カメラドライバが初期化される直前に呼び出されます．
    * NULLを指定することもでき、その場合はこのコールバックは呼ばれません。
    */
   retro_camera_lifetime_status_t deinitialized;
};

/* 位置情報データを更新する時間や距離の間隔を設定します。
 * 
 * すべてのロケーションベースの実装との互換性を確保するため、interval_msとinterval_distanceの両方に値を指定する必要があります。
 * 
 * interval_msは、ミリ秒単位で表される間隔です。
 * interval_distanceは距離の間隔をメートル単位で表したものです。
 */
typedef void (RETRO_CALLCONV *retro_location_set_interval_t)(unsigned interval_ms,
      unsigned interval_distance);

/* 位置情報サービスを開始します。デバイスは、定期的な間隔（retro_location_set_interval_t で定義される）で、
 * 現在の位置の変更をリッスンし始めます。*/
typedef bool (RETRO_CALLCONV *retro_location_start_t)(void);

/* 位置情報サービスを停止します。デバイスが現在の位置情報の変更を聞くのを停止します。 */
typedef void (RETRO_CALLCONV *retro_location_stop_t)(void);

/* 現在のロケーションの位置を取得します。前回の更新以降、新たな位置情報の更新が行われていない場合は、パラメータに0が設定されます。*/
typedef bool (RETRO_CALLCONV *retro_location_get_position_t)(double *lat, double *lon,
      double *horiz_accuracy, double *vert_accuracy);

/* ロケーションドライバーが初期化されたとき、または初期化解除されたときにシグナルを送るコールバックです。
 * retro_location_start_t は、初期化されたコールバックの中で呼び出すことができます。
 */
typedef void (RETRO_CALLCONV *retro_location_lifetime_status_t)(void);

struct retro_location_callback
{
   retro_location_start_t         start;
   retro_location_stop_t          stop;
   retro_location_get_position_t  get_position;
   retro_location_set_interval_t  set_interval;

   retro_location_lifetime_status_t initialized;
   retro_location_lifetime_status_t deinitialized;
};

enum retro_rumble_effect
{
   RETRO_RUMBLE_STRONG = 0,
   RETRO_RUMBLE_WEAK = 1,

   RETRO_RUMBLE_DUMMY = INT_MAX
};

/* ポート'port'に接続されたジョイパッドのランブル状態を設定します。
 * ランブルの効果は独立して制御され、例えば強いランブルを設定しても弱いランブルが上書きされることはありません。
 * 強さは[0, 0xffff]の範囲で設定できます。
 * 
 * ランブル状態の要求が満たされていれば、trueを返します。
 * 最初の retro_run() の前にこれを呼び出すと、おそらく false を返す。 */
typedef bool (RETRO_CALLCONV *retro_set_rumble_state_t)(unsigned port,
      enum retro_rumble_effect effect, uint16_t strength);

struct retro_rumble_interface
{
   retro_set_rumble_state_t set_rumble_state;
};

/* libretroにオーディオデータの書き込みを通知します。 */
typedef void (RETRO_CALLCONV *retro_audio_callback_t)(void);

/* True: フロントエンドのオーディオドライバーはアクティブで、コールバックが定期的に呼び出されることが期待されています。
 * False: フロントエンドのオーディオドライバーが一時停止しているか、非アクティブになっています。
 * set_stateがtrueで呼び出されるまで、オーディオコールバックは呼び出されません。
 * 初期状態はfalse（非アクティブ）です。
 */
typedef void (RETRO_CALLCONV *retro_audio_set_state_callback_t)(bool enabled);

struct retro_audio_callback
{
   retro_audio_callback_t callback;
   retro_audio_set_state_callback_t set_state;
};

/* retro_run()の最後の呼び出しから費やされた時間をマイクロ秒単位で libretro コアに通知する。
 * 
 * 毎フレーム、retro_run()の直前に呼び出されます。
 * フロントエンドは、早送り、スローモーション、フレームステッピングのような
 * ケースをサポートするために、タイミングを改ざんすることができます。
 * 
 * これらのシナリオでは、参照フレームの時間値が使用されます。 */
typedef int64_t retro_usec_t;
typedef void (RETRO_CALLCONV *retro_frame_time_callback_t)(retro_usec_t usec);
struct retro_frame_time_callback
{
   retro_frame_time_callback_t callback;
   /* 1フレームの時間を表します。1000000 / fps として計算されますが、
    * フレームステッピングなどが正確に行われるように、実装では丸めを解決します。 */
   retro_usec_t reference;
};

/* フロントエンドのオーディオバッファの現在の占有率を、libretroコアに通知します。
 *
 * - active: オーディオバッファが現在使用されている場合は「true」、
 *           フロントエンドでオーディオが無効になっている場合は、「false」になります。
 *
 * - occupancy: [0,100]の範囲の値で、オーディオバッファの占有率に対応しています。
 *
 * - underrun_likely: フロントエンドが次のフレーム中に
 *                    オーディオ バッファ アンダーランを予期している場合は「true」 
 *                    (コアがフレーム スキップを試行する必要があることを示します) 
 *
 * これは、毎フレーム retro_run() の直前に呼び出されます。 */
typedef void (RETRO_CALLCONV *retro_audio_buffer_status_callback_t)(
      bool active, unsigned occupancy, bool underrun_likely);
struct retro_audio_buffer_status_callback
{
   retro_audio_buffer_status_callback_t callback;
};

/* ハードウェアにレンダリングする場合は、これを retro_video_refresh_t に渡します。
 * retro_video_refresh_t に NULL を渡すことは、通常どおりフレームの複製です。 
 * */
#define RETRO_HW_FRAME_BUFFER_VALID ((void*)-1)

/* 現在の HW コンテキストを無効にします。
 * GL 状態はすべて失われるため、明示的に初期化解除してはなりません。
 * libretro コアで明示的な初期化解除が必要な場合は、 context_destroy コールバックを実装する必要があります。
 * 呼び出された場合、すべての GPU リソースを再初期化する必要があります。
 * 通常、フロントエンドがビデオ ドライバーを再初期化するときに呼び出されます。
 * ビデオ ドライバーが初期化されたときにも呼び出され、libretro コアがリソースを初期化できるようにします。 
 */
typedef void (RETRO_CALLCONV *retro_hw_context_reset_t)(void);

/* レンダリングされる現在のフレームバッファを取得します。
 * フレームごとに変わる可能性があります。
 */
typedef uintptr_t (RETRO_CALLCONV *retro_hw_get_current_framebuffer_t)(void);

/* HWコンテクストからシンボルを取得します。 */
typedef retro_proc_address_t (RETRO_CALLCONV *retro_hw_get_proc_address_t)(const char *sym);

enum retro_hw_context_type
{
   RETRO_HW_CONTEXT_NONE             = 0,
   /* OpenGL 2.x. ドライバーは、最新の互換性コンテキストの使用を選択できます。 */
   RETRO_HW_CONTEXT_OPENGL           = 1,
   /* OpenGL ES 2.0. */
   RETRO_HW_CONTEXT_OPENGLES2        = 2,
   /* モダンデスクトップコアGLのコンテキスト。
    * GLのバージョンを設定するには、version_major/version_minorフィールドを使用します。 */
   RETRO_HW_CONTEXT_OPENGL_CORE      = 3,
   /* OpenGL ES 3.0 */
   RETRO_HW_CONTEXT_OPENGLES3        = 4,
   /* OpenGL ES 3.1+. version_major/version_minorを設定します。
    * GLES2およびGLES3では、対応するenumを直接使用してください。*/
   RETRO_HW_CONTEXT_OPENGLES_VERSION = 5,

   /* Vulkan, RETRO_ENVIRONMENT_GET_HW_RENDER_INTERFACE を参照してください。 */
   RETRO_HW_CONTEXT_VULKAN           = 6,

   /* Direct3D, RETRO_ENVIRONMENT_GET_HW_RENDER_INTERFACEで返される
    * インターフェースのタイプを選択するためにversion_majorを設定します。 */
   RETRO_HW_CONTEXT_DIRECT3D         = 7,

   RETRO_HW_CONTEXT_DUMMY = INT_MAX
};

struct retro_hw_render_callback
{
   /* どのAPIを使用するか。libretro coreで設定します。 */
   enum retro_hw_context_type context_type;

   /* コンテキストが作成されたとき、またはリセットされたときに呼び出されます。
    * OpenGLのコンテキストはcontext_reset()が呼ばれた後にのみ有効になります。
    * 
    * context_resetが呼ばれると、libretro実装のOpenGLリソースが無効になることが保証されます。
    * 
    * アプリケーションのライフサイクルの中で、context_resetが複数回呼ばれる可能性があります。
    * context_resetが何の通知もなく呼び出された場合(context_destroy)、
    * OpenGLのコンテキストは失われているので、古いリソースを "free "しようとせずに、リソースを再作成する必要があります。
    */
   retro_hw_context_reset_t context_reset;

   /* フロントエンドで設定します。
    * TODO: これはかなり時代遅れです。
    * フロントエンドは事前に割り当てられたフレームバッファを提供すべきではありません。 */
   retro_hw_get_current_framebuffer_t get_current_framebuffer;

   /* フロントエンドで設定します。
    * WindowsのglClearを含む、すべての関連する関数を返すことができます。 */
   retro_hw_get_proc_address_t get_proc_address;

   /* レンダリングバッファに深度成分を付加するかどうかを設定します。
    * TODO: 時代遅れ。 */
   bool depth;

   /* ステンシル・バッファを添付するかどうかを設定します。
    * TODO: 時代遅れ。 */
   bool stencil;

   /* depthとstencilがtrueの場合、パックされた24/8バッファが追加されます。
    * ステンシルを付けただけでは無効で、無視されます。 */

   /* 従来の左下の原点規則を使用します。falseを設定すると、標準的なlibretroの左上原点のセマンティクスが使用されます。
    * TODO: GL専用インターフェースに移行 */
   bool bottom_left_origin;

   /* GLのコアコンテクストまたは GLES 3.1+ のメジャーバージョン番号。 */
   unsigned version_major;

   /* GLのコアコンテクストまたは GLES 3.1+ のマイナーバージョン番号。 */
   unsigned version_minor;

   /* 「true」の場合、フロントエンドはフルスクリーンの切り替えなどのシナリオで、
    * コンテキストのリセットを避けるために非常に多くのことを行うでしょう。
    * TODO: 時代遅れ？多分、フロントエンドは常にこのように仮定すべきでしょう ...
    */
   bool cache_context;

   /* コンテキストが復元不可能なほど失われた場合など、極端な状況ではリセットコールバックがまだ呼ばれることがあります。
    * 
    * 最適な安定性を得るためには、これを false に設定し、いつでもコンテキストをリセットできるようにします。
    */

   /* コンテキストがフロントエンドによって制御された方法で破壊される前に呼び出されるコールバックです。*/
   retro_hw_context_reset_t context_destroy;

   /* このステップでは、OpenGLリソースをきれいに初期化することができます。
    * context_destroyをNULLに設定すると、リソースは何の通知もなく破壊される。
    * 
    * context_destroyがNULLでない場合でも、context_resetがdestroyの通知なしに呼ばれることがある。
    * これは外部要因によってコンテキストが失われた場合に起こる（GL_ARB_robustnessで通知されるような場合）。
    * 
    * この場合、コンテキストはすでに失われているとみなされ、
    * libretroの実装はその後のcontext_resetでOpenGLリソースを解放しようとしてはならない。
    */

   /* デバッグコンテキストを作成します。 */
   bool debug_context;
};

/* RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK で渡されるコールバックタイプ。
 * キーボードイベントに応じてフロントエンドから呼び出されます。
 * キーが押されている場合は down が、離されている場合は false が設定されます。
 * keycodeは、charのRETROK値です。
 * characterは、押されたキーのテキスト文字です。(UTF-32)です。
 * key_modifiersは、RETROKMODの値を組み合わせたものです。
 * 
 * 押された/キーコードの状態は、文字とは関係ありません。
 * また、1つのキープレスから複数の文字が生成されることもあります。
 * キーコードイベントは、文字イベントとは別個に扱われるべきである。
 * しかし、可能であれば、フロントエンドはこれらを同期させるようにすべきである。
 * 文字だけが投稿された場合、キーコードはRETROK_UNKNOWNでなければなりません。
 * 
 * 同様に、キーコードイベントだけが発生して対応する文字がない場合、文字は0にすべきです。
 */
typedef void (RETRO_CALLCONV *retro_keyboard_event_t)(bool down, unsigned keycode,
      uint32_t character, uint16_t key_modifiers);

struct retro_keyboard_callback
{
   retro_keyboard_event_t callback;
};

/* RETRO_ENVIRONMENT_SET_DISK_CONTROL_INTERFACE および
 * RETRO_ENVIRONMENT_SET_DISK_CONTROL_EXT_INTERFACE のコールバックです。
 * 実行時に複数のディスクイメージを交換することができる実装に設定する必要があります。
 * 
 * 実装が自動的にこれを行うことができるなら、そうするように努力すべきである。
 * しかし、ユーザが手動で交換しなければならない場合もあります。
 * 
 * 概要 ディスクイメージを交換するには、set_eject_state(true)でディスクイメージをイジェクトする。
 * set_image_index(index)でディスクのインデックスを設定します。
 * set_eject_state(false)でディスクを再び挿入します。
 */

/* ejected が true の場合、仮想ディスクトレイを "eject" します。
 * ejected時にディスクイメージのインデックスを設定できます。
 */
typedef bool (RETRO_CALLCONV *retro_set_eject_state_t)(bool ejected);

/* 現在のイジェクト状態を取得します。初期状態は「not ejected」です。 */
typedef bool (RETRO_CALLCONV *retro_get_eject_state_t)(void);

/* 現在のディスクのインデックスを取得します。最初のディスクはインデックス0です。
 * 返り値が >= get_num_images() の場合、現在挿入されているディスクはありません。
 */
typedef unsigned (RETRO_CALLCONV *retro_get_image_index_t)(void);

/* image のインデックスを設定します。ディスクがイジェクトされたときにのみ呼び出されます。
 * この実装では、index >= get_num_images() を使用して「ディスクなし」を設定することをサポートしています。
 */
typedef bool (RETRO_CALLCONV *retro_set_image_index_t)(unsigned index);

/* 使用可能な image の総数を取得します。 */
typedef unsigned (RETRO_CALLCONV *retro_get_num_images_t)(void);

struct retro_game_info;

/* index に関連付けられたディスクイメージを交換する。
 * infoに渡す引数はretro_load_game()と同じ要件である。
 * この関数を呼び出す際には、仮想ディスクトレイが取り出されていなければならない。
 * 
 * info = NULL でディスクイメージを交換すると、内部リストからディスクイメージが削除されます。
 * その結果、get_image_index()の呼び出しが変わることがある。
 * 
 * 例：replace_image_index(1, NULL)で、以前のget_image_index()は4を返していました。
 * インデックス1は削除され、新しいインデックスは3となります。
 */
typedef bool (RETRO_CALLCONV *retro_replace_image_index_t)(unsigned index,
      const struct retro_game_info *info);

/* 内部のディスクリストに新しい有効なインデックス(get_num_images())を追加します。
 * get_num_images()の戻り値が1増加します。
 * replace_image_index でディスクイメージを設定するまで、
 * このイメージインデックスは使用できません。*/
typedef bool (RETRO_CALLCONV *retro_add_image_index_t)(void);

/* core_load_game()を呼び出す際に、ドライブに挿入する初期 image を設定します。
 * コンテンツをロードするときに初期インデックスを渡すことができないので（これは大きなAPIの変更を必要とします）、
 * これはコアのretro_load_game()/retro_load_game_special()の実装を呼び出す前に*フロントエンドによって設定されます。
 * したがって、コアは index/path 値をキャッシュして、retro_load_game()/retro_load_game_special() の中でそれらを処理しなければなりません。
 * - index' が無効な場合 (index >= get_num_images())、 コアは設定された値を無視し、代わりに 0 を使用しなければなりません。
 * - path' は純粋にエラーチェックのために使用されます。
 *   つまり、コンテンツが読み込まれる際に、コアは 'index' で
 *   指定されたディスクが指定されたファイルパスを持っているかどうかを確認する必要があります。
 *   これは、例えばユーザーが既存のM3Uプレイリストを変更する際に、
 *   間違った image が自動的に選択されないようにするためです。
 *   set_initial_image()はコンテンツをロードする前に呼ばれなければならないので、コアにこの処理を任せなければなりません。
 *   セットされたパスとコンテンツのパスが一致しない場合、コアはセットされた 'index' の値を無視し、代わりに 0 を使うべきです。
 * index または 'path' が無効な場合、またはコアがこの機能をサポートしていない場合は 'false' を返します。
 */
typedef bool (RETRO_CALLCONV *retro_set_initial_image_t)(unsigned index, const char *path);

/* 指定されたディスクイメージファイルのパスを取得します。
 * インデックスが無効な場合(index >= get_num_images())や、パスが利用できない場合は、falseを返します。
 */
typedef bool (RETRO_CALLCONV *retro_get_image_path_t)(unsigned index, char *path, size_t len);

/* 指定されたディスクイメージファイルに対して、コアが提供する「ラベル」を取得します。
 * 例えば、フロッピーディスクベースのコンテンツを実行しているコアでは、
 * セーブディスク、データディスク、レベルディスクなどを
 * ゲーム内のディスク変更プロンプトに対応した名前で一意にラベル付けすることができます
 * （そうすることで、フロントエンドは「無意味な」ディスクインデックス値よりも優れたユーザーガイダンスを提供することができます）。
 * インデックスが無効な場合 (index >= get_num_images())、またはラベルが利用できない場合には 'false' を返します。
 */
typedef bool (RETRO_CALLCONV *retro_get_image_label_t)(unsigned index, char *label, size_t len);

struct retro_disk_control_callback
{
   retro_set_eject_state_t set_eject_state;
   retro_get_eject_state_t get_eject_state;

   retro_get_image_index_t get_image_index;
   retro_set_image_index_t set_image_index;
   retro_get_num_images_t  get_num_images;

   retro_replace_image_index_t replace_image_index;
   retro_add_image_index_t add_image_index;
};

struct retro_disk_control_ext_callback
{
   retro_set_eject_state_t set_eject_state;
   retro_get_eject_state_t get_eject_state;

   retro_get_image_index_t get_image_index;
   retro_set_image_index_t set_image_index;
   retro_get_num_images_t  get_num_images;

   retro_replace_image_index_t replace_image_index;
   retro_add_image_index_t add_image_index;

   /* NOTE: set_initial_image()とget_image_path()の両方が実装されている場合にのみ、
    * フロントエンドが最後に使用したディスクインデックスの記録/復元を試みるようにしました。 */
   retro_set_initial_image_t set_initial_image; /* オプション - NULL の場合あり  */

   retro_get_image_path_t get_image_path;       /* オプション - NULL の場合あり  */
   retro_get_image_label_t get_image_label;     /* オプション - NULL の場合あり  */
};

enum retro_pixel_format
{
   /* 0RGB1555、ネイティブエンディアン。
    * 0ビットは0に設定する必要があります。
    * このピクセルフォーマットは、互換性の観点からのみデフォルトで設定されています。
    * 15/16ビットのピクセルフォーマットが必要な場合は、RGB565の使用を検討してください。 */
   RETRO_PIXEL_FORMAT_0RGB1555 = 0,

   /* XRGB8888、ネイティブエンディアン。
    * Xビットは無視されます。 */
   RETRO_PIXEL_FORMAT_XRGB8888 = 1,

   /* RGB565、ネイティブエンディアン。
    * このピクセルフォーマットは、幅広い低消費電力デバイスで
    * 一般的に利用可能なピクセルフォーマットであるため、
    * 15/16ビットフォーマットを希望する場合には、このフォーマットを使用することを推奨します。
    *
    * OpenGL ESなどのAPIでもネイティブにサポートされています。*/
   RETRO_PIXEL_FORMAT_RGB565   = 2,

   /* sizeof() == sizeof(int)であることを確認します。 */
   RETRO_PIXEL_FORMAT_UNKNOWN  = INT_MAX
};

struct retro_message
{
   const char *msg;        /* 表示されるメッセージ */
   unsigned    frames;     /* メッセージのフレーム数 */
};

enum retro_message_target
{
   RETRO_MESSAGE_TARGET_ALL = 0,
   RETRO_MESSAGE_TARGET_OSD,
   RETRO_MESSAGE_TARGET_LOG
};

enum retro_message_type
{
   RETRO_MESSAGE_TYPE_NOTIFICATION = 0,
   RETRO_MESSAGE_TYPE_NOTIFICATION_ALT,
   RETRO_MESSAGE_TYPE_STATUS,
   RETRO_MESSAGE_TYPE_PROGRESS
};

struct retro_message_ext
{
   /* 表示/ログ化されるメッセージ文字列 */
   const char *msg;
   /* OSD を対象とする場合のメッセージの持続時間 (ミリ秒単位)  */
   unsigned duration;
   /* OSD表示時のメッセージの優先順位
    * > 複数のメッセージが同時にフロントエンドに送信され、
    *   フロントエンドがそれらをすべて表示する能力を持っていない場合、
    *   *最も高い*優先値を持つメッセージが表示されるべきです。
    * > メッセージの優先度に上限はありません（unsignedデータタイプの範囲内）。
    * > 参照用フロントエンド（RetroArch）では、
    *   フロントエンドが生成する通知にも同じ優先度の値が使われており、
    *   重要度に応じて0〜3の値が割り当てられています。 */
   unsigned priority;
   /* メッセージのロギング レベル (info、warn、error など)  */
   enum retro_log_level level;
   /* メッセージの宛先。OSD、ロギングインターフェース、またはその両方 */
   enum retro_message_target target;
   /* OSD を対象とする場合のメッセージ「タイプ」 
    * > RETRO_MESSAGE_TYPE_NOTIFICATION:
    *   メッセージが、標準的なフロントエンドで生成された通知と同じ方法で処理されるべきであることを指定します。
    * > RETRO_MESSAGE_TYPE_NOTIFICATION_ALT:
    *   メッセージがユーザの注意や行動を必要とする通知であるが、
    *   標準的なフロントエンドで生成される通知とは異なる方法で表示されるべきであることを指定します。
    *   これは典型的には、（内部のフロントエンド・メッセージ・キューから独立して）
    *   直ちに表示されるべきメッセージ、および／または、
    *   フロントエンドが生成した通知と視覚的に区別されるべきメッセージに対応します。
    *   例えば、コアは、ディスク交換イベントに関連する情報をユーザーに知らせたいと思うかもしれません。
    *   この場合、フロントエンド自身が通知を提供することが期待される。
    *   コアが RETRO_MESSAGE_TYPE_NOTIFICATION タイプのメッセージを送信した場合、
    *   不快な「二重通知」が発生する可能性がある。
    *   したがって、RETRO_MESSAGE_TYPE_NOTIFICATION_ALTのメッセージは、
    *   通常の通知との視覚的な衝突が起こらないように提示されるべきである。
    * > RETRO_MESSAGE_TYPE_STATUS:
    *   メッセージが標準的な通知ではないことを示す。これは一般的に、コアの内部FPSなどの
    *   「ステータス」インジケータに対応するもので、コアの実行中に恒久的に表示されるか、
    *   ユーザの注意やアクションが必要であることを示唆しない方法で表示されることが意図されています。
    *   したがって、'Status'タイプのメッセージは、画面上の別の場所に、
    *   標準的なフロントエンドで生成された通知とRETRO_MESSAGE_TYPE_NOTIFICATION_ALTタイプの
    *   メッセージの両方と容易に区別できる方法で表示されるべきである。
    * > retro_message_type_progress:
    *   コアの内部タスクの進捗状況を報告するメッセージであることを示します。
    *   例えば、コア自身がファイルからのコンテンツの読み込みを処理する場合、
    *   これはファイルが読み込まれた割合に対応するかもしれない。
    *   また、オーディオ/ビデオ再生コアは、RETRO_MESSAGE_TYPE_PROGRESS型のメッセージを使用して、
    *   現在の再生位置をランタイムのパーセンテージで表示することができる。
    *   「Progress」タイプのメッセージは、したがって、リテラルのプログレスバーとして表示されるべきである。
    *   where:
    *   - 「retro_message_ext.msg」は進行状況バーのタイトル/ラベルです 
    *   - 「retro_message_ext.progress」はプログレスバーの長さを決定します 
    * NOTE: メッセージタイプは *hint* であり、フロントエンドはこれを無視することができます。
    *       フロントエンドが、標準的なフロントエンドが生成する通知以外の方法で
    *       メッセージを表示するサポートを欠いている場合は、 
    *       *すべての*メッセージをRETRO_MESSAGE_TYPE_NOTIFICATIONタイプとして扱います。 */
   enum retro_message_type type;
   /* OSDをターゲットにしていて、メッセージのタイプがRETRO_MESSAGE_TYPE_PROGRESSの場合のタスクの進捗状況。
    * > -1:    非計測/不確定
    * > 0-100: 現在の進捗率
    * NOTE: メッセージタイプはヒントなので、フロントエンドはプログレス値を無視しても構いません。
    * そのため、フロントエンドが生成する標準的な通知として表示されたときに、
    * メッセージの意図が明確になるように、コアはメッセージ文字列内に進捗率を含める必要があります。 */
   int8_t progress;
};

/* libretro の実装が、人間が読める文字列を使って 
 * libretro の入力バインドを内部の入力システムにマッピングする方法を説明します。
 * この文字列は、ユーザが入力を設定する際に利用できます。 */
struct retro_input_descriptor
{
   /* 与えられたパラメータと説明を関連付けます。 */
   unsigned port;
   unsigned device;
   unsigned index;
   unsigned id;

   /* パラメータの人間が読める記述。
    * ポインタは retro_unload_game() が呼ばれるまで有効でなければなりません。*/
   const char *description;
};

struct retro_system_info
{
   /* すべてのポインターはlibretroの実装が所有しており、
    * ポインターはunloadされるまで有効でなければなりません。 */

   const char *library_name;      /* ライブラリの説明的な名前。
                                   * バージョン番号などを含んではいけません。*/
   const char *library_version;   /* コアの説明バージョン。 */

   const char *valid_extensions;  /* コアが読み込むことのできるコンテンツ拡張子の一覧をパイプで区切った文字列。
                                   * 例えば、"bin|rom|iso "です。
                                   * 一般的には、拡張子をフィルタリングするためのGUIに使用されます。*/

   /* コンテンツファイルへの直接アクセスが必要なLibretroコア
    * (コンテンツファイルのパスを使って他のファイルのパスを決定するコアを含む)
    * は、need_fullpathをtrueに設定してください。
    *
    * フロントエンドがパッチなどを実行できるようにするため、
    * コアはneed_fullpathをfalseに設定するよう努めるべきです。
    *
    * need_fullpath が true で retro_load_game() が呼び出された場合:
    *    - retro_game_info::path は有効なパスを持つことが保証されています
    *    - retro_game_info::data と retro_game_info::size は無効です 
    *
    * need_fullpath が false で retro_load_game() が呼び出された場合:
    * - retro_game_info::path は NULL かもしれません
    * - retro_game_info::data と retro_game_info::size は有効であることが保証されています 
    *
    * 参照:
    *    - RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY
    *    - RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY
    */
   bool        need_fullpath;

   /* true の場合、フロントエンドは実際のコンテンツを読み込む前にアーカイブを抽出することができません。
    * zip 形式のアーカイブからゲームをロードする、ある種の libretro の実装に必要です。 */
   bool        block_extract;
};

/* 特定のコンテンツファイルタイプのフロントエンド処理を変更するオーバーライドを定義します。
 * RETRO_ENVIRONMENT_SET_CONTENT_INFO_OVERRIDE には、 
 * retro_system_content_info_override の配列が渡されます。
 * NOTE: 以下の説明では、retro_load_game() への参照が
 *       retro_load_game_special() に置き換えられる場合があります。  */
struct retro_system_content_info_override
{
   /* 「パイプ」文字で区切られた、オーバーライドが適用されるファイル拡張子のリスト 
    * (例: "md|sms|gg")
    * 許可されるファイル拡張子は retro_system_info::valid_extensions
    * および/または retro_subsystem_rom_info::valid_extensions に含まれるものに限定されます。 */
   const char *extensions;

   /* retro_system_info および/または retro_subsystem_rom_info に設定されている need_fullpath 値を上書きします。
    * To reiterate:
    *
    * need_fullpath が true で retro_load_game() が呼び出された場合: 
    *    - retro_game_info::path には、
    *      既存のファイルへの有効なパスが含まれていることが保証されています。 
    *    - retro_game_info::data と retro_game_info::size は無効です 
    *
    * need_fullpath が false で retro_load_game() が呼び出された場合: 
    *    - retro_game_info::path は NULL の可能性があります 
    *    - retro_game_info::data と
    *      retro_game_info::size は有効であることが保証されています
    *
    * 加えて:
    *
    * need_fullpath が true で retro_load_game() が呼び出された場合: 
    *    - retro_game_info_ext::full_path には、
    *      存在するファイルへの有効なパスが含まれていることが保証されています 
    *    - retro_game_info_ext::archive_path は NULL である可能性があります 
    *    - retro_game_info_ext::archive_file は NULL である可能性があります 
    *    - retro_game_info_ext::dir には、
    *      コンテンツ ファイルが存在するディレクトリへの有効なパスが含まれていることが保証されています 
    *    - retro_game_info_ext::name には、
    *      拡張子なしのコンテンツ ファイルのベース名が含まれることが保証されています。
    *    - retro_game_info_ext::ext には、
    *      コンテンツ ファイルの拡張子が小文字形式で含まれることが保証されています。
    *    - retro_game_info_ext::data と retro_game_info_ext::size は無効です 
    *
    * need_fullpath が false で retro_load_game() が呼び出された場合: 
    *    - retro_game_info_ext::file_in_archive が false の場合: 
    *       - retro_game_info_ext::full_path には、
    *         存在するファイルへの有効なパスが含まれていることが保証されています
    *       - retro_game_info_ext::archive_path は NULL である可能性があります 
    *       - retro_game_info_ext::archive_file は NULL である可能性があります
    *       - retro_game_info_ext::dir には、
    *         コンテンツ ファイルが存在するディレクトリへの有効なパスが含まれていることが保証されています 
    *       - retro_game_info_ext::name には、
    *         拡張子なしのコンテンツ ファイルのベース名が含まれることが保証されています。
    *       - retro_game_info_ext::ext には、
    *         コンテンツ ファイルの拡張子が小文字形式で含まれることが保証されています。 
    *    - retro_game_info_ext::file_in_archive が true の場合: 
    *       - retro_game_info_ext::full_path は NULL である可能性があります
    *       - retro_game_info_ext::archive_path には、
    *         コンテンツ ファイルが配置されている既存の
    *         圧縮ファイルへの有効なパスが含まれていることが保証されています 
    *       - retro_game_info_ext::archive_file は、
    *         retro_game_info_ext::archive_path によって参照される
    *         圧縮ファイル内の既存のコンテンツ ファイルへの有効なパスを含むことが保証されています。 
    *            例 「bar.sfc」を含む圧縮ファイル「/path/to/foo.zip」の場合
    *             > retro_game_info_ext::archive_path は「/path/to/foo.zip」になります 
    *             > retro_game_info_ext::archive_file は「bar.sfc」になります 
    *       - retro_game_info_ext::dir には、
    *         圧縮ファイル (コンテンツ ファイルを含む) が存在する
    *         ディレクトリへの有効なパスが含まれていることが保証されています。
    *       - retro_game_info_ext::name は、以下を含むことが保証されています。
    *         EITHER
    *         1) 圧縮ファイル (コンテンツ ファイルを含む) のベース名の(拡張子なし) 
    *         OR
    *         2) 圧縮ファイル内のコンテンツ ファイルのベース名 (拡張子なし) 
    *         いずれの場合も、コアは'name'をコンテンツファイルの正規の名前/IDとみなすべきです。
    *       - retro_game_info_ext::ext は、
    *         圧縮ファイル内のコンテンツ ファイルの拡張子を小文字形式で含むことが保証されています 
    *    - retro_game_info_ext::data と retro_game_info_ext::size は有効であることが保証されています  */
   bool need_fullpath;

   /* need_fullpath が false の場合、
    * retro_load_game() で利用可能なコンテンツ データ バッファーが「永続的」かどうかを指定します 
    *
    * persistent_data が false で retro_load_game() が呼び出された場合: 
    *    - retro_game_info::data と
    *      retro_game_info::size は retro_load_game() が戻るまで有効です
    *    - retro_game_info_ext::data と
    *      retro_game_info_ext::size は retro_load_game() が戻るまで有効です
    *
    * persistent_data が true で retro_load_game() が呼び出された場合: 
    *    - retro_game_info::data と
    *      retro_game_info::size は retro_deinit() が戻るまで有効です
    *    - retro_game_info_ext::data と
    *      retro_game_info_ext::size は retro_deinit() が戻るまで有効です */
   bool persistent_data;
};

/* retro_game_info に似ていますが、
 * ソース コンテンツ ファイルとゲーム メモリ バッファのステータスに関する拡張情報を提供します。
 * そして、retro_game_info_ext の配列は RETRO_ENVIRONMENT_GET_GAME_INFO_EXT によって返されます。 
 * NOTE: 以下の説明では、retro_load_game() への参照が
 *       retro_load_game_special() に置き換えられる場合があります。*/
struct retro_game_info_ext
{
   /* - file_in_archive が false の場合、
        既存のコンテンツ ファイル (UTF-8 エンコード) への有効なパスが含まれます。
    * - file_in_archive が true の場合、NULL の場合があります */
   const char *full_path;

   /* - file_in_archive が false の場合、NULL の場合があります 
    * - file_in_archive が true の場合、
    *   コンテンツ ファイルが配置されている
    *   既存の圧縮ファイルへの有効なパスが含まれています 
    *   (UTF-8 エンコード) */
   const char *archive_path;

   /* - file_in_archive が false の場合、NULL の場合があります
    * - file_in_archive が true の場合、archive_path (UTF-8 エンコード)
        によって参照される圧縮ファイル内に存在するコンテンツ ファイルへの有効なパスを含めます。 
    *      例: 「bar.sfc」を含む圧縮ファイル「/path/to/foo.zip」の場合 
    *      > archive_path は '/path/to/foo.zip' になります
    *      > archive_file は 'bar.sfc' になります*/
   const char *archive_file;

   /* - file_in_archiveがfalseの場合、
        コンテンツファイルが存在するディレクトリへの有効なパス
        （UTF-8でエンコードされたもの）が含まれます。
    * - file_in_archiveがtrueの場合、（コンテンツファイルを含む）
    *   圧縮ファイルが存在するディレクトリへの有効なパス
    *   （UTF-8でエンコードされたもの）が含まれます。*/
   const char *dir;

   /* コンテンツ ファイルの正規名/ID が含まれます (UTF-8 エンコード)。 
    * ロードされたファイルにちなんで名付けられた「補完的な」コンテンツ、
    * つまり異なるフォーマットのコンパニオン データ (ROM が必要とする CD イメージ)、
    * テクスチャ パック、内部的に処理される保存ファイルなどを
    * 識別する際の使用を目的としています。
    * - file_in_archive が false の場合、
    *   コンテンツ ファイルのベース名を拡張子なしで含みます。 
    * - file_in_archive が true の場合、文字列は実装固有です。 
    *   フロントエンドは、次の名前の値を設定することを選択できます。
    *   EITHER
    *   1) 圧縮ファイル (コンテンツ ファイルを含む) のベース名 (拡張子なし) 
    *   OR
    *   2) 圧縮ファイル内のコンテンツ ファイルのベース名 (拡張子なし) 
    *   RetroArchでは、(1)に従って「name」の値を設定します。
    *   複数の無関係なコンテンツファイルを含むアーカイブからの
    *   コンテンツの定期的な読み込みをサポートするフロントエンドは、
    *   (2)に従って「name」値を設定することができます。 */
   const char *name;

   /* - file_in_archive が false の場合、コンテンツ ファイルの拡張子が小文字形式で含まれます。 
    * - file_in_archive が true の場合、圧縮ファイル内のコンテンツ ファイルの拡張子を小文字形式で含みます。 */
   const char *ext;

   /* 実装固有のメタデータの文字列。 */
   const char *meta;

   /* ロードされたゲーム コンテンツのメモリ バッファ。 NULLになります :
    * IF
    * - retro_system_info::need_fullpath が true で、retro_system_content_info_override::need_fullpath が未設定の場合。
    * OR
    * - retro_system_content_info_override::need_fullpath が true である。 */
   const void *data;

   /* ゲーム コンテンツ メモリ バッファのサイズ (バイト単位) */
   size_t size;

   /* ロードされたコンテンツ ファイルが圧縮アーカイブ内にある場合は true */
   bool file_in_archive;

   /* - データが NULL の場合、値は未設定/無視されます
    * - データが NULL 以外の場合 :
    *   - persistent_data が false の場合、データとサイズは retro_load_game() が戻るまで有効です 
    *   - persistent_data が true の場合、データとサイズは retro_deinit() が戻るまで有効です。  */
   bool persistent_data;
};

struct retro_game_geometry
{
   unsigned base_width;    /* ゲームの名目上のビデオ幅 */
   unsigned base_height;   /* ゲームの名目上のビデオの高さ。 */
   unsigned max_width;     /* ゲームの最大可能幅 */
   unsigned max_height;    /* ゲームの最大可能高さ */

   float    aspect_ratio;  /* ゲームの公称アスペクト比。
                            * aspect_ratioが<= 0.0の場合は、
                            * base_width / base_heightのアスペクト比が想定されます。
                            * フロントエンドは必要に応じてこの設定を上書きすることができます。 */
};

struct retro_system_timing
{
   double fps;             /* ビデオ コンテンツの FPS。 */
   double sample_rate;     /* オーディオのサンプリングレート。*/
};

struct retro_system_av_info
{
   struct retro_game_geometry geometry;
   struct retro_system_timing timing;
};

struct retro_variable
{
   /* 環境文字列は、以下のようにセミコロンで区切られたキーと
    * 値のペアとしてフォーマットされます:
    * "key1=value1;key2=value2;..."
    */
   const char *key;

   /* 取得される値。キーが存在しない場合は、NULLが設定されます。 */
   const char *value;
};

struct retro_core_option_display
{
   /* RETRO_ENVIRONMENT_SET_CORE_OPTIONS_DISPLAYで設定する変数 */
   const char *key;

   /* ユーザーにコア・オプションを提示する際に、変数を表示するかどうかを指定する */
   bool visible;
};

/* コア オプションに許可される値の最大数 
 * > Note: C 言語の制限により、最大値を設定する必要があります。
 *   つまり、それぞれが可変サイズの配列を含む構造体の配列を作成することはできないため、
 *   retro_core_option_definition 値の配列は固定サイズでなければなりません。 
 *   128 のサイズ制限はバランスの取れた行為です。
 *   これは、すべての「正常な」コア オプションをサポートするのに十分な大きさである必要がありますが、
 *   大きすぎる設定は低メモリ プラットフォームに影響を与える可能性があります。
 *   実際には、コア オプションに 128 を超える値がある場合、実装に欠陥がある可能性があります。 
 *   上記APIリファレンスの引用:
 *      "可能な選択肢の数は非常に限られたものでなければなりません。
 *       つまり、キーボードなしで選択肢を循環させることが可能でなければなりません。"
 */
#define RETRO_NUM_CORE_OPTION_VALUES_MAX 128

struct retro_core_option_value
{
   /* 期待されるオプション値  */
   const char *value;

   /* 人間が読める値のラベル。
    * NULLの場合、値そのものがフロントエンドに表示されます。 */
   const char *label;
};

struct retro_core_option_definition
{
   /* RETRO_ENVIRONMENT_GET_VARIABLEで照会する変数。 */
   const char *key;

   /* 人間が読めるコアオプションの説明 (メニューラベルとして使用)  */
   const char *desc;

   /* 人間が読めるコアオプションの説明 (メニューサブラベルとして使用)  */
   const char *info;

   /* NULL で終了する retro_core_option_value 構造体の配列  */
   struct retro_core_option_value values[RETRO_NUM_CORE_OPTION_VALUES_MAX];

   /* コアオプションのデフォルト値。
    * retro_core_option_value 配列の値のひとつと一致しなければならず、
    * そうでなければ無視されます。*/
   const char *default_value;
};

struct retro_core_options_intl
{
   /* retro_core_option_definition 構造体の配列へのポインタ。
    * - 米国英語の実装 
    * - 有効な配列を指定する必要があります。 */
   struct retro_core_option_definition *us;

   /* retro_core_option_definition 構造体の配列へのポインタ。
    * - 現在のフロントエンド言語への実装
    * - NULL の場合があります */
   struct retro_core_option_definition *local;
};

struct retro_game_info
{
   const char *path;       /* UTF-8でエンコードされたゲームへのパスです。
                            * 他のパスを作成する際の参照として使われることもあります。
                            * ゲームが標準入力などから読み込まれた場合にはNULLでもよいのですが、
                            * この場合、一部のコアは`data`を読み込むことができません。
                            * そのため、NULLを渡すのではなく、ここで何かを作成したほうが、より多くのコアの成功につながります。
                            * retro_system_info::need_fullpath は、このパスが有効であることを要求します。*/
   const void *data;       /* 読み込まれたゲームのメモリバッファ。
                            * need_fullpathが設定されている場合はNULLとなります。 */
   size_t      size;       /* メモリバッファのサイズ。 */
   const char *meta;       /* 実装特有のメタデータの文字列。 */
};

#define RETRO_MEMORY_ACCESS_WRITE (1 << 0)
   /* コアは retro_framebuffer::data で提供されるバッファに書き込みます。 */
#define RETRO_MEMORY_ACCESS_READ (1 << 1)
   /* コアはretro_framebuffer::dataから読み込みます。 */
#define RETRO_MEMORY_TYPE_CACHED (1 << 0)
   /* データのメモリはキャッシュされています。
    * キャッシュされていない場合、バッファへのランダムな書き込みや読み出しは非常に遅くなることが予想されます。*/
struct retro_framebuffer
{
   void *data;                      /* コアがレンダリングできるフレームバッファ。
                                       フロントエンドが GET_CURRENT_SOFTWARE_FRAMEBUFFER で設定します。
                                       データの初期内容は不定です。 */
   unsigned width;                  /* コアが使用するフレームバッファの幅。コアによって設定されます。 */
   unsigned height;                 /* コアが使用するフレームバッファの高さ。コアによって設定されます。*/
   size_t pitch;                    /* 走査線の先頭から次の走査線の先頭までのバイト数です。
                                       フロントエンドが GET_CURRENT_SOFTWARE_FRAMEBUFFER で設定します。 */
   enum retro_pixel_format format;  /* コアがデータにレンダリングする際に使用しなければならないピクセルフォーマットです。
                                       このフォーマットは、 SET_PIXEL_FORMAT で使用されるフォーマットとは異なる場合があります。
                                       フロントエンドが GET_CURRENT_SOFTWARE_FRAMEBUFFER で設定します。 */

   unsigned access_flags;           /* コアがフレームバッファ内のメモリにアクセスする方法。
                                       RETRO_MEMORY_ACCESS_* flags.
                                       コアで設定。 */
   unsigned memory_flags;           /* メモリがどのようにマッピングされたかをコアに伝えるフラグです。
                                     * RETRO_MEMORY_TYPE_* flags.
                                     * フロントエンドが GET_CURRENT_SOFTWARE_FRAMEBUFFER で設定します。 */
};

/* フロントエンドの現在の fastforwarding mode を
 * オーバーライドするために libretro コアによって使用されます。 */
struct retro_fastforwarding_override
{
   /* 「fastforward」が真の場合に適用されるランタイム速度の倍率を指定します。
    * 例えば、60 FPSのコンテンツを実行する際に
    * 値を5.0に設定すると、早送り速度の上限が300 FPSになります。
    * なお、ホストハードウェアの処理能力が十分でない場合は、
    * 目標とする倍率に達しないことがあります。
    * 0.0（または0.0以上1.0未満）の値を設定すると、
    * 早送り速度の上限がなくなります（ハードウェアの処理能力によってのみ制限されます）。
    * 値が負の場合は無視されます
    *  (つまり、フロントエンドは独自に選択したランタイムスピードの倍率を使用します)。 */
   float ratio;

   /* Trueの場合、高速転送モードが有効になります。
    * falseの場合は、高速転送モードが無効になります。 */
   bool fastforward;

   /* trueの場合、フロントエンドがサポートしていれば、
    * 「fastforward」がtrueの間、画面上に通知が表示されます。
    * false の場合、およびフロントエンドでサポートされている場合は、
    * 画面上の早送りの通知はすべて抑制されます。*/
   bool notification;

   /* Trueの場合、コアが高速転送モードの有効化/無効化を単独で制御します。
    * フロントエンドは、'inhibit_toggle'がFalseに設定されるか、
    * コアがアンロードされるまで、'fastforward'で設定された状態を変更することができません。*/
   bool inhibit_toggle;
};

/* コールバック */

/* 環境コールバックです。
 * 一般的でないタスクを実行する方法を実装に与えます。拡張性があります。 */
typedef bool (RETRO_CALLCONV *retro_environment_t)(unsigned cmd, void *data);

/* フレームをレンダリングします。
 * ピクセルフォーマットは、変更されない限り15ビット0RGB1555ネイティブエンディアンです
 * （RETRO_ENVIRONMENT_SET_PIXEL_FORMAT参照）。
 *
 * 幅と高さはバッファの寸法を指定します。
 * Pitchはバッファの2行間の長さをバイト単位で指定します。
 *
 *
 * OpenGL ESなどの一部のグラフィックAPIは、]
 * メモリにパックされていないテクスチャを好まない場合があります。
 */
typedef void (RETRO_CALLCONV *retro_video_refresh_t)(const void *data, unsigned width,
      unsigned height, size_t pitch);

/* 1つのオーディオフレームをレンダリングします。
 * 一度に1つのサンプルを生成する実装の場合にのみ使用する必要があります。
 * フォーマットは、符号付き16ビットネイティブエンディアンです。
 */
typedef void (RETRO_CALLCONV *retro_audio_sample_t)(int16_t left, int16_t right);

/* 複数のオーディオフレームを一度にレンダリングします。
 *
 * 1フレームは、左右のチャンネルをインターリーブしたサンプルとして定義されます。
 * 例：int16_t buf[4] = { l, r, l, r }; は 2 フレームとなります。
 * オーディオコールバックは、必ず1つだけ使用してください。
 */
typedef size_t (RETRO_CALLCONV *retro_audio_sample_batch_t)(const int16_t *data,
      size_t frames);

/* Polls 入力。 */
typedef void (RETRO_CALLCONV *retro_input_poll_t)(void);

/* プレーヤー 'port' の入力を問い合わせます。デバイスは RETRO_DEVICE_MASK でマスクされます。
 * 
 * retro_set_controller_port_device() で設定された
 * RETRO_DEVICE_JOYPAD_MULTITAP のようなデバイスの特殊化は、
 * 入力を要求するために、より高いレベルの RETRO_DEVICE_JOYPAD をまだ使用します。
 */
typedef int16_t (RETRO_CALLCONV *retro_input_state_t)(unsigned port, unsigned device,
      unsigned index, unsigned id);

/* retro_set_environment()は retro_init()の前に呼ばれることが保証されています。
 * 
 * 残りの set_* 関数は retro_run() の最初の呼び出しが行われる前に呼び出されていることが保証されます。 */
RETRO_API void retro_set_environment(retro_environment_t);
RETRO_API void retro_set_video_refresh(retro_video_refresh_t);
RETRO_API void retro_set_audio_sample(retro_audio_sample_t);
RETRO_API void retro_set_audio_sample_batch(retro_audio_sample_batch_t);
RETRO_API void retro_set_input_poll(retro_input_poll_t);
RETRO_API void retro_set_input_state(retro_input_state_t);

/* Librari のグローバルな初期化/初期化解除。 */
RETRO_API void retro_init(void);
RETRO_API void retro_deinit(void);

/* RETRO_API_VERSIONを返さなければなりません。
 * APIが改訂されたときにABIの互換性を検証するために使用されます。 */
RETRO_API unsigned retro_api_version(void);

/* 静的に知られているシステム情報を取得します。
 * infoで提供されるポインタは、静的に割り当てられていなければなりません。
 * retro_init()の前であっても、いつでも呼び出すことができます。 */
RETRO_API void retro_get_system_info(struct retro_system_info *info);

/* システムオーディオ/ビデオのタイミングとジオメトリに関する情報を取得する。
 * retro_load_game()が正常に完了した後にのみ呼び出すことができる。
 * NOTE: この関数の実装では、必要に応じてすべての変数が初期化されない場合があります。
 * E.g. コアが特定のアスペクト比を必要としない場合、
 * geom.aspect_ratio は初期化されない可能性があります。  */
RETRO_API void retro_get_system_av_info(struct retro_system_av_info *info);

/* プレーヤーの「ポート」に使用するデバイスを設定します。
 * デフォルトでは、 RETRO_DEVICE_JOYPAD は使用可能なすべてのポートに接続されていると想定されています。
 * 特定のデバイス タイプを設定しても、libretro コアがその特定の
 * デバイス タイプに基づいて入力のみをポーリングするという保証はありません。 
 * これは、コアが適切な入力デバイス タイプを自動的に検出できない場合の
 * libretro コアへのヒントにすぎません。 また、コアがデバイス タイプに応じて動作を変更できる場合にも関連します。
 * 
 * コアの retro_set_controller_port_device の実装の一部として、
 * コアは RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS を呼び出して、
 * デバイス タイプの変更の結果としてコントロールの説明が変更された場合、フロントエンドに通知する必要があります。 
 */
RETRO_API void retro_set_controller_port_device(unsigned port, unsigned device);

/* 現在のゲームをリセットします。 */
RETRO_API void retro_reset(void);

/* ゲームを1ビデオフレーム分実行します。
 * retro_run()の間、input_pollコールバックは少なくとも一度は呼ばれなければならない。
 * 
 * retro_run()は、GET_CAN_DUPEがtrueを返した場合、明示的にフレームを複製しなければならない。
 * この場合、video コールバックはデータ用に NULL 引数を取ることができる。
 */
RETRO_API void retro_run(void);

/* 内部状態（セーブステート）をシリアライズするために、実装が必要とするデータの量を返す。
 * retro_load_game()とretro_unload_game()の間では、
 * フロントエンドがセーブステートバッファを一度だけ割り当てることができることを保証するために、
 * 返されたサイズは以前に返された値よりも大きくなることは許されません。
 */
RETRO_API size_t retro_serialize_size(void);

/* 内部状態をシリアル化します。
 * 失敗した場合、またはサイズがretro_serialize_size()よりも小さい場合、falseを返し、そうでない場合はtrueを返します。 */
RETRO_API bool retro_serialize(void *data, size_t size);
RETRO_API bool retro_unserialize(const void *data, size_t size);

RETRO_API void retro_cheat_reset(void);
RETRO_API void retro_cheat_set(unsigned index, bool enabled, const char *code);

/* ゲームをロードします。
 * 読み込みに成功した場合はtrueを、失敗した場合はfalseを返します。
 */
RETRO_API bool retro_load_game(const struct retro_game_info *game);

/* 特殊なゲームを読み込みます。極端な場合を除き、使用すべきではありません。 */
RETRO_API bool retro_load_game_special(
  unsigned game_type,
  const struct retro_game_info *info, size_t num_info
);

/* 現在ロードされているゲームをアンロードします。retro_deinit(void)の前に呼び出されます。 */
RETRO_API void retro_unload_game(void);

/* ゲームのリージョンを取得します。 */
RETRO_API unsigned retro_get_region(void);

/* メモリの領域を取得します。 */
RETRO_API void *retro_get_memory_data(unsigned id);
RETRO_API size_t retro_get_memory_size(unsigned id);

#ifdef __cplusplus
}
#endif

#endif
