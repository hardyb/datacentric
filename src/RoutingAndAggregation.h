#ifndef RoutingAndAggregation_h
#define RoutingAndAggregation_h

#define MY_C_PLUSPLUS 1

#define GRAD_FILES 1

#ifdef MY_C_PLUSPLUS
#include<iostream>
#include<sstream>
#include<fstream>
#endif


#include "userHeader.h"





#define NAMEEND 0
#define DOT 255


#define STATE 0
#define CONTEXT 1

#define SUPPLY              1
#define DEMAND              2
#define ENVIRONMENTAL       3
#define VENDOR              4

#define OCCUPANCY           1
#define TEMP                2
#define LIGHT               3

#define BID                 1
#define CURRENT             2


#define EVENTBASED                  128
#define QUERYBASED                  0
#define COLLABORATIONBASED          64

/*
static char STATENAME[][][] =
{
    {
        #define NOTUSED1            0
        {"NOTUSED1"},
        #define SUPPLY              1
        {"SUPPLY"},
        #define DEMAND              2
        {"DEMAND"},
        #define ENVIRONMENTAL       3
        {"ENVIRONMENTAL"},
        #define VENDOR              4
        {"VENDOR"}
    },
    {
        {""},
        {""}
    },
};
*/





struct NameNode
{
	char* Data;
	struct NameNode *Next;
};




typedef struct state
{
	char full_name[100];
	int eg2;
	int eg3;
}state;



typedef struct context
{
	char full_name[100];
	int eg2;
	int eg3;
}context;



typedef struct trie
{
   struct State* s;
   struct context* c;
   unsigned char keyelem;
   struct trie *first_child;
   struct trie *next_sibling;
   struct trie *rec;
}trie;





/*

		ss >> current >> in >> out >> action >> condition;


?, TOTAL_CONSUMPTION, ?, 1, 0
?, TOTAL_CONSUMPTION_THRESHOLD, ?, 1, 0


NO_THRESHOLD, TOTAL_CONSUMPTION_THRESHOLD, UNDER_THRESHOLD, ?, ?


UNDER_THRESHOLD, TOTAL_CONSUMPTION_THRESHOLD, OVER_THRESHOLD, ?, ?
OVER_THRESHOLD, TOTAL_CONSUMPTION_THRESHOLD, UNDER_THRESHOLD, ?, ?




UNDER_THRESHOLD, TOTAL_CONSUMPTION_THRESHOLD, OVER_THRESHOLD, ?, ?

UNDER_THRESHOLD, TOTAL_CONSUMPTION, OVER_THRESHOLD, ?, ?

UNDER_THRESHOLD, GRABBING, IMPENDING_THRESHOLD, ?, ?



 UNDER_THRESHOLD, TOTAL_CONSUMPTION_THRESHOLD, OVER_THRESHOLD, ?, ?






 */









#define MAXLEVELS 10

#define OBTAIN 1
#define DELIVER 2
#define REINFORCE_DELIVER 3
#define REINFORCE_OBTAIN 4
#define DELIVER_CORRECTION 5
#define DELIVER_NOCHANGE 6





#define ANY_STATE 0
//#define UNKNOWN_INTERFACE 65534
#define COST_ZERO 0

#define FALSE 0
#define TRUE 1




//#define DEBUG

//#ifndef max
//	#define max( a, b ) ( ((a) > (b)) ? (a) : (b) )
//#endif

//#ifndef min
//	#define min( a, b ) ( ((a) < (b)) ? (a) : (b) )
//#endif


#define ADVERT 0
#define INTEREST 1 // Need to look at this - it is being used by two bits of code differently!!!!!!!!!
#define REINFORCE 2
#define DATA 3
#define NEIGHBOR_BCAST 4
#define NEIGHBOR_UCAST 5
#define REINFORCE_INTEREST 6
#define COLLABORATION 7
#define REINFORCE_COLLABORATION 8
#define INTEREST_CORRECTION 9
#define BREAKAGE 10
#define REINFORCE_INTEREST_CANCEL 11



#define SINK_ACTION 1
#define SOURCE_ACTION 2
#define FORWARD_ACTION 3
#define TRANSFORM_ACTION 4
#define COLLABORATE_ACTION 5
#define COLLABORATE_INITIATOR__ACTION 6


// Carefull here when we iterate we may end up wanting to make
// a state a source and a source prefix
// need to check what code is dependant on these
// also in long term we probably need to use bit flags
// ideas for this below
#define FORWARD_AND_SOURCEPREFIX 10
#define FORWARD_AND_NOTSOURCEPREFIX 11



