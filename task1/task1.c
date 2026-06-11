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

static const char *phdr_type_str(Elf32_Word type) {
    if (type == PT_NULL)         return "NULL";
    if (type == PT_LOAD)         return "LOAD";
    if (type == PT_DYNAMIC)      return "DYNAMIC";
    if (type == PT_INTERP)       return "INTERP";
    if (type == PT_NOTE)         return "NOTE";
    if (type == PT_SHLIB)        return "SHLIB";
    if (type == PT_PHDR)         return "PHDR";
    if (type == PT_TLS)          return "TLS";
    if (type == PT_GNU_EH_FRAME) return "GNU_EH_FRAME";
    if (type == PT_GNU_STACK)    return "GNU_STACK";
    if (type == PT_GNU_RELRO)    return "GNU_RELRO";
    return "UNKNOWN";
}

void print_phdr_info(Elf32_Phdr *phdr, int i) {
    printf("%-15s 0x%06x 0x%08x 0x%08x 0x%05x 0x%05x %c%c%c 0x%x\n",
        phdr_type_str(phdr->p_type),
        phdr->p_offset,
        phdr->p_vaddr,
        phdr->p_paddr,
        phdr->p_filesz,
        phdr->p_memsz,
        (phdr->p_flags & PF_R) ? 'R' : ' ',
        (phdr->p_flags & PF_W) ? 'W' : ' ',
        (phdr->p_flags & PF_X) ? 'E' : ' ',
        phdr->p_align);

    if (phdr->p_type == PT_LOAD) {
        int prot = 0;
        if (phdr->p_flags & PF_R) prot |= PROT_READ;
        if (phdr->p_flags & PF_W) prot |= PROT_WRITE;
        if (phdr->p_flags & PF_X) prot |= PROT_EXEC;
        printf("  prot=0x%08x  flags=0x%08x\n", prot, MAP_PRIVATE | MAP_FIXED);
    }
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
    
    printf("%-15s Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align\n", "Type");
    foreach_phdr(map_start, print_phdr_info, 0);

    munmap(map_start, size);
    close(fd);
    return 0;
}