#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "WSNFile.h"
#include "WSNStruct.h"
#include "FlowSchedule.h"
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
		//==========================Wakeup ���A
		if(Enode->State=="Wakeup"){
			
		}
		//==========================Sleep ���A
		if(Enode->State=="Sleep"){
			
		}
		//==========================Transmission ���A
		if(Enode->State=="Transmission"){
			double P=0;

			P=IntervalPower(Pktsize,Enode->LatestTimeslot-Enode->ExTimeslot);
			Enode->energy=Enode->energy+P;
			//cout<<"Interval:"<<(Enode->LatestTimeslot-Enode->ExTimeslot)<<" E:"<<Enode->energy<<endl;
			Enode->ExTimeslot=Enode->LatestTimeslot;//���������o�e�ɶ�

			Enode->State="Idle";
		}
		//==========================Idle ���A
		if(Enode->State=="Idle"){

		}

		Enode=Enode->nextnd;
	}

}
/*==========================================
		�p��b��Connection interval�U��
			Power consumption
==========================================*/
double IntervalPower(int Pktnum,int interval){
	if(interval!=0){
		double Ipeak,Vpeak;
		double Tc;
		double power;

		Tc=interval*unit;
		
		//Ipeak=(Pktnum*Ie*Te+Isleep*(Tc-Te*Pktnum))/Tc;
		Ipeak=parma*Pktnum*exp(-parmb*Tc)+Isleep;
		
		Vpeak=Vcc-Ipeak;

		power=Vcc*Ipeak-(Ipeak*Ipeak)*K;
	
		//cout<<"T:"<<Timeslot<<" P:"<<power<<endl;

		return power;
	}else
		return 0;
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