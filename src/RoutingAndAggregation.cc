//SearchForKDGradientNodereinforceObtainGradientbestGradientToObtainUpdated

//#include "contiki.h"
#define NULL 0

unsigned char current_prefix_name[100];

char advertRetries;  // needs improving
char advertReinforceRetries; // needs improving


#include "RoutingAndAggregation.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>



#ifdef ROUTING_DEBUG
#define RD(x) std::cout << x
#else
#define RD(x)
#endif







//#define USE_NODE_STABILITY_COST 1

//#define MAKE_LOCAL_VARIABLES_FOR_DEBUGGER 1



//#define RETAFTERUCAST 0
#define TYPETHROUGH 0 //
#define TYPERECV 0 //
#define TEST3 0 //
#define TEST4 0 //
#define TEST5 0 //
#define TEST6 0 //
#define TEST7 0 //
#define TEST8 0 //
#define TEST9 0 //



//#define DBRETAFTERUCAST(thecode) if ( RETAFTERUCAST ) { thecode }
#define DBTYPETHROUGH(thecode) if ( TYPETHROUGH ) { thecode }
#define DBTYPERECV(thecode) if ( TYPERECV ) { thecode }
#define DB3(thecode) if ( TEST3 ) { thecode }
#define DB4(thecode) if ( TEST4 ) { thecode }
#define DB5(thecode) if ( TEST5 ) { thecode }
#define DB6(thecode) if ( TEST6 ) { thecode }
#define DB7(thecode) if ( TEST7 ) { thecode }
#define DB8(thecode) if ( TEST8 ) { thecode }
#define DB9(thecode) if ( TEST9 ) { thecode }










/*
TODO

Need to consider the stack in this code.
Since the framework is for multi-platform
i.e. 
how big it is 
whether it is maintained across certain calls
etc

*/


NameNode* SinkNames = NULL;
NameNode* SourceNames = NULL;


char queue[100];


//static int dataRate = 0;


void (*sendAMessage) (NEIGHBOUR_ADDR _interface, unsigned char* _msg, double _creationTime);
void (*handleApplicationData) (unsigned char* _msg, double _creationTime);
void (*bcastAMessage) (unsigned char* _msg);


// For the moment just provide a single time out
// the application code can just same identifier and cancel
// and reset timer if there is a repeat call to the  function
void (*setTimer) (TIME_TYPE timeout, void* relevantObject, void timeout_callback(void* relevantObject));





//static int currentState = 1;
static int K = 2;


struct RoutingData* rd;

//union rpacket incoming_packet;
//union rpacket outgoing_packet;

struct new_packet incoming_packet;
struct new_packet outgoing_packet;
struct new_packet broken_packet;


struct new_packet* sending_packet;


//unsigned char rMessage[MESSAGE_SIZE];
//for( unsigned i = 0; i < MESSAGE_SIZE; i++ )
//{
//	rMessage[i] = rmsg->getData(i);
//}

// Framework call
//rpacket p;

// CHECK MEMCPY IS MEMORY SOUND IF UNCOMMENTINBELOW
//memcpy(p.packet_bytes, rMessage, MESSAGE_SIZE);
//EV << "Received: " << messageName[p.the_message.message_type] << "\n";
//handle_message(rMessage, inf);

//void handle_message_breakup(unsigned char* _msg, NEIGHBOUR_ADDR inf)
//{
// CHECK MEMCPY IS MEMORY SOUND IF UNCOMMENTINBELOW
//	memcpy(incoming_packet.packet_bytes, _msg, MESSAGE_SIZE);
//	(*h[incoming_packet.the_message.message_type]) (inf);
//
//}





/*
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
*/

// CODE FOR SIMULATION ONLY AND BASED ON COMMON 32BIT TYPE SIZES
// =============================================================
/*
unsigned int getPacketSize(char messageType, unsigned int _dataSize)
{
    unsigned int messageTypeSize       = 1;
    unsigned int dataLenValSize        = 1;
    unsigned int dataSize              = _dataSize;
    unsigned int pathValueSize         = 2;
    unsigned int exceptedInterfaceSize = 8;
    unsigned int downInterfaceSize     = 8;
    unsigned int seqnoSize             = 1;

    unsigned int pktSize;


    switch ( messageType )
    {
        case INTEREST:
            pktSize = messageTypeSize +
                        dataLenValSize +
                        dataSize +
                        pathValueSize +
                        seqnoSize;
            return pktSize;
            break;
        default:
            break;
    }

}
*/




void read_packet(unsigned char* pkt, double creationTime)
{
    incoming_packet.creation_time = creationTime;
	incoming_packet.message_type = pkt[0];
	incoming_packet.length = pkt[1];
	free(incoming_packet.data);
	incoming_packet.data = (unsigned char*)malloc(incoming_packet.length+1);
	if (!incoming_packet.data)
	{
	    memcpy(incoming_packet.data, &(pkt[2]), incoming_packet.length);
	}
    //memset(incoming_packet.data, 0, sizeof(incoming_packet.length+1));
	memcpy(incoming_packet.data, &(pkt[2]), incoming_packet.length);
	incoming_packet.data[incoming_packet.length] = 0;

	//char lsb = pkt[2+incoming_packet.length];
	//char msb = pkt[2+incoming_packet.length+1];
	//incoming_packet.path_value = (signed short)pkt[2+incoming_packet.length];

	// need to take care that this is the right byte to use to ref the 2 bytes
	// ie msb? lsb? etc...
	signed short* x = (signed short*)(pkt+2+incoming_packet.length); // cast from 1st byte in buffer ie lsb
	incoming_packet.path_value = *x;

	NEIGHBOUR_ADDR* excepted_interface_ptr = (NEIGHBOUR_ADDR*)(pkt+2+incoming_packet.length+
	        sizeof(signed short));
	incoming_packet.excepted_interface = *excepted_interface_ptr;


    NEIGHBOUR_ADDR* down_interface_ptr = (NEIGHBOUR_ADDR*)(pkt+2+incoming_packet.length+
            sizeof(signed short) + sizeof(NEIGHBOUR_ADDR));
    incoming_packet.down_interface = *down_interface_ptr;

    char* seqn = (char*)(pkt+2+incoming_packet.length+
            sizeof(signed short) + sizeof(NEIGHBOUR_ADDR) + sizeof(NEIGHBOUR_ADDR));
    incoming_packet.seqno = *seqn;

}



void read_broken_packet(unsigned char* pkt, double creationTime)
{
    broken_packet.creation_time = creationTime;
    broken_packet.message_type = pkt[0];
    broken_packet.length = pkt[1];
    free(broken_packet.data);
    broken_packet.data = (unsigned char*)malloc(broken_packet.length+1);
    if (!broken_packet.data)
    {
        memcpy(broken_packet.data, &(pkt[2]), broken_packet.length);
    }
    //memset(broken_packet.data, 0, sizeof(broken_packet.length+1));
    memcpy(broken_packet.data, &(pkt[2]), broken_packet.length);
    broken_packet.data[broken_packet.length] = 0;

    //char lsb = pkt[2+broken_packet.length];
    //char msb = pkt[2+broken_packet.length+1];
    //broken_packet.path_value = (signed short)pkt[2+broken_packet.length];

    // need to take care that this is the right byte to use to ref the 2 bytes
    // ie msb? lsb? etc...
    signed short* x = (signed short*)(pkt+2+broken_packet.length); // cast from 1st byte in buffer ie lsb
    broken_packet.path_value = *x;

    NEIGHBOUR_ADDR* excepted_interface_ptr = (NEIGHBOUR_ADDR*)(pkt+2+broken_packet.length+
            sizeof(signed short));
    broken_packet.excepted_interface = *excepted_interface_ptr;


    NEIGHBOUR_ADDR* down_interface_ptr = (NEIGHBOUR_ADDR*)(pkt+2+broken_packet.length+
            sizeof(signed short) + sizeof(NEIGHBOUR_ADDR));
    broken_packet.down_interface = *down_interface_ptr;

    char* seqn = (char*)(pkt+2+broken_packet.length+
            sizeof(signed short) + sizeof(NEIGHBOUR_ADDR) + sizeof(NEIGHBOUR_ADDR));
    broken_packet.seqno = *seqn;

}




void path_value_into_buf(signed short _var, unsigned char* _buf)
{

    for (unsigned int i=0; i < sizeof(_var); i++)
    {
        _buf[i] = _var & 0xFF;
        _var >>= 8;
    }


}



void neighbour_addr_into_buf(NEIGHBOUR_ADDR _var, unsigned char* _buf)
{

    for (unsigned int i=0; i < sizeof(_var); i++)
    {
        _buf[i] = _var & 0xFF;
        _var >>= 8;
    }


}


unsigned int size_needed_for_outgoing_packet(new_packet* the_packet)
{
    unsigned int _size = sizeof(the_packet->message_type) + sizeof(the_packet->length)
            + the_packet->length +   sizeof(the_packet->path_value)
            +  sizeof(the_packet->excepted_interface)
            +  sizeof(the_packet->down_interface)
            +  sizeof(the_packet->seqno);
    return _size;
    /*
     * outgoing_packet.message_type 1
     * outgoing_packet.length 1
     * outgoing_packet.length (data length) e.g. "\2\2\255\6" (4)
     * outgoing_packet.path_value 2
     * outgoing_packet.excepted_interface 8
     * outgoing_packet.down_interface 8
     * outgoing_packet.seqno 1
     *
     * TOTAL 25
     */
}



unsigned int size_needed_for_outgoing_packet_old()
{
    unsigned int _size = sizeof(outgoing_packet.message_type) + sizeof(outgoing_packet.length)
            + outgoing_packet.length +   sizeof(outgoing_packet.path_value)
            +  sizeof(outgoing_packet.excepted_interface)
            +  sizeof(outgoing_packet.down_interface)
            +  sizeof(outgoing_packet.seqno);
    return _size;
    /*
     * outgoing_packet.message_type 1
     * outgoing_packet.length 1
     * outgoing_packet.length (data length) e.g. "\2\2\255\6" (4)
     * outgoing_packet.path_value 2
     * outgoing_packet.excepted_interface 8
     * outgoing_packet.down_interface 8
     * outgoing_packet.seqno 1
     *
     * TOTAL 25
     */
}



unsigned int sizeof_existing_packet(unsigned char* pkt)
{
    unsigned int _size = sizeof(outgoing_packet.message_type) + sizeof(outgoing_packet.length)
            + pkt[1] +   sizeof(outgoing_packet.path_value)
            +  sizeof(outgoing_packet.excepted_interface)
            +  sizeof(outgoing_packet.down_interface)
            +  sizeof(outgoing_packet.seqno);
    return _size;
    /*
     * outgoing_packet.message_type 1
     * outgoing_packet.length 1
     * pkt[1] (data length) e.g. "\2\2\255\6" (4)
     * outgoing_packet.path_value 2
     * outgoing_packet.excepted_interface 8
     * outgoing_packet.down_interface 8
     * outgoing_packet.seqno 1
     *
     * TOTAL 25
     */
}



unsigned int sizeof_existing_packet_withoutDownIF(unsigned char* pkt)
{
    unsigned int _size = sizeof(outgoing_packet.message_type) + sizeof(outgoing_packet.length)
            + pkt[1];

    switch ( pkt[0] )
    {
    case ADVERT:
    case INTEREST:
    case INTEREST_CORRECTION:
        _size = _size +   sizeof(outgoing_packet.path_value)+1; // seqno
                //+  sizeof(outgoing_packet.excepted_interface);  TEMP HACK
        break;
    case REINFORCE:
    case REINFORCE_INTEREST:
    case REINFORCE_INTEREST_CANCEL:
        break;
    }

    switch (outgoing_packet.message_type)
    {

    }
    return _size;
    /*
     * case ADVERT:
     * case INTEREST:
     * case INTEREST_CORRECTION:
     *
     * outgoing_packet.message_type 1
     * outgoing_packet.length 1
     * pkt[1] (data length) e.g. "\2\2\255\6" (4)
     * outgoing_packet.path_value 2
     * outgoing_packet.excepted_interface 8
     * outgoing_packet.down_interface 8
     * outgoing_packet.seqno 1
     *
     * TOTAL 25
     *
     * case REINFORCE:
     * case REINFORCE_INTEREST:
     * case REINFORCE_INTEREST_CANCEL:
     *
     * outgoing_packet.message_type 1
     * outgoing_packet.length 1
     * pkt[1] (data length) e.g. "\2\2\255\6" (4)
     * outgoing_packet.path_value 2
     * outgoing_packet.excepted_interface 8
     * outgoing_packet.down_interface 8
     * outgoing_packet.seqno 1
     *
     * TOTAL 25
     *
     */
}


new_packet* packetCopy(new_packet* currentPkt)
{
    new_packet* savePkt = (new_packet*)malloc(sizeof(new_packet));
    *savePkt = *currentPkt;
    savePkt->data = (unsigned char*)malloc(currentPkt->length+1);
    memcpy(savePkt->data, currentPkt->data, currentPkt->length);
    savePkt->data[currentPkt->length] = 0;

    return savePkt;
}







/*
 * Prepare a buffer for passing to upper layer to send
 */
unsigned char* write_packet(new_packet* the_packet)
{
    unsigned int size = size_needed_for_outgoing_packet(the_packet);
	unsigned char* pkt = (unsigned char*)malloc(size);
	unsigned int pkt_index = 0;

	pkt[pkt_index] = the_packet->message_type;
	pkt_index+=sizeof(the_packet->message_type);

	if ( (the_packet->message_type == 1) && !the_packet->length )
	{
	    RD( << "zero packet data length" << std::endl );
	}

	pkt[pkt_index] = the_packet->length;
	pkt_index+=sizeof(the_packet->length);

	memcpy(&(pkt[pkt_index]), the_packet->data, the_packet->length);
    pkt_index+=the_packet->length;

    path_value_into_buf(the_packet->path_value, &(pkt[pkt_index]));
    pkt_index+=sizeof(the_packet->path_value);

    neighbour_addr_into_buf(the_packet->excepted_interface, &(pkt[pkt_index]));
    pkt_index += sizeof(the_packet->excepted_interface);

    neighbour_addr_into_buf(the_packet->down_interface, &(pkt[pkt_index]));
    pkt_index += sizeof(the_packet->down_interface);

    pkt[pkt_index] = the_packet->seqno;

	//pkt[pkt_index] = (char)(outgoing_packet.path_value & 0xFF); // lsb goes first in buff;
	//pkt[pkt_index+1] = (char)(outgoing_packet.path_value >> 8); // msb goes second in buff;

	return pkt;
}






/*
 * Prepare a buffer for passing to upper layer to send
 */
unsigned char* write_packet_old()
{
    //outgoing_packet.length = outgoing_packet.data ? strlen((const char*)outgoing_packet.data) : 0;
    unsigned int size = size_needed_for_outgoing_packet_old();
    //unsigned int size = sizeof(outgoing_packet.message_type) + sizeof(outgoing_packet.length)
    //      + outgoing_packet.length +   sizeof(outgoing_packet.path_value)
    //        +  sizeof(outgoing_packet.excepted_interface)
    //        +  sizeof(outgoing_packet.down_interface);
    unsigned char* pkt = (unsigned char*)malloc(size);
    unsigned int pkt_index = 0;

    pkt[pkt_index] = outgoing_packet.message_type;
    pkt_index+=sizeof(outgoing_packet.message_type);

    if ( (outgoing_packet.message_type == 1) && !outgoing_packet.length )
    {
        RD( "zero packet data length" << std::endl );
    }

    pkt[pkt_index] = outgoing_packet.length;
    pkt_index+=sizeof(outgoing_packet.length);

    memcpy(&(pkt[pkt_index]), outgoing_packet.data, outgoing_packet.length);
    pkt_index+=outgoing_packet.length;

    path_value_into_buf(outgoing_packet.path_value, &(pkt[pkt_index]));
    pkt_index+=sizeof(outgoing_packet.path_value);

    neighbour_addr_into_buf(outgoing_packet.excepted_interface, &(pkt[pkt_index]));
    pkt_index += sizeof(outgoing_packet.excepted_interface);

    neighbour_addr_into_buf(outgoing_packet.down_interface, &(pkt[pkt_index]));
    pkt_index += sizeof(outgoing_packet.down_interface);

    pkt[pkt_index] = outgoing_packet.seqno;

    //pkt[pkt_index] = (char)(outgoing_packet.path_value & 0xFF); // lsb goes first in buff;
    //pkt[pkt_index+1] = (char)(outgoing_packet.path_value >> 8); // msb goes second in buff;

    return pkt;
}




/*
 * This may be for removal - StateNode BST replaced by trie
 */
/*
struct StateNode* newStateNode(int stateDataname)
{
	struct StateNode* n = (struct StateNode *)malloc(sizeof(struct StateNode));
	n->s = (struct State *)malloc(sizeof(struct State));
	n->s->dataName._dataname_struct1.the_dataname = stateDataname;
	n->s->bestGradientToObtain = NULL;
	n->s->bestGradientToDeliver = NULL;
	n->s->deliveryInterfaces = NULL;
	n->s->obtainInterface = NULL;
	n->s->obtainInterfaces = NULL;
	n->s->bestGradientToObtainUpdated = FALSE;
	n->s->bestGradientToDeliverUpdated = FALSE;
	n->s->action = FORWARD_ACTION;
	n->left = NULL;
	n->right = NULL;
	return n;
}
*/



/*
 * Keep this I think.  Looks like no changes
 */
struct InterfaceNode* newInterfaceNode(NEIGHBOUR_ADDR interfaceName)
{
	struct InterfaceNode* n = (struct InterfaceNode *)malloc(sizeof(struct InterfaceNode));
	n->i = (struct Interface *)malloc(sizeof(struct Interface));
	n->i->iName = interfaceName;
	n->i->up = TRUE;
	n->i->type = 0;
	n->left = NULL;
	n->right = NULL;
	return n;
}




void freeInterfaceNode(InterfaceNode* n)
{
    if ( n )
    {
        free(n->i);
        freeInterfaceNode(n->left);
        freeInterfaceNode(n->right);
        free(n);
    }
}





/*
 * This may be for removal - StateNode BST replaced by trie
 */
/*
 struct StateNode* InsertStateNode(struct StateNode** treeNode, int stateDataname)
 {
     if (*treeNode == NULL)
	 {
		*treeNode = newStateNode(stateDataname);
#ifdef DEBUG
		RD( "Inserted state: " << stateDataname << " = true" << std::endl );
#endif
		return *treeNode;
	 }

	 if (stateDataname == (*treeNode)->s->dataName._dataname_struct1.the_dataname )
	 {
#ifdef DEBUG
		 RD( "Inserted state: " << stateDataname << " = FALSE" << std::endl );
#endif
		 return *treeNode;
	 }

	 if (stateDataname < (*treeNode)->s->dataName._dataname_struct1.the_dataname)
	 {
	     // I think possibly returns need to occurr here also
		 InsertStateNode(&((*treeNode)->left), stateDataname);
	 }
	 else
	 {
	     // I think possibly returns need to occurr here also
		InsertStateNode(&((*treeNode)->right), stateDataname);
	 }
 }
*/




 /*
  * Keep this I think.  Looks like no changes
  *
  * Good idea to check though what happens if the interface already exists
  * A: IT CORRECTLY RETURNS THE EXISTING ONE
  *
  */
struct InterfaceNode* InsertInterfaceNode(struct InterfaceNode** treeNode, NEIGHBOUR_ADDR interfaceName, int* inserted)
//struct InterfaceNode* InsertInterfaceNode(struct InterfaceNode** treeNode, NEIGHBOUR_ADDR interfaceName)
 {
     if (*treeNode == NULL)
	 {
		*treeNode = newInterfaceNode(interfaceName);
#ifdef DEBUG
		RD( "Inserted interface: " << interfaceName << " = true" << std::endl );
#endif
		*inserted = 1;
		return *treeNode;
	 }

	 if (interfaceName == (*treeNode)->i->iName )
	 {
#ifdef DEBUG
		 RD( "Inserted interface: " << interfaceName << " = FALSE" << std::endl );
#endif
        *inserted = 0;
		 return *treeNode;
	 }

	 if ( interfaceName < (*treeNode)->i->iName )
	 {
		 return InsertInterfaceNode(&((*treeNode)->left), interfaceName, inserted);
	 }
	 else
	 {
		return InsertInterfaceNode(&((*treeNode)->right), interfaceName, inserted);
	 }
 }





 /*
  * New function for traversing a trie - specifically used for ucast all best grads
  * at the moment but may make more generic / more efficient in the future
  */
/*
 * Definitely need to make this function more generic:
 * THINGS I DON'T LIKE:
 * - Need to use this function where inf formal parameters are not really relevant
 * - eg iterating states to check for actions
 * - Don't like the global variable queue
 * - generally not as neat as i would like.
 *
 */
 void traverse(trie *t, char* str, NEIGHBOUR_ADDR inf, void process(State* s, unsigned char* _data, NEIGHBOUR_ADDR _if))
 {
 		if ( t != NULL )
 		{
 			if ( t->keyelem )
 			{
 				*(str) = t->keyelem;
 				*(str+1) = 0;
 				str++;
 			}
 			if ( t->s )
 			{
 				process(t->s, (unsigned char*)queue, inf);
 				//printf("key: %s    -    ", queue);
 				//printf("obj: %s\n", t->s->full_name);
 			}
 			t = t->first_child;
 			while(t != NULL)
 			{
 				traverse(t, str, inf, process);
 				t = t->next_sibling;
 			}
 		}
 }


 /*
  * For traversing the context trie
  * to obtain the shortest context
  * current version assumes there is only one of these!!
  */
  void getShortestContextTrie(trie *t, unsigned char* str, unsigned char* i, unsigned char* shortestContext)
  {
         if ( t != NULL )
         {
             if ( t->keyelem )
             {
                 *(i) = t->keyelem;
                 *(i+1) = 0;
                 i++;
             }
             if ( t->c )
             {
                 //if (!t->first_child)
                     strcpy((char*)shortestContext, (const char*)str);
                     return;
                     //memcpy(longestContext, queue, strlen(queue));
                     //process(t->c, (unsigned char*)queue);
                 //printf("key: %s    -    ", queue);
                 //printf("obj: %s\n", t->s->full_name);
             }

             t = t->first_child;
             while(t != NULL)
             {
                 getShortestContextTrie(t, str, i, shortestContext);
                 t = t->next_sibling;
             }
         }
  }



