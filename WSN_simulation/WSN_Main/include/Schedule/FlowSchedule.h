extern long int Timeslot;
extern double totalevent;			//Event�ƶq
extern bool Meetflag;				//�ݬO�_meet deadline
extern int ReadyQ_overflag; 
extern double Maxrate;				//�̰��t�׬�20bytes/slot
extern double payload;				//payload �� 20bytes
extern int Maxbuffersize;			//Maxbuffersize �� 4��packets
extern int Pktsize;					//�p��IntervalPower��pkt num

extern int EXECBclock;				//��DIF�PLazy �p�ɾ�
extern int dec_cof;					//Lazy��decrease�Y��
extern int Callbackclock;			//��DIF�PLazy �p�ɾ�
extern int overheadcount;
extern FrameTable *Cycle;
extern short int pollingcount;

void Schedule(int,int);	//Write-Request method, Service Interval method
/*=====================
	Node Packet setting
	Main Sche
=====================*/
void PacketQueue();
void NodeBufferSet(Node *);
void BLE_EDF(Node *);
void Write_Request();

/*=====================
	Write-Request��k
=====================*/
void NPEDF();			//Non-premptive Earliest Deadline First
void RoundRobin();		//Sequence request data in one cycle
void EIF();				//Earliest Interval First
void Polling();			//Sequnce request data with different cycle

void SingleNodeSchedule(int);
void LazyOnWrite();
void LazyIntervalCB();
void DIFCB();

/*=====================
	�T�{	�C�@packet
	�S��miss 
=====================*/
void CheckPkt();
void Finalcheck();

void Schedulability();
