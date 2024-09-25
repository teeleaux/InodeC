#include "allocation.h"
#include "inode.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h> //for å avrunde - ceil()


/* The number of bytes in a block.
 * Do not change.
 */
#define BLOCKSIZE 4096

/* The lowest unused node ID.
 * Do not change.
 */
static int num_inode_ids = 0;

/* This helper function computes the number of blocks that you must allocate
 * on the simulated disk for a give file system in bytes. You don't have to use
 * it.
 * Do not change.
 */
static int blocks_needed( int bytes )
{
    int blocks = bytes / BLOCKSIZE;
    if( bytes % BLOCKSIZE != 0 )
        blocks += 1;
    return blocks;
}

/* This helper function returns a new integer value when you create a new inode.
 * This helps you do avoid inode reuse before 2^32 inodes have been created. It
 * keeps the lowest unused inode ID in the global variable num_inode_ids.
 * Make sure to update num_inode_ids when you have loaded a simulated disk.
 * Do not change.
 */
static int next_inode_id( )
{
    int retval = num_inode_ids;
    num_inode_ids += 1;
    return retval;
}

/* Create a file below the inode parent. Parent must
 * be a directory. The size of the file is size_in_bytes,
 * and create_file calls the allocate_block() function
 * enough number of times to reserve blocks in the simulated
 * disk to store all of these bytes.
 * Returns a pointer to file's inodes.
 */
struct inode* create_file(struct inode* parent, char* name, int size_in_bytes) {
    // Allocate memory for the new inode
    struct inode* new_inode = (struct inode*)calloc(1, sizeof(struct inode));
    if (new_inode == NULL) {
        printf("Memory allocation failed\n");
        return NULL;
    }

    // Add the new file inode to the parent directory's list of children
    parent->num_children++;
    parent->children = (struct inode**)realloc(parent->children, parent->num_children * sizeof(struct inode*));
    parent->children[parent->num_children - 1] = new_inode;

    // Set attributes for the new inode
    new_inode->id = next_inode_id();
    new_inode->name = strdup(name); // Allocate memory for the name and copy it
    new_inode->is_directory = 0;
    new_inode->num_children = 0;
    new_inode->children = NULL;
    new_inode->filesize = size_in_bytes;
   // printf(" new_inode->filesize:  %d",  new_inode->filesize);
    new_inode->num_blocks = (new_inode->filesize / BLOCKSIZE + 1);
    new_inode->blocks = (size_t*)calloc(new_inode->num_blocks, sizeof(size_t));
    if (new_inode->blocks == NULL) {
        printf("Memory allocation failed\n");
        free(new_inode->name);
        free(new_inode);
        return NULL;
    }

    // Allocate blocks for the file using the allocate_block function
    for (int i = 0; i < new_inode->num_blocks; i++) {
        int block = allocate_block();
        if (block == -1) {
            // Error: Not enough space on the simulated disk
            free(new_inode->blocks); // Free the allocated blocks
            free(new_inode->name);
            free(new_inode); // Free the new_file inode
            printf("Error: Not enough space on the disk\n");
            return NULL;
        }
        new_inode->blocks[i] = block;
    }

    // Return the new inode
    return new_inode;
}

/* Create a directory below the inode parent. Parent must
 * be a directory.
 * Returns a pointer to file's inodes.
 */
struct inode* create_dir(struct inode* parent, char* name) {
    // Allocate memory for the new directory inode
    struct inode* new_directory = (struct inode*)calloc(1, sizeof(struct inode));
    if (new_directory == NULL) {
        printf("Memory allocation failed\n");
        return NULL;
    }

    // Link new directory to parent if parent is not NULL
    if (parent != NULL) {
        parent->num_children++;
        parent->children = (struct inode**)realloc(parent->children, parent->num_children * sizeof(struct inode*));
        parent->children[parent->num_children - 1] = new_directory;
    }

