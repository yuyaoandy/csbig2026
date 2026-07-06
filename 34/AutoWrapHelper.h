#ifndef AUTOWRAPHELPER_H
#define AUTOWRAPHELPER_H

#include <QObject>
#include <QPushButton>
#include <QHash>

class AutoWrapHelper : public QObject
{
    Q_OBJECT
public:
    // 为指定的按钮启用自动换行功能
    static void install(QPushButton *button, int paddingH = 10);

    // 更新按钮的文本（必须通过此函数，才能保留原始文本）
    static void setText(QPushButton *button, const QString &text);

private:
    explicit AutoWrapHelper(QPushButton *button, int paddingH, QObject *parent = nullptr);
    bool eventFilter(QObject *watched, QEvent *event) override;

    void applyWordWrap();
    int  calculateHeight(const QStringList &lines) const;

    QPushButton *m_button;
    QString      m_originalText;
    int          m_paddingH;
};

#endif // AUTOWRAPHELPER_H
