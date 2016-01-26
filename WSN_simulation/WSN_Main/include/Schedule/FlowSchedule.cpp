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

#include "../Struct/WSNFile.h"
#include "../Struct/WSNStruct.h"
#include "../Algorithm/ConnInterval.h"
#include "FlowSchedule.h"
#include "../Struct/WSNEnergy.h"

#undef  _ShowLog

using namespace std;

/*==========================
	�P�_Write-Request��k
==========================*/
void Schedule(int WRSche, int ServiceInterval){
	
	CheckPkt();	//�T�{���S��miss
	/*==========================
			�P�_�O��@node
			�άOmulti node
	==========================*/
	if((nodelevel1+nodelevel2)!=1){
		switch (WRSche){
		case 0:
			NPEDF();
			break;
		case 1:
			RoundRobin();
			break;
		case 2:
			EIF();
			break;
		case 3:
			Polling();
			break;
		}

		Write_Request();
	}else{
		SingleNodeSchedule(ServiceInterval);
	}
}

/*==========================================
			Arrange Queue
{ReadyQ & WaitQ is assign by pkt->readyflag}

	���إ��`total��Packet Queue
	�b���t���U�۪�node
==========================================*/
void PacketQueue(){
	/*--------------------------------
		Init ready linklist
	--------------------------------*/
	ReadyQ->readynextpkt=NULL;		//�NGlobal ReadyQ�U�@ready��NULL

	for(Node *n=Head->nextnd; n!=NULL; n=n->nextnd){
		n->pktQueue=NULL;
	}
	for(Packet* pkt=Head->nextnd->pkt; pkt!=NULL; pkt=pkt->nextpkt){
		pkt->readynextpkt=NULL;		//�U�@ready�]�w��NULL
		pkt->readyprepkt=NULL;		//�W�@ready�]�w��NULL
		pkt->nodereadynextpkt=NULL;
		pkt->nodereadyprepkt=NULL;

		pkt->searchdone=0;			//�|����L
	}
	
	/*--------------------------------
			Setting Global Queue
	--------------------------------*/
	ReadyQ_overflag=0;
	while(!ReadyQ_overflag){
		
		/*--------------------------
		��̤pdeadline pkt��packet
		--------------------------*/
		packet=NULL;
		for(Packet* pkt=Head->nextnd->pkt; pkt!=NULL; pkt=pkt->nextpkt){
			//�|��search�L
			if(pkt->searchdone==0){
				//assign �̤pdeadline pkt��packet
				if(packet==NULL){
					packet=pkt;
				}else{
					if(pkt->deadline < packet->deadline){
						packet=pkt;
					}
				}	
			}
		}
		
		/*--------------------------
		�N�̤pDeadline��JReadyQ��
		�üаO�w�M��L(searchdone=1)
		--------------------------*/
		if(packet!=NULL){
			packet->searchdone=1;

			if(Timeslot>=packet->arrival){
				packet->readyflag=1;			//�ʥ] Arrival
				packet->node->arrival_flag=1;	//node �t��arrival flag
			}else{
				packet->readyflag=0;//�ʥ] �|��Arrival
			}

			//Assign ��JReadyQ
			for(Packet* Rpkt=ReadyQ; Rpkt!=NULL; Rpkt=Rpkt->readynextpkt){
				if(Rpkt->readynextpkt==NULL){
					Rpkt->readynextpkt=packet;
					packet->readyprepkt=Rpkt;
					packet->readynextpkt=NULL;
				}
			}
			
		}else{
			ReadyQ_overflag=1;
		}

	}
	
	/*--------------------------
		�إߦU��node�W��
		ready packet queue
	--------------------------*/
	
	for(Packet* pkt=ReadyQ->readynextpkt; pkt!=NULL; pkt=pkt->readynextpkt){ //��packet priority
		
		//pkt����node
		node=pkt->node;
		
		//��node->pktQueue �̫�@��
		if(node->pktQueue==NULL){
			node->pktQueue=pkt;			
			pkt->nodereadynextpkt=NULL;
			pkt->nodereadyprepkt=NULL;
		}else{
			Packet *tmpreadypkt=node->pktQueue;

			while(tmpreadypkt->nodereadynextpkt!=NULL){
				tmpreadypkt=tmpreadypkt->nodereadynextpkt;
			}

			tmpreadypkt->nodereadynextpkt=pkt;				
			pkt->nodereadynextpkt=NULL;
			pkt->nodereadyprepkt=tmpreadypkt;
		}
	}
}

