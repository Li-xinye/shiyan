#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
	NOTYPE = 256, EQ,D_NUM

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
        {"\\b[0-9]+\\b",D_NUM}
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
                                        {
                                            tokens[nr_token].type=rules[i].token_type;
                                            strncpy(tokens[nr_token].str,substr_start,substr_len);
                                            nr_token++;
                                            tokens[nr_token].str[substr_len]='\0';
                                            break; 
                                        }
                                        case NOTYPE:
                                             break;
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
int priority(char t)
{
    int tag=0;
    if(t=='+'||t=='-')
        tag=1;
    else if(t=='*'||t=='/')
        tag=2;
    return tag;
}
int dominant_operation(int p,int q)
{
    int i,j,oper=p,tag=0,pri=10;
    for(i=p;i<=q;i++)
    {
        if(tokens[i].type=='(')
        {
            tag++;
            i++;
            for(j=i;j<=q;j++)
            {
                if(tokens[j].type=='(') tag++;
                else if(tokens[j].type==')') tag--;
                if(tag==0) break;
            }
        }
        else if(tokens[i].type==D_NUM) continue;
        else if(priority(tokens[i].type)<=pri)
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
            *success=true;
        if(!success)
            panic("Please implement me");
	return eval(0,nr_token-1);
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
             n=sscanf(tokens[p].str,"%d",&n);
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
               case '+' : return value1+value2;
               case '-' : return value1-value2;
               case '*' : return value1*value2;
               case '/' : return value1/value2;
               default : assert(0);
           }
       }
}
