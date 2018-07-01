#include "elevatorSystem.h"
#include <QFile>
#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QDebug>


/* 电梯系统初始化 */
ElevatorSystem::ElevatorSystem()
{
    /* 初始化系统时间 */
    SystemTime = 0;
    /* 初始化电梯 */
    for (int i = 0; i < 3; i++) {
        elevators[i] = Elevator(eleDistribution[i], eleDistribution[i + 1]);
    }
    /* 初始化楼层乘客链表 */
    for (int i = 0; i < 10; i++) {
        outFloorPeople[i] = new PassengerList();
    }
    /* 初始化楼层乘客数据 */
    readPassengers();

    /* 初始化输出数据 */
    QFile file("./SystemRunStatus.txt");
    if(!file.open(QIODevice::WriteOnly)){
        qDebug() << "Failed opening result data！" << endl;
        file.close();
        return;
    }
    file.write("");
    file.close();
}

/* UI单步运行电梯系统 */
void ElevatorSystem::stepRunElevator()
{
    /* 读取内外乘客请求 */
    readInOutRequest();

    /* 电梯运行逻辑系统 */
    for (int eleNo = 0; eleNo < 3; eleNo++) {

        /* 检测电梯运行状态是否改变 */
        changeElevatorStatus(eleNo);

        /* 判断电梯操作状态 （运行，开门，关门，进人，出人）*/
        switch (elevators[eleNo].elevatorStatus.status) {
        case 0: checkOpenDoor(eleNo); break; // 检测开门
        case 1: runElevator(eleNo);	break;   // 运行中
        case 2: openDoor(eleNo); break;		 // 开门中
        case 3: closeDoor(eleNo); break;	 // 关门中
        case 4: enterElevator(eleNo); break; // 进人中
        case 5: leaveElevator(eleNo); break; // 出人中
        case -1: checkOverWait(eleNo); break;// 检测超时
        case -2: break;                      // 就绪等待
        }
        /* 开门状态 非关门中 检测关门 */
        if (elevators[eleNo].doorStatus.status && elevators[eleNo].elevatorStatus.status != 3) {
            checkCloseDoor(eleNo);
        }
    }

    /* 系统时钟自增 */
    SystemTime++;
}


/* UI获取电梯运行状态 */
ElevatorMessage ElevatorSystem::getElevatorMessage(int eleNo)
{
    ElevatorMessage message(elevators[eleNo - 1]);
    return message;
}

/* UI获取楼层乘客人数 */
int ElevatorSystem::getFloorPassengerNum(int floorNo) {
    int count = 0;
    for (PassengerList *cur = outFloorPeople[floorNo - 1]; cur->next != nullptr; cur = cur->next) {
        if (cur->next->requestTime > SystemTime) {
            return count;
        }
        count++;
    }
    return count;
}


/* 乘客逻辑模块 */
//------------------------------------------------------------------------------------
/* 添加电梯乘客 */
void ElevatorSystem::addElevatorPassenger(int eleNo, int no, int time, int src, int tar) 
{
	/* 加入的乘客时间变为系统时间 */

	PassengerList *cur = elevators[eleNo].inElePeople;
	for (; cur->next != nullptr; cur = cur->next) {
		/* 乘客按时间顺序插入 */
		if (cur->next->requestTime > time) {
			auto *newNode = new PassengerList(no, SystemTime, src, tar);
			newNode->next = cur->next;
			cur->next = newNode;
			return;
		}
	}
	/* 插入楼层乘客尾部 */
	cur->next = new PassengerList(no, SystemTime, src, tar);
}

