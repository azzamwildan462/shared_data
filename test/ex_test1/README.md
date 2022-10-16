# Example 1

An Example for using shared memory for IPC (Inter Process Communication)
to communicate between Process or Thread in a local computer

Compile

```
mkdir build && cd build
cmake ..
make
```

Run shmem1 and shmem2 in different terminal tab

```
cd bin && ./shmem1
cd bin && ./shmem2
```

Or if not using CMakeLists (**not recommended**), you can use `-lshared_data` in compile flag for gcc,  
example:

```
gcc [source_code] -lshared_data
```
