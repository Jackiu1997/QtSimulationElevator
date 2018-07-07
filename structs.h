#ifndef STRUCTS_H
#define STRUCTS_H

/* 乘客链表 */
//--------------------------------------------------------------
struct PassengerList {
	int peoNo;							    // 乘客个人编号
	int requestTime;					    // 乘客请求时间
	int srcFloor;						    // 乘客起始楼层
	int tarFloor;						    // 乘客目标楼层
	int runStatus;						    // 乘客前行方向
	PassengerList *next;				

	PassengerList();
	PassengerList(int peono, int request, int src, int tar);
};


/* 电梯目标楼层链表 */
//--------------------------------------------------------------
struct TargetFloorList {
	int floor;							    // 目标楼层
	TargetFloorList *next;

	TargetFloorList(int f = 0) {
		floor = f;
		next = nullptr;
	}
};

/* 命令结构体 */
//--------------------------------------------------------------
struct Command {
	int status;						    	// 命令状态	
	int timer;						    	// 计时器
};

/* 电梯定义 */
//--------------------------------------------------------------
struct Elevator {
	int serviceFloors[2];				    // 电梯服务楼层区间
	float nowFloor;						    // 电梯当前楼层
	int nowLoad;						    // 电梯当前负载

	Command runStatus;					    // UP = 1, DOWN = -1
	Command doorStatus;					    // CLOSE = 0, OPEN = 1
	Command elevatorStatus;				    /* READYWAIT = -2, NOREQUEST = -1
										     * STAY = 0, RUN = 1
										     * DOOROPEN = 2, DOORCLOSE = 3
										     * INELE = 4, OUTELE = 5 */

	PassengerList *inElePeople;			    // 电梯乘客链表
	TargetFloorList *targetFloor;		    // 目标楼层链表

	Elevator();
	Elevator(int down, int up);
};

/* 电梯信息结构题体 */
//--------------------------------------------------------------
struct ElevatorMessage {
    float nowFloor;
    int nowLoad;

    Command runStatus;
    Command doorStatus;
    Command elevatorStatus;

	ElevatorMessage();
	ElevatorMessage(Elevator e);
};

#endif // STRUCTS_H