/* 添加楼层乘客 */
void ElevatorSystem::addFloorPassenger(int no, int time, int src, int tar) 
{
	PassengerList *cur = outFloorPeople[src - 1];
	for (; cur->next != nullptr; cur = cur->next) {
		/* 乘客存在，不加入 */
		if (cur->next->peoNo == no) {
			return;
		}
		/* 乘客按时间顺序插入 */
		else if (cur->next->requestTime > time) {
			auto *newNode = new PassengerList(no, time, src, tar);
			newNode->next = cur->next;
			cur->next = newNode;
			return;
		}
	}
	/* 插入楼层乘客尾部 */
	cur->next = new PassengerList(no, time, src, tar);
}

/* 读入乘客数据 */
void ElevatorSystem::readPassengers()
{
    int no, requestTime, srcFloor, tarFloor;
    QFile file("./passengers.dat");
    if(!file.open(QIODevice::ReadOnly)){
        qDebug() << "Failed reading passenger data!" << endl;
        file.close();
        return;
    }
    else {
        qDebug() << "Succeed reading passenger data!" << endl;
    }

    QTextStream *out = new QTextStream(&file);
    QStringList tempOption = out->readAll().split("\n");
    for(int i = 0 ; i < tempOption.count() -1; i++)
    {
        QStringList dataList = tempOption.at(i).split(" ");
        no = dataList.at(0).toInt();
        requestTime = dataList.at(1).toInt();
        srcFloor = dataList.at(2).toInt();
        tarFloor = dataList.at(3).toInt();

        addFloorPassenger(no, requestTime, srcFloor, tarFloor);
    }
    file.close();
}

/* 读取乘客请求,检测超出容忍时间乘客 */
void ElevatorSystem::readInOutRequest()
{
    /* 获取楼层中乘客请求 */
    for (int floorNo = 0; floorNo < 10; floorNo++) {
        for (PassengerList *cur = outFloorPeople[floorNo]; cur->next != nullptr; cur = cur->next) {
            if (cur->next->requestTime <= SystemTime) {
                if (SystemTime - cur->next->requestTime < maxTolerateTime) {
					getOutFloorRequest(cur->next->srcFloor, cur->next->tarFloor, cur->next->runStatus);
				}
				/* 超过最大容忍时间的乘客离开 */
				else {
					outputPeopleMessage(floorNo, cur->next->peoNo);
					cur->next = cur->next->next;
					if (cur->next == nullptr) {
						break;
					}
				}
            }
            else break;
        }
    }
    /* 获取电梯中乘客请求 */
    for (int eleNo = 0; eleNo < 3; eleNo++) {
        for (PassengerList *cur = elevators[eleNo].inElePeople; cur->next != nullptr; cur = cur->next) {
            getInElevatorRequest(eleNo, cur->next->tarFloor);
        }
    }
}

/* 响应楼层外部请求 */
void ElevatorSystem::getOutFloorRequest(int srcFloor, int tarFloor, int runStatus)
{
    int eleNo = 0, tag = 0;
    for (; eleNo < 3; eleNo++) {
        int serviceFloors[2] = { elevators[eleNo].serviceFloors[0], elevators[eleNo].serviceFloors[1] };
        /* 交汇区 */
        if (srcFloor == 1 && tarFloor >= serviceFloors[0] && tarFloor <= serviceFloors[1]) {
            break;
        }
        else if (srcFloor == serviceFloors[0]) {
            /* 乘客目标楼层在电梯服务区 */
            if (tarFloor >= serviceFloors[0] && tarFloor <= serviceFloors[1]) {
                break;
            }
            else if (tarFloor > serviceFloors[1]) {
                break;
            }
        }
        else if (srcFloor == serviceFloors[1]) {
            /* 乘客目标楼层在电梯服务区 */
            if (tarFloor >= serviceFloors[0] && tarFloor <= serviceFloors[1]) {
                break;
            }
            else if (tarFloor < serviceFloors[0]) {
                break;
            }
        }
        /* 非交汇区 */
        else if (srcFloor >= serviceFloors[0] && srcFloor <= serviceFloors[1]) {
            break;
        }
        else {
            tag++;
        }
    }

    if (tag == 3) return;


    TargetFloorList *cur = elevators[eleNo].targetFloor;
    if (cur->next == nullptr) {
        allocTargetFloor(eleNo, srcFloor);
    }
    else if (elevators[eleNo].nowLoad == 0 && cur->next->floor == 1 && cur->next->next == nullptr) {
        allocTargetFloor(eleNo, srcFloor);
    }
    else if (elevators[eleNo].runStatus.status == 1 && runStatus == 1) {
        if (elevators[eleNo].nowFloor <= srcFloor) {
            allocTargetFloor(eleNo, srcFloor);
        }
        else return;
    }
    else if (elevators[eleNo].runStatus.status == -1 && runStatus == -1) {
        if (elevators[eleNo].nowFloor >= srcFloor) {
            allocTargetFloor(eleNo, srcFloor);
        }
        else return;
    }
}

