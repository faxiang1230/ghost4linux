/*
 * jetcat (c)2004/GPL Oskar Schirmer, schirmer@scara.com
 * measure, control and limit pipe throughput
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/poll.h>
#include <sys/time.h>

const static char *version = "1.3";

#define TEN19 10000000000000000000ULL

static char *rbuf;
static long freq = 1000;
static long bufsize = 0x10000;
static long highwater = 1;
static long lowwater = 1;
static long readsize = 8192;
static long writesize = 8192;
static long maxidata = 0;
static long maximsec = 0;
static long maxodata = 0;
static long maxomsec = 0;
static unsigned long long bytecount = (unsigned long long) -1;
static unsigned long long byteskip = 0;

static unsigned long long sumin = 0;
static unsigned long long sumout = 0;
static unsigned long long sumin2 = 0; /* 10^19 */
static unsigned long long sumout2 = 0;

static void usage(char *argv0)
{
  fprintf(stderr,
  "Usage: %s [OPTIONS]\n"
  "  transmit data from stdin to stdout, statistics go to stderr.\n"
  "  different restrictions (timing, amount) may be applied.\n"
  "available options are:\n"
  "  -h, --help\t\t\tprint usage info%s\n"
  "  -V, --version\t\t\tprint version%s (%s)\n"
  "  -f, --freq <msec>\t\tset measurement frequency in msec, 0 for none\n"
  "  -r, --maxread%sread%s%sfrom stdin\n"
  "  -w, --maxwrite%swrite%s%sto stdout\n"
  "  -B, --buffer <size>\t\tuse buffer of <size> (default %ld)\n"
  "  -L, --low%slow%s%ld)\n"
  "  -H, --high%shigh%s%ld)\n"
  "  -R, --read%sread%s%s%ld)\n"
  "  -W, --write%swrite%s%s%ld)\n"
  "  -c, --count <cnt>\t\ttransmit no more than <cnt> bytes\n"
  "  -s, --skip <cnt>\t\tskip <cnt> bytes from stdin\n"
  "<msec> may be appended one of 's', 'm' or 'h', <cnt> one of 'k', 'm' or 'g'\n"
  ,
  argv0,
  " and exit",
  " and exit", version,
  " <cnt> <msec>\t", " at most <cnt> bytes ", "per <msec> ",
  " <cnt> <msec>\t", " at most <cnt> bytes ", "per <msec> ",
  bufsize,
  " <cnt>\t\tbuffer ",  "water mark (default ", lowwater,
  " <cnt>\t\tbuffer ",  "water mark (default ", highwater,
  "size <cnt>\t\t", " at most <cnt> bytes ", "at once (default ", readsize,
  "size <cnt>\t\t", " at most <cnt> bytes ", "at once (default ", writesize);
}

static void statistics(struct timeval *tv,
        unsigned long long cntin,
        unsigned long long cntout)
{
  static unsigned long long statin = 0;
  static unsigned long long statout = 0;
  fprintf(stderr, "%ld.%06ld: bytes in=%llu, out=%llu\n",
    tv->tv_sec, tv->tv_usec,
    cntin - statin,
    cntout - statout);
  statin = cntin;
  statout = cntout;
}

static void summationin(unsigned long long cntin)
{
  static unsigned long long lastin = 0;
  unsigned long i;
  i = cntin - lastin;
  lastin = cntin;
  sumin += i;
  if (sumin >= TEN19) {
    sumin -= TEN19;
    sumin2 += 1;
  }
}

static void summationout(unsigned long long cntout)
{
  static unsigned long long lastout = 0;
  unsigned long o;
  o = cntout - lastout;
  lastout = cntout;
  sumout += o;
  if (sumout >= TEN19) {
    sumout -= TEN19;
    sumout2 += 1;
  }
}

static struct timeval beginning;

static void finalstat(void)
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  if (tv.tv_usec < beginning.tv_usec) {
    tv.tv_sec -= 1;
    tv.tv_usec += 1000000;
  }
  tv.tv_sec -= beginning.tv_sec;
  tv.tv_usec -= beginning.tv_usec;
  fprintf(stderr, "bytes read: ");
  if (sumin2 > 0) {
    fprintf(stderr, "%llu%019llu", sumin2, sumin);
  } else {
    fprintf(stderr, "%llu", sumin);
  }
  fprintf(stderr, ", written: ");
  if (sumout2 > 0) {
    fprintf(stderr, "%llu%019llu", sumout2, sumout);
  } else {
    fprintf(stderr, "%llu", sumout);
  }
  fprintf(stderr, ", time: %u:%02u:%02u.%03u\n",
    tv.tv_sec / 3600,
    tv.tv_sec / 60 % 60,
    tv.tv_sec % 60,
    tv.tv_usec / 1000);
}

