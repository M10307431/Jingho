#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "../Struct/WSNFile.h"
#include "../Struct/WSNStruct.h"
#include "ConnInterval.h"
#include "TDMA.h"

using namespace std;

/*==============================================
				Construct
==============================================*/
TDMA::TDMA(){
	printf("TDMA Object\n");
}

/*===========================
		WSN�ݾQ
	>�]�w�Unode���ǿ�ؼ�
===========================*/
void TDMA::Topology(){
	double distance=100;

	/*---------------------
		��̵u�Z���W���۳s��
		�]�wnode�ǿ�ؼ�
	---------------------*/
	for(Node* n=Head->nextnd; n!=NULL; n=n->nextnd){
		//--------------------------Level 1 (Conn Node)
		if(n->hop==1){
			n->SendNode=Head;
		}

		//--------------------------Level 2	(Adv Node)
		if(n->hop!=1){
			distance=0;
			for(Node* cmpNode=Head->nextnd; cmpNode!=NULL; cmpNode=cmpNode->nextnd){
				if(cmpNode->hop==n->hop-1){ //�T�{cmpNode��n���e�@�h
					if(distance>sqrt(pow(n->coor_x-cmpNode->coor_x,2)+pow(n->coor_y-cmpNode->coor_y,2)) || distance==0){
						distance=sqrt(pow(n->coor_x-cmpNode->coor_x,2)+pow(n->coor_y-cmpNode->coor_y,2));
						n->SendNode=cmpNode;
					}
				}
			}
		}

	}
	
	/*---------------------
		Graph Conflict
	---------------------*/
	//=====================�إ�Edge ���HeadEdge
	Edge *tmpedge;
	HeadEdge=MainEdge;
	node=Head->nextnd;
	while(node!=NULL){
		tmpedge=new Edge;
		MainEdge->n1=node;
		MainEdge->n2=node->SendNode;
		
		//�إ�Child node
		if(node->SendNode->ChildNode==NULL){
			node->SendNode->ChildNode=node;
			node->SendNode->ChildNode->next_child=NULL;
		}else{
			Node* tmp_child=node->SendNode->ChildNode;
			while(tmp_child->next_child!=NULL){
				tmp_child=tmp_child->next_child;
			}
			tmp_child->next_child=node;
			tmp_child=tmp_child->next_child;
			tmp_child->next_child=NULL;
		}

		MainEdge->next_edge=tmpedge;
		tmpedge->pre_edge=MainEdge;
		MainEdge=tmpedge;

		node=node->nextnd;
	}
	MainEdge->next_edge=NULL;

	//=====================�إ�Conflict Edge
	Edge *MainConflictEdge=ConflictEdge;
	Node *Childchild_node;
	node=Head->nextnd;
	while(node!=NULL){

		//---------------------------------------------------SendNode
		ConflictEdge->n1=node;
		ConflictEdge->n2=node->SendNode;
		tmpedge=new Edge;
		ConflictEdge->next_edge=tmpedge;
		tmpedge->pre_edge=ConflictEdge;
		ConflictEdge=tmpedge;
		//------SendNode's edge1(Send)
		if(node->SendNode!=Head){
			ConflictEdge->n1=node;
			ConflictEdge->n2=node->SendNode->SendNode;
			tmpedge=new Edge;
			ConflictEdge->next_edge=tmpedge;
			tmpedge->pre_edge=ConflictEdge;
			ConflictEdge=tmpedge;
		}
		//------SendNode's edge2~(Child)
		Childchild_node=node->SendNode->ChildNode;
		if(Childchild_node==node)
				Childchild_node=Childchild_node->next_child;
		while(Childchild_node!=NULL){
			ConflictEdge->n1=node;
			ConflictEdge->n2=Childchild_node;
			tmpedge=new Edge;
			ConflictEdge->next_edge=tmpedge;
			tmpedge->pre_edge=ConflictEdge;
			ConflictEdge=tmpedge;

			Childchild_node=Childchild_node->next_child;
			if(Childchild_node==node)
				Childchild_node=Childchild_node->next_child;
		}

		//---------------------------------------------------ChildNode
		Node* Children=node->ChildNode;
		while(Children!=NULL){
			ConflictEdge->n1=node;
			ConflictEdge->n2=Children;
			tmpedge=new Edge;
			ConflictEdge->next_edge=tmpedge;
			tmpedge->pre_edge=ConflictEdge;
			ConflictEdge=tmpedge;
			//------ChildNode's edge2(Child)
			if(Children!=NULL){
				Childchild_node=Children->ChildNode;
				while(Childchild_node!=NULL){
					ConflictEdge->n1=node;
					ConflictEdge->n2=Childchild_node;
					tmpedge=new Edge;
					ConflictEdge->next_edge=tmpedge;
					tmpedge->pre_edge=ConflictEdge;
					ConflictEdge=tmpedge;

					Childchild_node=Childchild_node->next_child;
				}
			}

			Children=Children->next_child;
		}

		node=node->nextnd;
	}
	ConflictEdge->pre_edge->next_edge=NULL;
	ConflictEdge=MainConflictEdge;
}

