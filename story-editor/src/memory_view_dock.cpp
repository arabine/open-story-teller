#include "memory_view_dock.h"
#include "model/buffer/qmemorybuffer.h"

MemoryViewDock::MemoryViewDock(const QString &objectName, const QString &title)
    : DockWidgetBase(title, false)
{
    setObjectName(objectName);

    QHexOptions options;
    options.grouplength = 1; // Pack bytes as AABB
    options.bytecolors[0x00] = {Qt::lightGray, QColor()}; // Highlight '00's
    options.bytecolors[0xFF] = {Qt::darkBlue, QColor()};  // Highlight 'FF's

    m_hexview = new QHexView();
    m_hexview->setOptions(options);
    setWidget(m_hexview);
}

void MemoryViewDock::SetMemory(uint8_t *data, uint32_t size)
{
    QHexDocument* document = QHexDocument::fromMemory<QMemoryBuffer>(reinterpret_cast<char*>(data), size); /* Load data from In-Memory Buffer... */
    m_hexview->setDocument(document);
}
