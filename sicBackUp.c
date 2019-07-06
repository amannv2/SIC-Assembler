#include<stdio.h>
#include<string.h>
//#include<stdlib.h>

int byteWordObj(char *operand, int retOp);
int getOpcodes(char mnemonics[][10], char opcodes[][3]);
int search(char array[][10], int min, int max, char *search);
void createSymTab(char labelArray[][10], int locArray[], int count);
void displayAll(char labelArr[][10], char opcodeArr[][10], char operandArr[][10], char objCode[][7], int locArray[], int instrCount);

int main()
{
	FILE *fp, *finalFile;

	char *token;
	char del[] = " 	";					//delims to tokeenize instrn
	char buff[25], tempObj[10], finalObj[10], instr[5][10];
	char mnemonics[30][10], opcodes[30][3];
	char labelArr[100][10], opcodeArr[100][10], operandArr[100][10], objCode[100][7];
	char directives[6][10] = {"BYTE", "END", "RESB", "RESW", "START", "WORD"};	//should be in sorted order

	int locs = 0X0000, initLoc = 0X00, retLoc = 0X0000, temp = 0X000;
	int i, retOp = 0, retLabel = 0, retCheck = 0, maxOpcode = 0, flag = 0, locArray[100], fieldCount = 0, instrCount = 0;

	//allow user to enter the src file
	fp = fopen("SIC.txt", "r");


	fscanf(fp, "%s", &labelArr[instrCount]);
	fscanf(fp, "%*c%s", &opcodeArr[instrCount]);

	//strcpy(progName, label);					//keep a copy of program name

	if(strcmp(opcodeArr[0], "START") == 0)
	{
		printf("\nStarting PASS 1\n");
		fscanf(fp, "%*c%0X", &locs);
		sprintf(operandArr[instrCount], "%0X", locs);

		locArray[instrCount++] = locs;

		initLoc = locs;						//keep a copy of starting location of program

		fgets(buff, 10, fp);					//for \n that got left out in previous scan
		buff[0] = '\0';						//reset buffer

		while(1)
		{
			flag = 0;					//reset flag for obvious reasons
			i = 0;
			fieldCount = 0;

			if(fgets(buff, 25, fp) == NULL)			//scan whole instruction
				break;

			strtok(buff, "\n"); 				//consume trailing \n

			token = strtok(buff, del);			//tokenize the instruction on white space delims
			while(token != NULL)
			{
				strcpy(instr[i++], token);
				token = strtok(NULL, del);
				fieldCount++;
			}

			if(fieldCount < 3 && fieldCount > 0)		//case 1: label field is blank
			{
				strcpy(labelArr[instrCount], "XXXX");
				strcpy(opcodeArr[instrCount], instr[0]);
				strcpy(operandArr[instrCount], instr[1]);
			}
			else if(fieldCount == 3)			//case 2: all fields are present
			{
				strcpy(labelArr[instrCount], instr[0]);
				strcpy(opcodeArr[instrCount], instr[1]);
				strcpy(operandArr[instrCount], instr[2]);
			}
			else						//case 3: if 0 or more than 3 fields are present
			{
				printf("Illegal instruction at Line %d.", instrCount);
				return 0;
			}

			locArray[instrCount++] = locs;			//update location array


			for(i = 0; i < 6; i++)				//calculate location
			{
				if(strcmp(opcodeArr[instrCount - 1], directives[i]) == 0)	//check if opcode is Assembler Directive
				{
					retLoc = check(i, operandArr[instrCount - 1]);
					if(retLoc != -1)
						locs += retLoc;		//update location
					flag = 1;			//set flag 'cuz no need to update location again
					break;
					return 0;
				}
			}

			if(!flag)
				locs += 3;				//for normal case increase location by 3 Bytes

			instr[0][0] = '\0';
			instr[1][0] = '\0';
			instr[2][0] = '\0';
		}
		fclose(fp);
	}

	createSymTab(labelArr, locArray, instrCount);			//creating Symbol Table

	printf("\nPASS 1 completed\n");

	//	displayAll(labelArr, opcodeArr, operandArr, locArray, instrCount);

	printf("\nStarting PASS 2\n");


	maxOpcode = getOpcodes(mnemonics, opcodes);			//load opcode table
	if(maxOpcode == -1)
		return 0;
	

	for(i = 1; i < instrCount; i++)
	{
		retOp = search(mnemonics, 0, maxOpcode, opcodeArr[i]);	//check if opcode is present in mnemonic table

		if(retOp == -1)
		{
			retOp = search(directives, 0, 6, opcodeArr[i]);	//if not then check in directives
			if(retOp == -1)
			{
				printf("\nInvalid Instruction at Line %d\n", i);	//if not then illegal instruction
				return 0;
			}
			//case 0 Byte, 5 Word				//else perform task wrt to directive case
			if(retOp == 0 || retOp == 5)
			{
				temp = byteWordObj(operandArr[i], retOp);
				sprintf(objCode[i], "%06X", temp);
				//printf("-------------->%s", objCode[i]);
			}
			else
				strcpy(objCode[i], "");
		}
		else
		{							//if present in mnemonics, calculate object code
			if(strcmp(opcodeArr[i], "RSUB") == 0)
				strcpy(objCode[i], "4C0000");		//special case for RSUB
			else
			{
				retLabel = linSearch(labelArr, operandArr[i], instrCount);
				if(retLabel == -1)
				{
					//case for illegal and Index Addr
					retCheck = indexAddr(labelArr, operandArr[i], instrCount, i);
					if(retCheck == -1)
					{
						printf("\nInvalid Instruction at Line %d\n", i);
						return 0;
					}
					temp = locArray[retCheck];
					temp |= 1 << 15;		//SET the X bit
					//check opcode then merge with addr of retVal
					sprintf(tempObj, "%0X", temp);

					strcpy(objCode[i], opcodes[retOp]);
					sprintf(tempObj, "%0X", temp);
					strcat(objCode[i], tempObj);
					printf("\nObj Code: %s", objCode[i]);
					continue;

				}
				strcpy(objCode[i], opcodes[retOp]);
				sprintf(tempObj, "%0X", locArray[retLabel]);
				strcat(objCode[i], tempObj);
			}
		}

		tempObj[0] = '\0';
		finalObj[0] = '\0';
	}
	displayAll(labelArr, opcodeArr, operandArr, objCode, locArray, instrCount);
	return 0;
}

