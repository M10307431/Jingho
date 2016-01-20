#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <math.h>
#include <map>
#include <memory>

#include "../Struct/WSNFile.h"
#include "../Struct/WSNStruct.h"
#include "ConnInterval.h"
#include "../Schedule/FlowSchedule.h"
#include "../Struct/WSNEnergy.h"

#undef  _ShowLog

using namespace std;

/*==============================================
				Construct
==============================================*/
EventInterval::EventInterval(){
	printf("Connection Interval Object\n");
}

/*==============================================
	��ܻݭn���@�� Service interval �p��覡
	(Single node with varied data rate)
==============================================*/
void EventInterval::ServiceInterval_Algorithm(int Rateproposal){
	switch (Rateproposal){
	case 0:
		Event();		//�C�@node connection interval ���� 10ms
		break;
	case 1:
		MEI(NULL);		//��Demand bound�p�� service interval
		break;
	case 2:
		DIF();			//�ΦU��pkt�϶��p��Pload�p��rate��pkt�A�|�A�ഫ��service interval
		break;
	case 4:
		Greedy();		//�γ̵uminimum period��@service interval
		break;
	case 5:
		Single_IOS(IOS_ServiceInterval);
		break;
	default:
		break;
	}

	//Reset load
	for(Packet *pkt=Head->nextnd->pkt; pkt!=NULL; pkt=pkt->nextpkt){
		pkt->exeload=pkt->load;
		pkt->exehop=pkt->hop;
	}
	
}

/*==============================================
	��ܻݭn���@�� Connection interval ��k
	(Mulitple node with collision constrint)
==============================================*/
void EventInterval::ConnectionInterval_Algorithm(int proposal){

	if((nodelevel1+nodelevel2)!=1){
		switch (proposal){
		case 0:
			LDC();				//�U��service interval���Wnode1level�@��weight�A���tconnection interval
			break;	
		case 1:
			IntervalDivide();	//��minimum service interval���Wnode1level�@��weight�A���tconnection interval
			break;
		case 2:
			EIMA();				//��avg current�@��weight�A���tconnection interval
			break;
		case 3:
			Multiple_IOS(IOS_ConnectionInterval);
			break;
		default:
			break;
		}

		//�T�{�C�@node interval���p��10ms
		for(Node* node=Head->nextnd; node!=NULL; node=node->nextnd){
			if(node->eventinterval<Minumum_interval){
				node->eventinterval=Minumum_interval;
			}
		}
	}else{
		//-------------------------------�إ�FrameTable �åBassign connection interval
		double frameid=1;
		FrameTbl=new FrameTable;
		FrameTable* Ftbl=FrameTbl;
	
		for(TDMATable* tbl=TDMA_Tbl; tbl!=NULL; tbl=tbl->next_tbl){
			if(tbl->slot==frameid && tbl->n1->SendNode==Head){
				//Init setting
				Ftbl->id=frameid++;
				Ftbl->Currentflag=false;
				Ftbl->arrival=0;
				Ftbl->ConnNode= tbl->n1;					//Assign node����Frame

				//Waiting time & period setting
				Ftbl->Period=tbl->n1->eventinterval;		//Assign Service interval as period
				Ftbl->Deadline=tbl->n1->eventinterval;		//Assign Service interval as Deadline
				Ftbl->Size=tbl->n1->eventinterval;		//Service inteval�p��connection interval
				Ftbl->ConnNode->eventinterval=Ftbl->Size;	//��snode�W��connection interval
				Ftbl->Utilization=Ftbl->Size/Ftbl->Period;	

				//�إߤU�@��FrameTable
				Ftbl->next_tbl=new FrameTable;
				Ftbl->next_tbl->pre_tbl=Ftbl;
				Ftbl=Ftbl->next_tbl;
			}
		}
		Ftbl->pre_tbl->next_tbl=NULL;
	}
}

/*==============================================
		�C�@node connection interval ���� 10ms
==============================================*/
void EventInterval::Event(){
	for (Node* n=Head->nextnd; n!=NULL; n=n->nextnd){
		node->eventinterval=Minumum_interval;
	}
}

