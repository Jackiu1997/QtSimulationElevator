#include "simulationwindow.h"
#include "ui_simulationwindow.h"
#include <windows.h>

SimulationWindow::SimulationWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SimulationWindow)
{
    /* 加载贴图 */
    endIco.addFile(":/figure/res/end.png");
    pauseIco.addFile(":/figure/res/pause.png");
    continueIco.addFile(":/figure/res/continue.png");

    buildingImg.load(":/figure/res/building.png");
    eleOpenImg.load(":/figure/res/open.png");
    eleCloseImg.load(":/figure/res/close.png");
    eleHalfOpenImg.load(":/figure/res/halfopen.png");
    inImg.load(":/figure/res/passengerin.png");
    outImg.load(":/figure/res/passengerout.png");

    /* ui初始化 */
    ui->setupUi(this);
    this->setMinimumSize(700, 920);
    this->setMaximumSize(700, 920);
    this->setWindowTitle("Simulation Elevator");
    this->setWindowIcon(QIcon(":/figure/res/icon.png"));
    this->setAttribute(Qt::WA_StyledBackground,true);
    this->setStyleSheet("background-color: rgb(255, 255, 255)");

    ui->startButton->setIcon(continueIco);
    ui->endButton->setIcon(endIco);

    /* 初始化系统参数 */
    SystemTime = 0;
    buttonOption = false;
    updateMessage();
}

/* 单步运行电梯系统 */
void SimulationWindow::addSystemTime()
{
    simulationSystem.stepRunElevator();

    /* 更新UI参数 */
    updateMessage();

    /* 系统时钟自增 */
    Sleep(100);
    SystemTime++;

    /* 视图更新 */
    update();
}

SimulationWindow::~SimulationWindow()
{
    delete ui;
}

/* 更新UI参数 */
void SimulationWindow::updateMessage() {
    /* 更新电梯信息参数 */
    for (int eleNo = 1; eleNo <= 3; eleNo++) {
        eleMessage[eleNo - 1] = simulationSystem.getElevatorMessage(eleNo);
    }
    /* 更新楼层乘客参数 */
    for (int floorNo = 1; floorNo <= 10; floorNo++) {
        floorPassenger[floorNo - 1] = simulationSystem.getFloorPassengerNum(floorNo);
    }
}

void SimulationWindow::paintEvent(QPaintEvent *event) {
    QPainter painter(this);

    ui->lcdClock->display(SystemTime);
    /* 更新电梯状态lcd参数显示 */
    ui->lcdFloor1->display(eleMessage[0].nowFloor);
    ui->lcdFloor2->display(eleMessage[1].nowFloor);
    ui->lcdFloor3->display(eleMessage[2].nowFloor);
    ui->lcdLoad1->display(eleMessage[0].nowLoad);
    ui->lcdLoad2->display(eleMessage[1].nowLoad);
    ui->lcdLoad3->display(eleMessage[2].nowLoad);
    /* 更新楼层乘客lcd参数显示 */
    ui->lcdPassenger1->display(floorPassenger[0]);
    ui->lcdPassenger2->display(floorPassenger[1]);
    ui->lcdPassenger3->display(floorPassenger[2]);
    ui->lcdPassenger4->display(floorPassenger[3]);
    ui->lcdPassenger5->display(floorPassenger[4]);
    ui->lcdPassenger6->display(floorPassenger[5]);
    ui->lcdPassenger7->display(floorPassenger[6]);
    ui->lcdPassenger8->display(floorPassenger[7]);
    ui->lcdPassenger9->display(floorPassenger[8]);
    ui->lcdPassenger10->display(floorPassenger[9]);

    /* draw building frame */
    painter.drawImage(0, 0, buildingImg);

    /* draw elevators */
    drawElevator(painter);

    /* draw passengers */
    drawPassenger(painter);

    if (buttonOption) {
        addSystemTime();
    }
}

void SimulationWindow::drawElevator(QPainter &painter)
{
    for (int eleNo = 0; eleNo < 3; eleNo++) {
        ElevatorMessage message = eleMessage[eleNo];

        QRect elevatorRect = getEleFloorRect(eleNo + 1, message.nowFloor);

        QImage eleImg;
        /* 关门状态 */
        if (message.doorStatus.status == 0) {
            /* 半开状态 */
            if (message.elevatorStatus.status == 2 && message.elevatorStatus.timer == 1) {
                eleImg = eleHalfOpenImg;
            }
            else if (message.elevatorStatus.status == 2 && message.elevatorStatus.timer == 2) {
                eleImg = eleOpenImg;
            }
            else eleImg = eleCloseImg;
        }
        /* 开门状态 */
        else if (message.doorStatus.status == 1) {
            /* 半开状态 */
            if (message.elevatorStatus.status == 3 && message.elevatorStatus.timer == 1) {
                eleImg = eleHalfOpenImg;
            }
            else if (message.elevatorStatus.status == 3 && message.elevatorStatus.timer == 2) {
                eleImg = eleCloseImg;
            }
            else eleImg = eleOpenImg;
        }

        painter.drawImage(elevatorRect, eleImg);
    }
}

void SimulationWindow::drawPassenger(QPainter &painter) {
    for (int eleNo = 0; eleNo < 3; eleNo++) {
        ElevatorMessage message = eleMessage[eleNo];
        int nowFloor = message.nowFloor;
        int timer = message.elevatorStatus.timer;

        /* 电梯进人 */
        if (message.elevatorStatus.status == 4) {
            QRect passengerRect = getPassengerRect(eleNo + 1, nowFloor, 3 - timer);
            painter.drawImage(passengerRect, inImg);
        }
        /* 电梯出人 */
        else if (message.elevatorStatus.status == 5) {
            QRect passengerRect = getPassengerRect(eleNo + 1, nowFloor, timer - 1);
            painter.drawImage(passengerRect, outImg);
        }
    }
}

QRect SimulationWindow::getEleFloorRect(int eleNo, float nowFloor) {
    return QRect(elevatorWidth * (eleNo), elevatorHigh * (10 - nowFloor), elevatorWidth - 5, elevatorHigh - 5);
}

QRect SimulationWindow::getPassengerRect(int eleNo, float nowFloor, int timer) {
    int top = 0, left = 0;
    QRect eleRect = getEleFloorRect(eleNo, nowFloor);

    left = eleRect.left() + 10;
    top = eleRect.top() + 30;

    switch (timer) {
    case 0: return QRect(left, top, 50, 50);
    case 1: return QRect(4*elevatorWidth + 10, top, 50, 50);
    case 2: return QRect(4*elevatorWidth + 50, top, 50, 50);
    default: return QRect(left, top, 50, 50);
    }
}

void SimulationWindow::on_startButton_clicked()
{
    buttonOption = !buttonOption;
    if (buttonOption) {
        ui->startButton->setIcon(pauseIco);
    }
    else {
        ui->startButton->setIcon(continueIco);
    }
    update();
}

void SimulationWindow::on_endButton_clicked()
{
    this->close();
}
