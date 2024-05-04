
#include <QtWidgets>

#include "dragwidget.h"

DragWidget::DragWidget(QWidget *parent)
    : QFrame(parent)
{
    QLabel *FileIcon;

    setAcceptDrops(true);

    FileIcon = new QLabel(this);
    if (NULL != FileIcon)
    {
        FileIcon->setPixmap(QPixmap(":/UploadIcon.png"));
        FileIcon->show();
        FileIcon->setAttribute(Qt::WA_DeleteOnClose);
    }
}

void DragWidget::dragEnterEvent(QDragEnterEvent *event)
{
//    qDebug("DragWidget::dragEnterEvent");

    if (event->mimeData()->hasFormat("application/x-dnditemdata"))
    {
        if (event->source() == this)
        {
            event->setDropAction(Qt::MoveAction);
            event->accept();
        }
        else
        {
            event->acceptProposedAction();
        }
    }
    else
    {
        event->ignore();
    }
}

void DragWidget::dragMoveEvent(QDragMoveEvent *event)
{
//    qDebug("DragWidget::dragMoveEvent");

    if (event->mimeData()->hasFormat("application/x-dnditemdata"))
    {
        if (event->source() == this)
        {
            event->setDropAction(Qt::MoveAction);
            event->accept();
        }
        else
        {
            event->acceptProposedAction();
        }
    }
    else
    {
        event->ignore();
    }
}

void DragWidget::dropEvent(QDropEvent *event)
{
    qDebug("DragWidget::dropEvent");

    if (event->mimeData()->hasFormat("application/x-dnditemdata"))
    {
        QByteArray itemData = event->mimeData()->data("application/x-dnditemdata");
        QDataStream dataStream(&itemData, QIODevice::ReadOnly);

        //QString Name;
        //dataStream >> Name;

        if (event->source() == this)
        {
            event->setDropAction(Qt::MoveAction);
            event->accept();
        }
        else
        {
            event->acceptProposedAction();
        }
    }
    else
    {
        event->ignore();
    }
}

void DragWidget::mousePressEvent(QMouseEvent *event)
{
//    qDebug("DragWidget::mousePressEvent");

    QString Name;
    QByteArray itemData;
    QDataStream dataStream(&itemData, QIODevice::WriteOnly);

    Name = objectName();
    dataStream << Name;

    QMimeData *mimeData = new QMimeData;
    mimeData->setData("application/x-dnditemdata", itemData);

    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);

    if (drag->exec(Qt::CopyAction | Qt::MoveAction, Qt::CopyAction) == Qt::MoveAction)
    {
        //child->close();
    }
    else
    {
//        child->show();
//        child->setPixmap(pixmap);
    }
}
