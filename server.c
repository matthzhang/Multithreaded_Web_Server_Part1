/*
 * Project 3
 * Matthew Zhang		zhan7342
 * Alexander Pagels		pagel067
 */

#include "server.h"
#include "unistd.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>

//Global Variables [Values Set in main()]
int				queue_len       = INVALID;              				//Global integer to indicate the length of the queue
int				cache_len       = INVALID;                              //Global integer to indicate the length or # of entries in the cache        [Extra Credit B]
int				num_worker      = INVALID;                              //Global integer to indicate the number of worker threads
int				num_dispatcher	= INVALID;                              //Global integer to indicate the number of dispatcher threads
unsigned int	dynamic_flag	= INVALID_FLAG;                         //Global flag to indicate if the dynamic poool is being used or not         [Extra Credit A]
unsigned int	cache_flag      = INVALID_FLAG;                         //Global flag to indicate if the cache is being used or not                 [Extra Credit B]
struct			sigaction		action;                                 //Global signal handling structure for gracefully terminating from SIGINT
FILE 			*logfile;                                               //Global file pointer for writing to log file in worker


/* ************************ Global Hints **********************************/

//int ????			= 0;                                                //[Extra Credit B]  --> If using cache, how will you track which cache entry to evict from array?
int removeNext      = 0;                                                //[worker()]        --> How will you track which index in the request queue to remove next?
int insertNext      = 0;                                                //[dispatcher()]    --> How will you know where to insert the next request received into the request queue?
int num_request     = 0;                                                //[multiple funct]  --> How will you update and utilize the current number of requests in the request queue?


pthread_t 	workerThread[MAX_THREADS];                            		//[multiple funct]  --> How will you track the p_thread's that you create for workers?
pthread_t 	dispatchThread[MAX_THREADS];                                //[multiple funct]  --> How will you track the p_thread's that you create for dispatchers?
int 		arg_arr1[MAX_THREADS];
int 		arg_arr[MAX_THREADS];                                       //[multiple funct]  --> Might be helpful to track the ID's of your threads in a global array
int     	totalRequests[MAX_THREADS];									//[worker()] --> total number of requests processed by each worker thread
//pthread_t	???;                                                    	//[Extra Credit A]  --> If you create a thread pool worker thread, you need to track it globally


pthread_mutex_t lock			= PTHREAD_MUTEX_INITIALIZER;            //What kind of locks will you need to make everything thread safe?                                    [Hint you need multiple]
pthread_mutex_t queuelock		= PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t totalReqLock	= PTHREAD_MUTEX_INITIALIZER; 
pthread_cond_t	dispatcher		= PTHREAD_COND_INITIALIZER;             //What kind of conditionals will you need to signal different events (i.e. queue full, queue empty)   [Hint you need multiple]
pthread_cond_t	work			= PTHREAD_COND_INITIALIZER; 

request_t* 		queue[MAX_QUEUE_LEN];                                   //How will you track the requests globally between threads? How will you ensure this is thread safe?


//cache_entry_t* ?????;                                             	//[Extra Credit B]  --> How will you read from, add to, etc. the cache? Likely want thisto be global

/**********************************************************************************/
/*
	THE CODE STRUCTURE GIVEN BELOW IS JUST A SUGGESTION. FEEL FREE TO MODIFY AS NEEDED
*/
/* ************************ Signal Handler Code **********************************/
// handles termination of the program (when ^C is received)
void gracefulTerminationHandler(int sig_caught) {
	/* TODO (D.I)
	*    Description:      Mask SIGINT signal, so the signal handler does not get interrupted (this is a best practice)
	*    Hint:             See Lab Code
	*/
	signal(SIGINT, SIG_IGN);
	// masks SIGINT signal 
	/* TODO (D.II)
	*    Description:      Print to stdout the number of pending requests in the request queue
	*    Hint:             How should you check the # of remaining requests? This should be a global... Set that number to num_remn_req before print
	*/

	int num_remn_req = num_request;  
	printf("\nGraceful Termination: There are [%d] requests left in the request queue\n", num_remn_req);
	// outputs the number of requests remaining in the queue

	/* TODO (D.III)
	*    Description:      Terminate Server by closing threads, need to close threads before we do other cleanup
	*    Hint:             How should you handle running threads? How will the main function exit once you take care of threads?
	*                      If you are using the dynamic pool thread, you should stop that thread [Extra Credit A]
	*                      pthread_cancel will be your friend here... look at the boottom of server.h for helpful functions to be able to cancel the threads
	*/
	for(int i = 0; i < num_worker; i++){
		if(pthread_cancel(workerThread[i]) == -1)
			printf("Error in closing worker thread\n");
	}

	for(int j = 0; j < num_dispatcher; j++){
		if(pthread_cancel(dispatchThread[j]) == -1)
			printf("Error in closing dispatcher thread\n");
	}
	// Close worker and dispatcher threads and error checks
	
	/* TODO (D.IV)
	*    Description:      Close the log file
	*/

	if(fclose(logfile) != 0)
		printf("Error closing logfile\n");
	// close the logfile and error checks

	/* TODO (D.V)
	*    Description:      Remove the cache by calling deleteCache IF using cache [Extra Credit B]
	*/


	/* Once you reach here, the thread join calls blocking in main will succeed and the program should terminate */
	printf("end of graceful termination\n");
}
/**********************************************************************************/

