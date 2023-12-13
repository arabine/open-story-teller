#ifndef RESOURCE_MODEL_H
#define RESOURCE_MODEL_H

#include <QtGui>
#include "story_project.h"

#include <QAbstractTableModel>

class ResourceModel : public QAbstractTableModel
{

public:
    ResourceModel(StoryProject &project, QObject * parent = {});

    int rowCount(const QModelIndex &) const override;
    int columnCount(const QModelIndex &) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    void Append(const Resource & res);
    void Delete(int row);
    void Clear();
    void BeginChange();
    void EndChange();

private:
    StoryProject &m_project;
};

class ResourceFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    ResourceFilterProxyModel(QObject *parent = nullptr);


    void setFilterType(const QString &type);
protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
//    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

private:

    QString m_typeFilter;
};

#endif // RESOURCE_MODEL_H
