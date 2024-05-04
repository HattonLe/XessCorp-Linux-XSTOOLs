#include "mylist.h"

#include "../xstoolslib/utils.h"

// https://surfer.nmr.mgh.harvard.edu/ftp/dist/freesurfer/tutorial_versions/freesurfer/lib/qt/qt_doc/html/model-view-programming.html

MyList::MyList(QWidget *Parent) :
    QListWidget(Parent)
{
    //setViewMode(QListView::ListMode);
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDragDropMode(DragDropMode::DragDrop);
    setDragDropOverwriteMode(true);
    setDefaultDropAction(Qt::DropAction::CopyAction);
    setSelectionMode(QAbstractItemView::ExtendedSelection);

    //connect(this, SIGNAL(itemSelectionChanged()), SLOT(selectionChanged()));
}

MyList::~MyList()
{
}

void MyList::AddTo(QString FileUrl)
{
    QString Directory;
    QString Filename;

    // Strip off the file://
    if (7 < FileUrl.length())
    {
        Directory = FileUrl.mid(7);

        // Assuming we have anything left
        if (0 < Directory.length())
        {
            // Strip out filename & extension
            Filename = QString::fromStdString(StripPrefix(FileUrl.toStdString()));

            // Assuming we have anything left
            if (0 < Filename.length())
            {
                QListWidgetItem *NewItem;

                NewItem = new QListWidgetItem(this);
                if (NULL != NewItem)
                {
                    NewItem->setText(Filename);
                    NewItem->setData(Qt::UserRole, QVariant(Directory));
                    NewItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);

                    // Put the entry into the list box
                    addItem(NewItem);
                }
            }
        }
    }
}

QStringList *MyList::SelectedItems()
{
   QStringList *slist;

   slist = new QStringList();

   if (NULL != model())
   {
        if (0 < model()->rowCount())
        {
            QList<QListWidgetItem *> SelectedList;

            SelectedList = selectedItems();
            foreach (QListWidgetItem *Selected, SelectedList)
            {
                QString name = Selected->data(Qt::UserRole).value<QString>();

                slist->append(name);
            }
            //qDebug() << slist.join(",");
        }
    }
    return slist;
}

void MyList::HandleKeyEvent(QKeyEvent *keyEvent)
{
    if (keyEvent->KeyPress)
    {
        //qDebug() "KeyCode:" << keyEvent->key();

        if (Qt::Key_Delete == keyEvent->key())
        {
            RemoveSelectedItems();
        }
    }
}

void MyList::RemoveSelectedItems()
{
    QList<QListWidgetItem *> SelectedList;

    SelectedList = selectedItems();
    foreach (QListWidgetItem *Selected, SelectedList)
    {
        QString name = Selected->data(Qt::UserRole).value<QString>();
        // remove each item
        removeItemWidget(Selected);
        delete Selected;
    }
}

void MyList::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    QListWidget::selectionChanged(selected, deselected);

    emit SelectionUpdated(QString::number(selectedItems().count()));
}

// https://wiki.qt.io/QList_Drag_and_Drop_Example

void MyList::dropEvent(QDropEvent *event)
{
//    qDebug("MyList::dropEvent");

    if (event->mimeData()->hasFormat("application/x-dnditemdata"))
    {
        QByteArray itemData = event->mimeData()->data("application/x-dnditemdata");
        QDataStream dataStream(&itemData, QIODevice::ReadOnly);

        QString Name;
        dataStream >> Name;

//        qDebug() << "Droper is " << Name;

        emit FileUploadRequested(Name);
    }

    //if (event->mimeData()->hasFormat("application/x-item"))
    {
        const QMimeData *Info;
        Info = event->mimeData();

        if (Info->hasText()) { qDebug("Text"); }
        if (Info->hasHtml()) { qDebug("Html"); }
        if (Info->hasUrls()) { qDebug("Urls"); }
        if (Info->hasImage()) { qDebug("Image"); }
        if (Info->hasColor()) { qDebug("Color"); }


        // Dropping of multiple dragged files sets text() to concatonated filepaths delimited by \r\n
//        if (event->mimeData()->hasText())
//        {
//            AddToList(event->mimeData()->text());
//        }

        // For each file dropped, get the fullpath and append it to the listbox
        if (event->mimeData()->hasUrls())
        {
            foreach (QUrl url, event->mimeData()->urls())
            {
                QString name;

                name = url.toString();
                AddTo(name);
            }
        }
        //QString name = Info->data("application/x-item");

        //item->setStringList(list);
        //item->setIcon(QIcon(":/images/iString")); //set path to image
        event->accept();
        event->setDropAction(Qt::MoveAction);
    }
    //else
    {
    //    event->ignore();
    }
}

void MyList::dragMoveEvent(QDragMoveEvent *event)
{
    //qDebug("dragMoveEvent");

    event->setDropAction(Qt::MoveAction);
    event->accept();

//    if (event->mimeData()->hasFormat("application/x-item") && event->source() != this)
    {
//        event->setDropAction(Qt::MoveAction);
//        event->accept();
    }
//    else
    {
//        event->ignore();
    }
}

void MyList::startDrag(Qt::DropActions supportedActions)
{
//    qDebug("startDrag");

    //QListWidgetItem* item ;
    //QMimeData* mimeData = new QMimeData;
    //QByteArray ba;

    //item = this-> currentItem();

    //ba = item->text().toLatin1().data();
    //mimeData->setData("application/x-item", ba);
    //QDrag* drag = new QDrag(this);
    //drag->setMimeData(mimeData);
    //if (drag->exec(Qt::MoveAction) == Qt::MoveAction)
    {
        //delete takeItem(row(item));
     //   emit itemDropped();
    }
}

void MyList::dragEnterEvent(QDragEnterEvent *event)
{
//    qDebug("dragEnterEvent");
    event->accept();

//    if (event->mimeData()->hasFormat("application/x-item"))
    {
//        event->accept();
    }
//    else
    {
//        event->ignore();
    }
}

