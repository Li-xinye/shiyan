#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint32_t);

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
static int cmd_info(char *args)
{
    char *arg=strtok(args," ");
    if(strcmp(arg,"r")==0)
    {
        printf("eax 0x%x %d\n",cpu.eax,cpu.eax);
        printf("edx 0x%x %d\n",cpu.edx,cpu.edx);
        printf("ecx 0x%x %d\n",cpu.ecx,cpu.ecx);
        printf("ebx 0x%x %d\n",cpu.ebx,cpu.edx);
        printf("ebp 0x%x %d\n",cpu.ebp,cpu.ebp);
        printf("esi 0x%x %d\n",cpu.esi,cpu.esi);
        printf("edi 0x%x %d\n",cpu.edi,cpu.edi);
        printf("esp 0x%x %d\n",cpu.esp,cpu.esp);
    }
    if(strcmp(arg,"w")==0)
    {
        info_wp();
    }
    return 0; 
}
static int cmd_x(char *args)
{
    
    char *arg1=strtok(NULL," ");
    char *arg2=strtok(NULL," ");
    int n;
    lnaddr_t address;
    bool success;
    sscanf(arg1,"%d",&n);
    address = expr(arg2,&success);
    while(n--)
    {
        printf("%x ",lnaddr_read(address,4));
        address+=4;
        printf("\n");
    }
    return 0;
}

static int cmd_p(char *args)
{
    uint32_t n;
    bool suc;
    n=expr(args,&suc);
    if (suc)
         printf("%d\n",n);
    return 0;
}
static int cmd_w(char *args)
{
	WP *f;
	bool success;
	f = new_wp();
	printf("Watchpoint %d: %s\n",f->NO,args);
	f->value = expr(args,&success);
	strcpy(f->expr,args);
	if(!success) Assert(1,"wrong\n");
	printf("Value : %d\n",f->value);
	return 0;
		
}

static int cmd_d(char *args)
{
	int n;
	sscanf(args,"%d",&n);
	delete_wp(n);
	return 0;
}
static int cmd_help(char *args);
static int cmd_si(char *args);

static struct {
	char *name;
	char *description;
	int (*handler) (char *);
} cmd_table [] = {
	{ "help", "Display informations about all supported commands", cmd_help },
	{ "c", "Continue the execution of the program", cmd_c },
	{ "q", "Exit NEMU", cmd_q },
        { "si","Executes N steps of the program,if N is not inserted,executes one step",cmd_si},
        { "info", "r -print the registers,w -print watchpoints",cmd_info},
        { "x", "Caculate the expression and print the content of the address",cmd_x},
        { "p", "Caculate the value of the expression",cmd_p},
	{ "w", "Setting watchpoint",cmd_w},
	{ "d", "Delete watchpoint",cmd_d},
	/* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
	/* extract the first argument */
	char *arg = strtok(NULL, " ");
	int i;

	if(arg == NULL) {
		/* no argument given */
		for(i = 0; i < NR_CMD; i ++) {
			printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
		}
	}
	else {
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(arg, cmd_table[i].name) == 0) {
				printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
				return 0;
			}
		}
		printf("Unknown command '%s'\n", arg);
	}
	return 0;
}

static int cmd_si(char *args){
    char  *arg = strtok(args," ");
    int s;
    if(arg==NULL)
        s=1;
    else
        s=atoi(arg);
    cpu_exec(s);
    return 0;

};
void ui_mainloop() {
	while(1) {
		char *str = rl_gets();
		char *str_end = str + strlen(str);

		/* extract the first token as the command */
		char *cmd = strtok(str, " ");
		if(cmd == NULL) { continue; }

		/* treat the remaining string as the arguments,
		 * which may need further parsing
		 */
		char *args = cmd + strlen(cmd) + 1;
		if(args >= str_end) {
			args = NULL;
		}

#ifdef HAS_DEVICE
		extern void sdl_clear_event_queue(void);
		sdl_clear_event_queue();
#endif

		int i;
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(cmd, cmd_table[i].name) == 0) {
				if(cmd_table[i].handler(args) < 0) { return; }
				break;
			}
		}

		if(i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
	}
}