/*
 * For traversing the context trie
 * to obtain the longest context
 * current version assumes there is only one of these!!
 */
 void getLongestContextTrie(trie *t, unsigned char* str, unsigned char* i, unsigned char* longestContext)
 {
        if ( t != NULL )
        {
            if ( t->keyelem )
            {
                *(i) = t->keyelem;
                *(i+1) = 0;
                i++;
            }
            if ( t->c )
            {
                if (!t->first_child)
                    strcpy((char*)longestContext, (const char*)str);
                    //memcpy(longestContext, queue, strlen(queue));
                    //process(t->c, (unsigned char*)queue);
                //printf("key: %s    -    ", queue);
                //printf("obj: %s\n", t->s->full_name);
            }
            t = t->first_child;
            while(t != NULL)
            {
                getLongestContextTrie(t, str, i, longestContext);
                t = t->next_sibling;
            }
        }
 }



 void traverseContextTrie2(trie *t, char* str, void process(context* c, unsigned char* _context))
 {
        if ( t != NULL )
        {
            if ( t->keyelem )
            {
                *(str) = t->keyelem;
                *(str+1) = 0;
                str++;
            }
            if ( t->c )
            {
                process(t->c, (unsigned char*)queue);
                //printf("key: %s    -    ", queue);
                //printf("obj: %s\n", t->s->full_name);
            }
            t = t->first_child;
            while(t != NULL)
            {
                traverseContextTrie2(t, str, process);
                t = t->next_sibling;
            }
        }
 }




 void ucast_best_gradient(State* s, unsigned char* _data, NEIGHBOUR_ADDR _if)
  {
 	if ( s->bestGradientToObtain && (((*_data) & MSB2) == PUBLICATION) )
 	{
 		outgoing_packet.message_type = ADVERT;
 		outgoing_packet.length = strlen(queue); // strlen ok in this case
 		outgoing_packet.data = (unsigned char*)queue;
 		if (s->bestGradientToObtain->key2->iName == SELF_INTERFACE)
 		{
 	        outgoing_packet.path_value = 0;
 		}
 		else
 		{
#ifdef USE_NODE_STABILITY_COST
 	        outgoing_packet.path_value = s->bestGradientToObtain->costToObtain + nodeConstraint;
#else
            outgoing_packet.path_value = s->bestGradientToObtain->costToObtain;
#endif
 		}
 		outgoing_packet.excepted_interface = s->bestGradientToObtain->key2->iName;
 		outgoing_packet.down_interface = UNKNOWN_INTERFACE;
 		outgoing_packet.seqno = s->seqno;

        //sendAMessage(_if, write_packet());
        bcastAMessage(write_packet(&outgoing_packet));
 	}
 	if ( s->bestGradientToDeliver && (((*_data) & MSB2) == RECORD) )
 	{
 		outgoing_packet.message_type = INTEREST;
 		outgoing_packet.length = strlen(queue); // strlen ok in this case
 		outgoing_packet.data = (unsigned char*)queue;
        if (s->bestGradientToDeliver->key2->iName == SELF_INTERFACE)
        {
            outgoing_packet.path_value = 0;
        }
        else
        {
#ifdef USE_NODE_STABILITY_COST
            outgoing_packet.path_value = s->bestGradientToDeliver->costToDeliver + nodeConstraint;
#else
            outgoing_packet.path_value = s->bestGradientToDeliver->costToDeliver;
#endif
        }
 		outgoing_packet.excepted_interface = s->bestGradientToDeliver->key2->iName;
        outgoing_packet.down_interface = UNKNOWN_INTERFACE;
        outgoing_packet.seqno = s->seqno;

 		RD( "Sending interest packet to: " << _if << std::endl );
        //sendAMessage(_if, write_packet());
        bcastAMessage(write_packet(&outgoing_packet));
 	}


 	/*
 	 * Not quite sure how to deal with collaborations
 	 */
 	if ( s->bestGradientToDeliver && (((*_data) & MSB2) == COLLABORATIONBASED) )
    {
        outgoing_packet.message_type = COLLABORATION;
        outgoing_packet.length = strlen(queue); // strlen ok in this case
        outgoing_packet.data = (unsigned char*)queue;
        if (s->bestGradientToDeliver->key2->iName == SELF_INTERFACE)
        {
            outgoing_packet.path_value = 0;
        }
        else
        {
#ifdef USE_NODE_STABILITY_COST
            outgoing_packet.path_value = s->bestGradientToDeliver->costToDeliver + nodeConstraint;
#else
            outgoing_packet.path_value = s->bestGradientToDeliver->costToDeliver;
#endif
        }
        // Double check line below when testing collaboration stuff
        outgoing_packet.excepted_interface = s->bestGradientToDeliver->key2->iName;
        outgoing_packet.down_interface = UNKNOWN_INTERFACE;
        outgoing_packet.seqno = s->seqno;

        // ??? something here ???
        //sendAMessage(_if, write_packet());
        bcastAMessage(write_packet(&outgoing_packet));
    }


  }



bool write_gradients(void process(KDGradientNode* g, unsigned char* _name))
{
     return TraversGradientNodes(rd->grTree, process);

}



void write_connections(void process(State* s, unsigned char* _data, NEIGHBOUR_ADDR _if))
{
    traverse(rd->top_state, queue, 0, process);

}




/*
 * Updated this to use new trie traversal
 */
//int UcastAllBestGradients(StateNode* tree, NEIGHBOUR_ADDR inf)
int UcastAllBestGradients(trie* t, NEIGHBOUR_ADDR inf)
 {
	traverse(t, queue, inf, ucast_best_gradient);
	return 1;

	 /*
	 if ( tree )
	 {
			if ( tree->s->bestGradientToObtain && (tree->s->dataName._dataname_struct2.type & PUBLICATION))
			{
				//outgoing_packet.the_message.message_type = ADVERT;
				//outgoing_packet.the_message.data_value = tree->s->dataName._dataname_struct1.the_dataname;
				//outgoing_packet.the_message.path_value = tree->s->bestGradientToObtain->costToObtain + nodeConstraint;
				//sendAMessage(inf, outgoing_packet.packet_bytes);

				outgoing_packet.message_type = ADVERT;
				outgoing_packet.data = tree->s->dataName._dataname_struct1.the_dataname;
				outgoing_packet.the_message.path_value = tree->s->bestGradientToObtain->costToObtain + nodeConstraint;
				sendAMessage(inf, outgoing_packet.packet_bytes);
			}
			if ( tree->s->bestGradientToDeliver && !(tree->s->dataName._dataname_struct2.type & PUBLICATION) ) // RECORD
			{
				outgoing_packet.the_message.message_type = INTEREST;
				outgoing_packet.the_message.data_value = tree->s->dataName._dataname_struct1.the_dataname;
				outgoing_packet.the_message.path_value = tree->s->bestGradientToDeliver->costToDeliver + nodeConstraint;
				sendAMessage(inf, outgoing_packet.packet_bytes);
			}
	 }
	 else
	 {
		 return 0;
	 }
	 UcastAllBestGradients(tree->left, inf);
	 UcastAllBestGradients(tree->right, inf);
	 return 1;
	 */

 }



/*
 * It looks like this function is NOT USED
 */
/*
 int SendToAllInterfaces(InterfaceNode* tree, void* caller)
 {
	 if ( tree )
	 {
		 sendAMessage(tree->i->iName, outgoing_packet.packet_bytes);
	 }
	 else
	 {
		 return 0;
	 }
	 SendToAllInterfaces(tree->left, caller);
	 SendToAllInterfaces(tree->right, caller);
	 return 1;
 }
 */




/*
 * LOOKS OK NOW
 */
/*
 int SendToAllInterfacesExcept(InterfaceNode* tree, NEIGHBOUR_ADDR _exception)
 {
	 if ( tree )
	 {
		if (tree->i->iName != _exception)
		{
			if ( tree->i->iName != SELF_INTERFACE )
			{
				sendAMessage(tree->i->iName, write_packet());
			}
			//else
			//{
			//	sendAMessage(tree->i->iName, outgoing_packet.packet_bytes);
			//}
		}
	 }
	 else
	 {
		 return 0;
	 }
	 SendToAllInterfacesExcept(tree->left, _exception);
	 SendToAllInterfacesExcept(tree->right, _exception);
	 return 1;
 }
*/




	/*
	 * This traversal of the state nodes (now hierarchical in a trie) may present
	 * some problems now it is in a trie.
	 * I have thought over this and it is only used for taking action on those states for
	 * which we are sink or source and I wonder whether we ought not to just record these
	 * some where so we can simply refer to them directly rather than iterating through them all
	 *
	 * This would make it easier to access them
	 *
	 */


 /*
  * We have new trie traversal function so wonte be needing this
  * PROBABLY BE OBSOLETE - SEE OTHER FUNCTION BELOW
  */
/*
 bool TraversStateNodes(StateNode* tree, void process(State* s))
 {
	 if ( tree )
	 {
		 process(tree->s);
	 }
	 else
	 {
		 return false;
	 }
	 TraversStateNodes(tree->left, process);
	 TraversStateNodes(tree->right, process);
	 return true;
 }
*/




/*
 * SEE traverse - used by two calls
 void traverse2(trie *t, void process_state(State* s))
 {
 		if ( t != NULL )
 		{
 			if ( t->s )
 			{
 				process_state(t->s);
 			}
 			t = t->first_child;
 			while(t != NULL)
 			{
 				traverse2(t, process_state);
 				t = t->next_sibling;
 			}
 		}
 }
*/



/*
 void ucast_best_gradient(State* s, unsigned char* _data, NEIGHBOUR_ADDR _if)
  {
    if ( s->bestGradientToObtain && (((*_data) & MSB2) == PUBLICATION) )
    {
        outgoing_packet.message_type = ADVERT;
        outgoing_packet.length = strlen(queue);
        outgoing_packet.data = (unsigned char*)queue;
        outgoing_packet.path_value = s->bestGradientToObtain->costToObtain + nodeConstraint;
        sendAMessage(_if, write_packet());
    }
    if ( s->bestGradientToDeliver && (((*_data) & MSB2) == RECORD) )
    {
        outgoing_packet.message_type = INTEREST;
        outgoing_packet.length = strlen(queue);
        outgoing_packet.data = (unsigned char*)queue;
        outgoing_packet.path_value = s->bestGradientToDeliver->costToDeliver + nodeConstraint;
        sendAMessage(_if, write_packet());
    }


    if ( s->bestGradientToDeliver && (((*_data) & MSB2) == COLLABORATIONBASED) )
    {
        outgoing_packet.message_type = COLLABORATION;
        outgoing_packet.length = strlen(queue);
        outgoing_packet.data = (unsigned char*)queue;
        outgoing_packet.path_value = s->bestGradientToDeliver->costToDeliver + nodeConstraint;
        sendAMessage(_if, write_packet());
    }


  }

*/









void UpdateBestGradient(Interface* i, State* s)
{
    if ( i->up )
    {
        // Not sure if this seqno passing is the right idea
        // but may be doing away with the if down code
        insertKDGradientNode2(s, i, DELIVER_NOCHANGE, 0, rd->grTree, 0, s->seqno);
    }
}


void MarkIFDown(Interface* i, State* s)
{
    if ( i->iName == incoming_packet.down_interface )
    {
        i->up = 0;
    }

}


// PROBABLY DISCONTINUING THIS FUNCTION
int InterfaceDownCallBack(State* s, unsigned char* _buf, NEIGHBOUR_ADDR _if)
{
    if ( !s->bestGradientToDeliver->key2->up )
    {
        KDGradientNode* saveBestGradientToDeliver = s->bestGradientToDeliver;
        s->bestGradientToDeliver = 0;
        TraversInterfaceNodes(rd->interfaceTree, s, UpdateBestGradient);
        KDGradientNode* alternativeBestGradientToDeliver = s->bestGradientToDeliver;
        s->bestGradientToDeliver = saveBestGradientToDeliver;

        if ( alternativeBestGradientToDeliver && ((    *(incoming_packet.data) & MSB2) == RECORD) )
        {
            add(&(s->deliveryInterfaces), alternativeBestGradientToDeliver->key2);
            outgoing_packet.message_type = REINFORCE_INTEREST;
            //outgoing_packet.data = incoming_packet.data;
            outgoing_packet.data = _buf;
            outgoing_packet.length = strlen((const char*)_buf); // strl ok here
            outgoing_packet.path_value = 0;
            outgoing_packet.excepted_interface = UNKNOWN_INTERFACE;
            outgoing_packet.down_interface = _if;
            sendAMessage(alternativeBestGradientToDeliver->key2->iName, write_packet(&outgoing_packet), 0);
        }
    }

    return 0;

}


// PROBABLY DISCONTINUING THIS FUNCTION
void InterfaceDown(unsigned char* pkt, NEIGHBOUR_ADDR inf)
//void InterfaceDown(Interface* i, State* s)
{
    /*
     * JUST FOR INTERESTS SO FAR
     *
     * Initial idea - needs some work
     *
     */
    int inserted;
    Interface* i = InsertInterfaceNode(&(rd->interfaceTree), inf, &inserted)->i;
    i->up = 0;

    read_packet(pkt, 0);
    action_all_prefixes(rd->top_state, 0, incoming_packet.length,
            incoming_packet.data,
            current_prefix_name, inf, InterfaceDownCallBack);

#ifdef GRAD_FILES
    UpdateGradientFile();
#endif

}





// PROBABLY DOING AWAY WITH THIS
void handle_interest_correction(control_data cd)
{
    NEIGHBOUR_ADDR _interface = cd.incoming_if;

    if ( incoming_packet.excepted_interface == thisAddress )
    {
        //return;
    }

    // find incoming state s and interface i and current best cost
    trie* t = trie_add(rd->top_state, incoming_packet.data, STATE);
    State* s = t->s;
    int oldBestCost = s->bestGradientToDeliver->costToDeliver;
    int inserted;
    Interface* i = InsertInterfaceNode(&(rd->interfaceTree), _interface, &inserted)->i;

    //incoming_packet.data;
    //incoming_packet.path_value;
    //incoming_packet.down_interface;
    //incoming_packet.excepted_interface;
    //incoming_packet.length;
    //setDeliverGradient((char*)incoming_packet.data, _interface, incoming_packet.path_value);

    // mark down_interface down if we have it also
    // I THINK MAYBE WE SHOULDNT INSERT THIS IF WE DONT ALREADY HAVE IT ASAN INTERFACE
    Interface* down_i = InsertInterfaceNode(&(rd->interfaceTree), incoming_packet.down_interface, &inserted)->i;
    if ( down_i )
    {
        down_i->up = 0;
    }
    removeIF(&(s->deliveryInterfaces), down_i);
    removeIF(&(s->obtainInterfaces), down_i);

    //if ( incoming_packet.excepted_interface == thisAddress )
    //{
        /*
         * So, for a regular interest, we just would NOT add a gradient back to
         * _interface
         *
         * However, in this case this is a correction, so we may ALREADY have a gradient,
         * in which case we will probably want to remove it
         *
         * for the moment perhaps we should set path cost MAX_COST since we don't have
         * a removal function yet
         */
        // Just correct this gradient
        //insertKDGradientNode2(s, i, DELIVER_CORRECTION, MAX_COST, rd->grTree, 0);
        // NBNBNB:
        // We don't really want to add a MAX_COST one if there is already not one there any way!!!
    //}
    //else
    //{
        // Just correct this gradient
        insertKDGradientNode2(s, i, DELIVER_CORRECTION, incoming_packet.path_value, rd->grTree, 0, incoming_packet.seqno);
    //}





    // Now select new best gradient
    s->bestGradientToDeliver = 0;
    TraversInterfaceNodes(rd->interfaceTree, s, UpdateBestGradient);

    // if a new best cost then broadcast it
    if ( s->bestGradientToDeliver->costToDeliver != oldBestCost )//??? // best gradient changed
    {
        s->bestGradientToDeliverUpdated = false;
        outgoing_packet.message_type = INTEREST_CORRECTION;
        outgoing_packet.data = incoming_packet.data;
        outgoing_packet.length = strlen((const char*)incoming_packet.data);  // strlen ok here
#ifdef USE_NODE_STABILITY_COST
        outgoing_packet.path_value = s->bestGradientToDeliver->costToDeliver+nodeConstraint;
#else
        outgoing_packet.path_value = s->bestGradientToDeliver->costToDeliver;
#endif
        outgoing_packet.excepted_interface = s->bestGradientToDeliver->key2->iName;//_interface;
        outgoing_packet.down_interface = incoming_packet.down_interface;
        bcastAMessage(write_packet(&outgoing_packet));
    }

}






/*
 * LOOKS OK, NOT USED RIGHT NOW BUT VERY PROBABLY IN THE FUTURE
 */
 bool TraversInterfaceNodes(InterfaceNode* tree, State* s, void process(Interface* i, State* s))
 {
	 if ( tree )
	 {
		 process(tree->i, s);
	 }
	 else
	 {
		 return false;
	 }
	 TraversInterfaceNodes(tree->left, s, process);
	 TraversInterfaceNodes(tree->right, s, process);
	 return true;
 }





 unsigned int CountInterfaceNodes(InterfaceNode* tree)
 {
     unsigned int count = 0;
     if ( tree )
     {
         count++;
     }
     else
     {
         return count;
     }
     count += CountInterfaceNodes(tree->left);
     count += CountInterfaceNodes(tree->right);
     return count;
 }


/*
 unsigned int AverageNeighborLqi(InterfaceNode* tree)
 {
     //return (TotalNeighborLqi(tree) / CountInterfaceNodes(tree));

 }
*/


 unsigned int TotalNeighborLqi(InterfaceNode* tree)
 {
     unsigned int lqi = 0;
     if ( tree )
     {
         lqi+=(unsigned int)tree->i->lqi;
     }
     else
     {
         return lqi;
     }
     lqi += TotalNeighborLqi(tree->left);
     lqi += TotalNeighborLqi(tree->right);
     return lqi;
 }









 void MinMaxNeighborLqi(InterfaceNode* tree, unsigned int* maxlqi, unsigned int* minlqi)
 //unsigned int TotalNeighborLqi(InterfaceNode* tree, unsigned int* maxlqi, unsigned int* minlqi)
 {
     if ( tree )
     {
         unsigned int lqi = (unsigned int)tree->i->lqi;
         *maxlqi = lqi > *maxlqi ? lqi : *maxlqi;
         *minlqi = lqi < *minlqi ? lqi : *minlqi;
         //lqi+=(unsigned int)tree->i->lqi;
     }
     else
     {
         return;
         //return lqi;
     }
     MinMaxNeighborLqi(tree->left, maxlqi, minlqi);
     MinMaxNeighborLqi(tree->right, maxlqi, minlqi);
     //lqi += CountInterfaceNodes(tree->left);
     //lqi += CountInterfaceNodes(tree->right);
     return;
     //return lqi;
 }




















 /*
  * NO LONGER USED - REPLACED BY THE VARIOUS PREFIX MATCHING FUNCTIONS
  */
/*
 struct StateNode* FindStateNode(struct StateNode* tree, int val)
{
    struct StateNode *next = tree;
#ifdef DEBUG
	RD( "Searching for: " << val << std::endl );
#endif

    int i = 0;
	while (next != NULL) {
#ifdef DEBUG
		RD( "Reached node: " << next->s->dataName << std::endl );
#endif
		if (val == next->s->dataName._dataname_struct1.the_dataname) {
#ifdef DEBUG
			RD( "Found = true" << std::endl << std::endl );
#endif
            return next;
		} else if (val < next->s->dataName._dataname_struct1.the_dataname) {
            next = next->left;
        } else {
            next = next->right;
        }
		i++;
    }

    //not found
#ifdef DEBUG
	RD( "Found = false" << std::endl << std::endl );
#endif
    return NULL;
}
*/


/*
 * LOOKS FINE
 *
 * In the long term we need to do some work on the interface data type
 * For example, we need to ensure it is generic and will work for different
 * types, not just on compilation, but for gateways that span two address types
 *
 */
struct InterfaceNode* FindInterfaceNode(struct InterfaceNode* tree, NEIGHBOUR_ADDR val)
{
    struct InterfaceNode *next = tree;
#ifdef DEBUG
	RD( "Searching for: " << val << std::endl );
#endif

    int i = 0;
	while (next != NULL) {
#ifdef DEBUG
		RD( "Reached node: " << next->i->iName << std::endl );
#endif
		if (val == next->i->iName) {
#ifdef DEBUG
			RD( "Found = true" << std::endl << std::endl );
#endif
            return next;
		} else if (val < next->i->iName) {
            next = next->left;
        } else {
            next = next->right;
        }
		i++;
    }

    //not found
#ifdef DEBUG
	RD( "Found = false" << std::endl << std::endl );
#endif
    return NULL;
}





/*
 * Just about OK
 */
/*

NOT USED???
struct KDGradientNode* newKDGradientNode(char* fullyqualifiedname, NEIGHBOUR_ADDR iName, int obtain, int deliver)
{
	struct KDGradientNode* n = (struct KDGradientNode *)malloc(sizeof(struct KDGradientNode));
	// we need to make it so that the same st cannot get created twice
	// this is enforced in the same tree but not in different trees

	// CHANGE
	//n->key1 = InsertStateNode(&(rd->stateTree), sName)->s;
	trie* t = trie_add(rd->top_state, fullyqualifiedname, STATE);
	n->key1 = t->s;

	int inserted;
	n->key2 = InsertInterfaceNode(&(rd->interfaceTree), iName, &inserted)->i;
	n->costToObtain = obtain;
	n->costToDeliver = deliver;
	n->deliveryReinforcement = FALSE;
	n->obtainReinforcement = FALSE;
	n->left = NULL;
	n->right =NULL;
	return n;
}
 */





void freeKDGradientNode(KDGradientNode* g)
{
    if ( g )
    {
        // not removing n->key1 here
        // not removing n->key2 here
        freeKDGradientNode(g->left);
        freeKDGradientNode(g->right);
        free(g);
    }
}






struct KDGradientNode* newKDGradientNode2(State* s, Interface* i, int obtain, int deliver, char seqno)
{
	struct KDGradientNode* n = (struct KDGradientNode *)malloc(sizeof(struct KDGradientNode));
	// we need to make it so that the same st cannot get created twice
	// this is enforced in the same tree but not in different trees

	// CHANGE
	//n->key1 = InsertStateNode(&(rd->stateTree), sName)->s;
	//trie* t = trie_add(rd->top_state, fullyqualifiedname, STATE);
	//n->key1 = t->s;
	n->key1 = s;

	//n->key2 = InsertInterfaceNode(&(rd->interfaceTree), iName)->i;
	n->key2 = i;
	n->costToObtain = obtain;
	n->costToDeliver = deliver;
	n->seqno = seqno;
	n->deliveryReinforcement = FALSE;
	n->obtainReinforcement = FALSE;
	n->left = NULL;
	n->right =NULL;
	return n;
}




int GetStateStr(trie *t, State* _s, unsigned char* str)
{
       if ( t != NULL )
       {
           if ( t->keyelem )
           {
               *(str) = t->keyelem;
               //if ( t->keyelem == '\b' )
               //{
               //    int x = 0;
               //    RD( "x: " << x << "\n" );
               //}
               *(str+1) = 0;
               str++;
           }
           //if ( t->s )
           if ( t->s == _s )
           {
               //process(t->s, (unsigned char*)queue, inf);
               // we are finished
               return 1;


               //printf("key: %s    -    ", queue);
               //printf("obj: %s\n", t->s->full_name);
           }
           t = t->first_child;
           while(t != NULL)
           {
               if ( GetStateStr(t, _s, str) )
                   return 1;
               t = t->next_sibling;
           }
       }
       return 0;
}




bool TraversGradientNodes(struct KDGradientNode* tree, void process(KDGradientNode* g, unsigned char* _name))
{
    static unsigned char sName[20];
    if ( tree )
    {
        *sName = 0;
        GetStateStr(rd->top_state, tree->key1, sName);
        process(tree, sName);
    }
    else
    {
        return false;
    }
    TraversGradientNodes(tree->left, process);
    TraversGradientNodes(tree->right, process);
    return true;
}




/*
struct KDGradientNode* traverseKDGradientNode(State* s, Interface* i, int costType, int pCost, struct KDGradientNode* treeNode, int lev )
{
        if ( s == treeNode->key1 && i == treeNode->key2 )
        {
        }
        else
        {
            if ( lev == 0 )
            {
                if ( s > treeNode->key1 )
                {
                    treeNode->right = traverseKDGradientNode( s, i, costType, pCost, treeNode->right, (lev+1)%K );
                }
                else
                {
                    treeNode->left  = traverseKDGradientNode( s, i, costType, pCost, treeNode->left, (lev+1)%K );
                }
            }

            if ( lev == 1 )
            {
                if ( i > treeNode->key2 )
                {
                    treeNode->right = traverseKDGradientNode( s, i, costType, pCost, treeNode->right, (lev+1)%K );
                }
                else
                {
                    treeNode->left  = traverseKDGradientNode( s, i, costType, pCost, treeNode->left, (lev+1)%K );
                }
            }

            return treeNode;
        }


    return treeNode;
}
*/




void freeInterfaceList(struct InterfaceList* l)
{
    if (l)
    {
        freeInterfaceList(l->link);
        free(l);
    }
}





struct KDGradientNode* insertKDGradientNode2(State* s, Interface* i, int costType, int pCost, struct KDGradientNode* treeNode, int lev, char seqno)
{
    // NEED TO THINK ABOUT SEQNO & REINFORCEMENT

    if ( seqno < s->seqno )
    {
        return treeNode; // I think this is right, i.e. ignore any old ints/advs
        // CHECK THIS
        //Let's do it for now
    }