// say 8 or 16 bits
/*
 * BIT...
 * 1 - SOURCE        - means
 * 2 - SINK          - means
 * 3 - RECORD        - means
 * 4 - PUBLICATION   - means
 * 5 - COLLABERATION - means
 * 6 - PREFIX        - means
 * 7 - NONPREFIX     - means
 * 8 - SOURCE - means
 * 1 - SOURCE - means
 * 1 - SOURCE - means
 * 1 - SOURCE - means
 */













#define MAX_COST 65535

extern unsigned char nodeConstraint;
extern char nodeScope;
extern NEIGHBOUR_ADDR thisAddress;


////////////////////////////////////////////////////////






// action indexes
#define _null_action	0
#define _assign1		1
#define _assign2		2
#define _messageIn		3
#define _deliver		4
#define _messageIn		5
#define _gtr			6
#define _ngtr			7
#define _copyMessageIn	8
#define _forward		9
#define _addto			10



// data names
#define CONSUMPTION		5
#define THRESH			6
#define GRAB			7
#define PAUSE			8
#define TOTAL_CONSUMPTION	60
#define TOTAL_CONSUMPTION_THRESHOLD	60
#define NO_THRESHOLD		61
#define UNDER_THRESHOLD		62
#define IMPENDING_THRESHOLD	63
#define OVER_THRESHOLD		64
#define GRABBING			65










/////////////////////////////////////////////////////////




#define MESSAGE_ARRIVED 0x01
//unsigned char raw_data[100];







#define MESSAGE_SIZE 121




struct dataname_struct1
{
	int the_dataname;
};


struct dataname_struct2
{
	// (pub/rec) want to keep type some how probably as part of the top level name string
	unsigned char type;
	// this is now replaced with our hierarchical context idea
	unsigned char scope;  // this is the starting point for a simple scope setting see ** below
	unsigned char unused1;
	unsigned char unused2;
};


struct dataname_struct3
{
	unsigned short short1;
	unsigned short short2;
};



union dataname_struct // 4 bytes
{
	dataname_struct1 _dataname_struct1;
	dataname_struct2 _dataname_struct2;
	dataname_struct3 _dataname_struct3;
};



struct transition
{
	char source;
	char type;
	char param1;
	char param2;
	char dest;
};





struct message
{
	char message_type;			// assume size 1
	int data_value;			// assume size 4
	unsigned short path_value;	// assume size 2
	char spacer[117];				// assume size 20
};



struct data_message
{
	char message_type;			// assume size 1
	dataname_struct data_value; // assume size 4
	signed short in1;			// assume size 2
	signed short in2;			// assume size 2
	signed short in3;			// assume size 2
	char spacer[113];			// assume size 16
};



/* SIMPLE START
 *
 * Ideally we should implement maybe attribute value pairs for keys and
 *
 * In the case of consumption data
 * in1 - current consumption
 * in2 - maximum consumption
 * in3 - time to run in minutes
 */




/*
transition FSM[NUM_TRANSITIONS] =
{
IDLE_STATE, {_messageIn, THRESH, 0, 0}, ASSIGN_TH,
IDLE_STATE, {_messageIn, CONSUMPTION, 0, 0}, ASSIGN_CO,
IDLE_STATE, {_messageIn, GRAB, 0, 0}, CHECK,
ASSIGN_TH, {_assign1, _IN1, _VAR1, 0}, IDLE_STATE,
ASSIGN_CO, {_assign1, _IN1, _VAR2, 0}, IDLE_STATE,
CHECK, {_gtr, _VAR2, _IN1, _VAR1}, SENDPAUSE,
CHECK, {_ngtr, _VAR2, _IN1, _VAR1}, IDLE_STATE,
SENDPAUSE, {_deliver, PAUSE, 0, 0}, IDLE_STATE
};
*/




/*
struct fsm_message
{
	char message_type;			// assume size 1
	transition t[10];				// 120 bytes
};
*/


struct new_packet
{
	unsigned char message_type;
	unsigned char length;
	unsigned char* data;
	unsigned short path_value;
    NEIGHBOUR_ADDR excepted_interface;
    NEIGHBOUR_ADDR down_interface;
    char seqno;
};



struct control_data
{
    NEIGHBOUR_ADDR incoming_if;
    unsigned char incoming_lqi;
};


