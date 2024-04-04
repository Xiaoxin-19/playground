#ifndef WORK_H
#define WORK_H

/// @brief 传输协议
typedef struct
{
  /// @brief ox01 表示下载文件 ox02 表示上传文件 ox03 显示服务器可下载文件列表, ox04 表示下载文件列表, ox05 表示下载文件信息
  unsigned char type;
  unsigned len;
  char filename[1024];
} tran_protocol_t;

void work(int main_fd);

int send_file(tran_protocol_t *protocol, int client_socket);
#endif // WORK_H