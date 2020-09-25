#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>

typedef struct filterNode{
	char* operator;
	char* operand;
} filterNode;

typedef struct mapNode{
	char* operator;
	char* operand;
} mapNode;

typedef struct reduceNode{
	char* operator;
} reduceNode;

typedef struct fmrNode{
	int id;
	int dependencies[50];

	filterNode fNode;
	mapNode mNode;
	reduceNode rNode;

	char elements[50][50];
} fmrNode;

int readInput(char* fileName, fmrNode** root);
void insertElement(char elements[50][50], char* elem);

void displayAll(fmrNode* root);
void display(fmrNode node);

char* fmrNetwork(fmrNode* root, fmrNode* node);
char* fmrCompute(fmrNode* node);

int main(int argc, char* argv[]){
	if(argc < 2){
		printf("Insufficient parameters - Execute as follows: ./fmr <input_file>\nPlease try again.\n");
		exit(1);
	}

	fmrNode* root;

	int numberOfNodes = readInput(argv[1], &root);
	// displayAll(root);
	
	#ifdef LABTASK
		for(int i=0; i<numberOfNodes; i++){
			printf("\nNode %d: %s\n", i, fmrCompute(&root[i]));
		}
	#elif BONUS
		printf("\nBonus: %s\n", fmrNetwork(root, &root[0]));
	#endif
}

int readInput(char* fileName, fmrNode** root){
	FILE* fp;
	fp = fopen(fileName, "r");

	int numNodes;
	
	fscanf(fp, "%d\n", &numNodes);
	*root = (fmrNode*)malloc((numNodes+1) * sizeof(fmrNode));

	fmrNode* head = *root;

	char buf[255];
	char *tok;

	for(int i=0; i<numNodes; i++){
		memset(buf, '\0', sizeof(buf));
		fgets(buf, sizeof(buf), fp);
		
		head[i].id = i;

		tok = strtok(buf, ",");
		head[i].fNode.operator = (char*)malloc(strlen(tok)*sizeof(char));
		strcpy(head[i].fNode.operator, tok);

		tok = strtok(NULL, ",");
		head[i].fNode.operand = (char*)malloc(strlen(tok)*sizeof(char));
		strcpy(head[i].fNode.operand, tok);

		tok = strtok(NULL, ",");
		head[i].mNode.operator = (char*)malloc(strlen(tok)*sizeof(char));
		strcpy(head[i].mNode.operator, tok);

		tok = strtok(NULL, ",");
		head[i].mNode.operand = (char*)malloc(strlen(tok)*sizeof(char));
		strcpy(head[i].mNode.operand, tok);

		tok = strtok(NULL, ",");
		if(tok[strlen(tok)-1] == '\n') tok[strlen(tok)-1] = '\0';
		head[i].rNode.operator = (char*)malloc(strlen(tok)*sizeof(char));
		strcpy(head[i].rNode.operator, tok);
	}
	head[numNodes].id = -1;

	fscanf(fp, "\n");

	for(int i=0; i<numNodes; i++){
		memset(buf, '\0', sizeof(buf));
		fgets(buf, sizeof(buf), fp);

		tok = strtok(buf, ":");
		int counter = 0;
		
		head[i].dependencies[0] = -1;
		while(1){
			tok = strtok(NULL, ",");
			
			if(tok == NULL || tok[0] == '\n'){
				head[i].dependencies[counter] = -1;
				break;
			}else{
				int d = atoi(tok);
				head[i].dependencies[counter++] = d;
			}
		}
	}

	fscanf(fp, "\n");

	for(int i=0; i<numNodes; i++){
		memset(head[i].elements, '\0', 2500);
	
		memset(buf, '\0', sizeof(buf));
		fgets(buf, sizeof(buf), fp);

		tok = strtok(buf, ":");
		int counter = 0;
		
		strcpy(head[i].elements[0], "#");;
		while(1){
			tok = strtok(NULL, ",");
			
			if(tok == NULL || tok[0] == '\n'){
				strcpy(head[i].elements[counter], "#");
				break;
			}else{
				strcpy(head[i].elements[counter++], tok);
			}
		}
	}

	fclose(fp);

	return numNodes;
}

void insertElement(char elements[50][50], char* elem){
	int i;

	for(i=0; strcmp(elements[i], "#"); i++);

	strcpy(elements[i], elem);
	strcpy(elements[i+1], "#");
}

void displayAll(fmrNode* head){
	for(int i=0; head[i].id!=-1; i++){
		display(head[i]);
	}
}

