#define _FILE_OFFSET_BITS 64
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int opt_count = 0, /* -c */
    opt_notrep = 0, /* -d */
    opt_chars = 0, /* -s */
    opt_suprep = 0; /* -u */

int input_fd = 0, output_fd = 1;

void write_str(char * s) {
  write(output_fd,s,strlen(s));
}

void print_and_free(char * buf, int count) {
  char b[15];
  if (opt_count) {
    if (opt_notrep && count>1 
        || opt_suprep && count==1
        || opt_notrep==0 && opt_suprep==0) {
      int len = fmt_ulongpadright(b,count,7);
      b[len] = 0;
      write_str(b);
      write_str(" ");
      write_str(buf);
      write_str("\n");
    }
  } else {
    if (opt_notrep && count>1
        || opt_suprep && count==1
        || opt_notrep==0 && opt_suprep==0) {
      write_str(buf);
      write_str("\n");
    }
  }
  free(buf);
}

void error_msg(char * msg) {
  write(2,msg,strlen(msg));
  exit(EXIT_FAILURE);
}

int read_line(char ** buffer) {
  char c;
  int len;
  int b_len = 1;
  char * b = malloc(b_len);
  if (b==NULL)
    error_msg("error allocating memory.\n");
  strcpy(b,"");
  len = read(input_fd,&c,sizeof(c));
  while (len && c!='\n') {
    ++b_len;
    b = realloc(b,b_len);
    if (b==NULL)
      error_msg("error reallocating memory.\n");
    b[b_len-2] = c;
    b[b_len-1] = 0;
    len = read(input_fd,&c,sizeof(c));
  }
  *buffer = b;
  if (len==0) {
    return -1;
  } else {
    return b_len;
  }
}

int main(int argc, char * argv[]) {
  int i;
  char * a;
  char * current_line = NULL;
  int cur_line_count = 0;
  char * line_in_buffer;
  int line_len;
  for (i=1;i<argc;++i) {
    a = argv[i];
    if (a[0]=='-') {
      switch (a[1]) {
        case 'c': 
          opt_count = 1;
          break;
        case 'd':
          opt_notrep = 1;
          break;
        case 'f': /* currently unimplemented */
          break;
        case 's':
          if (argv[i+1]) {
            opt_chars = strtol(argv[i+1],NULL,10);
            ++i;
          } else {
            error_msg("Missing argument for -s.\n");
          }
          break;
        case 'u':
          opt_suprep = 1;
          break;
        default:
          error_msg("usage: uniq [-c|-d|-u] [-s number_chars] [input_file [output_file]]\n");
      }
    } else {
      break;
    }
  }
  if (argv[i]) {
    if (strcmp(argv[i],"-")) {
      input_fd = open(argv[i],O_RDONLY);
      if (input_fd < 0) {
        error_msg("Error: couldn't open input file.\n");
      }
    }
    if (argv[i+1]) {
      output_fd = open(argv[i+1],O_WRONLY);
      if (output_fd < 0) {
        error_msg("Error: couldn't open output file.\n");
      }
    }
  }
  line_len = read_line(&line_in_buffer);
  while (line_len>-1) {
    if (current_line==NULL) {
      current_line = line_in_buffer;
      cur_line_count = 1;
    } else {
      if (strcmp(current_line+opt_chars,line_in_buffer+opt_chars)==0) {
        ++cur_line_count;
        free(line_in_buffer);
      } else {
        print_and_free(current_line,cur_line_count);
        current_line = line_in_buffer;
        cur_line_count = 1;
      }
    }
    line_len = read_line(&line_in_buffer);
  }
  print_and_free(current_line,cur_line_count);
  return EXIT_SUCCESS;
}
