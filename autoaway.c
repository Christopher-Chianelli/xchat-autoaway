/*
 * autoaway.c
 * Copyright (C) 2017 Christopher Chianelli
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "xchat-plugin.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PNAME "AutoAway"
#define PDESC "Auto away when screen is locked"
#define PVERSION "0.1"

static xchat_plugin *ph;   /* plugin handle */
static int enable = 1;
static int away = 0;
static char here_nick[256];
static char away_nick[256];
static char away_msg[256];

static xchat_hook *timer_hook;


static int is_screen_locked() {
	FILE *fp;
	fp = popen("/usr/bin/gnome-screensaver-command -t", "r");
	if (!fp) {
		return 0;
	}
	char data[1024];
	fgets(data, sizeof(data) - 1, fp);
	return strcmp(data, "The screensaver is not currently active.\n");
}

static int check_if_user_left(void *userdata)
{
	if (is_screen_locked()) {
		xchat_commandf(ph, "ALLSERV NICK %s", away_nick);
		xchat_commandf(ph, "ALLSERV AWAY %s", away_msg);
		away = 1;
	}
	else if (away) {
		xchat_commandf(ph, "ALLSERV NICK %s", here_nick);
		xchat_commandf(ph, "ALLSERV BACK");
		away = 0;
	}
	return 1;       /* return 1 to keep the timeout going */
}

static int autoaway_toggle_cb(char *word[], char *word_eol[], void *userdata)
{
	if (!enable)
	{
		enable = 1;
		timer_hook = xchat_hook_timer(ph, 5000, check_if_user_left, NULL); 
		xchat_print(ph, "AutoAway now enabled!\n");
	} else
	{
		enable = 0;
		xchat_unhook(ph, timer_hook);
		xchat_print(ph, "AutoAway now disabled!\n");
	}

	return XCHAT_EAT_ALL;   /* eat this command so xchat and other plugins can't process it */
}

static int autoaway_set_here_name_cb(char *word[], char *word_eol[], void *userdata)
{
	strcpy(here_nick, word[2]);
	xchat_printf(ph, "Your name when not away is now %s\n", word[2]);
	return XCHAT_EAT_ALL;   /* eat this command so xchat and other plugins can't process it */
}

static int autoaway_set_away_name_cb(char *word[], char *word_eol[], void *userdata)
{
	strcpy(away_nick, word[2]);
	xchat_printf(ph, "Your name when away is now %s\n", word[2]);
	return XCHAT_EAT_ALL;   /* eat this command so xchat and other plugins can't process it */
}

static int autoaway_set_away_msg_cb(char *word[], char *word_eol[], void *userdata) {
	strcpy(away_msg, word_eol[2]);
	xchat_printf(ph, "Your away message is now \"%s\"\n", word_eol[2]);
	return XCHAT_EAT_ALL;
}

void xchat_plugin_get_info(char **name, char **desc, char **version, void **reserved)
{
	*name = PNAME;
	*desc = PDESC;
	*version = PVERSION;
}

int xchat_plugin_init(xchat_plugin *plugin_handle,
		char **plugin_name,
		char **plugin_desc,
		char **plugin_version,
		char *arg)
{
	/* we need to save this for use with any xchat_* functions */
	ph = plugin_handle;
	strcpy(here_nick, "user");
	strcpy(away_nick, "user|away");
	strcpy(away_msg, "");

	/* tell xchat our info */
	*plugin_name = PNAME;
	*plugin_desc = PDESC;
	*plugin_version = PVERSION;

	xchat_hook_command(ph, "AutoAwayToggle", XCHAT_PRI_NORM, autoaway_toggle_cb, "Usage: AUTOAWAYTOGGLE, Turns OFF/ON Auto AWAY", 0);
	xchat_hook_command(ph, "AwayUserName", XCHAT_PRI_NORM, autoaway_set_away_name_cb, "Usage: AWAYUSERNAME NICK, Set username when away", 0);
	xchat_hook_command(ph, "HereUserName", XCHAT_PRI_NORM, autoaway_set_here_name_cb, "Usage: HEREUSERNAME NICK, Set username when here", 0);
	xchat_hook_command(ph, "AwayMsg", XCHAT_PRI_NORM, autoaway_set_away_msg_cb, "Usage: AWAYMSG MSG, Set away message", 0);
	timer_hook = xchat_hook_timer(ph, 5000, check_if_user_left, NULL); 

	xchat_print(ph, "AutoAwayPlugin loaded successfully!\n");

	return 1;       /* return 1 for success */
}


