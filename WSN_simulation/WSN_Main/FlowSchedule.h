extern long int Timeslot;
extern double totalevent;			//Event�ƶq
extern bool Meetflag;				//�ݬO�_meet deadline
extern int ReadyQ_overflag; 
extern double Maxrate;				//�̰��t�׬�20bytes/slot
extern double payload;				//payload �� 20bytes
extern int Maxbuffersize;			//Maxbuffersize �� 6��packets
extern int Pktsize;					//�p��IntervalPower��pkt num
extern int TDMASlot;

void FlowEDF();
void PacketQueue();
void BufferSet();
void MainSchedule(int ,bool );
void Schedulability();

void NodeBufferSet(Node *);
void BLESchedule(int, bool);
void BLE_EDF();