    switch ( costType )
    {
        case REINFORCE_DELIVER:
        case REINFORCE_OBTAIN:
            // We think not possible for reinforcement seqno to be greater than current
            break;
            /*
             * case DELIVER:
             *
             * if ( seqno > s->seqno )
             *
             * IF: this is a rec and we are the sink for it
             * i.e. we have a best grad already set that is this data rec
             * interface to self and cost zero (and most likely grad and state seqno zero).
             *
             * THEN:  we must leave the best grad to self in place, set its seqno to this
             * incoming seqno value +1 and the same for the state object.
             *
             * Returning from this function with best updated assumes seqno set to the
             * incoming one at least but not increased and also the forwarded interest
             * will have an increased cost.
             *
             * ACTUALLY we JUST want to send out the initiating interest again with the
             * 'brand new' seqno.  I.E. the best grad with IF: Self COST: Zero.
             * Then return (not updated so the incoming interest is not forwarded
             *
             * I THINK CANNOT GET DATA STRING FOR SENDING HERE
             * SO MAYBE MOVE THIS IDEA TO handle_interest
             *
             */
        default:
            if ( seqno > s->seqno )
            {
                s->seqno = seqno;
                s->broken = 0;
                s->bestGradientToDeliver = NULL;
                s->bestGradientToObtain = NULL;
                freeInterfaceList(s->deliveryInterfaces);
                freeInterfaceList(s->obtainInterfaces);
                s->deliveryInterfaces = NULL;
                s->obtainInterfaces = NULL;
            }
            break;
    }

    RD( "insert grad type: " << costType << " Cost: " << pCost << " State: " << s << std::endl );
	if ( treeNode == NULL )
	{
		switch ( costType )
		{
			case OBTAIN:
				treeNode = newKDGradientNode2(s, i, pCost, MAX_COST, seqno);
				break;
            //case DELIVER_CORRECTION:
			case DELIVER:
				treeNode = newKDGradientNode2(s, i, MAX_COST, pCost, seqno);
				break;

			// NEED TO THINK ABOUT SEQNO & REINFORCEMENT
			case REINFORCE_DELIVER:
				treeNode = newKDGradientNode2(s, i, MAX_COST, MAX_COST, seqno);
				//treeNode->deliveryReinforcement = true;
				//treeNode->key1->deliveryInterface = treeNode->key2;
				add(&(treeNode->key1->deliveryInterfaces), treeNode->key2);
				break;
			case REINFORCE_OBTAIN:
				treeNode = newKDGradientNode2(s, i, MAX_COST, MAX_COST, seqno);
				//treeNode->obtainReinforcement = true;
				//treeNode->key1->obtainInterface = treeNode->key2;
				add(&(treeNode->key1->obtainInterfaces), treeNode->key2);
				break;
	        // Keep this????????????
			case DELIVER_NOCHANGE:
	            return treeNode;
	            break;
		}

	}
	else
	{
		// all identical keys to the incoming keys
		if ( s == treeNode->key1 && i == treeNode->key2 )
		{
			// the node for these keys already exists
			// overwrite of the data if better
			switch ( costType )
			{
				case OBTAIN:
                    if ( seqno > treeNode->seqno )
                    {
                        RD( "New seqno: " << seqno << ", first obt cost: " << pCost << std::endl );
                        treeNode->costToObtain = pCost;
                        treeNode->seqno = seqno;
                    }
                    else
                    {
                        if ( seqno == treeNode->seqno )
                        {
                            if ( pCost < treeNode->costToObtain )
                            {
                                treeNode->costToObtain = pCost;
                                if ( treeNode == treeNode->key1->bestGradientToObtain )
                                {
                                    // still same best interface but better cost
                                    treeNode->key1->bestGradientToObtainUpdated = TRUE;
                                }
                            }
                        }
                    }
					break;
                //case DELIVER_CORRECTION:
                //    treeNode->costToDeliver = pCost;
                //    break;
                case DELIVER:
                    if ( seqno > treeNode->seqno )
                    {
                        RD( "New seqno: " << seqno << ", first deliv cost: " << pCost << std::endl );
                        treeNode->costToDeliver = pCost;
                        treeNode->seqno = seqno;
                    }
                    else
                    {
                        if ( seqno == treeNode->seqno )
                        {
                            if ( pCost < treeNode->costToDeliver )
                            {
                                RD( "better deliver cost on this if, now: " << pCost << std::endl );
                                treeNode->costToDeliver = pCost;
                                if ( treeNode == treeNode->key1->bestGradientToDeliver )
                                {
                                    // still same best interface but better cost
                                    treeNode->key1->bestGradientToDeliverUpdated = TRUE;
                                }
                            }
                        }
                    }
					break;
				// NEED TO THINK ABOUT SEQNO & REINFORCEMENT
                case REINFORCE_DELIVER:
					//treeNode->deliveryReinforcement = true;
					//treeNode->key1->deliveryInterface = treeNode->key2;
					add(&(treeNode->key1->deliveryInterfaces), treeNode->key2);
					break;
				case REINFORCE_OBTAIN:
					//treeNode->obtainReinforcement = true;
					//treeNode->key1->obtainInterface = treeNode->key2;
					add(&(treeNode->key1->obtainInterfaces), treeNode->key2);
					break;
			}
		}
		else
		{
			if ( lev == 0 )
			{
				if ( s > treeNode->key1 )
				{
					treeNode->right = insertKDGradientNode2( s, i, costType, pCost, treeNode->right, (lev+1)%K, seqno);
				}
				else
				{
					treeNode->left  = insertKDGradientNode2( s, i, costType, pCost, treeNode->left, (lev+1)%K, seqno);
				}
			}

			if ( lev == 1 )
			{
				if ( i > treeNode->key2 )
				{
					treeNode->right = insertKDGradientNode2( s, i, costType, pCost, treeNode->right, (lev+1)%K, seqno);
				}
				else
				{
					treeNode->left  = insertKDGradientNode2( s, i, costType, pCost, treeNode->left, (lev+1)%K, seqno);
				}
			}

		    return treeNode;
		}
	}

	switch ( costType )
	{
		case OBTAIN:
		case REINFORCE_OBTAIN:
		    if (!treeNode->key1)
		    {
		        treeNode->key1 = 0;
		    }

            // Given the seqno code above, I am pretty sure this will work
            // correctly without amendment
		    if ( !treeNode->key1->bestGradientToObtain
				|| treeNode->costToObtain < treeNode->key1->bestGradientToObtain->costToObtain )
			{
				treeNode->key1->bestGradientToObtain = treeNode;
				treeNode->key1->bestGradientToObtainUpdated = TRUE;
			}
			break;
        case DELIVER:
        case DELIVER_NOCHANGE:
		case REINFORCE_DELIVER:
            if (!treeNode->key1)
            {
                treeNode->key1 = 0;
            }

            // Given the seqno code above, I am pretty sure this will work
            // correctly without amendment
			if ( !treeNode->key1->bestGradientToDeliver
				|| treeNode->costToDeliver < treeNode->key1->bestGradientToDeliver->costToDeliver )
			{
			    RD( "first or better best deliver cost, now: " << treeNode->costToDeliver << std::endl );
				treeNode->key1->bestGradientToDeliver = treeNode;
				treeNode->key1->bestGradientToDeliverUpdated = TRUE;
			}
			break;
	}

	return treeNode;
}




struct KDGradientNode* insertKDGradientNode1(unsigned char* fullyqualifiedname, NEIGHBOUR_ADDR iName, int costType, int pCost, struct KDGradientNode* treeNode, int lev, char seqno)
{
	// possibly remove these at some point
	// have t and i passed in
	trie* t = trie_add(rd->top_state, fullyqualifiedname, STATE);
	State* s = t->s;
	int inserted;
	Interface* i = InsertInterfaceNode(&(rd->interfaceTree), iName, &inserted)->i;

	return insertKDGradientNode2(s, i, costType, pCost, treeNode, lev, seqno);
}



/*
struct KDGradientNode* insertKDGradientNode(char* fullyqualifiedname, int iName, int costType, int pCost, struct KDGradientNode* treeNode, int lev )
{
	if ( treeNode == NULL )
	{
		switch ( costType )
		{
			case OBTAIN:
				treeNode = newKDGradientNode(fullyqualifiedname, iName, pCost, MAX_COST);
				break;
			case DELIVER:
				treeNode = newKDGradientNode(fullyqualifiedname, iName, MAX_COST, pCost);
				break;
			case REINFORCE_DELIVER:
				treeNode = newKDGradientNode(fullyqualifiedname, iName, MAX_COST, MAX_COST);
				//treeNode->deliveryReinforcement = true;
				//treeNode->key1->deliveryInterface = treeNode->key2;
				add(&(treeNode->key1->deliveryInterfaces), treeNode->key2);
				break;
			case REINFORCE_OBTAIN:
				treeNode = newKDGradientNode(fullyqualifiedname, iName, MAX_COST, MAX_COST);
				//treeNode->obtainReinforcement = true;
				//treeNode->key1->obtainInterface = treeNode->key2;
				add(&(treeNode->key1->obtainInterfaces), treeNode->key2);
				break;
		}

	}
	else
	{
		// all identical keys to the incoming keys
		if ( sName == treeNode->key1->dataName._dataname_struct1.the_dataname
			&& iName == treeNode->key2->iName )
		{
			// the node for these keys already exists
			// overwrite of the data if better
			switch ( costType )
			{
				case OBTAIN:
					if ( pCost < treeNode->costToObtain )
					{
						treeNode->costToObtain = pCost;
					}
					break;
				case DELIVER:
					if ( pCost < treeNode->costToDeliver )
					{
						treeNode->costToDeliver = pCost;
						if ( treeNode == treeNode->key1->bestGradientToDeliver )
						{
							// still same best interface but better cost
							treeNode->key1->bestGradientToDeliverUpdated = TRUE;
						}
					}
					break;
				case REINFORCE_DELIVER:
					//treeNode->deliveryReinforcement = true;
					//treeNode->key1->deliveryInterface = treeNode->key2;
					add(&(treeNode->key1->deliveryInterfaces), treeNode->key2);
					break;
				case REINFORCE_OBTAIN:
					//treeNode->obtainReinforcement = true;
					//treeNode->key1->obtainInterface = treeNode->key2;
					add(&(treeNode->key1->obtainInterfaces), treeNode->key2);
					break;
			}
		}
		else
		{
			if ( lev == 0 )
			{
				if ( sName > treeNode->key1->dataName._dataname_struct1.the_dataname )
				{
					treeNode->right = insertKDGradientNode( sName, iName, costType, pCost, treeNode->right, (lev+1)%K );
				}
				else
				{
					treeNode->left  = insertKDGradientNode( sName, iName, costType, pCost, treeNode->left, (lev+1)%K );
				}
			}

			if ( lev == 1 )
			{
				if ( iName > treeNode->key2->iName )
				{
					treeNode->right = insertKDGradientNode( sName, iName, costType, pCost, treeNode->right, (lev+1)%K );
				}
				else
				{
					treeNode->left  = insertKDGradientNode( sName, iName, costType, pCost, treeNode->left, (lev+1)%K );
				}
			}

		}
	}

	switch ( costType )
	{
		case OBTAIN:
		case REINFORCE_OBTAIN:
			if ( !treeNode->key1->bestGradientToObtain
				|| treeNode->costToObtain < treeNode->key1->bestGradientToObtain->costToObtain )
			{
				treeNode->key1->bestGradientToObtain = treeNode;
				treeNode->key1->bestGradientToObtainUpdated = TRUE;
			}
			break;
		case DELIVER:
		case REINFORCE_DELIVER:
			if ( !treeNode->key1->bestGradientToDeliver
				|| treeNode->costToDeliver < treeNode->key1->bestGradientToDeliver->costToDeliver )
			{
				treeNode->key1->bestGradientToDeliver = treeNode;
				treeNode->key1->bestGradientToDeliverUpdated = TRUE;
			}
			break;
	}

	return treeNode;
}
*/



void reinforceDeliverGradient(unsigned char* fullyqualifiedname, NEIGHBOUR_ADDR iName, char seqno)
{
	rd->grTree = insertKDGradientNode1(fullyqualifiedname, iName, REINFORCE_DELIVER, 0, rd->grTree, 0, seqno);
}


void reinforceObtainGradient(unsigned char* fullyqualifiedname, NEIGHBOUR_ADDR iName, char seqno)
{
	rd->grTree = insertKDGradientNode1(fullyqualifiedname, iName, REINFORCE_OBTAIN, 0, rd->grTree, 0, seqno);
}



void setObtainGradient(unsigned char* fullyqualifiedname, NEIGHBOUR_ADDR iName, int pCost, char seqno)
{
	rd->grTree = insertKDGradientNode1(fullyqualifiedname, iName, OBTAIN, pCost, rd->grTree, 0, seqno);
}


void setDeliverGradient(unsigned char* fullyqualifiedname, NEIGHBOUR_ADDR iName, int pCost, char seqno)
{
	rd->grTree = insertKDGradientNode1(fullyqualifiedname, iName, DELIVER, pCost, rd->grTree, 0, seqno);
}



/*
 * I THINK THIS IS UNUSED AND WAS INITIALLY FOR UNDERSTANDING GARDIENT DATA STRUCTURE
 */
/*
struct KDNode* newKDNode(int keys[], int newStateName, int _action, int _condition)
{
	struct KDNode* n = (KDNode *)malloc(sizeof(KDNode));
	n->keys[0] = InsertStateNode(&(rd->stateTree), keys[0])->s;
	n->keys[1] = InsertStateNode(&(rd->stateTree), keys[1])->s;
	n->newState = InsertStateNode(&(rd->stateTree), newStateName)->s;
	n->transitionAction = _action;
	n->transitionCondition = _condition;
	n->left = NULL;
	n->right =NULL;
	return n;
}
*/



/*
 * I THINK THIS IS UNUSED AND WAS INITIALLY FOR UNDERSTANDING GARDIENT DATA STRUCTURE
 */
/*
struct KDNode* insertKDNode( int stateDataname[], int newStateName, int _action, int _condition, struct KDNode* treeNode, int lev )
{
	int i;
	if ( treeNode==NULL )
	{
		treeNode = newKDNode(stateDataname, newStateName, _action, _condition);
	}
	else
	{
		// this line sets i == K if we have reached a node with
		// all identical keys to the incoming keys
		for ( i=0; i<K && stateDataname[i] == treeNode->keys[i]->dataName._dataname_struct1.the_dataname; i++ );
		if ( i==K )
		{
			// we have found a duplicate node
		}
		else
		{
			if ( stateDataname[lev] > treeNode->keys[lev]->dataName._dataname_struct1.the_dataname )
			{
				treeNode->right = insertKDNode( stateDataname, newStateName, _action, _condition, treeNode->right, (lev+1)%K );
			}
			else
			{
				treeNode->left  = insertKDNode( stateDataname, newStateName, _action, _condition, treeNode->left, (lev+1)%K );
			}
		}
	}
	return treeNode;
}
*/











struct KDGradientNode* SearchForKDGradientNode2( State* s, Interface* i, struct KDGradientNode* treeNode)
{
	int lev;
	for ( lev=0; treeNode != NULL; lev=(lev+1)%K )
	{
		// all identical keys to the incoming keys
		if ( s == treeNode->key1 && i == treeNode->key2 )
		{
			return treeNode;
		}

		if ( lev == 0 )
		{
			if ( s > treeNode->key1 )
			{
				treeNode = treeNode->right;
			}
			else
			{
				treeNode = treeNode->left;
			}
		}

		if ( lev == 1 )
		{
			if ( i > treeNode->key2 )
			{
				treeNode = treeNode->right;
			}
			else
			{
				treeNode = treeNode->left;
			}
		}
	}
	return treeNode; // now NULL
}



// seems as if not used at the moment
struct KDGradientNode* SearchForKDGradientNode1( unsigned char* _data, NEIGHBOUR_ADDR iName, struct KDGradientNode* treeNode)
{
// TODO
	/*
	 * Need to check which tri search to do here to see if we already have an existing state
	 */

	trie* t = trie_add(rd->top_state, _data, STATE);
	int inserted;
	struct InterfaceNode* i = InsertInterfaceNode(&(rd->interfaceTree), iName, &inserted);

	if ( t && i )
	{
		return SearchForKDGradientNode2( t->s, i->i, treeNode);

	}
	else
	{
		return NULL;
	}

}



/*
 * ONLY CALLED FROM OMNET, SO MAY NOT REQUIRE ANYMORE NOT SURE
 *
 * Am in process of doing a new version above
 *
 */
/*
struct KDGradientNode* SearchForKDGradientNode( int sName, int iName, struct KDGradientNode* treeNode)
{
	int lev, i;
	for ( lev=0; treeNode != NULL; lev=(lev+1)%K )
	{
		// all identical keys to the incoming keys
		if ( sName == treeNode->key1->dataName._dataname_struct1.the_dataname
			&& iName == treeNode->key2->iName )
		{
#ifdef DEBUG
			RD( "Found:     " << "[" << treeNode->keys[0]->dataName << ","
				<< treeNode->keys[1]->dataName << "," << treeNode->newState->dataName << "]" << std::endl );
#endif
			return treeNode;
		}

		if ( lev == 0 )
		{
			if ( sName > treeNode->key1->dataName._dataname_struct1.the_dataname )
			{
				treeNode = treeNode->right;
			}
			else
			{
				treeNode = treeNode->left;
			}
		}

		if ( lev == 1 )
		{
			if ( iName > treeNode->key2->iName )
			{
				treeNode = treeNode->right;
			}
			else
			{
				treeNode = treeNode->left;
			}
		}
	}
#ifdef DEBUG
	RD( "NOT Found: " << "[" << stateDataname[0] << ","
		<< stateDataname[1] << "]" << std::endl );
#endif
	return treeNode; // now NULL
}
*/



/*
 * WE THINK THIS IS NOLONGER USED ALONG WITH OTHER KD FUNCTIONS JUST USED TO BEGIN THE GRADIENT
 * DATA STRUCTURE CODE
 */
/*
struct KDNode* SearchForKDNode( int stateDataname[], struct KDNode* treeNode )
{
	int lev, i;
	for ( lev=0; treeNode != NULL; lev=(lev+1)%K )
	{
		// this line sets i == K if we have reached a node with
		// all identical keys to the incoming keys
		for ( i=0; i<K && stateDataname[i] == treeNode->keys[i]->dataName._dataname_struct1.the_dataname; i++ );
		if ( i==K )
		{
#ifdef DEBUG
			RD( "Found:     " << "[" << treeNode->keys[0]->dataName << ","
				<< treeNode->keys[1]->dataName << "," << treeNode->newState->dataName << "]" << std::endl );
#endif
			return treeNode;
		}

		if ( stateDataname[lev] > treeNode->keys[lev]->dataName._dataname_struct1.the_dataname )
		{
			treeNode = treeNode->right;
		}
		else
		{
			treeNode = treeNode->left;
		}
	}
#ifdef DEBUG
	RD( "NOT Found: " << "[" << stateDataname[0] << ","
		<< stateDataname[1] << "]" << std::endl );
#endif
	return treeNode; // now NULL
}
*/






/*
int getNewState(int input)
{
	int keys[2];
	keys[0] = input;
	keys[1] = currentState;
	struct KDNode* kdn = SearchForKDNode(keys, rd->kdRoot);
	if ( kdn )
	{
		return (currentState = kdn->newState->dataName);
	}
	else
	{
		return input;
	}

}
*/


/*
int SearchForKDNodeTransition(int input)
{
	int stateDataname[2];
	stateDataname[0] = input;
	stateDataname[1] = currentState;
	struct KDNode* treeNode = rd->kdRoot;

	int lev, i;
	for ( lev=0; treeNode != NULL; lev=(lev+1)%K )
	{
		// this line sets i == K if we have reached a node with
		// all identical keys to the incoming keys
		for ( i=0; i<K && stateDataname[i] == treeNode->keys[i]->dataName; i++ );
		if ( i==K )
		{
#ifdef DEBUG
			RD( "[" << currentState << "]     " << input << "  --->  " << treeNode->newState->dataName << std::endl );
#endif
			return (currentState = treeNode->newState->dataName);
		}

		if ( stateDataname[lev] > treeNode->keys[lev]->dataName )
		{
			treeNode = treeNode->right;
		}
		else
		{
			treeNode = treeNode->left;
		}
	}
#ifdef DEBUG
	RD( "[" << currentState << "]     " << input << "  --->  " << input << std::endl );
#endif
	return input; // now NULL
}
*/



#ifdef MY_C_PLUSPLUS

// UNUSED WE THINK
/*
void PrepareKDNodeDisplayStream(KDNode** treeNode, int level, std::ostringstream* os)
 {

	 if (*treeNode == NULL)
	 {
		os[level] << "(-,-) ";
		return;
	 }
	 else
	 {
		 os[level] << "(" << (*treeNode)->keys[0]->dataName._dataname_struct1.the_dataname << "," <<
			 (*treeNode)->keys[1]->dataName._dataname_struct1.the_dataname << "," << (*treeNode)->newState->dataName._dataname_struct1.the_dataname << ") ";
	 }

	 PrepareKDNodeDisplayStream(&((*treeNode)->left), level+1, os);
	 PrepareKDNodeDisplayStream(&((*treeNode)->right), level+1, os);
 }
*/


static std::ostringstream displayStream;


void displayContext(context* c, unsigned char* _context)
{
    displayStream << "CONTEXT:      " << _context;
    displayStream << std::endl;
}

void displayState(State* s, unsigned char* _data, NEIGHBOUR_ADDR _if)
{
    int i = 0;
    switch(s->action)
    {
    case SOURCE_ACTION:
        displayStream << "SOURCE STATE:        ";
        break;
    case SINK_ACTION:
        displayStream << "SINK STATE:        ";
        break;
    case FORWARD_ACTION:
        displayStream << "FORWARD STATE:        ";
        break;
    }

    while (_data[i] != 0)
    {
        displayStream << (unsigned int)_data[i] << " ";
        i++;
    }
    displayStream << std::endl;

    if (s->bestGradientToDeliver)
    {
        displayStream << "DELIVER IF:   " << s->bestGradientToDeliver->key2->iName << std::endl;
        displayStream << "DELIVER COST: " << s->bestGradientToDeliver->costToDeliver << std::endl;
    }
    if (s->bestGradientToObtain)
    {
        displayStream << "OBTAIN IF:    " << s->bestGradientToObtain->key2->iName << std::endl;
        displayStream << "OBTAIN COST:  " << s->bestGradientToObtain->costToObtain << std::endl;
    }
    displayStream << std::endl;
}





 void PrepareKDGradientNodeDisplayStream(KDGradientNode** treeNode, int level, std::ostringstream* os)
 {

	 if (*treeNode == NULL)
	 {
		os[level] << "(-,-) ";
		return;
	 }
	 else
	 {
		 os[level] << "(" << "unknown state, IF:" << //(*treeNode)->key1 << "," <<
			 (*treeNode)->key2->iName << "," << (*treeNode)->costToDeliver << ", " << (*treeNode)->costToObtain << ") ";
	 }

	 PrepareKDGradientNodeDisplayStream(&((*treeNode)->left), level+1, os);
	 PrepareKDGradientNodeDisplayStream(&((*treeNode)->right), level+1, os);
 }


 // UNUSED WE THINK
 /*
 void DisplayKDTree(KDNode** treeNode, std::ofstream& infile)
 {
	std::ostringstream os[MAXLEVELS];
	PrepareKDNodeDisplayStream(treeNode, 0, os);
	for ( int i = 0; i < MAXLEVELS; i++ )
	{
		infile << os[i].str() << std::endl;
	}
 }
 */



 void DisplayNodeDetails(std::ofstream& infile)
 {
     displayStream.clear();
     displayStream.str("");


     memset(queue, 0, 100);
     traverse(rd->top_state, queue, 0, displayState);

     memset(queue, 0, 100);
     traverseContextTrie2(rd->top_context, queue, displayContext);

     infile << displayStream.str() << std::endl;
 }


 void DisplayKDGradientTree(KDGradientNode** treeNode, std::ofstream& infile)
 {
	std::ostringstream os[MAXLEVELS];
	PrepareKDGradientNodeDisplayStream(treeNode, 0, os);
	for ( int i = 0; i < MAXLEVELS; i++ )
	{
		infile << os[i].str() << std::endl;
	}
 }
#endif








void removeIF(struct InterfaceList** l, struct Interface* _i)
{
    if(*l==NULL)
    {
        return;
    }

    struct InterfaceList *old,*temp;
    temp=*l;
    while( temp != NULL )
    {
        if(temp->i  == _i)
        {
            if( temp == *l )
            {
                *l=temp->link;
            }
            else
            {
                old->link=temp->link;
            }
            free(temp);
            temp = 0;
        }
        else
        {
            old=temp;
            temp=temp->link;
        }
    }
}



