#ifndef FILEVIWER_H
#define FILEVIWER_H


#include <QTreeView>

class QLineEdit;

class FileViewer : public QTreeView
{
    Q_OBJECT

public:
    explicit FileViewer(QWidget *parent = nullptr);
    void dragEnterEvent(class QDragEnterEvent * event) override;
    void dropEvent(class QDropEvent * event) override;
    void dragMoveEvent(class QDragMoveEvent * event) override;
    void startDrag(Qt::DropActions supportedActions) override;

    /**
     * Expand the first level in the view if there is only one root folder.
     * Typical use case: an archive with source code.
     */
    void expandIfSingleFolder();

    /**
     * Set whether the view should accept drop events.
     */
    void setDropsEnabled(bool enabled);

public Q_SLOTS:
    void renameSelectedEntry();

protected:
    bool eventFilter(QObject *object, QEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

Q_SIGNALS:
    void entryChanged(const QString &name);

private:
    void openEntryEditor(const QModelIndex &index);
    void closeEntryEditor();
    QModelIndex m_editorIndex;
    QLineEdit *m_entryEditor = nullptr;
};


#endif // FILEVIWER_H
