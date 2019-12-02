
#ifndef _CLI_CMD_H_
#define _CLI_CMD_H_

static int cmd_info(int argc, char *argv[]);
static int cmd_mem(int argc, char *argv[]);
static int cmd_reboot(int argc, char *argv[]);
static int cmd_reset(int argc, char *argv[]);

extern cli_cmd_t terminal_cmds[];

#endif /* _CLI_CMD_H_ */