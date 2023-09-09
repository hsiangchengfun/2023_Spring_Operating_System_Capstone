#include "mm.h"
#include "mini_uart.h"
#include "vfs.h"
#include "tmpfs.h"
#include "string.h"

struct vnode_operations* tmpfs_v_ops;
struct file_operations* tmpfs_f_ops;
int tmpfs_registered = 0;


int tmpfs_setup_mount(struct filesystem *fs, struct mount *mount){
    
    //set tmpfs on mount 
    mount->fs = fs;
    //create a new root node for tmpfs
    mount->root = tmpfs_new_node(NULL,"/",DIRECTORY);
    printf("[Tmpfs] set root: 0x%x\n", mount);
    printf("[Tmpfs] set root vnode: 0x%x\n", mount->root);
    return 0;

}

int tmpfs_register(){

    if(tmpfs_registered == 1)return -1;
    tmpfs_registered = 1;

    tmpfs_f_ops = (struct file_operations*)chunk_alloc(sizeof(struct file_operations));
    tmpfs_v_ops = (struct vnode_operations*)chunk_alloc(sizeof(struct vnode_operations));

    tmpfs_f_ops->open = tmpfs_open;
    tmpfs_f_ops->close = tmpfs_close;
    tmpfs_f_ops->write = tmpfs_write;
    tmpfs_f_ops->read = tmpfs_read;
    tmpfs_v_ops->load_vnode = NULL;



    tmpfs_v_ops->create = tmpfs_create;
    tmpfs_v_ops->mkdir = tmpfs_mkdir;
    tmpfs_v_ops->lookup = tmpfs_lookup;

    return 0;
}

struct vnode* tmpfs_new_node(struct tmpfs_internal *parent, const char *name, int type) {
    /*   
         vnode
           |
        internal
    */
    //create space for both
    struct vnode* new_vnode = (struct  vnode*)chunk_alloc(sizeof(struct vnode));
    struct tmpfs_internal* new_internal = (struct  tmpfs_internal*)chunk_alloc(sizeof(struct tmpfs_internal));
    
    //first setup internal
    strcpy(new_internal->name , name);
    new_internal->parent = parent;
    new_internal->type = type;
    new_internal->size = 0;
    new_internal->vnode = new_vnode;
    if(type == REGULAR_FILE){
        new_internal->data = malloc(MAX_FILESIZE);
    }else{
        new_internal->data = 0;
    }
    //then steup vnode
    new_vnode->internal = (void*)new_internal;
    new_vnode->f_ops = tmpfs_f_ops;
    new_vnode->v_ops = tmpfs_v_ops;
    new_vnode->mount = 0;
    if(parent != NULL){
        new_vnode->parent = parent->vnode;
    }

    return new_vnode;

}



//file operation

int tmpfs_open(struct vnode* file_node, struct file** target) {
    return SUCCESS;
}

int tmpfs_close(struct file *file) {
    if (file)
        return SUCCESS;
    else 
        return FAIL;
}

int tmpfs_write(struct file *file, const void *buf, unsigned len){
    /*Given the file handle, 
    VFS calls the corresponding write method to write the file starting from f_pos, 
    then updates f_pos and size after write. 
    (or not if it’s a special file) 
    Returns size written or error code on error.*/


    // 1. write len byte from buf to the opened file.
    // 2. return written size or error code if an error occurs.

    //get the inode
    struct tmpfs_internal* internal = file->vnode->internal;
    if(((struct tmpfs_internal*)(file->vnode->internal))->type != REGULAR_FILE){
        return FAIL;
    }

    char* dst = &(((char*)(internal->data))[file->f_pos]);
    char* src  = (char*)buf;
    
    //copy data from buffer
    int i = 0;
    for(i = 0;i < len && (i + internal->size) < MAX_FILESIZE ;i++ ){
        dst[i] = src[i];
    }
    internal->size += i;
    return i;
}

int tmpfs_read(struct file *file, void *buf, unsigned len){

    /*Given the file handle, 
    VFS calls the corresponding read method to read the file starting from f_pos, 
    then updates f_pos after read. 
    (or not if it’s a special file)

    Note that f_pos should not exceed the file size. 
    Once a file read reaches the end of file(EOF), it should stop. 
    Returns size read or error code on error.*/

    // 1. read min(len, readable size) byte to buf from the opened file.
    // 2. block if nothing to read for FIFO type
    // 2. return read size or error code if an error occurs.

    //get the inode
    struct tmpfs_internal* internal = file->vnode->internal;
    if(((struct tmpfs_internal*)(file->vnode->internal))->type != REGULAR_FILE){
        return FAIL;
    }

    char* src = &(((char*)(internal->data))[file->f_pos]);
    char* dst  = (char*)buf;
    //copy data from buffer
    int i = 0;
    for(i = 0;i < len && i < internal->size ;i++ ){
        dst[i] = src[i];
    }
    
    //no need to update internal size
    return i;



}



//vnode operation
int tmpfs_create(struct vnode *dir_node, struct vnode **target, const char *component_name){
    /*create an regular file on underlying file system, 
    should fail if file exist. 
    Then passes the file’s vnode back to VFS.*/
    struct tmpfs_internal* parent_internal = (struct tmpfs_internal*)(dir_node->internal);

    struct vnode* new_vnode = tmpfs_new_node(parent_internal , component_name , REGULAR_FILE);

    //link this vnode's internal to be one of parent internal's child
    parent_internal->child[parent_internal->size] =  (struct tmpfs_internal*)new_vnode->internal;
    parent_internal->size++;

    *target = new_vnode;
    return SUCCESS;
}




int tmpfs_mkdir(struct vnode *dir_node, struct vnode **target, const char *component_name){

    //same as creating file , just modify REGULARFILE to DIRECTORY
    struct tmpfs_internal* parent_internal = (struct tmpfs_internal*)(dir_node->internal);

    struct vnode* new_vnode = tmpfs_new_node(parent_internal , component_name , DIRECTORY);

    //link this vnode's internal to be one of parent internal's child
    parent_internal->child[parent_internal->size] =  (struct tmpfs_internal*)new_vnode->internal;
    parent_internal->size++;

    *target = new_vnode;
    return SUCCESS;

}

int tmpfs_lookup(struct vnode *dir_node, struct vnode **target, const char *component_name){

    if(strcmp( component_name , "") == 0){
        *target = dir_node;
        return 0;
    }

    //if find the name then set target and return type
    struct tmpfs_internal* internal = (struct tmpfs_internal*)(dir_node->internal);
    for( int i = 0; i < internal->size ; i++){
        
        if( strcmp(component_name , internal->child[i]->name) == 0 ){
            
            *target = internal->child[i]->vnode;
            return internal->child[i]->type;

        }
    }

    return FAIL;
    
}

