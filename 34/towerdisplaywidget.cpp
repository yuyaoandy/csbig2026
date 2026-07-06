#include "towerdisplaywidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QFont>

TowerDisplayWidget::TowerDisplayWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumWidth(120);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setMouseTracking(true);
}

void TowerDisplayWidget::setInsertionEnabled(bool enabled)
{
    m_insertionEnabled = enabled;
    if (!enabled) {
        m_hoveredGapIndex = -1;
        updateLayout();
        update();
    }
}

void TowerDisplayWidget::setTowerData(const QVector<TowerBlock> &tower,
                                      const QVector<QColor> &colors,
                                      const QVector<QString> &names)
{
    m_tower = tower;
    m_colors = colors;
    m_typeNames = names;
    m_hoveredGapIndex = -1;
    updateLayout();
    update();
}



void TowerDisplayWidget::updateLayout()
{
    m_yPositions.clear();
    m_gapRects.clear();

    if (m_tower.isEmpty()) {
        m_gapRects.append(QRect(0, 0, width(), height()));
        setFixedHeight(qMax(100, height()));
        return;
    }

    int y = m_topMargin;
    int totalHeight = m_topMargin;

    for (int i = 0; i < m_tower.size(); ++i) {
        m_yPositions.append(y);
        int spacing = m_baseSpacing;
        if (m_hoveredGapIndex == i + 1) {
            spacing += m_extraSpacing;
        }
        y += m_blockHeight + spacing;
    }

    if (m_tower.size() > 0) {
        totalHeight = m_yPositions.last() + m_blockHeight;
    }
    totalHeight += m_topMargin;
    setFixedHeight(qMax(totalHeight, 100));

    int x = (width() - m_blockWidth) / 2;
    m_gapRects.append(QRect(0, 0, width(), m_topMargin + 4));

    for (int i = 0; i < m_tower.size(); ++i) {
        int blockTop = m_yPositions[i];
        int blockBottom = blockTop + m_blockHeight;
        int nextTop;
        if (i + 1 < m_tower.size()) {
            nextTop = m_yPositions[i + 1];
        } else {
            nextTop = totalHeight;
        }
        int gapTop = blockBottom;
        int gapBottom = nextTop;
        int expandedTop = qMax(0, gapTop - 4);
        int expandedBottom = qMin(totalHeight, gapBottom + 4);
        if (expandedBottom > expandedTop) {
            m_gapRects.append(QRect(0, expandedTop, width(), expandedBottom - expandedTop));
        }
    }
}

void TowerDisplayWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int x = (width() - m_blockWidth) / 2;

    for (int i = 0; i < m_tower.size(); ++i) {
        const TowerBlock &block = m_tower.at(i);
        int y = m_yPositions[i];

        // 绘制外层积木
        if (block.typeIndex >= 0 && block.typeIndex < m_colors.size()) {
            painter.setBrush(m_colors.at(block.typeIndex));
            painter.setPen(Qt::black);
            painter.drawRect(x, y, m_blockWidth, m_blockHeight);
        }

        // 绘制内层积木
        bool hasInner = (block.innerType >= 0 && block.innerType < m_colors.size());
        if (hasInner) {
            int innerW = static_cast<int>(m_blockWidth * innerWidthRatio);
            int innerH = static_cast<int>(m_blockHeight * innerHeightRatio);
            int innerX = x + (m_blockWidth - innerW) / 2;
            int innerY = y + (m_blockHeight - innerH) / 2;
            painter.setBrush(m_colors.at(block.innerType));
            painter.setPen(Qt::black);
            painter.drawRect(innerX, innerY, innerW, innerH);
        }

        // ====== 显示卡牌名称 ======
        painter.setPen(Qt::gray);
        QFont font = painter.font();
        font.setPointSize(9);          // 字号可调
        font.setBold(1);

        painter.setFont(font);

        QString text;
        if (hasInner) {
            int idx = block.innerType;
            if (idx >= 0 && idx < m_typeNames.size())
                text = m_typeNames[idx];
        } else {
            int idx = block.typeIndex;
            if (idx >= 0 && idx < m_typeNames.size())
                text = m_typeNames[idx];
        }

        if (!text.isEmpty()) {
            QRect textRect;
            if (hasInner) {
                int innerW = static_cast<int>(m_blockWidth * innerWidthRatio);
                int innerH = static_cast<int>(m_blockHeight * innerHeightRatio);
                int innerX = x + (m_blockWidth - innerW) / 2;
                int innerY = y + (m_blockHeight - innerH) / 2;
                textRect = QRect(innerX, innerY, innerW, innerH);
            } else {
                textRect = QRect(x, y, m_blockWidth, m_blockHeight);
            }
            painter.drawText(textRect, Qt::AlignCenter, text);
        }
    }

    // 绘制悬停间隙指示（不变）
    if (m_insertionEnabled && m_hoveredGapIndex >= 0 && m_hoveredGapIndex < m_gapRects.size()) {
        QRect gapRect = m_gapRects[m_hoveredGapIndex];
        painter.setBrush(QColor(0, 255, 0, 80));
        painter.setPen(Qt::green);
        painter.drawRect(gapRect);
        painter.setPen(Qt::black);
        QFont font = painter.font();
        font.setPointSize(16);
        painter.setFont(font);
        painter.drawText(gapRect, Qt::AlignCenter, "+");
    }
}

// 以下鼠标事件函数保持原样，无改动
void TowerDisplayWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (m_insertionEnabled && m_hoveredGapIndex >= 0 && m_hoveredGapIndex < m_gapRects.size()) {
            emit insertRequested(m_hoveredGapIndex);
            return;
        }

        QPoint pos = event->pos();
        for (int i = 0; i < m_tower.size(); ++i) {
            int y = m_yPositions[i];
            int x = (width() - m_blockWidth) / 2;
            QRect blockRect(x, y, m_blockWidth, m_blockHeight);
            if (blockRect.contains(pos)) {
                bool innerClicked = false;
                const TowerBlock &block = m_tower.at(i);
                if (block.innerType >= 0 && block.innerType < m_colors.size()) {
                    int innerW = static_cast<int>(m_blockWidth * innerWidthRatio);
                    int innerH = static_cast<int>(m_blockHeight * innerHeightRatio);
                    int innerX = x + (m_blockWidth - innerW) / 2;
                    int innerY = y + (m_blockHeight - innerH) / 2;
                    QRect innerRect(innerX, innerY, innerW, innerH);
                    if (innerRect.contains(pos)) {
                        innerClicked = true;
                    }
                }
                emit blockClicked(i, innerClicked);
                return;
            }
        }
    }
    QWidget::mousePressEvent(event);
}

void TowerDisplayWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_insertionEnabled) {
        if (m_hoveredGapIndex != -1) {
            m_hoveredGapIndex = -1;
            updateLayout();
            update();
        }
        QWidget::mouseMoveEvent(event);
        return;
    }

    int newGap = -1;
    QPoint pos = event->pos();
    for (int i = 0; i < m_gapRects.size(); ++i) {
        if (m_gapRects[i].contains(pos)) {
            newGap = i;
            break;
        }
    }

    if (newGap != m_hoveredGapIndex) {
        m_hoveredGapIndex = newGap;
        updateLayout();
        update();
    }
    QWidget::mouseMoveEvent(event);
}

void TowerDisplayWidget::leaveEvent(QEvent *event)
{
    if (m_hoveredGapIndex != -1) {
        m_hoveredGapIndex = -1;
        updateLayout();
        update();
    }
    QWidget::leaveEvent(event);
}

void TowerDisplayWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateLayout();
    update();
}