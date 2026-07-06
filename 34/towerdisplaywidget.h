#ifndef TOWERDISPLAYWIDGET_H
#define TOWERDISPLAYWIDGET_H

#include <QWidget>
#include <QVector>
#include <QColor>

struct TowerBlock {
    int typeIndex = -1;
    int innerType = -1;
};

class TowerDisplayWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TowerDisplayWidget(QWidget *parent = nullptr);

    QVector<TowerBlock> towerData() const { return m_tower; }

    void setInsertionEnabled(bool enabled);
    // 增加 names 参数
    void setTowerData(const QVector<TowerBlock>& blocks,
                      const QVector<QColor>& colors,
                      const QVector<QString>& names);


signals:
    void blockClicked(int index, bool innerClicked);
    void insertRequested(int insertIndex);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
private:
    QVector<QString> m_typeNames;  // 存储每种卡牌的名称（中文）
    void updateLayout();   // 根据悬停状态计算积木Y位置和空隙矩形

    QVector<TowerBlock> m_tower;
    QVector<QColor> m_colors;

    int m_blockWidth = 80;
    int m_blockHeight = 30;
    int m_baseSpacing = 4;          // 基础间距
    int m_extraSpacing = 12;        // 悬停时额外增加的间距

    int m_topMargin = 10;

    double innerWidthRatio = 0.6;
    double innerHeightRatio = 0.5;

    bool m_insertionEnabled = true;
    int m_hoveredGapIndex = -1;          // -1 表示无悬停

    QVector<int> m_yPositions;           // 每个积木的顶部 Y（大小 = m_tower.size()）
    QVector<QRect> m_gapRects;           // 所有空隙的矩形（已扩展点击区域）
};

#endif // TOWERDISPLAYWIDGET_H