/*=========================
	Admission Control
	(�p��쥻��Tc �P 
	TDMA �]���y�������j)
=========================*/
void Schedulability(){
	bool reassign=true;
	double BlockingTime=0;
	double minperiod=10000000;
	double maxperiod=0;
	double U_bound=0;
	double sub=1000000000;
	FrameTable *EndFrame;

	for (FrameTable* Ftbl=FrameTbl; Ftbl!=NULL; Ftbl=Ftbl->next_tbl){
		if(Ftbl->Size>BlockingTime){
			BlockingTime=Ftbl->Size;
		}
		if(Ftbl->Period>maxperiod){
			maxperiod=Ftbl->Period;
		}
		if(minperiod>Ftbl->Period){
			minperiod=Ftbl->Period;
			EndFrame=Ftbl;
		}
	}

	while(reassign){
		reassign=false;
		while(maxperiod!=EndFrame->Period){

			//��M�n���Frame (����j��minperiod, �ۮt�̤p��)
			sub=100000000;
			for(FrameTable* Ftbl=FrameTbl; Ftbl!=NULL;Ftbl=Ftbl->next_tbl){
				if(Ftbl->Period>minperiod){
					if((Ftbl->Period-minperiod)<sub){
						sub=(Ftbl->Period-minperiod);
						EndFrame=Ftbl;
					}
				}
			}
			minperiod=EndFrame->Period;

			//�p��U bound
			for (FrameTable* Ftbl=FrameTbl; Ftbl!=NULL; Ftbl=Ftbl->next_tbl){
				if(Ftbl->Period<=minperiod){
					if(BlockingTime!=Ftbl->Size)
						U_bound=U_bound+(Ftbl->Size/Ftbl->Period);
				}
			}
			U_bound=U_bound+BlockingTime/minperiod;
		
			//�ݬO�_�ischedulable
			if(U_bound>=1){
				reassign=true;
			}
		}
		/*=================================
				���s�p��Event interval
		=================================*/
		if(reassign){
			//��̤j��Size���ק�
			for(FrameTable* Ftbl=FrameTbl; Ftbl!=NULL; Ftbl=Ftbl->next_tbl){
				if(Ftbl->Size==BlockingTime){
					BlockingTime--;
					Ftbl->Size--;
					Ftbl->ConnNode->eventinterval--;
				}
			}
		}
	}
	#ifdef _Showlog
		cout<<"Reassign Done"<<endl;
	#endif
}

/*=======================================
			Check every pkt
			Have to Meet Deadline
=======================================*/
void CheckPkt(){
	/*
	if(Timeslot==200){
		system("PAUSE");
	}
	*/

	for(Packet *pkt=Head->nextnd->pkt; pkt!=NULL; pkt=pkt->nextpkt){
		if( pkt->CMP_D<=Timeslot && pkt->deadline<=Timeslot){
			if(pkt->CMP_D<pkt->deadline){
				pkt->CMP_D=pkt->deadline;
			}
			pkt->CMP_D=pkt->CMP_D+pkt->period;

			pkt->Miss_count++;	

			Schdulefile<<Timeslot<<" ";
			Schdulefile<<"Node"<<pkt->nodeid<<" (PKT"<<pkt->id<<" Miss deadline"<<" Deadline "<<pkt->deadline<<" CMP_D:"<<pkt->CMP_D<<")";
			Schdulefile<<endl;

			Meetflag=false;
		}
	}
}

