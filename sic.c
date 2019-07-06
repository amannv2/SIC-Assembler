#include<stdio.h>
#include<string.h>
#include "sicHeader.h"

int main()
{
	FILE *fp, *finalFile;

	char *token, sub[5];
	char del[] = " 	";					//delims to tokeenize instrn
	char buff[25], tempObj[10], finalObj[10], instr[5][10];
	char mnemonics[30][10], opcodes[30][3];
	char labelArr[100][10], opcodeArr[100][10], operandArr[100][10], objCode[100][7];
	char directives[6][10] = {"BYTE", "END", "RESB", "RESW", "START", "WORD"};	//should be in sorted order

	int locs = 0X0000, initLoc = 0X00, retLoc = 0X0000, temp = 0X000;
	int i, retOp = 0, retLabel = 0, retCheck = 0, maxOpcode = 0, flag = 0, locArray[100], fieldCount = 0, instrCount = 0;

	//allow user to enter the src filE
	fp = fopen("SIC.txt", "r");


	fscanf(fp, "%s", &labelArr[instrCount]);
	fscanf(fp, "%*c%s", &opcodeArr[instrCount]);


	//strcpy(progName, label);					//keep a copy of program name

	if(strcmp(opcodeArr[0], "START") != 0)
	{
		printf("\nIllegal Satrt of Program.\n");
		return 0;
	}

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

		//check for comments
		if(instr[i][0] == '%')
			continue;

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

	createSymTab(labelArr, locArray, instrCount);			//creating Symbol Table

	printf("\nPASS 1 completed\n");

	//	displayAll(labelArr, opcodeArr, operandArr, objCode, locArray, instrCount);

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
				if(temp == -1)				//for BYTE X'--'
				{
					for(retOp = 2; retOp < (strlen(operandArr[i]) - 1); retOp++)
					{
						sub[++temp] = operandArr[i][retOp];
					}
					sub[strlen(operandArr[i]) - 3] = '\0';	//did this to fix a bug(u/k char was appended to string)
					strcpy(objCode[i], sub);
				}
				else
					sprintf(objCode[i], "%06X", temp);
			}
			else
				strcpy(objCode[i], "");			//no opcode for RESW/RESB/..
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

					strcpy(objCode[i], opcodes[retOp]);//store the objcode of opcode field
					sprintf(tempObj, "%0X", temp);	//convert hex to str
					strcat(objCode[i], tempObj);
					//				printf("\nObj Code: %s", objCode[i]);
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

	//Object Program Generation

	printf("\nGenerating Object Program..\n");

	createObjProg(labelArr[0], opcodeArr, locArray, objCode, instrCount);

	return 0;
}
