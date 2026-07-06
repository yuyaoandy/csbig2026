#ifndef CARDSELECTIONWIDGET_H
#define CARDSELECTIONWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

class CardSelectionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CardSelectionWidget(QWidget *parent = nullptr);
    ~CardSelectionWidget();
    int gain;
    QString getCurrentSelectedCardType() const { return m_currentSelectedType; }

    void clearSelection();
    void setupUI(int id=0,int mn=0);
    int id;
signals:
    void selectionConfirmed(const QString &cardType,int id,int gain);

private slots:
    void onCardButtonClicked();
    void onConfirmClicked();

private:
    void cleanupChildren();
    void updateCardStyle(QPushButton *btn, bool selected);
    QString getCardTypeForButton(QPushButton *btn) const;

    QPushButton *m_cardButtons[3];
    QPushButton *m_confirmButton;
    QPushButton *m_currentSelectedButton;   // 当前高亮的按钮
    QString m_currentSelectedType;          // 当前选中的卡片类型（实时）
};

#endif // CARDSELECTIONWIDGET_H