/*============================================
		�Nnode�W��Buffer���ǿ�
		�w��ثe��Buffer���ǿ�
		�i�J�@���u�|�ǿ�payload�j�p (20bytes)
============================================*/
void BLE_EDF(Node *node){
		CheckPkt(); //�]�ܦ��i��b���i�Jconnection event�õ����y��miss��� (�bnotify�ɦ���s�ѼơATimeslot�����[3�F)

		/*---------------------------------------------
				�P�_Flow �O�_��NULL
				�Y���ʥ]�h�i��ǿ�
				(�]�t�P�_�O�_����)
		---------------------------------------------*/
		PacketBuffer *Buffer=node->NodeBuffer;

		//Debug not normal situation
		if(Buffer->load<0){
			printf("Buffer->load<0\n");
			system("PAUSE");
		}

		//Transmission
		if(Buffer->load!=0){
			totalevent++;

			Node *tmpnode=Buffer->pkt->node;

			//cout<<"Time slot:"<<Timeslot<<" Node:"<<Buffer->pkt->node->id<<",";
			Schdulefile<<"Time slot:"<<Timeslot;
			
			//=============================================����ǿ�(�����p��payload�P�j��payload)
			packet=Buffer->pkt;
			Pktsize=Buffer->pktsize;
			if(packet->exeload<payload){
				Buffer->load=Buffer->load-packet->exeload;
				packet->exeload=0;

				packet->node->State="Transmission";
				packet->State="Transmission";		//�ǿ骬�A
			}else{
				Buffer->load=Buffer->load-payload;
				packet->exeload=packet->exeload-payload;

				packet->node->State="Transmission";
				packet->State="Transmission";		//�ǿ骬�A
			}

			Schdulefile<<" NP:"<<packet->node->id<<","<<packet->id;

			//=============================================�ǧ�,���U�@packet
			if(packet->exeload==0){

				packet->meetlatency_cnt++;
				packet->meetlatency=packet->meetlatency+(Timeslot - packet->arrival);

				//�P�_�O�_miss deadline
				if((Timeslot)>=packet->deadline){
#ifdef _ShowLog
					cout<<"(PKT"<<packet->id<<" Miss deadline"<<" Deadline "<<packet->deadline<<")";
#endif
					Schdulefile<<"(PKT"<<packet->id<<" Miss deadline"<<" Deadline "<<packet->deadline<<")";
					

					//packet->latency=packet->latency+(Timeslot-packet->deadline);//Record latency
					packet->latency=packet->latency+(Timeslot-packet->arrival);//Record latency
					
					Meetflag=false;
					//system("PAUSE");
				}

				packet->readyflag=0;
				packet->exeload=packet->load;
				packet->arrival=packet->deadline;
				packet->deadline=packet->deadline+packet->period;
				//packet->CMP_D=packet->CMP_D+packet->period;

				packet->exehop=packet->hop;	

				//Buffer���e����
				packet=packet->buffernextpkt;
				Buffer->pkt=packet;
			}

			//=============================================�T�{Buffer�S��packet,�Narrival_flag�]��false
			if(Buffer->load==0){
				tmpnode->arrival_flag=0;
			}
			
			Schdulefile<<endl;
		}else{
			Buffer=NULL;
		}
}