/* 响应电梯内部请求 */
void ElevatorSystem::getInElevatorRequest(int eleNo, int tarFloor)
{
    int tar = 0;
    if (tarFloor == 1) {
        tar = 1;
    }
    else if (tarFloor < elevators[eleNo].serviceFloors[0]) {
        tar = elevators[eleNo].serviceFloors[0];
    }
    else if (tarFloor > elevators[eleNo].serviceFloors[1]) {
        tar = elevators[eleNo].serviceFloors[1];
    }
    else {
        tar = tarFloor;
    }
    allocTargetFloor(eleNo, tar);
}

/* 查询是否还有乘客进电梯 */
PassengerList* ElevatorSystem::searchPassengerIn(int eleNo)
{
    /* 电梯满载时 */
    if (elevators[eleNo].nowLoad == maxLoad) {
        return nullptr;
    }

    int nowFloor = (int)elevators[eleNo].nowFloor;
    int serviceFloors[2] = { elevators[eleNo].serviceFloors[0], elevators[eleNo].serviceFloors[1] };

    for (PassengerList *cur = outFloorPeople[nowFloor - 1]; cur->next != nullptr; cur = cur->next) {
        int tarFloor = cur->next->tarFloor;
        /* 尚未发出请求的乘客 */
        if (cur->next->requestTime > SystemTime) {
            return nullptr;
        }
        /* 运行方向相同 */
        else if (cur->next->runStatus == elevators[eleNo].runStatus.status) {
            /* 交汇区 */
            if (nowFloor == 1) {
                /* 乘客目标楼层在电梯服务区 */
                if (tarFloor >= serviceFloors[0] && tarFloor <= serviceFloors[1]) {
                    return cur;
                }
            }
            else if (nowFloor == serviceFloors[0]) {
                /* 乘客目标楼层在电梯服务区  乘客目标楼层在服务层之上 */
                if ((tarFloor >= serviceFloors[0] && tarFloor <= serviceFloors[1]) || tarFloor > serviceFloors[1]) {
                    return cur;
                }
            }
            else if (nowFloor == serviceFloors[1]) {
                /* 乘客目标楼层在电梯服务区  乘客目标楼层在服务层之下*/
                if ((tarFloor >= serviceFloors[0] && tarFloor <= serviceFloors[1]) || tarFloor < serviceFloors[0]) {
                    return cur;
                }
            }
            /* 非交汇区 */
            else if (nowFloor >= serviceFloors[0] && nowFloor <= serviceFloors[1]) {
                return cur;
            }
        }
    }
    /* 楼层中无人进入 */
    return nullptr;
}

