#include <stdio.h>

#include "full_duplex_pipe.h"
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUF_SIZE 65536

struct {
  char *input, *output;
  int   input_present, output_present;
} args = {NULL, NULL, 0, 0};

void parse_args(int argc, char *argv[]) {
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
}

int main(int argc, char *argv[]) {
  parse_args(argc, argv);

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
    char   write_buf[BUF_SIZE];
    size_t bytes = 0;

    while ((bytes = read_full_duplex_pipe(&p, write_buf, BUF_SIZE)) > 0) {
      write_full_duplex_pipe(&p, write_buf, bytes);
#ifdef DEBUG_PRINT
      printf("child: read bytes from parent = %zu\n", bytes);
#endif
    }

    close_full_duplex_pipe(&p);
#ifdef DEBUG_PRINT
    printf("child: exiting\n");
#endif
    exit(0);
  }

  parent_full_duplex_pipe(&p);
  char   read_buf[BUF_SIZE];
  size_t currently_read = 0;

  while (currently_read < sb.st_size) {
    size_t read_bytes = read(in_fd, read_buf, BUF_SIZE);

    if (read_bytes == -1) {
      perror("Error reading input file: ");
      exit(1);
    }

    currently_read += read_bytes;
    size_t bytes = write_full_duplex_pipe(&p, read_buf, read_bytes);
    size_t from_process_bytes = read_full_duplex_pipe(&p, read_buf, bytes);
#ifdef DEBUG_PRINT
      printf("parent: read bytes from file = %zu\n", read_bytes);
      printf("parent: wrote bytes to child = %zu\n", bytes);
#endif
    write(out_fd, read_buf, from_process_bytes);
  }

  close_full_duplex_pipe(&p);

  int exit_code;
  waitpid(child, &exit_code, 0);

#ifdef DEBUG_PRINT
  printf("parent: child finished with exit code: %d\n", exit_code);
#endif

  close(in_fd);
  close(out_fd);
}