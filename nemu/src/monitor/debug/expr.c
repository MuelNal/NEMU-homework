#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>

int32_t find_op(int32_t p,int32_t q,bool *success);
bool check_parentheses(int32_t p, int32_t q,bool *success);
bool check_valid(int32_t p,int32_t q);
void find_NEG(int32_t p,int32_t q);

enum {
	NOTYPE = 256, EQ, NUM

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
	{"=", EQ},						// equal
	{"[0-9]+", NUM},					//number
	{"-", '-'},						//subtraction
	{"\\*", '*'},					//multiplication
	{"/", '/'},						//division
	{"\\(", '('},					//LPAREN
	{"\\)", ')'}						//RPAREN
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
	bool is_NEG;
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

				
				if(nr_token>31) return false;
				else{
					if(substr_len>31&&rules[i].token_type==NUM) {
						//assert(0);
						return false;
					}
					switch(rules[i].token_type) {
						case NOTYPE:break;
						case NUM:{						
							tokens[nr_token].type=rules[i].token_type;
							tokens[nr_token].is_NEG=false;
							strncpy(tokens[nr_token].str,substr_start,substr_len);
							tokens[nr_token].str[substr_len] = '\0'; //这里有一个溢出bug
							nr_token++;
							break;
						}
						case '+':
						case '-':
						case '*':
						case '/':
						case '(':
						case ')':
						case '=':{
							tokens[nr_token].type=rules[i].token_type;
							nr_token++;
							break;
						}
						default: panic("please implement me");
					}
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

int32_t eval(int32_t p,int32_t q,bool *success){
	if(*success==false){
		return 0;
	}
	if(p>q){
		return 0;
	}
	else if(p==q){
		if (tokens[p].type != NUM) {
    	*success = false;
    	return 0;
		}
		int val=atoi(tokens[p].str);
		if(!tokens[p].is_NEG){
			// printf("%d\n",val);
			return val;
		}
		else{
			//printf("%d\n",0-val);
			return 0-val;
		}
	}
	else if(check_parentheses(p, q,success) == true){
		return eval(p+1,q-1,success);
	}
	else{
		int op=find_op(p,q,success);
		//printf("op:%d\n",op);
		if (!*success) {
			return 0;
		}
		int val1=eval(p,op-1,success);
		if (!*success) {
			return 0;
		}
		int val2=eval(op+1,q,success);
		if (!*success) {
			return 0;
		}
		switch (tokens[op].type)
		{
		case '+': return val1+val2;
		case '-': return val1-val2;
		case '*': return val1*val2;
		case '/': if (val2 == 0) {
					*success = false;
					return 0;					
				}
				  return val1/val2;
		case NOTYPE: return eval(p+1,q,success);
		default: {
				assert(0);
				return 0;
			}
		}
	}
}

int32_t find_op(int32_t p,int32_t q,bool *success){
	if(*success==false){
		//assert(0);
		return 0;
	}
	int n=p;
	while(p<q){
		if(tokens[q].type=='+'||tokens[q].type=='-'){
			return q;
		}
		else if(tokens[q].type=='*'||tokens[q].type=='/'){
			if(n<q){
				n=q;
			}
		}
		else if(tokens[q].type==')'){
			int k=p;
			int lp=0;
			int i=p;
			for(;i<q;i++){
				if(tokens[i].type=='(') lp++;
				else if(tokens[i].type==')') lp--;
				if(lp==1&&tokens[i].type=='(') p=i;
			}
			if(check_parentheses(p,q,success)){
				q=p;
				p=k;
			}
			else{
				p++;
			}
		}
		q--;
	}
	return n;
}

bool check_parentheses(int32_t p, int32_t q,bool *success){
	if(*success==false){
		//assert(0);
		return 0;
	}
	if(tokens[p].type=='('&&tokens[q].type==')'&&p<q){
		if(!check_valid(p,q)){
			*success=false;
			return false;
		}
		int lp=0;
		while(p<q){
			if(tokens[p].type=='(') lp++;
			else if(tokens[p].type==')') lp--;
			if(lp==0) return false;
			p++;
		}
		return true;
	}
	else{
		return false;
	}		
}

bool check_valid(int32_t p,int32_t q){
	int op=0;
	int num=0;
	int lp=0;
	int rp=0;
	int i=p;
	for(;i<=q;i++){
		if(tokens[i].type==NUM) num++;
		else if(tokens[i].type=='(') lp++;
		else if(tokens[i].type==')') rp++;
		else if(tokens[i].type=='+'||tokens[i].type=='-'||tokens[i].type=='*'||tokens[i].type=='/')op++;
		if(rp>lp){
			return false;
		}
		if(op>num||num>op+1){
			return false;
		}
	}
	if(num-1==op&&lp==rp) return true;
	else {
		return false;
	}
}

void find_NEG(int32_t p,int32_t q){
	int i=p+1;
	for(;i<=q;i++){
		if(tokens[i].type==NUM&&tokens[i-1].type=='-'){
			if(i-1==p){
				tokens[i-1].type=NOTYPE;
				tokens[i].is_NEG=true;
			}
			else if(i-1>p){
				if(tokens[i-2].type=='('){
					tokens[i-1].type=NOTYPE;
					tokens[i].is_NEG=true;
				}
			}
		}
	}
}

int32_t expr(char *e, bool *success) {
	if(!make_token(e)) {
		*success = false;
		//assert(0);
		return 0;
	}
	
	find_NEG(0,nr_token-1);

	if(!check_valid(0,nr_token-1)){
		*success=false;
		//assert(0);
	}
	/* TODO: Insert codes to evaluate the expression. */
	else{
		return eval(0,nr_token-1,success);
	}
	return 0;
}

