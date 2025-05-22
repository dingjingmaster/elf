//
// Created by dingjing on 25-5-22.
//
#include "main-window.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QApplication>


int main (int argc, char* argv[])
{
    QApplication app(argc, argv);

    QString file;
    if (argc != 2) {
        QMessageBox msg(QMessageBox::Information,
            QObject::tr("Choose a elf file."),
            QObject::tr("Please select the ELF file you want to view."),
            QMessageBox::Ok | QMessageBox::Cancel);
        int ret = msg.exec();
        if (ret == QMessageBox::Cancel) {
            return 0;
        }
        QFileDialog fi(nullptr, QString(), "/usr/bin");
        ret = fi.exec();
        fi.setAcceptMode(QFileDialog::AcceptOpen);
        fi.setFileMode(QFileDialog::ExistingFile);
        if (ret == QFileDialog::Rejected) {
            return 0;
        }

        file = fi.selectedFiles().at(0);
    }
    else {
        file = QDir(argv[1]).absolutePath();
    }

    MainWindow win;

    win.parseFile(file);

    win.show();

    return app.exec();
}