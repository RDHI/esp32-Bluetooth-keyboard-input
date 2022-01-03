#include "keymapping.h"
xQueueHandle btkey_queue = NULL;
btkey bt_keymap[BT_KEY_MAX] = {
{0x4,   "aA"},
{0x5,   "bB"},
{0x6,   "cC"},
{0x7,   "dD"},
{0x8,   "eE"},
{0x9,   "fF"},
{0xa,   "gG"},
{0xb,   "hH"},
{0xc,   "iI"},
{0xd,   "jJ"},
{0xe,   "kK"},
{0xf,   "lL"},
{0x10,  "mM"},
{0x11,  "nN"},
{0x12,  "oO"},
{0x13,  "pP"},
{0x14,  "qQ"},
{0x15,  "rR"},
{0x16,  "sS"},
{0x17,  "tT"},
{0x18,  "uU"},
{0x19,  "vV"},
{0x1a,  "wW"},
{0x1b,  "xX"},
{0x1c,  "yY"},
{0x1d,  "zZ"},
{0x1e,  "1!"},
{0x1f,  "2@"},
{0x20,  "3#"},
{0x21,  "4$"},
{0x22,  "5%"},
{0x23,  "6^"},
{0x24,  "7&"},
{0x25,  "8*"},
{0x26,  "9("},
{0x27,  "0)"},
{0x2a,   "BACKSPACE"},
{0x2b,   "TAB"},
{0x2c,   "SPACE"},
{0x28,   "RETURN"},
{0x29,   "ESCAPE"},
{0x39,   "CAPSLOCK"},


{0x2d,   "-_"},
{0x2e,   "=+"},
{0x2f,   "[{"},
{0x30,   "]}"},
{0x31,   "\\|"},
//{0x32,   "Eur1"},
{0x33,   ";:"},
{0x34,   "\'\""},
{0x35,   "dummy"},
{0x36,   ",<"},
{0x37,   ".>"},
{0x38,   "/?"},
{0x01,   "LCTR,"},


};
static const char *TAG = "KEYMAPPING";
void init_btkey_queue()
{
    btkey_queue = xQueueCreate(BT_KEY_MAX * 2, sizeof(uint8_t));
}

void key_pressed(uint8_t* data, size_t size) {



    for(size_t i = 0; i < size; i++) {
        for(uint32_t j = 0; j < BT_KEY_MAX; j++) {
            if(bt_keymap[j].keyboard_scancode == data[i]) {              
                //send_scancode(bt_keymap[j].keyboard_scancode);
                xQueueSend(btkey_queue, &(bt_keymap[j].keyboard_scancode), (TickType_t) 0);
                ESP_LOGI(TAG, ": %s\n", bt_keymap[j].ascii_key);
                break;
            }
        }
    }
}

uint16_t Key2code(uint8_t keyboard_scancode, bool uppercase, bool IsAscii){

    if (IsAscii == false)
    {
        if ((keyboard_scancode>=0x04 && keyboard_scancode<=0x09)||(keyboard_scancode>=0x1e && keyboard_scancode<=0x27))
        {
            return (uint16_t)(0xff00+(bt_keymap[keyboard_scancode-4].ascii_key[0]));
        }
        
    }

    if (keyboard_scancode>=0x04 && keyboard_scancode<=0x27){
        if (uppercase == false){
            return (uint16_t)(bt_keymap[keyboard_scancode-4].ascii_key[0]);
        }
        else{
            return (uint16_t)(bt_keymap[keyboard_scancode-4].ascii_key[1]);
        }
    }

    switch(keyboard_scancode){
        case 0x2a: return 0x1000+keyboard_scancode; //backspace
        case 0x2b: return 0x1000+keyboard_scancode; //tab
        case 0x2c: return 0x1000+keyboard_scancode; //space
        case 0x28: return 0x1000+keyboard_scancode; //return
        case 0x29: return 0x1000+keyboard_scancode; //escape
        case 0x39: return 0x1000+keyboard_scancode; //caps lock
        case 0x01: return 0x1000+keyboard_scancode; //left control
    }

    if (keyboard_scancode>=0x2d && keyboard_scancode<=0x38 && keyboard_scancode!=0x32 && keyboard_scancode!=0x35){
    if (uppercase == false){
        return (uint16_t)(bt_keymap[keyboard_scancode-3].ascii_key[0]);
    }
    else{
        return (uint16_t)(bt_keymap[keyboard_scancode-3].ascii_key[1]);
    }
    }

    if (keyboard_scancode>=0x3a && keyboard_scancode<=0x45){

        return 0x0100+keyboard_scancode;
    
    }
    return 0;
}