/*=================================
		Structure
==================================*/
struct Flow{
	struct Packet* pkt;
	struct Node* sourcenode;
	struct Node* destinationnode;
};
struct PacketBuffer{
	double pktsize;	//1~6 packets
	int load;		//0~120bytes
	Packet* pkt;
};
struct Node{
	int id;					//Node ID
	short int hop;			//range 1~3
	short int color;		//�C��
	bool EvtArrival;
	string CMD;				//�q���ʧ@ "IDLE" "Notify" "SCAN" (�u�|�OConnSet)
	int FrameSize;			//TDMA������FrameSize
	char16_t Notify_evtcount;	//Notify evt�ƶq (typedef uint16_t char16_t)
	char16_t Tran_evtcount;		//Tran evt�ƶq (typedef uint16_t char16_t)

	Node* SendNode;			//�ǰe�`�I
	double coor_x,coor_y;	//�y��
	double radius;
	double distanceto_BS;	//��Base station �Z��
	double energy;
	double avgenergy;
	double EIMA_avgcurrent;
	double lifetime;
	double eventinterval;	//connection interval or advertisement interval
	short int ExTimeslot;
	short int LatestTimeslot;
	short int edge;			//�s���ƥ�
	bool order_flag;		//�bcoloring�ɡA�O�_��L
	short int arrival_flag;	//0->Interval�å���B1->�warrival�B-1->���ǿ駹��
	double miss_ratio;

	string State;			//Wakeup, Sleep, Transmission, Idle & Scan
	Packet* pkt;
	Packet* pktQueue;
	Node* nextnd;
	Node* prend;
	Node* ChildNode;//�l�`�I
	Node* next_child;

	Node* RecvNode;//�����`�I
	Node* next_recvnode;
	Packet* RecvPkt;
	Packet* next_recvpkt;
	int TDMArecv_flag;

	PacketBuffer* NodeBuffer;

	int ScanWin;
	int ScanInter;
	int AdvInter;

	bool ScanFlag;
	double ScanDuration;
	double EXEScanDuration;
	double SCAN_Compute(int ScanWin, int ScanInter, int AdvInter, int device){
		double AdvDelay=0.5*AdvInter+(10*(ScanWin/ScanInter)+AdvInter*((ScanInter-ScanWin)/ScanInter));
		double SCANDelay=AdvDelay*exp((2*device)/(3*AdvDelay));

		return SCANDelay;
	};
};

struct Packet{
	int id;
	int nodeid;
	int load;
	int exeload;
	double arrival;
	double period;
	double deadline;
	double utilization;
	int	 pirority;
	int	 doneflag;
	int	 readyflag;
	int	 searchdone;
	double eventinterval;
	string State;		//Transmission �B Idle

	short int hop;		//range 1~3
	short int exehop;		//range 1~3
	int destination;	//Nest node
	double rate;
	double SCAN_Duration;	//

	double CMP_D;		//�P�_�O�_�b��deadline�earrival�A��=>���W�|�[ �_=>Miss_count++ �é��W�|�[ (���FlowSchedule�UCheckPkt method)
	double Miss_count;	//Miss counting (���FlowSchedule�UCheckPkt method)
	double latency;			//�p��miss�ɪ�latency
	double meetlatency;		//�p��duration of sensor sensing arrival to tranmission
	double meetlatency_cnt;

	Node* node;

	Packet* nextpkt;		//Global packet link
	Packet* prepkt;			//Global packet link
	Packet* nodenextpkt;	//Local node packet link
	Packet* nodeprepkt;		//Local node packet link
	
	Packet* readynextpkt;	//Global ready packet link
	Packet* readyprepkt;	//Global ready packet link
	Packet* nodereadynextpkt;	//local node ready packet link
	Packet* nodereadyprepkt;	//local node ready packet link
	
	Packet* next_recvpkt;

	Packet* buffernextpkt;
};
struct Edge{
	Node *n1;
	Node *n2;

	Edge *next_edge;
	Edge *pre_edge;
};

struct FrameTable{
	bool Currentflag;
	
	short int id;
	double arrival;
	double Period;
	double Size;
	double Utilization;
	double Deadline;
	
	Node *ConnNode;
	Node *AdvNode;
	
	FrameTable* next_tbl;
	FrameTable* pre_tbl;

	FrameTable* polling_next;
};

struct TDMATable{
	short int slot;
	Node *n1;

	bool currslot;	//�O�_���{�b���檺slot
	double count;	//��slot�һݵ��ݮɶ�
	
	TDMATable * next_tbl;
	TDMATable * pre_tbl;
};
struct DIFtable{
	double load;
	double length;
	double density;
	double arrival;
	double deadline;
};

/*===========================
	Extern Global Variable
===========================*/

extern Edge *HeadEdge;
extern Edge *MainEdge;
extern Edge *ConflictEdge;
extern TDMATable *TDMA_Tbl;		//�D�n�ǿ骺TDMA�C��
extern FrameTable* FrameTbl;	//���Frame�W��schedule��
extern PacketBuffer* Buffer;
extern Node* SetHead;
extern Node* Head;
extern Packet* Headpacket;
extern Node *SetNode;
extern Node* node;	
extern Packet* packet;
extern Packet *ReadyQ;
extern Flow *Headflow;
extern stringstream stream;
extern int nodenum;
extern int nodelevel1;
extern int nodelevel2;
extern int pktnum;
extern long int Hyperperiod;
extern string str_coor_x,str_coor_y,str_radius;
extern string strload,strperiod,strutilization,strhop;

/*===========================
	�إ�Node & Packet
	�� Link List
===========================*/
void StructGEN();