/*========================================
		��SettingNode���nNodeBuffer�]�w
		Refresh SettingNode->NodeBuffer
========================================*/
void NodeBufferSet(Node * SettingNode){
	
	Node *Bufnode=SettingNode;

	//-------------------------------------------------------------�惡node��NodeBuffer�����t
	if(Bufnode==SettingNode){
		Bufnode->NodeBuffer->pktsize=0;	//����buffer�����ʥ]�q�M��
		Bufnode->NodeBuffer->load=0;	//load�M��
		Bufnode->NodeBuffer->pkt=NULL;	//pkt�M��
		
		Packet *tmpbufferpkt;
		int tmpsize=0;					//Buffer �ثepkt size
		int intervalsize=0;				//�i�몺packet�q

		packet=Bufnode->pktQueue;
		if(Bufnode->NodeBuffer->pktsize!=0){
			while(packet->readyflag!=1){			
				packet=packet->nodereadynextpkt;
			}		
		}

		/*-------------------------------------------
				Assign node->NodeBuffer��T
					pktsize �̦h��Maxbuffersize
					pkt->buffernextpkt
					load �̦h��Maxbuffersize*payload
		-------------------------------------------*/
		while(Bufnode->NodeBuffer->pktsize<Maxbuffersize && packet!=NULL){

			if(packet->readyflag!=1){			//�|��ready�A�������U�@packet
				packet=packet->nodereadynextpkt;
			}else{
			
				//�T�{packet���s�b��Buffer��
				bool existflag=false;
				for(Packet* pkt=Bufnode->NodeBuffer->pkt; pkt!=NULL; pkt=pkt->buffernextpkt){
					if(pkt==packet){
						existflag=true;
					}
				}

				//��JBuffer
				if(existflag!=true){
					//-----------------------------------------��JBuffer link list
					if(Bufnode->NodeBuffer->pktsize == 0){
						Bufnode->NodeBuffer->pkt=packet;
						tmpbufferpkt=Bufnode->NodeBuffer->pkt;
						tmpbufferpkt->buffernextpkt=NULL;
					}else {
						tmpbufferpkt->buffernextpkt=packet;
						tmpbufferpkt=packet;
						tmpbufferpkt->buffernextpkt=NULL;
					}
					
					//-----------------------------------------�]�wBuffer size
					tmpsize=Bufnode->NodeBuffer->pktsize;
					if((Bufnode->NodeBuffer->pktsize+ceil(packet->exeload/payload))<=Maxbuffersize){
						Bufnode->NodeBuffer->pktsize=(Bufnode->NodeBuffer->pktsize+ceil(packet->exeload/payload));
					}else{
						Bufnode->NodeBuffer->pktsize=Maxbuffersize;
					}
					intervalsize=Bufnode->NodeBuffer->pktsize-tmpsize;//�i�몺packet�q

					//-----------------------------------------�p��Buffer load
					if(packet->exeload>payload){
						double tmpload=packet->exeload;
						while(intervalsize!=0){
							if(tmpload>payload){
								Bufnode->NodeBuffer->load=Bufnode->NodeBuffer->load+payload;
							}else{
								Bufnode->NodeBuffer->load=Bufnode->NodeBuffer->load+tmpload;
							}
							tmpload=tmpload-payload;
							intervalsize--;
						}

					}else{
						Bufnode->NodeBuffer->load=Bufnode->NodeBuffer->load+packet->exeload;
					}
				}

				//���U�@��packet (����node �W pkt priority)
				packet=packet->nodereadynextpkt;
			}
		}

	}
}

/***********************************************
	����EIMA�W��node �ǿ�q��(�̷�EDF�覡)
	��node���ǿ�

	node->State���Q�q����node
	node->EvtArrival��event ��F
***********************************************/
void NPEDF(){
	
	/*----------------------------------------------
		Head��ConnNode�U���O�A�b�p��᪺FrameSize��
		FrameSize�Y�p�⧹���ߧY���U�@TDMA��frame����
		�惡frame�W��node��state�W���ܤ�
	----------------------------------------------*/
	
	if(Head->FrameSize<=0){ //(Head->RecvNode�ثe�O���צ�A������n��FrameSize���ק�)
		
		//��̤pFrameTable��deadline <Arrival>
		FrameTable* Work_tbl=NULL;
		for(FrameTable *Ftbl=FrameTbl; Ftbl!=NULL; Ftbl=Ftbl->next_tbl){
			if(Timeslot >= Ftbl->arrival){
				if(Work_tbl==NULL){
					Work_tbl=Ftbl;
				}else{
					if((Work_tbl->Deadline > Ftbl->Deadline) ){
						Work_tbl=Ftbl;
					}
				}
			}
		}
        
        if(Work_tbl!=NULL){
            Work_tbl->arrival=Work_tbl->arrival+Work_tbl->Period;
            Work_tbl->Deadline=Work_tbl->Deadline+Work_tbl->Period;
			Work_tbl->ConnNode->State="Notify";

			Head->FrameSize=Work_tbl->Size;            
            Head->FrameSize--;
        }
	}else{
		Head->FrameSize--;		
	}

	/*----------------------------------------------
				����Scan duration
	----------------------------------------------*/
	for(Node *n=Head->nextnd; n!=NULL; n=n->nextnd){
		if(n->SendNode!=Head && n->ScanFlag && n->SendNode!=Head->RecvNode){ //�ǿ�Node����Head, n�{�bscan �B�ǿ�Node�{�b�S�ǿ�
			if(n->EXEScanDuration>=0){
				n->EXEScanDuration--;
			}
		}
	}

}

