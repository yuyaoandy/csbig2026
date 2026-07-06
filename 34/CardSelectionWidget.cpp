#include "CardSelectionWidget.h"
#include <QSpacerItem>
#include <iostream>
#include "QPushButton"
#include "card.h"
#include "random"
#include "AutoWrapHelper.h"

CardSelectionWidget::CardSelectionWidget(QWidget *parent)
    : QWidget(parent)
    , m_currentSelectedButton(nullptr)
{
    setupUI();
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}

CardSelectionWidget::~CardSelectionWidget()
{
}
extern std::mt19937 rnd;
extern QVector<Card> stored_card;
void CardSelectionWidget::cleanupChildren()
{
    QList<QPushButton*> btns = findChildren<QPushButton*>();
    for (QPushButton *btn : btns) {
        btn->deleteLater();
    }
    for (int i = 0; i < 3; ++i) {
        m_cardButtons[i] = nullptr;
    }
    m_confirmButton = nullptr;
    m_currentSelectedButton = nullptr;
    m_currentSelectedType.clear();
    if (this->layout()) {
        QLayoutItem *item;
        while ((item = this->layout()->takeAt(0)) != nullptr) {
            delete item;
        }
        delete this->layout();
    }
}
void CardSelectionWidget::setupUI(int id,int mn)
{
    cleanupChildren();
    gain=mn;
    std::cerr<<"????"<<std::endl;
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    std::cerr<<"????"<<std::endl;
    mainLayout->setContentsMargins(20, 20, 20, 20);
    this->id=id;
    QHBoxLayout *cardsLayout = new QHBoxLayout;
    cardsLayout->setSpacing(20);
    struct CardInfo {
        QString type;
        QString displayText;
        QString icon;
        QString card_type;
    } infos[3];
    for (int i=0;i<3;++i){
        int z=rnd()%stored_card.size();
        int level=rnd()%(stored_card[z].max_level+1);
        infos[i].card_type=stored_card[z].name+QString("%1").arg(level);
        infos[i].type=stored_card[z].chinese+QString("%1").arg(level);
        infos[i].displayText=stored_card[z].description;
    }
    for (int i = 0; i < 3; ++i) {
     //   std::cerr<<"set"<<i<<std::endl;
        QPushButton *btn = new QPushButton(this);
        btn->setFixedSize(150, 190);
        btn->setProperty("cardType", infos[i].card_type);
        btn->setText(QString("%1\n\n\n\n%2").arg(infos[i].type, infos[i].displayText));
        AutoWrapHelper::install(btn, 10);
        btn->setCursor(Qt::PointingHandCursor);
        //btn->setWordWrap(true);
        connect(btn, &QPushButton::clicked, this, &CardSelectionWidget::onCardButtonClicked);

        m_cardButtons[i] = btn;
        cardsLayout->addWidget(btn);
        updateCardStyle(btn, false);
    }
    //std::cerr<<"the children count"<<cardsLayout->count()<<std::endl;
    cardsLayout->insertStretch(0, 1);
    cardsLayout->addStretch(1);
    mainLayout->addLayout(cardsLayout);

    // 弹簧将确认按钮推到底部居中
    mainLayout->addStretch();

    // 确认按钮
    m_confirmButton = new QPushButton("确认选择", this);
    m_confirmButton->setFixedWidth(120);
    connect(m_confirmButton, &QPushButton::clicked, this, &CardSelectionWidget::onConfirmClicked);

    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->addStretch();
    btnLayout->addWidget(m_confirmButton);
    btnLayout->addStretch();
    mainLayout->addLayout(btnLayout);
    mainLayout->addSpacing(20);
    //std::cerr<<"???"<<std::endl;
}

void CardSelectionWidget::updateCardStyle(QPushButton *btn, bool selected)
{
    if (!btn) return;

    QString cardType = btn->property("cardType").toString();
    QString bgColor="#E8F5E9";
    if(cardType.size()){
        //cerr<<cardType.toStdString()<<endl;
        if (cardType[0] == "S") {
    //    cerr<<"!!!"<<endl;
            bgColor = "#E3F2FD";
        } else if (cardType[0] == "A") {
            bgColor = "#FFF3E0";
        }
    }
    QString baseStyle = QString(
                            "background-color: %1;"
                            "border: 2px solid transparent;"
                            "border-radius: 12px;"
                            "font-size: 10px;"
                            "font-weight: bold;"
                            "color: #2C3E50;"
                            "padding: 10px;"
                            "text-align: center;"
                            ).arg(bgColor);

    if (selected) {
        QString selectedStyle = baseStyle +
                                "border: 2px solid #1976D2;"
                                            "background-color: %1;";
        selectedStyle = selectedStyle.arg(bgColor);
        btn->setStyleSheet(selectedStyle);
    } else {
        btn->setStyleSheet(baseStyle);
    }
}

void CardSelectionWidget::onCardButtonClicked()
{
    QPushButton *clickedBtn = qobject_cast<QPushButton*>(sender());
    if (!clickedBtn) return;
    std::cerr<<"clocked"<<std::endl;
    if (m_currentSelectedButton == clickedBtn) {
        // 取消选中
        updateCardStyle(clickedBtn, false);
        m_currentSelectedButton = nullptr;
        m_currentSelectedType.clear();
    } else {
        // 取消之前选中
        if (m_currentSelectedButton) {
            updateCardStyle(m_currentSelectedButton, false);
        }
        updateCardStyle(clickedBtn, true);
        m_currentSelectedButton = clickedBtn;
        m_currentSelectedType = clickedBtn->property("cardType").toString();
    }
}

void CardSelectionWidget::onConfirmClicked()
{
    //std::cerr<<"confirm"<<std::endl;
    emit selectionConfirmed(m_currentSelectedType,id,gain);
    //std::cerr<<"returned\n";
}

void CardSelectionWidget::clearSelection()
{
    if (m_currentSelectedButton) {
        updateCardStyle(m_currentSelectedButton, false);
        m_currentSelectedButton = nullptr;
    }
    m_currentSelectedType.clear();
}

QString CardSelectionWidget::getCardTypeForButton(QPushButton *btn) const
{
    return btn ? btn->property("cardType").toString() : QString();
}