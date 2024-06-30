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

Most of the time the snap item sizes are not actually included in the snapshot data. But they are looked up in a table.
Because client and server already know the fixed sizes of all snap items. But if there are new snap items added after a major
release without a breaking change. Then these new items contain a size field. This can be used by official releases to add new optional
items that get dropped by older clients (used by 0.7.5 race extension). And by custom modifications to extend the protocol (used by ddnet).

Those size fields are marked yellow to indicate this special case.

![dbg_snap 1 size](https://raw.githubusercontent.com/ChillerDragon/cdn/master/unknown_size_snap_item.png)

## errors are always printed

Here you can see me downloading a map with a wifi connection and 300 ping

![300 ping map download](https://raw.githubusercontent.com/ChillerDragon/cdn/master/300_ping_map_download.png)

