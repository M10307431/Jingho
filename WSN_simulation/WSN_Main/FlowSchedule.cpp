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
#include "WSNFile.h"
#include "WSNStruct.h"
#include "ConnInterval.h"
#include "FlowSchedule.h"
#include "WSNEnergy.h"

using namespace std;

/*==========================
	�D��Schedule
	�ۦP�C��P�ɶǿ�
	�P�䤣�I���P�ɶǿ�
	FlowSlot-->���ѭ��@Slot�}�l��(TDMATable)
	Flow_flag-->�P�_���L�I��(ConflictEdge)
==========================*/
void MainSchedule(int FlowSlot,bool Flow_flag){

	//-------------------------------��Xconnection interval��F��node
	Node *Flownode=Head->nextnd;
	while(Flownode!=NULL){	

		Buffer=Flownode->NodeBuffer;

		if(Timeslot % int(Flownode->eventinterval)==0 && Buffer->pkt!=NULL) //�Y��Recvnode�A�]�ݦҶq��interval
			Flownode->arrival_flag=1;
		
		/*--------------------------
			�YConnection interval��
			������pkt�i�H���O���warrival
			set arrival_flag��10
			���ݤU�@����pkt���p
		--------------------------*/
		if(Flownode->arrival_flag==10 && Buffer->pkt!=NULL) //�Y��Recvnode�A�]�ݦҶq��interval
			Flownode->arrival_flag=1;
		if(Timeslot % int(Flownode->eventinterval)==0 && Buffer->pkt==NULL) //�Y��Recvnode�A�]�ݦҶq��interval
			Flownode->arrival_flag=10;

		Flownode=Flownode->nextnd;
	}

	//-------------------------------��X�ثe�]�Ӷǿ骺TDMA slot id (FlowSlot)�A
	//���|�H�̦���time slot���D�A�å����̧Ǳ��p(�Y�n��slot�ǧ����U�@slot���D�ݭn�[�J�P�_)
	//�Y���ݥ[�J�n�� �e�@FlowSlot ����Slot
	
	int Maxslot=0;	//��XTDMA�̤jSlot id
	TDMATable *FlowTable=TDMA_Tbl;
	while(FlowTable!=NULL){
		/*
		if(FlowTable->n1->arrival_flag==1){
			FlowSlot=FlowTable->slot;
			break;
		}
		*/
		if(FlowTable->slot > Maxslot){
			Maxslot=FlowTable->slot;
		}

		FlowTable=FlowTable->next_tbl;
	}
	
	FlowSlot=TDMASlot++;
	if(FlowSlot>Maxslot){
		FlowSlot=1;
		TDMASlot=2;
	}
	//-------------------------------TDMA Table�U��FlowSlot ��node
	FlowTable=TDMA_Tbl;
	while(FlowTable!=NULL){
					
		Flow_flag=true;
		if(FlowTable->n1->arrival_flag==1 && FlowTable->slot==FlowSlot){//��w�garrival��node �B �b��FlowSlot�W
						
			//��FlowTable�W��n1�å������ǿ駹����node�I�� (ConflictEdge->n2��arrival_flag�i��1 �����i��-1)
			//(�z�פW�ӻ��bTDMA schedule�إ�Table�ɴN������o�@��)
			Edge *tmp_ConflictEdge=ConflictEdge;
			while(tmp_ConflictEdge!=NULL){
				if(tmp_ConflictEdge->n1==FlowTable->n1 && tmp_ConflictEdge->n2->arrival_flag==-1){ 
					Flow_flag=false;
					cout<<"At FlowSchedule the slot's node have conflict with each other"<<endl;
					printf("Node%d, Node%d Conflict\n",tmp_ConflictEdge->n1->id,tmp_ConflictEdge->n2->id);

					system("PAUSE");
				}
				tmp_ConflictEdge=tmp_ConflictEdge->next_edge;
			}

			//��Flow_flag�P�_FlowTable->n1�O�_�i�ǿ�
			Flownode=FlowTable->n1;
			if(Flow_flag){
				Buffer=Flownode->NodeBuffer;
							
				FlowEDF();

				Flownode->arrival_flag=-1;
			}

		}
		FlowTable=FlowTable->next_tbl;
	}

	//-----------------------------------�N�谵����flag�אּ�ǿ駹��
	Flownode=Head->nextnd;
	while(Flownode!=NULL){
		if(Flownode->arrival_flag==-1)
			Flownode->arrival_flag=0;
		Flownode=Flownode->nextnd;
	}
}