/* ******************** Dynamic Pool Code  [Extra Credit A] **********************/
// Function implements the policy to change the worker thread pool dynamically
// depending on the number of requests
void * dynamic_pool_size_update(void *arg) {

	/********************* DO NOT REMOVE SECTION - TOP     *********************/
	EnableThreadCancel();               //Allow thread to be asynchronously cancelled
	/********************* DO NOT REMOVE SECTION - BOTTOM  *********************/ 

	/* TODO (dynamic.I)
	*    Description:      Setup any cleanup handler functions to release any locks and free any memory allocated in this function
	*    Hint:             pthread_cleanup_push(pthread_lock_release,  <address_to_lock>);
	*                      pthread_cleanup_push(pthread_mem_release,   <address_to_mem>);
	*/

	int id = -1;

	/* TODO (dynamic.II)
	*    Description:      Get the id as an input argument from arg, set it to ID
	*/

	printf("%-30s [%3d] Started\n", "Dynamic Pool Thread", id);

	while(1) {
		/* TODO (dynamic.III)
		*    Description:      Run at regular intervals
		*                      Increase / decrease dynamically based on your policy
		*    Hint:             There will be limited help provided from TA's and professors here... Designed to be a challenge
		*                      Make sure this code is not run when the dynamic flag is 0
		*/
	}

	/* TODO (dynamic.IV)
	*    Description:      pop any cleanup handlers that were pushed onto the queue otherwise you will get compile errors
	*    Hint:             pthread_cleanup_pop(0);
	*                      Call pop for each time you call _push... the 0 flag means do not execute the cleanup handler after popping
	*/
}
/**********************************************************************************/

/* ************************ Cache Code [Extra Credit B] **************************/

// Function to check whether the given request is present in cache
int getCacheIndex(char *request){
	/* TODO (GET CACHE INDEX)
	*    Description:      return the index if the request is present in the cache otherwise return INVALID
	*/
	return INVALID;    
}

// Function to add the request and its file content into the cache
void addIntoCache(char *mybuf, char *memory , int memory_size){
	/* TODO (ADD CACHE)
	*    Description:      It should add the request at an index according to the cache replacement policy
	*                      Make sure to allocate/free memory when adding or replacing cache entries
	*/
}

// Function to clear the memory allocated to the cache
void deleteCache(){
	/* TODO (CACHE)
	*    Description:      De-allocate/free the cache memory
	*/
}

// Function to initialize the cache
void initCache(){
	/* TODO (CACHE)
	*    Description:      Allocate and initialize an array of cache entries of length cache size
	*/
}

/**********************************************************************************/

