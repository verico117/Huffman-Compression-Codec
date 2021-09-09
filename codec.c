#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <math.h>

struct Huff_Node{
    unsigned int data;
    unsigned int num_pres;
    struct Huff_Node *left, *right;
};

struct Huff_Tree{
    unsigned int size;
    struct Huff_Node **array;
};

struct Table{
    unsigned int data;
    int *code;
    int len;
};
struct Table *table;
int table_idx = 0;

struct Huff_Tree *Huff_init(int num_nodes);
struct Huff_Node *create_Node(unsigned int data, unsigned int num_pres);
struct Huff_Tree *build_tree(unsigned long int freqs[256][2], int size);
struct Huff_Tree *fill_heap(struct Huff_Tree* huff_ptr, unsigned long int freqs[256][2], int size);
struct Huff_Node *find_min(struct Huff_Tree *huff_ptr);
void remove_freq(struct Huff_Tree *huff_ptr);
struct Huff_Tree *buildAndInsertNode(struct Huff_Tree *huff_ptr, struct Huff_Node *Node1, struct Huff_Node *Node2);
void heapify(struct Huff_Tree *huff_ptr, int idx);
int val_check(unsigned long int freq[256][2], int size, unsigned char val);
void create_Table(struct Huff_Tree *huff_ptr, int size);
void inTraverse(struct Huff_Node *startNode, int array[], int i);
void fill_Table(struct Table *table_ptr, const int array[], struct Huff_Node *node, int j, int i);
void treeTraverse(struct Huff_Node *startNode, int array[], int i);
unsigned int getCode(int current, int *length);

