#define LAST_LOADABLE_SEG(prog_header, index) prog_header[index+1].p_type != PT_LOAD
#define UNPACK_KEY 0xff

void print_e(char *msg);
void unpack(char* elf_start, unsigned int size);
unsigned int getfilesize(char* path);
int is_image_valid(Elf32_Ehdr* hdr);
void* symbol_resolve(char* name, Elf32_Shdr *shdr, char* strings, char* elf_start);
void* load_elf_image(char* elf_start, unsigned int size);

