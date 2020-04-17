#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ, TK_SIXTEEN, TK_TEN, TK_REG, TK_UEQ, TK_POINT

  /* TODO: Add more token types */

};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"==", TK_EQ},         // equal
  {"\\-",'-'},		// minus
  {"\\*",'*'},		// multiply
  {"\\/",'/'},		// devide
  {"!=", TK_UEQ},	// unequal
  {"\\(",'('},		//
  {"\\)",')'},		//
  {"0x[A-F0-9]+",TK_SIXTEEN},	// 16
  {"[0-9]+",TK_TEN},		// 10
  {"\\$[a-ehilpx]{2,3}", TK_REG},	//register
  {"&&" , '&'},		//and
  {"\\|\\|", '|'},	//or
  {"!",'!'},		//not
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

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
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

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
	if(rules[i].token_type == TK_NOTYPE)	continue;
        switch (rules[i].token_type) {
		case '+':
		case '-':
		case '*':
		case '/':
		case '(':
		case ')':
		case '&':
		case '|':
		case TK_UEQ:
		case '!':
		case TK_SIXTEEN:
		case TK_TEN:
		case TK_EQ:
		case TK_REG:
			{
				tokens[nr_token].type = rules[i].token_type;
				strncpy(tokens[nr_token].str,substr_start,substr_len);
				nr_token++;
			}break;
	//	default: assert(0);	//error.
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

bool check_parentheses(int p,int q){
	int left = 0;
	int right = 0;
	if(!(tokens[p].type == '(' && tokens[q].type == ')'))
	{
		return false;
	}
	p++;
	for(;p<q;p++){
		if(tokens[p].type == '('){
			left +=1;
		}
		else if(tokens[p].type == ')'){
			right += 1;
		}
		if(right > left + 1)
			assert(0);
		else if (right == left + 1)
			return false;
	}
	if(left != right )	return false;
	return true;	
}

uint32_t find_dominated_op(int p,int q){
	int op = p;
	int quote_match = 0;
	for (int i = p;i < q;i++){
		if(tokens[i].type == '(')	quote_match++;
		else if(tokens[i].type == ')')	quote_match--;
		else if(quote_match == 0){
			if(tokens[i].type == TK_EQ || tokens[i].type == TK_UEQ || tokens[i].type == '&' || tokens[i].type == '|'){
				op = i;
				return op;
			}
			if(tokens[i].type == TK_POINT){
				continue;
			}	
			else if(tokens[i].type == '+' || tokens[i].type == '-' || tokens[i].type == '*' || tokens[i].type == '/'){
				if(tokens[i].type == '+' || tokens[i].type == '-'){
					if(tokens[i].type == '-' && (tokens[i-1].type == '+' || tokens[i-1].type == '-' || tokens[i-1].type == '*' || tokens[i-1].type == '/'));

					else op = i;
				}
				else{
					if(tokens[op].type == '+' || tokens[op].type == '-');
					else op = i;
				}

			}
		}

	}
	return op;
}


uint32_t eval(int p,int q){
	if(p > q){
		/*Bad expression*/
		return false;
	}
	else if(p == q){
		/*Single token.
		 * For now this token should be a number.
		 * Return the value of the number.
		 */
		int number = 0;
		if(tokens[p].type == TK_SIXTEEN)	sscanf(tokens[p].str,"%x",&number);
		else if (tokens[p].type == TK_TEN)	sscanf(tokens[p].str,"%d",&number);
		else if (tokens[p].type == TK_REG){
			for(int i= 0; i < 4;i++)	tokens[p].str[i] = tokens[p].str[i+1];
			if(strcmp(tokens[p].str,"eip") == 0 )	number = cpu.eip;
		
			else{
				int i = 0;
				for(;i < 8;i++){
					if(strcmp(tokens[p].str,regsl[i]) == 0 ){
						number = cpu.gpr[i]._32;
						break;
					}
				}
			//number = cpu.gpr[i]._32;
			}
		}
		return number;
	}
	else if (check_parentheses(p,q) == true){
		/*The expression is surrounded by a matched pair of parentheses.
		 * If that is the case, just throw away the parentheses.
		 */
		return eval(p+1,q-1);
	}
	else{
		/*We should do more things here.*/
		int op,val1,val2;
		op = find_dominated_op(p,q);
		if(op == p && tokens[p].type == TK_POINT)	//*
			return  vaddr_read(eval(p+1,q),4);
		if(op == p && tokens[p].type == '-')		//-
			return  -eval(op+1,q);
		if(op == p && tokens[p].type == '!')		//!
			return !eval(op+1,q);
		val1 = eval(p,op - 1);
		val2 = eval(op+1,q);
		switch(tokens[op].type){
			case '+':return val1 + val2;
			case '-':return val1 - val2;
			case '*':return val1 * val2;
			case '/':return val1 / val2;
			case TK_EQ: return val1 == val2;
			case TK_UEQ:return val1 != val2;
			case '&':return val1 & val2;
			case '|':return val1 | val2;
			default : assert(0);
		}
	}
//	return 0;

}


uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
 // TODO();
  int p = 0, q = nr_token - 1;
  
  for (int i = 0;i < nr_token;i++)
  {
	if(tokens[i].type == '*'&&(i == 0 || tokens[i-1].type == '+' || tokens[i-1].type == '-' || tokens[i-1].type == '/'))
	{
		tokens[i].type = TK_POINT;
	}
  }
  return eval(p,q);

  //return 0;
}