/***********************************************
	�����nHead���node��state �ഫcmd
	event arrival��NodeBufferSet�A�ݳ]�w��eEvtArrival�Ӱ����� ����ۤv�ǿ�ɶ��I��F
	�ǿ�]�w������A�NEvtArrival ����
***********************************************/
void RoundRobin(){
	/*----------------------------------------------
		Head��ConnNode�U���O�A�b�p��᪺FrameSize��
		FrameSize�Y�p�⧹���ߧY���U�@TDMA��frame����
		�惡frame�W��node��state�W���ܤ�
	----------------------------------------------*/
	
	if(Head->FrameSize<=0 ){ //(Head->RecvNode�ثe�O���צ�A������n��FrameSize���ק�)
		bool findflag=false;

		//��̤pFrameTable��deadline
		FrameTable *Work_tbl=FrameTbl;
		for(FrameTable *Ftbl=FrameTbl; Ftbl!=NULL; Ftbl=Ftbl->next_tbl){
			if(Ftbl->Currentflag==true && findflag==false){
				if(Ftbl->next_tbl!=NULL){
					Work_tbl=Ftbl->next_tbl;
				}else{
					Work_tbl=FrameTbl;
				}
				findflag=true;
			}

			Ftbl->Currentflag=false;
		}

		Work_tbl->Currentflag=true;
		Work_tbl->ConnNode->State="Notify";
		Head->FrameSize=Work_tbl->Size;

		Work_tbl->Deadline=Work_tbl->Deadline+Work_tbl->Period;

		Head->FrameSize--;
	}else{
		Head->FrameSize--;
	}
	
}

/***********************************************
	����EIMA�W��node �ǿ�q��(�̷�EDF�覡)
	��node���ǿ�

	node->State���Q�q����node		(�b����node�� master�N�|�n�D)
	node->EvtArrival��event ��F		(slave connection evt��F)

	(Frame Deadline assignment is connection interval between tx event to next event.)
	<D=timeslot+Tc, at event arrival>
***********************************************/
void EIF(){
	/*----------------------------------------------
		Head��ConnNode�U���O�A�b�p��᪺FrameSize��
		FrameSize�Y�p�⧹���ߧY���U�@TDMA��frame����
		�惡frame�W��node��state�W���ܤ�
	----------------------------------------------*/
	
	if(Head->FrameSize<=0){ //(Head->RecvNode�ثe�O���צ�A������n��FrameSize���ק�)
		
		//��Crtical Frame (Period�̤p) <No matter Arrival>
		FrameTable* CrticalFrame=FrameTbl;
		for(FrameTable* Ftbl=FrameTbl; Ftbl!=NULL; Ftbl=Ftbl->next_tbl){
			if(CrticalFrame->Deadline > Ftbl->Deadline){
				CrticalFrame=Ftbl;
			}
		}

		//��̤pFrameTable��deadline <Arrival>
		FrameTable* Work_tbl=NULL;
		for(FrameTable *Ftbl=FrameTbl; Ftbl!=NULL; Ftbl=Ftbl->next_tbl){
			if(Timeslot >= Ftbl->arrival){
				if(Work_tbl==NULL){
					Work_tbl=Ftbl;
				}else{
					if((Work_tbl->Deadline > Ftbl->Deadline) ){
						Work_tbl=Ftbl;
					}
				}
			}
		}

		//
		if(Work_tbl!=NULL){
			
			if(CrticalFrame!=Work_tbl){
				//if(CrticalFrame->Deadline < (int(Work_tbl->Size)+int(CrticalFrame->Size)+Timeslot)){
				if(floor(CrticalFrame->Deadline/CrticalFrame->Size)*CrticalFrame->Size < (int(Work_tbl->Size)+Timeslot)){
					if(Timeslot >= CrticalFrame->arrival){
						Work_tbl=CrticalFrame;
					}else{
						Work_tbl=NULL;
					}
				}
			}
			
			if(Work_tbl!=NULL){
				Work_tbl->arrival=Work_tbl->arrival+Work_tbl->Period;
				Work_tbl->ConnNode->State="Notify";
				Head->FrameSize=Work_tbl->Size;
				Work_tbl->Deadline=Work_tbl->Deadline+Work_tbl->Period;

				Head->FrameSize--;
			}
		}
		//Work_tbl->Deadline=Timeslot+Work_tbl->Period;
	}else{
		Head->FrameSize--;		
	}
	
}