/*==============================================
	Multiple event Energy Efficient Interval

	>���ƦnPacketQueue ��pkt
	>���O�p��bTslot���e��Demand_pkt Supply_pkt
	>Update Tslot
==============================================*/
void EventInterval::MEI(Node * MEINode){

	double Tc=0;			//�̫�o�X��service interval
	double Tslot=0;			//��Ǵ��ծɶ��I(���Demand_pkt v.s. Supply_pkt)			
	double Demand_pkt=0;	//�bTslot���e�һݪ�pkt�ƶq
	double Supply_pkt=0;	//�bTslot���e�t�δ��Ѫ�pkt�ƶq
	bool doneflag=false;	//�P�_node���Ҧ�pkt�H�p�⧹��

	/*------------------------------------------
		�����n�Ҧ�Conn/Adv Node
		�W��connection/advertisement interval
	------------------------------------------*/
	PacketQueue();		//����Ready Queue
	for (Node* n=Head->nextnd; n!=NULL; n=n->nextnd){
		
		//========================Init
		
		Packet *pkt=n->pktQueue;	//pkt�w�ƧǦn (����deadline)
		if(MEINode!=NULL){
			pkt=MEINode->pktQueue;	//�ھڬYnode�]�w
		}

		Tslot=0;
		Demand_pkt=0;
		Supply_pkt=0;
		Tc=pkt->period;		//��Minperiod, �]�w��Tc init
		Tslot=pkt->period;	//��Minperiod, �]�w��Tslot init
		doneflag=false;

		//========================�`�ǼW�[Tslot
		while(doneflag!=true){
			Demand_pkt=0;

			//��X�bTslot���e�һݪ�pkt�ƶq (Packet �ƶq --> Demand_pkt)----Step1
			pkt=n->pktQueue;
			while(pkt->period <= Tslot){
				Demand_pkt=Demand_pkt+(ceil(pkt->load/payload)*ceil(Tslot/pkt->period));	
				pkt=pkt->nodereadynextpkt;
				if(pkt==NULL)
					break;
			}

			//�p��bTslot���e�Ҵ��Ѫ�Service interval----------------------Step2
			Supply_pkt=floor(Tslot/Tc)*double(Maxbuffersize);
			
			//���Demand_pkt & Supply_pkt---------------------------------Step3
			while(Demand_pkt > Supply_pkt){
				Tc--;
				Supply_pkt=floor(Tslot/Tc)*double(Maxbuffersize);
			}
			
			//��sTime slot-----------------------------------------------Step4
			if(pkt!=NULL){
				Tslot=pkt->period;
			}else{
				doneflag=true;
			}
		}	

		//�w�g�p�⧹��node�Ҧ�pkt�Aassign Service Interval to node
		n->eventinterval=Tc;

		//�]�O�ھڬY�@node���p��A�ҥH�u�ݭp��@��
		if(MEINode!=NULL){
			break;			
		}
	}
}

