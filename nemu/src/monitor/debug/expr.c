#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
	NOTYPE = 256, EQ=254,D_NUM=250,HEX_NUM=245,REG=240,UEQ=235,AND=230,OR=225

	/* TODO: Add more token types */

};

static struct rule {
	char *regex;
	int token_type;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */

	{" +",	NOTYPE},				// spaces
	{"\\+", '+'},					// plus
	{"==", EQ},						// equal
        {"\\-",'-'},
        {"\\*",'*'},				
        {"\\/",'/'},
        {"\\(",'('},
        {"\\)",')'},
        {"\\b[0-9]+\\b",D_NUM},
	{"\\0[xX][0-9a-fA-F]+\\b",HEX_NUM},
	{"\\$[a-zA-Z]+\\b",REG},
        {"!=",UEQ},
	{"!",'!'},
	{"&&",AND},
	{"\\|\\|",OR}
	
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
	int i;
	char error_msg[128];
	int ret;

	for(i = 0; i < NR_REGEX; i ++) {
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		if(ret != 0) {
			regerror(ret, &re[i], error_msg, 128);
			Assert(ret == 0, "regex compilation failed: %s\n%s", error_msg, rules[i].regex);
		}
	}
}

typedef struct token {
	int type;
	char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
	int position = 0;
	int i;
	regmatch_t pmatch;
	
	nr_token = 0;

	while(e[position] != '\0') {
		/* Try all rules one by one. */
		for(i = 0; i < NR_REGEX; i ++) {
			if(regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;
				char *tmp = e + position + 1;
				Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);
				position += substr_len;

				/* TODO: Now a new token is recognized with rules[i]. Add codes
				 * to record the token in the array `tokens'. For certain types
				 * of tokens, some extra actions should be performed.
				 */

				switch(rules[i].token_type) {
                                        case '+':
                                        case '*':
                                        case '-':
                                        case '/':
                                        case '(':
                                        case ')':
                                        case D_NUM:
					case HEX_NUM:
					case UEQ:
					case '!':
					case AND:
					case OR:
					case EQ:
					
                                        {
                                            tokens[nr_token].type=rules[i].token_type;
                                            strncpy(tokens[nr_token].str,substr_start,substr_len);
                                            nr_token++;
                                            tokens[nr_token].str[substr_len]='\0';
                                            break; 
                                        }
                                        case NOTYPE:
                                             break;
					case REG:
					{
					    tokens[nr_token].type=rules[i].token_type;
					    strncpy(tokens[nr_token].str,tmp,substr_len-1);
					    nr_token++;
					    tokens[nr_token].str[substr_len]='\0';
					    break;  
					}
					default: panic("please implement me");
				}

				break;
			}
		}

		if(i == NR_REGEX) {
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}
	}
	return true; 
}
bool check_parentheses(int p,int q)
{
    int i, tag=0;
    if(tokens[p].type != '(' || tokens[q].type != ')')
        return false;
    for(i=p ; i<=q ;i++)
    {
        if( tokens[i].type=='(')
            tag++;
        else if(tokens[i].type==')')
            tag--;
        if(tag==0&&i<q)
            return false;
    }
    if(tag!=0) return false;
    return true;
}
int priority(int t)
{
	int tag=0;
	if(t=='+'||t=='-')
        	tag=4;
	else if(t=='*'||t=='/')
        	tag=5;
	else if(t==OR)
		tag=1;
	else if(t==AND)
		tag=2;
	else if(t==EQ||t==UEQ)
		tag=3;
	else if(t=='!')
		tag=6;
	return tag;
}
int dominant_operation(int p,int q)
{
    int i,oper=p,tag=0,pri=10;
    for(i=p;i<=q;i++)
    {
        if(tokens[i].type=='(')
        {
            tag++;
            i++;
            while(1)
            {
                if(tokens[i].type=='(') tag++;
                else if(tokens[i].type==')') tag--;
		i++;
                if(tag==0) break;
            }
	    if(i>q) break;
        }
        if(tokens[i].type==D_NUM||tokens[i].type==HEX_NUM||tokens[i].type==REG)
              continue;
        if(priority(tokens[i].type)<=pri)
              {
                  pri=priority(tokens[i].type);
                  oper=i;
              }
    }
    return oper;
}
uint32_t eval(int p,int q);
uint32_t expr(char *e, bool *success) {
	if(!make_token(e)) {
		*success = false;
		return 0;
	}

	/* TODO: Insert codes to evaluate the expression. */
        else
        {
            *success=true;
       	    return eval(0,nr_token-1);
        }
        panic("Please implement me");
        return 0;
}
uint32_t eval(int p,int q)
{
	if(p > q)
        {
        	printf("Bad expression!");
            	return 0; 
        }
	else if(p==q)
        {
        	uint32_t n;
	        if(tokens[p].type==D_NUM)
             		sscanf(tokens[p].str,"%d",&n);
	        if(tokens[p].type==HEX_NUM)
			sscanf(tokens[p].str,"%x",&n);
	        if(tokens[p].type==REG)
		{
			if (strlen (tokens[p].str) == 3)
			{
				int i;
				for( i=R_EAX;i<=R_EDI;i++ )
					if (strcmp (tokens[p].str,regsl[i])==0)
						break;
				if ( i>R_EDI )
					if (strcmp (tokens[p].str,"eip") == 0)
						n = cpu.eip;
				if( i<=R_EDI)	n=reg_l(i);
			}
			if ( strlen (tokens[p].str) == 2 )
			{
				if (tokens[p].str[1]=='x'||tokens[p].str[1]=='p'||tokens[p].str[1]=='i')
				{
					int i;
					for ( i=R_AX;i<=R_DI;i++ )
						if ( strcmp (tokens[p].str,regsw[i])==0 )
					n = reg_w(i);
				}	
				if ( tokens[p].str[1]=='l'||tokens[p].str[1]=='h' )
				{
					int i;
					for ( i=R_AL;i<=R_BH;i++ )
						if (strcmp(tokens[p].str,regsb[i])==0)
							break;
					n=reg_b(i);
				}
			}
		}
                return n;
        }
    else if(check_parentheses(p,q))
       {
            return eval(p+1,q-1);
       }
    else
       {
           int op=dominant_operation(p,q);
           uint32_t value1 = eval(p,op-1);
           uint32_t value2 = eval(op+1,q);
           switch (tokens[op].type){
               case '+' : return value1 + value2;
               case '-' : return value1 - value2;
               case '*' : return value1 * value2;
               case '/' : return value1 / value2;
	       case EQ : return value1 == value2;
	       case UEQ : return value1 != value2;
	       case AND : return value1 && value2;
	       case OR : return value1 || value2;
               default : assert(0);
           }
       }
}
