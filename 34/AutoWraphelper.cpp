#include "AutoWrapHelper.h"
#include <QFontMetrics>
#include <QResizeEvent>
#include <QDebug>

static QHash<QPushButton*, AutoWrapHelper*> g_helpers;

AutoWrapHelper::AutoWrapHelper(QPushButton *button, int paddingH, QObject *parent)
    : QObject(parent)
    , m_button(button)
    , m_paddingH(paddingH)
{
    m_originalText = button->text();
    button->installEventFilter(this);
    applyWordWrap();
}

void AutoWrapHelper::install(QPushButton *button, int paddingH)
{
    if (g_helpers.contains(button))
        return; // 已经安装过
    AutoWrapHelper *helper = new AutoWrapHelper(button, paddingH, button);
    g_helpers[button] = helper;
}

void AutoWrapHelper::setText(QPushButton *button, const QString &text)
{
    if (AutoWrapHelper *helper = g_helpers.value(button)) {
        helper->m_originalText = text;
        helper->applyWordWrap();
    } else {
        // 如果未安装自动换行，则直接设置文本
        button->setText(text);
    }
}

bool AutoWrapHelper::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_button && event->type() == QEvent::Resize) {
        applyWordWrap();
    }
    return QObject::eventFilter(watched, event);
}

void AutoWrapHelper::applyWordWrap()
{
    if (m_originalText.isEmpty()) {
        m_button->setText("");
        return;
    }

    int availableWidth = m_button->width() - m_paddingH * 2;
    if (availableWidth <= 0)
        return;

    QFontMetrics fm(m_button->font());
    QStringList lines;
    QString currentLine;
    int currentWidth = 0;

    for (const QChar &ch : m_originalText) {
        int charWidth = fm.horizontalAdvance(ch);  // Qt5.11+，旧版本用 fm.width(ch)
        if (currentWidth + charWidth > availableWidth && !currentLine.isEmpty()) {
            lines.append(currentLine);
            currentLine = ch;
            currentWidth = charWidth;
        } else {
            currentLine.append(ch);
            currentWidth += charWidth;
        }
    }
    if (!currentLine.isEmpty())
        lines.append(currentLine);

    QString wrappedText = lines.join("\n");
    m_button->setText(wrappedText);

    // 自动调整按钮高度（可选，如果不需要可以注释）
    int totalHeight = calculateHeight(lines);
    m_button->setMinimumHeight(totalHeight + m_paddingH * 2);
}

int AutoWrapHelper::calculateHeight(const QStringList &lines) const
{
    if (lines.isEmpty())
        return m_button->fontMetrics().lineSpacing();
    return m_button->fontMetrics().lineSpacing() * lines.size();
}