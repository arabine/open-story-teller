#ifndef RESOURCEMODEL_H
#define RESOURCEMODEL_H

#include <QtGui>
#include "story_project.h"

#include <QAbstractTableModel>
/*
class CustomTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit CustomTableModel(QObject *parent = nullptr);

    // Reimplemented virtual functions
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
    const int rows = 3;
    const int columns = 3;
    int dataStorage[3][3] = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9}
    };
};

*/

class ResourceModel : public QAbstractTableModel
{

public:
    ResourceModel(QObject * parent = {}) : QAbstractTableModel{parent} {}

    int rowCount(const QModelIndex &) const override { return m_data.count(); }
    int columnCount(const QModelIndex &) const override { return 3; }
    QVariant data(const QModelIndex &index, int role) const override {
        if (role != Qt::DisplayRole && role != Qt::EditRole) return {};
        const auto & res = m_data[index.row()];
        switch (index.column()) {
        case 0: return res.file.c_str();
        case 1: return res.format.c_str();
        case 2: return res.description.c_str();
        default: return {};
        };
    }
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override {
        if (orientation != Qt::Horizontal || role != Qt::DisplayRole) return {};
        switch (section) {
        case 0: return "File";
        case 1: return "Format";
        case 2: return "Description";
        default: return {};
        }
    }
    void append(const Resource & res) {
        beginInsertRows({}, m_data.count(), m_data.count());
        m_data.append(res);
        endInsertRows();
    }

    QList<Resource> GetData() const { return m_data; }

private:
    QList<Resource> m_data;
};

#endif // RESOURCEMODEL_H
