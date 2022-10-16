# Config files

## data.txt

Thie contain IPC id, key, shared_state.  
This file will be a bridge for multicast and IPC.  
If IPC set shared=1, that memory will be share to local network and local system.  
If IPC set shared=0, that memory will just be shared in local system.

```
[shmem_name] [shmem_key] [shared_state]
```

Example:

```
shmem1 123 1
shmem2 234 0
shmem3 213 1
shmem4 312 0
```
