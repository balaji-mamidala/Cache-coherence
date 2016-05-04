#include "MOSI_protocol.h"
#include "../sim/mreq.h"
#include "../sim/sim.h"
#include "../sim/hash_table.h"

extern Simulator *Sim;

/*************************
 * Constructor/Destructor.
 *************************/
MOSI_protocol::MOSI_protocol (Hash_table *my_table, Hash_entry *my_entry)
    : Protocol (my_table, my_entry)
{
  // Initialize lines to not have the data yet!
  this->state = MOSI_CACHE_I;
  Owner_nodeID = -1; //No owner for the cache line
}

MOSI_protocol::~MOSI_protocol ()
{
}

void MOSI_protocol::dump (void)
{
    const char *block_states[8] = {"X","I", "IS", "IM", "S", "SM","O", "M"};
    fprintf (stderr, "MOSI_protocol - state: %s\n", block_states[state]);
}

void MOSI_protocol::process_cache_request (Mreq *request)
{
	switch (state)
	{
    case MOSI_CACHE_I:  do_cache_I (request); break;
    case MOSI_CACHE_IS: do_cache_IS (request); break;
    case MOSI_CACHE_IM: do_cache_IM (request); break;
    case MOSI_CACHE_S:  do_cache_S (request); break;
    case MOSI_CACHE_SM: do_cache_SM (request); break;
    case MOSI_CACHE_O:  do_cache_O (request); break;
    case MOSI_CACHE_M:  do_cache_M (request); break;

    default:
        fatal_error ("Invalid Cache State for MOSI Protocol\n");
  }
}

void MOSI_protocol::process_snoop_request (Mreq *request)
{
	switch (state)
	{
    case MOSI_CACHE_I:  do_snoop_I (request); break;
    case MOSI_CACHE_IS: do_snoop_IS (request); break;
    case MOSI_CACHE_IM: do_snoop_IM (request); break;
    case MOSI_CACHE_S:  do_snoop_S (request); break;
    case MOSI_CACHE_SM: do_snoop_SM (request); break;
    case MOSI_CACHE_O:  do_snoop_O (request); break;
    case MOSI_CACHE_M:  do_snoop_M (request); break;

    default:
    	fatal_error ("Invalid Cache State for MOSI Protocol\n");
  }
}

inline void MOSI_protocol::do_cache_I (Mreq *request)
{
  switch (request->msg)
  {
    // If we get a request from the processor we need to get the data
    case LOAD:
      /* Line up the GETS in the Bus' queue */
    	send_GETS(request->addr);
    	/* The IS state means that we have sent the GET message and we are now waiting
    	 * on DATA
    	 */
    	state = MOSI_CACHE_IS;
    	/* This is a cache Miss */
    	Sim->cache_misses++;
    	break;

    case STORE:
    	/* Line up the GETM in the Bus' queue */
    	send_GETM(request->addr);
    	/* The IM state means that we have sent the GET message and we are now waiting
    	 * on DATA
    	 */
    	state = MOSI_CACHE_IM;
    	/* This is a cache Miss */
    	Sim->cache_misses++;
    	break;

    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: I state shouldn't see this message\n");
  }
}

inline void MOSI_protocol::do_cache_IS (Mreq *request)
{
  switch (request->msg)
  {
    /* If the block is in the IS state that means it sent out a GET message
     * and is waiting on DATA.  Therefore the processor should be waiting
     * on a pending request. Therefore we should not be getting any requests from
     * the processor.
     */
    case LOAD:
    case STORE:
    	request->print_msg (my_table->moduleID, "ERROR");
      fatal_error("Should only have one outstanding request per processor!");
    default:
      request->print_msg (my_table->moduleID, "ERROR");
      fatal_error ("Client: I state shouldn't see this message\n");
  }
}

inline void MOSI_protocol::do_cache_IM (Mreq *request)
{
  switch (request->msg)
  {
    /* If the block is in the IM state that means it sent out a GET message
     * and is waiting on DATA.  Therefore the processor should be waiting
     * on a pending request. Therefore we should not be getting any requests from
     * the processor.
     */
    case LOAD:
    case STORE:
    	request->print_msg (my_table->moduleID, "ERROR");
      fatal_error("Should only have one outstanding request per processor!");
    default:
      request->print_msg (my_table->moduleID, "ERROR");
      fatal_error ("Client: I state shouldn't see this message\n");
  }
}

