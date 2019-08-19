
#ifndef DFMHEADERVIEW_H
#define DFMHEADERVIEW_H

#include <QHeaderView>

class DFMHeaderView : public QHeaderView
{
    Q_OBJECT
public:
    explicit DFMHeaderView(Qt::Orientation orientation, QWidget *parent = nullptr);

    QSize sizeHint() const;

    using QHeaderView::updateGeometries;

protected:
    void mouseReleaseEvent(QMouseEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;

signals:
    void mouseReleased();
    void viewResized();
};

#endif // DFMHEADERVIEW_H
