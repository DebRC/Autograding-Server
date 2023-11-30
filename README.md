# Autograding-Server
Auto grading-Server Project developed as part of the CS744 course 
under the guidance of Professor Varsha Apte. We focused on developing a 
server for grading C++ programs. The primary objective was implementing various server architectures, 
including single-threaded, multi-threaded (with and without a thread pool), 
and asynchronous versions. 
A client-side application was also developed to facilitate the submission of C++ programs for grading.


## Project Components
- Server Implementations:
  * Single-Threaded Server: This version of the server handles grading tasks sequentially, catering to simpler scenarios and providing a baseline for performance comparison.
  * Multi-Threaded Server (Without Thread Pool): The multi-threaded version allows the server to handle multiple grading requests concurrently. Threads are created and managed individually for each incoming request.
  * Multi-Threaded Server (With Thread Pool): Introducing a thread pool helps manage resources more efficiently, limiting the number of concurrent threads and reusing them for subsequent requests.
  * Asynchronous Server: Leveraging asynchronous I/O operations, this version aims to maximize server responsiveness by handling multiple tasks simultaneously without creating a one-to-one mapping between threads and requests.

- Client Application: The client-side application is designed to send C++ programs to the server for grading. It interacts with the different server implementations to assess their performance and responsiveness.
- Load Testing: A crucial aspect of the project involves load testing each server implementation. This process is designed to simulate varying levels of concurrent requests to evaluate the servers' scalability, throughput, and overall performance under different workloads.

## How to Run?
- Single Threaded Server (v1)
  * Server
    * ``` bash run_server.sh < PortNum > ```
  * Client
    * ``` gcc -o client gradingclient.c ```
    * ``` ./client < IP:Port > < testFile > < numOfLoop > < sleepTime > ```
  * Load-Testing
    * ``` bash loadtest.sh < IP:Port > < testFile > < numOfLoop > < sleepTime > ```
   
- Multi Threaded Server [Without Thread Pool] (v2)
  * Server
    * ``` bash run_server.sh < PortNum > ```
  * Client
    * ``` gcc -o client gradingclient.c ```
    * ``` ./client < IP:Port > < testFile > < numOfLoop > < sleepTime > < timeOut > ```
  * Load-Testing
    * ``` bash loadtest.sh < IP:Port > < testFile > < numOfLoop > < sleepTime > < timeOut > ```
   
- Multi Threaded Server [With Thread Pool] (v3)
  * Server
    * ``` bash run_server.sh < NumberOfCores > < PortNum > < ThreadPoolSize > ```
  * Client
    * ``` make ```
    * ``` ./client < IP:Port > < testFile > < numOfLoop > < sleepTime > < timeOut > ```
  * Load-Testing
    * ``` bash loadtest.sh < IP:Port > < testFile > < numOfLoop > < sleepTime > < timeOut > ```
   
- Asynchronous Server (v4)
  * Server
    * ``` bash run_server.sh < numberOfCores > < portNum > < threadPoolSize > ```
  * Client
    * ``` make ```
    * ``` ./client < IP:Port > < testFile > < numOfLoop > < sleepTime > < timeOut > ```
  * Load-Testing (For single user)
    * ``` bash single_loadtest.sh <IP:Port> <testFile> ```
