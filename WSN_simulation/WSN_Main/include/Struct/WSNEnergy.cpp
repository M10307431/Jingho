#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "WSNFile.h"
#include "WSNStruct.h"
#include "../Schedule/FlowSchedule.h"
#include "WSNEnergy.h"

using namespace std;

/*==========================================
		���O�p��U��node�W��Power
		�C�@slot�W���[�`
		�Y��E=P*t
==========================================*/
void NodeEnergy(){
	/*----------------------------
		Wakeup Sleep	  ���s�����p
		Transmission Idle �s�����p
	----------------------------*/
	Node *Enode;
	Packet *Epkt;
	
	Enode=Head->nextnd;
	while(Enode!=NULL){
		Enode->LatestTimeslot=Timeslot;
		Enode=Enode->nextnd;
	}

	Enode=Head->nextnd;
	while(Enode!=NULL){
		//==========================Sleep ���A
		if(Enode->State=="Sleep"){
			double P=0;
			P=Vcc*I_sleep;
			Enode->energy=Enode->energy+P;
			Enode->State="Sleep";
		}
		//==========================Transmission ���A
		if(Enode->State=="Transmission"){
			double P=0;

			//P=IntervalPower(Pktsize,Enode->LatestTimeslot-Enode->ExTimeslot);
			
			//cout<<"Interval:"<<(Enode->LatestTimeslot-Enode->ExTimeslot)<<" E:"<<Enode->energy<<endl;
			//Enode->ExTimeslot=Enode->LatestTimeslot;//���������o�e�ɶ�
			P=Vcc*I_notify;
			Enode->energy=Enode->energy+P;
			Enode->State="Sleep";
			
		}
		//==========================Scan ���A
		if(Enode->State=="Scan"){
			
		}

		Enode=Enode->nextnd;
	}

}

/*==========================================
		�p��Node State
==========================================*/

void NodeState(){
	Node   *Snode;
	Packet *Spacket;
	bool tx_flag=false;

	Snode=Head->nextnd;
	
	while(Snode!=NULL){
		Spacket=Snode->pkt;
		while(Spacket!=NULL){
			if(Spacket->State=="Transmission"){
				Snode->State="Transmission";
				tx_flag=true;
			}
			if(Spacket->State=="Idle" && tx_flag!=true){
				Snode->State="Idle";
			}

			Spacket=Spacket->nodenextpkt;
		}
		Snode=Snode->nextnd;
		tx_flag=false;
	}
	delete Snode;
	delete Spacket;
}

/*========================================
		���e��Node��Energy�W���p��
		�B��buffer�b�šA�ߧY����State
		>EvtArrival ==True
		>State=="Transmission" && Head->RecvNode
========================================*/

void Node_EnergyState(Node *node){
	
	double v=I_sleep, b=I_notify;
	//I_sleep=0.0004;
	//I_notify=0.009999;
	if(node!=NULL){
		/*====================================================
					�̷Ӥ��P���A�� Power�p��
		====================================================*/
		//==========================Sleep ���A �Bconnection event��arrival
		if(node->State=="Sleep" && !node->EvtArrival){
			node->energy=node->energy+(I_sleep*Time_sleep);
		}

		//==========================Event arrival ���A
		if(node->EvtArrival){
			node->energy=node->energy+((I_notify*Time_notify)+(I_sleep*(0.003-Time_notify)));
			node->Notify_evtcount+=1;
		}else if(node->State=="Transmission" && Head->RecvNode==node){ //==========================Transmission ���A
			if(Time_Tran<Time_sleep){
				node->energy=node->energy+((I_Tran*Time_Tran)+(I_sleep*(Time_sleep-Time_Tran)));
			}else{
				node->energy=node->energy+(I_Tran*Time_Tran);
			}
			node->Tran_evtcount+=1;
		}
		
		//==========================Scan ���A
		if(node->State=="Scan"){
			
		}
	}

	I_sleep=v;

}