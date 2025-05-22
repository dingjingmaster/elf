#include <elf.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <locale.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "macros.h"

static const char* get_shdr_type (uint32_t type)
{
    switch (type) {
    case SHT_NULL: return "SHT_NULL(段头表入口, 未使用)";
    case SHT_PROGBITS: return "SHT_PROGBITS(程序数据)";
    case SHT_SYMTAB: return "SHT_SYMTAB(符号表)";
    case SHT_STRTAB: return "SHT_STRTAB(字符串表)";
    case SHT_RELA: return "SHT_RELA(重定位入口)";
    case SHT_HASH: return "SHT_HASH(符号hash表)";
    case SHT_DYNAMIC: return "SHT_DYNAMIC(动态链接信息)";
    case SHT_NOTE: return "SHT_NOTE()";
    case SHT_NOBITS: return "SHT_NOBITS(bss)";
    case SHT_REL: return "SHT_REL(重定位入口, 无附加项)";
    case SHT_SHLIB: return "SHT_SHLIB(预留)";
    case SHT_DYNSYM: return "SHT_DYNSYM(动态链接符号表)";
    case SHT_INIT_ARRAY: return "SHT_INIT_ARRAY(构造器数组)";
    case SHT_FINI_ARRAY: return "SHT_FINI_ARRAY(析构器数组)";
    case SHT_PREINIT_ARRAY: return "SHT_PREINIT_ARRAY(预构造函数数组)";
    case SHT_GROUP: return "SHT_GROUP(段组)";
    case SHT_SYMTAB_SHNDX: return "SHT_SYMTAB_SHNDX(扩展段索引)";
    case SHT_RELR: return "SHT_RELR(RELR相对位置)";
    case SHT_LOOS: return "SHT_LOOS(OS开始)";
    case SHT_GNU_ATTRIBUTES: return "SHT_GNU_ATTRIBUTES(Object属性)";
    case SHT_GNU_HASH: return "SHT_GNU_HASH(GNU样式的hash表)";
    case SHT_GNU_LIBLIST: return "SHT_GNU_LIBLIST(预链接库列表)";
    case SHT_CHECKSUM: return "SHT_CHECKSUM(DSO内容md5)";
    case SHT_LOSUNW: return "SHT_LOSUNW/SHT_SUNW_move";
    case SHT_SUNW_COMDAT: return "SHT_SUNW_COMDAT";
    case SHT_SUNW_syminfo: return "SHT_SUNW_syminfo";
    case SHT_GNU_verdef: return "SHT_GNU_verdef";
    case SHT_GNU_verneed: return "SHT_GNU_verneed";
    case SHT_GNU_versym: return "SHT_GNU_versym/SHT_HISUNW/SHT_HIOS";
    case SHT_LOPROC: return "SHT_LOPROC";
    case SHT_HIPROC: return "SHT_HIPROC";
    case SHT_LOUSER: return "SHT_LOUSER";
    case SHT_HIUSER: return "SHT_HIUSER";
    }

    return "Unknown";
}

static const char* get_shdr_flags(uint32_t flags)
{
    static char buf[64];

    buf[0] = (flags & SHF_WRITE) ? 'W' : ' ';
    buf[1] = (flags & SHF_ALLOC) ? 'A' : ' ';
    buf[2] = (flags & SHF_EXECINSTR) ? 'E' : ' ';
    buf[3] = (flags & SHF_MERGE) ? 'M' : ' ';
    buf[4] = (flags & SHF_STRINGS) ? 'S' : ' ';
    buf[5] = (flags & SHF_INFO_LINK) ? 'I' : ' ';
    buf[6] = (flags & SHF_LINK_ORDER) ? 'L' : ' ';
    buf[7] = (flags & SHF_OS_NONCONFORMING) ? 'O' : ' ';
    buf[8] = (flags & SHF_GROUP) ? 'G' : ' ';
    buf[9] = (flags & SHF_TLS) ? 'T' : ' ';
    buf[10] = (flags & SHF_COMPRESSED) ? 'C' : ' ';
    buf[11] = (flags & SHF_MASKOS) ? 'K' : ' ';
    buf[12] = (flags & SHF_MASKPROC) ? 'P' : ' ';
    buf[13] = (flags & SHF_GNU_RETAIN) ? 'R' : ' ';
    buf[14] = (flags & SHF_ORDERED) ? 'D' : ' ';
    buf[15] = (flags & SHF_EXCLUDE) ? 'U' : ' ';

    return buf;
}

