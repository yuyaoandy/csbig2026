#ifndef AUTOWRAPBUTTON_H
#define AUTOWRAPBUTTON_H

#include <QPushButton>

class AutoWrapButton : public QPushButton
{
    Q_OBJECT
public:
    explicit AutoWrapButton(const QString &text = "", QWidget *parent = nullptr);

    // 重写 setText，保存原始文本并触发自动换行
    void setText(const QString &text) ;

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    QString m_originalText;      // 原始文本（不含换行符）
    int     m_paddingH = 10;     // 左右内边距（像素），可根据按钮样式调整

    void applyWordWrap();        // 核心换行逻辑
    int  calculateHeight(const QStringList &lines) const;
};

#endif // AUTOWRAPBUTTON_H