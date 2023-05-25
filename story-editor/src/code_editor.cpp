#include "code_editor.h"

#include <QPainter>
#include <QTextBlock>
#include <QFontMetrics>

CodeEditor::CodeEditor(QWidget *parent) : QPlainTextEdit(parent)
{
    lineNumberArea = new LineNumberArea(this);

    connect(this, &CodeEditor::blockCountChanged, this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &CodeEditor::updateRequest, this, &CodeEditor::updateLineNumberArea);
    connect(this, &CodeEditor::cursorPositionChanged, this, &CodeEditor::highlightCurrentLine);

    connect(lineNumberArea, &LineNumberArea::sigLineNumberAreaClicked, this, &CodeEditor::sigLineNumberAreaClicked);

    setTabStopDistance(QFontMetricsF(font()).horizontalAdvance(' ') * 4);

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
}

int CodeEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;

    return space;
}


void CodeEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}


void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}



void CodeEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}


void CodeEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = QColor(Qt::darkGray).lighter(160);

        selection.format.setBackground(lineColor);
        selection.format.setForeground(Qt::black);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}


void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), Qt::lightGray);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::black);
            painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(),
                             Qt::AlignRight, number);

            if (m_breakpoints.contains(blockNumber +1))
            {
                QRectF rectangle(0, top, fontMetrics().height(), fontMetrics().height());
                painter.setPen(Qt::red);
                painter.setBrush(QBrush(Qt::red));
                painter.drawEllipse(rectangle);
            }
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }

}

int CodeEditor::ComputeLine(const QPointF &p)
{
    int line = -1;

    QTextBlock block = firstVisibleBlock();

    while (block.isValid())
    {
        int top = qRound(blockBoundingGeometry(block).top());
        int bottom = top + qRound(blockBoundingRect(block).height());

        if ((p.y() >= top) && (p.y() <= bottom))
        {
            line  = block.blockNumber() + 1;
            break;
        }

        block = block.next();
    }

    return line;
}

void CodeEditor::SetBreakPoints(const std::set<int> &bkp)
{
    m_breakpoints = bkp;
    update();
}


void LineNumberArea::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton)
    {
        QPointF p = event->position();
        int line = codeEditor->ComputeLine(p);
//        qDebug() << "Clicked line: " << line;
        if (line >= 0)
        {
            emit sigLineNumberAreaClicked(line);
        }
    }
}
