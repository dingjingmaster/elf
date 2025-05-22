//
// Created by dingjing on 25-5-22.
//

#ifndef elf_ELF_HEADER_MODEL_H
#define elf_ELF_HEADER_MODEL_H
#include <QSet>
#include <QAbstractTableModel>


class ElfHeaderModel final : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit ElfHeaderModel(QObject *parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex &parent=QModelIndex()) const override ;
    int columnCount(const QModelIndex& parent=QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    void addRow(const QString& key, const QString& value);

private:
    QSet<QString>                       mIdxKey;
    QList<QPair<QString, QString>>      mHeaders;
};




#endif // elf_ELF_HEADER_MODEL_H
