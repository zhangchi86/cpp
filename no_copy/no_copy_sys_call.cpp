#include <stdio.h>          //fprintf
#include <sys/sendfile.h>   //sendfile
#include <fcntl.h>          //open()
#include <inttypes.h>       //uint32_t
#include <stdlib.h>         //strtoul()
#include <sys/stat.h>       //fstat()
#include <sys/mman.h>       //mmap()
#include <unistd.h>         //read() write() pipe()

// 8192 usually best
const int READ_BUFFER_SIZE = 8192;

int call_sendfile(int fdi, int fdo) {
    struct stat file_stat;
    fstat(fdi, &file_stat); perror("fstat from_file");
    sendfile(fdo, fdi, NULL, file_stat.st_size); perror("sendfile");
    return 0;
}

int call_mmap_write(int fdi, int fdo) {
    struct stat file_stat;
    fstat(fdi, &file_stat); perror("fstat from_file");
    void* mmap_addr = mmap(NULL, file_stat.st_size, PROT_READ, MAP_PRIVATE, fdi, 0);
    int _ = write(fdo, mmap_addr, file_stat.st_size); perror("write");
    return 0;
}

int call_splice(int fdi, int fdo) {
    int pipefd[2];
    struct stat file_stat;
    fstat(fdi, &file_stat); perror("fstat from_file");
    fprintf(stderr, "file_stat.st_size=%ld\n", file_stat.st_size);
    int _;
    _ = pipe(pipefd); perror("pipe");
    while (true) {
        //_ = splice(fdi, NULL, pipefd[1], NULL, file_stat.st_size, SPLICE_F_MOVE); perror("splice read");
        _ = splice(fdi, NULL, pipefd[1], NULL, file_stat.st_size, SPLICE_F_MORE);
        if (_ == 0) break;
        //_ = splice(pipefd[0], NULL, fdo, NULL, file_stat.st_size, SPLICE_F_MOVE); perror("splice write");
        _ = splice(pipefd[0], NULL, fdo, NULL, file_stat.st_size, SPLICE_F_MOVE);
    }
    return 0;
}

int call_read_write(int fdi, int fdo) {
    char buff[READ_BUFFER_SIZE];
    while (read(fdi, buff, READ_BUFFER_SIZE) != 0) {
        int _ = write(fdo, buff, READ_BUFFER_SIZE);
    }
    return 0;
}

typedef int (*call_fptr_t)(int, int);

int main (int argc, char* argv[]) {
    // check input
    if (argc != 4) {
        fprintf(stderr, "usage:%s entray_id from_file to_file\n", argv[0]);
        return 0;
    }
    call_fptr_t call_fptrs[] = {call_sendfile, call_mmap_write, call_splice, call_read_write};
    uint32_t entray_id = strtoul(argv[1], NULL, 10);
    uint32_t call_fptr_num = sizeof(call_fptrs) / sizeof(call_fptr_t);
    if (entray_id >= call_fptr_num) {
        fprintf(stderr, "uint32_t entray_id[=%s] is invalid, need smaller than %d\n", argv[1], call_fptr_num);
        return 1;
    }

    // open file
    int fdi = open(argv[2], O_RDONLY); perror("open from_file");
    int fdo = open(argv[3], O_WRONLY | O_CREAT | O_TRUNC, 0644); perror("open to_file");

    call_fptrs[entray_id](fdi, fdo);
    return 0;
}
