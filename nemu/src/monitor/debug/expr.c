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
void find_DRpointer(int32_t p,int32_t q);
bool is_Uop(int32_t p,int32_t q);

enum {
	NOTYPE = 256, EQ, NUM, HEX, AND, OR, NEQ, NOT, DRp, NEG, REG
	// REG_EAX=0, REG_ECX=1, REG_EDX=2, REG_EBX=3, REG_ESP=4, REG_EDP=5, REG_ESI=6, REG_EDI=7, REG_EIP=8
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
	{"\\b[0-9]+\\b", NUM},			    //number
	{"-", '-'},						//subtraction
	{"\\*", '*'},					//multiplication
	{"/", '/'},						//division
	{"\\(", '('},					//LPAREN
	{"\\)", ')'},					//RPAREN
	// {"\\$eax", REG_EAX}, 			//eax
	// {"\\$ecx", REG_ECX}, 			//ecx
	// {"\\$edx", REG_EDX}, 			//edx
	// {"\\$ebx", REG_EBX}, 			//ebx
	// {"\\$esp", REG_ESP}, 			//esp
	// {"\\$edp", REG_EDP}, 			//edp
	// {"\\$esi", REG_ESI}, 			//esi
	// {"\\$edi", REG_EDI}, 			//edi
	// {"\\$eip", REG_EIP}, 			//eip
	{"\\$(eax|ecx|edx|ebx|esp|ebp|esi|edi|eip)",REG},
	{"\\b0x[0-9A-Fa-f]+\\b",HEX},	//hexadecimal-number
	{"&&",AND},						//and
	{"\\|\\|",OR},						//or
	{"!=",NEQ},				//not equal
	{"!",NOT},				//not
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
	// bool is_NEG;
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

				//Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);
				position += substr_len;

