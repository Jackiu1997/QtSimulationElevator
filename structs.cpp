#include "structs.h"

PassengerList::PassengerList() {
    peoNo = 0;
    requestTime = 0;
    srcFloor = 0;
    tarFloor = 0;
    runStatus = 0;
    next = nullptr;
}

PassengerList::PassengerList(int peono, int request, int src, int tar) {
    peoNo = peono;
    requestTime = request;
    srcFloor = src;
    tarFloor = tar;
    if (tarFloor > srcFloor) runStatus = 1;
    else runStatus = -1;
    next = nullptr;
}


Elevator::Elevator() {
    serviceFloors[0] = 1;
    serviceFloors[1] = 1;
    nowFloor = 1;
    nowLoad = 0;

    runStatus = { 1, 0 };
    doorStatus = { 0, 0 };
    elevatorStatus = { -1, 0 };

    inElePeople = new PassengerList;
    targetFloor = new TargetFloorList(0);
}

Elevator::Elevator(int down, int up) {
    serviceFloors[0] = down;
    serviceFloors[1] = up;
    nowFloor = 1;
    nowLoad = 0;

    runStatus = { 1, 0 };
    doorStatus = { 0, 0 };
    elevatorStatus = { -1, 0 };

    inElePeople = new PassengerList;
    targetFloor = new TargetFloorList(0);
}

ElevatorMessage::ElevatorMessage() {
    nowFloor = 1;
    nowLoad = 0;

    runStatus = {0, 0};
    doorStatus = {0, 0};
    elevatorStatus = {-2, 0};
}

ElevatorMessage::ElevatorMessage(Elevator ele) {
    nowFloor = ele.nowFloor;
    nowLoad = ele.nowLoad;

    runStatus = ele.runStatus;
    doorStatus = ele.doorStatus;
    elevatorStatus = ele.elevatorStatus;
}
