#include "resource_model.h"


QVariant ResourceModel::data(const QModelIndex &index, int role) const {
    if (role != Qt::DisplayRole && role != Qt::EditRole) return {};
    const auto & res = m_data[index.row()];
    switch (index.column()) {
    case 0: return res.file.c_str();
    case 1: return res.format.c_str();
    case 2: return res.description.c_str();
    default: return {};
    };
}

QVariant ResourceModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) return {};
    switch (section) {
    case 0: return "File";
    case 1: return "Format";
    case 2: return "Description";
    default: return {};
    }
}

void ResourceModel::append(const Resource &res) {
    beginInsertRows({}, m_data.count(), m_data.count());
    m_data.append(res);
    endInsertRows();
}

void ResourceModel::Clear()
{
    beginResetModel();
    m_data.clear();
    endResetModel();
}

QString ResourceModel::GetFileName(int row) {
    QString n;

    if (row < m_data.size())
    {
        n = m_data.at(row).file.c_str();
    }

    return n;
}
