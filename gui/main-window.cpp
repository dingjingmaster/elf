//
// Created by dingjing on 25-5-22.
//

#include "main-window.h"

#include <QFile>
#include <QLabel>
#include <QGridLayout>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTableWidget>

#include <elf.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "macros.h"


MainWindow::MainWindow(QWidget * parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("Elf file info"));

    setMinimumWidth(mMinWidth);
    setContentsMargins(0, 0, 0, 0);

    auto centralWidget = new QWidget;
    auto mainLayout = new QVBoxLayout();

    auto headerLayout = new QGridLayout;

    // file path
    auto pl = new QLabel;
    pl->setAlignment(Qt::AlignLeft);
    pl->setText(tr("ELF File: "));

    mFilePathLabel = new QLabel;
    mFilePathLabel->setAlignment(Qt::AlignLeft);
    headerLayout->addWidget(pl, 1, 1, 1, 1, Qt::AlignLeft);
    headerLayout->addWidget(mFilePathLabel, 1, 2, 1, 11, Qt::AlignLeft);

    // file size
    auto sl = new QLabel;
    sl->setAlignment(Qt::AlignLeft);
    sl->setText(tr("File Size: "));

    mFileSizeLabel = new QLabel;
    mFileSizeLabel->setAlignment(Qt::AlignLeft);
    headerLayout->addWidget(sl, 2, 1, 1, 1, Qt::AlignLeft);
    headerLayout->addWidget(mFileSizeLabel, 2, 2, 1, 11, Qt::AlignLeft);

    mainLayout->addLayout(headerLayout);

    // title
    auto label = new QLabel(tr("ELF file header: "));
    label->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    mainLayout->addWidget(label);

    // table view
    auto header = new QTableView();
    mElfHeaderModel = new ElfHeaderModel;
    header->setModel(mElfHeaderModel);
    header->setSelectionBehavior(QAbstractItemView::SelectRows);
    header->setSelectionMode(QAbstractItemView::SingleSelection);
    auto h = header->horizontalHeader();
    h->setSectionResizeMode(QHeaderView::Stretch);
    h->setSelectionBehavior(QAbstractItemView::SelectRows);
    h->setSelectionMode(QAbstractItemView::SingleSelection);
    mainLayout->addWidget(header);

    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);
}

