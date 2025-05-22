//
// Created by dingjing on 25-5-22.
//

#include "elf-header-model.h"

#include <QDebug>


ElfHeaderModel::ElfHeaderModel(QObject* parent)
    : QAbstractTableModel(parent)
{
}

int ElfHeaderModel::rowCount(const QModelIndex& parent) const
{
    return mHeaders.size();
}

int ElfHeaderModel::columnCount(const QModelIndex& parent) const
{
    return 2;
}

QVariant ElfHeaderModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Vertical) { return {}; }

    switch (section) {
    case 0:
        if (role == Qt::DisplayRole) {
            return tr("Field name");
        }
    case 1:
        if (role == Qt::DisplayRole) {
            return tr("Field value");
        }
    default:
        break;
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

void ElfHeaderModel::addRow(const QString& key, const QString& value)
{
    const int c = mHeaders.size();

    beginInsertRows(QModelIndex(), c, c + 1);
    if (!mIdxKey.contains(key)) {
        qInfo() << "Added header key" << key << "value" << value;
        mHeaders.append({key, value});
    }
    endInsertRows();
}

QVariant ElfHeaderModel::data(const QModelIndex& index, int role) const
{
    qInfo() << "Index: " << index.row() << " role: " << role;
    switch (index.column()) {
    case 0: {
        if (role == Qt::DisplayRole) {
            return mHeaders.at(index.row()).first;
        }
        break;
    }
    case 1: {
        if (role == Qt::DisplayRole) {
            return mHeaders.at(index.row()).second;
        }
        break;
    }
    default: break;
    }

    return {};
}
