# Copy

C program that copies files from <origin_directory> to <destination_directoty> using threads. It will create a log.csv file that contains the stats of each copied file
It has 3 main stages:
    - read : 
        - a single thread that will read the content of <origin_directory> recursively
        - it will write each file into a file buffer
    - copy :
        - n threads that will read from the file buffer
        - it will copy each file into <destination_directoty>.
        - one thread by file
        - it will write the stats (name, size, time) of each copied file into a log buffer
    - log :
        - a single thread that will read from log buffer
        - it will create the log.csv

to compile:
    gcc -o <command_name> main.c fileinfo.c loginfo.c functions.c -pthread

to run:
    <command_name> <origin_directory> <destination_directoty>

e.g.:
    gcc -o main main.c fileinfo.c loginfo.c functions.c -pthread
    ./main ./test ./destination

preconditions:
    * origin_directory must exist and contains some files
    * destination_directory must exist, without files in it
    * BUFFER_SIZE should not be 1

TODO :
    * set a path for .csv file
    * add columns header to .csv
    * configure number of threads used in each step