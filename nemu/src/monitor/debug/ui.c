#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}

static int cmd_si(char *args){ 
	//printf(".....\n");
	char *token = strtok(args," ");
	if(token == NULL)
	{
		cpu_exec(1);
		return 0;
	}
	//token = strtok(NULL," ");
	int num = atoi(token);
	//printf("%d \n",num);	
	cpu_exec(num);
	//printf("%d \n",num);
	//printf("this is cmd si.\n");
	return 0;
}

static int cmd_info(char *args){
	char *token = strtok(args," ");
	if(token == NULL)
	{
		return 0;
	}
	if(strcmp(token,"r")==0)
	{
		printf("%s:\t%8x\t\n",regsl[0],cpu.gpr[0]._32);
		for(int i=1;i<8;i++)
		{
			printf("%s:\t%8x\n",regsl[i],cpu.gpr[i]._32);
		}
	}

	else if(strcmp(token,"w")==0)
	{
		printf("tbd in PA1.3\n");
	}
	else
	{
		printf("???\n");
	}
	return 0;
}

static int cmd_x(char *args){
	char *token = strtok(args," ");
	if(token == NULL)
	{
		printf("time???");
		return 0;
	}
	int ts ;	//times
	ts = atoi(token);
	//printf("%d\n",ts);

	token = strtok(NULL," ");
	if(token == NULL)
	{
		printf("address???");
		return 0;
	}
	uint32_t ads;	//address
	sscanf(token,"%x",&ads);
	//printf("%#x\n",ads);
	
	for(int i = 0 ; i < ts;i++)
	{	
		printf("%#010x\t",ads);
		uint32_t temp = vaddr_read(ads,4);	//4.1
		printf("%#010x\t",temp);
		printf(" ... ");
		for(int j = 0 ; j < 4 ;j++)
		{
			temp = vaddr_read(ads,1);	//4.2
			printf("%02x ",temp);
			ads++;
		}

		printf("\n");
		//ad += 4;
	}

	return 0;
}

static int cmd_p(char *args)
{
	bool* flag = malloc(sizeof(bool));
	*flag = true;
	expr(args,flag);
	return 0;
}

static int cmd_help(char *args);

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si","Simple step", cmd_si },
  { "info","Print Register information", cmd_info },
  { "x", "Scan memory", cmd_x },
  { "p", "Evaluation of expression", cmd_p }
  /* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  while (1) {
    char *str = rl_gets();
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