/*==========================
	Flow EDF Scheduling
	Event Transmission
==========================*/
void FlowEDF(){
	
		/*---------------------------------------------
				�P�_Flow �O�_��NULL
				�Y���ʥ]�h�i��ǿ�
				(�]�t�P�_�O�_����)
		---------------------------------------------*/
		if(Buffer->pkt!=NULL){
			totalevent++;

			//cout<<"Time slot:"<<Timeslot<<" Node:"<<Buffer->pkt->node->id;
			Schdulefile<<"Time slot:"<<Timeslot;
			//=============================================����ǿ�
			packet=Buffer->pkt;
			Pktsize=Buffer->pktsize;

			while(Buffer->load!=0){
								
				packet->exeload--;
				Buffer->load--;
				packet->node->State="Transmission";
				packet->State="Transmission";		//�ǿ骬�A

				if(packet->exeload==0){

					//cout<<" Packet:"<<packet->id;
					Schdulefile<<" NP:"<<packet->node->id<<","<<packet->id;
					
					//�P�_�O�_�ݭnhop
					packet->exehop--;
					if(packet->exehop>0)
					{
						//��JSendNode ���i�J��packet��priority�@�w����
						
						packet->exeload=packet->load;
						Headflow->pkt=packet;
					}else if (packet->exehop==0){
						//�P�_�O�_miss deadline
						if((Timeslot)>=packet->deadline){
							
							cout<<"(PKT"<<packet->id<<" Miss deadline"<<" Deadline "<<packet->deadline<<")";
							Schdulefile<<"(PKT"<<packet->id<<" Miss deadline"<<" Deadline "<<packet->deadline<<")";

							Meetflag=false;
							//system("PAUSE");
						}
						
						packet->readyflag=0;
						packet->exeload=packet->load;
						packet->arrival=packet->deadline;
						packet->deadline=packet->deadline+packet->period;
						packet->State="Idle";		//�ǿ骬�A
						packet->exehop=packet->hop;	
					}

					//Buffer���e����
					packet=packet->buffernextpkt;
					Buffer->pkt=packet;	
				}
				if(Buffer->pkt==NULL)
					break;
			}
			Headflow->pkt=packet;//��m�|��@��packet

			if(Headflow->pkt!=NULL){
				//cout<<" Packet:"<<packet->id;
				Schdulefile<<" NP:"<<packet->node->id<<","<<packet->id;
				packet->State="Transmission";		//�ǿ骬�A
				packet->node->State="Transmission";		//�ǿ骬�A
			}
			
			//cout<<endl;
			Schdulefile<<endl;
			/*---------------------------
				�ǿ駹�ߧY��
				���A���� & Energy �p��
			---------------------------*/
			NodeEnergy();	//�p��ӷP����Energy

		}else{
			NodeEnergy();

			//cout<<"Time slot:"<<Timeslot<<" IDLE"<<endl;
			Schdulefile<<"Time slot:"<<Timeslot<<" IDLE"<<endl;
			
		}
}
/*==========================================
			Arrange Queue
{ReadyQ & WaitQ is assign by pkt->readyflag}

	���إ��`total��Packet Queue
	�b���t���U�۪�node
==========================================*/