static const char* get_phdr_type (uint32_t type)
{
    switch (type) {
    case PT_NULL: return "PT_NULL(Unused)";
    case PT_LOAD: return "PT_LOAD(可加载程序段)";
    case PT_DYNAMIC: return "PT_DYNAMIC(动态链接信息)";
    case PT_INTERP: return "PT_INTERP(程序解析器)";
    case PT_NOTE: return "PT_NOTE(辅助信息)";
    case PT_SHLIB: return "PT_SHLIB(保留)";
    case PT_PHDR: return "PT_PHDR(头表入口)";
    case PT_TLS: return "PT_TLS(线程局部存储段)";
    case PT_LOOS: return "PT_LOOS(操作系统专用)";
    case PT_GNU_EH_FRAME: return "PT_GNU_EH_FRAME(GCC .eh_frame_hdr段)";
    case PT_GNU_STACK: return "PT_GNU_STACK(表示堆栈可执行性)";
    case PT_GNU_RELRO: return "PT_GNU_RELRO(重定位后只读)";
    case PT_GNU_PROPERTY: return "PT_GNU_PROPERTY(Gnu 属性)";
    case PT_GNU_SFRAME: return "PT_GNU_SFRAME(SFrame 段)";
    case PT_LOSUNW /* PT_SUNWBSS */: return "PT_LOSUNW/PT_SUNWBSS(Sun 特殊段)";
    case PT_SUNWSTACK: return "PT_SUNWSTACK(栈段)";
    case PT_HISUNW: return "PT_HISUNW/PT_HIOS(OS 特殊段结束)";
    case PT_LOPROC: return "PT_LOPROC(处理器特殊段开始)";
    case PT_HIPROC: return "PT_HIPROC(处理器特殊段结束)";
    default:
    }

    return "Unknown";
}

static const char* get_phdr_flags(uint32_t flags)
{
    static char buf[16] = {0};

    buf[0] = (flags & PF_R) ? 'R' : ' ';
    buf[1] = (flags & PF_W) ? 'W' : ' ';
    buf[2] = (flags & PF_X) ? 'X' : ' ';
    buf[3] = (flags & PF_MASKOS) ? 'M' : ' ';
    buf[4] = (flags & PF_MASKPROC) ? 'C' : ' ';
    buf[5] = '\0';

    return buf;
}