/*
struct new_packet
{
	unsigned char message_type;
	unsigned char length;
	unsigned char* data;
	signed short path_value;
};
*/





union npacket
{
	unsigned char packet_bytes[MESSAGE_SIZE];
	message the_message;
	data_message the_data_message;
	//fsm_message the_fsm_message;
};





union rpacket
{
	unsigned char packet_bytes[MESSAGE_SIZE];
	message the_message;
	data_message the_data_message;
	//fsm_message the_fsm_message;
};



typedef struct Interface
{
	NEIGHBOUR_ADDR iName;
	int up; /* this is used as a bool */
	int type;
	unsigned char lqi;
};


typedef struct KDGradientNode;

struct InterfaceList
{
	 struct Interface* i;
	 struct InterfaceList *link;
};


typedef struct InterfaceList InterfaceList;




struct PacketQueue
{
     struct new_packet* i;
     struct PacketQueue *link;
};






//dataname?

//comms=record
//type=consumption
//region=room1


#define MSB 128  // 0b10000000
#define MSB2 192 // 0b11000000
#define PUBLICATION 128
#define RECORD 0



// Early data naming scheme more work needed

// MSB 1 = PUBLICATION 0 = RECORD
// type of data signified in next 7 bits - 0 to 127
// next byte is the region (rooms etc) - 0 to 255











/*

 ** In  the long term this will be used to indicate a kind of scope or namespace for nodes and names
 ** It will not be strictly hierarchical.  That is to say a node or name may for instance be in
 ** the scope of room1 but not in the scope of property1 in which that room exists.
 **
 ** However in practice it will be common to make all nodes in the property and then make certain
 ** nodes in certain rooms or regions
 **
 ** The utility may also be a named scope and owners of nodes may if they wish place none, often one,
 ** but also some or all of their nodes in the utility namespace.
 **
 ** an advert or interest or data message will only be forwarded or used if all its namespace names are
 ** amongst those at the node
 **
 **
 ** For our earlier experiment we will just 4 lots of 3 names for each 2 bits of the scope byte
 **
 ** 1, 2, 3
 ** 4, 8, 12
 ** 16, 32, 48
 ** 64, 128, 192
 **
 ** e.g. UTILITY1 | PROPERTY1 | ROOM1

*/


#define PROPERTY1	1
#define PROPERTY2	2
#define PROPERTY3	3
#define ROOM1		4
#define ROOM2		8
#define ROOM3		12
#define UTILITY1	16
#define UTILITY2	32
#define UTILITY3	48





// ** In the long













typedef struct State
{
	// Consider having state as just the int
	// and in future maybe a string
	// but possibly never a structure - no additional members needed



	// this is currently used as the key in the BST
	// but in a trie the key is the series of nodes in the tree
	//dataname_struct dataName;



	struct KDGradientNode* bestGradientToObtain;
	int bestGradientToObtainUpdated; /* used as a bool */
	struct InterfaceList* obtainInterfaces;
	struct Interface* obtainInterface;
	struct KDGradientNode* bestGradientToDeliver;
	int bestGradientToDeliverUpdated; /* used as a bool */
	struct InterfaceList* deliveryInterfaces;
    struct PacketQueue* pktQ;
	char seqno;
	char converged;
	char broken;
	int action;
	// consider putting some of this stuff into bit flags
	// pos some how combine updated flags and converged flags
	// save space variously
};




typedef struct QueueDeletion
{
    new_packet* pktToDelete;
    State* associatedState;
};




struct StateNode
{
	struct State* s;
	struct StateNode* left;
	struct StateNode* right;
};

typedef struct StateNode StateNode;


struct InterfaceNode
{
	struct Interface* i;
	struct InterfaceNode* left;
	struct InterfaceNode* right;
};

typedef struct InterfaceNode InterfaceNode;





typedef struct KDGradientNode
{
	// just these for now
	// also consider that maybe keys should be an array of State pointers
	// OK NOW WE USING POINTERS AND NEWKDNODE() creates or gets accordingly
	struct State* key1;
	struct Interface* key2;
	unsigned short costToObtain;
	unsigned short costToDeliver;
	char seqno;
	int deliveryReinforcement; /* used as a bool*/ // OLD - NOT USED?
	int obtainReinforcement; /* used as a bool*/ // OLD - NOT USED?
	struct KDGradientNode* left;
	struct KDGradientNode* right;


};




struct KDNode
{
	struct State* keys[2];
	struct State* newState;
	int transitionAction;
	int transitionCondition;
	struct KDNode* left;
	struct KDNode* right;
};

