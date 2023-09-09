# Lab7

<hr>

* A file system refers to a concrete file system type such as tmpfs,FAT32, etc. 
* And the virtual file system will be shortened as VFS.

<hr>


### Tree Structure
* A file system is usually hierarchical, and a tree is a suitable data structure to represent it.

* Each node represents an entity such as a file or directory in the file system.

* Concatenating all the edges’ name on the path generates a pathname, which VFS can parse and traverse from one node to another.

### Vnode
* We call a node in a VFS tree a **vnode**




<hr>

### **Vfs.h / Vfs.c**

#### **Functions**

1. rootfs_init<br>
    * In this part, you’ll need to implement tmpfs which follows the VFS interface, **setup tmpfs as the root** file system.
    * Create a filesystem **tmpfs**
    * Set up tmpfs attributes and mount on rootfs

2. initramfs_init<br>
    * You need to make initramfs as an read only file system which follows the VFS interface, and mount on "/initramfs".
    * Create directory "/initramfs" on root file system, then mount on it

    * Use vfs mkdir create a new directory to mount initramfs on it
    * parse

3. register_fs<br>
    * Pass to each filesystem's registration function by its name


4. create_fd<br>
    * Create space for a file struct and set attributes



#### **File Operation**

5. vfs_open<br>
    * Traverse to get last file/directory name
    * Use vnode_operation_lookup to check if the file is created
    * If so , set file discriptor and file operation open it
    * Else , if CREATE flag set , then file operation one and open it


6. vfs_close<br>
    * Get return value from the input file's file_operation_close

7. vfs_write<br>
    * Get return value from the input file's file_operation_write

8. vfs_read<br>
    * Get return value from the input file's file_operation_read


#### **Vnode Operation**

9. vfs_mkdir<br>
    * Create a directory on underlying file system, same as creating a regular file.
    * Create two Vnode , first for 


10. vfs_mount<br>
    * First traverse the target path
    * Consider two conditions
        * Filesystem is tmpfs
            * Pass to tmpfs_set_mount
            * Set up relative  attributes like mount (mounted record) and parent directorys
        * Filesystem is initramfs
            * Pass to initramfs_set_mount
            * Set up relative  attributes like mount (mounted record) and parent directorys

11. vfs_lookup<br>


12. vfs_chdir<br>
    * Create a Vnode pointer to get input pathname's directory 
    * If path doesn't exist then return Fail
    * Else set current working directory and return Success

13. traverse<br>
    * If input path is root then send the path without first '/' and set fs's root node as cur_root_node to second level traverse
    * Else set current working directory as cur_root_node and second level traverse


14. sec_traverse<br>
    * Extract the first directory (before the first '/')
    * Consider three conditions 
        * path == '.' 
            * Discard the '.' , cut path with '.' and set last input as new input to do sec traverse
        * path == '..'
            * Set parent node as cur_root_node and keep sec_traverse
        * path == ''
            * Return
    
    * Get the file type of path node by vnode_operation_lookup
    * Consider three conditions
        * Target node been mount 
            * Do sec traverse and set input as the mounted node
        * File type is Regular File
            * Set current node as target node
        * File type is Directory
            * Keep sec traversing






### **Tmpfs.h / Tmpfs.c**

#### **Functions**


1. tmpfs_new_node
    ```
          vnode
            |
        internal
    ```
    * Create a new vnode and internal
    * Set input to attributes of internal
    * Assign precreated tmpfs_f_ops and tmpfs_v_ops to the new vnode
    * Return the created vnode

2. tmpfs_setup_mount
    * Set mount fs
    * Create a new tmpfs node

3. tmpfs_register
    * Allocate memory for tmpfs's file and vnode operation
    * Set each attribute to each function

#### **File Operation**

4. tmpfs_open


5. tmpfs_close
    * If input file exist return SUCCESS
    * Else return FAIL

6. tmpfs_read
    * Check if the file type is REGULAR FILE
    * If not then return FAIL
    * Create an internal point to input file's vnode's internal
    * Set destination as input buffer , source as internal's file position
    * Copy source to destination


7. tmpfs_write
    * Check if the file type is REGULAR FILE
    * If not then return FAIL
    * Create an internal point to input file's vnode's internal
    * Set source as input buffer , destination as internal's file position
    * Copy source to destination

#### **Vnode Operation**

8. tmpfs_create
    * Create a parent internal point to input node's
    * Create a vnode with input name and set **REGULAR_FILE** flag
    * Set the new node as one child of parent internal's
    * Add up parent internal's size


9. tmpfs_mkdir
    * Create a parent internal point to input node's
    * Create a vnode with input name and set **DIRECTORY** flag
    * Set the new node as one child of parent internal's
    * Add up parent internal's size



10. tmpfs_lookup
    * Iterate all the internal childs of the directory node
    * If childs name equal to input then return its type
    * Else return FAIL





### **Initramfs.h / Initramfs.c**

**Same as Tmpfs**

#### Extra

1. parse_initramfs
    * First change direwctory to /initramfs
    * Use cpio's architecture to find
    * There are two conditions
        * c_mode == 0004 => Directory
            * If request file name isn't '.' or '..' then do same as **mkdir**
            * Else Do nothing
        * c_mode == 0008 => Regular File
            * If request file name isn't '.' or '..' then do same as **create**
            * Else Do nothing