/* 查询是否还有乘客出电梯 */
PassengerList* ElevatorSystem::searchPassengerOut(int eleNo)
{
    /* 电梯空载时 */
    if (elevators[eleNo].nowLoad == 0) {
        return nullptr;
    }

    int nowFloor = (int)elevators[eleNo].nowFloor;
    int serviceFloors[2] = { elevators[eleNo].serviceFloors[0], elevators[eleNo].serviceFloors[1] };

    for (PassengerList *cur = elevators[eleNo].inElePeople; cur->next != nullptr; cur = cur->next) {
        /* 乘客目标楼层为当前楼层 */
        if (cur->next->tarFloor == nowFloor) {
            return cur;
        }
        /* 跳过前往基层的乘客 */
        else if (cur->next->tarFloor == 1) {
            if (nowFloor == 1) return cur;
            else continue;
        }
        /* 乘客目标楼层小于电梯最小服务楼层 */
        else if (nowFloor == serviceFloors[0] && cur->next->tarFloor < serviceFloors[0]) {
            return cur;
        }
        /* 乘客目标楼层大于电梯最大服务楼层 */
        else if (nowFloor == serviceFloors[1] && cur->next->tarFloor > serviceFloors[1]) {
            return cur;
        }
    }
    /* 电梯中无人离开 */
    return nullptr;
}


/* 电梯逻辑模块 */
//------------------------------------------------------------------------------------
/* 采用LOOK算法：变更电梯运行状态 */
void ElevatorSystem::changeElevatorStatus(int eleNo)
{
    bool downTag = false, upTag = false, nowTag = false;
    float nowFloor = elevators[eleNo].nowFloor;

    TargetFloorList *cur = elevators[eleNo].targetFloor;
    /* 就绪等待状态 */
    if (elevators[eleNo].elevatorStatus.status == -2) {
        if (cur->next != nullptr) {
            elevators[eleNo].elevatorStatus = { 0, 0 };
        }
    }
    /* 无请求状态 */
    else if (cur->next == nullptr) {
        if (elevators[eleNo].elevatorStatus.status == -1) return;
        else elevators[eleNo].elevatorStatus = { -1, 0 };
    }
    /* 空载超时回归基层后，置就绪等待态 */
    else if (elevators[eleNo].nowLoad == 0 && nowFloor == 1 && cur->next->floor == 1 && cur->next->next == nullptr) {
        elevators[eleNo].elevatorStatus = { -2, 0 };
        cancelTargetFloor(eleNo, 1);
    }
    /* 其他操作状态 */
    else if (elevators[eleNo].elevatorStatus.status == -1) {
        elevators[eleNo].elevatorStatus = { 0, 0 };
    }


    /* 改变电梯行进状态 */

    /* 查询电梯目标楼层情况 */
    for (; cur->next != nullptr; cur = cur->next) {
        if (nowFloor == (float)cur->next->floor) {
            nowTag = true;
        }
        else if ((float)cur->next->floor > nowFloor) {
            upTag = true;
        }
        else if ((float)cur->next->floor < nowFloor) {
            downTag = true;
        }
    }

    /* 到达大楼基层，置1 */
    if (nowFloor == (float)1) {
        elevators[eleNo].runStatus.status = 1;
    }
    /* 到达服务顶层，置-1*/
    else if (nowFloor == (float)elevators[eleNo].serviceFloors[1]) {
        elevators[eleNo].runStatus.status = -1;
    }
    /* 到达目标楼层 上行行均无请求*/
    else if (nowTag && !upTag && !downTag) {
        /* 试探性翻转方向，检测开门 */
        int status = elevators[eleNo].elevatorStatus.status;
        if (status == 0 || status == 2) {
            if (searchPassengerIn(eleNo) == nullptr) {
                elevators[eleNo].runStatus.status = -elevators[eleNo].runStatus.status;
            }
            else if (searchPassengerIn(eleNo) == nullptr) {
                elevators[eleNo].runStatus.status = -elevators[eleNo].runStatus.status;
            }
        }
    }
    /* 到达非目标楼层 */
    else if (!nowTag) {
        /* 上行行均无请求 */
        if (!upTag && !downTag) {
            elevators[eleNo].elevatorStatus = { -1, 0 };
        }
        /* 电梯行进方向无请求，转向 */
        else if (!upTag && elevators[eleNo].runStatus.status == 1) {
            elevators[eleNo].runStatus.status = -1;
        }
        else if (!downTag && elevators[eleNo].runStatus.status == -1) {
            elevators[eleNo].runStatus.status = 1;
        }
    }


    /* 运行中 改变电梯操作状态 */
	if (elevators[eleNo].elevatorStatus.status == 1) {
		if (nowTag) checkOpenDoor(eleNo);
		else elevators[eleNo].elevatorStatus = { 1, 0 };
	}
}

