Project 3, Group 32
Matthew Zhang		zhan7342
Alexander Pagels	pagel067

Our program includes a makefile which, when you run make, will compile the program.  At that point, there are two options for running our program: "make test" or "make test_full". "test_full" will run the server with dynamic pool sizing and cache, while "test" will not include those options. Next, select a port number to use. From here, open a new terminal and download a file. For example, wget http://127.0.0.1:9000/image/jpg/29.jpg would download 29.jpg. However, the 9000 must first be replaced with the port number selected earlier. The user may create as many of these requests as they would like by simpling running the above wget command in the same terminal with different file paths for different files. Lastly, cntrl + C will cause the program to terminate gracefully and print out an end message. 

This program works by building a request queue with num_dispatcher dispatchers and num_workers workers to handle any processes on the server. Dispatchers are responsible for recieving any requests and storing those requests in a queue. Requests may only be added if the queue is not full. From here, workers take the request and use additional utility functions to get the file type, size, and contents and then process the request. The request is then removed from the queue. In order for this to happen however, the queue must not be empty. The user may continue to create new requests and then terminate the program using cntrl + c.

Our group implements none of the extra credit. We did attempt before submission, but what we tried caused errors in the entire program. The following two lines include what we attempted to do for the implementations of Parts A and B. 

Our policy to dynamically change the worker thread pool size is simply to add a worker whenever one is requested.

Our cache is declared as a pointer, then initiated with a malloc of size CACHE_SIZE.  This starts the cache with length 1, but will expand as needed.

Matthew did most of the code writing, error checking, and bug fixing for the main part of the project.  Alex wrote the function readFromDisk(), the extra credit portions, the ReadMe, and did some overall code housekeeping.  Alex's wife had a major surgery as we were assigned this project, so he was unable to participate for the first week and a bit.