/* ************************************ Utilities ********************************/
// Function to get the content type from the request
// getContentType returns the type from a request by checking for specific extensions within the path of the file
char* getContentType(char *mybuf) {
	/* TODO (Get Content Type)
	*    Description:      Should return the content type based on the file type in the request
	*                      (See Section 5 in Project description for more details)
	*    Hint:             Need to check the end of the string passed in to check for .html, .jpg, .gif, etc.
	*/
	char* ht 		= ".html";
	char* htm 		= ".htm";
	char* jpg 		= ".jpg";
	char* gif 		= ".gif";
	char* toReturn	= "";
	// possible file extensions

	if(strstr(mybuf, ht)){
		toReturn = "text/html";
	}

	else if(strstr(mybuf, htm)){
		toReturn = "text/html";
	}

	else if(strstr(mybuf, jpg)){
		toReturn = "image/jpeg";
	}

	else if(strstr(mybuf, gif)){
		toReturn = "image/gif";
	}

	else{
		toReturn = "text/plain";
	}
	// check which extension the file has, updates toReturn accordingly

	//TODO remove this line and return the actual content type
	return toReturn;
	// return the type of the file
}

// Function to open and read the file from the disk into the memory
// Add necessary arguments as needed
// readFromDisk mainly uses mybuf and memory. It opens mybuf, gets the size of mybuf, and reads the cotents of mybuf to memory
int readFromDisk(int fd, char *mybuf, void **memory) {
	/* TODO (ReadFile.I)
	*    Description:      Try and open requested file, return INVALID if you cannot meaning error
	*    Hint:             Consider printing the file path of your request, it may be interesting and you might have to do something special with it before opening
	*                      If you cannot open the file you should return INVALID, which should be handeled by worker
	*/
	
	memmove(mybuf, mybuf+1, strlen(mybuf));
	int new_fd = open(mybuf, O_RDONLY);
	// remove the leading / from mybuf and open

	if(new_fd < 0){
		printf("Error in reading file\n");
		return INVALID;
	}
	// error check open

	struct stat buf;
	fstat(new_fd, &buf);
	int size = buf.st_size;
	// get the size of mybuf and stores in size

	*memory = malloc(size * sizeof(char));
	read(new_fd, *memory, size * sizeof(char));
	// allocates memory for *memory, reads contents of the file onto *memory

	close(new_fd);
	// close the file
	return size;
	// return the file size

	
	/* TODO (ReadFile.II)
	*    Description:      Find the size of the file you need to read, read all of the contents into a memory location and return the file size
	*    Hint:             Using fstat or fseek could be helpful here
	*                      What do we do with files after we open them?
	*/  


	//TODO remove this line and follow directions above
	//return INVALID;
}

/**********************************************************************************/

// Function to receive the request from the client and add to the queue
// dispatch is responsible for adding requests to the queue by checking if the queue is full
void * dispatch(void *arg) {

	/********************* DO NOT REMOVE SECTION - TOP     *********************/
	EnableThreadCancel();                                         //Allow thread to be asynchronously cancelled
	/********************* DO NOT REMOVE SECTION - BOTTOM  *********************/ 

	/* TODO (B.I)
	*    Description:      Setup any cleanup handler functions to release any locks and free any memory allocated in this function
	*    Hint:             pthread_cleanup_push(pthread_lock_release,  <address_to_lock>);
	*                      pthread_cleanup_push(pthread_mem_release,   <address_to_mem>);   [If you are putting memory in the cache, who free's it? answer --> cache delete]
	*/
	pthread_cleanup_push(pthread_lock_release, &queuelock);
	// setup cleanup handler function, used for queuelock

	int id = -1;

	/* TODO (B.II)
	*    Description:      Get the id as an input argument from arg, set it to ID
	*/
	id = *(int *)arg;
	printf("%-30s [%3d] Started\n", "Dispatcher", id);
	// get id of dispatcher
 
	while (1) {

		/* TODO (B.INTERMEDIATE SUBMISSION)
		*    Description:      Receive a single request and print the conents of that request
		*                      The TODO's below are for the full submission, you do not have to use a 
		*                      buffer to receive a single request 
		*    Hint:             Helpful Functions: int accept_connection(void) | int get_request(int fd, char *filename
		*                      Recommend using the request_t structure from server.h to store the request
		*                      Print the request information using a command like this: 
		*                      printf(“Dispatcher Received Request: fd[%d] request[%s]\n”, <insert_fd>, <insert_str>); 
		*/

		/* TODO (B.III)
		*    Description:      Accept client connection
		*    Utility Function: int accept_connection(void) //utils.h => Line 24
		*    Hint:             What should happen if accept_connection returns less than 0?
		*/
		char buffer[BUFF_SIZE];
		int fd = accept_connection();
		// get the fd 
		if(fd < 0)
			return NULL;
		// error check fd

		/* TODO (B.IV)
		*    Description:      Get request from the client
		*    Utility Function: int get_request(int fd, char *filename); //utils.h => Line 41
		*    Hint:             What should happen if get_request does not return 0?
		*/
		int request = get_request(fd, buffer);
		if(request != 0)
			continue;
		// get the request and error check
		printf("Dispatcher Received Request: fd[%d] request[%s]\n", fd, buffer);

		/* TODO (B.V)
		*    Description:      Add the request into the queue
		*    Hint:             Utilize the request_t structure in server.h...
		*                      How can you safely add a request to somewhere that other threads can also access? 
		*                      Probably need some synchronization and some global memory... 
		*                      You cannot add onto a full queue... how should you check this? 
		*/
		pthread_mutex_lock(&queuelock);
		// use lock for synchronization

		while(insertNext == MAX_QUEUE_LEN)
			pthread_cond_wait(&work, &queuelock);
		// wait if insertNext is equal to MAX_QUEUE_LEN, this indicates that the queue is full

		request_t* toInsert	= malloc(sizeof(request_t)); 
		toInsert->fd = fd;
		toInsert->request = buffer;
		queue[insertNext] = toInsert;
		// allocate memory to the request that will be inserted and update fd and request variables
		insertNext++;
		queue_len++;
		// update insertNext and queue_len

		pthread_cond_signal(&dispatcher);
		// send dispatcher signal
		pthread_mutex_unlock(&queuelock);
		// unlock the lock
	}

	/* TODO (B.VI)
	*    Description:      pop any cleanup handlers that were pushed onto the queue otherwise you will get compile errors
	*    Hint:             pthread_cleanup_pop(0);
	*                      Call pop for each time you call _push... the 0 flag means do not execute the cleanup handler after popping
	*/
	pthread_cleanup_pop(0);
	// cleanup handler (lock)

	 /********************* DO NOT REMOVE SECTION - TOP     *********************/
	 return NULL;
	 /********************* DO NOT REMOVE SECTION - BOTTOM  *********************/ 
}

