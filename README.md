Unix Programing Final Project

Submitters:
David Tal - 204180269
Saar Pernik - 308491265

-----------------------------------------------
Compilation:

$ gcc  main.c  -lpthread -lcli -finstrument-functions  -rdynamic  -o main
-----------------------------------------------
Run Command: 

$ ./main -d /home/saar/Desktop/test/ -i 127.0.0.2
-----------------------------------------------
Netcat Command:

$ netcat -l -u -p 8080
-----------------------------------------------
Telnet Command

$ telnet localhost 8090
-----------------------------------------------

Notes:
- user:fred
- pass:nerk
- libcli should be installed.
- to edit the apache html page it should be editable or to run the program as root.