bool deliverReinforced(KDGradientNode* g)
{
    InterfaceList* temp = g->key1->deliveryInterfaces;
    while( temp !=NULL )
    {
        if ( temp->i->iName == g->key2->iName )
        {
            return true;
        }
        temp = temp->link;
    }
    return false;
}




bool obtainReinforced(KDGradientNode* g)
{
    InterfaceList* temp = g->key1->obtainInterfaces;
    while( temp !=NULL )
    {
        if ( temp->i->iName == g->key2->iName )
        {
            return true;
        }
        temp = temp->link;
    }
    return false;
}




/*
 * Basic Linked List specifically for storing a set of reinforced
 * obtain or deliver interfaces
 *
 * Definitely looks like it adds it at the end
 *
 * SEEMS OK
 *
 */
void add(struct InterfaceList** l, struct Interface* _i)
{
	struct InterfaceList *temp,*r;
	temp = *l;
	if( *l == NULL )
	{
		temp = (struct InterfaceList *)malloc(sizeof(struct InterfaceList));
		temp->i = _i;
		temp->link = NULL;
		*l = temp;
	}
	else
	{
		temp = *l;
#ifdef XXXXXX
            if ( temp->i->iName == _i->iName )
                return; // interface is already in list
#endif
		while( temp->link !=NULL )
		{
		    temp = temp->link;
#ifdef XXXXXX
            if ( temp->i->iName == _i->iName )
                return; // interface is already in list
#endif
		}
		r = (struct InterfaceList *)malloc(sizeof(struct InterfaceList));
		r->i = _i;
		r->link = NULL;
		temp->link = r;
	}
}









// If new_packet* _i is already in the state's queue the does nothing
// else creates copy, adds to queue and sets delete timer
void addCopyToQIfNotPresent(State* s, struct new_packet* _i)
{
    struct PacketQueue** l = &(s->pktQ);
    struct PacketQueue *temp,*r;
    temp = *l;
    if( *l == NULL )
    {
        temp = (struct PacketQueue *)malloc(sizeof(struct PacketQueue));
        temp->i = packetCopy(_i);
        QueueDeletion* qd = (QueueDeletion*)malloc(sizeof(QueueDeletion));
        qd->pktToDelete = temp->i;
        qd->associatedState = s;
        setTimer(2.00, qd, delete_queued_data);
        temp->link = NULL;
        *l = temp;
    }
    else
    {
        temp = *l;
        while( temp->link !=NULL )
        {
            if ( temp->i == _i )
                return;
            temp = temp->link;
        }
        if ( temp->i == _i )
            return;

        r = (struct PacketQueue *)malloc(sizeof(struct PacketQueue));
        r->i = packetCopy(_i);
        QueueDeletion* qd = (QueueDeletion*)malloc(sizeof(QueueDeletion));
        qd->pktToDelete = r->i;
        qd->associatedState = s;
        setTimer(2.00, qd, delete_queued_data);
        r->link = NULL;
        temp->link = r;
    }
}




// adds at back
void addPkt(struct PacketQueue** l, struct new_packet* _i)
{
    struct PacketQueue *temp,*r;
    temp = *l;
    if( *l == NULL )
    {
        temp = (struct PacketQueue *)malloc(sizeof(struct PacketQueue));
        temp->i = _i;
        temp->link = NULL;
        *l = temp;
    }
    else
    {
        temp = *l;
        while( temp->link !=NULL )
        {
            temp = temp->link;
        }
        r = (struct PacketQueue *)malloc(sizeof(struct PacketQueue));
        r->i = _i;
        r->link = NULL;
        temp->link = r;
    }
}







/*=============================== FSM VARS =====================================*/

// FSM variables
int vars[10];

// possibly not used in the future
//static int currentState = 0;




/*=============================== FSM ACTIONS NEW =====================================*/

bool null_action(void* p1, void* p2)
{
	//printf("Action null_action: ");
	return true;

}

bool assign1(void* p1, void* p2)
{
	//printf("Action assign1: ");
	//vars[param[1]] = vars[param[0]];
	*(unsigned short*)p1 = *(unsigned short*)p2;
	return true;
}


bool assign2( void* p1,  void* p2)
{
	//printf("Action assign2: ");
	//vars[param[1]] = param[0];
	*(unsigned short*)p1 = *(unsigned short*)p2;
	return true;
}


bool addto( void* p1,  void* p2)
{
	//printf("Action assign2: ");
	//vars[param[1]] = param[0];
	*(unsigned short*)p1 += *(unsigned short*)p2;
	return true;
}


bool messageIn(void* p1, void* p2)
{
	if ( (rd->flags & MESSAGE_ARRIVED) )//&& (*p1) == incoming_packet.the_data_message.data_value._dataname_struct1.the_dataname )
	{
		// if on turns the flag off?
		// and if off turns the flag on?
		rd->flags ^= MESSAGE_ARRIVED;

		//10000
		//00100
		return true;
	}
	return false;
}


bool copyMessageIn(void* p1, void* p2)
{
	if ( (rd->flags & MESSAGE_ARRIVED) )//&& *p1 == incoming_packet.the_data_message.data_value._dataname_struct1.the_dataname )
	{

		//*p2 = incoming_packet.the_data_message.in1;


		//printf("Action copyMessageIn: ");

		// turns the flag off
		rd->flags ^= MESSAGE_ARRIVED;
		return true;
	}
	return false;

}




NEIGHBOUR_ADDR excludedInterface;
int consider_sending_data(State* s, unsigned char* _buf, NEIGHBOUR_ADDR _if)
{
    int sent = 0;
    // but it's the query SOURCE data we need to send not the
    bool query_based = ( (*(sending_packet->data)&MSB2) == RECORD );
    bool event_based = ( (*(sending_packet->data)&MSB2) == PUBLICATION );
    //bool collaberation_based = ( (*(sending_packet->data)&MSB2) == COLLABORATIONBASED );


    // Not all collaborators have delivery grads to self
    // only the collaborator initiator
    // needs some thought
    if ( s->action == COLLABORATE_ACTION && excludedInterface != SELF_INTERFACE )
    {
        handleApplicationData(sending_packet->data, sending_packet->creation_time);
        sent = 1;
    }

    // non-initiating collaborator nodes may still participate in forwarding
    if ( s->deliveryInterfaces )
    {
        //dataRate = 0;
        // possibly not thread safe
        //incoming_packet.the_message.data_value = s->dataName._dataname_struct1.the_dataname;
        //incoming_packet.the_data_message.data_value._dataname_struct1.the_dataname = s->dataName._dataname_struct1.the_dataname;

        //incoming_packet.message_type = DATA;
        //incoming_packet.data = _buf;
        //incoming_packet.length = strlen((char*)_buf);
        //incoming_packet.path_value = 0;
        //handle_data(SELF_INTERFACE);


        if ( event_based )
        {
            if (       (s->bestGradientToObtain && s->obtainInterfaces)
                    && (s->bestGradientToObtain->key2 == s->obtainInterfaces->i)    )
            {
                RD( "Reinforced obtain is still the ...SAME... as best" << std::endl );
            }
            else
            {
                RD( "Reinforced obtain is ...NO LONGER THE SAME... as best" << std::endl );
            }
        }

        InterfaceList* temp = s->deliveryInterfaces;
        while( temp !=NULL )
        {
            if ( temp->i->up && temp->i->iName != excludedInterface )
            {
                if ( temp->i->iName == SELF_INTERFACE )
                {
                    // is the data item enough for the application
                    // is length, path value etc etc needed?
                    handleApplicationData(sending_packet->data, sending_packet->creation_time);
                    sent = 1;

#ifdef XXXXXX
                    setTimer(CANCEL_TIMER, s, start_reinforce_retry); // is s the right object?
#endif
                    // think we remove sending_packet from queue here
                    // but may be different for advert cases which have multiple
                    // deliver interfaces
                }
                else
                {
                    // some how we need to know whether this send
                    // from the queue
                    sendAMessage(temp->i->iName, write_packet(sending_packet), sending_packet->creation_time);
                    sent = 1;
                    // length taken from incoming/outgoing message not strlen

                    // think we remove sending_packet from queue here
                    // but may be different for advert cases which have multiple
                    // deliver interfaces
                }
                if ( query_based )
                {
                    // here we want to know what?
                    // ok, we have record data to send and we are
                    // sending it on the interface that has been added
                    // to the list on reinforcement
                    //
                    // that interface was the one in the best grad for
                    // that data
                    //
                    if ( s->bestGradientToDeliver->key2 == temp->i )
                    {
                        RD( "Reinforced is still the ...SAME... as best" << std::endl );
                    }
                    else
                    {
                        RD( "Reinforced is ...NO LONGER THE SAME... as best" << std::endl );
                    }
                    break;
                }
            }
            temp = temp->link;
        }

    }
    else
    {
        // New code below - old code falsed out - can switch between using true/false
        bool newCode = true;

        if ( newCode )
        {
            // If sending data and there are no delivery interfaces then
            // always queue the pkt, will be deleted or send as soon as node is converged
            new_packet* savePkt = packetCopy(sending_packet);
            addPkt(&(s->pktQ), savePkt);
            QueueDeletion* qd = (QueueDeletion*)malloc(sizeof(QueueDeletion));
            qd->pktToDelete = savePkt;
            qd->associatedState = s;
            setTimer(2.00, qd, delete_queued_data);

            // If data arrives at a converged node with no reinforcement then initiate
            // reinforcement from here.  This can occur following a breakage
            if( s->converged )
            {
                if ( query_based && s->bestGradientToDeliver )
                {
                    start_reinforce_interest(_buf, excludedInterface, s->seqno); // excludedInterface is incoming interface
#ifdef GRAD_FILES
                    UpdateGradientFile();
#endif
                }
                if ( query_based && s->bestGradientToDeliver )
                {
                    start_reinforce(_buf, excludedInterface, s->seqno); // excludedInterface is incoming interface
#ifdef GRAD_FILES
                    UpdateGradientFile();
#endif
                }
            }
        }






        if ( !newCode && (query_based && s->bestGradientToDeliver) )
        {
            // place in queue for sending later

            new_packet* savePkt = packetCopy(sending_packet);
            addPkt(&(s->pktQ), savePkt);
            QueueDeletion* qd = (QueueDeletion*)malloc(sizeof(QueueDeletion));
            qd->pktToDelete = savePkt;
            qd->associatedState = s;
            setTimer(1.00, qd, delete_queued_data);




            // there is a best interest gradient, but it has not been
            // reinforced (yet), possibilities are:




            /*
             *
             * there is a best interest gradient, but it has not been
             * reinforced (yet), possibilities are:
             *
             * 1) reinforcement was failed to be received so reinforce now?
             *
             *    SO...
             *    Suggest a solution
             *    reinforce now, but how we know not to wait?
             *    we can check converged.  If the node HAS converged
             *    but there is no reinforcement we can assume
             *    that it is a failed to receive
             *
             *
             *
             *
             *
             * 2) It's just been cleared by a new seqno
             *
             *    Can we associate a seqno with a data message?
             *    We think NO!  Because, for example a seqno may be
             *    associated with a particular interest state and so
             *    each of these may have different seqnos dependant on
             *    need to reconverge, but a single data message may be
             *    associated with more than one interest grad (eg 2 dif' seqnos).
             *
             *    On the other hand...  you could have a seqno in a specific
             *    data packet, but you would have to have the prefix that was
             *    matched for grad selection in the packet too so you knew
             *    whether at the next node the specific prefix's seqno had
             *    been updated
             *
             *    NB: Should newseqnos clear the state action back to
             *    just 'SOURCE', if it is a source, no think not necessar
             *    having a prefix grad for a set source is a different thing
             *
             *    SO...
             *    Suggest a solution
             *    Just wait, a reinforcement will come along from
             *    behind shortly after the source gets convergence timeout
             *
             *    wait how long?
             *
             *
             * 3) It's still awaiting reinforcement for the current seqno,
             *    but is this possible?  Yes but only if we are source and
             *    data just arrived for forwarding FROM THE APP'
             *
             *    SO...
             *    Suggest a solution
             *    Just wait, a reinforcement will come along from
             *    behind shortly after the source gets convergence timeout
             *
             *    wait how long?
             *
             *    perhaps we can know timeouts have been set and reinforcements
             *    are awaited
             *
             *    Can we see if we are source
             *
             */

            // 3)
            //if ( s->prefix == FORWARD_AND_SOURCEPREFIX )
            //{
                // so wait for timeout and reinforcement
            //    new_packet* savePkt = packetCopy(sending_packet);
            //    addPkt(&(s->pktQ), savePkt);


            //    QueueDeletion* qd = (QueueDeletion*)malloc(sizeof(QueueDeletion));
            //    qd->pktToDelete = savePkt;
            //    qd->associatedState = s;
            //    setTimer(1.00, qd, delete_queued_data);

            //}


            // if converged, but reinforcement failed, then try reinforcement again
            // otherwise allow reinforcement to send queued data
            if( s->converged )
            {
                start_reinforce_interest(_buf, excludedInterface, s->seqno); // excludedInterface is incoming interface
#ifdef GRAD_FILES
                UpdateGradientFile();
#endif

                // send data straight away?

                //setTimer(0.01, savePkt, send_queued_data);

                /*
                 * start_reinforce_interest may be no good for a non-source
                 * node because it sets an obtain to self
                 *
                 * also there is a general issue that the obtain grad
                 * cannot be set because we had no incoming reinforcement
                 *
                 * we only have the incoming data...
                 * may be it would be correct to reinforce obtain
                 * grad for every prefix of incoming data
                 */


                // what about:
                //control_data cd;
                //cd.incoming_if = excludedInterface;  // i.e. incoming interface
                //handle_reinforce_interest(cd);
            }

        }
    }

    return sent;

}




void action_all_prefixes(trie *t, int i, int n, unsigned char *str, unsigned char* buf, NEIGHBOUR_ADDR _if, int process(State* s, unsigned char* _buf, NEIGHBOUR_ADDR _if))
{
      trie *current = t;

      if ( i >= n )
      {
           if ( current->s )
           {
               //RD( "\t- Found " );
               //RD( current->s->full_name << endl );
               //void f4(State* s, char* _buf, NEIGHBOUR_ADDR _if)
               *(buf) = '\0';
               process(current->s, current_prefix_name, _if);
           }
           return;
      }

      unsigned char c = str[i];
      t = t->first_child;
      t = trie_at_level(t,c);
      if(t == NULL)
      {
           for ( i++; i < n && str[i] != DOT; i++ );
           //if ( str[i] == '.' ) RD( "\t- Moving to next section " << endl );
           i--;
           t = current;
           action_all_prefixes(t, i+1, n, str, buf, _if, process);
      }
      else
      {
          *(buf) = c;
          //*(buf+1) = '\0';

          action_all_prefixes(t, i+1, n, str, buf+1, _if, process);
          for ( i++; i < n && str[i] != DOT; i++ );
          //if ( str[i] == '.' ) RD( "\t- Moving to next section " << endl );
          i--;
          t = current;
          action_all_prefixes(t, i+1, n, str, buf, _if, process);
      }

      return;

}







bool deliver(void* p1, void* p2)
{
	// In the long term I think this action (function) should be
	// delivering what is passed into it by the formal parameters, since it is
	// design to forward aggregated values etc...
	//
	//outgoing_packet.message_type = DATA;
	//outgoing_packet.data = 0;
	//outgoing_packet.length = 0;
	//outgoing_packet.path_value = 0;

	//outgoing_packet.the_data_message.message_type = DATA;
	//outgoing_packet.the_data_message.data_value._dataname_struct3.short1 = *p1;
	//outgoing_packet.the_data_message.data_value._dataname_struct3.short2 = *p2;
	//StateNode* sn = FindStateNode(rd->stateTree, outgoing_packet.the_data_message.data_value._dataname_struct1.the_dataname);




    //action_all_prefixes(rd->top_state, 0, strlen((const char*)outgoing_packet.data), (const char*)outgoing_packet.data,
    //        current_prefix_name, 0, consider_sending_data);
    action_all_prefixes(rd->top_state, 0, outgoing_packet.length, outgoing_packet.data,
            current_prefix_name, 0, consider_sending_data);
    return true;


	/*
    trie* t = trie_lookup_longest_prefix_extra2(rd->top_state, (const char*)outgoing_packet.data);

	// not quite right 'cos we just using the best interest gradient
	// rather than the reinforced one

	InterfaceList* temp = NULL;
	if ( t )
	{
		temp = t->s->deliveryInterfaces;
	}
	while( temp !=NULL )
	{
		// we have temporarily made some assumptions in the above and below code
		if ( temp->i->iName == SELF_INTERFACE )
		{
			// is the data item enough for the application
			// is length, path value etc etc needed?
			handleApplicationData(outgoing_packet.data);
		}
		else
		{
			//sendAMessage(0, 0); //????????????
			sendAMessage(temp->i->iName, write_packet());
		}
		temp = temp->link;
	}
	return true;
	*/

}




bool forward(void* p1, void* p2)
{
	//unsigned short temp1 = incoming_packet.the_data_message.data_value._dataname_struct3.short1;
	//unsigned short temp2 = incoming_packet.the_data_message.data_value._dataname_struct3.short2;
	//*p1 = incoming_packet.the_data_message.data_value._dataname_struct3.short1;
	//*p2 = incoming_packet.the_data_message.data_value._dataname_struct3.short2;

	outgoing_packet.message_type = incoming_packet.message_type;
	outgoing_packet.data = incoming_packet.data;
	outgoing_packet.length = incoming_packet.length;
	outgoing_packet.path_value = incoming_packet.path_value;
	outgoing_packet.down_interface = incoming_packet.down_interface;
	outgoing_packet.excepted_interface = incoming_packet.excepted_interface;
	outgoing_packet.seqno = incoming_packet.seqno;
	outgoing_packet.creation_time = incoming_packet.creation_time;

	sending_packet = &outgoing_packet;

	deliver(p1, p2);
	return false;

}



bool gtr(void* p1, void* p2)
{
	if ( *((unsigned short*)p1) > *((unsigned short*)p2) )
	{
		return true;
	}
	return false;

}


bool ngtr(void* p1, void* p2)
{
	return !gtr(p1, p2);
}





/*===================================================================================*/




bool (*a[11]) (void* p1, void* p2) =
{
null_action,
assign1,
assign2,
messageIn,
deliver,
messageIn,
gtr,
ngtr,
copyMessageIn,
forward,
addto
};









bool moveOnFSM(int theRole)
{
	unsigned char* raw_data;
	char size;
    char transitionSegment;
    char transitionList;
	char size_of_transitions;
	char num_of_transitions;
	transition* FSM;
	unsigned char* currentState;  // is this eclipsing the global static int one?

	raw_data = rd->role[theRole];
	size = raw_data[0];
    transitionSegment = raw_data[3];
    currentState = raw_data+transitionSegment;
    transitionList = transitionSegment+1;
	size_of_transitions = size - transitionSegment;
	num_of_transitions = size_of_transitions/sizeof(transition);
	FSM = (transition*)(raw_data+transitionList);

	bool transitioned = false;
	for ( int i = 0; i < num_of_transitions; i++ )
	{
		if ( FSM[i].source == *currentState )
		{
			/*
			4
			255
			0
			====
			12
			232
			3
			These bytes are the wrong way round - the byte we refer to is treated as the lsb
			and the next byte as the msb, we need to access param and param+1 and reverse them
			or we need to put them in the other way around
			*/
			void* p1 = (void*)(raw_data+FSM[i].param1);
			//unsigned short v1 = *p1;
			void* p2 = (void*)(raw_data+FSM[i].param2);
			//unsigned short v2 = *p2;
			//printf("State %d - Action %s ", *currentState, a[FSM[i].type]);
			if ( a[  (int)(FSM[i].type)  ](p1, p2) )
			{
				printf("Succeeded\n");
				printf("Moving to state number:    %d\n", FSM[i].dest);
				*currentState = FSM[i].dest;
				transitioned = true;
				return transitioned;
			}
			else
			{
				printf(": No change\n");
			}
		}
	}
	//printf("Stopping at state: %d\n", currentState);
	//getch();
	return transitioned;
}










void kickFSM()
{
	for ( int i = 0; rd->role[i] != 0 && i < 4; i++ )
	{
		bool _continue = true;
		//printf("MOVE ON FSM[0]\n");
		//printf("====================\n");
		while ( _continue )
		{
			_continue = moveOnFSM(i);
		}
	}

}












int interest_breakage_process_prefix(State* s, unsigned char* _buf, NEIGHBOUR_ADDR _if)
{

    // if a data packet has just arrived and finds a best grad but
    // no reinforcement then it can wait for convergence and send
    // when the node is fully converged

    // HOWEVER - If the data packet has just arrived but cannot be sent
    // on due to a breakage then it can be queued and we can wait for
    // complete reconvergence on a new seqno, however there is no saying that
    // this node will be used for delivery in this subsequent convergence

    // NBNB - This may also be the case when a data packet arrives at a node
    // that has jusy been noted not converged due to a new seqno, i.e. the finalisation
    // of convergence with the new seqno may not result in this node being used
    // in the reinforced path.

    // THEREFORE the data may be left orphaned waiting for reinforcement to trigger
    // it being forwarded!!!!!!!!!


    /*
     * was the broken interface in the delivery list for this prefix state?
     * i.e. in all likelihood
     * was it this prefix state that we matched on incoming data and then
     * used to find the now broken interface in its delivery interface list?
     */
    InterfaceList* temp = s->deliveryInterfaces;
    while( temp !=NULL )
    {
        if ( temp->i->iName == _if )
        {
            // If so, broadcast a BREAKAGE message for that prefix state

            // Only mark as broken when participated in forwarding
            // s->broken = 1;

            new_packet* savePkt = packetCopy(&broken_packet);
            addPkt(&(s->pktQ), savePkt);
            QueueDeletion* qd = (QueueDeletion*)malloc(sizeof(QueueDeletion));
            qd->pktToDelete = savePkt;
            qd->associatedState = s;
            setTimer(2.00, qd, delete_queued_data);

            outgoing_packet.message_type = BREAKAGE;
            outgoing_packet.data = _buf;
            outgoing_packet.length = strlen((const char*)_buf); // strl ok here
            outgoing_packet.path_value = 0;
            outgoing_packet.excepted_interface = UNKNOWN_INTERFACE;
            outgoing_packet.down_interface = _if; // probably discontinuing this feature
            outgoing_packet.seqno = s->seqno;
            bcastAMessage(write_packet(&outgoing_packet));
            break;
        }
        temp = temp->link;
    }

    if ( s->broken )
    {
        // MOVE TO ABOVE
        // new_packet* savePkt = packetCopy(&broken_packet);
        // addPkt(&(s->pktQ), savePkt);

        // LEAVE AS CONVERGED UNTIL NEW SEQNO INTEREST FROM SINK
        // s->converged = 0; // may be wait for the new seqno interest

        // LEAVE THIS UNTIL NEW SEQNO INTEREST FROM SINK
        // SO IF FAILED SUBSEQUENT DATA SEND FAILS CAN ALSO
        // TRY TO FIX BREAKAGE
        // freeInterfaceList(s->deliveryInterfaces);
        // s->deliveryInterfaces = NULL;
    }

    return 0;

}

// not used any more?
void interest_breakage_just_ocurred(unsigned char* pkt, NEIGHBOUR_ADDR inf, double creationTime)
{
    /*
     * A data record (interest/query based) was just sent on the matching
     * prefix state's delivery list interface
     * BUT the send failed and the parameters here have been provided
     * by a mac call back.
     *
     * He have actual data sent but not the matched prefix state
     * so check all matching ones for the broken interface
     *
     */

    read_broken_packet(pkt, creationTime);
    action_all_prefixes(rd->top_state, 0, broken_packet.length,
            broken_packet.data,
            current_prefix_name, inf, interest_breakage_process_prefix);

}



void initiate_new_advert(void* relevantObject)
{
    State* s = (State*)relevantObject;

    // think we should just use what we get in
    //t->s->seqno++;
    // may be we SHOULD NOT actually increase ABOVE
    // but just pass in below an increased value
    // becuase may be it later takes IMPORTANT action BASED ON whether
    // the seqno has just increased!!!!!!!!!!!!!!!!!!!!!!!!!

    static unsigned char data[20];
    GetStateStr(rd->top_state, s, data);

    weAreSourceFor(data, s->seqno+1);
    s->converged = 1; // SHOULD THIS ALWAYS BE '1' FOR STABLE SINK OF REC ???
    // NBNBNBNB  -  NOT YET CODED the roll over after 255 seqnos
    UcastAllBestGradients(rd->top_state, 0);

}



