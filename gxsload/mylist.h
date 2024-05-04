#ifndef MYLIST_H
#define MYLIST_H

#include <QtWidgets>

class MyList : public QListWidget
{
    Q_OBJECT

public slots:
    virtual void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

signals:
    void SelectionUpdated(const QString&);
    void FileUploadRequested(const QString&);
    void itemSelectionChanged();

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);
    void startDrag(Qt::DropActions supportedActions);

 public:
    MyList(QWidget *parent = 0);
    ~MyList();

    void AddTo(QString name);

    void HandleKeyEvent(QKeyEvent *keyEvent);

    void RemoveSelectedItems();

    QStringList *SelectedItems();
};

#endif // MYLIST_H
