#include "vfs.h"
#include "sched.h"
#include "mini_uart.h"
#include "mm.h"
#include "string.h"
#include "tmpfs.h"
#include "initramfs.h"
#include "fat32.h"
//1. tmpfs
//2. initramfs

struct mount *rootfs;


void rootfs_init(){

    struct filesystem *tmpfs = (struct filesystem *)chunk_alloc(sizeof(struct filesystem));
    
    tmpfs->name = (char*)chunk_alloc(16);
    strcpy(tmpfs->name , "tmpfs");

    tmpfs->setup_mount = tmpfs_setup_mount;
    register_fs(tmpfs);

    rootfs = (struct filesystem *)chunk_alloc(sizeof(struct filesystem));
    tmpfs->setup_mount(tmpfs, rootfs);


}


void initramfs_init() {
    
    vfs_mkdir("/initramfs");
    vfs_mount("/initramfs", "initramfs");
    parse_initramfs();

}


void fat32_init() {

    vfs_mkdir("/boot");
    vfs_mount("/boot", "fat32");
    parse_fat32sd();

}


int register_fs(struct filesystem *fs){
    if(strcmp(fs->name , "tmpfs") == 0){
        return tmpfs_register();
    }else if (strcmp(fs->name, "initramfs") == 0) {
        return initramfs_register();
    } else if (strcmp(fs->name, "fat32") == 0) {
        return fat32_register();
    }

    return -1;
}


struct file* create_fd(struct vnode* target, int flags){
    //create file descriptor
    //the kernel creates a file handle in the table 
    //and returns the index (file descriptor) to the user. 
    
    struct file* fd = (struct file*)chunk_alloc(sizeof(struct file));
    fd->vnode = target;
    fd->f_ops = target->f_ops;
    fd->flags = flags;
    fd->f_pos = 0;

    return fd;

}

int vfs_open(const char *pathname, int flags, struct file **target){
    
    *target = 0;
    struct vnode* target_dir;
    char target_path[VFS_PATHMAX];
    // printf("1 ");
    // printf(target_path);
    // printf("\n");
    traverse(pathname , &target_dir , target_path);
    // printf("2 ");
    // printf(target_path);
    // printf("\n");
    

    struct vnode* target_file;
    int f_type = target_dir->v_ops->lookup(target_dir , &target_file , target_path);

    if(f_type == REGULAR_FILE){
        
        *target = create_fd(target_file , flags);
        return (*target)->f_ops->open(target_file,target);
    }else if( flags && O_CREAT ){
        int new_f = target_dir->v_ops->create(target_dir, &target_file , target_path);
        
        if(new_f < 0) return FAIL;
        
        *target = create_fd(target_file , flags);
        
        return (*target)->f_ops->open(target_file,target);
    }

    return FAIL;

}

int vfs_close(struct file *file){

    int ret = file->f_ops->close(file);
    if(ret == SUCCESS){
        chunk_free(file);
    }
    return ret;

}

int vfs_write(struct file *file, const void *buf, unsigned len){

    /*
     VFS calls the corresponding write method to write the file starting from f_pos,
     then updates f_pos and size after write. (or not if it’s a special file) 
     Returns size written or error code on error.
    */
    // 1. write len byte from buf to the opened file.
    // 2. return written size or error code if an error occurs.
    return file->f_ops->write(file,buf,len);

}
int vfs_read(struct file *file, void *buf, unsigned len){
    // 1. read min(len, readable size) byte to buf from the opened file.
    // 2. block if nothing to read for FIFO type
    // 2. return read size or error code if an error occurs.
    return file->f_ops->read(file,buf,len);
}





//vnode operation

int vfs_mkdir(const char *pathname){

    // create a directory on underlying file system, 
    // same as creating a regular file.

    char child_name[VFS_PATHMAX];
    struct vnode* target_dir;

    traverse(pathname , &target_dir , child_name);

    struct vnode* child_dir;
    int type = target_dir->v_ops->mkdir(target_dir , &child_dir , child_name);

    if(type < 0)return type;
    else return SUCCESS;


}