typedef struct KDNode KDNode;



typedef struct RoutingData
{
	// pos replace with top_state
	struct StateNode* stateTree;

	struct InterfaceNode* interfaceTree;
	struct KDGradientNode* grTree;
	struct PacketQueue* pktQ;


	struct KDNode* kdRoot;
	//struct trie* contexts;

	struct trie* top_state;
	struct trie* top_context;
    unsigned int pkts_received;
    unsigned int pkts_ignored;





	unsigned char flags;

	int currentState;
	unsigned char* role[4];
};



extern struct RoutingData* rd;

/*
#ifdef MY_C_PLUSPLUS
extern "C" {
#endif
*/


void removeIF(struct InterfaceList** l, struct Interface* _i);
void add(struct InterfaceList** q, struct Interface* _i);
void delete_queued_data(void* relevantObject);
StateNode* newStateNode(int stateDataname);
InterfaceNode* newInterfaceNode(int interfaceName);
void freeInterfaceNode(InterfaceNode* n);
StateNode* InsertStateNode(StateNode** treeNode, int stateDataname);
InterfaceNode* InsertInterfaceNode(InterfaceNode** treeNode, NEIGHBOUR_ADDR interfaceName, int* inserted);
bool TraversStateNodes(StateNode* tree, void process(State* s));
bool TraversInterfaceNodes(InterfaceNode* tree, State* s, void process(Interface* i, State* s));
unsigned int CountInterfaceNodes(InterfaceNode* tree);
unsigned int TotalNeighborLqi(InterfaceNode* tree);
void MinMaxNeighborLqi(InterfaceNode* tree, unsigned int* maxlqi, unsigned int* minlqi);
unsigned int AverageNeighborLqi(InterfaceNode* tree);
bool TraversGradientNodes(struct KDGradientNode* tree, void process(KDGradientNode* g, unsigned char* _name));
bool deliverReinforced(KDGradientNode* g);
bool obtainReinforced(KDGradientNode* g);
StateNode* FindStateNode(StateNode* tree, int val);
InterfaceNode* FindInterfaceNode(InterfaceNode* tree, NEIGHBOUR_ADDR val);
struct KDGradientNode* newKDGradientNode(int sName, int iName, int obtain, int deliver);
void freeKDGradientNode(KDGradientNode* g);
struct KDGradientNode* insertKDGradientNode( int sName, int iName, int costType, int pCost, struct KDGradientNode* treeNode, int lev );
void reinforceDeliverGradient(char* fullyqualifiedname, NEIGHBOUR_ADDR iName);
void reinforceObtainGradient(int sName, NEIGHBOUR_ADDR iName);
void setObtainGradient(int sName, int iName, int pCost);
void setDeliverGradient(int sName, int iName, int pCost);
KDNode* newKDNode(int keys[], int newStateName);
KDNode* insertKDNode( int stateDataname[], int newStateName, KDNode* treeNode, int lev );
//struct KDGradientNode* SearchForKDGradientNode( int sName, int iName, struct KDGradientNode* treeNode);
struct KDGradientNode* SearchForKDGradientNode1( unsigned char* _data, int iName, struct KDGradientNode* treeNode);
struct KDGradientNode* SearchForKDGradientNode2( State* s, Interface* i, struct KDGradientNode* treeNode);
KDNode* SearchForKDNode( int stateDataname[], KDNode* treeNode );
int getNewState(int input);
int SearchForKDNodeTransition(int input);

struct KDGradientNode* insertKDGradientNode2(State* s, Interface* i, int costType, int pCost, struct KDGradientNode* treeNode, int lev, char seqno);

int consider_reinforce_interest(State* s, unsigned char* _buf, NEIGHBOUR_ADDR _if);


/*
#ifdef MY_C_PLUSPLUS
}
#endif
*/



#ifdef MY_C_PLUSPLUS
//void PrepareKDNodeDisplayStream(KDNode** treeNode, int level, std::ostringstream* os);
void PrepareKDGradientNodeDisplayStream(KDGradientNode** treeNode, int level, std::ostringstream* os);
//void DisplayKDTree(KDNode** treeNode, std::ofstream& infile);
void DisplayKDGradientTree(KDGradientNode** treeNode, std::ofstream& infile);
void DisplayNodeDetails(std::ofstream& infile);

#endif



/*================================ MESSAGE HANDLERS ==============================*/

