/*
* jetcat (c)2004/GPL Oskar Schirmer, schirmer@scara.com
* measure, control and limit pipe throughput
*/

/*
* modifications done by Frank Stephen fra_step@yahoo.com
* now prints out sum of transferred data, only needed for g4l
* to show gauge/progress bar
*
*/

/* This is a rewrite of the above programs to just include the
* feature that is needed by g4l. Thus making it a little smaller.
* not sure if it makes it faster, but it seems to be as fast as
* any of my hard drives can read the data.
* this modification by Michael Setzer II mikes@kuentos.guam.net
*/

//#include <errno.h>
#include <stdio.h>
//#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>

int main(int argc, char* argv[])
{
  char c[8192];
  int n, n1;
  unsigned long time_offset = 0;
  float mbrate;
  float perc;
  struct timeval beginning;
  struct timeval tv;
  float bfsize, bfsizek;
  unsigned long long actsum;
//  unsigned
  long int thissec;
//    unsigned long int elapse;
  unsigned int elapse;
  register int count2=0;
  actsum=0;
  perc=0.0;
  gettimeofday(&beginning, NULL);
  sscanf(argv[4],"%f",&bfsize);
  bfsizek = bfsize/1024.0;
  thissec=beginning.tv_sec;
  while ((n = read(0, &c, 8192))) // read pipe.
  {
    n1= write(STDOUT_FILENO, &c, n);
    actsum+=8;
    count2++;
//    fprintf(stderr,"%d\n",n);
// If either Input goes to Zero for reading,
// or output goes to 0 when cleaning disk space
    if (n <= 0  || n1<=0)
    break;
    if (count2==512)
    {
      count2=0;
      gettimeofday(&tv, NULL);
      if ((tv.tv_sec % 5) ==0 && tv.tv_sec != thissec)
      {
        thissec=tv.tv_sec;
        if (tv.tv_sec < beginning.tv_sec)
        time_offset = 3600L * 24;
        perc=(actsum*100.0/bfsize);
        elapse=(long)(time_offset + tv.tv_sec - beginning.tv_sec);
        mbrate = (actsum/1024.0)/elapse;
        fprintf(stderr, "%.2f%% \t%12.2fMB of %.2fMB \ttime: %u:%02u:%02u  %.2fMB/sec\n",
        perc, actsum/1024.0, bfsizek, elapse / 3600, elapse / 60 % 60, elapse % 60, mbrate);
      }
    }
  }
  perc=100.0;
  elapse=(long)(time_offset + tv.tv_sec - beginning.tv_sec);
  mbrate = (actsum/1024.0)/elapse;
  fprintf(stderr, "%.2f%%\t%12.2fMB of %.2fMB \ttime: %u:%02u:%02u  %.2fMB/sec\n",
  perc, actsum/1024.0, bfsizek, elapse / 3600, elapse / 60 % 60, elapse % 60, mbrate);
  fprintf(stderr,"100\n");
  return 0;
}
