// Made by Chance Penner and Haonan Hu
#define _GNU_SOURCE
#include <fcntl.h>
#include <sys/wait.h>
#include <stdio.h>     /* standard I/O functions                         */
#include <stdlib.h>    /* exit                                           */
#include <string.h>    /* memset                                         */
#include <stdbool.h>   /* using bool data type			                     */
#include <unistd.h>    /* standard unix functions, like getpid()         */
#include <signal.h>    /* signal name macros, and the signal() prototype */
#include <sys/types.h> /*needed for pid_t                                */
#include <dirent.h>
#include <errno.h>
#include <stdbool.h>

#define MAX_LENGTH 500
#define  BUFFER_SIZE 24


struct Process
{
  int jobId;
  int pid;
  char* command;
};

static int jobsTotal;
static struct Process jobsList[MAX_LENGTH];
static pid_t processId;
static char* envVars[MAX_LENGTH];
static int envSize;

//forward declaration
void runProgram(char** prog, char* workingDir);
void execute(char path[500][500], int numPaths, char** cmd);

void cyan()
{
  printf("\033[0;36m");
}

void red()
{
  printf("\033[0;31m");
}

void reset()
{
  printf("\033[0m");
}

void parseInput(char* input, char** inputArgs)
{

  int it = 0;

  char *parsed = strtok(input, " \n\t");

  while (parsed != NULL)
    {
      inputArgs[it] = parsed;
      // printf("parsed: %d %s\n", it, parsed);

      parsed = strtok(NULL, " \n\t");
      it++;
    }

}

void redirect(char* input, char* workingDir, char path[500][500], int numPaths)
{

  char* left[100];
  char* right[100];
  char* leftParsed;
  char* rightParsed;

  memset(left, 0, sizeof(left));
  memset(right, 0, sizeof(right));



  int redirectIndex = 0;
  char* redirectSymbol;

  for (size_t i = 0; i < strlen(input); i++)
  {
    if(input[i] == '<')
    {
      redirectIndex = i;
      redirectSymbol = "<";
      break;
    }
    else if(input[i] == '>')
    {
      redirectIndex = i;
      redirectSymbol = ">";
      break;
    }
  }

  //becomes what's to the left of the redirectSymbol
  leftParsed = strtok(input, redirectSymbol);
  rightParsed = strtok(NULL, " \n\t");


  // printf("\nleftParsed: %s\n", leftParsed);
  // printf("rightParsed: %s\n", rightParsed);

  parseInput(leftParsed, left);
  parseInput(rightParsed, right);

  if(strcmp(redirectSymbol, "<") == 0)
  {
    //redirect standard input

    if(strncmp(left[0], "./", 2) == 0)
    {
      //if running with ./
      // int o = dup(fileno(stdin));
      // freopen(right[0],"r",stdin);
      // runProgram(left, workingDir);
      // dup2(o,fileno(stdin));
      // close(o);

      int fd0 = open(right[0], O_RDONLY);
      dup2(fd0, STDIN_FILENO);
      runProgram(left, workingDir);
      close(fd0);
      
      }
    else
    {
      //running a terminal command
      // int o = dup(fileno(stdin));
      // freopen(right[0],"r",stdin);
      // execute(path, numPaths, left);
      // dup2(o,fileno(stdin));
      // close(o);

      int fd0 = open(right[0], O_RDONLY);
      dup2(fd0, STDIN_FILENO);
      execute(path, numPaths, left);
      close(fd0);
    }

  }
  else if(strcmp(redirectSymbol, ">") == 0)
  {
    //redirect standard output

    if(strncmp(left[0], "./", 2) == 0)
    {
      //if running with ./
      int o = dup(fileno(stdout));
      freopen(right[0],"w",stdout);
      runProgram(left, workingDir);
      dup2(o,fileno(stdout));
      close(o);
    }
    else
    {
      //running a terminal command
      int o = dup(fileno(stdout));
      freopen(right[0],"w",stdout);
      execute(path, numPaths, left);
      dup2(o,fileno(stdout));
      close(o);
    }
  }

}

