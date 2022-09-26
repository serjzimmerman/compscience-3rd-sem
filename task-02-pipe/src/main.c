#include <stdio.h>

#include "full_duplex_pipe.h"
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

struct {
  char *input, *output;
  int   input_present, output_present;
} args;

int main(int argc, char *argv[]) {
  args.input = NULL;
  args.output = NULL;

  int opt;
  while ((opt = getopt(argc, argv, "i:o:")) != -1) {
    switch (opt) {
    case 'i':
      args.input_present = 1;
      args.input = optarg;
      break;
    case 'o':
      args.output = optarg;
      args.output_present = 1;
      break;
    default: fprintf(stderr, "Unknown option -%c", optopt); exit(1);
    }
  }

  if (!args.input_present || !args.output_present) {
    fprintf(stderr, "Invalid options, you must specify input and output\n");
    exit(1);
  }

  int in_fd = open(args.input, O_RDONLY);
  if (in_fd == -1) {
    perror("Error opening input file: ");
    exit(1);
  }

  int out_fd = open(args.output, O_WRONLY | O_CREAT, 0666);
  if (out_fd == -1) {
    perror("Error opening output file: ");
    exit(1);
  }

  struct stat sb;
  if (stat(args.input, &sb) == -1) {
    perror("Error reading input file: ");
    exit(1);
  }

  struct full_duplex_pipe p;
  create_full_duplex_pipe(&p);

  int child;
  if ((child = fork()) == 0) {
    child_full_duplex_pipe(&p);
    char   write_buf[32];
    size_t bytes = 0;

    while ((bytes = read_full_duplex_pipe(&p, write_buf, 32)) > 0) {
      write_full_duplex_pipe(&p, write_buf, bytes);
    }

    exit(0);
  }

  parent_full_duplex_pipe(&p);
  char   read_buf[32], read_buf_copy[32];
  size_t currently_read = 0;

  while (currently_read < sb.st_size) {
    size_t read_bytes = read(in_fd, read_buf, 32);

    if (read_bytes == -1) {
      perror("Error reading input file: ");
      exit(1);
    }

    currently_read += read_bytes;
    size_t bytes = write_full_duplex_pipe(&p, read_buf, read_bytes);

    size_t from_process_bytes = read_full_duplex_pipe(&p, read_buf_copy, bytes);
    write(out_fd, read_buf_copy, from_process_bytes);
  }

  int exit_code;
  waitpid(child, &exit_code, 0);

  close_full_duplex_pipe(&p);

  close(in_fd);
  close(out_fd);
}