#ifndef FULL_DUPLEX_PIPE_H
#define FULL_DUPLEX_PIPE_H

#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

struct full_duplex_pipe {
  enum {
    FULL_DUPLEX__UNKNOWN,
    FULL_DUPLEX__PARENT,
    FULL_DUPLEX__CHILD,
  } type;

  int rx_fd[2];
  int tx_fd[2];
};

static inline int create_full_duplex_pipe(struct full_duplex_pipe *p) {
  assert(p);
  int fds[2], res = pipe(fds);

  p->rx_fd[0] = fds[0];
  p->tx_fd[0] = fds[1];

  if (res) {
#ifndef NDEBUG
    perror("Error while creating pipe:");
#endif
    return res;
  }

  res = pipe(fds);

  p->rx_fd[1] = fds[0];
  p->tx_fd[1] = fds[1];

  if (res) {
#ifndef NDEBUG
    perror("Error while creating pipe:");
#endif
    close(p->rx_fd[0]);
    close(p->tx_fd[0]);
    return res;
  }

  p->type = FULL_DUPLEX__UNKNOWN;

  return 0;
}

static inline void close_full_duplex_pipe(struct full_duplex_pipe *p) {
  assert(p);

  switch (p->type) {
  case FULL_DUPLEX__UNKNOWN: {
    close(p->rx_fd[0]);
    close(p->rx_fd[1]);
    close(p->tx_fd[0]);
    close(p->tx_fd[1]);
  } break;
  case FULL_DUPLEX__PARENT: {
    close(p->rx_fd[1]);
    close(p->tx_fd[0]);
  } break;
  case FULL_DUPLEX__CHILD: {
    close(p->rx_fd[0]);
    close(p->tx_fd[1]);
  }
  }
}

static inline int parent_full_duplex_pipe(struct full_duplex_pipe *p) {
  assert(p);

  if (p->type == FULL_DUPLEX__UNKNOWN) {
    close(p->rx_fd[0]);
    close(p->tx_fd[1]);
    p->type = FULL_DUPLEX__PARENT;
    return 0;
  }

  if (p->type == FULL_DUPLEX__CHILD) {
    return -1;
  }

  return 0;
}

static inline int child_full_duplex_pipe(struct full_duplex_pipe *p) {
  assert(p);

  if (p->type == FULL_DUPLEX__UNKNOWN) {
    close(p->rx_fd[1]);
    close(p->tx_fd[0]);
    p->type = FULL_DUPLEX__CHILD;
    return 0;
  }

  if (p->type == FULL_DUPLEX__CHILD) {
    return -1;
  }

  return 0;
}

static inline ssize_t write_full_duplex_pipe(struct full_duplex_pipe *p, void *buf, size_t size) {
  assert(p);
  switch (p->type) {
  case FULL_DUPLEX__UNKNOWN: return -1;
  case FULL_DUPLEX__PARENT: return write(p->tx_fd[0], buf, size);
  case FULL_DUPLEX__CHILD: return write(p->tx_fd[1], buf, size);
  }
}

static inline ssize_t read_full_duplex_pipe(struct full_duplex_pipe *p, void *buf, size_t size) {
  assert(p);
  switch (p->type) {
  case FULL_DUPLEX__UNKNOWN: return -1;
  case FULL_DUPLEX__PARENT: return read(p->rx_fd[1], buf, size);
  case FULL_DUPLEX__CHILD: return read(p->rx_fd[0], buf, size);
  }
}

#endif