/*===========================
		�����
	��U�Ӱ϶�(interval)
	<arrival -> period> 
step1:��U�϶� ���㪺packet
step2:�U�϶���(packet->load�[�`) ���H (interval) <interval�|��϶����̤jarrival�P�̤jdeadline�B�n�ư��Hassign�L��packet�϶�>
step3:�p��U�϶� rate 
step4:��X�̤jrate , ��b�϶���packet assign ��rate
(��϶��ɭn�N��rate���϶��ɶ�����)

map��
���t�n�C�@packet��rate
===========================*/
void EventInterval::DIF(){
	PacketQueue();

	Packet * DIFpacket;
	map<double,map<double,DIFtable>> Table;	//�G��map ���e�榡��DIFtable map[arrival][deadline]
	double maxarrvial,maxdeadline;			//��̤jDensity�� ���϶�
	double Maxdesity=0;						//��϶��� �̤jDensity
	bool Doneflag=false;					//����assign��rate
	
	while(Doneflag!=true){
		/*============================
			DIF init
			��U�϶�
		============================*/
		DIFpacket=Head->nextnd->pkt;
		while(DIFpacket!=NULL){
			Table[DIFpacket->arrival][DIFpacket->deadline];
		
			for (map<double,map<double,DIFtable>>::iterator a=Table.begin(); a!=Table.end(); a++){
				for(map<double,DIFtable>::iterator p=Table[a->first].begin(); p!=Table[a->first].end(); p++){
					Table[a->first][p->first];
					Table[a->first][p->first].load=0;
					Table[a->first][p->first].length=0;
				}
			}
		
			DIFpacket=DIFpacket->nextpkt;
		}
		/*============================
			�U�϶���(packet->load�[�`)
				   (�϶�����)
				   (�p��U�϶� rate)
		============================*/
		for (map<double,map<double,DIFtable>>::iterator a=Table.begin(); a!=Table.end(); a++){
			for(map<double,DIFtable>::iterator p=Table[a->first].begin(); p!=Table[a->first].end(); p++){
				DIFpacket=Head->nextnd->pkt;

				while(DIFpacket!=NULL){
					//�T�warrival �� deadline�p
					if(a->first < p->first){
						if(a->first <= DIFpacket->arrival && DIFpacket->deadline <= p->first && DIFpacket->rate==0){ //�|��assigh rate�B����϶���
							Packet* tmpDIFpacket;
							double start=a->first;
							double end=p->first;
							
							//��J�϶��B�϶���load�`�� �H�� ���϶�Density
							Table[a->first][p->first].length=p->first - a->first;	//length <deadline-arrival>
							while(start!=end){
								
								tmpDIFpacket=Head->nextnd->pkt;
								while(tmpDIFpacket!=NULL){
									//�ư��b���϶���assign�Lrate��packet
									if(tmpDIFpacket->rate!=0){
										if(start>=tmpDIFpacket->arrival && (start+1)<=tmpDIFpacket->deadline){
											Table[a->first][p->first].length--;
											tmpDIFpacket=NULL;
										}
									}
									if(tmpDIFpacket!=NULL)
										tmpDIFpacket=tmpDIFpacket->nextpkt;
								}
								start++;
							}

							Table[a->first][p->first].load=Table[a->first][p->first].load+DIFpacket->load*((p->first-a->first)/DIFpacket->period);
							Table[a->first][p->first].density=Table[a->first][p->first].load/Table[a->first][p->first].length;
							
							//��X�̤jrate,�ì����϶�
							if(Table[a->first][p->first].density>=Maxdesity){
								maxarrvial=a->first;
								maxdeadline=p->first;
								Maxdesity=Table[a->first][p->first].density;
							}
						}
					}

					DIFpacket=DIFpacket->nextpkt;
				}
			}	
		}
		/*============================
			�b�϶���X�϶�����packet
		��assign rate �� packet->rate�W
		============================*/
		DIFpacket=Head->nextnd->pkt;
		while(DIFpacket!=NULL){
			if(maxarrvial <= DIFpacket->arrival && DIFpacket->deadline <= maxdeadline ){
				//�|��assign rate
				if(DIFpacket->rate==0){
					DIFpacket->rate=Table[maxarrvial][maxdeadline].density;
					
					DIFpacket->rate=(DIFpacket->rate);
				}
			}

			DIFpacket=DIFpacket->nextpkt;
		}
		Maxdesity=0;

		/*==================
			�T�{�Ҧ�packet
			�w�gassign�� rate
		====================*/
		Doneflag=true;
		DIFpacket=Head->nextnd->pkt;
		while(DIFpacket!=NULL){
			if(DIFpacket->rate==0)
				Doneflag=false;
			DIFpacket=DIFpacket->nextpkt;
		}
	}
	
	delete []DIFpacket;
}

/*============================================
				Greedy
		��Xnode���̤pperiod�A
		�N��period ����service interval
============================================*/
void EventInterval::Greedy(){
	for(Node *n=Head->nextnd; n!=NULL; n=n->nextnd){

		//Find the minimum period
		Packet* Minpkt=NULL;
		for(Packet* pkt=n->pkt; pkt!=NULL; pkt=pkt->nodenextpkt){
			if(Minpkt==NULL){
				Minpkt=pkt;
			}else{
				if(Minpkt->period > pkt->period){
					Minpkt=pkt;
				}
			}
		}

		//Assign Service interval
		n->eventinterval=Minpkt->period;
	}
}

/*============================================
				IOS
		��defualt setting���O�H�T�w�ɶ��]�w
============================================*/
void EventInterval::Single_IOS(double ServiceInterval){
	for(Node* n=Head->nextnd; n!=NULL; n=n->nextnd){
		n->eventinterval=ServiceInterval;
	}
}

