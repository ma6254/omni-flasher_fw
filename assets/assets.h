#ifndef ASSETS_H
#define ASSETS_H

#include <lvgl.h>
#include "proj_config.h"

#if CFG_USE_STARTUP_GIF
LV_IMG_DECLARE(startup_gif);
#endif // CFG_USE_STARTUP_GIF

LV_IMG_DECLARE(remote_icon);
LV_IMG_DECLARE(flash_icon);
LV_IMG_DECLARE(serial_icon);
LV_IMG_DECLARE(sd_card_icon);
LV_IMG_DECLARE(settings_icon);
LV_IMG_DECLARE(info_icon);
LV_IMG_DECLARE(jtag_icon);

// remote_screen
LV_IMG_DECLARE(usb_icon);
LV_IMG_DECLARE(wifi_icon);
LV_IMG_DECLARE(flashrom_logo);


// settings_screen
LV_IMG_DECLARE(sun_icon);

LV_FONT_DECLARE(main_menu_screen_item_font);
LV_FONT_DECLARE(remote_screen_item_font);
LV_FONT_DECLARE(serial_screen_item_font);
LV_FONT_DECLARE(settings_screen_item_font);

#endif // ASSETS_H