    // Set attributes for the new directory inode
    new_directory->id = next_inode_id();
    new_directory->name = strdup(name);  // Allocate memory for the name and copy it
    new_directory->is_directory = 1;
    new_directory->num_children = 0;
    new_directory->children = (struct inode**)calloc(1, sizeof(struct inode*));
    if (new_directory->children == NULL) {
        printf("Memory allocation failed\n");
        free(new_directory);
        return NULL;
    }
    new_directory->filesize = 0;
    new_directory->num_blocks = 0;
    new_directory->blocks = NULL;

    // Return the new inode
    return new_directory;
}

struct inode* find_inode_by_id( struct inode** inodeContainer, int antNoder, int id )
{
    //"Parent must point to a directory inode. If no such inode exists, then the function returns NULL." 
    //Litt usikker paa om man maa skrive if ( parent->is_directory == 0 ) isteden
    for (int i = 0; i<antNoder; i++){

        if (inodeContainer[i]->id == id){
        	return inodeContainer[i];
        }
    }
    return NULL;
}

struct inode* find_inode_by_name( struct inode* parent, char* name )
{
    //"Parent must point to a directory inode. If no such inode exists, then the function returns NULL." 
    //Litt usikker paa om man maa skrive if ( parent->is_directory == 0 ) isteden
    if(parent->is_directory == '\0'){
        fprintf(stderr, "Parent inode is not a directory.\n");
        return NULL;
    }

    for(int i = 0; i < parent->num_children; i++){

           struct inode* child = (struct inode*) parent->children[i];

           if(strcmp(child->name, name) == 0){
               return child;
           }
       }
    return NULL;
}

void set_pointer( struct inode* parent, struct inode** inodeContainer, int antNoder){

	for (int i = 0; i < parent->num_children; i++){
        for (int j = 0; j < antNoder; j++) {

            if (parent->children[i]->id == inodeContainer[j]->id ) {
                //perform swap of inodes, so iNode->children will get nodes we want and inodeContainer will get the filler nodes
                
		//Proposed solution no 1
		//struct inode* fromContainer = malloc(sizeof(struct inode));
		//fromContainer = inodeContainer[j];
		//inodeContainer[j] = parent->children[i];
		//parent->children[i] = fromContainer;
		//free (fromContainer);
                //break;
		
		//Proposed solution no 2
                free(parent->children[i]);
                parent->children[i] = inodeContainer[j];
                break;
            }		
        }
	}
}

static int verified_delete_in_parent(struct inode* parent, struct inode* node )
{
    if (!parent->is_directory){
    fprintf(stderr, "Parent inode is not a directory.\n");
    return -1;
    }
    
    for (int i = 0; i<parent->num_children; i++)
    {
        if (parent->children[i]->id == node->id)
        {
            return -1;
        }
    }
    return 0;
}

int is_node_in_parent( struct inode* parent, struct inode* node )
{
    if (!parent->is_directory){
    fprintf(stderr, "is_node_in_parent: Parent inode %s is not a directory.\n", parent->name);    return -1;
    }
    
    for (int i = 0; i<parent->num_children; i++)
    {
        if (parent->children[i]->id == node->id)
        {
            return 0;
        }
    }
    return -1;
}