/* 电梯移动 */
void ElevatorSystem::runElevator(int eleNo) {
    /* 判断电梯运行方向 */
    switch (elevators[eleNo].runStatus.status) {
    case 1: elevators[eleNo].nowFloor += 0.5; break;
    case -1:elevators[eleNo].nowFloor -= 0.5; break;
    }
}

/* 开电梯门 */
void ElevatorSystem::openDoor(int eleNo)
{
    int gapTime = elevators[eleNo].elevatorStatus.timer;

    /* 初始开门 进行开门 */
    if (!gapTime || gapTime < openTime) {
        elevators[eleNo].elevatorStatus.timer++;
    }
    /* 完成开门 */
    else if (gapTime == openTime) {
        elevators[eleNo].doorStatus = { 1, 0 };
        outputRunStatus(eleNo, 0);

        /* 完成开门后 有人出门置 5，有人进门置 4 */
        if (searchPassengerOut(eleNo) != nullptr) {
            elevators[eleNo].elevatorStatus = { 5, 0 };
            leaveElevator(eleNo);
        }
        else if (searchPassengerIn(eleNo) != nullptr) {
            elevators[eleNo].elevatorStatus = { 4, 0 };
            enterElevator(eleNo);
        }
        else elevators[eleNo].elevatorStatus = { 0, 0 };
    }
}

/* 关电梯门 */
void ElevatorSystem::closeDoor(int eleNo)
{
    int gapTime = elevators[eleNo].elevatorStatus.timer;

    /* 初始关门 进行关门 */
    if (!gapTime || gapTime < closeTime) {
        elevators[eleNo].elevatorStatus.timer++;

        /* 如果关门期间有人进入 */
        if (searchPassengerIn(eleNo) != nullptr) {
            /* 初始关门中，取消关门，继续进人 */
            if (!gapTime) {
                elevators[eleNo].elevatorStatus = { 4, 0 };
                enterElevator(eleNo);
            }
            /* 进行关门中，取消关门，继续开门 */
            else if (gapTime < closeTime) {
                elevators[eleNo].elevatorStatus = { 2, openTime - gapTime };
                openDoor(eleNo);
            }
        }
    }
    /* 完成关门 */
    else if (gapTime == closeTime) {
        elevators[eleNo].doorStatus = { 0, 0 };
        outputRunStatus(eleNo, 0);

        /* 取消目标楼层，改变操作状态为 RUN */
        cancelTargetFloor(eleNo, (int)elevators[eleNo].nowFloor);
        elevators[eleNo].elevatorStatus = { 1, 0 };
    }
}

/* 乘客进入 */
void ElevatorSystem::enterElevator(int eleNo)
{
    int gapTime = elevators[eleNo].elevatorStatus.timer;

    /* 初始化乘客进入电梯 进行乘客进入电梯 */
    if (!gapTime || gapTime < 3) {
        elevators[eleNo].elevatorStatus.timer++;
    }
    /* 完成乘客进入电梯 */
    else if (gapTime == 3) {
        PassengerList *cur = searchPassengerIn(eleNo);

        /* 响应进电梯乘客请求 */
		getInElevatorRequest(eleNo, cur->next->tarFloor);
		outputRunStatus(eleNo, cur->next->peoNo);

		/* 进电梯后加入该乘客,删除楼层中乘客 */
		addElevatorPassenger(eleNo, cur->next->peoNo, cur->next->requestTime, cur->next->srcFloor, cur->next->tarFloor);
		cur->next = cur->next->next;
		elevators[eleNo].nowLoad++;

        /* 有人进入 */
        if (searchPassengerIn(eleNo) != nullptr) {
            elevators[eleNo].elevatorStatus = { 4, 0 };
            enterElevator(eleNo);
        }
        /* 无人进入 */
        else elevators[eleNo].elevatorStatus = { 0, 0 };
    }
}

