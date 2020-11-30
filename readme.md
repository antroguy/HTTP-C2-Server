### Summary
The HTTPS C2 Server is a C2 framework that utilizes PNG images to establish command and control to deployed clients. I did this strictly for educational purposes to gain an understanding of how to develop C2 servers/clients at the system API level, as well as better prepare myself to defend against malicious threat actors. The C2 server was developed on Ubuntu and the C2 client was developed for Windows 10 OS. Both the C2 server and client were developed in C++. Be aware that this is my first attempt at developing a project in C++, do not expect expert level code! XD]

The program flow itself is simple. When a host is compromised with the HTTP-C2-Client binary it will first reach out to the server to grab the default C2 image (default.png) via a HTTP GET request. It is just a picture of a tree. The client will then decode the image to grab the initialization commands. The only initialization commands currently available is a command to generate a unique client ID (UID) and a command to change the URL of the clients next GET request (The next image it will grab for commands). The UID is a concatenation of the compromised hosts MAC address and hostname. The client will then encode an image with the UID and send the encoded image to the server via an HTTP POST request. The server will decode the received image for the clients UID and update its database of active clients. Next, the client will perform a GET request to the URL provided from the server (For now it is tree.png, this will be modifiable in future updates), and decode the image for new commands. If the image is not available from the server, an HTTP Status 400 error will be returned to the client (This just means the server has not issued any new commands). The client will then sleep for the time length of the beacon variable (default is 0) before re-trying another GET request for the image. If the GET request was successful, the client will decode the image, process the image for new commands, and execute the commands.      

View the following link to see the flow of the program. 

