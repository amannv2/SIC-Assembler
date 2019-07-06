#define sicHeader

int byteWordObj(char *operand, int retOp);
int getOpcodes(char mnemonics[][10], char opcodes[][3]);
int search(char array[][10], int min, int max, char *search);
void createSymTab(char labelArray[][10], int locArray[], int count);
void createObjProg(char *proName, char opcodeArr[][10], int locArray[], char objCode[][7], int instrCount);
void displayAll(char labelArr[][10], char opcodeArr[][10], char operandArr[][10], char objCode[][7], int locArray[], int instrCount);

