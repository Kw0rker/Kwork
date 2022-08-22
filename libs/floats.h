
#define DIV_FLOAT(X,Y)\
//x=(-1)^s*2^e*m \
//y=(-1)^s*2^e*m \	
//x/y=(-1)^(s1-s2)*2^(e1-e2)*(m1/m2)

#define GET_SIGN_BIT(VAR_HASH)\
LOAD(VAR_HASH,'V')\
CREATE_INSTRUCTION(LOG_LES,0)\

#define GET_EXPONENT(VAR_HASH)\
LOAD(0x7f800000,'C')\
BIT_AND(VAR_HASH)
#define GET_MANTISA(VAR_HASH)
LOAD(0x007fffff,'C')\
FIND_OR_CREATE(VAR_HASH,'V')\
CREATE_INSTRUCTION(BIT_AND,symbolTable[ad]->location)\

#define SET_SIGN_BIT(VAR_HASH)\
LOAD(0x7fffffff,'C')
FIND_OR_CREATE(VAR_HASH,'V')\
CREATE_INSTRUCTION(BIT_XOR,symbolTable[ad]->location)\

#define SET_EXPONENT(VAR_HASH)\
FIND_OR_CREATE(VAR_HASH,'V')\
CREATE_INSTRUCTION(BIT_OR,symbolTable[ad]->location)\

#define SET_MANTISA(VAR_HASH)\
FIND_OR_CREATE(VAR_HASH,'V')\
CREATE_INSTRUCTION(BIT_OR,symbolTable[ad]->location)\

#define DIV_FLOAT(X,Y)\
/*x/y=(2*((s1-s2)%2))*2^(e1-e2)*(m1/m2)*/ \
GET_SIGN_BIT(Y)\
CREATE_INSTRUCTION(STORE,MAX_CODE_SIZE - (total_vars+1))\
GET_EXPONENT(X)\
CREATE_INSTRUCTION(STORE,MAX_CODE_SIZE - (total_vars+2))\
GET_EXPONENT(Y)\
CREATE_INSTRUCTION(STORE,MAX_CODE_SIZE - (total_vars+3))\
GET_MANTISA(X)\
CREATE_INSTRUCTION(STORE,MAX_CODE_SIZE - (total_vars+4))\
GET_MANTISA(Y)\
CREATE_INSTRUCTION(STORE,MAX_CODE_SIZE - (total_vars+5))\
LOAD(X)\
CREATE_INSTRUCTION(SUB,MAX_CODE_SIZE - (total_vars+1))\
FIND_OR_CREATE(2,'C')\
CREATE_INSTRUCTION(MOD,symbolTable[ad]->location)\
CREATE_INSTRUCTION(MUL,symbolTable[ad]->location)\
FIND_OR_CREATE(1,'C')\
CREATE_INSTRUCTION(SUB,symbolTable[ad]->location)\
CREATE_INSTRUCTION(STORE,MAX_CODE_SIZE - (total_vars+6))\
CREATE_INSTRUCTION(LOAD,MAX_CODE_SIZE - (total_vars+2))\
CREATE_INSTRUCTION(SUB,MAX_CODE_SIZE - (total_vars+3))\
CREATE_INSTRUCTION(STORE,MAX_CODE_SIZE - (total_vars+7))\
/*result of pow*/\
CREATE_INSTRUCTION(LOAD,symbolTable[ad]->location)\
CREATE_INSTRUCTION(STORE,MAX_CODE_SIZE - (total_vars+8))\
/*loop counter*/\
CREATE_INSTRUCTION(LOAD,symbolTable[ad]->location)\
CREATE_INSTRUCTION(STORE,MAX_CODE_SIZE - (total_vars+9))\
/*loop loading*/\
CREATE_INSTRUCTION(LOAD,MAX_CODE_SIZE - (total_vars+9))\
/*sub pow*/\
CREATE_INSTRUCTION(SUB,MAX_CODE_SIZE - (total_vars+7))\
/*check if less than zero*/\
CREATE_INSTRUCTION(LOG_LESS,0)\
/*jump to the end of loop if false*/\
CREATE_INSTRUCTION(BRANCHZERO,function_pointer+(local_comands+8))\
/*multiply result by 2 and store in result*/\
CREATE_INSTRUCTION(LOAD,MAX_CODE_SIZE - (total_vars+8))\
FIND_OR_CREATE(2,'C')\
CREATE_INSTRUCTION(MUL,symbolTable[ad]->location)\
CREATE_INSTRUCTION(STORE,MAX_CODE_SIZE - (total_vars+8))\
/*increment counter by one*/\
CREATE_INSTRUCTION(LOAD,MAX_CODE_SIZE - (total_vars+9))\
FIND_OR_CREATE(1,'C')\
CREATE_INSTRUCTION(ADD,symbolTable[ad]->location)\
CREATE_INSTRUCTION(STORE,MAX_CODE_SIZE - (total_vars+9))\
/*jump tp loop loading instruction*/\
CREATE_INSTRUCTION(BRANCH,function_pointer+(local_comands-10))\
/*load result of pow loop (2^pow)*/\
CREATE_INSTRUCTION(LOAD,MAX_CODE_SIZE - (total_vars+8))\
/*multiply by mantisa of first number*/\
CREATE_INSTRUCTION(MUL,MAX_CODE_SIZE - (total_vars+4))\
/*multiply by 2(s1-s2)%2*/\
CREATE_INSTRUCTION(MUL,MAX_CODE_SIZE - (total_vars+6))\
/*divide by mantisa of second number*/\
CREATE_INSTRUCTION(DIV,MAX_CODE_SIZE - (total_vars+5))\