/*========================
	�U�۸`�I���C��
========================*/
void TDMA::NodeColoring(){
	Edge *N_CEdge=ConflictEdge;
	Edge *AssignEdge;
	Node *AssignNode;
	bool Assign_flag=false;
	int colorid=1;

	/*--------------------
		edge�p�� & Init
	--------------------*/
	node=N_CEdge->n1;
	while(N_CEdge!=NULL){
		
		if(node==N_CEdge->n1){
			node->edge=node->edge+1;
		}else{
			node=N_CEdge->n1;
			node->edge=node->edge+1;
		}
			
		N_CEdge=N_CEdge->next_edge;
	}
	
	node=Head->nextnd;
	while(node!=NULL){
		node->order_flag=false;
		node->color=0;
		node=node->nextnd;
	}

	/*--------------------
		���Ncolor assign
		���̤j��edge
	--------------------*/
	while(!Assign_flag){
		//--------------------------------��X�̤jedge��node�A��AssignNode 
		AssignNode=Head->nextnd;
		node=Head->nextnd;
		//����X�٥�assign��node
		while(node!=NULL){
			if(node->order_flag==false){
				AssignNode=node;
				break;
			}
			node=node->nextnd;
		}

		node=Head->nextnd;
		while(node!=NULL){
			if(node->edge > AssignNode->edge && node->order_flag==false){
				AssignNode=node;
			}
			node=node->nextnd;
		}

		//--------------------------------��AssignNode color
		N_CEdge=ConflictEdge;
		while(N_CEdge->n1 !=AssignNode)
			N_CEdge=N_CEdge->next_edge;
		AssignEdge=N_CEdge;				//AssignNode�bConflictEdge�����Ĥ@��node
	
		colorid=1;
		AssignNode->color=colorid;
		node=Head->nextnd;
		bool research_flag=false;
		while(node!=NULL){

			if(node->color == colorid){	//�ۦP�C�⪺---(1)
				N_CEdge=AssignEdge;				
				while(N_CEdge!=NULL){				
					if(N_CEdge->n1==AssignNode && N_CEdge->n2==node ){//�I����---(2)
						colorid++;
						research_flag=true;	//�Y���ۦP�C��B�I����node�A�h�ݭn���s����node����@��
					}
					N_CEdge=N_CEdge->next_edge;
				}
			}
			if(research_flag){
				node=Head;
				research_flag=false;
			}
			node=node->nextnd;
		}
		AssignNode->order_flag=true;
		AssignNode->color=colorid;

		//--------------------------------�P�_�O�_�����H�j�M����
		Assign_flag=true;
		node=Head->nextnd;
		while(node!=NULL){
			if(node->color==0)
				Assign_flag=false;
			node=node->nextnd;
		}
	}
}

/*========================
	TDMA Schedule
	0->�ۤv����k�A�u���@��superslot
	1->�w�Ǩ�AP���D�A�ҥH�������쪺�`�I�ݦA�[�Jsuperslot 
========================*/
void TDMA::TDMA_Assignment(int method){
	delete TDMA_Tbl;TDMA_Tbl=NULL;
	TDMA_Tbl=new TDMATable;

	int colorid=1;		//���P��slot time
	int Maxcolor=0;		//�̤j��color

	TDMATable *tmp_tbl;
	TDMATable *MainTable=TDMA_Tbl;
	Edge *N_CEdge=ConflictEdge;

	//----------------------------------------����X�̤jcolor id
	node=Head->nextnd;
	while(node!=NULL){
		if(node->color > Maxcolor)
			Maxcolor=node->color;
		node=node->nextnd;
	}

	//-----------------------------------�����t�ۦP�C�⪺node �b�P�@slot�W <�u����@superslot>
	while(Maxcolor>=colorid){
		for(Node* n=Head->nextnd; n!=NULL; n=n->nextnd){
			if(n->color == colorid){
				//�ݬO�_�ǵ���Lnode
				if(n->SendNode==Head){
					n->TDMArecv_flag=0;
				}else{
					n->SendNode->TDMArecv_flag=1;
				}

				//assign pair{slot,node} (�Y�t���ۦPcolor�Anode�i�֦�B�z)
				TDMA_Tbl->slot=colorid;
				TDMA_Tbl->n1=n;
				
				//�إߤU�@Table
				tmp_tbl=new TDMATable;
				TDMA_Tbl->next_tbl=tmp_tbl;
				tmp_tbl->pre_tbl=TDMA_Tbl;
				TDMA_Tbl=tmp_tbl;
			}
		}
		colorid++;
	}

	/*===========================================
				�ݬO�_�n����ĤG��superslot
	===========================================*/
	int Mincolor;
	bool Doneflag=false;

	switch(method){

	case 1:
		//-----------------------------------�[�W����superslot�A���C�@�ʥ]�A�ɪ��Ǩ�GW (�w��2~ hop���`�I)	
		do{
			//----------------------����̤pcolor
			Mincolor=colorid+1;
			node=Head->nextnd;
			while(node!=NULL){
				if(node->TDMArecv_flag==1 && node->color<Mincolor){
					Mincolor=node->color;
				}
				node=node->nextnd;
			}
	
			//----------------------Assign TDMA {slot,node}
			node=Head->nextnd;
			while(node!=NULL){
				if(node->TDMArecv_flag==1 && node->color==Mincolor){
			
					//assign pair{slot,node}
					TDMA_Tbl->slot=colorid;
					TDMA_Tbl->n1=node;
					TDMA_Tbl->count=0;

					tmp_tbl=new TDMATable;
					TDMA_Tbl->next_tbl=tmp_tbl;
					tmp_tbl->pre_tbl=TDMA_Tbl;
					TDMA_Tbl=tmp_tbl;

					node->TDMArecv_flag=0;
				}
				node=node->nextnd;
			}
			colorid++;

			//----------------------Check all node flag is 0
			Doneflag=false;
			node=Head->nextnd;
			while(node!=NULL){
				if(node->TDMArecv_flag==1){
					Doneflag=true;
				}
				node=node->nextnd;
			}

		}while(Doneflag);
		break;

	default:
		break;
	}

	//��TDMA Table����
	TDMA_Tbl=TDMA_Tbl->pre_tbl;
	TDMA_Tbl->next_tbl=NULL;

	TDMA_Tbl=MainTable;
}