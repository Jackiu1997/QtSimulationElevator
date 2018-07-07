#ifndef ELEVATORSYSTEM_H
#define ELEVATORSYSTEM_H

#include "structs.h"

class ElevatorSystem 
{
private:
    /* 电梯系统参数 */
	//------------------------------------------------------------------------------------
    int SystemTime;
    static const int MAX_LOAD = 10;									    // 电梯最大负载
	static constexpr int ELE_DISTRIBUTION[4] = {1,4,7,10};              // 电梯群服务楼层
	/* main time */
    static const int RUN_TIME = 2;										// 电梯移动时间
    static const int OPEN_TIME = 2;										// 电梯开门时间
    static const int CLOSE_TIME = 2;									// 电梯关门时间
	static const int INOUT_TIME = 3;									// 乘客进出时间
    static const int CHECK_TIME = 4;									// 检测关门周期
    static const int MAX_STAY_TIME = 30;								// 电梯最大停留时间
	static const int MAX_TOLERATE_TIME = 180;							// 乘客最大容忍时间
	/* main data */
	Elevator elevators[3];												// 电梯数据数组
	PassengerList *outFloorPeople[10];									// 楼层乘客链表
	

	/* 乘客逻辑模块 */
	//------------------------------------------------------------------------------------
	void addElevatorPassenger(int eleNo, int no, int time, int src, int tar);
    void addFloorPassenger(int no, int time, int src, int tar);
	void readPassengers();												// 初始化乘客数据模块
	void readInOutRequest();											// 内外乘客请求处理
	void getOutFloorRequest(int srcFloor, int tarFloor, int runStatus); // 读取楼层中乘客请求
	void getInElevatorRequest(int eleNo, int tarFloor);				    // 读取电梯内乘客请求	
	PassengerList* searchPassengerIn(int eleNo);						// 进入电梯乘客检索模块
	PassengerList * searchPassengerOut(int eleNo);						// 离开电梯乘客检索模块


	/* 电梯逻辑模块 */
	//------------------------------------------------------------------------------------
	/* 电梯系统运行模块 */
    void changeElevatorStatus(int eleNo);                               // 电梯状态判断模块
	void runElevator(int eleNo);										// 电梯移动
	void openDoor(int eleNo);											// 开电梯门
	void closeDoor(int eleNo);											// 关电梯门
	void enterElevator(int eleNo);										// 乘客离开
	void leaveElevator(int eleNo);										// 乘客进入

	/* 电梯目标楼层处理模块 */
	void allocTargetFloor(int eleNo, int tarFloor);	                    // 分配电梯的目标楼层
	void cancelTargetFloor(int eleNo, int tarFloor);                    // 取消电梯的目标楼层

	/* 检测模块 */
	void checkOpenDoor(int eleNo);										// 检测能否开门模块
	void checkCloseDoor(int eleNo);										// 检测能否关门模块
	void checkOverWait(int eleNo);										// 检测超时等待模块


	/* 结果输出模块 */
	//------------------------------------------------------------------------------------
    void outputRunStatus(int eleNo, int peoNo);                         // 运行状态导出模块
	void outputPeopleMessage(int floorNo, int peoNo);

public:
    ElevatorSystem();
    void stepRunElevator();                                             // 单步电梯运行电梯

    /* 外部UI数据接口 */
    //------------------------------------------------------------------------------------
    int getFloorPassengerNum(int floorNo);                              // 输出楼层乘客数目
    ElevatorMessage getElevatorMessage(int eleNo);                      // 输出电梯数据信息
};

#endif // ELEVATORSYSTEM_H
