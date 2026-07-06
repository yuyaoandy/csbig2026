#include "storywidget.h"

StoryNodeWidget::StoryNodeWidget(const StoryNode &node, QWidget *parent)
    : QWidget(parent) {
    setStyleSheet("background-color: #2b2b2b;");
    //setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setFixedSize(800,600);
    //storyContainer->setSizePolicy(QSizePoli
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);
    mainLayout->setAlignment(Qt::AlignHCenter);

    // ---- 上方文本 ----
    QLabel *textLabel = new QLabel(node.text, this);
    textLabel->setWordWrap(true);
    textLabel->setAlignment(Qt::AlignCenter);          // 文字水平居中
    textLabel->setStyleSheet("color: #f0f0f0; font-size: 16px; background: transparent;");
    textLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainLayout->addWidget(textLabel, 1);

    // ---- 下方按钮区域 ----
    QWidget *btnContainer = new QWidget(this);
    QVBoxLayout *btnLayout = new QVBoxLayout(btnContainer);
    btnLayout->setSpacing(10);
    btnLayout->setAlignment(Qt::AlignHCenter);         // 按钮在容器内水平居中

    for (const auto &choice : node.choices) {
        QPushButton *btn = new QPushButton(choice.text, this);
        btn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        btn->setStyleSheet(
            "QPushButton {"
            "   color: #f0f0f0;"
            "   background-color: #3a3a3a;"
            "   border: 1px solid #5a5a5a;"
            "   border-radius: 6px;"
            "   padding: 12px 8px;"
            "   font-size: 14px;"
            "   text-align: center;"                // 文本居中
            "}"
            "QPushButton:hover { background-color: #4a4a4a; }"
            "QPushButton:pressed { background-color: #2a2a2a; }"
            );

        if (choice.target == -1) {
            connect(btn, &QPushButton::clicked, [this, choice]() {
                emit effectsTriggered(choice.effects);   // 先执行效果
                emit terminate();
            });
        } else {
            connect(btn, &QPushButton::clicked, [this, choice]() {
                emit effectsTriggered(choice.effects);
                emit nodeSelected(choice.target);
            });
        }
        btnLayout->addWidget(btn);

        btnLayout->addWidget(btn);
    }

    btnLayout->addStretch();   // 将按钮推到上方（如果不想要多余空白可去掉）
    mainLayout->addWidget(btnContainer, 0);
}