[HTTPS-C2-Program-Flow](https://github.com/antroguy/HTTPS-C2-Server/tree/master/Documentation/C2_HTTPS_Program_Flow.pdf)

I decided to use BGRA (Blue-Green-Red-Alpha) formatted PNG images for C2 for this project. I initially tried to use JPG images to hide C2 commands and data, however after a lot of issues and further research I realized that the JPG compression algorithm is lossy, meaning you lose portions of the raw data. I quickly switched to using PNG images which generally use lossless compression algorithm. BGRA PNG images consist of 32-bit pixels, 8 bits for each color/alpha. All commands and data are stored in the LSB of the red pixels. I could have probably stored the data in the alpha pixel, however storing the data in the red pixel bits showed no real visual change to the images.

### QuickStart
First clone the HTTPS-C2-Server repository from a Ubuntu VM:
```
$ git clone https://github.com/antroguy/HTTPS-C2-Server
```
Go to HTTPS-C2-Server/build and run the following command to start the server. The default port is 8080. A future update will allow you to specify the port and host address to bind.
```
$ ./serverMain
```
From here you can type "show" to show all the options available.

![alt text](https://github.com/antroguy/HTTPS-C2-Server/blob/master/Documentation/Show-Command.PNG)

The commands that are available are as follows:

* **set:**  Set an option value

* **show:** Show all options

* **bots:** Show all active clients and their IDs

* **run:**  Generate an encoded PNG image with the corresponding option commands. 

Lets go over the available options real quick.

* **KILL** - This can be set to True or left blank. If True, the client will terminate the process after completing all the commands issued by the server. 
  * Example: "set KILL True".
 
* **CONF** - Modify the variable of a client. The only supported variable at this time is beacon. To set the beacon time (How frequently the client reaches back to the server for commands), you will need to run the command "set CONF beacon-20000", where the variable 20000 is in milliseconds.
  * Example: "set CONF beacon-20000"

* **EXEC** - Set the command to be executed on the client. This is a way to issue commands to the clients without establishing a remote shell.
  * Example: "set EXEC ipconfig".
 
* **UPLOAD** - Upload a file to the clients. Here you will need to set the full path of the file you want the clients to download.
  * Example: "set UPLOAD /tmp/netcat"

* **ID** - Set the ID of the client you want to run the command. This can either be set to "ALL", for which every single client will process the commands, or you can set the ID to the ID number of a connected client. To see the IDs of all connected clients, issue the command "bots".
  * Example: "set ID ALL" OR "Set ID 0".

* **SHELL** - Establish a remote shell to a client. For this option you will be setting the port that you want the client to connect to.
  * Example: "set SHELL 8888".

Once your server is up and running you will need to execute the C2-Client executable on a windows device. First compile the Windows C2 client with the server IP modified to your hosts IP. Once you run the executable on the compromised machine you should see the C2 client perform it's initiation sequence (Provides the server with its unique Client ID).

![alt text](https://github.com/antroguy/HTTPS-C2-Server/blob/master/Documentation/Client_Connection.PNG)

If you type in the command "bots", the server will display all active bots with their ID number. For example, in the image below, if you wanted to issue a command that only bot "A8E5E45D06555A-DESKTOP-71HBDL1" processed, you would need to set the ID option to "0".

![alt text](https://github.com/antroguy/HTTPS-C2-Server/blob/master/Documentation/Bots_Command.PNG)

For this example, we will grab the IP configuration of the compromised host shown above. We will need to set the ID option as well as the EXEC option to the command we want to run. We will also set the beacon timespan to 20 seconds. See example below.

![alt text](https://github.com/antroguy/HTTPS-C2-Server/blob/master/Documentation/Example_1.PNG)

Once the options are set, issue the command "run". This will encode an image with the desired commands. The client will fetch the image, decode the image for the specified commands, run the commands, encode the output into an image, and send the encoded image back to the server. The server will then decode the image, grab the output from the client, and display it on the terminal.

![alt text](https://github.com/antroguy/HTTPS-C2-Server/blob/master/Documentation/Output_Example.PNG)

If we wanted to establish a remote shell to the compromised host, we will need to set the SHELL option to the designated port we want the client to connect to. The ID option cannot be set to "ALL" when establishing a remote shell, instead only one unique ID must be set. 

![alt text](https://github.com/antroguy/HTTPS-C2-Server/blob/master/Documentation/Shell_Example.PNG)

Once ready type in the "run" command. A handler will be setup on the designated port waiting for the clients incoming connection. Once you are done with the shell, simply type "exit" into the terminal to exit the remote session. 

![alt text](https://github.com/antroguy/HTTPS-C2-Server/blob/master/Documentation/Shell_Example2.PNG)

If you want to terminate all active clients, simply set the KILL option to "True" and the ID option to "ALL" and run.

As stated previously this is my first draft of the project and requires a lot of code cleanup and much further development. Feel free to view the source code to gain an understanding of what I have done. I am open to all constructive criticism and feedback, thank you! 

### Features
HTTPS Server Commands
* ID   - Sets the ID of the BOT for which the command is meant. The ID can be set to "ALL" or the actual String ID sent to the server from the client. 
* KILL - Tells the client to kill itself. Future update will incorporate self-deletion/cleanup.
* CONF - Sets a configuration variable on the client. Acceptable parameters to change are “url” or “beacon”.
* SHELL - Establishes a shell with the specified client. Requires ID to be set (Can only establish one concurrent session at a time. This is not encrypted at the moment. Encryption will be incorporated in a future update. 
* EXEC - Executes a command on the client. (Execution output is encoded and sent back to server via a PNG image)
* UPLOAD - Server uploads a specified file to the client. (Currently un-encrypted)

### Future Updates
* Incorporate SSL encryption into HTTPS POST/GET requests. Currently HTTP is being utilized.
* Add command "DOWNLOAD" - Server will be able to download data from a client via an encrypted channel. 
* Modify command "UPLOAD" - Incorporate Encryption
* Modify command "KILL" - Incorporate process self-deletion/cleanup
* Add multithreading capabilities for processing multiple requests. Currently only one request can be processed at a time from a single client. 
* Ability to alter URL that C2 clients will reach back to.
* Ability to upload any RGBA file for encoding/decoding messages. (Current default image is default.png)
* Create Dockers for both the Server/Client for easy deployment/compilation.
* Allow command line arguments to specify port and host address to bind to.

### Requirements
#### Server
* Server has only been tested on Ubuntu 18.04 LT
#### Client
* Client has only been tested on Windows 10/2016

### Disclaimer
Code samples for the C2 Server are provided for educational purposes. Adequate defenses can only be built by researching attack techniques available to malicious actors. Using this code against target systems without prior permission is illegal in most jurisdictions. The author is not liable for any damages from misuse of this information or code.