void display(fmrNode node){
	printf("FMR Node %d:\n", node.id);
	printf("\tFilter: Operator(%s), Operand(%s)\n", node.fNode.operator, node.fNode.operand); 
	printf("\tMap: Operator(%s), Operand(%s)\n", node.mNode.operator, node.mNode.operand); 
	printf("\tReduce: Operator(%s)\n", node.rNode.operator); 
	
	printf("\tDependencies:\n\t\tFMR nodes: ");
	for(int x=0; node.dependencies[x]!=-1; x++){
		printf("%d, ", node.dependencies[x]);
	}
	printf("\n\tElements:\n\t\t");
	for(int x=0; strcmp(node.elements[x], "#"); x++){
		printf("%s, ", node.elements[x]);
	}
	printf("\n");
}

//LAB TASK
//fmrCompute function executes the ( echo-filter-map-reduce ) pipeline for a single node
char* fmrCompute(fmrNode* node){
	if(node->elements[0][0]=='#'){
		return "0";
	}

	pid_t pid_1, pid_2, pid_3, pid_4;

	int p1[2], p2[2], p3[2], p4[2];

	//STEP 1
	//Initialize pipes p1, p2, p3 and p4
	pipe(p1);
	pipe(p2);
	pipe(p3);
	pipe(p4);
	
	//STEP 2
	//Create first child process with fork and store the return value in pid_1
	pid_1 = fork();

	//STEP 3 - Condition for first child
	
	if(pid_1 == 0){		
		
		//STEP 4
		//In this first child process, we want to send everything that is printed on the standard output, to the next child process through pipe p1
		//So, redirect standard output of this child process to p1's write end
		//And, close all other pipe ends except the ones used to redirect I/O (very important)
		dup2(p1[1], STDOUT_FILENO);
		close(p1[0]);
		close(p2[0]);
		close(p2[1]);
		close(p3[0]);
		close(p3[1]);
		close(p4[0]);
		close(p4[1]);

		char argv[255];
		memset(argv, '\0', sizeof(argv));
		for(int i=0; strcmp(node->elements[i], "#"); i++){
			strcat(argv, node->elements[i]);
			strcat(argv, "\n");
		}

		//STEP 5
		//Execute the /bin/echo program with argv as argument
		execl("/bin/echo", "/bin/echo", argv, (char*)NULL);

		exit(0);
	}
	
	
	
	//STEP 6
	//Create second child process with fork and store the return value in pid_2
	pid_2 = fork();

	//STEP 7 - Condition for second child
	if(pid_2 == 0){		
	
		//STEP 8
		//In this second child process, the output from the first child process (/bin/echo) will be taken as input
		//So, redirect the necessary pipe end to standard input
		//Also, the output from this second child process will be sent to the next child process through pipe p2
		//So, redirect the standard output to the necessary pipe end
		//And, close all other pipe ends except the ones used to redirect I/O (very important)
		dup2(p1[0], STDIN_FILENO);
		dup2(p2[1], STDOUT_FILENO);
		close(p1[1]);
		close(p2[0]);
		close(p3[0]);
		close(p3[1]);
		close(p4[0]);
		close(p4[1]);


		//STEP 9
		//Execute the ./filter program with the necessary arguments from node
		//node is an fmrNode* type (please check the structure definitions at the top to find out how we can access the filter arguments using fmrNode* type)
		execl("./filter","./filter", node->fNode.operator, node->fNode.operand, (char*)NULL);

		exit(0);
	}

	//STEP 10
	//Create third child process with fork and store the return value in pid_3
	pid_3 = fork();

	//STEP 11 - Condition for third child
	if(pid_3 == 0){		
	
		//STEP 12
		//In this third child process, the output from the second child process (./filter) will be taken as input
		//So, redirect the necessary pipe end to standard input
		//Also, the output from this third child process will be sent to the next child process through pipe p3
		//So, redirect the standard output to the necessary pipe end
		//And, close all other pipe ends except the ones used to redirect I/O (very important)
		dup2(p2[0], STDIN_FILENO);
		dup2(p3[1], STDOUT_FILENO);
		close(p1[0]);
		close(p1[1]);
		close(p2[1]);
		close(p3[0]);
		close(p4[0]);
		close(p4[1]);


		//STEP 13
		//Execute the ./map program with the necessary arguments from node
		//node is an fmrNode* type (please check the structure definitions at the top to find out how we can access the map arguments using fmrNode* type)
		execl("./map","./map", node->mNode.operator, node->mNode.operand, (char*)NULL);
		
		exit(0);
	}

	//STEP 14
	//Create fourth child process with fork and store the return value in pid_4
	pid_4 = fork();

	//STEP 15 - Condition for fourth child
	if(pid_4 == 0){		
	
		//STEP 16
		//In this fourth child process, the output from the third child process (./map) will be taken as input
		//So, redirect the necessary pipe end to standard input
		//Also, the output from this fourth child process will be sent to the parent process through pipe p4
		//So, redirect the standard output to the necessary pipe end
		//And, close all other pipe ends except the ones used to redirect I/O (very important)
		dup2(p3[0], STDIN_FILENO);
		dup2(p4[1], STDOUT_FILENO);
		close(p1[0]);
		close(p1[1]);
		close(p2[0]);
		close(p2[1]);
		close(p3[1]);
		close(p4[0]);


		//STEP 17
		//Execute the ./reduce program with the necessary arguments from node
		//node is an fmrNode* type (please check the structure definitions at the top to find out how we can access the reduce arguments using fmrNode* type)
		execl("./reduce","./reduce", node->rNode.operator, (char*)NULL);

		exit(0);
	}

	//STEP 18
	//In this parent process scope, the final output from the fourth child process will eventually be taken as input
	//Close all other pipe ends here, except pipe p4, since p4 will be used to read in the output of the fourth child process
	//Closing all other pipe ends is crucial
	close(p1[0]);
	close(p1[1]);
	close(p2[0]);
	close(p2[1]);
	close(p3[0]);
	close(p3[1]);

	//STEP 19
	//Wait for all the four child processes here
	//The wait is necessary because we want to get a synchronous output from the four child processes
	//Also, we don't want to end up with orphaned processes; so, the waits are essential
	waitpid(pid_1, NULL, 0);
	waitpid(pid_2, NULL, 0);
	waitpid(pid_3, NULL, 0);
	waitpid(pid_4, NULL, 0);

	char* buf = (char*)malloc(25*sizeof(char*));
	memset(buf, '\0', 25);
	//STEP 20
	//Use the read system call to read the final output of the ( echo-filter-map-reduce ) pipeline, from p4 and store the read value into buf
	read(p4[0], buf, sizeof(buf));

	//STEP 21
	//Close p4 (very important)
	close(p4[0]);
	close(p4[1]);

	return buf;
}