inline void MOSI_protocol::do_cache_S (Mreq *request)
{
  switch (request->msg)
  {
    case LOAD:
      //Data is already there in cache line
      send_DATA_to_proc(request->addr);
    	break;

    case STORE:
    	/* Line up the GETM in the Bus' queue */
    	send_GETM(request->addr);
    	/* Move to M-state*/
    	state = MOSI_CACHE_SM;
    	// We will still get data from memory as we have placed GetM on memory
    	Sim->cache_misses++;
    	break;

    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: I state shouldn't see this message\n");
  }
}

inline void MOSI_protocol::do_cache_SM (Mreq *request)
{
  switch (request->msg)
  {
    /* If the block is in the SM state that means it sent out a GET message
     * and is waiting on DATA.  Therefore the processor should be waiting
     * on a pending request. Therefore we should not be getting any requests from
     * the processor.
     */
    case LOAD:
    case STORE:
    	request->print_msg (my_table->moduleID, "ERROR");
      fatal_error("Should only have one outstanding request per processor!");
    default:
      request->print_msg (my_table->moduleID, "ERROR");
      fatal_error ("Client: S state shouldn't see this message\n");
  }
}

inline void MOSI_protocol::do_cache_O (Mreq *request)
{
  switch (request->msg)
  {
    case LOAD:
      //Data is already there in cache line
      send_DATA_to_proc(request->addr);
    	break;

    case STORE:
      Owner_nodeID = request->src_mid.nodeID;
    	/* Line up the GETM in the Bus' queue */
    	send_GETM(request->addr);
    	/* Move to M-state*/
    	state = MOSI_CACHE_SM;
    	// Owner has the data. Still it is treated as coherence miss.
    	Sim->cache_misses++;
    	break;

    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: I state shouldn't see this message\n");
  }
}

//inline void MOSI_protocol::do_cache_OM (Mreq *request)
//{
//  switch (request->msg)
//  {
//    /* If the block is in the SM state that means it sent out a GET message
//     * and is waiting on DATA.  Therefore the processor should be waiting
//     * on a pending request. Therefore we should not be getting any requests from
//     * the processor.
//     */
//    case LOAD:
//    case STORE:
//    	request->print_msg (my_table->moduleID, "ERROR");
//      fatal_error("Should only have one outstanding request per processor!");
//    default:
//      request->print_msg (my_table->moduleID, "ERROR");
//      fatal_error ("Client: S state shouldn't see this message\n");
//  }
//}

inline void MOSI_protocol::do_cache_M (Mreq *request)
{
  switch (request->msg)
  {
    case LOAD:
    case STORE:
    	//Data is already there in cache line
      send_DATA_to_proc(request->addr);
    	break;

    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: I state shouldn't see this message\n");
  }
}

inline void MOSI_protocol::do_snoop_I (Mreq *request)
{
  switch (request->msg)
  {
    case GETS:
    case GETM:
    case DATA:
    	/**
    	 * If we snoop a message from another cache and we are in I, then we don't
    	 * need to do anything!  We obviously cannot supply data since we don't have
    	 * it, and we don't need to downgrade our state since we are already in I.
    	 */
    	break;

    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: I state shouldn't see this message\n");
  }
}

inline void MOSI_protocol::do_snoop_IS (Mreq *request)
{
  switch (request->msg)
  {
    case GETS:
    case GETM:
      /** While in IS we will see our own GETS or GETM on the bus.  We should just
       * ignore it and wait for DATA to show up.
       */
      break;

    case DATA:
      /** IS state meant that the block had sent the GET and was waiting on DATA.
       * Now that Data is received we can send the DATA to the processor and finish
       * the transition to M.
       */
      send_DATA_to_proc(request->addr);
      state = MOSI_CACHE_S;
      break;

    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: I state shouldn't see this message\n");
  }
}