void PacketQueue(){
	
	Packet *camparepkt;
	Packet *tmpReadyQ=new Packet;
	tmpReadyQ->readynextpkt=ReadyQ;

	packet=Head->nextnd->pkt;
	while(packet!=NULL){
		packet->readynextpkt=NULL;	//�U�@ready�]�w��NULL
		packet->readyprepkt=NULL;	//�W�@ready�]�w��NULL
		packet->searchdone=0;				//�|����L
		packet=packet->nextpkt;
	}

	packet=Head->nextnd->pkt;
	ReadyQ_overflag=0;
	while(!ReadyQ_overflag){
	
		/*--------------------------
			��searchdone��0 ��
			�e���packet �����
			(packet & camparepkt)
		--------------------------*/
	
		packet=Head->nextnd->pkt;
		while(packet!=NULL){
			if(packet->searchdone==1 )
				packet=packet->nextpkt;
			else
				break;
		}

		if(packet!=NULL)
			camparepkt=packet->nextpkt;
		while(camparepkt!=NULL){
			if(camparepkt->searchdone==1 )
				camparepkt=camparepkt->nextpkt;
			else
				break;
		}
		
		/*--------------------------
			��X�̤pDeadline
			�����j�M�@�M
		--------------------------*/
	
		if(packet!=NULL && camparepkt!=NULL){
			while(camparepkt!=NULL){

				//��packet�U�@�Ӥ����camparepkt
				if(packet!=NULL)
					camparepkt=packet->nextpkt;
				while( camparepkt!=NULL){
					if(camparepkt->searchdone==1)
						camparepkt=camparepkt->nextpkt;
					else
						break;
				}
				
				//�Y��packet->deadline >camparepkt->deadline ,�Npacket=camparepkt
				while(camparepkt!=NULL){
					if(packet->deadline >camparepkt->deadline ){
						packet=camparepkt;
					}
					if(camparepkt!=NULL)
							camparepkt=camparepkt->nextpkt;
					while(camparepkt!=NULL){
						if(camparepkt->searchdone==1 )
							camparepkt=camparepkt->nextpkt;
						else
							break;
					}
				}
			}
		}else if(camparepkt==NULL){
			ReadyQ_overflag=1;//���ѤU�̫�@��packet�A�Y�i�פ�M��
		}
	
		/*--------------------------
		�N�̤pDeadline��JReadyQ��
		�üаO�w�M��L(searchdone=1)
		--------------------------*/
		packet->searchdone=1;

		if(Timeslot>=packet->arrival){
			packet->readyflag=1;//�ʥ] Arrival
			packet->node->arrival_flag=1;
		}else{
			packet->readyflag=0;//�ʥ] �|��Arrival
		}

		ReadyQ->readynextpkt=packet;
		packet->readyprepkt=ReadyQ;
		ReadyQ=packet;
	}
	ReadyQ->readynextpkt=NULL;
	ReadyQ=tmpReadyQ->readynextpkt;

	delete camparepkt;camparepkt=NULL;
	delete tmpReadyQ;tmpReadyQ=NULL;
	//----------------------------------------Global Ready Queue done

	/*--------------------------
		�إߦU��node�W��
		ready packet queue
	--------------------------*/
	Node *tmp_node=Head->nextnd;
	Packet *tmpreadypkt;
	while(tmp_node!=NULL){
		tmpreadypkt=tmp_node->pkt;
		tmp_node->pktQueue=NULL;
		while(tmpreadypkt!=NULL){
			tmpreadypkt->nodereadynextpkt=NULL;
			tmpreadypkt->nodereadyprepkt=NULL;
			tmpreadypkt=tmpreadypkt->nextpkt;
		}
		tmp_node=tmp_node->nextnd;
	}

	Packet *tmp_nodepkt=ReadyQ->readynextpkt;
	while(tmp_nodepkt!=NULL){
		//������ݪ� node �P����
		tmp_node=tmp_nodepkt->node;

		//��node->pktQueue �̫�@��
		tmpreadypkt=tmp_node->pktQueue;
		if(tmp_node->pktQueue==NULL){
			tmp_node->pktQueue=tmp_nodepkt;			
			tmp_node->pktQueue->nodereadynextpkt=NULL;
			tmp_node->pktQueue->nodereadyprepkt=NULL;
		}else{
			while(tmpreadypkt->nodereadynextpkt!=NULL){
				tmpreadypkt=tmpreadypkt->nodereadynextpkt;
			}

			tmpreadypkt->nodereadynextpkt=tmp_nodepkt;				
			tmpreadypkt->nodereadynextpkt->nodereadynextpkt=NULL;
			tmpreadypkt->nodereadynextpkt->nodereadyprepkt=tmp_nodepkt->nodereadyprepkt;
		}

		//���U�@��Global Queue packet
		tmp_nodepkt=tmp_nodepkt->readynextpkt;
	}
}
/*=============================================	
		�إߦnBuffer�W��packet
	pkt link list, load, packet size
=============================================*/
void BufferSet(){
	Node *Bufnode=Head->nextnd;

	while(Bufnode!=NULL){
		Bufnode->NodeBuffer->pktsize=0;//����buffer�����ʥ]�q�M��
		Bufnode->NodeBuffer->load=0;
		Bufnode->NodeBuffer->pkt=NULL;
		
		Packet *tmpbufferpkt;
		int tmpsize=Bufnode->NodeBuffer->pktsize;
		int intervalsize=0;

		packet=Bufnode->pktQueue;
		if(Bufnode->NodeBuffer->pktsize!=0){
			while(packet->readyflag!=1)			
				packet=packet->nodereadynextpkt;
			if(packet==tmpbufferpkt)
				packet=packet->nodereadynextpkt;
		}

		while(Bufnode->NodeBuffer->pktsize<Maxbuffersize && packet!=NULL){

			if(packet->readyflag!=1){			//�|��ready�A�������U�@packet
				packet=packet->nodereadynextpkt;
			}else{
			
				//�T�{packet���s�b��Buffer��
				bool existflag=false;
				Packet* existpkt=Bufnode->NodeBuffer->pkt;
				while(existpkt!=NULL){
					if(existpkt==packet)
						existflag=true;
					existpkt=existpkt->buffernextpkt;
				}

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
							if(tmpload>payload)
								Bufnode->NodeBuffer->load=Bufnode->NodeBuffer->load+payload;
							else
								Bufnode->NodeBuffer->load=Bufnode->NodeBuffer->load+tmpload;
							tmpload=tmpload-payload;
							intervalsize--;
						}

					}else{
						Bufnode->NodeBuffer->load=Bufnode->NodeBuffer->load+packet->exeload;
					}
				}
				packet=packet->nodereadynextpkt;
			}
		}

		Bufnode=Bufnode->nextnd;
	}
}

