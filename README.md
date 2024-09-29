# Copy

command <origin_directory> <destination_directoty>

origin_directory must exist and contain files
destination_directory must exist and not include several folder
    correct : ./dest
    incorrect : ./dest/sub



it works as expected when buffer is greater than items to copy
it works as expected when num of threads is less than items to copy


pending to fix
when buffer is less than items to copy
    so thread is waiting to write
    this happens then readDirectory is blocked due to no space in buffer to write
    so it waits until other thread to read from it in order to have an empty space
    but as readDirectory is not a thread it will block the whole program
how to fix it:
    handle readDirectory as thread
when num of threads is greater than items to copy