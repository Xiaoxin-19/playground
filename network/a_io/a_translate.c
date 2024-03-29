#include <common_header.h>
#include <aio.h>

#define BSZ 4096
#define NBUF 8
enum rwop
{
  UNUSED = 0,
  READ_PENDING = 1,
  WRITE_PENDING = 2
};

struct buf
{
  enum rwop op;
  int last;
  struct aiocb aiocb;
  unsigned char data[BSZ];
};

struct buf bufs[NBUF];

unsigned char
translate(unsigned char c)
{
  if (isalpha(c))
  {
    if (c >= 'n')
    {
      c -= 13;
    }
    else if (c >= 'a')
    {
      c += 13;
    }
    else if (c >= 'N')
    {
      c -= 13;
    }
    else
    {
      c += 13;
    }
  }
  return c;
}

int main(int argc, char *argv[])
{
  ARGS_CHECK(argc, 3);
  int ifd, ofd, i, j, n, err, numop;
  struct stat sbuf;
  const struct aiocb *aiolist[NBUF];
  off_t off = 0;

  ERROR_CHECK((ifd = open(argv[1], O_RDONLY)) < 0, -1, "open error");
  ERROR_CHECK((ofd = open(argv[2], O_RDWR | O_CREAT | O_TRUNC, 0664)) < 0, -1, "open error");

  ERROR_CHECK(fstat(ifd, &sbuf) < 0, -1, "fstat error");

  for (i = 0; i < NBUF; i++)
  {
    bufs[i].op = UNUSED;
    bufs[i].aiocb.aio_buf = bufs[i].data;
    bufs[i].aiocb.aio_sigevent.sigev_notify = SIGEV_NONE;
    aiolist[i] = NULL;
  }

  numop = 0;
  while (true)
  {
    for (i = 0; i < NBUF; i++)
    {
      switch (bufs[i].op)
      {
      case UNUSED:
      {
        if (off < sbuf.st_size)
        {
          bufs[i].op = READ_PENDING;
          bufs[i].aiocb.aio_fildes = ifd;
          bufs[i].aiocb.aio_offset = off;
          off += BSZ;
          if (off >= sbuf.st_size)
          {
            bufs[i].last = 1;
          }
          bufs[i].aiocb.aio_nbytes = BSZ;
          ERROR_CHECK(aio_read(&bufs[i].aiocb) < 0, -1, "aio_read error");
          aiolist[i] = &bufs[i].aiocb;
          numop++;
        }
        break;
      }
      case READ_PENDING:
      {
        if ((err = aio_error(&bufs[i].aiocb)) == EINPROGRESS)
        {
          continue;
        }
        if (err != 0)
        {
          ERROR_CHECK(err, -1, "aio_error failed");
          perror("read failed");
          exit(1);
        }
        if ((n = aio_return(&bufs[i].aiocb)) < 0)
        {
          perror("aio_return failed");
          exit(1);
        }
        if (n != BSZ && !bufs[i].last)
        {
          perror("short read");
          exit(1);
        }
        for (j = 0; j < n; j++)
        {
          bufs[i].data[j] = translate(bufs[i].data[j]);
        }
        bufs[i].op = WRITE_PENDING;
        bufs[i].aiocb.aio_fildes = ofd;
        bufs[i].aiocb.aio_nbytes = n;
        if (aio_write(&bufs[i].aiocb) < 0)
        {
          perror("aio_write failed");
          exit(1);
        }
        break;
      }
      case WRITE_PENDING:
      {
        if ((err = aio_error(&bufs[i].aiocb)) == EINPROGRESS)
        {
          continue;
        }

        if (err != 0)
        {
          ERROR_CHECK(err, -1, "aio_error failed");
          perror("write failed");
          exit(1);
        }
        if ((n = aio_return(&bufs[i].aiocb)) < 0)
        {
          perror("aio_return failed");
          exit(1);
        }

        if (n != bufs[i].aiocb.aio_nbytes)
        {
          perror("short write");
          exit(1);
        }
        aiolist[i] = NULL;
        bufs[i].op = UNUSED;
        numop--;
        break;
      }
      default:
        break;
      }
      if (numop == 0)
      {
        if (off >= sbuf.st_size)
        {
          break;
        }
      }
      else
      {
        if (aio_suspend(aiolist, NBUF, NULL) < 0)
        {
          perror("aio_suspend failed");
          exit(1);
        }
      }
    }
  }
  bufs[0].aiocb.aio_fildes = ofd;
  if (aio_fsync(O_SYNC, &bufs[0].aiocb) < 0)
  {
    perror("aio_fsync failed");
    exit(1);
  }
  exit(0);
}