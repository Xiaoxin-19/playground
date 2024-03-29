#include <common_header.h>

#define BSZ 4096

unsigned char buf[BSZ];

unsigned char translate(unsigned char c)
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

  int ifd, ofd, i, n, nw;
  if ((ifd = open(argv[1], O_RDONLY)) < 0)
  {
    perror("open error");
    exit(1);
  }
  if ((ofd = open(argv[2], O_RDWR | O_CREAT | O_TRUNC, 0664)) < 0)
  {
    perror("open error");
    exit(1);
  }

  while ((n = read(ifd, buf, BSZ)) > 0)
  {
    for (i = 0; i < n; i++)
    {
      buf[i] = translate(buf[i]);
    }
    if ((nw = write(ofd, buf, n)) != n)
    {
      if (nw < 0)
      {
        perror("write error");
        exit(1);
      }
      else
      {
        fprintf(stderr, "short write (%d/%d)", nw, n);
      }
    }
  }
  fsync(ofd);
  return 0;
}