int main (int argc, char *argv[])
{
	int i = 0, j = 0, k = 0, z = 0, x = 0;
	int freq = 0;
	long int fsize = 0;
	unsigned long int freq_array[256][2], temp[1][2];
	unsigned char *file_data;
	FILE *inputfpt, *outputfpt;
	struct Huff_Tree *huff_heap;
	FILE *dest;
	int a,b,c;
	unsigned int index;
	unsigned int mode1 = 0;
	unsigned int mode = 0;
	unsigned int *size = 0;
	unsigned int length = 0;
	unsigned int start;
	unsigned int current;
	unsigned int bits_in_buff = 0;
	unsigned int count;
	unsigned int found;
	unsigned char *out;
	unsigned char *fptr;
	unsigned char *end;
	unsigned char hoff[4];	
	unsigned char buffer[4];
	unsigned int buffer32 = 0;
	unsigned int code = 0;
	unsigned int mask = 0;
	unsigned int fake = 0;
	unsigned int character;
	unsigned long codeCount = 0;

	//=========================================================================//	
	//	                          Parse Arguments                              //
	//=========================================================================//	
	while((index = getopt(argc, argv, "c:d:o:")) != -1)
	{
		switch(index)
		{
			case 'o':
					mode1 = 1;
					out = strdup(optarg);
					break;
						
			case 'c': 
					mode = 1;		//compress
					fptr = strdup(optarg);
					printf("Compressing: %s\n", fptr);
					break;
			
			case 'd':
					mode = 2;		//decompress
					fptr = strdup(optarg);
					printf("Decompressing: %s\n", fptr);
		}
	}	
	
	//=========================================================================//	
	//                                compress                                 //
	//=========================================================================//	
	if((mode == 1) && (mode1 == 1))			//compress
	{

    	inputfpt = fopen(fptr,"rb");
		if(inputfpt == NULL)
		{
			printf("Unable to open %s\n", fptr);
			exit(0);
		}
    	outputfpt = fopen(out, "w+b");
		if(outputfpt == NULL)
		{
			printf("Unable to open %s\n", out);
			exit(0);
		}

		end = fptr + strlen(fptr);
		while(end > fptr && *end != '.')
		{
			--end;
		}
		if(end > fptr)
		{
			*end = '\0';
		}
		strcpy(hoff,".hoff");
		strcat(fptr,hoff);
		printf("Compressed File: %s\n", fptr);

    	fseek(inputfpt,0L,SEEK_END);
    	fsize = ftell(inputfpt);
    	rewind(inputfpt);
    	file_data = (unsigned char *)calloc(fsize, sizeof(unsigned char));
    	fread(file_data, 1, fsize, inputfpt);

    	while(i < fsize)
	{
    	    if(val_check(freq_array,k,file_data[i]) == 0)
	    {
    	        i = i + 1;
    	    }else
	    {
    	        for(j = 0; j < fsize; j++)
		{
    	            if(file_data[j] == file_data[i])
		    {
    	                freq = freq + 1;
    	            }
    	        }
    	        freq_array[k][0] = file_data[i];
    	        freq_array[k][1] = freq;

    	        k = k + 1;
    	        i = i + 1;
    	        freq = 0;
    	    }
    	}
		
    	// Sort freqs
    	for(z = 0; z < k; z++)
	{
    	    for(x = 0; x < k; x++)
	    {
    	        if(freq_array[x][1] > freq_array[z][1])
		{
    	            temp[0][0] = freq_array[x][0];
    	            temp[0][1] = freq_array[x][1];
    	            freq_array[x][0] = freq_array[z][0];
    	            freq_array[x][1] = freq_array[z][1];
    	            freq_array[z][0] = temp[0][0];
    	            freq_array[z][1] = temp[0][1];
    	        }
    	    }
    	}
        // write number of freqs to file
        fputc(k, outputfpt);
		
        // Save freqs to file in sorted order
        for(i = 0; i < k; i++){
            fputc(freq_array[i][0], outputfpt);
            fwrite(&freq_array[i][1],1,5,outputfpt);
        }

    	// build tree
    	huff_heap = build_tree(freq_array,k);
    	
	// build table
    	create_Table(huff_heap, k);
		
		//write output
		dest = fopen(fptr,"wb");
		size = (int *)calloc(1, sizeof(int));	
		rewind(inputfpt);
		while((current = fgetc(inputfpt)) != EOF)
		{
			codeCount++;
		}	
		rewind(inputfpt);
		fwrite(&codeCount, sizeof(unsigned long),1,dest);
		while((current = fgetc(inputfpt)) != EOF)
		{
			code = getCode(current,size);
			length = *size;
			code <<= ((32 - bits_in_buff) - (length));
			buffer32 |= code;
			bits_in_buff += length;
	
			for(i = 0; i < 4; i++)
			{
				mask = buffer32;
				mask >>= i * 8;
				mask &= 255;
				buffer[3-i] = 0;
				buffer[3-i] |= mask;
			}
	
			while(bits_in_buff > 8)
			{
				fputc(buffer[0], dest);
				buffer32 <<= 8;
				buffer[0] = buffer[1];
				buffer[1] = buffer[2];
				buffer[2] = buffer[3];
				buffer[3] = 0;
				bits_in_buff -= 8;
			}
		
		}
		fputc(buffer[0], dest);
		free(file_data);
		free(size);
		fclose(dest);
		fclose(inputfpt);
		fclose(outputfpt);	
 
	//=========================================================================//	
	//                              decompress                                 //
	//=========================================================================//	
	}else if((mode == 2) && (mode1 == 1))		//decompress
	{
        // rebuild table
        outputfpt = fopen(out, "rb");
		if(outputfpt == NULL)
		{
			printf("Unable to open %s\n", out);
			exit(0);
		}

        fsize = fgetc(outputfpt);
        for(i = 0; i < (fsize * 2); i++)
	{
            freq_array[i][0] = fgetc(outputfpt);
            fread(&freq_array[i][1],1,5,outputfpt);
        }
        huff_heap = build_tree(freq_array, fsize);
        create_Table(huff_heap, fsize);

		inputfpt = fopen(fptr, "r");
		end = fptr + strlen(fptr);
		while(end > fptr && *end != '.')
		{
			--end;
		}
		if(end > fptr)
		{
			*end = '\0';
		}
		strcpy(hoff,".uhoff");
		strcat(fptr,hoff);
		printf("Decompressed File: %s\n", fptr);
		dest = fopen(fptr, "wb");

		struct Huff_Node *start = huff_heap->array[0];
		
		mask = 128;
		fread(&codeCount, sizeof(unsigned long),1,inputfpt);

		fread(&current,sizeof(char),1,inputfpt);
		bits_in_buff = 7;
	
		//write output	
		printf("codeCount: %lu\n", codeCount);	
		i = 0;
		while(i < codeCount)
		{
			start = huff_heap->array[0];
			while((start->left != NULL) && (start->right != NULL))
			{
				if((current &  mask) == 0)
				{
					start = start->left;
				}
				else
				{
					start = start->right;
				}

				if(bits_in_buff == 0)
				{
					fread(&current,sizeof(char),1,inputfpt);
					bits_in_buff = 7;
				}
				else
				{
					current <<= 1;
					bits_in_buff--;
				}
			}
			fputc(start->data,dest);
			i++;
		}
	
		fclose(dest);		//.uhoff
		fclose(inputfpt);   //.hoff
		fclose(outputfpt);  //output.txt

	}else		//improper use case
	{	
		printf("Improper Use!\n");
		printf("Compress   Format: ./[executable] -c [filename] -o [keyFile]\n");
		printf("Decompress Format: ./[executalbe] -d [filename] -o [keyFile]\n");
		exit(0);
	}

    return(0);
}

//=======================================//
//              functions
//=======================================//

void treeTraverse(struct Huff_Node *startNode, int array[], int i)
{	
    if(startNode->right != NULL)
    {
        array[i] = 1;
        inTraverse(startNode->right, array, i + 1);
    }
    if(startNode->left == NULL && startNode->right == NULL)
    {
        fill_Table(table, array, startNode, table_idx, i);
        table_idx = table_idx + 1;
    }
    if(startNode->left != NULL)
    {
        array[i] = 0;
        inTraverse(startNode->left, array, i + 1);
    }

	printf("Hello!\n");
}

