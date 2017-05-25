#include <stdio.h>
#include <ctype.h>
#include <string.h>


int main(int argc, char **argv)
{
	FILE *fp1;
	char filename[13]="TESTA.TMP";
	int i,num=100;
	register unsigned long j;
	unsigned long mb32=32L*1024L*1024L;
	int n;
	char x;


  system("clear");
	while (x!=EOF)
	{
	strcpy(filename,"TEST");
	sprintf(&filename[4],"%3d",num++);
	strcat(filename,".TMP");
	fp1=fopen(filename,"wb");
	fprintf(stderr,"\n%s  2GB FILE 64 BLOCKS OF 32MB EACH\n[%64s]\r[",filename," ");
	for(i=0;i<64;i++)
	{
		for (j=0;j<mb32;j++)
		{
			x=putc(0,fp1);
			if (x==EOF)
				j=mb32;
		}
		fprintf(stderr,"%c",'>');
		if (x==EOF)
			i=64;
	}
	fclose(fp1);
	}
	fprintf(stderr,"\n\nDONE\n");
	do
	{
		strcpy(filename,"TEST");
		sprintf(&filename[4],"%3d",--num);
		strcat(filename,".TMP");
		fprintf(stderr,"\nDeleting File - %s",filename);
		remove(filename);
	} while (num>100);
	fprintf(stderr,"\n\nClearing completed\n");
  return 0;
}
