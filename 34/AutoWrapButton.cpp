#include "AutoWrapButton.h"
#include <QFontMetrics>
#include <QResizeEvent>
#include <QDebug>

AutoWrapButton::AutoWrapButton(const QString &text, QWidget *parent)
    : QPushButton(parent)
{
    setText(text);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    installEventFilter(this);   // 监听 resize 事件
}

void AutoWrapButton::setText(const QString &text)
{
    m_originalText = text;
    applyWordWrap();
}

void AutoWrapButton::applyWordWrap()
{
    if (m_originalText.isEmpty()) {
        QPushButton::setText("");
        return;
    }

    // 1. 计算可用宽度（按钮宽度 - 左右内边距）
    int availableWidth = width() - m_paddingH * 2;
    if (availableWidth <= 0) {
        return;
    }

    QFontMetrics fm(font());
    QStringList lines;
    QString currentLine;
    int currentWidth = 0;

    // 2. 逐字符遍历（支持中英文混合）
    for (const QChar &ch : m_originalText) {
        int charWidth = fm.horizontalAdvance(ch);  // Qt5.11+ 推荐
        // 如果当前行加上这个字符会超过可用宽度，且当前行非空
        if (currentWidth + charWidth > availableWidth && !currentLine.isEmpty()) {
            lines.append(currentLine);      // 保存当前行
            currentLine = ch;               // 新行从当前字符开始
            currentWidth = charWidth;
        } else {
            currentLine.append(ch);
            currentWidth += charWidth;
        }
    }
    if (!currentLine.isEmpty()) {
        lines.append(currentLine);
    }

    // 3. 用换行符拼接后设置文本
    QString wrappedText = lines.join("\n");
    QPushButton::setText(wrappedText);

    // 4. 自动调整按钮高度，确保所有行都能显示
    int totalHeight = calculateHeight(lines);
    setMinimumHeight(totalHeight + m_paddingH * 2);
}

int AutoWrapButton::calculateHeight(const QStringList &lines) const
{
    if (lines.isEmpty())
        return fontMetrics().lineSpacing();
    return fontMetrics().lineSpacing() * lines.size();
}

bool AutoWrapButton::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == this && event->type() == QEvent::Resize) {
        applyWordWrap();    // 按钮大小改变时重新换行
    }
    return QPushButton::eventFilter(obj, event);
}