int byteWordObj(char *operand, int retOp)
{
	int temp = 0X000000, i = 0;
	int max = strlen(operand) - 1;
	//WORD
	if(retOp == 5)
	{
		temp += atoi(operand);
		return temp;
	}

	//BYTE
	if(retOp == 0)
	{
		if(operand[0] == 'C')
		{
			printf("\nObj Code: ");
			for(i = 2; i < max; i++)
			{
				temp *= 16*16;
				temp += (int)operand[i];
			}
			return temp;
		}
		else if(operand[0] == 'X')
		{
			for(i = 2; i < max; i++)
			{
				temp *+ 16;
				temp += (int)operand[i];
			}
			printf("-------------->%0X", temp);
			return temp;
		}
		printf("\n");
	}
}

int indexAddr(char labelArr[][10], char *operand, int max, int curr)
{
	int count = 0, retVal = -1;
	char *token;
	char temp[3][7];

	token = strtok(operand, ",");
	while(token != NULL)
	{
		strcpy(temp[count], token);
		token = strtok(NULL, ",");
		count++;
	}
	if(count > 2 || (strcmp(temp[count - 1], "X") != 0))
	{
		return -1;
		//throw error
	}
	retVal = linSearch(labelArr, temp[0], max);
	if(retVal == -1)
		return -1;
	else
		return retVal;
}


int linSearch(char array[][10], char *search, int max)
{
	int i = 0;
	for(i = 1; i < max; i++)
	{
		if(strcmp(array[i], search) == 0)
		{
			return i;
		}
	}
	return -1;
}

int search(char array[][10], int min, int max, char *search)
{
	int mid;

	while(min <= max)
	{
		mid = (min + max)/2;
		if(strcmp(array[mid], search) < 0)
			min = mid + 1;
		else if(strcmp(array[mid], search) > 0)
			max = mid - 1;
		else
			return mid;
	}
	return -1;
}

int getOpcodes(char mnemonics[][10], char opcodes[][3])
{
	int i = 0, j = 0;
	FILE *fp;

	if((fp = fopen("OpcodeTable.txt", "r")) == NULL)
	{
		printf("Can't load the Opcode Table! Exiting.");
		return -1;
	}

	fscanf(fp, "%[^-]s", &mnemonics[i]);
	fscanf(fp, "%*c%[^\n]s", &opcodes[i++]);

	while(1)
	{
		fscanf(fp, "%*c%[^-]s", &mnemonics[i]);
		if(fscanf(fp, "%*c%[^\n]s", &opcodes[i]) == EOF)
			break;
		i++;
	}

	fclose(fp);
	return i;
	//	for(j = 0; j < i; j++)
	//		printf("%s\t-\t%s\n", mnemonics[j], opcodes[j]);
}


void displayAll(char labelArr[][10], char opcodeArr[][10], char operandArr[][10], char objCode[][7], int locArray[], int instrCount)
{
	int i;

	printf("\nLocation\tLabel\tOpcode\tOperand\tObject Code\n");
	for(i = 0; i < instrCount; i++)      
	{
		if(strcmp(labelArr[i], "XXXX") == 0)
			printf("\n%0X\t\t%s\t%s\t%s\n", locArray[i], opcodeArr[i], operandArr[i], objCode[i]);
		else
			printf("\n%0X\t%s\t%s\t%s\t%s\n", locArray[i], labelArr[i], opcodeArr[i], operandArr[i], objCode[i]);
	}
}


void createSymTab(char labelArr[][10], int locArr[], int count)
{
	FILE *fp;
	int i = 0;

	fp = fopen("SymTab.txt", "w");

	printf("\nGenerating Symbol Table..\n");

	for(i = 1; i < count; i++)					// i = 1 because first instruction is program's name and location to start
	{
		if(strcmp(labelArr[i], "XXXX") != 0)			//skip if no label
			fprintf(fp, "%0X-%s\n", locArr[i], labelArr[i]);
	}

	fclose(fp);

	printf("\nSymbol Table is created.\n");
}

int check(int i, char *operand)						//look for respective Assembler Directive
{
	int num = 0X0000;

	switch(i)
	{
		case 0: //BYTE
			if(operand[0] == 'C')
			{
				return strlen(operand) - 3;		//return total num of chars i.e, 1B for each char
			}
			else if(operand[0] == 'X')
			{
				if((strlen(operand) - 3) % 2 == 0)
					return (strlen(operand) - 3)/2;	//X means HEX i.e, 4 bits for each bit hence dividing by 2
				else
					return ((strlen(operand) - 3)/2) + 1;
			}
			else
				printf("Error at %s", operand);		//else show error
			return -1;
			break;

		case 1: //printf("END");
			break;

		case 2: //RESB
			num = atoi(operand);
			return num;					//operand Bytes are reserved
			break;

		case 3: //RESW
			num = atoi(operand);
			return num * 3;					//operand * 3 Bytes(i. e., 1 Word) are reserved
			break;

		case 4: //printf("START");
			break;

		case 5: //WORD
			return 3;					//1 Word = 3 Bytes
			break;
	}
}
