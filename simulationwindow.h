#ifndef SIMULATIONWINDOW_H
#define SIMULATIONWINDOW_H

#include <QWidget>
#include <QImage>
#include <QIcon>
#include <QPainter>

#include "structs.h"
#include "elevatorsystem.h"

namespace Ui {
class SimulationWindow;
}

class SimulationWindow : public QWidget
{
    Q_OBJECT

public:
    explicit SimulationWindow(QWidget *parent = 0);
    ~SimulationWindow();

private slots:
    void on_startButton_clicked();
    void on_endButton_clicked();

private:
    /* 电梯主控制系统 */
    //------------------------------------------------------------------------------------
    ElevatorSystem simulationSystem;
    void addSystemTime();                                          // 递增系统时钟

    /* UI系统参数 */
    //------------------------------------------------------------------------------------
    int SystemTime;                                                 // 系统时钟
    bool buttonOption;                                              // 电梯运行控制
    int floorPassenger[10];                                         // 楼层乘客数目
    ElevatorMessage eleMessage[3];                                  // 电梯信息参数
    static const int elevatorHigh = 90;                             // 电梯IMG高度
    static const int elevatorWidth = 70;                            // 电梯IMG宽度


    /* UI界面资源 */
    //------------------------------------------------------------------------------------
    Ui::SimulationWindow *ui;                                       // 界面UI

    QIcon endIco;                                                   // 结束按钮ICO
    QIcon pauseIco;                                                 // 暂停状态ICO
    QIcon continueIco;                                              // 继续状态ICO
    QImage buildingImg;                                             // 大楼背景IMG
    QImage eleOpenImg;                                              // 开启电梯IMG
    QImage eleCloseImg;                                             // 关闭电梯IMG
    QImage eleHalfOpenImg;                                          // 半开电梯IMG
    QImage inImg;                                                   // 乘客进入IMG
    QImage outImg;                                                  // 乘客离开IMG


    /* UI控制模块 */
    //------------------------------------------------------------------------------------
    void paintEvent(QPaintEvent *event);
    void drawElevator(QPainter &painter);                           // 绘制电梯模块
    void drawPassenger(QPainter &painter);                          // 绘制乘客模块
    QRect getEleFloorRect(int eleNo, float nowFloor);               // 获取电梯绘制区域
    QRect getPassengerRect(int eleNo, float nowFloor, int timer);   // 获取乘客绘制区域
};

#endif // SIMULATIONWINDOW_H
