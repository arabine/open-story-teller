#ifndef RESOURCE_MODEL_H
#define RESOURCE_MODEL_H

#include <QtGui>
#include "story_project.h"

#include <QAbstractTableModel>

class ResourceModel : public QAbstractTableModel
{

public:
    ResourceModel(QObject * parent = {}) : QAbstractTableModel{parent} {}

    int rowCount(const QModelIndex &) const override { return m_data.count(); }
    int columnCount(const QModelIndex &) const override { return 3; }
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    void append(const Resource & res);

    void Clear();

    QString GetFileName(int row);

    QList<Resource> GetData() const { return m_data; }

private:
    QList<Resource> m_data;
};

#endif // RESOURCE_MODEL_H