int delete_file( struct inode* parent, struct inode* node )
{
    //Det antas at parent faktisk er en directory og node er en file.
    //Parent is a direct parent to the node, then the node can be deleted
    if (is_node_in_parent(parent, node) == 0) // måtte sette == 0 siden vår funksjon returnerer 0 for success!
    {
        //Move to the specified location in the array where the element you want to remove is.
        struct inode** newChildrenArray = calloc(parent->num_children-1, sizeof(struct inode*));

	//for (int u = 0; u < parent->num_children; u++)
	//{
	//    printf("CHILD FØR SLETTING I delete_file: %s, ", parent->children[u]->name);
	//}

	int pos = 0;

        for (int i = 0; i < parent->num_children; i++){
            if (parent->children[i]->id == node->id){
                pos = i;
                break;
            } else 
	    {
	        newChildrenArray[i] = parent->children[i];
	    }
        }

        //Copy the next element to the current element of array. Which is you need to perform array[i] = array[i + 1]
        for (int j = pos; j < parent->num_children-1; j++){
            newChildrenArray[j] = parent->children[j+1];
        }

        parent->num_children--; // Lower increment number of children
        free(parent->children);
	parent->children = newChildrenArray;


	//for (int f = 0; f < parent->num_children; f++)
        //{
        //    printf("CHILD ETTER SLETTING I delete_file: %s, ", parent->children[f]->name);
        //}

	//Sikkerhetssjekk. Er antageligvis her verified_delete_in_parent er tenkt aa brukes.
        if (verified_delete_in_parent(parent, node) != 0)
        {
            fprintf(stderr, "The node was not deleted. Function verified_delete_in_parent(parent, node) returned -1");
            return -1;
        }

        for (int k = 0; k < node->num_blocks; k++){
            free_block(node->blocks[k]);
        }

	free(node->blocks);
        free(node->name);
        free(node);
    }
    else
    {
        fprintf(stderr, "Parent is not a direct parent of node. The node will not be deleted.");
        return -1;
    }
    return 0;

}

int delete_dir( struct inode* parent, struct inode* node )
{

    //Det antas at begge parameterene parent og node faktisk er directories.
    if (node->num_children != 0) // endret fra 0 til NULL
    {
        fprintf(stderr, "The node contains children. The node will not be deleted.\n");
        return -1;
    }

    //Parent is a direct parent to the node AND the node has no children itself, then the node can be deleted
    if (is_node_in_parent(parent, node) == 0)
    {
	//Move to the specified location in the array where the element you want to remove is.
        struct inode** newChildrenArray = calloc(parent->num_children-1, sizeof(struct inode*));

        //for (int u = 0; u < parent->num_children; u++)
        //{
        //    printf("CHILD FØR SLETTING I delete_file: %s, ", parent->children[u]->name);
        //}

        int pos = 0;

        for (int i = 0; i < parent->num_children; i++){
            if (parent->children[i]->id == node->id){
                pos = i;
                break;
            } else
            {
                newChildrenArray[i] = parent->children[i];
            }
        }

        //Copy the next element to the current element of array. Which is you need to perform array[i] = array[i + 1]
        for (int j = pos; j < parent->num_children-1; j++){
            newChildrenArray[j] = parent->children[j+1];
        }

        parent->num_children--; // Lower increment number of children
        free(parent->children);
        parent->children = newChildrenArray;

	//Sikkerhetssjekk. Er antageligvis her verified_delete_in_parent er tenkt aa brukes.
    	if (verified_delete_in_parent(parent, node) != 0)
    	{
            fprintf(stderr, "Function verified_delete_in_parent(parent, node) returned -1. The node was not deleted. ");
            return -1;
    	}
	
        free(node->name);
	free(node->children);
        free(node);
        //Don't use free(node->children), because node->num_children = 0. "The number of files and directories is stored in num_children. If it is 0, children is NULL"
        }
    else
    {
        fprintf(stderr, "Parent is not a direct parent to the node. The node will not be deleted.");
        return -1;
    }

    return 0;
}

/* Read the file master_file_table and create an inode in memory
 * for every inode that is stored in the file. Set the pointers
 * between inodes correctly.
 * The file master_file_table remains unchanged.
 */

struct inode* load_inodes( char* master_file_table ){
    // open file
    FILE* file = fopen(master_file_table, "rb");
    if(file == NULL){
        fprintf(stderr, "load_inodes file = NULL");
        return NULL;
    }

    struct inode** inodeBeholder = calloc(1, sizeof(struct inode*));
    if (inodeBeholder == NULL) {
        printf("%s", "inodeBeholder == NULL");
        return NULL;
    }
    