/*=========================================
			��@node�W��schedule
=========================================*/
void SingleNodeSchedule(int ServiceInterval){
	Node *n=Head->nextnd;
	
	//-------------------------------------Callback Timer Trigger
	switch(ServiceInterval){
	case 2: //------------DIF
		DIFCB();
		break;
	case 3:	//------------Lazy
		LazyIntervalCB();
		break;
	}
	
	//---------------------------------------�P�_connection event�O�_arrival
	if(Timeslot % int(n->eventinterval)==0){
		if(ServiceInterval==3){ //------------Lazy
			LazyOnWrite();		//�Y��OnWrite�h�P�_
		}

		NodeBufferSet(n);
		n->EvtArrival=true;
		
		Head->RecvNode=n;		//Head->RecvNode����
		n->State="Transmission";
		Timeslot=Timeslot+2;

	}else{
		n->EvtArrival=false;
	}

	//---------------------------------------�ǿ�
	if(n->State=="Transmission"){
		BLE_EDF(n);				//��n���ǿ�
	}
	Node_EnergyState(n);		//�p��n��Energy

	//---------------------------------------�ǿ駹��
	if(n->NodeBuffer->load==0){
		Head->RecvNode=NULL;
		n->State="Sleep";
	}
}

/*=========================================
		Lazy Decrease Alorithm
=========================================*/
void LazyOnWrite(){
	double Rate_data,Rate_BLE;
	double load=0,min_deadline=-1;
	double load_minperiod=-1,minperiod=-1;

	for(Packet *pkt=Head->nextnd->pkt; pkt!=NULL; pkt=pkt->nextpkt){
		if(pkt->readyflag){
			load=load+pkt->exeload;
			if(min_deadline==-1 || pkt->deadline<min_deadline){
				min_deadline=pkt->deadline;
			}
		}

		if(minperiod==-1 || pkt->period<minperiod){
			minperiod=pkt->period;
			load_minperiod=pkt->load;
		}
	}
	Rate_data=load/(min_deadline-Timeslot);							//�p�⦹�ɪ�data rate
	Rate_BLE=(payload*Maxbuffersize)/(Head->nextnd->eventinterval);	//�p�⦹�ɪ�BLE rate

	/*
	if(Rate_data>=Rate_BLE){
		Head->nextnd->eventinterval=10;
		
		Callbackclock=EXECBclock;		//Reset timer
	}
	*/
	
	if(Rate_data>=Rate_BLE){
		Head->nextnd->eventinterval=10;
		
		Callbackclock=EXECBclock;		//Reset timer
	}

}
/*=========================================
		Timer Callback
		���sassign connection interval
=========================================*/
void LazyIntervalCB(){
	if(Callbackclock==0){
		double MaxRate_data,MinRate_BLE,Rate_BLE;
		double Rate_CB,Rate_reduce;
		double load=0,min_deadline=-1,long_period=-1, min_period=-1, load_minperiod;

		for(Packet *pkt=Head->nextnd->pkt; pkt!=NULL; pkt=pkt->nextpkt){
		
			load=load+pkt->load;
			if(min_deadline==-1 || pkt->deadline<min_deadline){
					min_deadline=pkt->deadline;
			}
			if(long_period==-1 || pkt->period>long_period){
				long_period=pkt->period;
			}
			if(min_period==-1 || pkt->period<min_period){
				min_period=pkt->period;
				load_minperiod=pkt->load;
			}
		}

		//MaxRate_data=load/(min_deadline-Timeslot);						//�p�⦹�ɪ�data rate
		MaxRate_data=(payload*Maxbuffersize)/(min_period);						//�p�⦹�ɪ�data rate
		Rate_CB=(payload*Maxbuffersize)/EXECBclock;						//�p�⦹�ɪ�BLE rate
		MinRate_BLE=(payload*Maxbuffersize)/(long_period);				//�p�⦹�ɪ�BLE rate
		Rate_BLE=(payload*Maxbuffersize)/(Head->nextnd->eventinterval);	//�p�⦹�ɪ�BLE rate

		Rate_reduce=dec_cof*(MaxRate_data-MinRate_BLE);

		if(Rate_CB<(Rate_BLE-Rate_reduce) && Rate_reduce>0){
			Head->nextnd->eventinterval=(payload*Maxbuffersize)/(Rate_BLE-Rate_reduce);
		}

		if(Timeslot==0){
			Head->nextnd->eventinterval=10;
		}else if(Head->nextnd->eventinterval<10){
			Head->nextnd->eventinterval=10;
		}

		//Reset timer
		if(EXECBclock<(Head->nextnd->eventinterval*overheadcount)){
			Callbackclock=EXECBclock;
		}else{
			Callbackclock=Head->nextnd->eventinterval*overheadcount;
		}
	}else{
		Callbackclock--;
	}
}