void initiate_new_interest(void* relevantObject)
{
    State* s = (State*)relevantObject;

    // think we should just use what we get in
    //t->s->seqno++;
    // may be we SHOULD NOT actually increase ABOVE
    // but just pass in below an increased value
    // becuase may be it later takes IMPORTANT action BASED ON whether
    // the seqno has just increased!!!!!!!!!!!!!!!!!!!!!!!!!

    static unsigned char data[20];
    GetStateStr(rd->top_state, s, data);

    weAreSinkFor(data, s->seqno+1);
    s->converged = 1; // SHOULD THIS ALWAYS BE '1' FOR STABLE SINK OF REC ???
    // NBNBNBNB  -  NOT YET CODED the roll over after 255 seqnos
    UcastAllBestGradients(rd->top_state, 0);

}





void handle_breakage(control_data cd)
{
    // NBNBNBNBNBNBNBNBNB
    // ==============================
    // I think this breakage will get network broadcasted EVERYWHERE til
    // it reaches the oroginator, but for adverts it only needs to stay
    // within the relevant context
    // POSSIBLE FIX NEEDED

    // we can use this if it is pefix state coming and not actual data
    trie* t = trie_add(rd->top_state, incoming_packet.data, STATE);

    bool broken = t->s->broken;
    bool this_seqno = ( incoming_packet.seqno == t->s->seqno );

    if ( this_seqno && !broken )
    {
        if ( t->s->action == SINK_ACTION )
        {
            // think we should just use what we get in
            //t->s->seqno++;
            // may be we SHOULD NOT actually increase ABOVE
            // but just pass in below an increased value
            // becuase may be it later takes IMPORTANT action BASED ON whether
            // the seqno has just increased!!!!!!!!!!!!!!!!!!!!!!!!!
            //weAreSinkFor(incoming_packet.data, t->s->seqno+1);
            // NBNBNBNB  -  NOT YET CODED the roll over after 255 seqnos
            //UcastAllBestGradients(rd->top_state, 0);

            t->s->broken = 1;
            setTimer(0.03, t->s, initiate_new_interest);
        }
        else
        {
            t->s->broken = 1;
            outgoing_packet.message_type = BREAKAGE;
            outgoing_packet.length = incoming_packet.length;
            outgoing_packet.data = incoming_packet.data;
            outgoing_packet.path_value = 0;
            outgoing_packet.excepted_interface = UNKNOWN_INTERFACE;
            outgoing_packet.down_interface = UNKNOWN_INTERFACE; // discontinuing?
            outgoing_packet.seqno = incoming_packet.seqno;
            bcastAMessage(write_packet(&outgoing_packet));
        }
    }
    // else
    // !this_seqno && !broken ||
    // this_seqno && broken ||
    // !this_seqno && broken

}


/*================================ MESSAGE HANDLERS ==============================*/



void (*h[11]) (control_data cd) =
{
handle_advert,
handle_interest,
handle_reinforce,
handle_data,
handle_neighbor_bcast,
handle_neighbor_ucast,
handle_reinforce_interest,
handle_collaboration,
handle_reinforce_collaboration,
handle_interest_correction,
handle_breakage
};










void setMessageCallBack(void (*_sendAMessage) (NEIGHBOUR_ADDR _interface, unsigned char* _msg, double _creationTime))
{
	sendAMessage = _sendAMessage;
}
 
void setBroadcastCallBack(void (*_bcastAMessage) (unsigned char* _msg))
{
	bcastAMessage = _bcastAMessage;
}
 

void setApplicationCallBack(void (*_handleApplicationData) (unsigned char* _msg, double _creationTime))
{
	handleApplicationData = _handleApplicationData;
}


void setTimerCallBack(void (*_setTimer) (TIME_TYPE timeout, void* relevantObject, void timeout_callback(void* relevantObject)))
{
    setTimer = _setTimer;
}




// Name Level 1
#define DEMANDPUBLICATION 129
#define SUPPLYPUBLICATION 130
#define ENVIRONMENTPUBLICATION 131
#define DEMANDRECORD 1
#define SUPPLYRECORD 2
#define ENVIRONMENTRECORD 3

// Name Level 2




void weAreSourceFor(unsigned char* _data, char seqno)
{
#ifdef XXXXXX
    trie* t = trie_add(rd->top_state, _data, STATE);
#endif

    if ( ((*_data) & MSB2) == PUBLICATION )
	{
	    // reboot not properly supported for now, first seqno always zero
		setObtainGradient(_data, SELF_INTERFACE, 0, seqno);
#ifdef XXXXXX
		if ( advertRetries < 2 ) // needs improving
		    // if 1 then we are about to retry first time
		    // if 2 then we are about to retry second time so don't reset timer
		{
	        setTimer(ADVERT_TIMEOUT, t->s, initiate_new_advert);
	        advertRetries++;
		}
#endif
	}

	//StateNode* sn = FindStateNode(rd->stateTree, _data._dataname_struct1.the_dataname);
	//struct StateNode* sn = InsertStateNode(&(rd->stateTree), _data._dataname_struct1.the_dataname);

#ifndef XXXXXX
	trie* t = trie_add(rd->top_state, _data, STATE);
#endif

	if ( t )
	{

	}
    t->s->action = SOURCE_ACTION;

	//addName(&SourceNames, _data);


}


void weAreSinkFor(unsigned char* _data, char seqno)
{
	if ( ((*_data) & MSB2) == RECORD )
	{
		// For now we only support network restart not sink restart
	    // ie always begin with zero seqno
	    setDeliverGradient(_data, SELF_INTERFACE, 0, seqno);
	}

	//StateNode* sn = FindStateNode(rd->stateTree, _data._dataname_struct1.the_dataname);
	//struct StateNode* sn = InsertStateNode(&(rd->stateTree), _data._dataname_struct1.the_dataname);
	trie* t = trie_add(rd->top_state, _data, STATE);

	t->s->action = SINK_ACTION;
	//addName(&SinkNames, _data);
}




void weAreCollaboratorInitiatorFor(unsigned char* _data, char seqno)
{
    if ( ((*_data) & MSB2) == COLLABORATIONBASED )
    {
        // setObtainGradient(_data, SELF_INTERFACE, 0);   ??????

        // For now we only support network restart not sink restart
        // ie always begin with zero seqno
        setDeliverGradient(_data, SELF_INTERFACE, 0, seqno);
    }

    //StateNode* sn = FindStateNode(rd->stateTree, _data._dataname_struct1.the_dataname);
    //struct StateNode* sn = InsertStateNode(&(rd->stateTree), _data._dataname_struct1.the_dataname);
    trie* t = trie_add(rd->top_state, _data, STATE);

    t->s->action = COLLABORATE_INITIATOR__ACTION;
    //addName(&SinkNames, _data);
}






void weAreCollaboratorFor(unsigned char* _data, char seqno)
{
    if ( ((*_data) & MSB2) == COLLABORATIONBASED )
    {
        // setObtainGradient(_data, SELF_INTERFACE, 0);   ??????
        //setDeliverGradient(_data, SELF_INTERFACE, 0);   ??????
    }

    //StateNode* sn = FindStateNode(rd->stateTree, _data._dataname_struct1.the_dataname);
    //struct StateNode* sn = InsertStateNode(&(rd->stateTree), _data._dataname_struct1.the_dataname);
    trie* t = trie_add(rd->top_state, _data, STATE);

    t->s->action = COLLABORATE_ACTION;
    //addName(&SinkNames, _data);
}



/*
void weAreCollaboratorInitiatorFor(char* _data)
{
    if ( ((*_data) & MSB2) == COLLABORATIONBASED )
    {
        // setObtainGradient(_data, SELF_INTERFACE, 0);   ??????
        setDeliverGradient(_data, SELF_INTERFACE, 0);
    }

    //StateNode* sn = FindStateNode(rd->stateTree, _data._dataname_struct1.the_dataname);
    //struct StateNode* sn = InsertStateNode(&(rd->stateTree), _data._dataname_struct1.the_dataname);
    trie* t = trie_add(rd->top_state, _data, STATE);

    t->s->action = COLLABORATE_INITIATOR__ACTION;
    //addName(&SinkNames, _data);
}
*/




#ifdef GRAD_FILES

static std::ofstream myfile;



static void write_one_gradient(KDGradientNode* g, unsigned char* _name)
{
    myfile << "CONNECTION" << std::endl;
    myfile << std::hex << std::uppercase << thisAddress << std::endl;
    for ( int i = 0; _name[i] != 0; i++ )
    {
        //unsigned char ch = (unsigned char)_name[i];
        myfile.width(2);
        myfile.fill('0');
        myfile << std::hex << std::uppercase << (unsigned int)_name[i];
    }
    //if ( strlen((const char*)_name) == 8 )
    //{
    //    int x = 0;
    //    RD( "x: " << x << "\n" );
    //}
    myfile << std::endl;
    myfile << std::dec << g->key1->action << std::endl;
    myfile << std::hex << std::uppercase << g->key2->iName << std::endl;
    myfile << std::dec << "seqno: " << (unsigned int)g->seqno << std::endl;
    myfile << std::boolalpha << (bool)g->key2->up << std::endl;
    myfile << std::dec << g->costToDeliver << std::endl;
    myfile << std::dec<< g->costToObtain << std::endl;
    myfile << std::boolalpha << (g->key1->bestGradientToDeliver == g) << std::endl;
    myfile << std::boolalpha <<  deliverReinforced(g) << std::endl;
    myfile << std::boolalpha << (g->key1->bestGradientToObtain == g) << std::endl;
    myfile << std::boolalpha <<  obtainReinforced(g) << std::endl;

}



void UpdateGradientFile()
{
    std::string s;
    std::ostringstream ss;
    ss.clear();
    ss.str(s);
    ss << "C:\\GradientData\\" << std::hex << std::uppercase << thisAddress << "Connections.txt";

    //int remove_failure = std::remove(ss.str().c_str());
    //if ( remove_failure )
    //{
    //    EV << "File removal failure " << remove_failure << " \n";
    //}
    myfile.open (ss.str().c_str(), std::ios::trunc);

    // need to improve this
    //write_connections(write_one_connection);
    write_gradients(write_one_gradient);
    myfile.close();

}

#endif


void send_data(int len, unsigned char* _data, double _creationTime)
{
    incoming_packet.message_type = DATA;
    incoming_packet.length = len;
    free(incoming_packet.data);
    incoming_packet.data = (unsigned char*)malloc(incoming_packet.length+1);
    memcpy(incoming_packet.data, _data, incoming_packet.length);
    incoming_packet.data[incoming_packet.length] = 0;
    incoming_packet.path_value = 0;
    incoming_packet.down_interface = UNKNOWN_INTERFACE;
    incoming_packet.excepted_interface = UNKNOWN_INTERFACE;
    incoming_packet.creation_time = _creationTime;
    control_data cd;
    cd.incoming_if = SELF_INTERFACE;
    cd.incoming_lqi = 0;
    handle_data(cd);
}



void handle_message(unsigned char* _msg, NEIGHBOUR_ADDR inf, unsigned char lqi, double creationTime)
{
#ifdef MAKE_LOCAL_VARIABLES_FOR_DEBUGGER
    // CODE FOR VIEWING VARIABLES IN DEBUGGER ONLY
    unsigned char _msg_view[25];
    memcpy(_msg_view, _msg, 25);
#endif

    read_packet(_msg, creationTime);

#ifdef MAKE_LOCAL_VARIABLES_FOR_DEBUGGER
    // CODE FOR VIEWING VARIABLES IN DEBUGGER ONLY
    struct new_packet incoming_packet_view;
    incoming_packet_view = incoming_packet;
#endif




    rd->pkts_received++;

    unsigned int mt1 = (unsigned int)incoming_packet.message_type;
    DBTYPERECV(RD( "received msg type: " << mt1 << std::endl);)
    switch ( incoming_packet.message_type )
    {
        //case NEIGHBOR_UCAST:
        //case NEIGHBOR_BCAST:
        case ADVERT:
        case INTEREST:
        case COLLABORATION:
        case INTEREST_CORRECTION:
            // NBNBNB:
            // THIS 'MAY' BE WRONG  -  THINK ABOUT MULTIPLE DATA NAME ADVS AND INTRSTS
            //if ( !FindInterfaceNode(rd->interfaceTree, inf) &&
            //        CountInterfaceNodes(rd->interfaceTree) > 9 )
            //{
            //    return;

            //}

            if ( !FindInterfaceNode(rd->interfaceTree, inf) )
            {
                unsigned int numNeighbours = CountInterfaceNodes(rd->interfaceTree);
                if ( numNeighbours > 9 )
                {
                    rd->pkts_ignored++;
                    return;
                }
                else
                {
                    unsigned char lqiThreshold = numNeighbours > 4 ? 100 : numNeighbours * 20;
                    if ( lqi < lqiThreshold )
                    {
                        rd->pkts_ignored++;
                        return;
                    }
                }
            }




            // THOUGHT THIS WAS NOT OK (Due to seqno idea)  -    BUT NOW THINK ITS OK
            // Because it checks whether we have already accepted the if (OFCOURSE)

            //if ( !FindInterfaceNode(rd->interfaceTree, inf) &&
            //        numNeighbours > 9 )
            //{
            //    rd->pkts_ignored++;
            //    return;
            //}
            break;
    }
    unsigned int mt2 = (unsigned int)incoming_packet.message_type;
    DBTYPETHROUGH(RD( "type: " << mt2 << " is through" << std::endl );)

	//unsigned int mylqi = 7 * (lqi/255);
    //unsigned int coutLqi = (unsigned int)lqi;
    //RD( "LQI: " << std::dec << coutLqi << std::endl );
    //RD( "MYLQI: " << mylqi << std::endl );
	//incoming_packet.path_value += mylqi;

	control_data cd;
	cd.incoming_if = inf;
	cd.incoming_lqi = 0xFF - lqi;
	cd.incoming_lqi = cd.incoming_lqi == 0 ? 1 : cd.incoming_lqi;


	//InterfaceNode* in = FindInterfaceNode(rd->interfaceTree, inf);
	//InsertInterfaceNode();






	(*h[incoming_packet.message_type]) (cd);

#ifdef GRAD_FILES
	switch ( incoming_packet.message_type )
	{
    case ADVERT:
    case INTEREST:
    case COLLABORATION:
    case REINFORCE:
    case REINFORCE_INTEREST:
    case REINFORCE_COLLABORATION:
    case INTEREST_CORRECTION:
    case REINFORCE_INTEREST_CANCEL:
        UpdateGradientFile();
	    break;
	}
#endif



}



/*
 * In theory link cost can be informed by the context, eg type of routing, type of data etc...
 */
unsigned short outgoingLinkCost(control_data cd)
{
#ifdef USE_NODE_STABILITY_COST
    unsigned short outgoingLink_cost = incoming_packet.path_value+(unsigned short)cd.incoming_lqi+(unsigned short)nodeConstraint;
#else
    unsigned short outgoingLink_cost = incoming_packet.path_value+(unsigned short)cd.incoming_lqi;
#endif




    return outgoingLink_cost;
}




unsigned short incomingLinkCost(control_data cd)
{
#ifdef USE_NODE_STABILITY_COST
    unsigned short incomingLink_cost = incoming_packet.path_value+(unsigned short)cd.incoming_lqi;
#else
    unsigned short incomingLink_cost = incoming_packet.path_value+(unsigned short)cd.incoming_lqi;
#endif
    return incomingLink_cost;
}




void interest_convergence_timeout(void* relevantObject)
{
    //unsigned char* data = (unsigned char*)relevantObject;
    //trie* t = trie_add(rd->top_state, data, STATE);
    //State* s = t->s;

    State* s = (State*)relevantObject;
    s->converged = 1;

    if ( s->prefix == FORWARD_AND_SOURCEPREFIX
            && s->bestGradientToDeliver
            && !s->deliveryInterfaces )
    {
        static unsigned char data[20];
        GetStateStr(rd->top_state, s, data);
        // second parameter no longer used, pos pass s in future to save
        // unnecessary work in the function
        start_reinforce_interest(data, SELF_INTERFACE, s->seqno);
#ifdef GRAD_FILES
        UpdateGradientFile();
#endif
    }


/*
 * This applies to intervening nodes (not sources) which have a newly
 * converged best grad to the sink and have queued data waiting
 * they can reinforce to the sink and get that data sent sooner
 * than waiting for / depending on a source to reinforce from behind them
 */
    if ( s->prefix != FORWARD_AND_SOURCEPREFIX && s->pktQ
            && s->bestGradientToDeliver
            && !s->deliveryInterfaces )
    {
        static unsigned char data[20];
        GetStateStr(rd->top_state, s, data);
        // second parameter no longer used, pos pass s in future to save
        // unnecessary work in the function
        start_reinforce_interest(data, UNKNOWN_INTERFACE, s->seqno);
#ifdef GRAD_FILES
        UpdateGradientFile();
#endif
    }

    /*
     * I think we may also need to reinforce interest when
     * there are items in the queue for this state because
     * after a (re)convergence it is not guarenteed that (where
     * a data packet has previously stopped due to an immediate breakage
     * or an ongoing new seqno reconvergence due to a breakage else where)
     * the grad will be reinforced at that node from behind, i.e. the new
     * convergence may result in a different path
     */

}









void collaboration_convergence_timeout(void* relevantObject)
{
    //unsigned char* data = (unsigned char*)relevantObject;
    //trie* t = trie_add(rd->top_state, data, STATE);
    //State* s = t->s;

    State* s = (State*)relevantObject;
    s->converged = 1;

    if ( s->prefix == FORWARD_AND_COLLABORATIONPREFIX
            && s->bestGradientToDeliver
            && !s->deliveryInterfaces )
    {
        static unsigned char data[20];
        GetStateStr(rd->top_state, s, data);
        // second parameter no longer used, pos pass s in future to save
        // unnecessary work in the function
        start_reinforce_collaboration(data, SELF_INTERFACE, s->seqno);
#ifdef GRAD_FILES
        UpdateGradientFile();
#endif
    }

    if ( s->prefix != FORWARD_AND_COLLABORATIONPREFIX && s->pktQ
            && s->bestGradientToDeliver
            && !s->deliveryInterfaces )
    {
        static unsigned char data[20];
        GetStateStr(rd->top_state, s, data);
        // second parameter no longer used, pos pass s in future to save
        // unnecessary work in the function
        start_reinforce_collaboration(data, UNKNOWN_INTERFACE, s->seqno);
#ifdef GRAD_FILES
        UpdateGradientFile();
#endif
    }


}













//HEREHERE
int isPrefix(State* s, unsigned char* _buf, NEIGHBOUR_ADDR _if)
{
    // pos condition record here?  no need must be if its a prefix

    //s->prefix = FORWARD_AND_SOURCEPREFIX;

    if ( ((*_buf) & MSB2) == RECORD )
    {
        s->prefix = FORWARD_AND_SOURCEPREFIX;
    }

    if ( ((*_buf) & MSB2) == COLLABORATIONBASED )
    {
        s->prefix = FORWARD_AND_COLLABORATIONPREFIX;
    }





    return 0;
}



int numSinks;
int count_sink(State* s, unsigned char* _buf, NEIGHBOUR_ADDR _if)
{
    if ( s->action == SINK_ACTION )
    {
        if ( ((*_buf) & MSB2) == PUBLICATION )
        {
            numSinks++;
        }
    }

    return 0;

}








void setAllPrefixStatus(State* s, unsigned char* _data, NEIGHBOUR_ADDR _if)
{
    if ( s->action == SOURCE_ACTION )
    {
        /*
         * If ANY state is a prefix of THIS source record state
         * then mark EACH as a source prefix (prefix of a source record state)
         */
        //if ( ((*_data) & MSB2) == RECORD )
        //{
            action_all_prefixes(rd->top_state, 0, strlen((const char*)_data), _data,
                   current_prefix_name, 0, isPrefix); // CHECK IF ZERO OK? YES.
        //}
    }

    if ( s->action == COLLABORATE_ACTION )
    {
        /*
         * If ANY state is a prefix of THIS source record state
         * then mark EACH as a source prefix (prefix of a source record state)
         */
        //if ( ((*_data) & MSB2) == COLLABORATIONBASED )
        //{
            action_all_prefixes(rd->top_state, 0, strlen((const char*)_data), _data,
                   current_prefix_name, 0, isPrefix); // CHECK IF ZERO OK? YES.
        //}
    }

    numSinks = 0;
    // pos condition pub here? not sure.
    action_all_prefixes(rd->top_state, 0, strlen((const char*)_data), _data,
            current_prefix_name, _if, count_sink);
    /*
     * If THIS state is a suffix of ANY sink state
     * then mark IT as a sink suffix (suffix of a sink state)
     */
    if ( numSinks )
    {
        s->prefix = FORWARD_AND_SINK_SUFFIX;
    }

}

/*
RD( "FOUND UNREINFORCED ADVERT" << std::endl );
numSinks = 0;
action_all_prefixes(rd->top_state, 0, strlen((const char*)_data), _data,
        current_prefix_name, _if, count_sink);
if ( numSinks )
{
    RD( "TRY REINFORCING ADVERT" << std::endl );
    // second parameter no longer used, pos pass s in future to save
    // unnecessary work in the function
    start_reinforce(_data, 0, s->seqno);
}
*/
















void handle_interest_dummy(control_data cd)
{
    RD( "Interest received from: " << cd.incoming_if << " lqi: " << cd.incoming_lqi << std::endl );

    NEIGHBOUR_ADDR _interface = cd.incoming_if;
    trie* t = trie_add(rd->top_state, incoming_packet.data, STATE);

    // CHECK THIS IS RIGHT - ITS TO DO WITH OLD SEQNO
    State* s = t->s;

    if ( incoming_packet.seqno < s->seqno )
    {
        return; // I think this is right, i.e. ignore any old seq ints/advs
        // CHECK THIS
    }

    if ( (incoming_packet.seqno == s->seqno) && s->converged )
    {
        return;
    }

    if ( incoming_packet.seqno > s->seqno )
    {
        /*
         * IF: this is a rec and we are the sink for it
         * i.e. we have a best grad already set that is this data rec
         * interface to self and cost zero (and most likely grad and state seqno zero).
         *
         * THEN:  we must leave the best grad to self in place, set its seqno to this
         * incoming seqno value +1 and the same for the state object.
         *
         * ACTUALLY we JUST want to send out the initiating interest again with the
         * 'brand new' seqno.  I.E. the best grad with IF: Self COST: Zero.
         *
         *
         */
        if ( s->bestGradientToDeliver &&
                s->bestGradientToDeliver->key2->iName == SELF_INTERFACE &&
                s->bestGradientToDeliver->costToDeliver == 0 &&
                s->bestGradientToDeliver->seqno == 0 &&
                s->seqno == 0) // pos improve condition
        {
            s->bestGradientToDeliver->seqno = incoming_packet.seqno+1;
            s->seqno = incoming_packet.seqno+1;
            //s->bestGradientToDeliverUpdated = 0;

            outgoing_packet.message_type = INTEREST;

            outgoing_packet.length = incoming_packet.length;
            outgoing_packet.data = incoming_packet.data;

            //outgoing_packet.length = strlen(queue); // strlen ok in this case
            //outgoing_packet.data = (unsigned char*)queue; // cannot get

            outgoing_packet.path_value = 0;
            //outgoing_packet.excepted_interface = s->bestGradientToDeliver->key2->iName;
            outgoing_packet.excepted_interface = UNKNOWN_INTERFACE; // not used now?
            outgoing_packet.down_interface = UNKNOWN_INTERFACE; // not used now?
            outgoing_packet.seqno = s->seqno;

            RD( "Resending initiating interest packet [new seqno]" << std::endl );
            bcastAMessage(write_packet(&outgoing_packet));
        }
    }

    int inserted;
    Interface* i = InsertInterfaceNode(&(rd->interfaceTree), _interface, &inserted)->i;
    i->lqi = cd.incoming_lqi;

    setDeliverGradient(incoming_packet.data, _interface, incomingLinkCost(cd), incoming_packet.seqno);

    // check this out!!
    if ( incoming_packet.down_interface == thisAddress )
        return;

    if ( t )
    {
        if ( t->s->bestGradientToDeliverUpdated )
        {
            // THIS MECHANISM FOR NOW BUT MAY NEED A SUFFIX FUNCTION

            // this is a once only on first sight for all the (interest) states
            // we have ever seen, to say whether or not it is a prefix of one
            // that was previously set as SOURCE_ACTION
            if ( t->s->prefix != FORWARD_AND_SOURCEPREFIX &&
                    t->s->prefix != PREFIX_CHECKED )
            {
                traverse(rd->top_state, queue, 0, setAllPrefixStatus);
                if ( t->s->prefix != FORWARD_AND_SOURCEPREFIX )
                {
                    t->s->prefix = PREFIX_CHECKED;
                }
            }

            // try doing timeout for ALL nodes
            t->s->converged = 0;
            setTimer(0.2, t->s, interest_convergence_timeout);

            t->s->bestGradientToDeliverUpdated = false;
            outgoing_packet.message_type = INTEREST;
            outgoing_packet.data = incoming_packet.data;
            outgoing_packet.length = incoming_packet.length;
            outgoing_packet.path_value = outgoingLinkCost(cd);
            outgoing_packet.excepted_interface = _interface;
            outgoing_packet.down_interface = UNKNOWN_INTERFACE;
            outgoing_packet.seqno = incoming_packet.seqno; // IS THIS RIGHT - WHAT IF CHANGED BY setDeliverGradient?
            bcastAMessage(write_packet(&outgoing_packet));
        }
    }

}


