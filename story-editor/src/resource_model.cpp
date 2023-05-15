#include "resource_model.h"


ResourceModel::ResourceModel(StoryProject &project, QObject *parent)
    : QAbstractTableModel{parent}
    , m_project(project)
{

}

int ResourceModel::rowCount(const QModelIndex &) const
{
    return m_project.ResourcesSize();
}

int ResourceModel::columnCount(const QModelIndex &) const
{
    return 4;
}

QVariant ResourceModel::data(const QModelIndex &index, int role) const {
    if (role != Qt::DisplayRole && role != Qt::EditRole) return {};
    Resource res;
    if (m_project.GetResourceAt(index.row(), res))
    {
        switch (index.column()) {
        case 0: return res.file.c_str();
        case 1: return res.format.c_str();
        case 2: return res.description.c_str();
        case 3: return res.type.c_str();
        default: ;
        };
    }
    return {};
}

QVariant ResourceModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) return {};
    switch (section) {
    case 0: return "File";
    case 1: return "Format";
    case 2: return "Description";
    case 3: return "Type";
    default: return {};
    }
}

void ResourceModel::append(const Resource &res) {
    beginInsertRows({}, m_project.ResourcesSize(), m_project.ResourcesSize());
    m_project.AppendResource(res);
    endInsertRows();
}

void ResourceModel::Clear()
{
    beginResetModel();
    m_project.ClearResources();
    endResetModel();
}

void ResourceModel::BeginChange()
{
    beginResetModel();
}

void ResourceModel::EndChange()
{
    endResetModel();
}


// ------------------------------- PROXY MODEL -------------------------------


ResourceFilterProxyModel::ResourceFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

void ResourceFilterProxyModel::setFilterType(const QString & type)
{
    m_typeFilter = type;
    invalidateFilter();
}


bool ResourceFilterProxyModel::filterAcceptsRow(int sourceRow,
                                              const QModelIndex &sourceParent) const
{
    QModelIndex indexType = sourceModel()->index(sourceRow, 3, sourceParent);

    return (sourceModel()->data(indexType).toString() == m_typeFilter);
}
/*
bool ResourceFilterProxyModel::lessThan(const QModelIndex &left,
                                      const QModelIndex &right) const
{
    QVariant leftData = sourceModel()->data(left);
    QVariant rightData = sourceModel()->data(right);

    if (leftData.userType() == QMetaType::QDateTime) {
        return leftData.toDateTime() < rightData.toDateTime();
    } else {
        static const QRegularExpression emailPattern("[\\w\\.]*@[\\w\\.]*");

        QString leftString = leftData.toString();
        if (left.column() == 1) {
            const QRegularExpressionMatch match = emailPattern.match(leftString);
            if (match.hasMatch())
                leftString = match.captured(0);
        }
        QString rightString = rightData.toString();
        if (right.column() == 1) {
            const QRegularExpressionMatch match = emailPattern.match(rightString);
            if (match.hasMatch())
                rightString = match.captured(0);
        }

        return QString::localeAwareCompare(leftString, rightString) < 0;
    }
}

*/


