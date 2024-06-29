# hacking-on-protocol

Verbose teeworlds 0.7 client and server. Dumping network traffic and showing details for many possible protocol error cases.

By default the client and server will always log an error message if something went wrong.
For example if a security token is not matching. Or a sequence number is wrong.
Or if the peer is responding to slow and a resend is initiated.
Or if a chunk header is invalid.


All those cases would just silently drop in a vanilla client or server.


And then there are also additional config variables
- `cl_port` Port to use for client connections to server (0 to choose a random port, 1024 or higher to set a manual port, requires a restart)
- `debug` Debug mode 0=off 1=verbose 2=more 3=more 4=full spam
- `clean` Clean log output to hide all non network related messages
- `dbg_master` Verbose master data logging
- `dbg_snap` Verbose snap data logging 0=off 1=non empty 2=including empty snaps

## debug snapshot payloads

```
./teeworlds "dbg_snap 1;connect localhost"
```

![dbg_snap 1 ints](https://raw.githubusercontent.com/ChillerDragon/cdn/master/client_snap_int_tabel.png)
![dbg_snap 1 structs](https://raw.githubusercontent.com/ChillerDragon/cdn/master/client_snap_int_structs.png)