void MainWindow::parseFile(const QString& fileName)
{
    C_RETURN_IF_OK(fileName.isNull() || fileName.isEmpty() || !QFile::exists(fileName));

    mFilePathLabel->setText(fileName);

    int fd = 0;
    void* fAddr = nullptr;
    QFile file(fileName);
    mFileSize = file.size();
    mFileSizeLabel->setText(QString("%1 (%2)").arg(mFileSize).arg(QLocale().formattedDataSize(mFileSize)));


    // TODO:// 检查文件是否存在
    // TODO:// 检查是否是文件
    // TODO:// 检查是否是 ELF 文件

    do {
        fd = open(file.fileName().toUtf8().constData(), O_RDONLY);
        C_BREAK_IF_FAIL(fd >=0);

        fAddr = mmap(nullptr, mFileSize, PROT_READ, MAP_PRIVATE, fd, 0);
        C_BREAK_IF_OK(MAP_FAILED == fAddr);

        // 读取 ELF 头
        Elf64_Ehdr* header = (Elf64_Ehdr*)fAddr;

        // 检查 ELF 魔数
        if (0 != memcmp(header->e_ident, ELFMAG, SELFMAG)) {
            // TODO:// 弹框提醒
            // printf("不是ELF文件\n");
            break;
        }

        // magic
        QString magicVal;
        QString magicKey = tr("magic: ");
        for (int i = 0; i < EI_NIDENT; ++i) {
            magicVal.append(QString("%1 ").arg(header->e_ident[i], 2, 16, QLatin1Char('0')));
        }
        mElfHeaderModel->addRow(magicKey, magicVal);

        // class
        switch (header->e_ident[EI_CLASS]) {
            case ELFCLASS32: mElfHeaderModel->addRow(tr("Class: "), tr("ELF32")); break;
            case ELFCLASS64: mElfHeaderModel->addRow(tr("Class: "), tr("ELF64")); break;
            default:
            case ELFCLASSNONE: mElfHeaderModel->addRow(tr("Class: "), tr("ELFNONE")); break;
        }

        // data
        switch (header->e_ident[EI_DATA]) {
            case ELFDATA2LSB: mElfHeaderModel->addRow(tr("Data: "), tr("2's complement, little endian")); break;
            case ELFDATA2MSB: mElfHeaderModel->addRow(tr("Data: "), tr("2's complement, big endian")); break;
            default:
            case ELFDATANONE: mElfHeaderModel->addRow(tr("Data: "), tr("Invalid data encoding")); break;
        }

        // version
        if (header->e_ident[EI_VERSION] == EV_CURRENT) {
            mElfHeaderModel->addRow(tr("Version: "), tr("Current version"));
        }
        else {
            mElfHeaderModel->addRow(tr("Version: "), tr("Invalid ELF version"));
        }

        // OS/ABI
        switch (header->e_ident[EI_OSABI]) {
            case ELFOSABI_NONE: mElfHeaderModel->addRow(tr("OS/ABI: "), tr("UNIX System V ABI")); break;
            case ELFOSABI_HPUX: mElfHeaderModel->addRow(tr("OS/ABI: "), tr("HP-UX")); break;
            case ELFOSABI_NETBSD: mElfHeaderModel->addRow(tr("OS/ABI: "), tr("NetBSD")); break;
            case ELFOSABI_GNU /* ELFOSABI_LINUX */: mElfHeaderModel->addRow(tr("OS/ABI: "), tr("Object uses GNU ELF extensions")); break;
            case ELFOSABI_SOLARIS: mElfHeaderModel->addRow(tr("OS/ABI: "), tr("Sun Solaris")); break;
            case ELFOSABI_AIX: mElfHeaderModel->addRow(tr("OS/ABI: "), tr("IBM AIX")); break;
            case ELFOSABI_IRIX: mElfHeaderModel->addRow(tr("OS/ABI: "), tr("SGI Irix")); break;
            case ELFOSABI_FREEBSD: mElfHeaderModel->addRow(tr("OS/ABI: "), tr("FreeBSD")); break;
            case ELFOSABI_TRU64: mElfHeaderModel->addRow(tr("OS/ABI: "), tr("Compaq TRU64 UNIX")); break;
            case ELFOSABI_MODESTO: mElfHeaderModel->addRow(tr("OS/ABI: "), tr("Novell Modesto")); break;
            case ELFOSABI_OPENBSD: mElfHeaderModel->addRow(tr("OS/ABI: "), tr("OpenBSD")); break;
            case ELFOSABI_ARM_AEABI: mElfHeaderModel->addRow(tr("OS/ABI: "), tr("ARM EABI")); break;
            case ELFOSABI_ARM: mElfHeaderModel->addRow(tr("OS/ABI: "), tr("ARM")); break;
            case ELFOSABI_STANDALONE: mElfHeaderModel->addRow(tr("OS/ABI: "), tr("Standalone (embedded) application")); break;
            default: mElfHeaderModel->addRow(tr("OS/ABI: "), tr("Unknown")); break;
        }

        // ABI version
        mElfHeaderModel->addRow(tr("ABI version: "), QString("%1").arg(header->e_ident[EI_ABIVERSION]));

        // type
        switch (header->e_type) {
            default:
            case ET_NONE:   mElfHeaderModel->addRow(tr("Object file type: "), tr("No file type")); break;
            case ET_REL:    mElfHeaderModel->addRow(tr("Object file type: "), tr("Relocatable file")); break;
            case ET_EXEC:   mElfHeaderModel->addRow(tr("Object file type: "), tr("Executable file")); break;
            case ET_DYN:    mElfHeaderModel->addRow(tr("Object file type: "), tr("Shared object file")); break;
            case ET_CORE:   mElfHeaderModel->addRow(tr("Object file type: "), tr("Core file")); break;
            case ET_LOOS:   mElfHeaderModel->addRow(tr("Object file type: "), tr("OS-specific range start")); break;
            case ET_HIOS:   mElfHeaderModel->addRow(tr("Object file type: "), tr("OS-specific range end")); break;
            case ET_LOPROC: mElfHeaderModel->addRow(tr("Object file type: "), tr("Processor-specific range start")); break;
            case ET_HIPROC: mElfHeaderModel->addRow(tr("Object file type: "), tr("Processor-specific range end")); break;
        }

        auto getMachine = [=] (int m) -> QString {
            switch (m) {
                default:
                case EM_NONE: return tr("No machine");
                case EM_M32:return tr("AT&T WE 32100");
                case EM_SPARC:return tr("SUN SPARC");
                case EM_386:return tr("Intel 80386");
                case EM_68K:return tr("Motorola m68k family");
                case EM_88K:return tr("Motorola m88k family");
                case EM_IAMCU:return tr("Intel MCU");
                case EM_860:return tr("Intel 80860");
                case EM_MIPS:return tr("MIPS R3000 big-endian");
                case EM_S370:return tr("IBM System/370");
                case EM_MIPS_RS3_LE:return tr("MIPS R3000 little-endian");
                case EM_PARISC:return tr("HPPA");
                case EM_VPP500: return tr("Fujitsu VPP500");
                case EM_SPARC32PLUS: return tr("Sun's 'v8plus'");
                case EM_960: return tr("Intel 80960");
                case EM_PPC: return tr("PowerPC");
                case EM_PPC64: return tr("PowerPC 64-bit");
                case EM_S390: return tr("IBM S390");
                case EM_SPU: return tr("IBM SPU/SPC");
                case EM_V800: return tr("NEC V800 series");
                case EM_FR20: return tr("Fujitsu FR20");
                case EM_RH32: return tr("TRW RH-32");
                case EM_RCE: return tr("Motorola RCE");
                case EM_ARM: return tr("ARM");
                case EM_FAKE_ALPHA: return tr("Digital Alpha");
                case EM_SH: return tr("Hitachi SH");
                case EM_SPARCV9: return tr("SPARC v9 64-bit");
                case EM_TRICORE: return tr("Siemens Tricore");
                case EM_ARC: return tr("Argonaut RISC Core");
                case EM_H8_300: return tr("Hitachi H8/300");
                case EM_H8_300H: return tr("Hitachi H8/300H");
                case EM_H8S: return tr("Hitachi H8S");
                case EM_H8_500: return tr("Hitachi H8/500");
                case EM_IA_64: return tr("Intel Merced");
                case EM_MIPS_X: return tr("Stanford MIPS-X");
                case EM_COLDFIRE: return tr("Motorola Coldfire");
                case EM_68HC12: return tr("Motorola M68HC12");
                case EM_MMA: return tr("Fujitsu MMA Multimedia Accelerator");
                case EM_PCP: return tr("Siemens PCP");
                case EM_NCPU: return tr("Sony nCPU embedded RISC");
                case EM_NDR1: return tr("Denso NDR1 microprocessor");
                case EM_STARCORE: return tr("Motorola Start*Core processor");
                case EM_ME16: return tr("Toyota ME16 processor");
                case EM_ST100: return tr("STMicroelectronic ST100 processor");
                case EM_TINYJ: return tr("Advanced Logic Corp. Tinyj emb.fam");
                case EM_X86_64: return tr("AMD x86-64 architecture");
                case EM_PDSP: return tr("Sony DSP Processor");
                case EM_PDP10: return tr("Digital PDP-10");
                case EM_PDP11: return tr("Digital PDP-11");
                case EM_FX66: return tr("Siemens FX66 microcontroller");
                case EM_ST9PLUS: return tr("STMicroelectronics ST9+ 8/16 mc");
                case EM_ST7: return tr("STmicroelectronics ST7 8 bit mc");
                case EM_68HC16: return tr("Motorola MC68HC16 microcontroller");
                case EM_68HC11: return tr("Motorola MC68HC11 microcontroller");
                case EM_68HC08: return tr("Motorola MC68HC08 microcontroller");
                case EM_68HC05: return tr("Motorola MC68HC05 microcontroller");
                case EM_SVX: return tr("Silicon Graphics SVx");
                case EM_ST19: return tr("STMicroelectronics ST19 8 bit mc");
                case EM_VAX: return tr("Digital VAX");
                case EM_CRIS: return tr("Axis Communications 32-bit emb.proc");
                case EM_JAVELIN: return tr("Infineon Technologies 32-bit emb.proc");
                case EM_FIREPATH: return tr("Element 14 64-bit DSP Processor");
                case EM_ZSP: return tr("LSI Logic 16-bit DSP Processor");
                case EM_MMIX: return tr("Donald Knuth's educational 64-bit proc");
                case EM_HUANY: return tr("Harvard University machine-independent object files");
                case EM_PRISM: return tr("SiTera Prism");
                case EM_AVR: return tr("Atmel AVR 8-bit microcontroller");
                case EM_FR30: return tr("Fujitsu FR30");
                case EM_D10V: return tr("Mitsubishi D10V");
                case EM_D30V: return tr("Mitsubishi D30V");
                case EM_V850: return tr("NEC v850");
                case EM_M32R: return tr("Mitsubishi M32R");
                case EM_MN10300: return tr("Matsushita MN10300");
                case EM_MN10200: return tr("Matsushita MN10200");
                case EM_PJ: return tr("picoJava");
                case EM_OPENRISC: return tr("OpenRISC 32-bit embedded processor");
                case EM_ARC_COMPACT: return tr("ARC International ARCompact");
                case EM_XTENSA: return tr("Tensilica Xtensa Architecture");
                case EM_VIDEOCORE: return tr("Alphamosaic VideoCore");
                case EM_TMM_GPP: return tr("Thompson Multimedia General Purpose Proc");
                case EM_NS32K: return tr("National Semi. 32000");
                case EM_TPC: return tr("Tenor Network TPC");
                case EM_SNP1K: return tr("Trebia SNP 1000");
                case EM_ST200: return tr("STMicroelectronics ST200");
                case EM_IP2K: return tr("Ubicom IP2xxx");
                case EM_MAX: return tr("MAX processor");
                case EM_CR: return tr("National Semi. CompactRISC");
                case EM_F2MC16: return tr("Fujitsu F2MC16");
                case EM_MSP430: return tr("Texas Instruments msp430");
                case EM_BLACKFIN: return tr("Analog Devices Blackfin DSP");
                case EM_SE_C33:return tr("Seiko Epson S1C33 family");
                case EM_SEP:return tr("Sharp embedded microprocessor");
                case EM_ARCA:return tr("Arca RISC");
                case EM_UNICORE: return tr("PKU-Unity & MPRC Peking Uni. mc series");
                case EM_EXCESS: return tr("eXcess configurable cpu");
                case EM_DXP:return tr("Icera Semi. Deep Execution Processor");
                case EM_ALTERA_NIOS2:return tr("Altera Nios II");
                case EM_CRX:return tr("National Semi. CompactRISC CRX");
                case EM_XGATE:return tr("Motorola XGATE");
                case EM_C166:return tr("Infineon C16x/XC16x");
                case EM_M16C:return tr("Renesas M16C");
                case EM_DSPIC30F:return tr("Microchip Technology dsPIC30F");
                case EM_CE:return tr("Freescale Communication Engine RISC");
                case EM_M32C:return tr("Renesas M32C");
                case EM_TSK3000: return tr("Altium TSK3000");
                case EM_RS08: return tr("Freescale RS08");
                case EM_SHARC: return tr("Analog Devices SHARC family");
                case EM_ECOG2: return tr("Cyan Technology eCOG2");
                case EM_SCORE7: return tr("Sunplus S+core7 RISC");
                case EM_DSP24: return tr("New Japan Radio (NJR) 24-bit DSP");
                case EM_VIDEOCORE3: return tr("Broadcom VideoCore III");
                case EM_LATTICEMICO32: return tr("RISC for Lattice FPGA");
                case EM_SE_C17: return tr("Seiko Epson C17");
                case EM_TI_C6000: return tr("Texas Instruments TMS320C6000 DSP");
                case EM_TI_C2000: return tr("Texas Instruments TMS320C2000 DSP");
                case EM_TI_C5500: return tr("Texas Instruments TMS320C55x DSP");
                case EM_TI_ARP32: return tr("Texas Instruments App. Specific RISC");
                case EM_TI_PRU: return tr("Texas Instruments Prog. Realtime Unit");
                case EM_MMDSP_PLUS: return tr("STMicroelectronics 64bit VLIW DSP");
                case EM_CYPRESS_M8C: return tr("Cypress M8C");
                case EM_R32C: return tr("Renesas R32C");
                case EM_TRIMEDIA: return tr("NXP Semi. TriMedia");
                case EM_QDSP6: return tr("QUALCOMM DSP6");
                case EM_8051: return tr("Intel 8051 and variants");
                case EM_STXP7X: return tr("STMicroelectronics STxP7x");
                case EM_NDS32: return tr("Andes Tech. compact code emb. RISC");
                case EM_ECOG1X: return tr("Cyan Technology eCOG1X");
                case EM_MAXQ30: return tr("Dallas Semi. MAXQ30 mc");
                case EM_XIMO16: return tr("New Japan Radio (NJR) 16-bit DSP");
                case EM_MANIK: return tr("M2000 Reconfigurable RISC");
                case EM_CRAYNV2: return tr("Cray NV2 vector architecture");
                case EM_RX: return tr("Renesas RX");
                case EM_METAG: return tr("Imagination Tech. META");
                case EM_MCST_ELBRUS: return tr("MCST Elbrus");
                case EM_ECOG16: return tr("Cyan Technology eCOG16");
                case EM_CR16: return tr("National Semi. CompactRISC CR16");
                case EM_ETPU: return tr("Freescale Extended Time Processing Unit");
                case EM_SLE9X: return tr("Infineon Tech. SLE9X");
                case EM_L10M: return tr("Intel L10M");
                case EM_K10M: return tr("Intel K10M");
                case EM_AARCH64: return tr("ARM AARCH64");
                case EM_AVR32: return tr("Amtel 32-bit microprocessor");
                case EM_STM8: return tr("STMicroelectronics STM8");
                case EM_TILE64: return tr("Tilera TILE64");
                case EM_TILEPRO: return tr("Tilera TILEPro");
                case EM_MICROBLAZE: return tr("Xilinx MicroBlaze");
                case EM_CUDA: return tr("NVIDIA CUDA");
                case EM_TILEGX: return tr("Tilera TILE-Gx");
                case EM_CLOUDSHIELD: return tr("CloudShield");
                case EM_COREA_1ST: return tr("KIPO-KAIST Core-A 1st gen.");
                case EM_COREA_2ND: return tr("KIPO-KAIST Core-A 2nd gen.");
                case EM_ARCV2: return tr("Synopsys ARCv2 ISA. ");
                case EM_OPEN8: return tr("Open8 RISC");
                case EM_RL78: return tr("Renesas RL78");
                case EM_VIDEOCORE5: return tr("Broadcom VideoCore V");
                case EM_78KOR: return tr("Renesas 78KOR");
                case EM_56800EX: return tr("Freescale 56800EX DSC");
                case EM_BA1: return tr("Beyond BA1");
                case EM_BA2: return tr("Beyond BA2");
                case EM_XCORE: return tr("XMOS xCORE");
                case EM_MCHP_PIC: return tr("Microchip 8-bit PIC(r)");
                case EM_INTELGT: return tr("Intel Graphics Technology");
                case EM_KM32: return tr("KM211 KM32");
                case EM_KMX32: return tr("KM211 KMX32");
                case EM_EMX16: return tr("KM211 KMX16");
                case EM_EMX8: return tr("KM211 KMX8");
                case EM_KVARC: return tr("KM211 KVARC");
                case EM_CDP: return tr("Paneve CDP");
                case EM_COGE: return tr("Cognitive Smart Memory Processor");
                case EM_COOL: return tr("Bluechip CoolEngine");
                case EM_NORC: return tr("Nanoradio Optimized RISC");
                case EM_CSR_KALIMBA: return tr("CSR Kalimba");
                case EM_Z80: return tr("Zilog Z80");
                case EM_VISIUM: return tr("Controls and Data Services VISIUMcore");
                case EM_FT32: return tr("FTDI Chip FT32");
                case EM_MOXIE: return tr("Moxie processor");
                case EM_AMDGPU: return tr("AMD GPU");
                case EM_RISCV: return tr("RISC-V");
                case EM_BPF: return tr("Linux BPF -- in-kernel virtual machine");
                case EM_CSKY: return tr("C-SKY");
                case EM_LOONGARCH: return tr("LoongArch");
            }
            return tr("unknown");
        };

        // machine
        mElfHeaderModel->addRow(tr("Architecture: "), getMachine(header->e_machine));

        // version
        mElfHeaderModel->addRow(tr("Object file version: "), QString("%1").arg(header->e_version));

        // entry
        mElfHeaderModel->addRow(tr("Entry point virtual address: "), QString("0x%1").arg(header->e_entry, 0, 16, QLatin1Char('0')));

        // ph offset
        mElfHeaderModel->addRow(tr("Program header table file offset: "), QString("%1").arg(header->e_phoff, 0, 10, QLatin1Char('0')));

        // sh offset
        mElfHeaderModel->addRow(tr("Section header table file offset: "), QString("%1").arg(header->e_shoff, 0, 10, QLatin1Char('0')));

        // flags
        mElfHeaderModel->addRow(tr("Processor-specific flags: "), QString("0x%1").arg(header->e_flags, 0, 16, QLatin1Char('0')));

        // ELF header size
        mElfHeaderModel->addRow(tr("ELF header size in bytes: "), QString("%1 (bytes)").arg(header->e_ehsize, 0, 10, QLatin1Char('0')));

        // ph size
        mElfHeaderModel->addRow(tr("Program header table entry size: "), QString("%1 (bytes)").arg(header->e_phentsize, 0, 10, QLatin1Char('0')));

        // ph num
        mElfHeaderModel->addRow(tr("Program header table entry count: "), QString("%1").arg(header->e_phnum, 0, 10, QLatin1Char('0')));

        // sh size
        mElfHeaderModel->addRow(tr("Section header table entry size: "), QString("%1 (bytes)").arg(header->e_shentsize, 0, 10, QLatin1Char('0')));

        // sh num
        mElfHeaderModel->addRow(tr("Section header table entry count: "), QString("%1").arg(header->e_shnum, 0, 10, QLatin1Char('0')));

        // sh strndx
        mElfHeaderModel->addRow(tr("Section header string table index: "), QString("%1").arg(header->e_shstrndx, 0, 10, QLatin1Char('0')));
    } while (false);

    if (fd >= 0) {
        ::close(fd);
    }
    C_FREE_FUNC(fAddr, munmap, mFileSize);
}