/*
void handle_advert(NEIGHBOUR_ADDR _interface);
void handle_interest(NEIGHBOUR_ADDR _interface);
void handle_reinforce(NEIGHBOUR_ADDR _interface);
void handle_data(NEIGHBOUR_ADDR _interface);
void handle_neighbor_bcast(NEIGHBOUR_ADDR _interface);
void handle_neighbor_ucast(NEIGHBOUR_ADDR _interface);
*/

void setMessageCallBack(void (*_sendAMessage) (NEIGHBOUR_ADDR _interface, unsigned char* _msg));
void setBroadcastCallBack(void (*_bcastAMessage) (unsigned char* _msg));
void setApplicationCallBack(void (*_handleApplicationData) (unsigned char* _msg));
void setTimerCallBack(void (*_setTimer) (TIME_TYPE timeout, void* relevantObject, void timeout_callback(void* relevantObject)));
void weAreSourceFor(unsigned char* _data);
void weAreSinkFor(unsigned char* _data, char seqno);
int UcastAllBestGradients(trie* t, NEIGHBOUR_ADDR inf);
void weAreCollaboratorFor(unsigned char* _data);
void weAreCollaboratorInitiatorFor(unsigned char* _data);
void send_data(int len, unsigned char* _data);
void handle_message(unsigned char* _msg, NEIGHBOUR_ADDR inf, unsigned char lqi);
void read_packet(unsigned char* pkt);
unsigned int size_needed_for_outgoing_packet();
unsigned int sizeof_existing_packet(unsigned char* pkt);
unsigned int sizeof_existing_packet_withoutDownIF(unsigned char* pkt);
void handle_advert(control_data cd);
void handle_interest(control_data cd);
void handle_reinforce(control_data cd);
void start_reinforce_interest(unsigned char* fullyqualifiedname, NEIGHBOUR_ADDR _if, char seqno);
void handle_reinforce_interest(control_data cd);
void handle_data(control_data cd);
void handle_neighbor_bcast(control_data cd);
void handle_neighbor_ucast(control_data cd);
void handle_collaboration(control_data cd);
void handle_reinforce_collaboration(control_data cd);
void handle_interest_correction(control_data cd);
void handle_breakage(control_data cd);
void interest_breakage_just_ocurred(unsigned char* pkt, NEIGHBOUR_ADDR inf);
void StartUp();
void self_message(void * msg);
void regular_checks(void);
void getLongestContextTrie(trie *t, unsigned char* str, unsigned char* i, unsigned char* longestContext);
void getShortestContextTrie(trie *t, unsigned char* str, unsigned char* i, unsigned char* shortestContext);


void UpdateBestGradient(Interface* i, State* s);

void InterfaceDown(unsigned char* pkt, NEIGHBOUR_ADDR inf);




state* state_new(void);
context* context_new(void);
trie* trie_new(void);
void trie_free(trie* t);
trie* trie_at_level(trie *t, unsigned char c);
trie* trie_add(trie *t, unsigned char *str, int object);
void write_connections(void process(State* s, unsigned char* _data, NEIGHBOUR_ADDR _if));
bool write_gradients(void process(KDGradientNode* g, unsigned char* _name));
void UpdateGradientFile();



/*
BEST DESCRIPTION
This function trie_lookup_longest_prefix_extra2 (needs renaming)
Is the one used to check an incoming reinforcement or data message against
already recorded gradients.  The gradient will include
the hierarchical data name followed by the hierarchical context name
the match is true if every name found in the record is a prefix of
every name in the incoming message
*/
trie* trie_lookup_longest_prefix_extra2(trie *t, const char *str);


void action_all_prefixes(trie *t, int i, int n, unsigned char *str, unsigned char* buf, NEIGHBOUR_ADDR _if, int process(State* s, unsigned char* _buf, NEIGHBOUR_ADDR _if));



/*
BEST DESCRIPTION
This function is used to match the context name part of the incoming interest
or advertisement messages against the recorded hierarchical contexts at the node
the message is passing through.

A match is made if the context name in the message is a prefix of a hierarchical
context name in the given context record at the node

*/
trie* trie_lookup2(trie *t, unsigned char *str);




trie* f(trie *t, int i, int n, const char *str, trie *rec);
void new_one(trie *t, const char *str, NEIGHBOUR_ADDR _if);



// The application related data names
void addName(NameNode** Head, char* name);
void iterateNameData(NameNode* Head);
void iterateNameDataForSpecificPurpose(NameNode* Head);



















#endif



