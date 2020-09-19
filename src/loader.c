#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libelf.h>
#include <sys/mman.h>
#include <dlfcn.h>



#include "loader.h"
/*
 * Static ELF Loader for x86 Linux
 * 
 */

void print_e(char* msg)
{
    fputs(msg, stderr);
}

void unpack(char* elf_start, unsigned int size)
{
    printf("[*] Unpacking in memory...\n");
    for(unsigned int i = 0; i <= size; i++)
    {
        elf_start[i] ^= UNPACK_KEY;
    }
}

unsigned int getfilesize(char* path) 
{ 
    FILE* fp = fopen(path, "r"); 
    if (fp == NULL) { 
        print_e("[getfilesize] File Not Found!\n"); 
        exit(0);
    } 
  
    fseek(fp, 0L, SEEK_END); 
    long int res = ftell(fp); 
    fclose(fp); 
  
    return res; 
} 

int is_image_valid(Elf32_Ehdr* hdr)
{
    if(hdr->e_ident[EI_MAG0] == ELFMAG0 &&  // 0x7f
       hdr->e_ident[EI_MAG1] == ELFMAG1 &&  // 'E'
       hdr->e_ident[EI_MAG2] == ELFMAG2 &&  // 'L'
       hdr->e_ident[EI_MAG3] == ELFMAG3 &&  // 'F'
       hdr->e_type == ET_EXEC) {
        return -1;
    } 
    else {
        return 0;
    }
    
}


void* symbol_resolve(char* name, Elf32_Shdr* shdr, char* strings, char* elf_start)
{
    Elf32_Sym* syms = (Elf32_Sym*)(elf_start + shdr->sh_offset);
    int i;
    for(i = 0; i < shdr->sh_size / sizeof(Elf32_Sym); i += 1) {
        if (strcmp(name, strings + syms[i].st_name) == 0) {
           /*
            * In static ELF  files,  the st_value  member  is 
            * not an offset but rather a virtual address itself
            * hence,  it's not necessary  to calculate  an RVA.
            */
            return (void *)syms[i].st_value; 
        }
    }
    return NULL;
}

void* load_elf_image(char* elf_start, unsigned int size)
{
    // declaring local vars
    Elf32_Ehdr *hdr     = NULL;
    Elf32_Phdr *phdr    = NULL;
    Elf32_Shdr *shdr    = NULL;
    char *strings       = NULL; // string table           (offset in file)
    char *start         = NULL; // start of a segment     (offset in file)
    char *target_addr   = NULL; // base addr of a segment (in virtual memory)
    void *entry         = NULL; // entry point            (in virtual memory)
    int i               = 0;    // counter for program headers

    // unpacking
    unpack(elf_start, size);

    // start proccessing
    hdr = (Elf32_Ehdr *) elf_start;
    printf("[*] Validating ELF...\n");

    if(!is_image_valid(hdr)) {
        print_e("[load_elf_image] invalid ELF image\n");
        return 0;
    }

    printf("[*] Loading/mapping segments...\n");
    phdr = (Elf32_Phdr *)(elf_start + hdr->e_phoff);

    for(i=0; i < hdr->e_phnum; ++i) {
        if(phdr[i].p_type != PT_LOAD) {
                continue;
        }
 
        if(phdr[i].p_filesz > phdr[i].p_memsz) {
                print_e("[load_elf_image] p_filesz > p_memsz\n");
                return 0;
        }

        if(!phdr[i].p_filesz) {
                continue;
        }

        start = elf_start + phdr[i].p_offset;
        target_addr = (char* )phdr[i].p_vaddr;

        if (LAST_LOADABLE_SEG(phdr, i)) {
            phdr[i].p_memsz += 0x1000; // extra-page padding
        }

        // allocating memory for the segment
	    mmap(target_addr, phdr[i].p_memsz, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANON | MAP_PRIVATE, -1, 0); // hotfix patched by BigShaq

        // moving things around *poof* 
        memmove(target_addr,start,phdr[i].p_filesz);

        // setting permissions
            if(!(phdr[i].p_flags & PF_W)) {
                    mprotect((unsigned char *) target_addr,
                              phdr[i].p_memsz,
                              PROT_READ);
            }

            if(phdr[i].p_flags & PF_X) {
                    mprotect((unsigned char *) target_addr,
                            phdr[i].p_memsz,
                            PROT_EXEC);
            }
    }

    printf("[*] Looking for a .symtab section header...\n");

    shdr = (Elf32_Shdr *)(elf_start + hdr->e_shoff);

    for(i=0; i < hdr->e_shnum; ++i) {
        if (shdr[i].sh_type == SHT_SYMTAB) {
            printf("[*] SHT_SYMTAB entry was found, looking for main() ...\n");

            strings = elf_start + shdr[shdr[i].sh_link].sh_offset;
            entry = symbol_resolve("main", shdr + i, strings, elf_start); 

	        printf("[*] main at: %p\n", entry);
            break;
        }
    }

   return entry;
}

int main(int argc, char** argv, char** envp)
{
    int (*ptr)(int, char **, char**);
    FILE* elf            = fopen(argv[1], "rb");
    unsigned int filesz  = getfilesize(argv[1]);
    char* buf            = malloc(filesz);


    fread(buf, filesz, 1, elf);
    fclose(elf);
    ptr = load_elf_image(buf, filesz);
    free(buf);

    if(ptr) {
        printf("[*] jumping to %p \n----------------\n", ptr);
        ptr(argc,argv,envp); // https://youtu.be/SF3UZRxQ7Rs 
    } 
    else {
        printf("[!] Quitting...\n");
    }
    return 0;
}
