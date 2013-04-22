



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
            //  start_reinforce_interest((char*)_data, _if);
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




/*
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
         //
         // IF: this is a rec and we are the sink for it
         // i.e. we have a best grad already set that is this data rec
         // interface to self and cost zero (and most likely grad and state seqno zero).
         //
         // THEN:  we must leave the best grad to self in place, set its seqno to this
         // incoming seqno value +1 and the same for the state object.
         //
         // ACTUALLY we JUST want to send out the initiating interest again with the
         // 'brand new' seqno.  I.E. the best grad with IF: Self COST: Zero.
         //
         //
         //
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
*/




/*
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
         //
         // IF: this is a rec and we are the sink for it
         // i.e. we have a best grad already set that is this data rec
         // interface to self and cost zero (and most likely grad and state seqno zero).
         //
         // THEN:  we must leave the best grad to self in place, set its seqno to this
         // incoming seqno value +1 and the same for the state object.
         //
         // ACTUALLY we JUST want to send out the initiating interest again with the
         // 'brand new' seqno.  I.E. the best grad with IF: Self COST: Zero.
         //
         //
         //
        if ( s->bestGradientToDeliver &&
                s->bestGradientToDeliver->key2->iName == SELF_INTERFACE &&
                s->bestGradientToDeliver->costToDeliver == 0 &&
                s->bestGradientToDeliver->seqno == 0 &&
                s->seqno == 0) // pos improve condition
        {
            s->bestGradientToDeliver->seqno = incoming_packet.seqno+1;
            s->seqno = incoming_packet.seqno+1;
            s->reinforcementRetries = 0;
            s->rrqRetries = 0;

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
*/

