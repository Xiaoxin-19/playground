#include <common_header.c>

void copy_file(char *src, char *dest)
{
    FILE *src_fp = fopen(src, "rb");
    ERROR_CHECK(src_fp, NULL, "fopen src faild!");
    FILE *dest_fp = fopen(dest, "wb");
    if (dest_fp == NULL)
    {
        free(src_fp);
        perror("fopen dest faild!");
        exit(1);
    }
    char buf[1024] = {0};
    size_t count;
    while ((count = fread(buf, 1, sizeof(buf), src_fp)) > 0)
    {
        fwrite(buf, 1, count, dest_fp);
    }
    fclose(src_fp);
    fclose(dest_fp);
}
int main(int argc, char *args[])
{
    ARGS_CHECK(argc, 3);
    copy_file(args[1], args[2]);
    return 0;
}
