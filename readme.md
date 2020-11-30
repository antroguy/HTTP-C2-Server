### Summary
The HTTPS C2 Server is a C2 framework that utilizes PNG images to establish command and control to deployed clients. I did this strictly for educational purposes in order to gain an understanding of how to develop C2 servers/clients at the system API level, as well as better prepare myself to defend agaisnt malicious threat actors. The C2 server was developed on Ubuntu and the C2 client was developed for Windows 10 OS. Both the C2 server and client were deveveloped in C++. Be aware that this is my first attempt at developing a project in C++, don't expect expert level code! XD

### QuickStart
1. First clone the HTTPS-C2-Server repository from a Ubuntu VM:
```
$ git clone https://github.com/antroguy/HTTPS-C2-Server
```
2. Go to HTTPS-C2-Server/build and run the following command to start the server. The default port is 8080. A future update will allow you to specify the port and host address to bind. 
```
$ ./serverMain
```
From here you can type "show" in order to show all the options available.

![alt text] https://github.com/antroguy/HTTPS-C2-Server/blob/master/Documentation/Show-Command.PNG

The commands that are available are as follows:

**set:**  Set an option value
**show:** Show all options
**bots:** Show all active clients and their IDs
**run:**  Generate an encoded PNG image with the corresponding option commands. 

Lets go over the available options real quick.

**KILL** - This can be set to True or left blank. If True, the client will terminate the process after completing all the commands issued by the server. 
 * Example: "Set KILL True".
**CONF** - Modify the variable of a client. The only supported variable at this time is beacon. To set the beacon time (How frequently the client reaches back to the server for commands), you will need to run the command "set CONF beacon-20000", where the variable 20000 is in milliseconds.
 * Example: "Set CONF beacon-100"
**EXEC** - Set the command to be executed on the client. This is a way to issue commands to the clients without establishing a remote shell.
 * Example: "Set EXEC ipconfig".
**UPLOAD** - Upload a file to the clients. Here you will need to set the full path of the file you want the clients to download.
 * Example: "Set UPLOAD /tmp/netcat"
**ID** - Set the ID of the client you want to run the command. This can either be set to "ALL", for which every single client will process the commands, or you can set the ID to the ID number of a connected client. To see the IDs of all connected clients issue the command "bots".
 * Example: "Set ID ALL" OR "Set ID 0".
**SHELL** - Establish a remote shell to a client. For this option you will be setting the port that you want the client to connect to.
 * Example: "Set SHELL 8888".

View the following link to see the flow of the program. 

[HTTPS-C2-Program-Flow](https://github.com/antroguy/HTTPS-C2-Server/tree/master/Documentation/C2_HTTPS_Program_Flow.pdf)

### Features
HTTPS Server Commands
* ID   - Sets the ID of the BOT for which the command is meant. The ID can be set to "ALL" or the actual String ID sent to the server from the client. 
* KILL - Tells the client to kill itself. Future update will incorporate self deletion/cleanup.
* CONF - Sets a configuration variable on the client. Acceptbale parameters to change are url or beacon.
* SHELL - Establishes a shell with the specified client. Requires ID to be set (Can only establish one concurrent session at a time. This is not encrypted at the moment. Encryption will be incorporated in a future update. 
* EXEC - Executes a command on the client. (Execution output is encoded and sent back to server via a PNG image)
* UPLOAD - Server uploads a specified file to the client. (Currently un-encrypted)

### Future Updates
* Encorporate SSL encryption into HTTPS POST/GET requests. Currently HTTP is being utilized.
* Add command "DOWNLOAD" - Server will be able to download data from a client via an encrypted channel. 
* Modify command "UPLOAD" - Encorporate Encryption
* Modify command "KILL" - Encorporate process self deletion/cleanup
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