//HEREAGAIN


/*
 * Should these start_reinforce... functions be incorporated into the handle ones
 * only treat as arrived from SELF?
 */
// probably not thread safe
void start_reinforce(unsigned char* fullyqualifiedname, NEIGHBOUR_ADDR _if, char seqno)
{
    RD( "ABOUT to ADV Reinforce" << std::endl );
    // did we already do this at start up when we made us a sink for this name?
    // NO!!! I think we did set best grad but did not REINFORCE


    if ( _if != UNKNOWN_INTERFACE )
    {
        reinforceDeliverGradient(fullyqualifiedname, SELF_INTERFACE, seqno);
    }


    //StateNode* sn = FindStateNode(rd-, specific_data_value._dataname_struct1.the_dataname);
    trie* t = trie_add(rd->top_state, fullyqualifiedname, STATE);




    if ( t && t->s->bestGradientToObtain )
    {
        // next hop already reinforced so stop here
#ifndef XXXXXX // if XXXXXX the reinforce all the way through
        if ( t->s->obtainInterface )
            return;
#endif

        // find the next interface to reinforce
        NEIGHBOUR_ADDR interface = t->s->bestGradientToObtain->key2->iName;

        // also reinforce the path to the source for breakage messages
        // even if it is self
        reinforceObtainGradient(fullyqualifiedname, interface, seqno);
        // INCLUDE THIS?
        setTimer(0.01, t->s, send_queued_data);

        // If interface is self do not send message on
        if ( interface == SELF_INTERFACE )
            return;

        outgoing_packet.message_type = REINFORCE;
        outgoing_packet.data = (unsigned char*)fullyqualifiedname;
        outgoing_packet.length = strlen((const char*)outgoing_packet.data);
        outgoing_packet.down_interface = UNKNOWN_INTERFACE;
        outgoing_packet.excepted_interface = UNKNOWN_INTERFACE;
        outgoing_packet.path_value = 0; // consider at some point whether this zero is right at start?
        outgoing_packet.seqno = seqno;
        RD( "FIRST ADV Reinforce to " << std::hex << interface << std::endl );
        sendAMessage(interface, write_packet(&outgoing_packet), 0);
    }

}


#ifdef XXXXXX
// using initiate_new_advert instead
//void advert_retry(void* relevantObject)
//{
//}
#endif



#ifdef XXXXXX
void start_reinforce_retry(void* relevantObject)
{
    State* s = (State*)relevantObject;
    static unsigned char data[20];
    GetStateStr(rd->top_state, s, data);
#ifdef XXXXXX
    if ( advertReinforceRetries < 2 ) // needs improving
        // if 1 then we are about to retry first time
        // if 2 then we are about to retry second time so don't reset timer
    {
        setTimer(REINFORCE_TIMEOUT, s, start_reinforce_retry);
        advertReinforceRetries++;
    }
#endif
    start_reinforce(data, SELF_INTERFACE, s->seqno);
}
#endif

void advert_convergence_timeout(void* relevantObject)
{
    //unsigned char* data = (unsigned char*)relevantObject;
    //trie* t = trie_add(rd->top_state, data, STATE);
    //State* s = t->s;

    State* s = (State*)relevantObject;
    s->converged = 1;

    if ( s->prefix == FORWARD_AND_SINK_SUFFIX
            && s->bestGradientToObtain
            && !s->obtainInterfaces )
    {
        static unsigned char data[20];
        GetStateStr(rd->top_state, s, data);
        // second parameter no longer used, pos pass s in future to save
        // unnecessary work in the function
        start_reinforce(data, SELF_INTERFACE, s->seqno);
#ifdef XXXXXX
        setTimer(REINFORCE_TIMEOUT, s, start_reinforce_retry);
        advertReinforceRetries++;
#endif

#ifdef GRAD_FILES
        UpdateGradientFile();
#endif
    }


/*
 * This code doesn't really help - except may be as a way to get
 * data back for re-sending, and also it may create delivery interfaces
 * from the src to tthis node which may be a dead end.  Look at this more
 * when finishing off work for event based data breakages
    if ( s->prefix != FORWARD_AND_SINK_SUFFIX && s->pktQ
            && s->bestGradientToObtain
            && !s->obtainInterfaces )
    {
        static unsigned char data[20];
        GetStateStr(rd->top_state, s, data);
        // second parameter no longer used, pos pass s in future to save
        // unnecessary work in the function
        start_reinforce(data, UNKNOWN_INTERFACE, s->seqno);
#ifdef GRAD_FILES
        UpdateGradientFile();
#endif
    }
*/

    /*
     * I think we may also need to reinforce interest when
     * there are items in the queue for this state because
     * after a (re)convergence it is not guarenteed that (where
     * a data packet has previously stopped due to an immediate breakage
     * or an ongoing new seqno reconvergence due to a breakage else where)
     * the grad will be reinforced at that node from behind, i.e. the new
     * convergence may result in a different path
     */

}






void handle_advert(control_data cd)
{
    RD( "Advert received from: " << cd.incoming_if << " lqi: " << cd.incoming_lqi << std::endl );
    NEIGHBOUR_ADDR _interface = cd.incoming_if;

    trie* t;
    char dot = DOT;
    unsigned char* ptr = (unsigned char*)strchr((char*)incoming_packet.data, dot);
    if ( ptr )
    {
        t = trie_lookup2(rd->top_context, ptr+1);
    }
    if ( !t )
    {
        return;
    }

    t = trie_add(rd->top_state, incoming_packet.data, STATE);

    // CHECK THIS IS RIGHT - ITS TO DO WITH OLD SEQNO
    State* s = t->s;

    if ( incoming_packet.seqno < s->seqno )
    {
        return; // I think this is right, i.e. ignore any old seq ints/advs
        // CHECK THIS
    }

    if ( (incoming_packet.seqno == s->seqno) && s->converged )
    {
        return;
    }

    if ( incoming_packet.seqno > s->seqno )
    {
        /*
         * IF: this is a rec and we are the sink for it
         * i.e. we have a best grad already set that is this data rec
         * interface to self and cost zero (and most likely grad and state seqno zero).
         *
         * THEN:  we must leave the best grad to self in place, set its seqno to this
         * incoming seqno value +1 and the same for the state object.
         *
         * ACTUALLY we JUST want to send out the initiating interest again with the
         * 'brand new' seqno.  I.E. the best grad with IF: Self COST: Zero.
         *
         *
         */
        if ( s->bestGradientToObtain &&
                s->bestGradientToObtain->key2->iName == SELF_INTERFACE &&
                s->bestGradientToObtain->costToObtain == 0 &&
                s->bestGradientToObtain->seqno == 0 &&
                s->seqno == 0) // pos improve condition
        {
            s->bestGradientToObtain->seqno = incoming_packet.seqno+1;
            s->seqno = incoming_packet.seqno+1;
            //s->bestGradientToDeliverUpdated = 0;

            outgoing_packet.message_type = ADVERT;

            outgoing_packet.length = incoming_packet.length;
            outgoing_packet.data = incoming_packet.data;

            //outgoing_packet.length = strlen(queue); // strlen ok in this case
            //outgoing_packet.data = (unsigned char*)queue; // cannot get

            outgoing_packet.path_value = 0;
            //outgoing_packet.excepted_interface = s->bestGradientToDeliver->key2->iName;
            outgoing_packet.excepted_interface = UNKNOWN_INTERFACE; // not used now?
            outgoing_packet.down_interface = UNKNOWN_INTERFACE; // not used now?
            outgoing_packet.seqno = s->seqno;

            RD( "Resending initiating advert packet [new seqno]" << std::endl );
            bcastAMessage(write_packet(&outgoing_packet));
        }
    }

    int inserted;
    Interface* i = InsertInterfaceNode(&(rd->interfaceTree), _interface, &inserted)->i;
    i->lqi = cd.incoming_lqi;

    setObtainGradient(incoming_packet.data, _interface, incomingLinkCost(cd), incoming_packet.seqno);

    if ( t )
    {
        if ( t->s->bestGradientToObtainUpdated )
        {
            // THIS MECHANISM FOR NOW BUT MAY NEED A SUFFIX FUNCTION

            // this is a once only on first sight for all the (interest) states
            // we have ever seen, to say whether or not it is a prefix of one
            // that was previously set as SOURCE_ACTION
            if ( t->s->prefix != FORWARD_AND_SINK_SUFFIX &&
                    t->s->prefix != PREFIX_CHECKED )
            {
                traverse(rd->top_state, queue, 0, setAllPrefixStatus);
                if ( t->s->prefix != FORWARD_AND_SINK_SUFFIX )
                {
                    t->s->prefix = PREFIX_CHECKED;
                }
            }

            // try doing timeout for ALL nodes
            t->s->converged = 0;
            setTimer(0.2, t->s, advert_convergence_timeout);






            t->s->bestGradientToObtainUpdated = false;
            outgoing_packet.message_type = ADVERT;
            outgoing_packet.data = incoming_packet.data;
            outgoing_packet.length = incoming_packet.length;
            outgoing_packet.path_value = outgoingLinkCost(cd);
            outgoing_packet.excepted_interface = _interface;
            outgoing_packet.down_interface = UNKNOWN_INTERFACE;
            outgoing_packet.seqno = incoming_packet.seqno; // IS THIS RIGHT - WHAT IF CHANGED BY setDeliverGradient?
            bcastAMessage(write_packet(&outgoing_packet));
        }
	}


}







void handle_advert_almost_same_as_old(control_data cd)
{
    RD( "Advert received from: " << cd.incoming_if << " lqi: " << cd.incoming_lqi << std::endl );
    NEIGHBOUR_ADDR _interface = cd.incoming_if;

    trie* t = trie_add(rd->top_state, incoming_packet.data, STATE);

    // CHECK THIS IS RIGHT - ITS TO DO WITH OLD SEQNO
    State* s = t->s;

    if ( incoming_packet.seqno < s->seqno )
    {
        return; // I think this is right, i.e. ignore any old seq ints/advs
        // CHECK THIS
    }

    if ( (incoming_packet.seqno == s->seqno) && s->converged )
    {
        return;
    }
















    //static rpacket p;
    //static struct StateNode* n;
    //trie* t;
    //static struct KDGradientNode* k;

    // a gradient to self with zero obtain cost indicates this is the source
    // hopefully can get away without this cos setobtaingrad will do nothing
    // cos it already has a gradient to self of zero cost and bestgradienttoobtain
    // will NOT have been updated
    //if ( (k = SearchForKDGradientNode(incoming_packet.the_message.data_value, SELF_INTERFACE, rd->grTree)) )
    //{
    //  if (  k->costToDeliver == 0 )
    //  {
    //      return;
    //  }
    //}

    // possibly removing this
    //if ( SearchForKDGradientNode(incoming_packet.the_message.data_value, SELF_INTERFACE, rd->grTree) )
    //{
    //  return;
    //}

    //dataname_struct dataName;
    //char messageScope;

    //dataName._dataname_struct1.the_dataname = incoming_packet.the_message.data_value;
    //messageScope = dataName._dataname_struct2.scope;

    // here is where (instead of this old style scope) we need to run one of the
    // trie prefix functions - not sure which one

    char dot = DOT;
    unsigned char* ptr = (unsigned char*)strchr((char*)incoming_packet.data, dot);
    if ( ptr )
    {
        t = trie_lookup2(rd->top_context, ptr+1);
    }


    //if ( (nodeScope & messageScope) != messageScope )
    if ( !t )
    {
        return;
    }


    t = trie_add(rd->top_state, incoming_packet.data, STATE);
    int inserted;
    Interface* i = InsertInterfaceNode(&(rd->interfaceTree), _interface, &inserted)->i;
    i->lqi = cd.incoming_lqi;



    //setObtainGradient(incoming_packet.data, _interface, incoming_packet.path_value);
    //setObtainGradient(incoming_packet.data, _interface, incoming_packet.path_value+(unsigned short)cd.incoming_lqi);
    setObtainGradient(incoming_packet.data, _interface, incomingLinkCost(cd), incoming_packet.seqno);


    //void setObtainGradient(char* fullyqualifiedname, NEIGHBOUR_ADDR iName, int pCost)

    //if ( (n = FindStateNode(rd->top_state, incoming_packet.the_message.data_value)) )

    // AN IDEA!!!
    // this code will be to
    //set a time variable when first advert arrives at pub sink
    //wait briefly until assume all gradient path values are in
    //so definite best gradient is in
    /*
    if ( !t->s->reinforcement_initiation_requirement_checked )
    {

        RD( "using:   xyz.ab.cd" << endl );
        new_one(rd->top_state, (const char*)_data);

        trie* r = rd->top_state;
        while ( r = r->rec )
        {
            if ( r->s->bestGradientToDeliver && !r->s->deliveryInterfaces )
            {
                start_reinforce_interest((char*)_data, _if);
            }
            //RD( "Listing: " << r->s->full_name << endl );

        }


         find any PUB SINKS in the trie in this node
        t->s->reinforcement_initiation_requirement_checked = true;
        t->s->reinforcement_initiation_required = true; // maybe do a time variable instead of bool

    }
    */



    if ( t )
    {
        if ( t->s->bestGradientToObtainUpdated )
        {
            t->s->bestGradientToObtainUpdated = false;
            outgoing_packet.message_type = ADVERT;
            //outgoing_packet.length = strlen((char*)incoming_packet.data);
            outgoing_packet.data = incoming_packet.data;
            outgoing_packet.length = incoming_packet.length;
            //outgoing_packet.path_value = incoming_packet.path_value+nodeConstraint;
            //outgoing_packet.path_value = incoming_packet.path_value+(unsigned short)cd.incoming_lqi;
            outgoing_packet.path_value = outgoingLinkCost(cd);
            outgoing_packet.excepted_interface = _interface;
            bcastAMessage(write_packet(&outgoing_packet));
            //SendToAllInterfacesExcept(rd->interfaceTree, _interface);



// here send to self if the state is marked for action as source
// self would then create a gradient to self, deliver cost zero and reinforce it, then send
// a reinforcement message to the best obtain gradient

// HOWEVER
// we need to wait a moment in case we dont have the best obtain gradient

// may be we need a different tack
// may be we should not respond to the advert, but perhaps the application should
// occasionally reinforce for the interested items

// NOW I think we need a function that runs from time to time






        }




    }


}





void handle_interestTODELETETODELETE(control_data cd)
{
    RD( "Interest received from: " << cd.incoming_if << " lqi: " << cd.incoming_lqi << std::endl );

    NEIGHBOUR_ADDR _interface = cd.incoming_if;
    if ( incoming_packet.excepted_interface == thisAddress )
    {
        //return;
    }

    trie* t = trie_add(rd->top_state, incoming_packet.data, STATE);

    // CHECK THIS IS RIGHT - ITS TO DO WITH OLD SEQNO
    State* s = t->s;

    if ( incoming_packet.seqno < s->seqno )
    {
        return; // I think this is right, i.e. ignore any old seq ints/advs
        // CHECK THIS
    }

    if ( (incoming_packet.seqno == s->seqno) && s->converged )
    {
        return;
    }

    if ( incoming_packet.seqno > s->seqno )
    {
        /*
         * IF: this is a rec and we are the sink for it
         * i.e. we have a best grad already set that is this data rec
         * interface to self and cost zero (and most likely grad and state seqno zero).
         *
         * THEN:  we must leave the best grad to self in place, set its seqno to this
         * incoming seqno value +1 and the same for the state object.
         *
         * ACTUALLY we JUST want to send out the initiating interest again with the
         * 'brand new' seqno.  I.E. the best grad with IF: Self COST: Zero.
         *
         *
         */
        if ( s->bestGradientToDeliver &&
                s->bestGradientToDeliver->key2->iName == SELF_INTERFACE &&
                s->bestGradientToDeliver->costToDeliver == 0 &&
                s->bestGradientToDeliver->seqno == 0 &&
                s->seqno == 0) // pos improve condition
        {
            s->bestGradientToDeliver->seqno = incoming_packet.seqno+1;
            s->seqno = incoming_packet.seqno+1;
            //s->bestGradientToDeliverUpdated = 0;

            outgoing_packet.message_type = INTEREST;

            outgoing_packet.length = incoming_packet.length;
            outgoing_packet.data = incoming_packet.data;

            //outgoing_packet.length = strlen(queue); // strlen ok in this case
            //outgoing_packet.data = (unsigned char*)queue; // cannot get

            outgoing_packet.path_value = 0;
            //outgoing_packet.excepted_interface = s->bestGradientToDeliver->key2->iName;
            outgoing_packet.excepted_interface = UNKNOWN_INTERFACE; // not used now?
            outgoing_packet.down_interface = UNKNOWN_INTERFACE; // not used now?
            outgoing_packet.seqno = s->seqno;

            RD( "Resending initiating interest packet [new seqno]" << std::endl );
            bcastAMessage(write_packet(&outgoing_packet));
        }
    }

    int inserted;
    Interface* i = InsertInterfaceNode(&(rd->interfaceTree), _interface, &inserted)->i;
    i->lqi = cd.incoming_lqi;

    setDeliverGradient(incoming_packet.data, _interface, incomingLinkCost(cd), incoming_packet.seqno);

    // check this out!!
    if ( incoming_packet.down_interface == thisAddress )
        return;

    if ( t )
    {
        if ( t->s->bestGradientToDeliverUpdated )
        {
            // THIS MECHANISM FOR NOW BUT MAY NEED A SUFFIX FUNCTION

            // this is a once only on first sight for all the (interest) states
            // we have ever seen, to say whether or not it is a prefix of one
            // that was previously set as SOURCE_ACTION
            if ( t->s->prefix != FORWARD_AND_SOURCEPREFIX &&
                    t->s->prefix != PREFIX_CHECKED )
            {
                traverse(rd->top_state, queue, 0, setAllPrefixStatus);
                if ( t->s->prefix != FORWARD_AND_SOURCEPREFIX )
                {
                    t->s->prefix = PREFIX_CHECKED;
                }
            }

            // try doing timeout for ALL nodes
            t->s->converged = 0;
            setTimer(0.2, t->s, interest_convergence_timeout);

            t->s->bestGradientToDeliverUpdated = false;
            outgoing_packet.message_type = INTEREST;
            outgoing_packet.data = incoming_packet.data;
            outgoing_packet.length = incoming_packet.length;
            outgoing_packet.path_value = outgoingLinkCost(cd);
            outgoing_packet.excepted_interface = _interface;
            outgoing_packet.down_interface = UNKNOWN_INTERFACE;
            outgoing_packet.seqno = incoming_packet.seqno; // IS THIS RIGHT - WHAT IF CHANGED BY setDeliverGradient?
            bcastAMessage(write_packet(&outgoing_packet));
        }
    }

}






















void handle_collaboration(control_data cd)
{
    RD( "Collaboration received from: " << cd.incoming_if << " lqi: " << cd.incoming_lqi << std::endl );

    NEIGHBOUR_ADDR _interface = cd.incoming_if;

    trie* t = trie_add(rd->top_state, incoming_packet.data, STATE);

    // CHECK THIS IS RIGHT - ITS TO DO WITH OLD SEQNO
    State* s = t->s;

    if ( incoming_packet.seqno < s->seqno )
    {
        return; // I think this is right, i.e. ignore any old seq ints/advs
        // CHECK THIS
    }

    if ( (incoming_packet.seqno == s->seqno) && s->converged )
    {
        return;
    }


    if ( incoming_packet.seqno > s->seqno )
    {
        /*
         * IF: this is a rec and we are the sink for it
         * i.e. we have a best grad already set that is this data rec
         * interface to self and cost zero (and most likely grad and state seqno zero).
         *
         * THEN:  we must leave the best grad to self in place, set its seqno to this
         * incoming seqno value +1 and the same for the state object.
         *
         * ACTUALLY we JUST want to send out the initiating interest again with the
         * 'brand new' seqno.  I.E. the best grad with IF: Self COST: Zero.
         *
         *
         */
        if ( s->bestGradientToDeliver &&
                s->bestGradientToDeliver->key2->iName == SELF_INTERFACE &&
                s->bestGradientToDeliver->costToDeliver == 0 &&
                s->bestGradientToDeliver->seqno == 0 &&
                s->seqno == 0) // pos improve condition
        {
            s->bestGradientToDeliver->seqno = incoming_packet.seqno+1;
            s->seqno = incoming_packet.seqno+1;

            outgoing_packet.message_type = COLLABORATION;

            outgoing_packet.length = incoming_packet.length;
            outgoing_packet.data = incoming_packet.data;
            outgoing_packet.path_value = 0;
            outgoing_packet.excepted_interface = UNKNOWN_INTERFACE; // not used now?
            outgoing_packet.down_interface = UNKNOWN_INTERFACE; // not used now?
            outgoing_packet.seqno = s->seqno;

            RD( "Resending initiating interest packet [new seqno]" << std::endl );
            bcastAMessage(write_packet(&outgoing_packet));
        }
    }

    int inserted;
    Interface* i = InsertInterfaceNode(&(rd->interfaceTree), _interface, &inserted)->i;
    i->lqi = cd.incoming_lqi;
    setDeliverGradient(incoming_packet.data, _interface, incomingLinkCost(cd), incoming_packet.seqno);

    if ( t )
    {
        if ( t->s->bestGradientToDeliverUpdated )
        {
            // THIS MECHANISM FOR NOW BUT MAY NEED A SUFFIX FUNCTION

            // this is a once only on first sight for all the (interest) states
            // we have ever seen, to say whether or not it is a prefix of one
            // that was previously set as SOURCE_ACTION
            if ( t->s->prefix != FORWARD_AND_COLLABORATIONPREFIX &&
                    t->s->prefix != PREFIX_CHECKED )
            {
                traverse(rd->top_state, queue, 0, setAllPrefixStatus);
                if ( t->s->prefix != FORWARD_AND_COLLABORATIONPREFIX )
                {
                    t->s->prefix = PREFIX_CHECKED;
                }
            }

            // try doing timeout for ALL nodes
            t->s->converged = 0;
            setTimer(0.2, t->s, collaboration_convergence_timeout);

            t->s->bestGradientToDeliverUpdated = false;
            outgoing_packet.message_type = COLLABORATION;
            outgoing_packet.data = incoming_packet.data;
            outgoing_packet.length = incoming_packet.length;
            outgoing_packet.path_value = outgoingLinkCost(cd);
            outgoing_packet.excepted_interface = _interface;
            outgoing_packet.down_interface = UNKNOWN_INTERFACE;
            outgoing_packet.seqno = incoming_packet.seqno; // IS THIS RIGHT - WHAT IF CHANGED BY setDeliverGradient?
            bcastAMessage(write_packet(&outgoing_packet));
        }
    }









    ////////////////////////////////////////////



    //static rpacket p;
    //trie* t;
    //static struct KDGradientNode* k;
    //t = trie_add(rd->top_state, incoming_packet.data, STATE);
    //int inserted;
    //Interface* i = InsertInterfaceNode(&(rd->interfaceTree), _interface, &inserted)->i;
    //setDeliverGradient(incoming_packet.data, _interface, incoming_packet.path_value+(unsigned short)cd.incoming_lqi, incoming_packet.seqno);

    if ( t )
    {
        if ( t->s->bestGradientToDeliverUpdated )
        {
            t->s->bestGradientToDeliverUpdated = false;
            outgoing_packet.message_type = COLLABORATION;
            outgoing_packet.data = incoming_packet.data;
            outgoing_packet.length = incoming_packet.length;
            //outgoing_packet.path_value = incoming_packet.path_value+nodeConstraint;
            outgoing_packet.path_value = incoming_packet.path_value+(unsigned short)cd.incoming_lqi;
            outgoing_packet.excepted_interface = _interface;
            bcastAMessage(write_packet(&outgoing_packet));
            //SendToAllInterfacesExcept(rd->interfaceTree, _interface);
        }
    }

}








