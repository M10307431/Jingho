#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <limits>
#include <math.h>
#include <map> 
#include <memory>
#include <conio.h>

#include "include/Struct/WSNFile.h"
#include "include/Struct/WSNStruct.h"
#include "include/Algorithm/ConnInterval.h"
#include "include/Schedule/FlowSchedule.h"
#include "include/Struct/WSNEnergy.h"
#include "include/Algorithm/TDMA.h"

using namespace std;

/*=================================
		Experiment Setting
==================================*/
const float inv_r=40;					//Input �U��rate���Z
const float MIN_Rate=40;				//Input�}�l��rate
const float MAX_Rate=960;				//Input�̲�rate
const short int Set=100;				//Input set �ƶq

short int readsetting=1;				//�O�_�nŪ�����aSetting.txt
short int Service_interval=3;			//0=>Event, 1=>MEI, 2=>DIF, 3=>Lazy and 4=>Greedy (Min period) <��@node�W��varied data�վ�>, 5=>IOS(30ms)
short int Connection_Interval=2;		//0=>LDC (�Uservice interval��node1level), 1=>Greedy (�� Min interval��node1level), 2=>EIMA <TDMA�Mconnection interval�W���ե�>, 3=>IOS(20ms)
short int WriReq_Sche=2;				//0=>NPEDF 1=>RR 2=>EIF 3=>Polling <Gateway �q��node�ǿ鶶��>

bool sche_flag=false;					//�O�_�n����schedulability
int EXECBclock=200;						//Lazy Timer (ms)
int dec_cof=5;							//Lazy��decrease�Y��
bool EIMADemand_flag=false;				//�P�_EIMA�p��O�_�n��demand bound�p��

/*=================================
		Global value
==================================*/
short int TDMAproposal=0;				//TDMA��assign��k 0=>�ۤv����k(�u���@��superslot), 1=>Node base��k (�|�A����[�Jsuperslot)
int Pktsize=0;							//�p��IntervalPower��pkt num
double Meetcount=0;						//�bSet�ƶq���Ameet���ƶq
double AverageE=0;						//���ަ��Smiss�A�p�����Set��energy

int overheadcount=6;					//�ʺA����interval�ɻݭn����6��interval�~���ܰ�
FrameTable* Cycle=NULL;					//Polling schedule�Ψ쪺�ഫcyle
short int pollingcount=1;				//Polling schedule �p��cycle id

int Callbackclock;						//�p��Callback�ɶ�
Edge *HeadEdge=new Edge;
Edge *MainEdge=new Edge;
Edge *ConflictEdge=new Edge;
FrameTable *FrameTbl=new FrameTable;
TDMATable *TDMA_Tbl=new TDMATable;
PacketBuffer* Buffer=new PacketBuffer;
Node* SetHead=new Node;
Node* Head=new Node;
Packet* Headpacket=new Packet;
Node *SetNode=new Node;
Node* node=new Node;
Packet* packet=new Packet;
Packet *ReadyQ=new Packet();
Flow *Headflow=new Flow();

int ReadyQ_overflag=0;
stringstream stream;
string str_coor_x,str_coor_y,str_radius;
string strload,strperiod,strutilization,strhop;
int nodenum=0;						//�Ҧ�node�`��
int nodelevel1=0;					//Connection node�ƶq		(��input���o)
int nodelevel2=0;					//Advertisement node�ƶq		(��input���o)
int pktnum=0;						//�C�@node����packet�ƶq		(��input���o)
long int Timeslot=0;				//Schedule�� �p�ƪ�time slot (��input���o)
long int Hyperperiod=0;				//Hyper-period				(��input���o)
double Maxrate=20;					//�̰��t�׬�20bytes/slot
double payload=20;					//payload �� 20bytes
int Maxbuffersize=4;				//Maxbuffersize �� 4��packets

double slotinterval=10;				//�̵uconnection interval��10ms
double Minumum_interval=10;			//�̵uconnection interval��10ms
double Connectioninterval=0;		//Conneciton inteval �u�|�b10ms~4000ms
double totalevent=0;				//Event�ƶq
bool Meetflag=true;					//�ݬO�_meet deadline