int vfs_mount(const char *target, const char *filesystem){

    struct vnode *mount_dir;

    char path_left[VFS_PATHMAX];
    traverse(target, &mount_dir, path_left);
    
    struct mount* mt = (struct mount*)chunk_alloc(sizeof(struct mount));
    
    //two kinds => tmpfs / initramfs
    if(strcmp(filesystem,"tmpfs") == 0){
        //mount the tmpfs
        struct filesystem *tmpfs = (struct filesystem *)chunk_alloc(sizeof(struct filesystem));
    
        tmpfs->name = (char*)chunk_alloc(16);
        strcpy(tmpfs->name , "tmpfs");

        tmpfs->setup_mount = tmpfs_setup_mount;
        register_fs(tmpfs);

        tmpfs->setup_mount(tmpfs, mt);

        mount_dir->mount = mt;
        mt->root->parent = mount_dir->parent;

    }else if (strcmp(filesystem, "initramfs") == 0) {
        //mount initramfs
        struct filesystem *initramfs = (struct filesystem *)chunk_alloc(sizeof(struct filesystem));
     
        initramfs->name = (char *)chunk_alloc(16);
        strcpy(initramfs->name, "initramfs");
     
        initramfs->setup_mount = initramfs_setup_mount;
        register_fs(initramfs);
     
        initramfs->setup_mount(initramfs, mt);
     
        mount_dir->mount = mt;
        mt->root->parent = mount_dir->parent;

    }else if (strcmp(filesystem, "fat32") == 0) {

        struct filesystem *fat32 = (struct filesystem *)chunk_alloc(sizeof(struct filesystem));
        fat32->name = (char *)chunk_alloc(16);
        strcpy(fat32->name, "fat32");
        fat32->setup_mount = fat32_setup_mount;
        register_fs(fat32);
        fat32->setup_mount(fat32, mt);
        mount_dir->mount = mt;
        mt->root->parent = mount_dir->parent;

    }




    return SUCCESS;





}

int vfs_lookup(const char *pathname, struct vnode **target) {
    return SUCCESS;
}

int vfs_chdir(const char *pathname){

    //change directory
    struct vnode *target_dir;

    char path_left[VFS_PATHMAX];//the path want to change - current path
    traverse(pathname, &target_dir, path_left);

    if (strcmp(path_left, "") != 0) {
        return FAIL;
    } else {
        current->cwd = target_dir;      //current working directory 
        return SUCCESS;
    }
}



void traverse(const char* pathname, struct vnode** target_node, char *target_path){
    //if is root then set rootfs's root node
    if(pathname[0] == '/'){
                printf("[debug] traverse absolute path: %s\n", pathname);

        struct vnode* root_node = rootfs->root;
        sec_traverse(root_node , pathname + 1 , target_node , target_path);
    }else{//else set current task
        printf("[debug] traverse relative path: %s\n", pathname);

        struct vnode* root_node = current->cwd;
        sec_traverse(root_node , pathname , target_node , target_path);
    }

}

void sec_traverse(struct vnode *node, const char *pathname, struct vnode **target_node, char *target_path){

    *target_node = node;
    
    
    // printf();
    // printf("\n");

    int i=0;
    while(pathname[i]){
        if(pathname[i]=='/')break;
        target_path[i] = pathname[i];
        i++;
    }
    target_path[i++] = '\0';
    
    // 1."."
    // 2.".."
    if(strcmp(target_path , ".") == 0){
        
        sec_traverse(node , pathname + i , target_node , target_path);
        return;

    }else if(strcmp(target_path , "..") == 0){
        if(node->parent == NULL) return ;
        
        sec_traverse(node->parent , pathname + i , target_node , target_path);
    }else if( strcmp(target_path , "") == 0){
        return ;
    }

    //get the file type of path node
    int f_type = node->v_ops->lookup(node ,target_node,target_path);
    printf("[debug] lookup ret: %d\n", f_type);

    if((*target_node)->mount != NULL){
        printf("[debug] mountpoint found during lookup: vnode 0x%x\n", (*target_node)->mount->root);

        sec_traverse((*target_node)->mount->root , pathname + i ,target_node , target_path);
    }else if(f_type == REGULAR_FILE){
        printf("[debug] find REGULAR FILE\n");
        
        *target_node = node;
    
    }else if(f_type == DIRECTORY){
       printf("[debug] find DIRECTORY\n");
    
        sec_traverse((*target_node) , pathname + i , target_node , target_path);

    }else if (f_type == FAIL && node->v_ops->load_vnode != NULL) {
        // Fat32 load
        printf("[debug] find load vnode\n");
    
        int ret = node->v_ops->load_vnode(node, target_path);
        if (ret == SUCCESS) {
            sec_traverse(node, pathname, target_node, target_path);
        }
    }

}