/* 乘客离开 */
void ElevatorSystem::leaveElevator(int eleNo)
{
    int gapTime = elevators[eleNo].elevatorStatus.timer;

    /* 初始化乘客离开电梯 进行乘客离开电梯 */
    if (!gapTime || gapTime < 3) {
        elevators[eleNo].elevatorStatus.timer++;
    }
    /* 完成乘客离开电梯 */
    else if (gapTime == 3) {
        PassengerList *cur = searchPassengerOut(eleNo);
        int nowFloor = (int)elevators[eleNo].nowFloor, tarFloor = cur->next->tarFloor;
        int serviceFloors[2] = { elevators[eleNo].serviceFloors[0], elevators[eleNo].serviceFloors[1] };

        /* 目标在服务楼层上或下 */
        bool overTag = false;
        if ((nowFloor == serviceFloors[1] && tarFloor > serviceFloors[1]) || (nowFloor == serviceFloors[0] && tarFloor < serviceFloors[0])) {
            overTag = true;
        }
        /* 超出服务楼层离开电梯后加入当前楼层 */
        if (overTag) {
            addFloorPassenger(cur->next->peoNo, SystemTime, nowFloor, tarFloor);
        }

        outputRunStatus(eleNo, cur->next->peoNo);
        cur->next = cur->next->next;
        elevators[eleNo].nowLoad--;

        /* 有人离开 */
        if (searchPassengerOut(eleNo) != nullptr) {
            elevators[eleNo].elevatorStatus = { 5, 0 };
            leaveElevator(eleNo);
        }
        /* 有人进入 */
        else if (searchPassengerIn(eleNo) != nullptr) {
            elevators[eleNo].elevatorStatus = { 4, 0 };
            enterElevator(eleNo);
        }
        /* 无人进出 */
        else elevators[eleNo].elevatorStatus = { 0, 0 };
    }
}

/* 电梯目标楼层处理模块 */
//------------------------------------------------------------
/* 分配电梯的目标楼层 */
void ElevatorSystem::allocTargetFloor(int eleNo, int tarFloor)
{
    TargetFloorList *cur = elevators[eleNo].targetFloor;
    for (; cur->next != nullptr; cur = cur->next) {
        /* 目标楼层存在，返回 */
        if (cur->next->floor == tarFloor) return;
    }
    /* 目标楼层不存在，插入调度尾 */
    cur->next = new TargetFloorList(tarFloor);
}

/* 取消电梯的目标楼层 */
void ElevatorSystem::cancelTargetFloor(int eleNo, int tarFloor)
{
    TargetFloorList *cur = elevators[eleNo].targetFloor;
    /* 查找删除该目标楼层 */
    for (; cur->next != nullptr; cur = cur->next) {
        if (cur->next->floor == tarFloor) {
            cur->next = cur->next->next;
            return;
        }
    }
}