unsigned int getCode(int current, int *length)
{
	int found = 0;
	int i = 0;
	int j = 0;
	unsigned int code = 0;	

	while(found == 0)
	{
		if(current == table[i].data)
		{
			*length = table[i].len;
			for(j = 0; j < *length; j++)
			{
				if(table[i].code[j] == 1)
				{
					code += pow(2,(*length-1)-j);
				}
			}
			found = 1;
		}
		i++;
	}	
	return code;
}

struct Huff_Tree *Huff_init(int num_nodes)
{
    struct Huff_Tree *huff_ptr = (struct Huff_Tree *)calloc(num_nodes, sizeof(struct Huff_Tree));
    huff_ptr->size = num_nodes;
    huff_ptr->array = (struct Huff_Node **)calloc(num_nodes, sizeof(struct Huff_Node *));
    return(huff_ptr);
}

struct Huff_Node *create_Node(unsigned int data, unsigned int num_pres)
{
    struct Huff_Node* huff_temp = (struct Huff_Node *)calloc(1, sizeof(struct Huff_Node));
    huff_temp->left = NULL;
    huff_temp->right = NULL;
    huff_temp->data = data;
    huff_temp->num_pres = num_pres;
    return(huff_temp);
}

struct Huff_Tree *build_tree(unsigned long int freqs[256][2], int size)
{
    struct Huff_Node *min_nodeA, *min_nodeB;
    struct Huff_Tree *huff_ptr = Huff_init(size);
    huff_ptr = fill_heap(huff_ptr,freqs,size);
    while(huff_ptr->size > 1) 
    {
        min_nodeA = find_min(huff_ptr);
        min_nodeB = find_min(huff_ptr);
        huff_ptr = buildAndInsertNode(huff_ptr,min_nodeA,min_nodeB);
    }
    return(huff_ptr);
}

struct Huff_Node *find_min(struct Huff_Tree *huff_ptr)
{
    struct Huff_Node *min = huff_ptr->array[0];
    remove_freq(huff_ptr);
    return(min);
}

void remove_freq(struct Huff_Tree *huff_ptr)
{
    for(int i = 1; i < huff_ptr->size; i++)
    {
        huff_ptr->array[i - 1] = huff_ptr->array[i];
    }
    huff_ptr->size--;
}

struct Huff_Tree *fill_heap(struct Huff_Tree *huff_ptr, unsigned long int freq[256][2], int size)
{
    int i = 0;
    while(i < size)
    {
        huff_ptr->array[i] = create_Node(freq[i][0], freq[i][1]);
        i++;
    }
    return(huff_ptr);
}

struct Huff_Tree *buildAndInsertNode(struct Huff_Tree *huff_ptr, struct Huff_Node *Node1, struct Huff_Node *Node2)
{
    struct Huff_Node *top_Node = create_Node('$',Node1->num_pres + Node2->num_pres);
    int i = 0, x = 0, loc = -1;

    ++huff_ptr->size;
    top_Node->left = Node1;
    top_Node->right = Node2;
    // Insert Node
    if(huff_ptr->size == 1) 
    {
        huff_ptr->array[0] = top_Node;
    }else 
    {
        for (i = 0; i < huff_ptr->size - 1; i++) 
	{
            if (top_Node->num_pres >= huff_ptr->array[i]->num_pres) 
	    {
                loc = i;
            }
        }
        for (x = huff_ptr->size - 1; x >= loc + 1; x--) 
	{
            if (x == loc + 1) 
	    {
                huff_ptr->array[x] = top_Node;
            } else 
	    {
                huff_ptr->array[x] = huff_ptr->array[x - 1];
            }
        }
    }
    return(huff_ptr);
}

int val_check(unsigned long int freq[256][2], int size, unsigned char val)
{
    for(int i = 0; i < size; i++)
    {
        if(freq[i][0] == val)
	{
            return(0);
        }
    }
    return(1);
}

void create_Table(struct Huff_Tree *huff_ptr, int size)
{
    table = (struct Table *)calloc(size, sizeof(struct Table));
    int array[size];
    struct Huff_Node *start = huff_ptr->array[0];
    // Inorder Traversal to determine codes
    inTraverse(start,array,0);
}

void inTraverse(struct Huff_Node *startNode, int array[], int i)
{
    if(startNode->right != NULL)
    {
        array[i] = 1;
        inTraverse(startNode->right, array, i + 1);
    }
    if(startNode->left == NULL && startNode->right == NULL)
    {
        fill_Table(table, array, startNode, table_idx, i);
        table_idx = table_idx + 1;
    }
    if(startNode->left != NULL)
    {
        array[i] = 0;
        inTraverse(startNode->left, array, i + 1);
    }
}

void fill_Table(struct Table *table_ptr, const int array[], struct Huff_Node *node, int j, int i)
{
    table_ptr[j].data = node->data;
    table_ptr[j].len = i;
    table_ptr[j].code = (int *)calloc(i, sizeof(int));
    for(int x = 0; x < i; x++)
    {
        table_ptr[j].code[x] = array[x];
    }
}