void handle_collaborationOLDOLDOLD(control_data cd)
{

    NEIGHBOUR_ADDR _interface = cd.incoming_if;

    //static rpacket p;
    trie* t;
    //static struct KDGradientNode* k;

    t = trie_add(rd->top_state, incoming_packet.data, STATE);
    int inserted;
    //Interface* i = InsertInterfaceNode(&(rd->interfaceTree), _interface, &inserted)->i;
    InsertInterfaceNode(&(rd->interfaceTree), _interface, &inserted)->i;
    //setDeliverGradient(incoming_packet.data, _interface, incoming_packet.path_value);
    setDeliverGradient(incoming_packet.data, _interface, incoming_packet.path_value+(unsigned short)cd.incoming_lqi, incoming_packet.seqno);

    if ( t )
    {
        if ( t->s->bestGradientToDeliverUpdated )
        {
            t->s->bestGradientToDeliverUpdated = false;
            outgoing_packet.message_type = COLLABORATION;
            outgoing_packet.data = incoming_packet.data;
            outgoing_packet.length = incoming_packet.length;
            //outgoing_packet.path_value = incoming_packet.path_value+nodeConstraint;
            outgoing_packet.path_value = incoming_packet.path_value+(unsigned short)cd.incoming_lqi;
            outgoing_packet.excepted_interface = _interface;
            bcastAMessage(write_packet(&outgoing_packet));
            //SendToAllInterfacesExcept(rd->interfaceTree, _interface);
        }
    }

}





/*

previously this call on every node at 2s intervals:

void regular_checks(void)
{
    traverse(rd->top_state, queue, 0, processState);
}

calling back to this for every state held for any reason:

void processState(State* s, unsigned char* _data, NEIGHBOUR_ADDR _if)
{
}

in there amongst other things, in particular we do this:

if ( s->action == SOURCE_ACTION )
{
    if ( ((*_data) & MSB2) == RECORD )
    {
            action_all_prefixes(rd->top_state, 0, strlen((const char*)_data), _data,
                   current_prefix_name, _if, consider_reinforce_interest);
#ifdef GRAD_FILES
            UpdateGradientFile();
#endif
    }
}




Now!  Can we really do the same with this new mechanism
Here's a possibility where it may go wrong and essentially
always might have done.


say interest for X is passed out at 1.0s
and interest for Y is passed out at 1.5s

say this node in question is source for both.
When (because the node has had no X update for 0.1s) the
call back comes in, the traversal will find Y as well
and if it has had at least one new interest it will
commence reinforcement even though it may be due
more...


So does the call back need to be specific to a state?
Or can we marked states in some way to see which ones
are ready to reinforce?












*/















void removePktFromQueue(struct PacketQueue** l, struct new_packet* _i)
{
    if(*l==NULL)
    {
        return;
    }

    struct PacketQueue *old,*temp;
    temp=*l;
    while( temp != NULL )
    {
        if(temp->i  == _i)
        {
            if( temp == *l )
            {
                *l=temp->link;
            }
            else
            {
                old->link=temp->link;
            }
            free(temp->i->data);
            free(temp->i);
            free(temp);
            temp = 0;
        }
        else
        {
            old=temp;
            temp=temp->link;
        }
    }
}



bool in_queued_data(State* s, new_packet* pktToCheck)
{

    PacketQueue** l = &(s->pktQ);
    PacketQueue* temp = *l;
    while( temp !=NULL )
    {
        if ( temp->i == pktToCheck )
        {
            return true;
        }
        else
        {
            l = &(temp->link);
            temp = temp->link;
        }
    }

    return false;
}




void delete_queued_data(void* relevantObject)
{
    QueueDeletion* qd = (QueueDeletion*)relevantObject;

    PacketQueue** l = &(qd->associatedState->pktQ);
    PacketQueue* temp = *l;
    while( temp !=NULL )
    {
        if ( temp->i == qd->pktToDelete )
        {
            *l = temp->link;
            free(temp->i->data);
            free(temp->i);
            free(temp);
            temp = *l; // don't break, iterate entire list just in case duplicates
        }
        else
        {
            l = &(temp->link);
            temp = temp->link;
        }
    }

    free(qd);

    return;
}





// use this when a reinforcement has just occurred
void send_queued_data(void* relevantObject)
{
    //new_packet* pkt = (new_packet*)relevantObject;

    State* s = (State*)relevantObject;

    PacketQueue** l = &(s->pktQ);
    PacketQueue* temp = *l;
    while( temp !=NULL )
    {
        sending_packet = temp->i;
        static unsigned char data[20];
        GetStateStr(rd->top_state, s, data);
        if ( consider_sending_data(s, data, 0) )
        {
            *l = temp->link;
            free(temp->i->data);
            free(temp->i);
            free(temp);
            temp = *l;
        }
        else
        {
            l = &(temp->link);
            temp = temp->link;
        }
    }

    return;



}


























void handle_interest(control_data cd)
{
    RD( "Interest received from: " << cd.incoming_if << " lqi: " << cd.incoming_lqi << std::endl );

    NEIGHBOUR_ADDR _interface = cd.incoming_if;
    if ( incoming_packet.excepted_interface == thisAddress )
    {
        //return;
    }

    trie* t = trie_add(rd->top_state, incoming_packet.data, STATE);

    // CHECK THIS IS RIGHT - ITS TO DO WITH OLD SEQNO
    State* s = t->s;

    if ( incoming_packet.seqno < s->seqno )
    {
        return; // I think this is right, i.e. ignore any old seq ints/advs
        // CHECK THIS
    }

    if ( (incoming_packet.seqno == s->seqno) && s->converged )
    {
        return;
    }

    if ( incoming_packet.seqno > s->seqno )
    {
        /*
         * IF: this is a rec and we are the sink for it
         * i.e. we have a best grad already set that is this data rec
         * interface to self and cost zero (and most likely grad and state seqno zero).
         *
         * THEN:  we must leave the best grad to self in place, set its seqno to this
         * incoming seqno value +1 and the same for the state object.
         *
         * ACTUALLY we JUST want to send out the initiating interest again with the
         * 'brand new' seqno.  I.E. the best grad with IF: Self COST: Zero.
         *
         *
         */
        if ( s->bestGradientToDeliver &&
                s->bestGradientToDeliver->key2->iName == SELF_INTERFACE &&
                s->bestGradientToDeliver->costToDeliver == 0 &&
                s->bestGradientToDeliver->seqno == 0 &&
                s->seqno == 0) // pos improve condition
        {
            s->bestGradientToDeliver->seqno = incoming_packet.seqno+1;
            s->seqno = incoming_packet.seqno+1;
            //s->bestGradientToDeliverUpdated = 0;

            outgoing_packet.message_type = INTEREST;

            outgoing_packet.length = incoming_packet.length;
            outgoing_packet.data = incoming_packet.data;

            //outgoing_packet.length = strlen(queue); // strlen ok in this case
            //outgoing_packet.data = (unsigned char*)queue; // cannot get

            outgoing_packet.path_value = 0;
            //outgoing_packet.excepted_interface = s->bestGradientToDeliver->key2->iName;
            outgoing_packet.excepted_interface = UNKNOWN_INTERFACE; // not used now?
            outgoing_packet.down_interface = UNKNOWN_INTERFACE; // not used now?
            outgoing_packet.seqno = s->seqno;

            RD( "Resending initiating interest packet [new seqno]" << std::endl );
            bcastAMessage(write_packet(&outgoing_packet));
        }
    }















	//static rpacket p;
	//static struct StateNode* n;
	//trie* t;
	//static struct KDGradientNode* k;

	//struct KDGradientNode* setDeliverGradient(int sName, NEIGHBOUR_ADDR iName, int pCost);


	/*
	 * Comment this code out for the moment
	 * because we think we may want to query for narrow context but at a distance from some some other context
	 */
	//char* ptr = strchr((const char*)incoming_packet.data, DOT);
	//if ( ptr )
	//{
	//	t = trie_lookup2(rd->top_context, ptr+1);
	//}
	//if ( !t )
	//{
	//	return;
	//}



	int inserted;
    Interface* i = InsertInterfaceNode(&(rd->interfaceTree), _interface, &inserted)->i;
    //Interface* i = InsertInterfaceNode(&(rd->interfaceTree), _interface)->i;
    i->lqi = cd.incoming_lqi;




	//t = trie_add(rd->top_state, incoming_packet.data, STATE);

    // CHECK THIS IS RIGHT - ITS TO DO WITH OLD SEQNO
	//State* s = t->s;

	//if ( s->converged )
	//{

	//}



    //setDeliverGradient(incoming_packet.data, _interface, incoming_packet.path_value);
    setDeliverGradient(incoming_packet.data, _interface, incomingLinkCost(cd), incoming_packet.seqno);

    // check this out!!
    if ( incoming_packet.down_interface == thisAddress )
        return;

	//if ( (n = FindStateNode(rd->stateTree, incoming_packet.the_message.data_value)) )
	if ( t )
	{
		if ( t->s->bestGradientToDeliverUpdated )
		{
		    // THIS MECHANISM FOR NOW BUT MAY NEED A SUFFIX FUNCTION

		    // this is a once only on first sight for all the (interest) states
		    // we have ever seen, to say whether or not it is a prefix of one
		    // that was previously set as SOURCE_ACTION
            if ( t->s->prefix != FORWARD_AND_SOURCEPREFIX &&
                    t->s->prefix != PREFIX_CHECKED )
            {
                traverse(rd->top_state, queue, 0, setAllPrefixStatus);
                if ( t->s->prefix != FORWARD_AND_SOURCEPREFIX )
                {
                    t->s->prefix = PREFIX_CHECKED;
                }
            }

            // try doing timeout for ALL nodes
            t->s->converged = 0;
            setTimer(0.2, t->s, interest_convergence_timeout);

            //if ( t->s->prefix == FORWARD_AND_SOURCEPREFIX )
            //{
                //char* x = (char*)malloc(strlen((const char*)incoming_packet.data)+1);
                //strcpy(x, (const char*)incoming_packet.data);
                //setTimer(0.1, x, interest_convergence_timeout);

                //setTimer(0.1, t->s, source_interest_convergence_timeout);

                // Alternative longer method?
                //unsigned char* x = (unsigned char*)malloc(incoming_packet.length+1);

                // CHECK MEMCPY IS MEMORY SOUND IF UNCOMMENTINBELOW
                //memcpy(x, incoming_packet.data, incoming_packet.length);
                //x[incoming_packet.length] = 0;
            //}
            //else
            //{
                //setTimer(0.1, t->s, interest_convergence_timeout);
            //}





			t->s->bestGradientToDeliverUpdated = false;
			outgoing_packet.message_type = INTEREST;
            outgoing_packet.data = incoming_packet.data;
            outgoing_packet.length = incoming_packet.length;
            //outgoing_packet.path_value = incoming_packet.path_value+nodeConstraint;
            outgoing_packet.path_value = outgoingLinkCost(cd);
            outgoing_packet.excepted_interface = _interface;
            outgoing_packet.down_interface = UNKNOWN_INTERFACE;
            outgoing_packet.seqno = incoming_packet.seqno; // IS THIS RIGHT - WHAT IF CHANGED BY setDeliverGradient?
            bcastAMessage(write_packet(&outgoing_packet));
			//SendToAllInterfacesExcept(rd->interfaceTree, _interface);
		}
	}

}




// probably not thread safe
void handle_reinforce(control_data cd)
{
    NEIGHBOUR_ADDR _interface = cd.incoming_if;
    RD( "ADV Reinforce received from " << std::hex << _interface << std::endl );

    // reinforce the the preceding interface in the direction of sink
	// CHANGE_
	//reinforceDeliverGradient(incoming_packet.data    .the_message.data_value, _interface);
	reinforceDeliverGradient(incoming_packet.data, _interface, incoming_packet.seqno);


	//StateNode* sn = FindStateNode(rd->stateTree, incoming_packet.the_message.data_value);

	// not sure whether we want this or an add to obtain the trie node
	// if we send a reinforcement that is the same as the data we are going to send
	// then this trie_lookup_longest_prefix_extra2 is ok
	//
	// if we do this longest prefix match before we send and then send the same as
	// the gradient then a straight match (add method?) will probably be ok
	//
	//trie* t = trie_lookup_longest_prefix_extra2(rd->top_state, outgoing_packet.data);
	trie* t = trie_add(rd->top_state, incoming_packet.data, STATE);

    setTimer(0.01, t->s, send_queued_data);


    // a reinforced obtain gradient to self indicates this is the source, so stop
    // and ultimately allow this source to send data
    //KDGradientNode* gn;
	//if ( (gn = SearchForKDGradientNode(incoming_packet.the_message.data_value, SELF_INTERFACE, rd->grTree)) )
			//&& gn->key1->obtainInterface == gn->key2 )
	//{
	//	return;
	//}

	if ( t && t->s->bestGradientToObtain )
	{
		//if ( g->key1->bestGradientToObtain->obtainReinforcement ) // NEED TO CHECK LIST NOW


#ifndef XXXXXX // if XXXXXX send reinforcement right through
	    // next hop already reinforced so stop here
	    if ( t->s->obtainInterfaces )//obtainReinforcement ) // NEED TO CHECK LIST NOW
			return;
#endif

		// find the next interface to reinforce
		NEIGHBOUR_ADDR interface = t->s->bestGradientToObtain->key2->iName;

        // also reinforce the path to the source for breakage messages
		// even if it is self
		reinforceObtainGradient(incoming_packet.data, interface, incoming_packet.seqno);

		// If interface is self do not send message on
		if ( interface == SELF_INTERFACE )
#ifndef XXXXXX
			return;
#else
		{
	        setTimer(CANCEL_TIMER, t->s, initiate_new_advert);
            return;
		}
#endif

		outgoing_packet.message_type = REINFORCE;
        outgoing_packet.data = incoming_packet.data;
        outgoing_packet.length = incoming_packet.length;
		outgoing_packet.path_value = 0;
        outgoing_packet.down_interface = incoming_packet.down_interface;
        outgoing_packet.excepted_interface = incoming_packet.excepted_interface;
        RD( "Forwarding ADV Reinforce to " << std::hex << interface << std::endl );
		sendAMessage(interface, write_packet(&outgoing_packet), 0);

	}

}




void handle_reinforce_interest(control_data cd)
{
    NEIGHBOUR_ADDR _interface = cd.incoming_if;

	reinforceObtainGradient(incoming_packet.data, _interface, incoming_packet.seqno);

	trie* t = trie_add(rd->top_state, incoming_packet.data, STATE);


	if ( t && t->s->bestGradientToDeliver )
	{
        KDGradientNode* selectedGradientToDeliver = t->s->bestGradientToDeliver;

        //TraversInterfaceNodes(rd->interfaceTree, 0, MarkIFDown);
        // above is re downinterface, no longer used?

        // this section is to do with having multiple
        // reinforced best interest grads created from
        // using 2nd best grads when best if is down
        //
        // however, think this is no longer used!
        //InterfaceList* temp = t->s->deliveryInterfaces;
        //while( temp !=NULL )
        //{
        //    if ( temp->i->up )
        //        return;
        //    temp = temp->link;
        //}
        // in normal practice t->s->deliveryInterfaces will be null
        // just before reinforcement

        if ( !selectedGradientToDeliver->key2->up )
        {
            //KDGradientNode* saveBestGradientToDeliver = t->s->bestGradientToDeliver;
            //t->s->bestGradientToDeliver = 0;
            //TraversInterfaceNodes(rd->interfaceTree, t->s, UpdateBestGradient);
            //selectedGradientToDeliver = t->s->bestGradientToDeliver;
            //t->s->bestGradientToDeliver = saveBestGradientToDeliver;
            // this I think is also not used any more
            // it's where we update to the 2nd best grad if one is down
        }


        if ( selectedGradientToDeliver )
        {
            NEIGHBOUR_ADDR interface = selectedGradientToDeliver->key2->iName;
            //reinforceDeliverGradient((char*)incoming_packet.data, interface); //???
            add(&(t->s->deliveryInterfaces), selectedGradientToDeliver->key2); //maybe

            setTimer(0.01, t->s, send_queued_data);

            if ( interface == SELF_INTERFACE )
                return;

            outgoing_packet.message_type = REINFORCE_INTEREST;
            outgoing_packet.data = incoming_packet.data;
            outgoing_packet.length = incoming_packet.length;
            outgoing_packet.path_value = 0;
            outgoing_packet.down_interface = incoming_packet.down_interface;
            outgoing_packet.excepted_interface = incoming_packet.excepted_interface;
            outgoing_packet.seqno = incoming_packet.seqno;
            sendAMessage(interface, write_packet(&outgoing_packet), 0);
        }
	}




}





void handle_reinforce_collaboration(control_data cd)
{
    NEIGHBOUR_ADDR _interface = cd.incoming_if;

    // reinforce obtain back to collaboration reinforcement originator
    // because it is the sender
    reinforceObtainGradient(incoming_packet.data, _interface, incoming_packet.seqno);

    trie* t = trie_add(rd->top_state, incoming_packet.data, STATE);

    if ( t && t->s->bestGradientToDeliver )
    {
        // if we have reached a node which has already had forward reinforcement done
        if ( t->s->deliveryInterfaces )
        {
            // just reinforce also DELIVER back to collaboration reinforcement originator
            // because it wants to receive as well, then finish
            reinforceDeliverGradient(incoming_packet.data, _interface, incoming_packet.seqno);
            setTimer(0.01, t->s, send_queued_data);
            return;
        }

        // reinforce also DELIVER back to collaboration reinforcement originator
        // because it wants to receive as well
        reinforceDeliverGradient(incoming_packet.data, _interface, incoming_packet.seqno);

        // now find the forward interface and reinforce that for delivery
        NEIGHBOUR_ADDR interface = t->s->bestGradientToDeliver->key2->iName;
        reinforceDeliverGradient(incoming_packet.data, interface, incoming_packet.seqno);
        setTimer(0.01, t->s, send_queued_data);

        // If interface is self do not send message on
        if ( interface == SELF_INTERFACE )
        {
            return;
        }

        outgoing_packet.message_type = REINFORCE_COLLABORATION;
        outgoing_packet.data = incoming_packet.data;
        outgoing_packet.length = incoming_packet.length;
        outgoing_packet.path_value = 0;
        outgoing_packet.down_interface = incoming_packet.down_interface;
        outgoing_packet.excepted_interface = incoming_packet.excepted_interface;
        sendAMessage(interface, write_packet(&outgoing_packet), 0);
    }

}





/*
	if ( s->action == SINK_ACTION )
	{
		//if ( s->dataName._dataname_struct2.type & PUBLICATION )
		if ( ((*_data) & MSB2) == PUBLICATION )
		{
			if ( s->bestGradientToObtain && !s->obtainInterfaces )
			{

(char* fullyqualifiedname, NEIGHBOUR_ADDR iName)



 */









/*
 * Should these start_reinforce... functions be incorporated into the handle ones
 * only treat as arrived from SELF?
 */
// probably not thread safe
void start_reinforce_interest(unsigned char* fullyqualifiedname, NEIGHBOUR_ADDR _if, char seqno)
{
    if ( _if != UNKNOWN_INTERFACE )
    {
        reinforceObtainGradient(fullyqualifiedname, _if, seqno);
    }
	//StateNode* sn = FindStateNode(rd->stateTree, specific_data_value._dataname_struct1.the_dataname);
	trie* t = trie_add(rd->top_state, fullyqualifiedname, STATE);

	if ( t && t->s->bestGradientToDeliver )
	{
		// next hop already reinforced so stop here
		if ( t->s->deliveryInterfaces )
			return;

		// find the next interface to reinforce
		NEIGHBOUR_ADDR interface = t->s->bestGradientToDeliver->key2->iName;

        // also reinforce the path to the source for breakage messages
		// even if it is self
		reinforceDeliverGradient(fullyqualifiedname, interface, seqno);
        setTimer(0.01, t->s, send_queued_data);

		// If interface is self do not send message on
		if ( interface == SELF_INTERFACE )
			return;

		outgoing_packet.message_type = REINFORCE_INTEREST;
		outgoing_packet.data = (unsigned char*)fullyqualifiedname;
        outgoing_packet.length = strlen((const char*)outgoing_packet.data);
        outgoing_packet.down_interface = UNKNOWN_INTERFACE;
        outgoing_packet.excepted_interface = UNKNOWN_INTERFACE;
		outgoing_packet.path_value = 0;
		outgoing_packet.seqno = seqno;
		sendAMessage(interface, write_packet(&outgoing_packet), 0);

	}

}




/*
 * Should these start_reinforce... functions be incorporated into the handle ones
 * only treat as arrived from SELF?
 */
void start_reinforce_collaboration(unsigned char* fullyqualifiedname, NEIGHBOUR_ADDR _if, char seqno)
{
    // ??????????????
    if ( _if != UNKNOWN_INTERFACE )
    {
        reinforceObtainGradient(fullyqualifiedname, _if, seqno);
    }

    // think 'obtain' grad to self is ok for collaborators
    // but allow calling code to choose - see above
    //reinforceObtainGradient(fullyqualifiedname, SELF_INTERFACE, seqno);

    //StateNode* sn = FindStateNode(rd->stateTree, specific_data_value._dataname_struct1.the_dataname);
    trie* t = trie_add(rd->top_state, fullyqualifiedname, STATE);

    if ( t && t->s->bestGradientToDeliver )
    {
        if ( t->s->deliveryInterfaces )
        {
            // next hop already reinforced so stop here
            // I think the deliver gradient to self should probably only
            // be on the collaborator initiator
            // otherwise may cause issues because new start ups will get
            // passed a grad showing best cost to several different places

            //reinforceDeliverGradient(fullyqualifiedname, SELF_INTERFACE, seqno);
            return;
        }

        // find the next interface to reinforce
        NEIGHBOUR_ADDR interface = t->s->bestGradientToDeliver->key2->iName;

        // also reinforce the path to the source for breakage messages
        // even if it is self

        // Think not to do this see above comment
        // data will get delivered by virtue of the state action type
        //reinforceDeliverGradient(fullyqualifiedname, SELF_INTERFACE, seqno);

        reinforceDeliverGradient(fullyqualifiedname, interface, seqno);


        //?????????
        setTimer(0.01, t->s, send_queued_data);



        // If interface is self do not send message on
        // NEED to think a bit more about this iro collaberation
        if ( interface == SELF_INTERFACE )
            return;

        outgoing_packet.message_type = REINFORCE_COLLABORATION;
        outgoing_packet.data = (unsigned char*)fullyqualifiedname;
        outgoing_packet.length = strlen((const char*)outgoing_packet.data);
        outgoing_packet.down_interface = UNKNOWN_INTERFACE;
        outgoing_packet.excepted_interface = UNKNOWN_INTERFACE;
        outgoing_packet.path_value = 0;
        sendAMessage(interface, write_packet(&outgoing_packet), 0);

    }

}









// probably not thread safe
void handle_data(control_data cd)
{
    NEIGHBOUR_ADDR _interface = cd.incoming_if;
    excludedInterface = _interface;

	//DATANAME = incoming_packet.the_data_message.data_value;
	//IN1      = incoming_packet.the_data_message.in1;
	//incoming_packet.the_data_message.data_value._dataname_struct2.type = 0;

	//OUT1     = incoming_packet.the_data_message.data_value;

	// set message arrived -
	// once a check by a role has been made that this particular message has arrived
	// message arrived flag is turned off
	rd->flags ^= MESSAGE_ARRIVED;


	kickFSM();


}