void makePipe(char* input, char* workingDir, char path[500][500], int numPaths)
{
  char* left[100];
  char* right[100];
  char* leftParsed;
  char* rightParsed;

  memset(left, 0, sizeof(left));
  memset(right, 0, sizeof(right));



  int redirectIndex = 0;
  char* redirectSymbol;

  for (size_t i = 0; i < strlen(input); i++)
  {
    if(input[i] == '|')
    {
      redirectIndex = i;
      redirectSymbol = "|";
      break;
    }
  }


  //becomes what's to the left of the redirectSymbol
  leftParsed = strtok(input, redirectSymbol);
  rightParsed = strtok(NULL, "\n\t");


  // printf("\nleftParsed: %s\n", leftParsed);
  // printf("rightParsed: %s\n", rightParsed);

  parseInput(leftParsed, left);
  parseInput(rightParsed, right);


  pid_t pid_1;
  int p1[2];
  //Initialize pipe p1
  pipe(p1);

  //Create first child process with fork and store the return value in pid_1
  pid_1 = fork();

  if(pid_1 == 0)
  {
    //child process
    dup2(p1[1], STDOUT_FILENO);
    close(p1[0]); //close the read end of the pipe

    if(strncmp(left[0], "./", 2) == 0)
    {
      runProgram(left, workingDir);
    }
    else
    {
      execute(path, numPaths, left);
    }
    exit(0);
  }
  else if(pid_1 > 0)
  {
    waitpid(pid_1,NULL,0);

    dup2(p1[0], STDIN_FILENO);
    close(p1[1]);
    if(strncmp(left[0], "./", 2) == 0)
    {
      runProgram(right, workingDir);
    }
    else
    {
      execute(path, numPaths, right);
    }
  }
  else
  {
    //failure
    fprintf(stderr, "Error\n");
    exit(-1);
  }


}

void removeJob(pid_t removePid)
{
  //-1 pid indicates a completed process
  for (size_t i = 1; i < jobsTotal; i++)
  {
    if(jobsList[i].pid == removePid)
    {
      jobsList[i].pid = -1;
      return;
    }
  }
}

void backgroundExecute(char* input, char* workingDir, char path[500][500], int numPaths)
{
  char* inputArgs[100];
  char* originalInput = malloc(strlen(input) + 1); // +1 for the null-terminator


  memset(inputArgs, 0, sizeof(inputArgs));

  strcpy(originalInput, input);

  parseInput(input, inputArgs);

  for (size_t i = 0; i < sizeof(inputArgs); i++)
  {
    if(strcmp(inputArgs[i], "&") == 0)
    {
      inputArgs[i] = NULL;
      break;
    }
  }

  jobsList[jobsTotal].command = strtok(originalInput, "&");
  strcat(jobsList[jobsTotal].command, "&");



  pid_t pid_1;

  pid_1 = fork();
  processId = pid_1;

  if(pid_1 == 0)
  {
    //child process
    printf("\n[%d] %d running in background\n", jobsTotal, getpid());

    if(strncmp(inputArgs[0], "./", 2) == 0)
    {
      runProgram(inputArgs, workingDir);
    }
    else
    {
      execute(path, numPaths, inputArgs);
    }

    printf("[%d] %d finished %s\n", jobsTotal, getpid(), jobsList[jobsTotal].command);


    exit(0);
  }
  else if(pid_1 > 0)
  {
    //parent process
    jobsList[jobsTotal].pid = pid_1;
    jobsList[jobsTotal].jobId = jobsTotal;


    // printf("command: %s\n", jobsList[jobsTotal].command);
    // printf("pid: %d\n", jobsList[jobsTotal].pid);
    // printf("jobId: %d\n", jobsList[jobsTotal].jobId);

    jobsTotal++;
    // sleep(1);

  }
  else
  {
    //failure
    fprintf(stderr, "Error\n");
    exit(-1);
  }





}

void getJobs()
{
  //need to check if any background processes finished so we can remove them before printing them
  pid_t childPid=waitpid(processId, NULL, WNOHANG);
  if(processId == childPid)
  {
    //remove the newly executed background process
    removeJob(childPid);
  }

  for (size_t i = 1; i < jobsTotal; i++)
  {
    if(jobsList[i].pid != -1)
    {
      //process is still running in background
      printf("[%d] %d %s\n", jobsList[i].jobId, jobsList[i].pid, jobsList[i].command);
    }
  }
}