// //EXTRA CREDIT (attempt only when you have a working fmrCompute function)
// //fmrNetwork creates a hierarchical tree of ( echo-filter-map-reduce ) pipelines
// char* fmrNetwork(fmrNode* root, fmrNode* currentNode){
// 	int p0[2];

// 	//EXTRA CREDIT - STEP 1
// 	//Initialize the pipe p0
	

// 	//We will visit every child node from the current node (according to the current node's dependencies)
// 	//For every child node, we will create a child process
// 	for(int i=0; currentNode->dependencies[i]!=-1; i++){		
// 		pid_t pid_fmr;
		
// 		//EXTRA CREDIT - STEP 2
// 		//Create a child process for the ith child node, with fork and store the return value into pid_fmr

// 		if(/*EXTRA CREDIT - STEP 3 - Check condition for child process*/){

// 			//EXTRA CREDIT - STEP 4
// 			//Here, we are inside a process that is dedicated for the ith child of the current node
// 			//Now, this ith child node can also have child nodes of its own
// 			//So, recursively call the fmrNetwork function with the appropriate arguments and store the return value into fmrVal
// 			//This return value represents the final output of fmrCompute for all descendent nodes starting from this ith child
// 			char* fmrVal = "Replace this string by the recursive fmrNetwork call and the return value will go into fmrVal";


// 			//EXTRA CREDIT - STEP 5
// 			//Use the write system call to send the fmrVal from the ith child node to its parent (i.e. currentNode) through pipe p0

// 			//EXTRA CREDIT - STEP 6
// 			//Close pipe p0 inside this ith child process
			
// 			exit(0);
// 		}
// 	}

// 	//EXTRA CREDIT - STEP 7
// 	//We are out of the scope of the for loop which means that we are out of the scope of currentNode's children nodes
// 	//So, we are now in the scope of currentNode itself
// 	//currentNode will eventually "read" the fmrVals that were sent by its children through pipe p0
// 	//So, we don't need the read end of pipe p0 - close the write end of pipe p0 here

// 	char val[50];
// 	memset(val, '\0', sizeof(val));
	

// 	//There will be as many fmrVals inside the pipe p0 as the number of currentNode's children
// 	while(/*EXTRA CREDIT - STEP 8 - read from p0 and store into val*/){
// 		//printf("\nFMR Node %d - FMR Value: %s\n", currentNode->id, val);

// 		insertElement(currentNode->elements, val);
// 	}

// 	//EXTRA CREDIT - STEP 9
// 	//Since reading from the pipe p0 is finished, you should close the read end of pipe p0 here

	
// 	// display(*currentNode) //You can use display to print the currentNode (useful for debugging)


// 	//EXTRA CREDIT - STEP 10
// 	//At this point, we have resolved the currentNode's dependencies (i.e. inserted the fmrVals from currentNode's children, into the currentNode's elements array)
// 	//So, with a completed elements array of the current node, we can now compute the ( echo-filter-map-reduce ) pipeline for the "currentNode"
// 	char* fmrVal = "Replace this string by an fmrCompute call for the currentNode and the return value will be stored into fmrVal";


// 	//Here, we return the fmrVal of the currentNode, to the parent node of the currentNode 
// 	//This value is what we used as a return value of the recursive fmrNetwork call inside the for loop above (for the parent node of the currentNode)
// 	return fmrVal;
// }