	*inodeBeholder = NULL;  // initialized to NULL
	
    int antNoder = 0;

    //Create buffer for file which we will actually use
    fseek(file, 0, SEEK_END);
    int fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    unsigned char* buffer = calloc(fileSize, sizeof(unsigned char));

    if (buffer == NULL) {
        fprintf(stderr, "Failed to allocate memory for the buffer.\n");
        fclose(file);
        return NULL;
    }

    int bytesRead = fread(buffer, 1, fileSize, file);

    if (bytesRead != fileSize) {
        fprintf(stderr, "Failed to read the whole file.\n");
        fclose(file);
        free(buffer);
        return NULL;
    }

    fclose(file);

    int bytesProcessed = 0;
	int teller = 0;
    while (bytesProcessed < fileSize){
    	teller++;
        struct inode* new_inode = calloc(1, sizeof(struct inode));
        
        new_inode->id = (int)(buffer[bytesProcessed]) | (buffer[bytesProcessed+1]) | (buffer[bytesProcessed+2]) | buffer[bytesProcessed+3];
        bytesProcessed += 4;
        
        int len = (int)((buffer[bytesProcessed]) | (buffer[bytesProcessed+1]) | (buffer[bytesProcessed+2]) | buffer[bytesProcessed+3]);
        bytesProcessed += 4;
        new_inode->name = calloc(len, sizeof(char));
        
        for (int o = 0; o < len; o++){
            int intFromBuffer = (int)buffer[bytesProcessed];
            new_inode->name[o] = (char)intFromBuffer;
            bytesProcessed += 1;
        }
        
        int dirFlag = (int)buffer[bytesProcessed];
        if (dirFlag == 1)
        {
	        new_inode->is_directory = '\x01';
        } else 
	    {
	        new_inode->is_directory = '\0';
	    }
        
        bytesProcessed += 1;

        if (new_inode->is_directory) {
            new_inode->num_children = (int)(buffer[bytesProcessed]) | (buffer[bytesProcessed+1]) | (buffer[bytesProcessed+2]) | buffer[bytesProcessed+3];
            bytesProcessed += 4;
            new_inode->children = calloc(new_inode->num_children, sizeof(struct inode*));

            for ( int i = 0; i < new_inode->num_children; i++) {
                struct inode* filler = calloc(1, sizeof(struct inode));
                
                filler->id = (int)(buffer[bytesProcessed]) | (buffer[bytesProcessed+1]) | (buffer[bytesProcessed+2]) | buffer[bytesProcessed+3] | (buffer[bytesProcessed+4]) | (buffer[bytesProcessed+5]) | buffer[bytesProcessed+6] | (buffer[bytesProcessed+7]);
   				
                bytesProcessed += 8;
                new_inode->children[i] = filler;
				
                new_inode->filesize = 0;
                new_inode->num_blocks = 0;
                new_inode->blocks = NULL;
            }
        } else {
            new_inode->filesize = (buffer[bytesProcessed+3] << 24) | 
                                 (buffer[bytesProcessed +2 ] << 16) | 
                                 (buffer[bytesProcessed + 1] << 8)  | 
                                 buffer[bytesProcessed];
           
            bytesProcessed += 4;
            
            new_inode->num_blocks = (int)(buffer[bytesProcessed]) | (buffer[bytesProcessed+1]) | (buffer[bytesProcessed+2]) | buffer[bytesProcessed+3];
            bytesProcessed += 4;
            new_inode->blocks = calloc(new_inode->num_blocks, sizeof(size_t));

            for (int t = 0; t < new_inode->num_blocks; t++){
                new_inode->blocks[t] = (size_t)(buffer[bytesProcessed]) | (buffer[bytesProcessed+1]) | (buffer[bytesProcessed+2]) | buffer[bytesProcessed+3] | (buffer[bytesProcessed+4]) | (buffer[bytesProcessed+5]) | buffer[bytesProcessed+6] | (buffer[bytesProcessed+7]);
                bytesProcessed += 8;
            }
            new_inode->num_children = 0;
            new_inode->children = NULL;
        }

        antNoder++;
        inodeBeholder = realloc(inodeBeholder, sizeof(struct inode*) * antNoder);
        inodeBeholder[antNoder-1] = new_inode;
    }

