# File Copy Program

C program that copies files from `<origin_directory>` to `<destination_directory>` using threads. It will create a `log.csv` file that contains the stats of each copied file.

## Stages

The program has three main stages:

- **Read**:
    - A single thread that will read the content of `<origin_directory>` recursively.
    - It will write each file into a file buffer.

- **Copy**:
    - `n` threads that will read from the file buffer.
    - Each thread will copy one file into `<destination_directory>`.
    - It will write the stats (name, size, time) of each copied file into a log buffer.

- **Log**:
    - A single thread that will read from the log buffer.
    - It will create the `log.csv`.

## Compilation

to compile:
```
    gcc -o <command_name> main.c fileinfo.c loginfo.c functions.c -pthread
```

to run:
```
    <command_name> <origin_directory> <destination_directoty> <num_of_threads> <log_file>
```
- <origin_directory> 
- <destination_directoty> 
- <num_of_threads> : number of threads created for copy stage
- <log_file> : name of log file

e.g.:
```
    gcc -o main main.c fileinfo.c loginfo.c functions.c -pthread
    ./main ./test ./destination 20 ./log.csv
```

## Preconditions

- `<origin_directory>` must exist and contains some files
- `<destination_directory>` must exist, without files in it
- `BUFFER_SIZE` should not be 1

## TODO

- configure number of threads used in each stage