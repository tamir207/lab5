#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <elf.h>

int foreach_phdr(void *map_start, void (*func)(Elf32_Phdr *, int), int arg) {
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *) map_start;
    Elf32_Phdr *phdr = (Elf32_Phdr *)((char *) map_start + ehdr->e_phoff);
    int i;
    for (i = 0; i < ehdr->e_phnum; i++) {
        func(phdr, i);
        phdr = (Elf32_Phdr *)((char *) phdr + ehdr->e_phentsize);
    }
    return 0;
}

void print_phdr(Elf32_Phdr *phdr, int i) {
    printf("Program header number %d at address %p\n", i, (void *) phdr);
}

int main(int argc, char **argv) {
    int fd;
    int size;
    void *map_start;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <ELF file>\n", argv[0]);
        return 1;
    }

    fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    map_start = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map_start == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    foreach_phdr(map_start, print_phdr, 0);

    munmap(map_start, size);
    close(fd);
    return 0;
}