void handle_neighbor_bcast(control_data cd)
{
    NEIGHBOUR_ADDR _interface = cd.incoming_if;
    //int inserted;
    //InterfaceNode* in = InsertInterfaceNode(&(rd->interfaceTree), _interface, &inserted);
    InterfaceNode* in = FindInterfaceNode(rd->interfaceTree, _interface);

    outgoing_packet.down_interface = UNKNOWN_INTERFACE;
    //if ( !inserted )
    if ( in )
    {
        // we have seen this node before (already in list )
        // it has just come back up
        outgoing_packet.down_interface = _interface; // no longer used?
        in->i->up = 1;
        printf("Existing interface back up\n");
    }
    else
    {
        //printf("Inserted interface\n");
    }


	//outgoing_packet.message_type = NEIGHBOR_UCAST;
	//outgoing_packet.length = 0;
	//outgoing_packet.data = 0;
	//outgoing_packet.path_value = 0;
	//outgoing_packet.excepted_interface = UNKNOWN_INTERFACE;
	//outgoing_packet.down_interface = UNKNOWN_INTERFACE;
	//bcastAMessage(write_packet());
	//printf("using NEIGHBOR_UCAST\n");

    //int advertsFound = UcastAllBestGradients(rd->top_state, _interface);
    UcastAllBestGradients(rd->top_state, _interface);
	//printf("Forwarded our adverts\n");

}


void handle_neighbor_ucast(control_data cd)
{
    NEIGHBOUR_ADDR _interface = cd.incoming_if;
    int inserted;
	InsertInterfaceNode(&(rd->interfaceTree), _interface, &inserted);
	//printf("Inserted interface\n");
	//DBRETAFTERUCAST(return;)

    printf("About to forward our adverts\n");
    //int advertsFound = UcastAllBestGradients(rd->top_state, _interface);
    UcastAllBestGradients(rd->top_state, _interface);
}




void StartUp()
{
    advertRetries = 0;  // needs improving
    advertReinforceRetries = 0;  // needs improving

	// not sure if this is necessary for ansii c
	memset(&incoming_packet, 0, sizeof(incoming_packet));
	memset(&outgoing_packet, 0, sizeof(outgoing_packet));

    //int advertsFound = UcastAllBestGradients(rd->top_state, 0);
    UcastAllBestGradients(rd->top_state, 0);

	outgoing_packet.message_type = NEIGHBOR_BCAST;
	outgoing_packet.data = 0;
    outgoing_packet.length = 0;
    outgoing_packet.down_interface = UNKNOWN_INTERFACE;
    outgoing_packet.excepted_interface = UNKNOWN_INTERFACE;
	outgoing_packet.path_value = 0;
	bcastAMessage(write_packet(&outgoing_packet));

}


/*
void self_handle_advert(struct State* advert)
{
if ( advert->action == ACTION_SINK )
{

// NEED TO BE CAREFUL  -  outgoing_packet already in use?

    KDGradientNode* g = reinforceDeliverGradient(theadvert, SELF_INTERFACE);


	if ( g->key1->bestGradientToObtain )
	{
		// already reinforced so stop here
		if ( g->key1->bestGradientToObtain->obtainReinforcement )
			return;

		// otherwise prepare and forward the reinforcement on this next entry interface
		NEIGHBOUR_ADDR interface = g->key1->bestGradientToObtain->key2->iName;
        //int instance = 0;
        reinforceObtainGradient(incoming_packet.the_message.data_value, interface);
		//TicTocMsg20* newmsg = MessageHelper::newMessage(REINFORCE, sm.data, instance, 0);

		outgoing_packet.the_message.message_type = REINFORCE;
		outgoing_packet.the_message.data_value = incoming_packet.the_message.data_value;
		outgoing_packet.the_message.path_value = 0;
		sendAMessage(interface, outgoing_packet.packet_bytes);

	}

}


//if so:



}
*/

void self_message(void * msg)
{


//are we interested in this advert?

//if so:



}




/*
char current_prefix_name[100];
void action_all_prefixes(trie *t, int i, int n, const char *str, char* buf, NEIGHBOUR_ADDR _if, void process(State* s, char* _buf, NEIGHBOUR_ADDR _if))
{
      trie *current = t;

      if ( i >= n )
      {
           if ( current->s )
           {
               //RD( "\t- Found " );
               //RD( current->s->full_name << endl );
               //void f4(State* s, char* _buf, NEIGHBOUR_ADDR _if)
               *(buf) = '\0';
               process(current->s, current_prefix_name, _if);
           }
           return;
      }

      const char c = str[i];
      t = t->first_child;
      t = trie_at_level(t,c);
      if(t == NULL)
      {
           for ( i++; i < n && str[i] != DOT; i++ );
           //if ( str[i] == '.' ) RD( "\t- Moving to next section " << endl );
           i--;
           t = current;
           action_all_prefixes(t, i+1, n, str, buf, _if, process);
      }
      else
      {
          *(buf) = c;
          //  *(buf+1) = '\0';

          action_all_prefixes(t, i+1, n, str, buf+1, _if, process);
          for ( i++; i < n && str[i] != DOT; i++ );
          //if ( str[i] == '.' ) RD( "\t- Moving to next section " << endl );
          i--;
          t = current;
          action_all_prefixes(t, i+1, n, str, buf, _if, process);
      }

      return;

}
*/







/*
	if ( s->action == SINK_ACTION )
	{
		//if ( s->dataName._dataname_struct2.type & PUBLICATION )
		if ( ((*_data) & MSB2) == PUBLICATION )
		{
			if ( s->bestGradientToObtain && !s->obtainInterfaces )
			{
 */



/*
 * the application layer needs to pass the full text of the key
 * that the application has an interest in or a publication for
 *
 * Also the full text of they key is sent when int, adv, reinf or data
 * messages are set
 *
 * So...
 * what if when we say we are sink for
 *
 */



/*
 * This is to send reinforcements along prefixes
 * if the prefix has a best deliver gradient but no selected (reinforced) delivery interface
 */
int consider_reinforce_interest(State* s, unsigned char* _buf, NEIGHBOUR_ADDR _if)
{
    if ( s->bestGradientToDeliver && !s->deliveryInterfaces )
    {
        // second parameter no longer used, pos pass s in future to save
        // unnecessary work in the function
        start_reinforce_interest(_buf, SELF_INTERFACE, s->seqno);
    }

    return 0;

}



/*
 * This is to send reinforcements along prefixes
 * if the prefix has a best deliver gradient but no selected (reinforced) delivery interface
 */
int consider_reinforce_collaberation(State* s, unsigned char* _buf, NEIGHBOUR_ADDR _if)
{
    if ( s->bestGradientToDeliver && !s->deliveryInterfaces )
    {
        // second parameter no longer used, pos pass s in future to save
        // unnecessary work in the function
        start_reinforce_collaboration(_buf, 0, s->seqno);
    }

    return 0;

}






/*
 * This is to send data along reinforced prefixes
 */
/*
void consider_sending_data(State* s, char* _buf, NEIGHBOUR_ADDR _if)
{
    // but it's the query SOURCE data we need to send not the

    if ( s->deliveryInterfaces )
    {
        //dataRate = 0;
        // possibly not thread safe
        //incoming_packet.the_message.data_value = s->dataName._dataname_struct1.the_dataname;
        //incoming_packet.the_data_message.data_value._dataname_struct1.the_dataname = s->dataName._dataname_struct1.the_dataname;

        //incoming_packet.message_type = DATA;
        //incoming_packet.data = _buf;
        //incoming_packet.length = strlen((char*)_buf);
        //incoming_packet.path_value = 0;
        handle_data(SELF_INTERFACE);
    }

}
*/




// not used any more
void processState(State* s, unsigned char* _data, NEIGHBOUR_ADDR _if)
{
	//int temp = dataRate;

    if ( ((*_data) & MSB2) == PUBLICATION )
    {
        if ( s->bestGradientToObtain && !s->obtainInterfaces )
        {
            RD( "FOUND UNREINFORCED ADVERT" << std::endl );
            numSinks = 0;
            action_all_prefixes(rd->top_state, 0, strlen((const char*)_data), _data,
                    current_prefix_name, _if, count_sink);
            if ( numSinks )
            {
                RD( "TRY REINFORCING ADVERT" << std::endl );
                // second parameter no longer used, pos pass s in future to save
                // unnecessary work in the function
                start_reinforce(_data, 0, s->seqno);
            }
        }

    }

	//if ( s->action == SINK_ACTION )
	//{
		//if ( s->dataName._dataname_struct2.type & PUBLICATION )
		//if ( ((*_data) & MSB2) == PUBLICATION )
		//{
		    /*
		     * In the long term we could set the need for reinforcement in the fowarding state
		     * when it arrives at each node and give it a time then this iteration would simply
		     * check for required reinforcements that are old enough to know that there is less
		     * likelihood that the best gradient will change
		     *
		     * Here for the moment we need to check not the key we are at but
		     * all prefix keys (which may include this one) if there prefix matching
		     * keys which have best obtain gradient and no obtain interfaces then
		     * we need to reinforce each of these
		     */


            //action_all_prefixes(rd->top_state, 0, strlen((const char*)_data), (const char*)_data,
            //        current_prefix_name, _if, consider_reinforce_interest);



			//if ( s->bestGradientToObtain && !s->obtainInterfaces )
			//{
				// received at least one advertisement
				// but consider some try next regular check mechanism
				// to enable the arrival of further advertisements

	// TODO
				// This code is wrong because handle_reinforce is dependent on
				// the inpacket which may be set to anything

				// can we set it and use it?
				// can we guarantee this wont't screw up something else?


				//incoming_packet.the_message.data_value = s->dataName;
				//handle_reinforce(SELF_INTERFACE);
				//start_reinforce((char*)_data, _if);
			//}
		//}
	//}


    if ( s->action == COLLABORATE_ACTION )
    {
        if ( ((*_data) & MSB2) == COLLABORATIONBASED )
        {
            /*
             * This function actions every stored full data name prefix for a given full data name
             * the action is to consider collaborative reinforcement of each best deliver gradient
             */
            action_all_prefixes(rd->top_state, 0, strlen((const char*)_data), _data,
                    current_prefix_name, _if, consider_reinforce_collaberation);
        }


        // SOME WHERE ROUND HERE NEED TO ALSO SEND DATA??  OR SEE SOURCE BIT??
        incoming_packet.message_type = DATA;
        incoming_packet.length = strlen((char*)_data);

        free(incoming_packet.data);
        incoming_packet.data = (unsigned char*)malloc(incoming_packet.length+1);
        //strcpy(incoming_packet.data, _data);
        memcpy(incoming_packet.data, _data, incoming_packet.length);
        incoming_packet.data[incoming_packet.length] = 0;

        incoming_packet.path_value = 0;
        control_data cd;
        cd.incoming_if = SELF_INTERFACE;
        cd.incoming_lqi = 0;
        handle_data(cd);

    }


	if ( s->action == SOURCE_ACTION )
	{
		//if ( !(s->dataName._dataname_struct2.type & PUBLICATION) ) // RECORD
		if ( ((*_data) & MSB2) == RECORD )
		{
            /*
             * In the long term we could set the need for reinforcement in the forwarding state
             * when it arrives at each node and give it a time then this iteration would simply
             * check for required reinforcements that are old enough to know that there is less
             * likelihood that the best gradient will change
             *
             * Here for the moment we need to check not the key we are at but
             * all prefix keys (which may include this one) if there prefix matching
             * keys which have best deliver gradient and no deliver interfaces then
             * we need to reinforce each of these
             */

		    /*
		     * This function actions every stored full data name prefix for a given full data name
		     * the action is to consider reinforcing each best deliver gradient
		     */
		        // TEMP REMOVAL
		        action_all_prefixes(rd->top_state, 0, strlen((const char*)_data), _data,
		               current_prefix_name, _if, consider_reinforce_interest);
#ifdef GRAD_FILES
		        UpdateGradientFile();
#endif


	             //incoming_packet.message_type = DATA;
	               // incoming_packet.data = _data;
	                //incoming_packet.length = strlen((char*)_data);
	                //incoming_packet.path_value = 0;
	               //action_all_prefixes(rd->top_state, 0, strlen((const char*)_data), (const char*)_data,
	               //        current_prefix_name, _if, consider_sending_data);


		       //if ( s->bestGradientToDeliver && !s->deliveryInterfaces )
			//{
			//	start_reinforce_interest((char*)_data, _if);
			//}
		}


        // COMMENT OUT
		// THIS INITIATED IN THE APP NOW
		//incoming_packet.message_type = DATA;
        //incoming_packet.length = strlen((char*)_data);
        //free(incoming_packet.data);
        //incoming_packet.data = (unsigned char*)malloc(incoming_packet.length+1);

        // CHECK MEMCPY IS MEMORY SOUND IF UNCOMMENTINBELOW
		//memcpy(incoming_packet.data, _data, incoming_packet.length);
        //incoming_packet.data[incoming_packet.length] = 0;
        //incoming_packet.path_value = 0;
        //handle_data(SELF_INTERFACE);



		// some of this data stuff probably ought
		// to be inside the application layer
		// are we incrementing this in the right place?
		// does it matter?
		//dataRate++;
        //new_one(rd->top_state, (const char*)_data, _if);

        //if ( s->deliveryInterfaces )//&& dataRate > 150)
		//{
			//dataRate = 0;
			// possibly not thread safe
			//incoming_packet.the_message.data_value = s->dataName._dataname_struct1.the_dataname;
			//incoming_packet.the_data_message.data_value._dataname_struct1.the_dataname = s->dataName._dataname_struct1.the_dataname;

		    //incoming_packet.message_type = DATA;
            //incoming_packet.data = _data;
            //incoming_packet.length = strlen((char*)_data);
            //incoming_packet.path_value = 0;
			//handle_data(SELF_INTERFACE);
		//}
	}

}

void regular_checks(void)
{
	/*
	 * This traversal of the state nodes (now hierarchical in a trie) may present
	 * some problems now it is in a trie.
	 * I have thought over this and it is only used for taking action on those states for
	 * which we are sink or source and I wonder whether we ought not to just record these
	 * some where so we can simply refer to them directly rather than iterating through them all
	 *
	 * This would make it easier to access them
	 *
	 */
	//TraversStateNodes(rd->stateTree, processState);
	//traverse2(rd->top_state, processState);
    //dataRate++;
    //if ( dataRate > 25)
    //{
    //    dataRate = 0;
        traverse(rd->top_state, queue, 0, processState);
    //}

	//void traverse2(trie *t, void process_state(struct state* s))




}

//state* state_new();
//context* context_new();
//trie* trie_new();


state* state_new()
{
   state *s = (state*)malloc(sizeof(state));
   s->full_name[0] = '\0';
   s->eg2 = 2;
   s->eg3 = 3;
   return s;
}




//struct StateNode* newStateNode(int stateDataname)
struct State* newStateObject()
{
	//struct StateNode* n = (struct StateNode *)malloc(sizeof(struct StateNode));
	//n->s = (struct State *)malloc(sizeof(struct State));
	struct State* s = (struct State *)malloc(sizeof(struct State));
	//n->s->dataName._dataname_struct1.the_dataname = stateDataname;
	s->bestGradientToObtain = NULL;
	s->bestGradientToDeliver = NULL;
	s->deliveryInterfaces = NULL;
	s->obtainInterface = NULL;
	s->obtainInterfaces = NULL;
	s->pktQ = NULL;
	s->bestGradientToObtainUpdated = FALSE;
	s->bestGradientToDeliverUpdated = FALSE;
	s->action = FORWARD_ACTION;
	s->seqno = 0;
	s->broken = 0;
	s->converged = 0;
	s->prefix = 0;

	//n->left = NULL;
	//n->right = NULL;
	//return n;
	return s;
}






void freeStateObject(State* s)
{
    if (s)
    {
        // not removing s->bestGradientToObtain here, use freeKDGradientNode()
        // not removing s->bestGradientToDeliver here, use freeKDGradientNode()
        // not removing actual 'Interface' here, use freeInterfaceNode()
        // not removing actual 'Interface' here, use freeInterfaceNode()
        freeInterfaceList(s->deliveryInterfaces);
        freeInterfaceList(s->obtainInterfaces);
        free (s);
    }
}












context* context_new()
{
   context *c = (context*)malloc(sizeof(context));
   c->full_name[0] = '\0';
   c->eg2 = 2;
   c->eg3 = 3;
   return c;
}



void context_free(context* c)
{
    if (c)
    {
        free(c);
    }
}







trie* trie_new()
{
   trie *t = (trie*)malloc(sizeof(trie));
   //t->substr = (char*)malloc(10);
   t->s = NULL;
   t->c = NULL;
   t->keyelem = 0;
   t->first_child = NULL;
   t->next_sibling = NULL;

   return t;
}






void trie_free(trie* t)
{
   if (t)
   {
       trie_free(t->first_child);
       trie_free(t->next_sibling);
       freeStateObject(t->s);
       context_free(t->c);
       free(t);
   }
}







/*
The function is used to traverses the siblings
this one puts a line '-' if already have node we are looking for
*/
trie* trie_at_level(trie *t, unsigned char c)
{
   while(t != NULL)
   {
      if(t->keyelem == c)
      {
		 //RD( "-" << c );
         return t;
      }
      t = t->next_sibling;
   }
   return NULL;
}



/*
This version of the add only adds the object to the very last node
in the key string
*/
/*
 * MAYBE!!!!!!!!!!!
 * We should have a version of this which just searches for an exact match not adds
 */

// t must already be a new starting trie
trie* trie_add(trie *t, unsigned char *str, int object)
{
   const int n = strlen((const char*)str);
   int i;
   //trie* found;
   //RD( "Adding: " );

   if ( !n )
   {
       RD( "null state name!" << std::endl );
   }

   for(i=0; i<n; i++)
   {
      unsigned char c = str[i];
      trie* parent = t;

      t = t->first_child;
      t = trie_at_level(t,c);
      //t = found = trie_at_level(t,c);
      //if ( !found )
      if ( !t )
      {
         t = trie_new();
         t->keyelem = c;
         //t->substr[1] = '\0';
         //RD( c );
         t->next_sibling = parent->first_child;
         parent->first_child = t;
      }
   }

   //if ( !found )
   //{
    //   if ( object == STATE )
    //   {
    //     t->s = newStateObject();
    //   }
    //   if ( object == CONTEXT )
    //   {
    //     t->c = context_new();
    //   }
   //}

   if ( object == STATE && !(t->s) )
   {
    t->s = newStateObject();
   }

   if ( object == CONTEXT && !(t->c) )
   {
    t->c = context_new();
   }

   //RD( endl );
   return t;
}







///////////////////////////////////////////////////////////////////////////////////

// Not used we think...
trie* trie_lookup_longest_prefix_extra2(trie *t, unsigned char *str)
{
   const int n = strlen((const char*)str);
   int i;
   trie *current = t;
   //RD( "Matching: " << str << endl );
   //RD( "Prefix: " );

   for(i=0; i<n; i++, current = t)
   {
      unsigned char c = str[i];
      t = t->first_child;
      t = trie_at_level(t,c);
      if(t == NULL)
	  {
		   //RD( "\t- Found" );
		   for ( i++; i < n && str[i] != DOT; i++ );
		   //if ( str[i] == '.' ) RD( "\t- Moving to next section " << endl << "Prefix: ");
		   i--;
		   t = current;
	  }
   }

   if ( current->s )
   {
	   //RD( "\t- Found" << endl );
	   return current;
   }
   else
   {
	   //RD( "\t- Not Found" << endl );
	   return NULL;
   }
}



trie* trie_lookup2(trie *t, unsigned char *str)
{
   const int n = strlen((const char*)str);
   int i;

   for(i=0; i<n; i++)
   {
      unsigned char c = str[i];
      t = t->first_child;
      t = trie_at_level(t,c);
      if(t == NULL)
         return NULL; // originally return zero
   }
   if ( t->c )
   {
	   return t; // originally return t->on
   }

   return NULL;
}



// Not used we think...
trie* f(trie *t, int i, int n, unsigned char *str, trie *rec)
{
	  trie *current = t;

	  if ( i >= n )
	  {
		   if ( current->s )
		   {
			   //RD( "\t- Found " );
			   // add to list
			   //RD( current->s->full_name << endl );
			   rec->rec = current;
			   return current;
		   }
		   else
		   {
			   //RD( "\t- Not Found" << endl );
			   rec->rec = NULL;
			   return rec;
		   }
	  }

      unsigned char c = str[i];
	  t = t->first_child;
      //t = trie_at_level_noline(t,c);
      t = trie_at_level(t,c);
      if(t == NULL)
	  {
		   for ( i++; i < n && str[i] != DOT; i++ );
		   //if ( str[i] == '.' ) RD( "\t- Moving to next section " << endl );
		   i--;
		   t = current;
		   rec = f(t, i+1, n, str, rec);
	  }
	  else
	  {
		  rec = f(t, i+1, n, str, rec);
		  for ( i++; i < n && str[i] != DOT; i++ );
		  //if ( str[i] == '.' ) RD( "\t- Moving to next section " << endl );
		  i--;
		  t = current;
		  rec = f(t, i+1, n, str, rec);
	  }

	  return rec;

}





// NOT USED WE think...
void new_one(trie *t, unsigned char *str, NEIGHBOUR_ADDR _if)
{
   const int n = strlen((const char*)str);
   //f(t, 0, n, str, t);
   action_all_prefixes(t, 0, n, str, current_prefix_name, _if, consider_reinforce_interest);
}


// The application related data names

void addName(NameNode** Head, char* name)
{
	struct NameNode *temp;
	temp=(struct NameNode *)malloc(sizeof(struct NameNode));
	temp->Data = name;
	temp->Next=(*Head);
	(*Head)=temp;
}


void iterateNameData(NameNode* Head)
{
	struct NameNode *cur_ptr=Head;
	while ( cur_ptr )
	{
		printf("%s\n", cur_ptr->Data);
		cur_ptr = cur_ptr->Next;
	}
	printf("\n\n");
}



void iterateNameDataForSpecificPurpose(NameNode* Head)
{
	struct NameNode *cur_ptr=Head;
	while ( cur_ptr )
	{
		printf("%s\n", cur_ptr->Data);
		cur_ptr = cur_ptr->Next;
	}
	printf("\n\n");
}






/*
 *
void handle_reinforce_interest(NEIGHBOUR_ADDR _interface)
{

    // reinforce the the preceding interface in the direction of sink
    // CHANGE_
    reinforceObtainGradient((char*)incoming_packet.data, _interface);

    //StateNode* sn = FindStateNode(rd->stateTree, incoming_packet.the_message.data_value);
    //trie* t = trie_lookup_longest_prefix_extra2(rd->top_state, (const char*)incoming_packet.data);


      - Lets try matching on prefix at reinforcement initiation then just reinforce the
      - same key string as used in the interest

    trie* t = trie_add(rd->top_state, (const char*)incoming_packet.data, STATE);

    // a reinforced obtain gradient to self indicates this is the source, so stop
    // and ultimately allow this source to send data
    //KDGradientNode* gn;
    //if ( (gn = SearchForKDGradientNode(incoming_packet.the_message.data_value, SELF_INTERFACE, rd->grTree)) )
            //&& gn->key1->obtainInterface == gn->key2 )
    //{
    //  return;
    //}

    //if ( sn && sn->s->bestGradientToDeliver )
    if ( t && t->s->bestGradientToDeliver )
    {
        // next hop already reinforced so stop here
        //if ( g->key1->bestGradientToObtain->obtainReinforcement )
        if ( t->s->deliveryInterfaces )
            return;

        // find the next interface to reinforce
        NEIGHBOUR_ADDR interface = t->s->bestGradientToDeliver->key2->iName;

        // also reinforce the path to the source for breakage messages
        // even if it is self
        reinforceDeliverGradient((char*)incoming_packet.data, interface);

        // If interface is self do not send message on
        if ( interface == SELF_INTERFACE )
            return;

        outgoing_packet.message_type = REINFORCE_INTEREST;
        outgoing_packet.data = incoming_packet.data;
        outgoing_packet.path_value = 0;
        sendAMessage(interface, write_packet());

    }

}

 */
