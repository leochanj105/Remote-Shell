# Remote-Shell
This is a socket/multiprocessing based remote shell on Linux.

## How to use
To start a local shell, simply compile LocalShell.c   <br>
To use the remote shell, compile both the server and the client with libsocket.c <br>
For example, gcc DaemonServer.c libsocket.c -o * <br>
To start the service, run ./DaemonServer [YOUR PORT] on a machine, then run one or more ./DaemonClient [YOUR SERVER ADDRESS] [SERVICE PORT]
