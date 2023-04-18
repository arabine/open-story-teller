#ifndef MEMORY_VIEWDOCK_H
#define MEMORY_VIEWDOCK_H

#include <QDockWidget>
#include <qhexview.h>

class MemoryViewDock : public QDockWidget
{
public:
    MemoryViewDock(const QString &objectName, const QString &title);

    void SetMemory(uint8_t *data, uint32_t size);

private:
    QHexView* m_hexview{nullptr};
};

#endif // MEMORY_VIEWDOCK_H