/**********************************************************************************/

// Function to retrieve the request from the queue, process it and then return a result to the client
// The worker function is used to retrieve requests from the queue and process them using the utility functions
void * worker(void *arg) {
	/********************* DO NOT REMOVE SECTION - TOP     *********************/
	EnableThreadCancel();                                   //Allow thread to be asynchronously cancelled
	/********************* DO NOT REMOVE SECTION - BOTTOM  *********************/

	
	// #pragma GCC diagnostic ignored "-Wunused-variable"      //TODO --> Remove these before submission and fix warnings
	// #pragma GCC diagnostic push                             //TODO --> Remove these before submission and fix warnings
	

	// Helpful/Suggested Declaration
	int		num_request 		= 0;                        //Integer for tracking each request for printing into the log
	bool	cache_hit  			= false;                    //Boolean flag for tracking cache hits or misses if doing [Extra Credit B]
	int		filesize    		= 0;             	        //Integer for holding the file size returned from readFromDisk or the cache
	void	*memory				= NULL;                     //memory pointer where contents being requested are read and stored
	int		fd					= INVALID;                  //Integer to hold the file descriptor of incoming request
	char	mybuf[BUFF_SIZE];                               //String to hold the file path from the request

	//#pragma GCC diagnostic pop								//TODO --> Remove these before submission and fix warnings

	/* TODO (C.I)
	*    Description:      Setup any cleanup handler functions to release any locks and free any memory allocated in this function
	*    Hint:             pthread_cleanup_push(pthread_lock_release,  <address_to_lock>);
	*                      pthread_cleanup_push(pthread_mem_release,   <address_to_mem>);   [If you are putting memory in the cache, who free's it? answer --> cache delete]
	*/
	pthread_cleanup_push(pthread_lock_release, &queuelock);
	pthread_cleanup_push(pthread_lock_release, &lock);
	pthread_cleanup_push(pthread_mem_release, memory);
	int id = *(int *)arg;
	// setup all cleanup handler functions, worker will use 2 locks and memory 
	// get the id from the passed in argument

	printf("%-30s [%3d] Started\n", "Worker", id);

	/* TODO (C.II)
	*    Description:      Get the id as an input argument from arg, set it to ID
	*/  

	while (1) {
		/* TODO (C.III)
		*    Description:      Get the request from the queue
		*    Hint:             You will need thread safe access to the queue... how?
		*                      How will you handle an empty queue? How can you tell dispatch the queue is open? 
		*                      How will you index into the request queue? Global variable probably... How will you update your request queue index?
		*                      IMPORTANT... if you are processing a request you cannot be cancelled... how do you block being cancelled? (see BlockCancelSignal()--> server.h) 
		*                      IMPORTANT... if you are blocking the cancel signal... when do you re-enable it?
		*/
		pthread_mutex_lock(&queuelock);
		// lock the first lock

		while(insertNext == removeNext){
			pthread_cond_wait(&dispatcher, &queuelock);
		}
		// wait if the queue is empty, in which case insertNext are removeNext are equal (both 0)

		request_t* toRemove;
		// request to be removed

		BlockCancelSignal();
		// prevent the thread from being cancelled 

		toRemove = queue[removeNext];
		// get the last request to be added (always at index 0)

		totalRequests[id]++;
		// update the number of requests handled by a given worker thread

		for(int i = 0; i < queue_len - 1; i++){
			queue[i] = queue[i + 1];
		}
		// update the index positions of the requests in the queue

		queue_len--;
		insertNext--;
		// update the length of the queue and insertNext

		/* TODO (C.IV)
		*    Description:      Get the data from the disk or the cache (extra credit B)
		*    Local Function:   int readFromDisk(//necessary arguments//);
		*                      int getCacheIndex(char *request);  //[Extra Credit B]
		*                      void addIntoCache(char *mybuf, char *memory , int memory_size);  //[Extra Credit B]
		*/
		int size = readFromDisk(toRemove->fd, toRemove->request, &memory);
		// get the size of the request and read request's contents into memory

		/* TODO (C.V)
		*    Description:      Log the request into the file and terminal
		*    Utility Function: LogPrettyPrint(FILE* to_write, int threadId, int requestNumber, int file_descriptor, char* request_str, int num_bytes_or_error, bool cache_hit);
		*    Hint:             Call LogPrettyPrint with to_write = NULL which will print to the terminal
		*                      You will need to lock and unlock the logfile to write to it in a thread safe manor
		*/
		pthread_mutex_lock(&lock);
		// lock the second lock 

		LogPrettyPrint(NULL, id, totalRequests[id], toRemove->fd, toRemove->request, size, cache_hit);
		LogPrettyPrint(logfile, id, totalRequests[id], toRemove->fd, toRemove->request, size, cache_hit);
		// log the file onto terminal and logfile
		
		pthread_mutex_unlock(&lock);
		//unlock the second lock


		/* TODO (C.VI)
		*    Description:      Get the content type and return the result or error
		*    Utility Function: (1) int return_result(int fd, char *content_type, char *buf, int numbytes); //utils.h => Line 63
		*                      (2) int return_error(int fd, char *buf); //utils.h => Line 75
		*    Hint:             Don't forget to free your memory and set it to NULL so the cancel hanlder does not double free
		*                      You need to focus on what is returned from readFromDisk()... if this is invalid you need to handle that accordingly
		*                      This might be a good place to re-enable the cancel signal... EnableThreadCancel() [hint hint]
		*/
		if(size == -1){
			return_error(toRemove->fd, toRemove->request);
		}
		// check if size is -1, call return_error if it is

		else{
			return_result(toRemove->fd, getContentType(toRemove->request), memory, size);
			free(memory);
		}
		// if size isn't -1, call return_result and free the allocated memory

		free(toRemove);
		toRemove = NULL;
		// free the removed request

		EnableThreadCancel();
		// allow thread to be cancelled 

		pthread_cond_signal(&work);
		// send the work signal

		pthread_mutex_unlock(&queuelock);
		// unlock the first queue
	}

	/* TODO (C.VII)
	*    Description:      pop any cleanup handlers that were pushed onto the queue otherwise you will get compile errors
	*    Hint:             pthread_cleanup_pop(0);
	*                      Call pop for each time you call _push... the 0 flag means do not execute the cleanup handler after popping
	*/
	pthread_cleanup_pop(0);
	pthread_cleanup_pop(0);
	pthread_cleanup_pop(0);
	// cleanup handlers for the two locks and memory address used in this function

	/********************* DO NOT REMOVE SECTION - TOP     *********************/
	return NULL;
	/********************* DO NOT REMOVE SECTION - BOTTOM  *********************/
}

