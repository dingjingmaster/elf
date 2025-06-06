//
// Created by dingjing on 25-5-22.
//

#ifndef elf_MAIN_WINDOW_H
#define elf_MAIN_WINDOW_H
#include <QLabel>
#include <QMainWindow>

#include "elf-header-model.h"


class MainWindow final : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent=nullptr);
    void parseFile (const QString& fileName);

private:
    qint64                          mFileSize{};
    const qint32                    mMinWidth = 400;
    QLabel*                         mFilePathLabel = nullptr;
    QLabel*                         mFileSizeLabel = nullptr;
    ElfHeaderModel*                 mElfHeaderModel;

};



#endif // elf_MAIN_WINDOW_H