/*==============================================
			Low Duty Cycle
		�C�@��connection interval��
		�U��Service interval���Wnodelevel1
==============================================*/
void EventInterval::LDC(){
	//-------------------------------Assign��FrameTbl,�u��Connection node��nodelevel1�ӥ�
	short int frameid=1;
	FrameTbl=new FrameTable;
	FrameTable* Ftbl=FrameTbl;

	//-------------------------------�إ�FrameTable �åBassign connection interval
	for(TDMATable* tbl=TDMA_Tbl; tbl!=NULL; tbl=tbl->next_tbl){
		if(tbl->slot==frameid && tbl->n1->SendNode==Head){	//�T�{���� & �e��Master
			//Init setting
			Ftbl->id=frameid++;
			Ftbl->Currentflag=false;
			Ftbl->arrival=0;
			Ftbl->ConnNode= tbl->n1;					//Assign node����Frame

			//Waiting time & period setting
			Ftbl->Period=tbl->n1->eventinterval;		//Assign Service interval as period
			Ftbl->Deadline=tbl->n1->eventinterval;		//Assign Service interval as Deadline
			Ftbl->Size=floor(Ftbl->Period/nodelevel1);	//Service inteval�p��connection interval
			Ftbl->ConnNode->eventinterval=Ftbl->Size;	//��snode�W��connection interval
			Ftbl->Utilization=Ftbl->Size/Ftbl->Period;		

			//�إߤU�@��FrameTable
			Ftbl->next_tbl=new FrameTable;
			Ftbl->next_tbl->pre_tbl=Ftbl;
			Ftbl=Ftbl->next_tbl;
		}
	}
	Ftbl->pre_tbl->next_tbl=NULL;

	//------------------------------------�T�{Frame�ƶq����nodelevel1�h
	if(--frameid>nodelevel1){
		printf("The Frame size is larger than nodelevel1, the FrameTable is error\n");
		system("PAUSE");
	}
	
	//------------------------------------Print �X��T
#ifdef _Showlog
	for(Node* node=Head->nextnd; node!=NULL; node=node->nextnd){
		cout<<	"Node"<<node->id<<"=> "<<
				"Interval="<<node->eventinterval<<", "<<
				"Slot="<<node->color<<", "<<
				"Scan Duaration="<<node->ScanDuration<<", "<<
				"SendNode="<<node->SendNode->id<<endl;
	}
	for(FrameTable* Ftbl=FrameTbl; Ftbl!=NULL; Ftbl=Ftbl->next_tbl){
		cout<<"Frame"<<Ftbl->id<<" =>"<<
			" size="<< Ftbl->Size<<","<<
			" period="<<Ftbl->Period<<endl;
	}
#endif
}
/*==================================
		IntervalDivide
	��̤p��Service interval���Wnodelevel1
	��Cnode��connection interval
==================================*/
void EventInterval::IntervalDivide(){

	double Mininterval=Hyperperiod;	//�̤p��Service interval
	short int frameid=1;
		
	//��̤p��Service interval
	for(Node *node=Head->nextnd; node!=NULL; node=node->nextnd){
		if(node->SendNode==Head && node->eventinterval<Mininterval){
			Mininterval=node->eventinterval;
		}
	}

	//-------------------------------�إ�FrameTable �åBassign connection interval
	FrameTbl=new FrameTable;
	FrameTable* Ftbl=FrameTbl;

	for(TDMATable* tbl=TDMA_Tbl; tbl!=NULL; tbl=tbl->next_tbl){
		if(tbl->slot==frameid && tbl->n1->SendNode==Head){ //�T�{���� & �e��Master
			//Init setting
			Ftbl->id=frameid++;
			Ftbl->Currentflag=false;
			Ftbl->arrival=0;
			Ftbl->ConnNode= tbl->n1;					//Assign node����Frame

			//Waiting time & period setting
			Ftbl->Period=tbl->n1->eventinterval;		//Assign Service interval as period
			Ftbl->Deadline=tbl->n1->eventinterval;		//Assign Service interval as Deadline
			Ftbl->Size=floor(Mininterval/nodelevel1);	//Service inteval�p��connection interval (��Minperiod)
			Ftbl->ConnNode->eventinterval=Ftbl->Size;	//��snode�W��connection interval
			Ftbl->Utilization=Ftbl->Size/Ftbl->Period;		

			//�إߤU�@��FrameTable
			Ftbl->next_tbl=new FrameTable;
			Ftbl->next_tbl->pre_tbl=Ftbl;
			Ftbl=Ftbl->next_tbl;
		}
	}
	Ftbl->pre_tbl->next_tbl=NULL;

	//------------------------------------�T�{Frame�ƶq����nodelevel1�h
	if(--frameid>nodelevel1){
		printf("The Frame size is larger than three, the FrameTable is error\n");
		system("PAUSE");
	}
	
	//-------------------------------Scan duration �p��
	/*
	for(Node* node=Head->nextnd; node!=NULL; node=node->nextnd){
		devicenum=0;
		MaxAdvinter=0;
		if(node->SendNode==Head){
			//����Device �ƶq & �����̤j�s�����Z
			Node *BelongNode=Head->nextnd;
			while(BelongNode!=NULL){
				if(node==BelongNode->SendNode){
					devicenum++;			
					if(BelongNode->eventinterval>MaxAdvinter){
						MaxAdvinter=BelongNode->eventinterval;
					}
				}
				BelongNode=BelongNode->nextnd;
			}

			//�p��Scan duration
			node->ScanDuration=node->SCAN_Compute(	node->ScanWin,
													node->ScanInter,
													MaxAdvinter,
													devicenum);
		}else{
			node->ScanDuration=0;
		}
	}
	*/
	//---------------------------------Print �X��T
	
	#ifdef _Showlog
	for(Node* node=Head->nextnd; node!=NULL; node=node->nextnd){
		cout<<	"Node"<<node->id<<"=> "<<
				"Interval="<<node->eventinterval<<", "<<
				"Slot="<<node->color<<", "<<
				"Scan Duaration="<<node->ScanDuration<<", "<<
				"SendNode="<<node->SendNode->id<<endl;
	}
	for(FrameTable* Ftbl=FrameTbl; Ftbl!=NULL; Ftbl=Ftbl->next_tbl){
		cout<<"Frame"<<Ftbl->id<<" =>"<<
			" size="<< Ftbl->Size<<","<<
			" period="<<Ftbl->Period<<endl;
	}
	#endif
}

