extern double Vcc;			//BLE �X�ʹq��
extern double I_sleep;		//Sleep �q�y 
extern double I_notify;		//Notify �q�y 
extern double Time_sleep;		//Sleep �q�y 10ms
extern double Time_notify;	//Notify �ɶ� 2.775ms
extern double I_Tran;	//Transmission �q�y 14.2744mA
extern double Time_Tran;	//Transmission �ɶ� 0.49ms
extern double BatteryCapacity;

extern double unit;			//�ɶ���쬰10ms
extern double parma;
extern double parmb;
extern bool practice;		//�t�X��@I_sleep 400uA

void NodeEnergy();
void NodeState();
void Node_EnergyState(Node *);