void execute(char path[500][500], int numPaths, char** cmd)
{
  bool executableFound = false;
  char* finalPath;
  char* fullPath;
  for (size_t k = 0; k < numPaths; k++)
  {
    // printf("\n[%zu]:%s\n", k, path[k]);

    //not full path
    if(cmd[0][0] != '/')
    {
      //cmd[0] represents the command typed, minus the flags
      char *fullPath = malloc(strlen(path[k]) + strlen(cmd[0]) + 2); // +1 for the null-terminator and +1 for '/'
      strcpy(fullPath, path[k]);
      // fullPath = "\0";
      // fullPath = strdup(path[k]);
      strcat(fullPath,"/");
      strcat(fullPath, cmd[0]);
      finalPath = fullPath;
    }
    else
    {
      finalPath = cmd[0];
    }

    // printf("\nfinalPath:%s", finalPath);


    int size = 0;
    while(cmd[size] != NULL)
    {
      size++;
    }

    // printf("\nSIZE: %d\n", size);


    if((access(finalPath, X_OK ) != -1 ) && !executableFound)
    {
      executableFound = true;
        // executable exists
        char *tempArgs[size+1];
        for (size_t i = 0; i < size; i++) {
          tempArgs[i] = cmd[i];
        }
        tempArgs[size] = NULL;


        //if there are args/flags
        if(size > 1)
        {
          pid_t pid;
          pid = fork();
          if(pid == 0)
          {
            //child process
            // printf("\nEXECUTING EXECVP\n");
            execvpe(finalPath, tempArgs, envVars);
            exit(0);
          }
          else if(pid > 0)
          {
            //parent process
            waitpid(pid, NULL, 0);
          }
          else
          {
            //failure
            fprintf(stderr, "Error\n");
            exit(-1);
          }
        }
        else
        {
          //there are no args/flags
          pid_t pid;
          pid = fork();
          if(pid == 0)
          {
            //child process
            // printf("\nEXECUTING EXECLP\n");
            // execlp(finalPath, finalPath, NULL);


            execle(finalPath, finalPath, NULL, envVars);

            exit(0);
          }
          else if(pid > 0)
          {
            //parent process
            waitpid(pid, NULL, 0);
          }
          else
          {
            //failure
            fprintf(stderr, "Error\n");
            exit(-1);
          }
        }

    }

  }
  if(!executableFound)
  {
    printf("command not found: %s\n", cmd[0]);
  }
}



void runProgram(char** prog, char* workingDir)
{
  //prog[0] + 1 will be '/prog' instead of './prog'
  char* finalPath = malloc(strlen(workingDir) + (strlen(prog[0]+1)) + 1);  //allocate space for program and null terminator
  strcat(finalPath, workingDir);
  strcat(finalPath, prog[0]+1);

  pid_t pid;
  pid = fork();

  if(pid == 0)
  {
    //child process


    if(prog[1] == NULL)
    {
      //No arguments
      // printf("finalPath: %s\n", finalPath);
      execle(finalPath, prog[0]+2, NULL, envVars);
      printf("No such file or directory\n");

      exit(0);
    }
    else
    {
      //program with arguments
      int size = 0;
      while(prog[size] != NULL)
      {
        size++;
      }

      char *tempArgs[size+1];
      for (size_t i = 0; i < size; i++) {
        if(i == 0)
        {
          //need to only pass in prog name, not the './'
          tempArgs[i] = prog[i]+2;
        }
        else
        {
          tempArgs[i] = prog[i];
        }
      }
      tempArgs[size] = NULL;

      //there are arguments
      // printf("finalPath: %s\n", finalPath);
      execvpe(finalPath, tempArgs, envVars);
      printf("No such file or directory\n");
      exit(0);
    }
  }
  else if(pid > 0)
  {
    //parent process
    waitpid(pid, NULL, 0);
  }
  else
  {
    //failure
    fprintf(stderr, "Error\n");
    exit(-1);
  }

}