/*==================================
	Energy Efficeincy Multilple Access
	
	weight�ӨM�w�n���h��percent��
	service interval
	(weight���p���avg current)
==================================*/
void EventInterval::EIMA(){

	short int frameid=1;
	double Sum_weight=0;

	//�Y��interval�j��4�� (400 unit��10ms)�A�n���escaling�ҥHunit�|��1ms
	for(Node* n=Head->nextnd; n!=NULL; n=n->nextnd){
		if(n->eventinterval>400){
			unit=0.001;		
		}
	}

	//����Node avg current��interval�ե�
	for(Node* n=Head->nextnd; n!=NULL; n=n->nextnd){
		
		//=======================�p��Service interval��average current ----------------Weight v1
		double avgcurrent=(((I_notify*Time_notify)+(I_sleep*(n->eventinterval*unit-Time_notify)))/(n->eventinterval*unit));
		
		//=======================�p��Service interval��sequence average current -------Weight v2
		double Qc=0, Qt=0;
		double totalcnt=0;
		double Seqavgcurrent=0;

		Qc=(Hyperperiod/n->eventinterval)*I_notify*Time_notify;			//��@event�ҳy����current consumption
		for(Packet* pkt=n->pkt; pkt!=NULL; pkt=pkt->nodenextpkt){
			double count=((Hyperperiod/pkt->period)*ceil(pkt->load/payload));
			totalcnt+=count;
		}
		Qt=(totalcnt-(Hyperperiod/n->eventinterval))*I_Tran*Time_Tran;	//�s��event�ҳy����current consumption
		Seqavgcurrent=((Qc+Qt)+I_sleep*(Hyperperiod*unit-(Qc/I_notify)-(Qt/I_Tran))) / (Hyperperiod*unit);

		//=======================�p��total weight
		n->EIMA_avgcurrent=avgcurrent;
		Sum_weight=Sum_weight+n->EIMA_avgcurrent;
	}

	//-------------------------------�إ�FrameTable �åBassign connection interval
	FrameTbl=new FrameTable;
	FrameTable* Ftbl=FrameTbl;
	
	for(TDMATable* tbl=TDMA_Tbl; tbl!=NULL; tbl=tbl->next_tbl){
		if(tbl->slot==frameid && tbl->n1->SendNode==Head){
			//Init setting
			Ftbl->id=frameid++;
			Ftbl->Currentflag=false;
			Ftbl->arrival=0;
			Ftbl->ConnNode= tbl->n1;					//Assign node����Frame

			//Waiting time & period setting
			Ftbl->Period=tbl->n1->eventinterval;		//Assign Service interval as period
			Ftbl->Deadline=tbl->n1->eventinterval;		//Assign Service interval as Deadline
			Ftbl->Size=floor(((tbl->n1->EIMA_avgcurrent)/Sum_weight)
						* tbl->n1->eventinterval);		//Service inteval�p��connection interval
			Ftbl->ConnNode->eventinterval=Ftbl->Size;	//��snode�W��connection interval
			Ftbl->Utilization=Ftbl->Size/Ftbl->Period;	

			//�إߤU�@��FrameTable
			Ftbl->next_tbl=new FrameTable;
			Ftbl->next_tbl->pre_tbl=Ftbl;
			Ftbl=Ftbl->next_tbl;
		}
	}
	Ftbl->pre_tbl->next_tbl=NULL;

	/*=======================================
			�[�JDemand bound
		��<�̤pperiod��size> & <�̤jsize>
		�Y<�̤pperiod��size> + <�̤jsize>�j�� <�̤pperiod>
		�C�@connection interval�վ㬰<�̤pperiod���P2>
	=======================================*/
	if(EIMADemand_flag){
		double Minperiod_size=0;
		double Minperiod_period=-1;
		double _Maxsize=0;
		for(FrameTable* Ftbl=FrameTbl; Ftbl!=NULL; Ftbl=Ftbl->next_tbl){
			//Find size of minimum period
			if(Minperiod_period==-1){
				Minperiod_period=Ftbl->Period;
				Minperiod_size=Ftbl->Size;
			}else{
				if(Ftbl->Period<Minperiod_period){
					Minperiod_period=Ftbl->Period;
					Minperiod_size=Ftbl->Size;
				}
			}

			//Find max size
			if(Ftbl->Size>_Maxsize){
				_Maxsize=Ftbl->Size;
			}
		}

		if((_Maxsize+Minperiod_size)>Minperiod_period){
			for(FrameTable* Ftbl=FrameTbl; Ftbl!=NULL; Ftbl=Ftbl->next_tbl){
				Ftbl->Size=Minperiod_period/2;
				Ftbl->ConnNode->eventinterval=Ftbl->Size;	//��snode�W��connection interval
			}	
		}
	}

	//------------------------------------�T�{Frame�ƶq����nodelevel1�h
	if(--frameid>nodelevel1){
		printf("The Frame size is larger than three, the FrameTable is error\n");
		system("PAUSE");
	}

	//---------------------------------Print �X��T
#ifdef _Showlog
	for(Node* node=Head->nextnd; node!=NULL; node=node->nextnd){
		cout<<	"Node"<<node->id<<"=> "<<
				"Interval="<<node->eventinterval<<", "<<
				"Slot="<<node->color<<", "<<
				"Scan Duaration="<<node->ScanDuration<<", "<<
				"SendNode="<<node->SendNode->id<<endl;
	}
	for(FrameTable* Ftbl=FrameTbl; Ftbl!=NULL; Ftbl=Ftbl->next_tbl){
		cout<<"Frame"<<Ftbl->id<<" =>"<<
			" size="<< Ftbl->Size<<","<<
			" period="<<Ftbl->Period<<endl;
	}
#endif
}