/*=========================
	Admission Control
	(�p��쥻��Tc �P 
	TDMA �]���y�������j)
=========================*/
void Schedulability(){
	int n=0;
	int Tc=0;
	int differentvalue=0;
	int Slotid=0;
	int TDMASize=0;
	int preTXslot=0;
	int endTXslot=0;

	TDMATable *Stable=TDMA_Tbl;
	
	node=Head->nextnd;
	while(Stable!=NULL){
		if(TDMASize<Stable->slot){
			TDMASize=Stable->slot;
		}
		Stable=Stable->next_tbl;
	}
	Stable=TDMA_Tbl;


	/*---------------------------
			�p��C�@node
		�����Connection interval
		�PTDMA�y��������o�e����
	---------------------------*/
	while(node!=NULL){
		//�]�w�쥻��Connection interval
		Tc=node->eventinterval;
	
		//��X��TDMA�W�� �o�e��Slotid
		while(Stable->n1!=node){
			Stable=Stable->next_tbl;
		}
		Slotid = Stable->slot-1;
		Stable=TDMA_Tbl;

		//
		preTXslot=Slotid;
		n=1;
		differentvalue=0;
		while(n*Tc <= Hyperperiod){
			endTXslot=Slotid+TDMASize*floor(n*Tc/TDMASize);
			while(endTXslot < n*Tc)
				endTXslot=endTXslot+TDMASize;

			differentvalue=differentvalue+(Tc-(endTXslot-preTXslot));
		
			preTXslot=endTXslot;
			n++;
		}
		/*
		if(differentvalue<0){
			printf("Node%d Predict Miss deadline\n",node->id);
		}
		*/
		node=node->nextnd;
	}

}

void BLESchedule(int FlowSlot, bool Flow_flag){

	bool Notifydone=false;
	TDMATable *FlowTable=TDMA_Tbl;
	Node *Flownode=Head->nextnd;
	Node *tmpnode=Head->nextnd;

	//------------------------------------------------------------------------------�o�eACK��SCAN CMD
	Flownode=Head->nextnd;
	while(Flownode!=NULL){
		if(FlowSlot % int(Flownode->eventinterval)==0){
			if(Flownode->arrival_flag==1){
				NodeBufferSet(Flownode);		//�惡node��Buffer setting
				Flownode->ContinueNotify=true;	//�i�ǿ�

			}
		}	
		
		Flownode=Flownode->nextnd;
	}

	//--------
	Buffer=NULL;
	if(NotifyNode!=NULL){
		Buffer=NotifyNode->NodeBuffer;
	}else{
		Flownode=Head->nextnd;
		while(Flownode!=NULL){
			if(Flownode->ContinueNotify){
				NotifyNode=Flownode;	//�]�w�����b�ǿ骺node�A���N���W�i��ǿ�
				Buffer=Flownode->NodeBuffer;			
				break;
			}
			Flownode=Flownode->nextnd;
		}
	}
	if(Buffer!=NULL)
		BLE_EDF(NotifyNode);
	
}

