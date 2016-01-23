#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include "WSNFile.h"
#include "WSNStruct.h"
#include "FlowSchedule.h"
using namespace std;

/*===========================
		GEN & Output 
		File Path
===========================*/
string filename;
fstream GENfile;
fstream Schdulefile;
fstream Powerfile;
fstream Resultfile;

string GENPath="..\\GENresult\\";
string SchedulePath="..\\WSNresult\\TSB_TDMA_ToAp\\";
string PowerPath="..\\WSNresult\\TSB_TDMA_ToAp\\";
string ResultPath="..\\WSNresult\\TSB_TDMA_ToAp\\";

/*===========================
		�NGEN����ƨ��J �B
		�إ߿�X���
===========================*/
void CreateFile(float U,int Set){
	//��JGEN���ɦW
	filename="U";filename.append(to_string(U));filename.append("_Set");filename.append(to_string(Set));filename.append(".txt");

	//��JGEN�� ���|+�ɦW
	string GENBuffer=GENPath;
	GENBuffer.append(filename);

	//=========================�}��GENFile
	GENfile.open(GENBuffer, ios::in);	//�}���ɮ�.�g�J���A
	if(!GENfile){//�p�G�}���ɮץ��ѡAfp��0�F���\�Afp���D0
		cout<<"Fail to open file: "<<GENBuffer<<endl;
		system("PAUSE");
	}

	//========================�}��ScheduleFile
	string ScheduleFileBuffer=SchedulePath;
	ScheduleFileBuffer.append("Schedule_");
	ScheduleFileBuffer.append(filename);

	Schdulefile.open(ScheduleFileBuffer, ios::out);	//�}���ɮ�.�g�J���A
	if(!Schdulefile){//�p�G�}���ɮץ��ѡAfp��0�F���\�Afp���D0
		cout<<"Fail to open file: "<<ScheduleFileBuffer<<endl;
		system("PAUSE");
	}

	//========================�}��Resultfile
	string ResultBuffer=ResultPath;
	ResultBuffer.append("Result_");
	ResultBuffer.append(filename);

	Resultfile.open(ResultBuffer, ios::out);	//�}���ɮ�.�g�J���A
	if(!Resultfile){//�p�G�}���ɮץ��ѡAfp��0�F���\�Afp���D0
		cout<<"Fail to open file: "<<ResultBuffer<<endl;
		system("PAUSE");
	}
}

/*===========================
	��@taskset�x�s
===========================*/
void SaveFile(short int setnum){
	double totalenergy=0;
	node=Head->nextnd;
	while(node!=NULL){
		cout<<"Node"<<node->id<<" E:"<<node->energy<<endl;
		totalenergy=totalenergy+node->energy;
		node=node->nextnd;
	}
			
	Resultfile<<"TotalEnergy:"<<totalenergy<<endl;
	cout<<"TotalEnergy:"<<totalenergy<<endl;
	
	//---------------------------------------------------TDMA table
	TDMATable *FlowTable=TDMA_Tbl;
	while(FlowTable!=NULL){
		cout<<"S"<<FlowTable->slot<<" n"<<FlowTable->n1->id<<endl;
		Resultfile<<"S"<<FlowTable->slot<<" n"<<FlowTable->n1->id<<endl;

		FlowTable=FlowTable->next_tbl;
	}



	//---------------------------------------------------�s���U��node��T
	Resultfile<<"Energy Tc, Color Conflict_node, coor_x coor_y Send_node"<<endl;

	node=Head->nextnd;
	while(node!=NULL){
		double AvgRate=0;
		double Pktnum=0;
		packet=node->pkt;
		while(packet!=NULL){
			AvgRate=AvgRate+packet->rate;
			Pktnum++;
			packet=packet->nodenextpkt;
		}
		AvgRate=AvgRate/Pktnum;

		//Node id:energy connectioninterval,color conflictnode...,coor_x coor_y SendNode 
		Resultfile<<"Node"<<node->id<<":";
		Resultfile<<node->energy<<" "<<node->eventinterval<<",";
				
		Resultfile<<node->color<<" ";
		Edge *printedge=ConflictEdge;
		while(printedge!=NULL){
			if(printedge->n1->id==node->id)
				Resultfile<<printedge->n2->id<<" ";
			printedge=printedge->next_edge;
		}
		Resultfile<<",";

		Resultfile<<node->coor_x<<" "<<node->coor_y<<" ";
		Resultfile<<node->SendNode->id<<endl;

		//Resultfile<<"Total Event:"<<totalevent<<endl;

		node=node->nextnd;
	}
	if(Meetflag==true){
		Resultfile<<"Meet Deadline:MEET"<<endl;
		cout<<"Meet Deadline:MEET"<<endl;
				
		node=Head->nextnd;
		SetNode=SetHead->nextnd;
		while(node!=NULL){
			SetNode->avgenergy=SetNode->avgenergy+node->energy;
					
			SetNode=SetNode->nextnd;
			node=node->nextnd;
		}
		SetNode=SetHead->nextnd;

		AverageE=AverageE+totalenergy;
		Meetcount++;
	}
	else{
		Resultfile<<"Meet Deadline:MISS"<<endl;
		cout<<"Meet Deadline:MISS"<<endl;
	}
	
	Resultfile<<"==============================================="<<setnum<<endl;
	Schdulefile<<"=============================================="<<setnum<<endl;
	cout<<"=============================================="<<setnum<<endl;
}

/*===========================
	���Q�βvSet�x�s
===========================*/
void SaveSet(int Set){
	cout<<"FinalResult"<<endl;
	cout<<"Meet="<<Meetcount<<endl;
	cout<<"Miss="<<Set-Meetcount<<endl;
	cout<<"MeetRatio="<<Meetcount/Set<<endl;
	SetNode=SetHead->nextnd;
	while(SetNode!=NULL){
		cout<<"Node"<<SetNode->id<<"="<<SetNode->avgenergy/Meetcount<<endl;
		SetNode=SetNode->nextnd;
	}
	cout<<"AverageEnergy="<<AverageE/Meetcount<<endl;
	cout<<"=============================================="<<endl;

	Resultfile<<"FinalResult"<<endl;
	Resultfile<<"Meet="<<Meetcount<<endl;
	Resultfile<<"Miss="<<Set-Meetcount<<endl;
	Resultfile<<"MeetRatio="<<Meetcount/Set<<endl;
	SetNode=SetHead->nextnd;
	while(SetNode!=NULL){
		Resultfile<<"Node"<<SetNode->id<<"="<<SetNode->avgenergy/Meetcount<<endl;
		SetNode=SetNode->nextnd;
	}
	Resultfile<<"AverageEnergy="<<AverageE/Meetcount<<endl;

	GENfile.close();
	Schdulefile.close();
	Resultfile.close();
}