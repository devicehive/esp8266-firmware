/*
 * dhserial_commandline.c
 *
 * Copyright 2015 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 * Description: Command line interpreter
 *
 */

#include <ets_sys.h>
#include <osapi.h>
#include "dhserial_commandline.h"
#include "dhserial_commands.h"
#include "snprintf.h"

typedef void (*CommandFunc)(const char *arg);

typedef struct {
	char *name;
	char *help;
	CommandFunc func;
} DHCOMMAND;

void command_help();
static const DHCOMMAND mCommands[] = {
		{"config", "print current configuration", dhserial_commands_config},
		{"configure", "run setup utility", dhserial_commands_configure},
		{"status", "print current status", dhserial_commands_status},
		{"scan", "scan wireless networks", dhserial_commands_scan},
		{"nslookup", "resolve domain IP address", dhserial_commands_nslookup},
		{"ping", "ping remote host", dhserial_commands_ping},
		{"dmesg", "activate debug mode, it also prints few last events, use 'q' for exit", dhserial_commands_debug},
		{"history", "print command history", dhserial_commands_history},
		{"uname", "print firmware info", dhserial_commands_uname},
		{"echo", "print the same, just for test", dhserial_commands_echo},
		{"reboot", "restart device", dhserial_commands_reboot},
		{"help", "print help", command_help}
};

void ICACHE_FLASH_ATTR command_help() {
	uart_send_line("Welcome to the DeviceHive firmware. List of accepted command:\r\n");
	int i;
	for(i = 0; i < sizeof(mCommands)/sizeof(DHCOMMAND); i++ ) {
		uart_send_str(mCommands[i].name);
		uart_send_str(" - ");
		uart_send_line(mCommands[i].help);
	}
}

int ICACHE_FLASH_ATTR line_containt_command(const char *line, const char *command, int check_end) {
	while(*command) {
		if(*line == 0 || *line != *command)
			return 0;
		line++;
		command++;
	}
	if(check_end == 0 || *line == 0 || *line == ' ')
		return 1;
	return 0;
}

void ICACHE_FLASH_ATTR dhserial_commandline_do(const char *command) {
	if(command[0] == 0)
		return;
	int i;
	for(i = 0; i < sizeof(mCommands)/sizeof(DHCOMMAND); i++ ) {
		if(line_containt_command(command, mCommands[i].name, 1)) {
			const char *args = command;
			while(*args && *args != ' ')
				args++;
			while(*args == ' ')
				args++;
			mCommands[i].func(args);
			return;
		}
	}
	uart_send_str(command);
	uart_send_line(": Unknown command. Type 'help' for help.");
}

char * ICACHE_FLASH_ATTR dhserial_commandline_autocompleater(const char *pattern) {
	static int mLastIndex = 0;
	static char *mLastReturned = "";
	static char mLastPatter[81] = "";
	if(pattern[0] == 0)
		return 0;
	if(os_strcmp(pattern, mLastReturned)) {
		mLastIndex = 0;
	} else {
		mLastIndex++;
		pattern = mLastPatter;
	}
	int i;
	for(i = mLastIndex; i < sizeof(mCommands)/sizeof(DHCOMMAND); i++ ) {
		if(line_containt_command(mCommands[i].name, pattern, 0)) {
			mLastIndex = i;
			mLastReturned = mCommands[i].name;
			snprintf(mLastPatter, sizeof(mLastPatter), "%s", pattern);
			return mLastReturned;
		}
	}
	if(mLastIndex) {
		mLastIndex = 0;
		return dhserial_commandline_autocompleater(pattern);
	}
	return 0;
}