int main(int argc, char* argv[])
{
    if (2 != argc) {
        printf("Usage: %s <elf file>\n", argv[0]);
        return 1;
    }

    setlocale(LC_ALL, "");

    int fd = 0;
    int ret = 0;
    int64_t fSize = 0;
    void* fAddr = NULL;

    {
        struct stat statBuf;
        if (0 > stat(argv[1], &statBuf)) {
            printf("stat error: %s\n", strerror(errno));
            return 2;
        }
        if (!S_ISREG(statBuf.st_mode)) {
            printf("%s is not a regular file\n", argv[1]);
            return 3;
        }
        fSize = statBuf.st_size;
    }


    do {
        fd = open(argv[1], O_RDONLY);
        C_BREAK_IF_FAIL(fd >= 0);

        fAddr = mmap(NULL, fSize, PROT_READ, MAP_PRIVATE, fd, 0);
        C_BREAK_IF_OK(MAP_FAILED == fAddr);

        // 读取 ELF 头
        Elf64_Ehdr* header = (Elf64_Ehdr*)fAddr;

        // 检查 ELF 魔数
        if (0 != memcmp(header->e_ident, ELFMAG, SELFMAG)) {
            printf("不是ELF文件\n");
            break;
        }

        printf("ELF '%s' 头:\n", argv[1]);
        printf("\t魔数: ");
        for (int i = 0; i < EI_NIDENT; ++i) {
            printf("%02x ", header->e_ident[i]);
        }
        printf("\n");

        printf("\t类别: ");
        switch (header->e_ident[EI_CLASS]) {
        case ELFCLASS32: printf("ELF32\n"); break;
        case ELFCLASS64: printf("ELF64\n"); break;
        default:
        case ELFCLASSNONE: printf("ELFNONE\n"); break;
        }

        printf("\t数据: ");
        switch (header->e_ident[EI_DATA]) {
        case ELFDATA2LSB: printf("2补码 小端序\n"); break;
        case ELFDATA2MSB: printf("2补码 大端序\n"); break;
        default:
        case ELFDATANONE: printf("未知\n"); break;
        }

        printf("\t版本: %s\n", header->e_ident[EI_VERSION] == EV_CURRENT ? "Current(1)" : "None(0)");

        printf("\tOS/ABI: ");
        switch (header->e_ident[EI_OSABI]) {
        default:
        case ELFOSABI_NONE: printf("UNIX System V ABI\n"); break;
        case ELFOSABI_HPUX: printf("HP-UX\n"); break;
        case ELFOSABI_NETBSD: printf("NetBSD\n"); break;
        case ELFOSABI_GNU /* ELFOSABI_LINUX */: printf("GNU ELF extensions\n"); break;
        case ELFOSABI_SOLARIS: printf("Sun Solaris\n"); break;
        case ELFOSABI_AIX: printf("IBM AIX\n"); break;
        case ELFOSABI_IRIX: printf("SGI Irix\n"); break;
        case ELFOSABI_FREEBSD: printf("FreeBSD\n"); break;
        case ELFOSABI_TRU64: printf("TRU64 UNIX\n"); break;
        case ELFOSABI_MODESTO: printf("Novell Modesto\n"); break;
        case ELFOSABI_OPENBSD: printf("OpenBSD\n"); break;
        case ELFOSABI_ARM_AEABI: printf("ARM EABI\n"); break;
        case ELFOSABI_ARM: printf("ARM\n"); break;
        case ELFOSABI_STANDALONE: printf("标准嵌入式程序\n"); break;
        }

        printf("\tABI版本: %d\n", header->e_ident[EI_ABIVERSION]);

        printf("\t(以上信息来自 Elf64_Ehdr::e_ident, 共 %ld 字节，实际仅使用8个字节，剩余留待扩展)\n", sizeof(header->e_ident));

        printf("\t类型: ");
        switch (header->e_type) {
        default:
        case ET_NONE: printf("未知类型\n"); break;
        case ET_REL: printf("可重定向文件\n"); break;
        case ET_EXEC: printf("可执行文件\n"); break;
        case ET_DYN: printf("可执行文件(位置无关)\n"); break;
        case ET_CORE: printf("Core文件\n"); break;
        }

        printf("\t机器类型: ");
        switch (header->e_machine) {
        default:
        case EM_NONE: printf("未知(%u)", header->e_machine); break;
        case EM_M32: printf("AT&T WE 32100"); break;
        case EM_SPARC: printf("SUN SPARC"); break;
        case EM_386: printf("Intel 80386"); break;
        case EM_68K: printf("Motorola m68k"); break;
        case EM_88K: printf("Motorola m88k"); break;
        case EM_IAMCU: printf("Intel MCU"); break;
        case EM_860: printf("Intel 80860"); break;
        case EM_MIPS: printf("MIPS R3000 big-endian"); break;
        case EM_S370: printf("IBM System/370"); break;
        case EM_MIPS_RS3_LE: printf("MIPS R3000 little-endian"); break;
        /* 11 - 14 保留 */
        case EM_PARISC: printf("HPPA"); break;
        /* 16 保留 */
        case EM_VPP500: printf("Fujitsu VPP500"); break;
        case EM_X86_64: printf("AMD x86-64"); break;
        }
        printf(" (机器类型目前支持: %d 种)\n", EM_NUM);

        printf("\t二进制版本: %u\n", header->e_version);

        printf("\t程序入口地址: 0x%0lx\n", header->e_entry);
        printf("\t程序头表偏移位置: %lu字节\n", header->e_phoff);
        printf("\t段头表偏移位置: %lu字节\n", header->e_shoff);
        printf("\t处理器特定标志: %u\n", header->e_flags);
        printf("\t头部大小: %u\n", header->e_ehsize);
        printf("\t程序头表中每个条目大小: %u字节\n", header->e_phentsize);
        printf("\t程序头表数目: %u个\n", header->e_phnum);
        printf("\t程序段表中每个条目大小: %u字节\n", header->e_shentsize);
        printf("\t程序段表数目: %u\n", header->e_shnum);
        printf("\t段名字符串索引: %u\n", header->e_shnum);
        printf("\n");

        printf("程序头表(描述了文件如何被加载到内存中):\n");
        {
            printf("\t%-12s %-60s %-20s %-20s %-26s %-20s %-20s %-4s %-4s\n",
                "序号", "类型", "偏移", "虚拟地址", "物理地址", "文件中大小", "内存中大小", "标记", "对齐");
            if (header->e_phnum > 0 && header->e_phoff != 0) {
                Elf64_Phdr* phdr = (Elf64_Phdr*)(fAddr + header->e_phoff);
                for (int i = 0; i < header->e_phnum; i++) {
                    printf("\t%-6d %-60s %-20ld 0x%016lx 0x%016lx %-20ld %-20ld %-4s %-4ld\n",
                        i,
                        get_phdr_type(phdr[i].p_type),
                        phdr[i].p_offset,
                        phdr[i].p_vaddr,
                        phdr[i].p_paddr,
                        phdr[i].p_filesz,
                        phdr[i].p_memsz,
                        get_phdr_flags(phdr[i].p_flags),
                        phdr[i].p_align);
                }
            }
        }

        printf("\n程序段头表:\n");
        {
            if (header->e_shnum > 0 && header->e_shoff != 0) {
                int maxW = 0;
                char* shStrTab = NULL;
                Elf64_Shdr* shdr = (Elf64_Shdr*)(fAddr + header->e_shoff);
                if (header->e_shstrndx < header->e_shnum) {
                    shStrTab = (char*) (fAddr + shdr[header->e_shstrndx].sh_offset);
                }

                for (int i = 0; i < header->e_shnum; i++) {
                    if (shStrTab && shdr[i].sh_name) {
                        int len = (int) strlen(shStrTab + shdr[i].sh_name);
                        if (len > maxW) {
                            maxW = len;
                        }
                    }
                }

                printf("%-4s %-*s %-12s %-18s %-18s %-18s %-4s %-s\n",
                    "Idx", maxW, "段名", "类型", "虚拟地址", "段文件内偏移", "大小", "标记", "对齐");

                for (int i = 0; i < header->e_shnum; i++) {
                    printf("%-4d %-*s %-18s 0x%lx %-ld %-ld %-4s %-ld\n",
                        i,
                        maxW,
                        shStrTab ? shStrTab + shdr[i].sh_name : "<null>",
                        get_shdr_type(shdr[i].sh_type),
                        shdr[i].sh_addr,
                        shdr[i].sh_offset,
                        shdr[i].sh_size,
                        get_shdr_flags(shdr[i].sh_flags),
                        shdr[i].sh_addralign);
                }
            }
        }

    } while (0);


    if (fd >= 0) { close(fd); }
    C_FREE_FUNC(fAddr, munmap, fSize);

    return 0;
}