void DIFCB(){
	if(Callbackclock==0){
		double MaxRate=0;
		double MinPeriod=-1;
		for(Packet *pkt=Head->nextnd->pkt; pkt!=NULL; pkt=pkt->nextpkt){
			//��ready�n���̤jrate
			
			if(pkt->readyflag){
				if(pkt->rate>MaxRate){
					MaxRate=pkt->rate;
				}
			}

			//��Minimum period
			if(pkt->period<MinPeriod || MinPeriod==-1){
				MinPeriod=pkt->period;
			}
		}

		Head->nextnd->eventinterval=(payload*Maxbuffersize)/MaxRate;
		if(Head->nextnd->eventinterval<10){
			Head->nextnd->eventinterval=10;
		}
		
		//Reset timer
		Callbackclock=Head->nextnd->eventinterval*overheadcount;	//�ܤֻݭn������~�వ���
	}else{
		Callbackclock--;
	}
}

void Finalcheck(){
	for(Packet* pkt=Head->nextnd->pkt; pkt!=NULL; pkt=pkt->nextpkt){
		if(pkt->deadline<=Timeslot){
			pkt->latency=pkt->latency+(Timeslot-pkt->arrival);
		}
	}
}

/*================================================
			
================================================*/
void Polling(){
	/*----------------------------------------------
	----------------------------------------------*/
	if(Head->FrameSize<=0 ){ //(Head->RecvNode�ثe�O���צ�A������n��FrameSize���ק�)
		if(Cycle!=NULL){
			Cycle=Cycle->polling_next;
		}
		if(Cycle==NULL){
			short int count=pollingcount;
			FrameTable *tmptbl=NULL;
			for(FrameTable* tbl=FrameTbl; tbl!=NULL; tbl=tbl->next_tbl){
				tbl->Currentflag=false;
			}

			while(count){
				//��̤pperiod�B���Qassign�JCycle
				tmptbl=NULL;
				for(FrameTable* tbl=FrameTbl; tbl!=NULL; tbl=tbl->next_tbl){
					if(tbl->Currentflag==false){	//�|��assign
						if(tmptbl==NULL){
							tmptbl=tbl;
						}else if(tbl->Period < tmptbl->Period){
							tmptbl=tbl;
						}
					}
				}

				//��Jcycle
				tmptbl->Currentflag=true;
				if(Cycle==NULL){
					Cycle=tmptbl;
					Cycle->polling_next=NULL;
				}else{
					FrameTable *subcycle=Cycle;
					while(subcycle->polling_next!=NULL){
						subcycle=subcycle->polling_next;
					}
					subcycle->polling_next=tmptbl;
					subcycle->polling_next->polling_next=NULL;
				}

				count--;
			}

			//�U�@��polling cycle
			pollingcount++;
			if(pollingcount>nodelevel1){
				pollingcount=1;
			}
		}

		FrameTable* Work_tbl=Cycle;

		Work_tbl->Currentflag=true;
		Work_tbl->ConnNode->State="Notify";

		Head->FrameSize=Work_tbl->Size;
		Work_tbl->Deadline=Work_tbl->Deadline+Work_tbl->Period;

		Head->FrameSize--;
	}else{
		Head->FrameSize--;
	}

}


