#include "fileViewer.h"

#include <QHeaderView>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QMouseEvent>
#include <QLineEdit>

FileViewer::FileViewer(QWidget *parent)
    : QTreeView(parent)
{
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAlternatingRowColors(true);
    setAnimated(true);
    setAllColumnsShowFocus(true);
    setSortingEnabled(true);
    setDragEnabled(true);
    setDropIndicatorShown(true);

    setDropsEnabled(false);
    header()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

void FileViewer::startDrag(Qt::DropActions supportedActions)
{
    //only start the drag if it's over the filename column. this allows dragging selection in
    //tree/detail view
    if (currentIndex().column() != 0) {
        return;
    }

    QTreeView::startDrag(supportedActions);
}

void FileViewer::expandIfSingleFolder()
{
    if (model()->rowCount() == 1) {
        expandToDepth(0);
    }
}

void FileViewer::setDropsEnabled(bool enabled)
{
    setAcceptDrops(enabled);
    setDragDropMode(enabled ? QAbstractItemView::DragDrop : QAbstractItemView::DragOnly);
}

void FileViewer::dragEnterEvent(QDragEnterEvent * event)
{


    if (event->source() == this) {
        //we don't support internal drops yet.
        return;
    }

    QTreeView::dragEnterEvent(event);
}

void FileViewer::dropEvent(QDropEvent * event)
{

    if (event->source() == this) {
        //we don't support internal drops yet.
        return;
    }

    QTreeView::dropEvent(event);
}

void FileViewer::dragMoveEvent(QDragMoveEvent * event)
{

    if (event->source() == this) {
        //we don't support internal drops yet.
        return;
    }

    QTreeView::dragMoveEvent(event);
    if (event->mimeData()->hasFormat(QStringLiteral("text/uri-list"))) {
        event->acceptProposedAction();
    }
}

bool FileViewer::eventFilter(QObject *object, QEvent *event)
{
    if (object == m_entryEditor && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Escape) {
            closeEntryEditor();
            return true;
        }
    }
    return false;
}

void FileViewer::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_editorIndex.isValid()) {
        closeEntryEditor();
    } else {
        QTreeView::mouseReleaseEvent(event);
    }
}

void FileViewer::keyPressEvent(QKeyEvent *event)
{
    if (m_editorIndex.isValid()) {
        switch (event->key()) {
        case Qt::Key_Return:
        case Qt::Key_Enter: {
            QLineEdit* editor = static_cast<QLineEdit*>(indexWidget(m_editorIndex));
            emit entryChanged(editor->text());
            closeEntryEditor();
            break;
        }
        default:
            QTreeView::keyPressEvent(event);
        }
    } else {
        QTreeView::keyPressEvent(event);
    }
}

void FileViewer::renameSelectedEntry()
{
    QModelIndex currentIndex = selectionModel()->currentIndex();
    currentIndex = (currentIndex.parent().isValid())
                   ? currentIndex.parent().child(currentIndex.row(), 0)
                   : model()->index(currentIndex.row(), 0);
    openEntryEditor(currentIndex);
}

void FileViewer::openEntryEditor(const QModelIndex &index)
{
    m_editorIndex = index;
    openPersistentEditor(index);
    m_entryEditor = static_cast<QLineEdit*>(indexWidget(m_editorIndex));
    m_entryEditor->installEventFilter(this);
    m_entryEditor->setText(index.data().toString());
    m_entryEditor->setFocus(Qt::OtherFocusReason);
    m_entryEditor->selectAll();
}

void FileViewer::closeEntryEditor()
{
    m_entryEditor->removeEventFilter(this);
    closePersistentEditor(m_editorIndex);
    m_editorIndex = QModelIndex();
}
