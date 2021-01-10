# Volume Mounter Service

A small Windows "service" (it's not actually a service, since that would require
additional setup) which allows external applications to mount and unmount a
given volume (identified by its GUID) by issuing a request over a named pipe.
It's essentially a RPC mechanism for `SetVolumeMountPoint` and
`DeleteVolumeMountPoint`.

This was meant as a "means to an end" solution to allow non-elevated
applications to add and remove mount points. It also means that any application
can do that as long as it can communicate over a named pipe and knows the name
of that pipe.

The attached PowerShell script can be used to issue mount/unmount requests from
commandline applications :

```
cli.ps1 -Mount -DriveLetter Z -VolumeGUID 342ae97d-df6b-4939-b357-edcaaf530257
cli.ps1 -Unmount -DriveLetter Z
```

This was (and still is) my first attempt at writing PowerShell, so the code
quality is probably beyond horrible to any native speakers of that language.
Similarly, the server code had been written before I started learning about what
WinAPI is really all about, so it probably could use some improvement.