/* 检测模块 */
//------------------------------------------------------------
/* 检测能否开门模块 */
void ElevatorSystem::checkOpenDoor(int eleNo) 
{
	int nowFloor = (int)elevators[eleNo].nowFloor;
	int serviceFloors[2] = { elevators[eleNo].serviceFloors[0], elevators[eleNo].serviceFloors[1] };

	/* 开门状态，不开门 */
	if (elevators[eleNo].doorStatus.status == 1) return;
	/* 关门状态，检测目标楼层 */
	else {
		bool nowTag = false;
		/* 检测是否为目标楼层 */
		for (TargetFloorList *cur = elevators[eleNo].targetFloor; cur->next != nullptr; cur = cur->next) {
			if (nowFloor == (float)cur->next->floor) {
				nowTag = true;
				break;
			}
		}
		/* 处于服务楼层 */
		if (nowTag && (nowFloor == 1 || nowFloor >= serviceFloors[0] && nowFloor <= serviceFloors[1]) ) {
			/* 有人进出 */
			if (searchPassengerOut(eleNo) != nullptr || searchPassengerIn(eleNo) != nullptr) {
				elevators[eleNo].elevatorStatus = { 2, 0 };
				return;
			}
		}
		/* 不处于服务楼层 */
		elevators[eleNo].elevatorStatus = { 1, 0 };
	}
}

/* 检测能否关门模块 */
void ElevatorSystem::checkCloseDoor(int eleNo)
{
    int stayTimer = elevators[eleNo].doorStatus.timer;

    /* 检测计时初始化 检测计时进行中*/
    if (!stayTimer || stayTimer < checkTime) {
        elevators[eleNo].doorStatus.timer++;

        /* 如果出人进人中，跳过 */
        if (elevators[eleNo].elevatorStatus.status == 5 || elevators[eleNo].elevatorStatus.status == 4);
        /* 如果有人进入 */
        else if (searchPassengerIn(eleNo) != nullptr) {
            elevators[eleNo].elevatorStatus = { 4, 0 };
            enterElevator(eleNo);
        }
    }
    /* 到达检测周期，完成检测 */
    else if (stayTimer == checkTime) {
        /* 如果出人进入中，重置计时 */
        if (elevators[eleNo].elevatorStatus.status == 5 || elevators[eleNo].elevatorStatus.status == 4) {
            elevators[eleNo].doorStatus.timer = 0;
        }
        /* 如果非进人中，重置计时，继续进人 */
        else if (searchPassengerIn(eleNo) != nullptr) {
            elevators[eleNo].doorStatus.timer = 0;

            elevators[eleNo].elevatorStatus = { 4, 0 };
            enterElevator(eleNo);
        }
        /* 处于停留态 无人进入 关门 */
        else if (elevators[eleNo].elevatorStatus.status == 0 && searchPassengerIn(eleNo) == nullptr) {
            elevators[eleNo].doorStatus.timer = 0;

            elevators[eleNo].elevatorStatus = { 3, 0 };
            closeDoor(eleNo);
        }
    }
}

/* 检测超时等待模块 */
void ElevatorSystem::checkOverWait(int eleNo)
{
    int waitTimer = elevators[eleNo].elevatorStatus.timer;

    /* 初始化等待计时 进行等待计时 */
    if (!waitTimer || waitTimer < maxStayTime) {
        elevators[eleNo].elevatorStatus.timer++;
    }
    /* 完成等待计时 */
    else if (waitTimer == maxStayTime) {
        outputRunStatus(eleNo, 0);
        allocTargetFloor(eleNo, 1);
        elevators[eleNo].elevatorStatus = { 1, 0 };
    }
}

