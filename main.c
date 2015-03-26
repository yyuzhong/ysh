#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define YSH_TOKENS_BUFSIZE 64
#define YSH_TOKENS_DELIM " \t\r\n\a"
#define YSH_COMMANDS_SIZE 32
#define YSH_COMMANDS_DELIM ";"

#define YSH_LINE_BUFSIZE 1024

int ysh_cd(char **args);
int ysh_quit(char **args);

char *builtin_str[] = {
  "quit",
  "cd"
};

int (*builtin_func[]) (char **) = {
  &ysh_quit,
  &ysh_cd
};

int ysh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

int ysh_cd(char **args)
{
  //printf("Run built in cd command\n");
  if (args[1] == NULL) {
    fprintf(stderr, "ysh: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("ysh");
    }
  }
  return 1;
}

int ysh_quit(char **args)
{
  //printf("Run built in quit command\n");
  return 0;
}

char *ysh_read_line(void)
{
  int bufsize = YSH_LINE_BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;

  if (!buffer) {
    fprintf(stderr, "ysh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    // Read a character
    c = getchar();

    // If we hit EOF, replace it with a null character and return.
    if (c == EOF || c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }
    position++;

    // If we have exceeded the buffer, reallocate.
    if (position >= bufsize) {
      bufsize += YSH_LINE_BUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
        fprintf(stderr, "ysh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}

char **ysh_split_command(char *line)
{
  int bufsize = YSH_TOKENS_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token, **tokens_backup;
  //printf("split command:%s\n",line);
  if (!tokens) {
    fprintf(stderr, "ysh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, YSH_TOKENS_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += YSH_TOKENS_BUFSIZE;
      tokens_backup = tokens;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
		free(tokens_backup);
        fprintf(stderr, "ysh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, YSH_TOKENS_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}

int ysh_run_commands(char *line)
{
  int comm_size = YSH_COMMANDS_SIZE, comm_num = 0;
  int i,j;
  char **comms = malloc(comm_size * sizeof(char*));
  char *tmp_comm,**comms_backup;
  char *quit_comm = NULL;// "quit";
  int ret = 1;
  //printf("Run one line command:%s\n",line);
  if (!comms) {
    fprintf(stderr, "ysh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  tmp_comm = strtok(line, YSH_COMMANDS_DELIM);
  while (tmp_comm != NULL) {
    //printf("Get one command:%s\n",tmp_comm);
    if(strcmp(tmp_comm,builtin_str[0]) == 0)
    {
      quit_comm = builtin_str[0];
    } else {
      comms[comm_num] = tmp_comm;
      comm_num++;
      if (comm_num >= comm_size) {
        comm_size += YSH_COMMANDS_SIZE;
        comms_backup = comms;
        comms = realloc(comms, comm_size * sizeof(char*));
        if (!comms) {
	  free(comms_backup);
          fprintf(stderr, "ysh: allocation error\n");
          exit(EXIT_FAILURE);
        }
      }
    }
    tmp_comm = strtok(NULL, YSH_COMMANDS_DELIM);
  }
  comms[comm_num] = quit_comm;

  for(i=0; i<=comm_num; i++)
  {
    char *one_comm = comms[i];
    char **args = ysh_split_command(one_comm);
    pid_t pid;
    int status;
    int built_in = 0;
    //printf("Run command: %s\n", args[0]);
    if (args[0] == NULL) {
      continue;    
    }
    for (j = 0; j < ysh_num_builtins(); j++) {
      if (strcmp(args[0], builtin_str[j]) == 0) {
        built_in = 1;
        break;
      }
    }
    if(built_in)
    {
       //printf("Run builtin command: %s\n",args[0]);
       status = (*builtin_func[j])(args);
       if(status == 0 ) ret = status;
    } else {
      //printf("Run normal command: %s\n",args[0]);
      pid = fork();
      if (pid == 0) {
        // Child process
        if (execvp(args[0], args) == -1) {
          perror("ysh");
        }
        exit(EXIT_FAILURE);
      } else if (pid < 0) {
        // Error forking
        perror("ysh");
      } else {
        // Parent process
        do {
          waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
      }
    }
  }
  return ret;
}

void ysh_loop(void)
{
  char *line;
  int status;

  do {
    printf("ysh> ");
    line = ysh_read_line();
    status = ysh_run_commands(line);
    free(line);
  } while (status);
}

void ysh_run_file(char *file_name)
{
  FILE * fp;
  char * line = NULL;
  size_t len = 0;
  ssize_t read;
  int status;

  fp = fopen(file_name, "r");
  if (fp == NULL)
    exit(EXIT_FAILURE);

    while ((read = getline(&line, &len, fp)) != -1) {
      printf("Read and Run:%s\n", line);
      status = ysh_run_commands(line);
      if(!status) break;
    }

    fclose(fp);
    if (line)
       free(line);
    exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
  if(argc>1)
  {
    ysh_run_file(argv[1]);
  } else {
    // Run command loop.
    ysh_loop();
  }
  return EXIT_SUCCESS;
}

