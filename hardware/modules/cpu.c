#include "cpu.h"

static int FindFields (FILE *fp, unsigned long long int *fields)
{
  int retval;
  char buffer[BUF_MAX];

  if (!fgets (buffer, BUF_MAX, fp)) {
    perror ("ERROR - Can't get buffer for /proc/stat location");
  }


  // line starts with c and a string. This is to handle cpu, cpu[0-9]+
  retval = sscanf (buffer, "c%*s %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu",
                            &fields[0], 
                            &fields[1], 
                            &fields[2], 
                            &fields[3], 
                            &fields[4], 
                            &fields[5], 
                            &fields[6], 
                            &fields[7], 
                            &fields[8], 
                            &fields[9]); 

  if (retval == 0) { return -1; } // dont looking
  if (retval < 4) {
    fprintf (stderr, "Error reading first 4 /proc/stat cpu field\n");
    return 0;
  }
  return 1;
}
 
int main (void)
{
  FILE *fp;
  unsigned long long int fields[10];
  unsigned long long int total_tick[MAX_CPU];
  unsigned long long int total_tick_old[MAX_CPU];
  unsigned long long int idle[MAX_CPU];
  unsigned long long int idle_old[MAX_CPU];
  unsigned long long int del_total_tick[MAX_CPU];
  unsigned long long int del_idle[MAX_CPU];
  int i;
  int cpus = 0;
  int count;
  double percent_usage;

  fp = fopen ("/proc/stat", "r");
  if (fp == NULL) {
    perror ("Error");
  }
 
 
  while (read_fields (fp, fields) != -1) {
    for (i=0, total_tick[cpus] = 0; i<10; i++) {
      total_tick[cpus] += fields[i];
    }

    idle[cpus] = fields[3]; /* idle ticks index */
    cpus++;
  }
 
  while (1) {
    sleep (1); // 1 second delta
    fseek (fp, 0, SEEK_SET);
    fflush (fp);
    for (count = 0; count < cpus; count++) {
      total_tick_old[count] = total_tick[count];
      idle_old[count] = idle[count];
     
      if (!read_fields (fp, fields)) {
	printf("ERROR - can't read field of CPU core %d\n", count - 1);
	break;
      }
      
      for (i=0, total_tick[count] = 0; i<10; i++) {
	total_tick[count] += fields[i];
      }

      idle[count] = fields[3];
 
      del_total_tick[count] = total_tick[count] - total_tick_old[count];
      del_idle[count] = idle[count] - idle_old[count];
 
      percent_usage = ((del_total_tick[count] - del_idle[count]) / (double) del_total_tick[count]) * 100;
      
      if (count == 0) {
	printf ("Total CPU Usage: %3.2lf%%\n", percent_usage);
      } else {
	printf ("\tCPU%d Usage: %3.2lf%%\n", count - 1, percent_usage);
      }
    }
    printf ("\n");
  }
 
  fclose (fp);
 
  return 0;
}

