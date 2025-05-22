//
// Created by dingjing on 25-5-22.
//

#include "main-window.h"

#include <QFile>
#include <QLabel>
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

    setContentsMargins(0, 0, 0, 0);

    auto centralWidget = new QWidget;
    auto mainLayout = new QVBoxLayout();

    // file path
    auto phl = new QHBoxLayout;
    auto pl = new QLabel;
    pl->setAlignment(Qt::AlignLeft);
    pl->setText(tr("ELF File: "));

    mFilePath = new QLabel;
    mFilePath->setAlignment(Qt::AlignLeft);
    phl->addWidget(pl);
    phl->addWidget(mFilePath);
    phl->addStretch();
    mainLayout->addLayout(phl);

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
    mFilePath->setText(fileName);

    int fd = 0;
    void* fAddr = nullptr;
    QFile file(fileName);
    qint64 fSize = file.size();

    // TODO:// 检查文件是否存在
    // TODO:// 检查是否是文件
    // TODO:// 检查是否是 ELF 文件

    do {
        fd = open(file.fileName().toUtf8().constData(), O_RDONLY);
        C_BREAK_IF_FAIL(fd >=0);

        fAddr = mmap(nullptr, fSize, PROT_READ, MAP_PRIVATE, fd, 0);
        C_BREAK_IF_OK(MAP_FAILED == fAddr);

        // 读取 ELF 头
        Elf64_Ehdr* header = (Elf64_Ehdr*)fAddr;

        // 检查 ELF 魔数
        if (0 != memcmp(header->e_ident, ELFMAG, SELFMAG)) {
            // TODO:// 弹框提醒
            // printf("不是ELF文件\n");
            break;
        }

        QString magicVal;
        QString magicKey = tr("magic: ");
        for (int i = 0; i < EI_NIDENT; ++i) {
            magicVal.append(QString("%1 ").arg(header->e_ident[i]));
        }
        mElfHeaderModel->addRow(magicKey, magicVal);

    } while (false);

    if (fd >= 0) {
        ::close(fd);
    }
    C_FREE_FUNC(fAddr, munmap, fSize);


}