inline void MOSI_protocol::do_snoop_IM (Mreq *request)
{
  switch (request->msg)
  {
    case GETS:
    case GETM:
      /** While in IM we will see our own GETS or GETM on the bus.  We should just
       * ignore it and wait for DATA to show up.
       */
      break;

    case DATA:
      /** IM state meant that the block had sent the GET and was waiting on DATA.
       * Now that Data is received we can send the DATA to the processor and finish
       * the transition to M.
       */
      send_DATA_to_proc(request->addr);
      state = MOSI_CACHE_M;
      break;

    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: I state shouldn't see this message\n");
  }
}

inline void MOSI_protocol::do_snoop_S (Mreq *request)
{
  switch (request->msg)
  {
    case GETS:
      /** Someone else needs the cache line which will be provided by memory.
          We should just ignore it.
      */
      break;

    case GETM:
      /** Someone is going to modify the cache line. We should move to Invalid state
       */
      state = MOSI_CACHE_I;
      break;

    case DATA:
      /** Someone else needed the cache line data which is available to that processer.
          We should just ignore it.
      */
      break;

    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: I state shouldn't see this message\n");
  }
}

inline void MOSI_protocol::do_snoop_SM (Mreq *request)
{
  switch (request->msg)
  {
    case GETS:
      if( (-1!=Owner_nodeID) && (false==get_shared_line()) )
      {
        //give data
        set_shared_line();
        send_DATA_on_bus(request->addr,request->src_mid);
      }
      break;

    case GETM:
      if( (-1!=Owner_nodeID) && (false==get_shared_line()) )
      {
        //give data
        set_shared_line();
        send_DATA_on_bus(request->addr,request->src_mid);
      }
      if (request->src_mid.nodeID != Owner_nodeID)
      {
        //Change state
        state = MOSI_CACHE_IM;
        Owner_nodeID = -1;
      }
      break;

    case DATA:
      Owner_nodeID = -1;

      //Get data
      send_DATA_to_proc(request->addr);

      //Change to modify state
      state = MOSI_CACHE_M;
      break;

    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: State shouldn't see this message\n");
    }
}

inline void MOSI_protocol::do_snoop_O (Mreq *request)
{
  switch (request->msg)
  {
    case GETS:
      /** Someone needs the cache line to read which we have.
          We should provide that data.
      */
      set_shared_line();
      send_DATA_on_bus(request->addr,request->src_mid);
    	break;

    case GETM:
      /** Someone needs the cache line to read which we have.
          We should provide that data.
      */
      set_shared_line();
      send_DATA_on_bus(request->addr,request->src_mid);
    	state = MOSI_CACHE_I;
    	break;

    case DATA:
      /** Someone else needed the cache line data which is available to that processer.
          We should just ignore it.
      */
    	break;

    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: I state shouldn't see this message\n");
  }
}

//inline void MOSI_protocol::do_snoop_OM (Mreq *request)
//{
//  switch (request->msg)
//  {
//    case GETS:
//      break;
//
//    case GETM:
//      //give data
//      set_shared_line();
//      send_DATA_on_bus(request->addr,request->src_mid);
//      send_DATA_to_proc(request->addr);
//      state = MOSI_CACHE_M;
//      break;
//
//    case DATA:
//      break;
//
//    default:
//        request->print_msg (my_table->moduleID, "ERROR");
//        fatal_error ("Client: State shouldn't see this message\n");
//    }
//}

inline void MOSI_protocol::do_snoop_M (Mreq *request)
{
  switch (request->msg)
  {
    case GETS:
      /** Someone needs the cache line to read which we have.
          We should provide that data.
      */
      set_shared_line();
      send_DATA_on_bus(request->addr,request->src_mid);
    	state = MOSI_CACHE_O;
    	break;

    case GETM:
      /** Someone needs the cache line to read which we have.
          We should provide that data.
      */
      set_shared_line();
      send_DATA_on_bus(request->addr,request->src_mid);
    	state = MOSI_CACHE_I;
    	break;


    case DATA:
      /** Someone else needed the cache line data which is available to that processer.
          We should just ignore it.
      */
    	break;

    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: I state shouldn't see this message\n");
  }
}