static void signalhandler(int sig)
{
  switch (sig) {
  case SIGINT:
    fprintf(stderr, "\n");
    finalstat();
    break;
  }
  exit(0);
}

static char *timeunits = "SMH";
static long timemultipliers[] = {1000, 60000, 3600000};
static char *dataunits = "KMG";
static long datamultipliers[] = {1024, 1048576, 1073741824};

static long optamount(char *units, long *multipliers, char *opt, char *argv0)
{
  long a;
  char *endptr;
  char e;
  a = strtol(opt, &endptr, 0);
  e = *endptr;
  if (e != 0) {
    if (endptr[1] == 0) {
      e = toupper(e);
      while (*units != '\0') {
        if (*units == e) {
          long am = a * *multipliers;
          if (am / *multipliers != a) {
            fprintf(stderr, "integer overflow: %s\n", opt);
            usage(argv0);
            exit(1);
          }
          return am;
        }
        units += 1;
        multipliers += 1;
      }
    }
    fprintf(stderr, "unknown unit: %c\n", e);
    usage(argv0);
    exit(1);
  }
  return a;
}

static void readopts(int argc, char *argv[])
{
  int c;
  int a = argc-1;
  char **v = &argv[1];
  while (a > 0) {
    const static struct opt {
      char *olong;
      char oparms;
      char oshort;
    } opt[] = {
      {"help", 0, 'h'},
      {"version", 0, 'V'},
      {"freq", 1, 'f'},
      {"maxread", 2, 'r'},
      {"maxwrite", 2, 'w'},
      {"buffer", 1, 'B'},
      {"low", 1, 'L'},
      {"high", 1, 'H'},
      {"readsize", 1, 'R'},
      {"writesize", 1, 'W'},
      {"count", 1, 'c'},
      {"skip", 1, 's'},
      {NULL, 0, 0}};
    c = -1;
    if (v[0][0] != '-') {
      fprintf(stderr, "extra argument on line: %s\n", v[0]);
      usage(argv[0]);
      exit(1);
    }
    if (v[0][1] == '-') {
      const struct opt *o = &opt[0];
      while ((o->olong != NULL)
          && (strcmp(o->olong, &v[0][2]))) {
        o += 1;
      }
      if (o->olong == NULL) {
        fprintf(stderr, "%s%s\n", "unknown option: ", v[0]);
        usage(argv[0]);
        exit(1);
      }
      a -= 1;
      c = o->oshort;
      if (a < o->oparms) {
        fprintf(stderr, "%s%s\n", "not enough parameters with option ",
                &v[0][0]);
        usage(argv[0]);
        exit(1);
      }
      v += 1;
    } else {
      const struct opt *o = &opt[0];
      c = v[0][1];
      while ((o->olong != NULL)
          && (o->oshort != c)) {
        o += 1;
      }
      if (o->olong == NULL) {
        fprintf(stderr, "%s-%c\n", "unknown option: ", c);
        usage(argv[0]);
        exit(1);
      }
      if (v[0][2] != 0) {
        if (o->oparms == 0) {
          v[0] += 1;
          v[0][0] = '-';
        } else {
          v[0] += 2;
        }
      } else {
        v += 1;
        a -= 1;
      }
      if (a < o->oparms) {
        fprintf(stderr, "%s-%c\n", "not enough parameters with option ", c);
        usage(argv[0]);
        exit(1);
      }
    }
    switch (c) {
      case 'f':
        freq = optamount(timeunits, timemultipliers, v[0], argv[0]);
        v += 1;
        a -= 1;
        break;
      case 'r':
        maxidata = optamount(dataunits, datamultipliers, v[0], argv[0]);
        maximsec = optamount(timeunits, timemultipliers, v[1], argv[0]);
        if ((maxidata <= 0) || (maximsec <= 0)) {
          fprintf(stderr, "%s must not be less than 1\n",
            "maxread <cnt> or <msec>");
          exit(1);
        }
        v += 2;
        a -= 2;
        break;
      case 'w':
        maxodata = optamount(dataunits, datamultipliers, v[0], argv[0]);
        maxomsec = optamount(timeunits, timemultipliers, v[1], argv[0]);
        if ((maxodata <= 0) || (maxomsec <= 0)) {
          fprintf(stderr, "%s must not be less than 1\n",
            "maxwrite <cnt> or <msec>");
          exit(1);
        }
        v += 2;
        a -= 2;
        break;
      case 'B':
        bufsize = optamount(dataunits, datamultipliers, v[0], argv[0]);
        v += 1;
        a -= 1;
        break;
      case 'L':
        lowwater = optamount(dataunits, datamultipliers, v[0], argv[0]);
        v += 1;
        a -= 1;
        break;
      case 'H':
        highwater = optamount(dataunits, datamultipliers, v[0], argv[0]);
        v += 1;
        a -= 1;
        break;
      case 'R':
        readsize = optamount(dataunits, datamultipliers, v[0], argv[0]);
        v += 1;
        a -= 1;
        break;
      case 'W':
        writesize = optamount(dataunits, datamultipliers, v[0], argv[0]);
        v += 1;
        a -= 1;
        break;
      case 'c':
        bytecount = optamount(dataunits, datamultipliers, v[0], argv[0]);
        v += 1;
        a -= 1;
        break;
      case 's':
        byteskip = optamount(dataunits, datamultipliers, v[0], argv[0]);
        v += 1;
        a -= 1;
        break;
      case 'h':
        usage(argv[0]);
        exit(0);
      case 'V':
        fprintf(stderr, "%s\n", version);
        exit(0);
    }
  }
  if ((lowwater <= 0) || (highwater <= 0)) {
    fprintf(stderr, "%s must not be less than 1\n", "water mark");
    exit(1);
  }
  if ((lowwater > bufsize) || (highwater > bufsize)) {
    fprintf(stderr, "%s must not be greater than %s\n",
      "water mark", "buffer size");
    exit(1);
  }
  if (readsize <= 0) {
    fprintf(stderr, "%s must not be less than 1\n", "read size");
    exit(1);
  }
  if (writesize <= 0) {
    fprintf(stderr, "%s must not be less than 1\n", "write size");
    exit(1);
  }
  if (bufsize <= 0) {
    fprintf(stderr, "%s must not be less than 1\n", "buffer size");
    exit(1);
  }
  rbuf = malloc(bufsize);
  if (rbuf == NULL) {
    fprintf(stderr, "failed to allocate memory (%ld)\n", bufsize);
    exit(1);
  }
}