void Write_Request(){
	
	bool NotifyFlag=true;		//�T�{�ǿ�u��@��(ConnSet)
	bool IncreaseSlot=false;	//�T�{�O�_�ݭn�[event�ɶ�

	/*----------------------------------------------
		ConnSet���Q�q����Node�������ǿ��SCAN
		Notify & Scan�i�P�ɹB�@

		node�O�_��event arrival		(node->EvtArrival)
		node�O�_���Q�q��				(Head->RecvNode, call BLE_EDF(node))
		node energy consumption		(node->State)
	----------------------------------------------*/
	for(Node *n=Head->nextnd; n!=NULL; n=n->nextnd){
		
		//-------------------State��Transmission, �u�bevent arrival �ɤ~��Buffer�W��
		
		if(Timeslot % int(n->eventinterval)==0){
			NodeBufferSet(n);		//��z�nn����NodeBuffer
			n->EvtArrival =true;	//�惡node�]�wEvtArrival

		}else{
			n->EvtArrival =false;
		}
		
		//-------------------�i��ǿ� [���Q�q��(node->State=="Transmission") �B event arrival(node->EvtArrival)]
		if(n->State!="Sleep"){
			bool eventarrival=false;
			
			if(n->EvtArrival && n->State=="Notify" && (Head->RecvNode==NULL || Head->RecvNode==n)){
				Head->RecvNode=n;			//Head->RecvNode����
				n->State="Transmission";	//�������A
				
				//�]���T�{��n�A�b��ǿ�ɶ���2.675ms�A�ҥH���[2ms (�^�hmain�|�b�[1ms)
				IncreaseSlot=true;
			}
			
			//-------------------�i��ǿ� (Head->RecvNode�n�T�{�ثe�Snode�ά���enode)
			if(Head->RecvNode==n && NotifyFlag){
				BLE_EDF(n);				//��n���ǿ�
				/*
				if(n->NodeBuffer->load==0){
					Node_EnergyState(n);	//�p��n��Energy
					Head->RecvNode=NULL;
				}
				*/
				NotifyFlag=false;
			}

			//-------------------State��Scan
			if(n->State=="Scan"){

			}
		}

		//---------------------------------------Power consumption & State����
		Node_EnergyState(n);	//�p��n��Energy
		if(n->NodeBuffer->load==0 && n->State=="Transmission" && Head->RecvNode==n){
			n->State="Sleep";
			Head->RecvNode=NULL;
		}
	}

	//
	if(IncreaseSlot){
		Timeslot=Timeslot+2;
		Head->FrameSize=Head->FrameSize-2;
	}
}
