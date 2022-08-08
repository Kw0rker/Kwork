
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
//x=(-1)^s*2^e*m \
//y=(-1)^s*2^e*m \	
// x/y=(2*((s1-s2)%2))*2^(e1-e2)*(m1/m2)