void EventInterval::Multiple_IOS(double ConnectionInterval){
	for(Node* n=Head->nextnd; n!=NULL; n=n->nextnd){
		n->eventinterval=ConnectionInterval;
	}

	//-------------------------------�إ�FrameTable �åBassign connection interval
	short int frameid=1;
	FrameTbl=new FrameTable;
	FrameTable* Ftbl=FrameTbl;
	
	for(TDMATable* tbl=TDMA_Tbl; tbl!=NULL; tbl=tbl->next_tbl){
		if(tbl->slot==frameid && tbl->n1->SendNode==Head){
			//Init setting
			Ftbl->id=frameid++;
			Ftbl->Currentflag=false;
			Ftbl->arrival=0;
			Ftbl->ConnNode= tbl->n1;					//Assign node����Frame

			//Waiting time & period setting
			Ftbl->Period=tbl->n1->eventinterval;		//Assign Service interval as period
			Ftbl->Deadline=tbl->n1->eventinterval;		//Assign Service interval as Deadline
			Ftbl->Size=tbl->n1->eventinterval;		//Service inteval�p��connection interval
			Ftbl->ConnNode->eventinterval=Ftbl->Size;	//��snode�W��connection interval
			Ftbl->Utilization=Ftbl->Size/Ftbl->Period;	

			//�إߤU�@��FrameTable
			Ftbl->next_tbl=new FrameTable;
			Ftbl->next_tbl->pre_tbl=Ftbl;
			Ftbl=Ftbl->next_tbl;
		}
	}
	Ftbl->pre_tbl->next_tbl=NULL;
}
/*=====================================
	��ݭnScan ��Conn Node
	��Scan duration �p��A�H��Tc���s�p��
=====================================*/
void EventInterval::IntervalReassign(){
	double MaxAdvinter=0;	//�̪��s�����Z
	short int devicenum=0;	//Adv Device �ƶq

	for(Node *node=Head->nextnd; node!=NULL; node=node->nextnd){
		if(node->SendNode==Head){
			MaxAdvinter=0;
			devicenum=0;

			//����Device �ƶq & �����̤j�s�����Z
			for(Node *AdvNode=Head->nextnd; AdvNode!=NULL; AdvNode=AdvNode->nextnd){
				if(node==AdvNode->SendNode){
					AdvNode->pkt->node=node;	//�D�n�O�n�N����node�ର����Conn Node�A�ت����bPacketQueue�� �n��node�Wreadynextpkt�w�ơA�BMEI���s�p��
												//��������n�]�w�^�� (Node�W��nodenextpkt�å�����)
					devicenum++;			
					if(AdvNode->eventinterval>MaxAdvinter){
						MaxAdvinter=AdvNode->eventinterval;
					}
				}
			}

			if(devicenum>0){
				//�p��Scan duration (int ScanWin, int ScanInter, int AdvInter, int device)
				node->ScanDuration=node->SCAN_Compute(	node->ScanWin,
														node->ScanInter,
														MaxAdvinter,
														devicenum);
				node->EXEScanDuration=node->ScanDuration;
				node->ScanFlag=false;

				MEI(node);

				for(Node *AdvNode=Head->nextnd; AdvNode!=NULL; AdvNode=AdvNode->nextnd){
					if(node==AdvNode->SendNode){
						AdvNode->pkt->node=AdvNode;
					}
				}
			}
		}
	}
}

void EventInterval::Rate_TO_Interval(int defaultMinperiod){
	//�P�_�O�_�ݭn����Service interval
	double exconnectioninterval=Connectioninterval;
	Connectioninterval=0;
	Packet *tmppkt=Buffer->pkt;
	double Allload=Buffer->load;
	double Maxrate=0;
	double Minperiod;

	while(tmppkt!=NULL){
		if(tmppkt->rate > Maxrate){
			if(tmppkt->exeload <= payload){
				Connectioninterval=Connectioninterval+(tmppkt->exeload/tmppkt->rate);
				Allload=Allload-tmppkt->exeload;
			}else{
				Connectioninterval=Connectioninterval+(payload/tmppkt->rate);
				Allload=Allload-payload;
			}
		}
		tmppkt=tmppkt->buffernextpkt;
	}
		
	if(Connectioninterval<1){
		Connectioninterval=exconnectioninterval;//�]�w�����e��interval
	}else{
		Connectioninterval=floor(Connectioninterval);
	}

	//�P�_Connectioninterval�O�_�j��Minperiod
	if(Connectioninterval>defaultMinperiod)
		Connectioninterval=defaultMinperiod;

}