int main(int argc, char *argv[])
{
  int bufin, bufout, bufused;
  int tmostat, tmomaxi, tmomaxo, freqsec, frequsec, maxicnt, maxocnt,
        maxisec, maxiusec, maxosec, maxousec;
  struct pollfd pfd[2];
  struct timeval now, laststat, lastmaxi, lastmaxo;
  unsigned long long cntin, cntout;
  pfd[0].fd = STDIN_FILENO;
  pfd[0].events = POLLIN;
  pfd[1].fd = STDOUT_FILENO;
  pfd[1].events = POLLOUT;
  readopts(argc, argv);
  signal(SIGINT, (void *) (*signalhandler));
  signal(SIGPIPE, SIG_IGN);
  bufin = bufout = bufused = 0;
  cntin = cntout = 0;
  freqsec = freq / 1000;
  frequsec = (freq % 1000) * 1000;
  maxisec = maximsec / 1000;
  maxiusec = (maximsec % 1000) * 1000;
  maxosec = maxomsec / 1000;
  maxousec = (maxomsec % 1000) * 1000;
  gettimeofday(&beginning, NULL);
  laststat = beginning;
  lastmaxi = lastmaxo = laststat;
  tmomaxi = tmomaxo = tmostat = -1;
  maxicnt = (maxidata > 0) ? maxidata : -1;
  maxocnt = (maxodata > 0) ? maxodata : -1;
  if (bytecount == 0) {
    finalstat();
    return 0;
  }
  while (1) {
    struct pollfd *p;
    int nfds, tmo;
    pfd[0].revents = 0;
    if (((bufsize - bufused) >= highwater)
     && (maxicnt != 0)
     && (pfd[0].fd >= 0)
     && (bytecount != 0)) {
      p = &pfd[0];
      nfds = 1;
    } else {
      p = &pfd[1];
      nfds = 0;
    }
    if ((bufused >= lowwater)
     && (maxocnt != 0)) {
      nfds += 1;
    }
    if (freq != 0) {
      gettimeofday(&now, NULL);
      tmostat = freq + (laststat.tv_sec - now.tv_sec) * 1000
        + (laststat.tv_usec - now.tv_usec) / 1000;
      if (tmostat < 0) {
        tmostat = 0;
      }
      tmo = tmostat;
    } else if (maximsec || maxomsec) {
      gettimeofday(&now, NULL);
      tmo = -1;
    }
    if (maximsec > 0) {
      tmomaxi = maximsec + (lastmaxi.tv_sec - now.tv_sec) * 1000
        + (lastmaxi.tv_usec - now.tv_usec) / 1000;
      if (tmomaxi < 0) {
        tmomaxi = 0;
      }
      if (((unsigned)tmo) > ((unsigned)tmomaxi)) {
        tmo = tmomaxi;
      }
    }
    if (maxomsec > 0) {
      tmomaxo = maxomsec + (lastmaxo.tv_sec - now.tv_sec) * 1000
        + (lastmaxo.tv_usec - now.tv_usec) / 1000;
      if (tmomaxo < 0) {
        tmomaxo = 0;
      }
      if (((unsigned)tmo) > ((unsigned)tmomaxo)) {
        tmo = tmomaxo;
      }
    }
    nfds = poll(p, nfds, tmo);
    if (tmostat == 0) {
      statistics(&now, cntin, cntout);
      laststat.tv_sec += freqsec;
      laststat.tv_usec += frequsec;
      if (laststat.tv_usec >= 1000000) {
        laststat.tv_sec += 1;
        laststat.tv_usec -= 1000000;
      }
      if (now.tv_sec - laststat.tv_sec > 1) {
        laststat = now;
      }
    }
    if (tmomaxi == 0) {
      lastmaxi.tv_sec += maxisec;
      lastmaxi.tv_usec += maxiusec;
      if (lastmaxi.tv_usec >= 1000000) {
        lastmaxi.tv_sec += 1;
        lastmaxi.tv_usec -= 1000000;
      }
      if (now.tv_sec - lastmaxi.tv_sec > 1) {
        lastmaxi = now;
      }
      maxicnt = maxidata;
    }
    if (tmomaxo == 0) {
      lastmaxo.tv_sec += maxosec;
      lastmaxo.tv_usec += maxousec;
      if (lastmaxo.tv_usec >= 1000000) {
        lastmaxo.tv_sec += 1;
        lastmaxo.tv_usec -= 1000000;
      }
      if (now.tv_sec - lastmaxo.tv_sec > 1) {
        lastmaxo = now;
      }
      maxocnt = maxodata;
    }
    if ((nfds > 0) && (pfd[0].revents)) {
      int n;
      if (pfd[0].revents & (POLLERR | POLLNVAL)) {
        fprintf(stderr, "%s%s0x%x)\n", "error on std", "in (", pfd[0].revents);
        finalstat();
        return 1;
      }
      n = ((bufout > bufin) ? bufout : bufsize) - bufin;
      if (n > readsize) {
        n = readsize;
      }
      if ((maxicnt > 0) && (n > maxicnt)) {
        n = maxicnt;
      }
      if (bytecount < (unsigned int) n) {
        n = bytecount;
      }
      n = read(STDIN_FILENO, &rbuf[bufin], n);
      if (n < 0) {
        fprintf(stderr, "%s%s%d)\n", "error on std", "in (", n);
        finalstat();
        return 1;
      }
      if (n == 0) {
        close(STDIN_FILENO);
        pfd[0].fd = -1;
        lowwater = 1;
      } else {
        cntin += n;
        summationin(cntin);
        bufused += n;
        bufin += n;
        if (bufin >= bufsize) {
          bufin = 0;
        }
        if (maxicnt > 0) {
          maxicnt -= n;
        }
        if (byteskip > 0) {
          if (n > byteskip) {
            bufout += byteskip;
            n -= byteskip;
            byteskip = 0;
          } else {
            bufin = 0;
            byteskip -= n;
            n = 0;
          }
          bufused = n;
        }
        if (bytecount != (unsigned long long) -1) {
          bytecount -= n;
          if (bytecount == 0) {
            close(STDIN_FILENO);
            pfd[0].fd = -1;
            lowwater = 1;
          }
        }
      }
      nfds -= 1;
    }
    if ((nfds > 0) && (pfd[1].revents)) {
      int n;
      if (pfd[1].revents & (POLLERR | POLLHUP | POLLNVAL)) {
        if (pfd[1].revents & (POLLERR | POLLNVAL)) {
          fprintf(stderr, "%s%s0x%x)\n", "error on std", "out (", pfd[1].revents);
          finalstat();
          return 1;
        }
        finalstat();
        return 0;
      }
      n = ((bufin > bufout) ? bufin : bufsize) - bufout;
      if (n > writesize) {
        n = writesize;
      }
      if ((maxocnt > 0) && (n > maxocnt)) {
        n = maxocnt;
      }
      n = write(STDOUT_FILENO, &rbuf[bufout], n);
      if (n < 0) {
        fprintf(stderr, "%s%s%d)\n", "error on std", "out (", n);
        finalstat();
        return 1;
      }
      if (n == 0) {
        finalstat();
        return 0;
      }
      cntout += n;
      summationout(cntout);
      bufused -= n;
      bufout += n;
      if (bufout >= bufsize) {
        bufout = 0;
      }
      if (maxocnt > 0) {
        maxocnt -= n;
      }
    }
    if ((pfd[0].fd < 0)
     && (bufused == 0)) {
      finalstat();
      return 0;
    }
  }
  return 0;
}
