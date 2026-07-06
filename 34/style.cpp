#include<QString>
#include<style.h>
namespace Button_Style{
QString GAME_START_BTN =
    "QPushButton { background-color: #4CAF50; color: white; border: none; border-radius: 5px; padding: 8px 16px; }"
    "QPushButton:hover { background-color: #45a049; }";

QString CONTRIBUTOR_BTN =
    "QPushButton { background-color: #008CBA; color: white; border-radius: 5px; padding: 8px 16px; }"
    "QPushButton:hover { background-color: #007B9A; }";
QString FIGHT_BTN=    "QPushButton { background-color: #008CBA; color: white; border-radius: 5px; padding: 8px 16px; }"
                    "QPushButton:hover { background-color: #FF3F3F; }";
QString NORMALFIGHT_BTN="QPushButton { border: none;width: 40px; height: 40px; }"
                          "QPushButton { background-image:  url(:/resources/images/normal_battle.png); }";
QString NORMALFIGHT_BTN_SL="QPushButton { border: 2px solid %1; width: 40px; height: 40px; }"
                             "QPushButton { background-image:  url(:/resources/images/normal_battle.png); }";
QString SHOP_BTN="QPushButton { border: none;width: 40px; height: 40px; }"
                   "QPushButton { background-image:  url(:/resources/images/shop.png); }";
QString SHOP_BTN_SL="QPushButton { border: 2px solid %1;width: 40px; height: 40px; }"
                      "QPushButton { background-image:  url(:/resources/images/shop.png); }";
QString MASTER_BTN="QPushButton { width: 40px; height: 40px; }"
                     "QPushButton { background-image:  url(:/resources/images/master.png); }";
QString SLEEP_BTN="QPushButton { width: 40px; height: 40px; }"
                    "QPushButton { background-image:  url(:/resources/images/sleep.png); }";
QString MYSTERY_BTN="QPushButton { border: none;width: 40px; height: 40px; }"
                      "QPushButton { background-image:  url(:/resources/images/mystery.png); }";
QString MYSTERY_BTN_SL="QPushButton { border: 2px solid %1; width: 40px; height: 40px; }"
                         "QPushButton { background-image:  url(:/resources/images/mystery.png); }";
QString MONEY_BTN="QPushButton {border: 1px solid #ccc;border-radius: 5px; width:85px ; height:25px; }"
                    "QPushButton {background-image: url(:/resources/images/shop2.png);background-repeat: no-repeat;background-position: left center;}"
                    "QPushButton {text-align: right;padding-left: 30px;padding-right: 10px;font-size: 10px;";
QString REPAIR_BTN="QPushButton { border: none;width: 40px; height: 40px; }"
                     "QPushButton { background-image:  url(:/resources/images/repair.png); }";
QString REPAIR_BTN_SL="QPushButton { border: 2px solid %1; width: 40px; height: 40px; }"
                         "QPushButton { background-image:  url(:/resources/images/repair.png); }";
QString START_BTN="QPushButton { border: none;width: 40px; height: 40px; }"
                    "QPushButton { background-image:  url(:/resources/images/start.png); }";
QString START_BTN_SL="QPushButton { border: 2px solid %1; width: 40px; height: 40px; }"
                       "QPushButton { background-image:  url(:/resources/images/start.png); }";
}