/**********************************************************************************/

// responsible for setting everything up (create worker/dispatcher threads, change cwd, open logfile, setup gracefulTerminationHandler, etc)
int main(int argc, char **argv) {

	/********************* DO NOT REMOVE SECTION - TOP     *********************/
	// Error check on number of arguments
	if(argc != 9){
		printf("usage: %s port path num_dispatcher num_workers dynamic_flag cache_flag queue_length cache_size\n", argv[0]);
		return -1;
	}

	//Input Variables
	#pragma GCC diagnostic ignored "-Wunused-variable"      //TODO --> Remove these before submission and fix warnings
	#pragma GCC diagnostic push                             //TODO --> Remove these before submission and fix warnings
	int k;
	int port            = -1;
	char path[BUFF_SIZE] = "no path set\0";
	num_dispatcher      = -1;                               //global variable
	num_worker          = -1;                               //global variable
	dynamic_flag        = 99999;                            //global variable
	cache_flag          = 99999;                            //global variable
	queue_len           = -1;                               //global variable
	cache_len           = -1;                               //global variable

	#pragma GCC diagnostic pop                              //TODO --> Remove these before submission and fix warnings

	/********************* DO NOT REMOVE SECTION - BOTTOM  *********************/

	/* TODO (A.I)
	*    Description:      Get the input args --> (1) port (2) path (3) num_dispatcher (4) num_workers (5) dynamic_flag (6) cache_flag (7) queue_length (8) cache_size
	*/
	port			= atoi(argv[1]);
	strcpy(path, argv[2]);
	num_dispatcher	= atoi(argv[3]);
	num_worker		= atoi(argv[4]);
	dynamic_flag	= atoi(argv[5]);
	cache_flag		= atoi(argv[6]); 
	queue_len		= atoi(argv[7]);
	cache_len 		= atoi(argv[8]);
	//gets input parameters

	/* TODO (A.II)
	*    Description:     Perform error checks on the input arguments
	*    Hints:           (1) port: {Should be >= MIN_PORT and <= MAX_PORT} | (2) path: {Consider checking if path exists (or wil be caught later)} 
	*                     (3) num_dispatcher: {Should be >= 1 and <= MAX_THREADS} | (4) num_workers: {Should be >= 1 and <= MAX_THREADS}
	*                     (5) dynamic_flag: {Should be 1 or 0} | (6) cache_flag: {Should be 1 or 0} | (7) queue_length: {Should be >= 1 and <= MAX_QUEUE_LEN}
	*                     (8) cache_size: {Should be >= 1 and <= MAX_CE}
	*/
	if(port < MIN_PORT || port > MAX_PORT){
		printf("Error, port is out of bounds\n");
		return;
	}

	if(access(path, F_OK) != 0){
		printf("Error, path is not a valid file\n");
		return;
	}

	if(num_dispatcher < 1 || num_dispatcher > MAX_THREADS){
		printf("Error, num_dispatcher is out of bounds\n");
		return;
	}

	if(num_worker < 1 || num_worker > MAX_THREADS){
		printf("Error, num_worker is out of bounds\n");
		return;
	}

	if(dynamic_flag != 1 && dynamic_flag != 0) {
		printf("Error, dynamic_flag is invalid\n");
		return;
	}

	if(cache_flag != 1 && cache_flag != 0) {
		printf("Error, cache_flag is invalid\n");
		return;
	}

	if(queue_len < 1 || queue_len > MAX_QUEUE_LEN){
		printf("Error, queue_len is out of bounds\n");
		return;
	}

	if(cache_len < 1 || cache_len > MAX_QUEUE_LEN){
		printf("Error, cache_len is out of bounds\n");
		return;
	}
	//error check each input



	/********************* DO NOT REMOVE SECTION - TOP    *********************/
	printf("Arguments Verified: \n\
			Port:           [%d]\n\
			Path:           [%s]\n\
			num_dispatcher: [%d]\n\
			num_workers:    [%d]\n\
			dynamic_flag:   [%s]\n\
			cache_flag:     [%s]\n\
			queue_length:   [%d]\n\
			cache_size:     [%d]\n\n", port, path, num_dispatcher, num_worker, dynamic_flag ? "TRUE" : "FALSE", cache_flag ? "TRUE" : "FALSE", queue_len, cache_len);
	/********************* DO NOT REMOVE SECTION - BOTTOM  *********************/

	/* TODO (A.III)
	*    Description:      Change SIGINT action for graceful termination
	*    Hint:             Implement gracefulTerminationHandler(), use global "struct sigaction action", see lab 8 for signal handlers
	*/
	// struct sigaction newAction;
	// newAction.sa_handler = gracefulTerminationHandler;
	// newAction.sa_flags 	 = 0;
	// sigemptyset(&newAction.sa_mask);
	// sigaction(SIGINT, &newAction, NULL);

	signal(SIGINT, gracefulTerminationHandler);

	/* TODO (A.IV)
	*    Description:      Open log file
	*    Hint:             Use Global "File* logfile", use LOG_FILE_NAME as the name, what open flags do you want?
	*/
	if((logfile = fopen(LOG_FILE_NAME, "w")) == NULL)
		printf("Error opening log file\n");
	//open log file, error check

	/* TODO (A.V)
	*    Description:      Change the current working directory to server root directory
	*    Hint:             Check for error!
	*/
	if(chdir(path) == -1)
		printf("Error chaning cwd to server root directory\n");
	//change cwd and error check

	/* TODO (A.VI)
	*    Description:      Initialize cache (IF CACHE FLAG SET) (extra credit B)
	*    Local Function:   void    initCache();
	*/


	/* TODO (A.VII)
	*    Description:      Start the server
	*    Utility Function: void init(int port); //utils.h => Line 14
	*/
	init(port);
	//start server
	

	/* TODO (A.VIII)
	*    Description:      Create dispatcher and worker threads (all threads should be detachable)
	*    Hints:            Use pthread_create, you will want to store pthread's globally
	*                      You will want to initialize some kind of global array to pass in thread ID's
	*                      How should you track this p_thread so you can terminate it later? [global]
	*/

	for(k = 0; k < MAX_THREADS; k++){
		arg_arr[k]  = k;
		arg_arr1[k] = k;
	}
	//set thread id for worker and dispatcher

	for(k = 0; k < num_worker; k++){
		if(pthread_create(&(workerThread[k]), NULL, worker, (void *)&arg_arr1[k]) != 0)
				printf("Thread %d failed to create\n", k);
	}
	//create worker threads

	for(k = 0; k < num_dispatcher; k++){
		if(pthread_create(&(dispatchThread[k]), NULL, dispatch, (void *)&arg_arr[k]) != 0)
				printf("Thread %d failed to create\n", k);
	}
	//create dispatcher threads

	for(k = 0; k < num_worker; k++){
		totalRequests[k] = 0;
	}
	//initialize array to count total requests for each worker thread

	/* TODO (A.IX)
	*    Description:      Create dynamic pool manager thread (IF DYNAMIC FLAG SET) [Extra Credit A]
	*    Hint:             Dynamic pool manager is a thread, it's ID should be DYNAM_POOL_TID
	*                      How should you track this p_thread so you can terminate it later? [global]
	*/


	/* TODO (A.X)
	*    Description:      Wait for each of the threads to complete their work
	*    Hint:             What can you call that will wait for threads to exit? How can you get threads to exit from ^C (or SIGINT)
	*                      If you are using the dynamic pool flag, you should wait for that thread to exit too
	*/
	for(k = 0; k < num_worker; k++){
		if(pthread_join(workerThread[k], NULL) != 0){
			printf("Error joining worker threads\n");
			return;
		}
	}
	//wait for worker thread to exit and error check

	for(k = 0; k < num_dispatcher; k++){
		if(pthread_join(dispatchThread[k], NULL) != 0){
			printf("Error joining dispatcher threads\n");
			return;
		}
	}
	//wait for dispatcher thread to exit and error check


	/* SHOULD NOT HIT THIS CODE UNLESS RECEIVED SIGINT AND THREADS CLOSED */
	/********************* DO NOT REMOVE SECTION - TOP     *********************/
	printf("web_server closing, exiting main\n");
	fflush(stdout);
	return 0;
	/********************* DO NOT REMOVE SECTION - BOTTOM  *********************/  
}