int main(int argc, char **argv, char **envp)
{
  //first, copy over initial envp into our envVars
  envSize = 0;
  // *envVars = malloc(MAX_LENGTH);

  for (size_t i = 0; i < sizeof(envp); i++)
  {
    // strcpy(envVars[i], envp[i]);
    envVars[i] = strdup(envp[i]);
    // printf("envp[%zu]:%s\n",i, envp[i]);
    // printf("envVars[%zu]:%s\n", i,envVars[i]);

    envSize++;
  }

  char cwd[MAX_LENGTH];
  getcwd(cwd, sizeof(cwd));
  char input[MAX_LENGTH]; //input string
  char* inputArgs[100]; //input args

  char home[1000];
  strcpy(home,getenv("HOME"));
  char path[500][MAX_LENGTH]; //gets all typed in paths, whether good or bad
  char tempPath[1000];
  strcpy(tempPath, getenv("PATH"));
  int numPaths = 0;
  char workingDir[MAX_LENGTH];
  // char buf[BUFFER_SIZE];
  jobsTotal = 1; //initialize jobs to 1


  if(argc > 1)
  {
    red();
    printf("\nToo many arguments!\n\n");
    reset();
    return 1;
  }


  strcpy(workingDir, home);
  chdir(workingDir);



  // //set working directory and make sure it is valid
  // DIR* dir = opendir(home);
  // if(strcmp(home, "./" ) == 0)
  // {
  //
  //   //sets the workingDir to the current directory when calling quash
  //   getcwd(workingDir, sizeof(workingDir));
  //   //update home
  //   home = malloc(strlen(workingDir) + 1); // +1 for the null-terminator and +1 for '/'
  //   strcpy(home, workingDir);
  //
  //
  //   chdir(workingDir);
  //   printf("working dir: %s\n", workingDir);
  // }
  // else if(dir)
  // {
  //   strcpy(workingDir, home);
  //   chdir(workingDir);
  // }
  // else if (ENOENT == errno)
  // {
  //     /* Directory does not exist. */
  //     printf("The home directory does not exist: %s \n", home);
  //     return 0;
  // }
  // else
  // {
  //     /* opendir() failed for some other reason. */
  // }

  //only one path
  if(strstr(tempPath,":") == NULL)
  {

    strcpy(path[0], tempPath);
    numPaths++;
  }
  else
  {
    char* split;
    split = strtok(tempPath, ":");
    while(split != NULL)
    {
      strcpy(path[numPaths], split);
      numPaths++;
      split = strtok(NULL, ":");
    }
  }




  for (size_t i = 0; i < numPaths; i++)
  {
    // DIR* dir = opendir(path[i]);
    //
    if(strcmp(path[i], "./" ) == 0)
    {
      //sets the path to the current directory when calling quash
      getcwd(path[i], sizeof(path[i]));
    }
    // else if (ENOENT == errno)
    // {
    //     /* Directory does not exist. */
    //     red();
    //     printf("\nThe path directory does not exist: %s\n", path[i]);
    //     reset();
    //     strcpy(path[i], "\0");
    // }
    // else
    // {
    //     /* opendir() failed for some other reason. */
    // }


    if(strcmp(path[i], "./" ) == 0)
    {
      //sets the path to the current directory when calling quash
      getcwd(path[i], sizeof(path[i]));
    }

  }

  while(true)
  {
    
  
    pid_t childPid=waitpid(processId, NULL, WNOHANG);
    if(processId == childPid)
    {
      //remove the newly executed background process
      removeJob(childPid);
    }
    // printf("\n\nchildPid: %d\n\n", childPid);



    //clear out the inputArgs
    memset(inputArgs, 0, sizeof(inputArgs));
    memset(input, 0, strlen(input));

    

    char cwd[1024];
    getcwd(cwd, sizeof(cwd));

    // red();
    // printf("quash:%s\n", cwd);
    // reset();

    //if the input is STDIN_FILENO (so we are reading user input, not from a file)
    if(isatty(STDIN_FILENO))
    {
      cyan();
      printf("quash:%s ʕ•́ᴥ•̀ʔっ ", workingDir);
      reset();
    }

    //with fgets, there is an added '\n' at the end, which increases the length by 1,
    //so this else statement gets rid of that

    //if fgets == NULL, that means it reached the end of file
    //so that means we did an input redirect and reached the end of file
    if (fgets(input, sizeof(input), stdin) == NULL)
    {
        // printf("end of file: %s\n", input);
        // printf("Fail to read the input stream\n");
        // printf("\n");

        // int o = dup(fileno(stdin));
        // freopen("quash","r",stdin);
        // dup2(o,fileno(stdin));
        // close(o);
        freopen("/dev/tty", "rw", stdin);
        continue;

        // return 0;
    }
    else
    {
        input[strlen(input) - 1] = '\0';
    }

  

    // printf("input:%s\n",input);


    if(strcmp(input, "cd ./") == 0)
    {
      //do nothing
    }
    else if(strcmp(input, "cd .") == 0)
    {
      //do nothing
    }
    else if(strcmp(input, "cd ..") == 0)
    {


      //points to the last '/' char in the currentDir
      char* lastSlash = strrchr(workingDir, '/');
      //removes the last directory, hence goes back one
      lastSlash[0] = '\0';
      if(workingDir[0] == '\0')
      {
        //went to the beginning directory, so workingDir would be empty
        //change workingDir to the '/' directory, which is the beginning
        strcpy(workingDir,"/");
      }
      chdir(workingDir);

    }
    else if((strncmp(input, "cd", 2) == 0) && (strlen(input) > 3))
    {

      //input+3 changes the pointer to the directory typed after cd
      //e.g. 'cd User/Desktop', then input+3 points to 'U' in 'User/Desktop' so that
      //only that part is copied
      char* tempDir = input+3;
      //this will be just the new dir e.g. 'cd Project1', newDir becomes '/Project1'

      DIR* dir = opendir(tempDir);
      if(dir)
      {
        if(tempDir[0] == '/')
        {
          strcpy(workingDir, tempDir);
        }
        else
        {
          char *newDir = malloc(strlen(workingDir) + strlen(tempDir) + 2); // +1 for the null-terminator and +1 for '/'
          strcpy(newDir, workingDir);
          //if not in the '/' directory, need to add '/' characters
          if(!(strcmp(workingDir,"/") == 0))
          {
            strcat(newDir,"/");
          }
          strcat(newDir, tempDir);
          strcpy(workingDir, newDir);
        }
        chdir(workingDir);
      }
      else if (ENOENT == errno)
      {
          /* Directory does not exist. */
          printf("cd: %s: No such file or directory\n", tempDir);
      }
      else
      {
          /* opendir() failed for some other reason. */
      }

    }
    else if(strncmp(input, "cd", 2) == 0)
    {
      strcpy(workingDir, home);
      chdir(home);
    }
    else if((strcmp(input, "quit") == 0) || (strcmp(input, "exit") == 0))
    {
      return 0;
    }
     else if((strncmp(input, "quit", 4) == 0) || (strncmp(input, "exit", 4) == 0))
    {
      return 0;
    }
    else if((strstr(input, "<") != NULL) || (strstr(input, ">") != NULL))
    {
      //redirect standard output
      redirect(input, workingDir, path, numPaths);
    }
    else if((strstr(input, "|") != NULL))
    {
      //create pipe
      makePipe(input, workingDir, path, numPaths);
    }
    else if((strstr(input, "&") != NULL))
    {
      //run process in the background
      backgroundExecute(input, workingDir, path, numPaths);
    }
    else if((strcmp(input, "jobs") == 0))
    {
      //print all background jobs
      getJobs();
    }
    else if(strncmp(input, "./", 2) == 0)
    {
      //run an executable
      parseInput(input, inputArgs);
      runProgram(inputArgs, workingDir);
    }

    else if(strlen(input) == 0)
    {
      //no input, so do nothing
    }
    else if(strcmp(input, "printenv PATH") == 0)
    {
      char tempPrintPath[MAX_LENGTH];
      memset(tempPrintPath, 0, sizeof(tempPrintPath));

      for (size_t i = 0; i < numPaths; i++)
      {
        strcat(tempPrintPath, path[i]);
        if(i != numPaths-1)
        {
          strcat(tempPrintPath, ":");
        }
      }

      printf("%s\n", tempPrintPath);
    }
    else if(strcmp(input, "printenv HOME") == 0)
    {
      printf("%s\n", home);
    }
    else if(strncmp(input, "set HOME=", 9) == 0)
    {
      //set home directory and make sure it is valid
      // printf("input+9: %s\n", input+9);
      DIR* tempHomeDir = opendir(input+9);
      if(strcmp(input+9, "./" ) == 0)
      {
        //update home
        strcpy(home, workingDir);
      }
      else if(tempHomeDir)
      {
        strcpy(home, input+9);
      }
      else if (ENOENT == errno)
      {
          /* Directory does not exist. */
          printf("The home directory does not exist: %s \n", input+9);
      }
      else
      {
          /* opendir() failed for some other reason. */
      }
    }
    else if(strncmp(input, "set PATH=", 9) == 0)
    {
      char userPath[MAX_LENGTH][MAX_LENGTH];
      memset(userPath, 0, sizeof(userPath));
      
      // char** userPath = malloc(sizeof(MAX_LENGTH));

      int numUserPaths = 0;
      bool allPathsValid = true;
      //set PATH. If invalid, remove it from path

      // clear current path
      // for (size_t i = 0; i < numPaths; i++)
      // {
      //   strcpy(path[i], "\0");
      // }
      // numPaths = 0;


      //if only one path
      if(strstr(input+9,":") == NULL)
      {
        // printf("%s\n",userPath[0] );
        strcpy(userPath[0], input+9);
        numUserPaths++;
      }
      else
      {
        char* split;
        split = strtok(input+9, ":");
        while(split != NULL)
        {
          strcpy(userPath[numUserPaths], split);
          numUserPaths++;
          split = strtok(NULL, ":");
        }
      }


      // for (size_t i = 0; i < numUserPaths; i++)
      // {
      //   DIR* dir = opendir(userPath[i]);
      //   // printf("[%zu]%s\n", i, userPath[i]);

      //   if(strcmp(userPath[i], "./" ) == 0)
      //   {
      //     //sets the path to the current directory when calling quash
      //     getcwd(userPath[i], sizeof(userPath[i]));
      //   }
      //   else if(dir)
      //   {
      //     //if directory exists
      //     //do nothing
      //   }
      //   else if (ENOENT == errno)
      //   {
      //       /* Directory does not exist. */
      //       red();
      //       printf("\nThe path directory does not exist: %s \n", userPath[i]);
      //       reset();
      //       strcpy(userPath[i], "\0");
      //       // allPathsValid = false;
      //   }
      // }

      memset(path, 0, sizeof(path));
      numPaths = 0;
      for (size_t i = 0; i < numUserPaths; i++)
      {
        strcpy(path[numPaths], userPath[i]);
        numPaths++;
      }
      // for(size_t i=0;i<numUserPaths;i++)
      // {
      //   printf("[%zu]:%s\n", i, userPath[i]);
      // }
        // numPaths = numUserPaths;

      // if(allPathsValid)
      // {
      //   //clear current path
      //   for (size_t i = 0; i < numUserPaths; i++)
      //   {
      //     strcpy(path[i], userPath[i]);
      //   }
      //   numPaths = numUserPaths;
      // }
      // else
      // {
      //   // red();
      //   printf("\nPATH not changed\n");
      //   // reset();
      // }

    }
    else if((strncmp(input, "set ", 4) == 0) && (strstr(input, "=") != NULL))
    {
      // printf("in set\n");
      char* varName = strtok(input, "=") + 4;
      char* varValue = strtok(NULL, " \n\t");
      char *nameAndValue = malloc(strlen(varName) + strlen(varValue) + 2); // +1 for the null-terminator and +1 for '='
      strcat(nameAndValue, varName);
      strcat(nameAndValue, "=");
      strcat(nameAndValue, varValue);

      //add the new environment variable to envVars
      // strcpy(envVars[envSize], nameAndValue);
      envVars[envSize] = strdup(nameAndValue);
      envSize++;

      // for (size_t i = 0; i < envSize; i++) {
      //   printf("%s\n", envVars[i]);
      // }
      // printf("exiting set\n");

    }
    else
    {
      //assuming system call

      parseInput(input, inputArgs);
      execute(path, numPaths, inputArgs);


    }




  }
  return 0;
}