void BLE_EDF(Node *BLE_NotifyNode){
	
		/*---------------------------------------------
				�P�_Flow �O�_��NULL
				�Y���ʥ]�h�i��ǿ�
				(�]�t�P�_�O�_����)
		---------------------------------------------*/
		if(Buffer->pkt!=NULL){
			totalevent++;
			Node *tmpnode=Buffer->pkt->node;

			cout<<"Time slot:"<<Timeslot<<" Node:"<<Buffer->pkt->node->id<<",";
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
			NodeEnergy();	//�p��ӷP����Energy
			Schdulefile<<" NP:"<<packet->node->id<<","<<packet->id;
			cout<<"P:"<<packet->id<<endl;

			//=============================================�ǧ�,���U�@packet
			if(packet->exeload==0){

				//�P�_�O�_miss deadline
				if((Timeslot)>=packet->deadline){
							
					cout<<"(PKT"<<packet->id<<" Miss deadline"<<" Deadline "<<packet->deadline<<")";
					Schdulefile<<"(PKT"<<packet->id<<" Miss deadline"<<" Deadline "<<packet->deadline<<")";

					Meetflag=false;
					//system("PAUSE");
				}

				packet->readyflag=0;
				packet->exeload=packet->load;
				packet->arrival=packet->deadline;
				packet->deadline=packet->deadline+packet->period;
				packet->State="Idle";		//�ǿ骬�A
				packet->exehop=packet->hop;	

				//Buffer���e����
				packet=packet->buffernextpkt;
				Buffer->pkt=packet;	
			}

			//=============================================�T�{Buffer�S��packet,�Narrival_flag�]��false
			if(Buffer->load==0){
				tmpnode->arrival_flag=0;
				tmpnode->ContinueNotify=false;
				NotifyNode=NULL;

				int Maxslot=0;	//��XTDMA�̤jSlot id
				TDMATable *FlowTable=TDMA_Tbl;
				while(FlowTable!=NULL){

					if(FlowTable->slot > Maxslot){
						Maxslot=FlowTable->slot;
					}

					FlowTable=FlowTable->next_tbl;
				}
	
				TDMASlot++;
				if(TDMASlot>Maxslot){
					TDMASlot=1;
				}
			}
			
			Schdulefile<<endl;
		}else{
			//BLE_NotifyNode->arrival_flag=0;

		}
}

void NodeBufferSet(Node * SettingNode){
	Node *Bufnode=Head->nextnd;

	//-------------------------------------------------------------�����S�w��node
	while(Bufnode!=SettingNode)
		Bufnode=Bufnode->nextnd;

	//-------------------------------------------------------------�惡node��NodeBuffer�����t
	if(Bufnode==SettingNode){
		Bufnode->NodeBuffer->pktsize=0;//����buffer�����ʥ]�q�M��
		Bufnode->NodeBuffer->load=0;
		Bufnode->NodeBuffer->pkt=NULL;
		
		Packet *tmpbufferpkt;
		int tmpsize=Bufnode->NodeBuffer->pktsize;
		int intervalsize=0;

		packet=Bufnode->pktQueue;
		if(Bufnode->NodeBuffer->pktsize!=0){
			while(packet->readyflag!=1)			
				packet=packet->nodereadynextpkt;
			if(packet==tmpbufferpkt)
				packet=packet->nodereadynextpkt;
		}

		while(Bufnode->NodeBuffer->pktsize<Maxbuffersize && packet!=NULL){

			if(packet->readyflag!=1){			//�|��ready�A�������U�@packet
				packet=packet->nodereadynextpkt;
			}else{
			
				//�T�{packet���s�b��Buffer��
				bool existflag=false;
				Packet* existpkt=Bufnode->NodeBuffer->pkt;
				while(existpkt!=NULL){
					if(existpkt==packet)
						existflag=true;
					existpkt=existpkt->buffernextpkt;
				}

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
							if(tmpload>payload)
								Bufnode->NodeBuffer->load=Bufnode->NodeBuffer->load+payload;
							else
								Bufnode->NodeBuffer->load=Bufnode->NodeBuffer->load+tmpload;
							tmpload=tmpload-payload;
							intervalsize--;
						}

					}else{
						Bufnode->NodeBuffer->load=Bufnode->NodeBuffer->load+packet->exeload;
					}
				}
				packet=packet->nodereadynextpkt;
			}
		}

	}
}