double IOS_ServiceInterval=40;		//��@node�� IOS�t�ι��device�Wservice interval�վ� (ms)
double IOS_ConnectionInterval=20;	//Muliple node�� IOS�t�ι��device�Wconnection interval�վ� (ms)
/*========================================
		Power function parameter
========================================*/
double Vcc=3.3;					//BLE �X�ʹq��
double I_sleep=0.000001;		//Sleep �q�y 1uA
double Time_sleep=0.001;		//Sleep �ɶ� 1ms (uint time)
double I_notify=0.008246;		//Notify �q�y 8.246mA
double Time_notify=0.002675;	//Notify �ɶ� 2.675ms
double I_Tran=0.009564;			//Transmission �q�y 14.274mA
double Time_Tran=0.00182;		//Transmission �ɶ� 0.49ms
double BatteryCapacity=0.230;	//230mAh (����L�g�� �O�H540mAh <Energy Efficient MAC for Qos Traffic in Wireless Body Area Network)>
double unit=0.001;				//�ɶ���쬰1ms

/*========================================
		Create Object
========================================*/
EventInterval Interval_obj;
TDMA TDMA_obj;

/*========================================
			Main Function
========================================*/
int main(int argc, char* argv[]){
	/*
	cout<<"Type single node interval(0->Event, 1->MEI):";
	cin>>Service_interval;
	cout<<"Type TDMA table (0->single superslot, 1->Node base):";
	cin>>TDMAproposal;
	cout<<"Type TDMA with interval (0->EIMA, 1->Divide small interval by TDMA size):";
	cin>>Connection_Interval;
	cout<<"Type TDMA schedule (0->EDF, 1->TDMA table):";
	cin>>WriReq_Sche;
	*/	

	for(float U=MIN_Rate; U<=MAX_Rate; U+=inv_r){
		cout<<"Data Rate: "<<U<<endl;
		
		//-----------------------------Init setting for new set
		SetHead->lifetime=0;
		delete SetNode;SetNode=NULL;
		Meetcount=0;
		AverageE=0;
		totalevent=0;

		//-----------------------------Read setting file
		CreateFile(U,Set,argv[0]);//�}��WSNGEN �åB�إ߿�X�ɮ� (WSNFile.cpp)
		if(readsetting==1){
			ExperimentSetting(&Service_interval, &Connection_Interval, &WriReq_Sche);//������]�w��X
		}

		/*===================================================
					�b�P�@Data Rate�U �]Set��
		===================================================*/
		for(short int setnum=0;setnum<Set;setnum++){
			Meetflag=true;		//Reset Meet flag
			Timeslot=0;			//Reset time slot
			
			Hyperperiod=0;		
			totalevent=0;
			Cycle=NULL;
			pollingcount=1;

			/*==========================
				�إ�Linklist�H��
				GEN����Ʃ�i�h
				(WSNStruct.cpp)
			==========================*/
			StructGEN();		
			
			/*==========================
				�p��Service interval
			==========================*/
			Interval_obj.ServiceInterval_Algorithm(Service_interval);		//�w�Ʀn�U��node�W��interval

			/*==========================
				Topology & TDMA assignment
			==========================*/
			TDMA_obj.Topology();						//�إ߶ǿ�ؼ�node, �Z�����Y & �إ�conflict���Y
			TDMA_obj.NodeColoring();					//�w�ƦU��node��color
			TDMA_obj.TDMA_Assignment(TDMAproposal);		//�w�ƦnTDMA_Tbl

			/*==========================
				�p��Connection interval 
			==========================*/
			Interval_obj.ConnectionInterval_Algorithm(Connection_Interval);		//�]�tTDMA�Ҷq,��node�W��interval�ק� �B�t��Scan duration �p��

			/*=========================
				Schedulability test
			=========================*/
			if(sche_flag){
				Schedulability();
			}

			/*==========================
				EDF scheduling
				(FlowSchedule.cpp)
				<Head, TDMA_Tbl> 
			==========================*/
			Head->RecvNode=NULL;		//Head �����`�I�n�]�w��NULL
			Head->FrameSize=0;			//Head �|counting waiting time
			TDMA_Tbl->currslot=true;	//�@�}�l�Ĥ@�ӭn��true
			Callbackclock=0;

			while(Timeslot<=Hyperperiod){
				PacketQueue();
				Schedule(WriReq_Sche,Service_interval);
				
				Timeslot++;
			}
			Finalcheck();

			/*==========================
					END
			==========================*/
			SaveFile(setnum);//(WSNFile.cpp)

		}//Set End

		SaveSet(U,Set);//(WSNFile.cpp)
	}
	CloseFinal();

	cout<<"Simulation Done :)"<<endl;
	system("PAUSE");
	return 0;
}
