#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <ctype.h>
#include <dir.h>
#include <string.h>


void main(int argc, char **argv)
{
	FILE *fp1;
	char filename[13]="TESTA.TMP";
	int i,num=100;
	register unsigned long j;
	unsigned long mb32=32L*1024L*1024L;
	int n;
	char x;

	if (argc == 2)
	{
		i=toupper(argv[1][0])-'A';
		setdisk(i);
	}

	chdir("\\");
	clrscr();
	while (x!=EOF)
	{
	strcpy(filename,"TEST");
	sprintf(&filename[4],"%3d",num++);
	strcat(filename,".TMP");
	fp1=fopen(filename,"wb");
	printf("\n%c:\\%s  2GB FILE 64 BLOCKS OF 32MB EACH\n[%64s]\r[",getdisk()+ 'A',filename," ");
	for(i=0;i<64;i++)
	{
		for (j=0;j<mb32;j++)
		{
			x=putc(NULL,fp1);
			if (x==EOF)
				j=mb32;
		}
		printf("%c",177);
		if (x==EOF)
			i=64;
	}
	fclose(fp1);
	}
	printf("\n\nDONE\n");
	do
	{
		strcpy(filename,"TEST");
		sprintf(&filename[4],"%3d",--num);
		strcat(filename,".TMP");
		printf("\nDeleting File - %s",filename);
		remove(filename);
	} while (num>100);
	printf("\n\nClearing completed\n");
}