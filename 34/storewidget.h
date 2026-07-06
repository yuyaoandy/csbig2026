// storewidget.h
#ifndef STOREWIDGET_H
#define STOREWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QList>
#include <QScrollArea>
#include <QGridLayout>

struct StoreItem {
    int id;
    QString name;
    QString desc;
    QString truename;
    int price;
    QString iconChar;
    int rarity;          // 1~5，1最低
};

class StoreWidget : public QWidget
{
    Q_OBJECT
public:
    explicit StoreWidget(QWidget *parent = nullptr, int pid = 0);
    ~StoreWidget();
    int id;
    void rebuild();

signals:
    void goback(int id = 0);
    void cardBought(const QString &cardName, int price);

private slots:
    void purchaseItemById(int itemId);
    void onClose();

private:
    void initUI();
    void initDefaultItems();
    void rebuildItemsView();
    void updateMoneyDisplay();
    void showTemporaryMessage(const QString &msg, bool isError = false);
    QString getRarityColor(int rarity) const;  // 新增：根据稀有度返回背景色
    QLabel *moneyLabel;
    QLabel *messageLabel;
    QWidget *itemsContainer;
    QGridLayout *itemsLayout;
    QScrollArea *scrollArea;
    QList<StoreItem> availableItems;
    QList<QWidget*> itemWidgets;
    int nextItemId;
protected:
    void showEvent(QShowEvent *event) override;
};

#endif // STOREWIDGET_H