    for (int i = 0; i < antNoder; i++) {
        struct inode * iNode = inodeBeholder[i];

        if (iNode->is_directory && iNode-> num_children > 0){
            set_pointer(iNode, inodeBeholder, antNoder);
        }
    }
    
    struct inode* rootOrig = inodeBeholder[0];
    
    //printf("THIS IS THE NAME OF ROOT ORIG: %s\n", rootOrig->name);
    free(inodeBeholder);
    free(buffer);
    //print_names_of_inodes(rootOrig);
    //liksom_debug_fs(rootOrig);
    
    return rootOrig;
}


/* The function save_inode is a recursive functions that is
 * called by save_inodes to store a single inode on disk,
 * and call itself recursively for every child if the node
 * itself is a directory.
 */
static void save_inode( FILE* file, struct inode* node )
{
    if( !node ) return;

    int len = strlen( node->name ) + 1;

    fwrite( &node->id, 1, sizeof(int), file );
    fwrite( &len, 1, sizeof(int), file );
    fwrite( node->name, 1, len, file );
    fwrite( &node->is_directory, 1, sizeof(char), file );
    if( node->is_directory )
    {
        fwrite( &node->num_children, 1, sizeof(int), file );
        for( int i=0; i<node->num_children; i++ )
        {
            struct inode* child = node->children[i];
            size_t id = child->id;
            fwrite( &id, 1, sizeof(size_t), file );
        }

        for( int i=0; i<node->num_children; i++ )
        {
            struct inode* child = node->children[i];
            save_inode( file, child );
        }
    }
    else
    {
        fwrite( &node->filesize, 1, sizeof(int), file );
        fwrite( &node->num_blocks, 1, sizeof(int), file );
        for( int i=0; i<node->num_blocks; i++ )
        {
            fwrite( &node->blocks[i], 1, sizeof(size_t), file );
        }
    }
}

void save_inodes( char* master_file_table, struct inode* root )
{
    if( root == NULL )
    {
        fprintf( stderr, "root inode is NULL\n" );
        return;
    }

    FILE* file = fopen( master_file_table, "w" );
    if( !file )
    {
        fprintf( stderr, "Failed to open file %s\n", master_file_table );
        return;
    }

    save_inode( file, root );

    fclose( file );
}

/* This static variable is used to change the indentation while debug_fs
 * is walking through the tree of inodes and prints information.
 */
static int indent = 0;

/* Do not change.
 */
void debug_fs( struct inode* node )
{
    if( node == NULL ) return;
    for( int i=0; i<indent; i++ )
        printf("  ");

    if( node->is_directory )
    {
        printf("%s (id %d)\n", node->name, node->id );
        indent++;
        for( int i=0; i<node->num_children; i++ )
        {
            struct inode* child = (struct inode*)node->children[i];
            debug_fs( child );
        }
        indent--;
    }
    else
    {
        printf("%s (id %d size %db blocks ", node->name, node->id, node->filesize );
        for( int i=0; i<node->num_blocks; i++ )
        {
            printf("%d ", (int)node->blocks[i]);
        }
        printf(")\n");
    }
}

/* Do not change.
 */
void fs_shutdown( struct inode* inode )
{
    if( !inode ) return;

    if( inode->is_directory )
    {
        for( int i=0; i<inode->num_children; i++ )
        {
            fs_shutdown( inode->children[i] );
        }
    }

    if( inode->name )     free( inode->name );
    if( inode->children ) free( inode->children );
    if( inode->blocks )   free( inode->blocks );
    free( inode );
}

