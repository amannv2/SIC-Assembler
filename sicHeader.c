#include<stdio.h>
#include<string.h>
#include "sicHeader.h"

void createObjProg(char *proName, char opcodeArr[][10], int locArray[], char objCode[][7], int instrCount)
{
	//choose a dynamic name
	FILE *fp;
	int i, count = 0, new = 1;
	unsigned long locFP = 0, curFP = 0;
	char buff[100];
	fp = fopen("objectProgram.txt", "w");			//creating objProgram file


	//write Header Record
	fprintf(fp, "H^%-6s^%06X^%06X", proName, locArray[1], locArray[instrCount - 2] - locArray[1] + 1);

	//write Text Record
	fprintf(fp, "\nT");
	for(i = new; i < instrCount - 1; i++)
	{

		if((strcmp("RESW", opcodeArr[i]) == 0) || ((strcmp("RESB", opcodeArr[i])) == 0))
		{
			if(count == 0)
				continue;
			count = 0;
			fprintf(fp, "\nT");
		}

		else if((count + strlen(objCode[i])) > 60)
		{
			curFP = ftell(fp);
			
			//update length
			fseek(fp, locFP - 2, SEEK_SET);

			fprintf(fp, "%0X", (count/2));
			//fprintf(fp, "HELLLOOOO");

			count = 0;

			fseek(fp, curFP, SEEK_SET);
			fprintf(fp, "\nT^%06X^--", locArray[i]);

			locFP = ftell(fp);

			fprintf(fp, "^%s", objCode[i]);
			count += strlen(objCode[i]);
		}

		else
		{
			if(count == 0)
			{	fprintf(fp, "^%06X^--", locArray[i]);
				locFP = ftell(fp);
			}

			count += strlen(objCode[i]);
			fprintf(fp, "^%s", objCode[i]);
			
			curFP = ftell(fp);

			//update length
			fseek(fp, locFP - 2, SEEK_SET);
			fprintf(fp, "%02X", (count/2));
			fseek(fp, curFP, SEEK_SET);
		}
	}

	fprintf(fp, "\nE^%06X", locArray[1]);


	fclose(fp);

	printf("\nObject Program: \n\n");
	//display
	fp = fopen("objectProgram.txt", "r");
	while(fgets(buff, 100, fp) != NULL)
	{
		printf("%s", buff);
	}
	fclose(fp);
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
			//printf("\nObj Code: ");
			for(i = 2; i < max; i++)
			{
				temp *= 16*16;
				temp += (int)operand[i];
			}
			return temp;
		}
		else if(operand[0] == 'X')
			return -1;
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

	printf("\n\nLocation\tLabel\tOpcode\tOperand\tObject Code\n");
	for(i = 0; i < instrCount; i++)      
	{
		if(strcmp(labelArr[i], "XXXX") == 0)
			printf("\n%0X\t\t\t%s\t%s\t%s\n", locArray[i], opcodeArr[i], operandArr[i], objCode[i]);
		else
			printf("\n%0X\t\t%s\t%s\t%s\t%s\n", locArray[i], labelArr[i], opcodeArr[i], operandArr[i], objCode[i]);
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
			return 0;
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