				/* TODO: Now a new token is recognized with rules[i]. Add codes
				 * to record the token in the array `tokens'. For certain types
				 * of tokens, some extra actions should be performed.
				 */

				
				if(nr_token>31) return false;
				else{
					if(substr_len>31&&(rules[i].token_type==NUM||rules[i].token_type==HEX)) {
						//assert(0);
						return false;
					}
					switch(rules[i].token_type) {
						case NOTYPE:break;
						case REG:
						case NUM:
						// {						
						// 	tokens[nr_token].type=rules[i].token_type;
						// 	// tokens[nr_token].is_NEG=false;
						// 	strncpy(tokens[nr_token].str,substr_start,substr_len);
						// 	tokens[nr_token].str[substr_len] = '\0'; //这里有一个溢出bug
						// 	nr_token++;
						// 	break;
						// }
						case HEX:
						{
							tokens[nr_token].type=rules[i].token_type;
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
						case AND:
						case OR:
						case NEQ:
						case EQ:
						case NOT:
						{
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
		uint32_t val;
		if (tokens[p].type == NUM) {
			val=strtoul(tokens[p].str,NULL,10);
			return val;
		}
		else if(tokens[p].type==HEX){
			val=strtoul(tokens[p].str,NULL,16);
			return val;
		}
		else if(tokens[p].type==REG){
			if(strcmp(tokens[p].str,"$eax")==0){
				return cpu.eax;
			}
			else if(strcmp(tokens[p].str,"$ecx")==0){
				return cpu.ecx;
			}
			else if(strcmp(tokens[p].str,"$edx")==0){
				return cpu.edx;
			}
			else if(strcmp(tokens[p].str,"$ebx")==0){
				return cpu.ebx;
			}
			else if(strcmp(tokens[p].str,"$esp")==0){
				return cpu.esp;
			}
			else if(strcmp(tokens[p].str,"$ebp")==0){
				return cpu.ebp;
			}
			else if(strcmp(tokens[p].str,"$esi")==0){
				return cpu.esi;
			}
			else if(strcmp(tokens[p].str,"$edi")==0){
				return cpu.edi;
			}
			else if(strcmp(tokens[p].str,"$eip")==0){
				return cpu.eip;
			}
		}
		else{
			*success = false;
    		return 0;
		}

		// if(!tokens[p].is_NEG){
		// 	// printf("%d\n",val);

		// }
		// else{
		// 	//printf("%d\n",0-val);
		// 	return 0-val;
		// }
	}
	else if(check_parentheses(p, q,success) == true){
		return eval(p+1,q-1,success);
	}
	else{
		if(*success==false){
			return 0;
		}
		if(is_Uop(p,q)){
			switch (tokens[p].type)
			{
			case NOT: return !eval(p+1,q,success);
			case DRp: return swaddr_read(eval(p+1,q,success),4);
			case NEG: return 0-eval(p+1,q,success);
			default: assert(0);break;
			}
		}
		else{
			int op=find_op(p,q,success);
			// printf("op:%d\n",op);
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
			case AND: return val1&&val2;
			case OR:  return val1||val2;
			case EQ:  return val1==val2;
			case NEQ: return val1!=val2;
			default: {
				printf("%d\n",tokens[op].type);	
				assert(0);
					return 0;
				}
			}
		}		
	}
	return 0;
}

int32_t find_op(int32_t p,int32_t q,bool *success){
	if(*success==false){
		//assert(0);
		return 0;
	}
	bool p1=false;
	bool p2=false;
	bool p3=false;
	int n1=p;
	int n2=p;
	int n3=p;
	while(p<q){
		if(tokens[q].type==AND||tokens[q].type==OR){
			return q;
		}
		else if(tokens[q].type==EQ||tokens[q].type==NEQ){
			p1=true;
			if(n1<q){
				n1=q;
			}
		}
		else if(tokens[q].type=='+'||tokens[q].type=='-'){
			p2=true;
			if(n2<q){
				n2=q;
			}
		}
		else if(tokens[q].type=='*'||tokens[q].type=='/'){
			p3=true;
			if(n3<q){
				n3=q;
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
	if(p1) return n1;
	else if(p2) return n2;
	else return n3;
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
		if(tokens[i].type==NUM||tokens[i].type==REG||tokens[i].type==HEX) num++;
		else if(tokens[i].type=='(') lp++;
		else if(tokens[i].type==')') rp++;
		else if(tokens[i].type=='+'||tokens[i].type=='-'||tokens[i].type=='*'||tokens[i].type=='/'||
				tokens[i].type==AND||tokens[i].type==OR||tokens[i].type==EQ||tokens[i].type==NEQ)op++;
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
	int i=p;
	for(;i<=q;i++){
		if(tokens[i].type=='-'&&(i==0||tokens[i-1].type=='('||tokens[i-1].type==NOT||tokens[i-1].type==DRp))
		{
			tokens[i].type=NEG;
		}
	}
}

void find_DRpointer(int32_t p,int32_t q){
	int i=p;
	for(; i<=q; i++) {
		if(tokens[i].type == '*' && (i == 0||tokens[i-1].type=='(' || tokens[i-1].type=='+'||tokens[i-1].type=='-'||
			tokens[i-1].type=='/'||tokens[i-1].type==AND||tokens[i-1].type==OR||tokens[i-1].type==NOT||
			tokens[i-1].type==NEQ||tokens[i-1].type==EQ||tokens[i-1].type==NEG||tokens[i-1].type==DRp) ) 
			{
				tokens[i].type = DRp;
			}
	}
}

bool is_Uop(int32_t p,int32_t q){
	// printf("%d\t%d\n",p,q);
	if(tokens[p].type==NEG||tokens[p].type==DRp||tokens[p].type==NOT){
		if(q==p+1||(tokens[p+1].type=='('&&tokens[q].type==')')||(tokens[p+1].type==NEG||tokens[p+1].type==DRp||tokens[p+1].type==NOT)){
			// printf("true\n");
			return true;
		}
	}
	return false;
}

uint32_t expr(char *e, bool *success) {
	if(!make_token(e)) {
		*success = false;
		//assert(0);
		return 0;
	}
	
	find_NEG(0,nr_token-1);
	find_DRpointer(0,nr_token-1);

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

