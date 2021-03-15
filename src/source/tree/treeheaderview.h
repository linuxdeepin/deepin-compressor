/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     gaoxiang <gaoxiang@uniontech.com>
*
* Maintainer: gaoxiang <gaoxiang@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef TREEHEADERVIEW_H
#define TREEHEADERVIEW_H

#include <DLabel>
#include <DHeaderView>

DWIDGET_USE_NAMESPACE

#define SCROLLMARGIN 10

class TreeHeaderView;
// 返回上一级
class PreviousLabel: public Dtk::Widget::DLabel
{
    Q_OBJECT
public:
    explicit PreviousLabel(TreeHeaderView *parent = nullptr);
    ~ PreviousLabel() override;

    /**
     * @brief setPrePath    设置上一级路径
     * @param strPath   上一级路径
     */
    void setPrePath(const QString &strPath);

protected:
    void paintEvent(QPaintEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void focusInEvent(QFocusEvent *event) Q_DECL_OVERRIDE;
    void focusOutEvent(QFocusEvent *event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;

signals:
    void doubleClickedSignal();

private:
    TreeHeaderView *headerView_;

    bool focusIn_ = false;
};

// 表头
class TreeHeaderView : public DHeaderView
{
    Q_OBJECT
public:
    explicit TreeHeaderView(Qt::Orientation orientation, QWidget *parent = nullptr);
    ~TreeHeaderView() override;

    QSize sizeHint() const override;
    int sectionSizeHint(int logicalIndex) const;

    inline int getSpacing() const { return m_spacing; }
    inline void setSpacing(int spacing) { m_spacing = spacing; }

    /**
     * @brief getpreLbl    获取上一级指针
     * @return
     */
    PreviousLabel *getpreLbl();

//    /**
//     * @brief setPrePath    设置上一级路径
//     * @param strPath   上一级路径
//     */
//    void setPrePath(const QString &strPath);


    /**
     * @brief setPreLblVisible  设置上一级选项是否可见
     * @param bVisible  是否可见标志位
     */
    void setPreLblVisible(bool bVisible);

protected:
    //void paintEvent(QPaintEvent *e) override;
    virtual void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const override;

    void resizeEvent(QResizeEvent *event) override;

private:
    int m_spacing {1};

    PreviousLabel *m_pPreLbl;
};

#endif // TREEHEADERVIEW_H