void ElevatorSystem::outputRunStatus(int eleNo, int peoNo)
{
    QFile file("./SystemRunStatus.txt");
    if(!file.open(QIODevice::WriteOnly | QIODevice::Append)){
        qDebug() << "Failed opening result data！" << endl;
        file.close();
        return;
    }

    QTextStream fout(&file);
    switch (elevators[eleNo].elevatorStatus.status)
    {
    /* 检测等待超时 */
    case -1:
        qDebug() << "Time:\t" << SystemTime - maxStayTime << " - " << SystemTime << "\t\t\teleNo:\t" << eleNo + 1 << "\tnow floor:\t" << elevators[eleNo].nowFloor << "\tover wait time" << endl;
        fout << "Time:\t" << SystemTime - maxStayTime << " - " << SystemTime << "\t\t\teleNo:\t" << eleNo + 1 << "\tnow floor:\t" << elevators[eleNo].nowFloor << "\tover wait time\r\n";
        break;
    /* 电梯完成开门 */
    case 2:
        qDebug() << "Time:\t" << SystemTime - openTime << " - " << SystemTime << "\t\t\teleNo:\t" << eleNo + 1 << "\tnow floor:\t" << elevators[eleNo].nowFloor << "\topen door" << endl;
        fout << "Time:\t" << SystemTime - openTime << " - " << SystemTime << "\t\t\teleNo:\t" << eleNo + 1 << "\tnow floor:\t" << elevators[eleNo].nowFloor << "\topen door\r\n";
        break;
    /* 电梯完成关门 */
    case 3:
        qDebug() << "Time:\t" << SystemTime - closeTime << " - " << SystemTime << "\t\t\teleNo:\t" << eleNo + 1 << "\tnow floor:\t" << elevators[eleNo].nowFloor << "\tclose door" << endl;
        fout << "Time:\t" << SystemTime - closeTime << " - " << SystemTime << "\t\t\teleNo:\t" << eleNo + 1 << "\tnow floor:\t" << elevators[eleNo].nowFloor << "\tclose door\r\n";
        break;
    /* 乘客完成进入 */
    case 4:
        qDebug() << "Time:\t" << SystemTime - 3 << " - " << SystemTime << "\t\t\teleNo:\t" << eleNo + 1 << "\tnow floor:\t" << elevators[eleNo].nowFloor << "\tpassenger in:\tNo-" << peoNo << endl;
        fout << "Time:\t" << SystemTime - 3 << " - " << SystemTime << "\t\t\teleNo:\t" << eleNo + 1 << "\tnow floor:\t" << elevators[eleNo].nowFloor << "\tpassenger in:\tNo-" << peoNo << "\r\n";
        break;
    /* 乘客完成离开 */
    case 5:
        qDebug() << "Time:\t" << SystemTime - 3 << " - " << SystemTime << "\t\t\teleNo:\t" << eleNo + 1 << "\tnow floor:\t" << elevators[eleNo].nowFloor << "\tpassenger out:\tNo-" << peoNo << endl;
        fout << "Time:\t" << SystemTime - 3 << " - " << SystemTime << "\t\t\teleNo:\t" << eleNo + 1 << "\tnow floor:\t" << elevators[eleNo].nowFloor << "\tpassenger out:\tNo-" << peoNo <<"\r\n";
        break;
    /* 超出最大容忍时间 */
    default:
        qDebug() << "Time:\t" << SystemTime - maxTolerateTime << " - " << SystemTime << "\tnow floor:\t" << eleNo + 1 << "\tpassenger leave:\tNo-" << peoNo << endl;
        fout << "Time:\t" << SystemTime - maxTolerateTime << " - " << SystemTime << "\tnow floor:\t" << eleNo + 1 << "\tpassenger leave:\tNo-" << peoNo << "\r\n";
        break;
    }

    file.close();
}

void ElevatorSystem::outputPeopleMessage(int floorNo, int peoNo) 
{
	QFile file("./SystemRunStatus.txt");
    if(!file.open(QIODevice::WriteOnly | QIODevice::Append)){
        qDebug() << "Failed opening result data！" << endl;
        file.close();
        return;
    }

    QTextStream fout(&file);

    qDebug() << "Time:\t" << SystemTime - maxTolerateTime << " - " << SystemTime << "\tnow floor:\t" << floorNo + 1 << "\tpassenger leave:\tNo-" << peoNo << endl;
	fout << "Time:\t" << SystemTime - maxTolerateTime << " - " << SystemTime << "\tnow floor:\t" << floorNo + 1 << "\tpassenger leave:\tNo-" << peoNo << endl;

	file.close();
}
