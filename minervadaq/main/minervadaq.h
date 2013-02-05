#include <string>
/*! The include files needed for the network
 *  sockets 
 */
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include "readoutObject.h"

sig_atomic_t continueRunning;          /*!< Used by the SIGTERM/SIGINT signal handler to tell the main loop to quit (guaranteed atomic write) */
void quitsignal_handler(int signum);   /*!< The signal handler for SIGTERM/SIGINT */

boost::mutex main_mutex; /*!< A BOOST multiple exclusion for use in threaded operation */

/*! short sleep function */
int minervasleep(int us);

bool data_ready, evt_record_available;   /*!<data status variables */
/*! a function for selecting a trigger and waiting on it */
int TriggerDAQ(acquire_data *daq, unsigned short int triggerType, RunningModes runningMode,
	controller *tmpController, sig_atomic_t & continueFlag); 
/*! the function which governs the entire data acquisition sequence */
int TakeData(acquire_data *daq, event_handler *evt, int croc_id, int channel_id,int thread, 
              et_att_id  attach, et_sys_id  sys_id, bool readFPGA=true, int nReadoutADC=8); 
/*! Get the Global Gate value indexed in /work/conditions/global_gate.dat */
unsigned long long inline GetGlobalGate();
/*! Put the Global Gate value into /work/conditions/global_gate.dat */
void inline PutGlobalGate(unsigned long long ggate);
/*! Write the (complete, as of trigger X) SAM metadata */
int WriteSAM(const char samfilename[], 
	const unsigned long long firstEvent, const unsigned long long lastEvent, 
	const std::string datafilename, const int detector, const char configfilename[], 
	const int runningMode, const int eventCount, const int runNum, const int subNum, 
	const unsigned long long startTime, const unsigned long long stopTime);
/*! Write the up-to-date last trigger information file */
int WriteLastTrigger(const char filename[], const int run, const int subrun, 
	const unsigned long long triggerNum, const int triggerType,
	const unsigned long long triggerTime);
/*! Synch readout nodes - write */ 
template <typename Any> int SynchWrite(int socket_handle, Any *data);
/*! Synch readout nodes - listen */ 
template <typename Any> int SynchListen(int socket_connection, Any *data); 

/* Send a Sentinel Frame. */
bool SendSentinel(acquire_data *daq, event_handler *event_data, et_att_id attach, et_sys_id sys_id);

/* "New" readout structure functions and variables. */
std::list<readoutObject*> readoutObjects; 
int TakeData(acquire_data *daq, event_handler *evt, et_att_id attach, et_sys_id sys_id, 
        std::list<readoutObject*> *readoutObjects, const int allowedTime, const bool readFPGA, 
	const int nReadoutADC, const bool zeroSuppress);

/* some logging files for debugging purposes */
#if TIME_ME
std::ofstream take_data_extime_log; /*!<an output file for tiing data */
#endif
std::ofstream trigger_log; /*!<an output file for trigger debuggin */

// Socket Communication Functions
int CreateSocketPair(int &workerToSoldier_socket_handle, int &soldierToWorker_socket_handle);
int SetupSocketService(struct sockaddr_in &socket_service, struct hostent *node_info, 
	std::string hostname, const int port );

// Socket Communication Vars.
// Trigger check - be sure both nodes agree on the trigger type.
unsigned short int workerToSoldier_trig[1]; // trigger check the worker -> soldier 
unsigned short int soldierToWorker_trig[1]; // trigger check the soldier -> worker 
// Error check - share information between nodes about readout / timeout errors.
unsigned short int workerToSoldier_error[1]; // error check the worker -> soldier 
unsigned short int soldierToWorker_error[1]; // error check the soldier -> worker 
// Gate check - be sure both nodes agree on the gate number at end of gate.
int workerToSoldier_gate[1]; // gate check the worker -> soldier 
int soldierToWorker_gate[1]; // gate check the soldier -> worker 

// worker to soldier service vars.
struct in_addr     workerToSoldier_socket_address;
int                workerToSoldier_socket_connection; 
bool               workerToSoldier_socket_is_live;

// soldier to worker service vars.
struct in_addr     soldierToWorker_socket_address;
int                soldierToWorker_socket_connection; 
bool               soldierToWorker_socket_is_live;


// Base values for cycle between set of ports.
unsigned short workerToSoldier_port = 1110;
unsigned short soldierToWorker_port = 1120; 

#if MASTER&&(!SINGLEPC) // Soldier Node
/* minervadaq server for "master" (soldier node) DAQ */
struct sockaddr_in          workerToSoldier_service;
struct sockaddr_in          soldierToWorker_service;
int                         workerToSoldier_socket_handle;
int                         soldierToWorker_socket_handle;
struct hostent *            worker_node_info; // server on worker node
#endif

#if (!MASTER)&&(!SINGLEPC) // Worker Node
/* minervadaq client for "slave" (worker node) DAQ */
struct sockaddr_in          workerToSoldier_service;
struct sockaddr_in          soldierToWorker_service;
int                         workerToSoldier_socket_handle;
int                         soldierToWorker_socket_handle;
struct hostent *            soldier_node_info; // server on soldier node
#endif

