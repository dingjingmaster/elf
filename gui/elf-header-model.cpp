//
// Created by dingjing on 25-5-22.
//

#include "elf-header-model.h"

#include <QDebug>

#include "macros.h"


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
    C_RETURN_IF_OK(key.isNull() || key.isEmpty());

    const int c = mHeaders.size();

    beginInsertRows(QModelIndex(), c, c);
    if (!mIdxKey.contains(key)) {
        // qInfo() << "Added header key" << key << "value" << value;
        mHeaders.append({key, value});
    }
    endInsertRows();
}

QVariant ElfHeaderModel::data(const QModelIndex& index, int role) const
{
    switch (index.column()) {
    case 0: {
        if (role == Qt::DisplayRole) {
            return mHeaders.at(index.row()).first;
        }
        else if (role == Qt::TextAlignmentRole) {
            return QVariant(Qt::AlignHCenter | Qt::AlignRight);
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
