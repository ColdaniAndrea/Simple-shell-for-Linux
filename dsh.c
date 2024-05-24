/*

 COLDANI ANDREA mat. 894684


 ELABORATO LINGUAGGIO C

 SISTEMI OPERATIVI

*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_LINE 4096
#define MAX_ARGS 256
#define MAX_PATH 512
#define MAX_PROMPT 32
#define VAR_SIZE 100 //NEW CODE

char _path[MAX_PATH] = "/bin/:/usr/bin/";
struct Element* variables[VAR_SIZE]; //NEW CODE

struct Element{ //NEW CODE
	char* name;
	char* value;
};

void panic(const char* msg){
	//delete all allocated memory
	for(int i = 0; i < VAR_SIZE; i++){
	   if(variables[i] != NULL) free(variables[i]);
	}
	if(errno) {
	  fprintf(stderr, "ERROR: %s: %s", msg, strerror(errno));
	}else {
	  fprintf(stderr, "ERROR: %s\n\n", msg);
	}
	exit(EXIT_FAILURE);
}

int prompt(char* buf, size_t buf_size, const char* prompt_string) {
	printf("%s", prompt_string);
	if(fgets(buf, buf_size, stdin) == NULL) {
	   return EOF;
	}
	size_t cur = -1;
	do {
	     cur++;
	     if(buf[cur] == '\n') {
		 buf[cur] = '\0';
		 break;
	     }
	    }while(buf[cur] != '\0');
	return cur;
}

void set_var(char* var, char* content, struct Element* variables[]) { //NEW CODE
	int i = 0;
	if(strcmp(var, "path") == 0 || strcmp(var, "PATH") == 0) { //path case
	  if(strlen(content) > MAX_PATH) { //Check path length
	    panic("Path too long");
	  }
	  strcpy(_path, content);
	  printf("New path: %s\n", content);
	  return;
	}else { //all other cases
	 for(int k = 0; k < VAR_SIZE; k++){ //if already exists: replace
	  if(variables[k] != NULL){
	    if(strcmp(variables[k]->name, var) == 0) {
	      variables[k]->value = strdup(content);
	      printf("Variable already exists: replaced succesfully\n");
	      return;
	    }
	  }
	 }
	}
	while(variables[i] != NULL && i < VAR_SIZE){ //last case: adding new variables
	  i++;
	}
	if(i == VAR_SIZE) {
	  panic("Too much variables decleared");
	}else {
	  variables[i] = (struct Element*)malloc(sizeof(struct Element));
	  variables[i]->name = strdup(var);
	  variables[i]->value = strdup(content);
	}
	printf("New variable: %s -> %s\n", var, content);
}

void show_var(struct Element* variables[]){ //NEW CODE
	for(int i = 0; i < VAR_SIZE; i++){
	    if(variables[i] != NULL) {
	  	printf("%d- %s", i + 1, variables[i]->name);
	  	printf(" ");
	  	printf("%s", variables[i]->value);
	  	printf("\n");
	    }
	}
}

void delete_var(char *var, struct Element* variables[]) { //NEW CODE
	int i = 0, found = 0;
	while(found == 0 && i < VAR_SIZE){ //search if exists
	  if(variables[i] != NULL) {
	    if(strcmp(variables[i]->name, var) == 0) {
	      found = 1;
	    }else i++;
	  }else i++;
	}
	if(i == VAR_SIZE) {
	  panic("Specified Var doesn't exist");
	}else {
	  free(variables[i]);
	  variables[i] = NULL;
	  printf("Variable deleted succesfully\n");
	}
}

void var_value(char* arg, struct Element* variables[]){
	int i = 0, found = 0;
	if(strcmp(arg, "PATH") == 0 || strcmp(arg, "path") == 0) { // case it's the path
	   printf("PATH: %s\n", _path);
	}else {
	  while(found == 0 && i < VAR_SIZE){ //search if exists
	    if(variables[i] != NULL) {
	      if(strcmp(variables[i]->name, arg) == 0) {
	        found = 1;
	      }else i++;
	    }else i++;
	  }
	  if(i == VAR_SIZE) {
	    panic("Specified Var doesn't exist");
	  }else {
	    printf("%s\n", variables[i]->value);
	  }
	}
}

void path_lookup(char* abs_path, const char* rel_path) {
	char* prefix;
	char buf[MAX_PATH];
	if(abs_path == NULL || rel_path == NULL)
	  panic("get_abs_path: parameter error");
	prefix = strtok(_path, ":");
	while(prefix != NULL) {
	  strcpy(buf, prefix);
	  strcat(buf, rel_path);
	  if(access(buf, X_OK) == 0) {
	    strcpy(abs_path, buf);
	    return;
	  }
	  prefix = strtok(NULL, ":");
	}
	strcpy(abs_path, rel_path);
}

void exec_rel2abs(char** arg_list) {
	if(arg_list[0][0] == '/') {
	    // assume absolute path
	    execv(arg_list[0], arg_list);
	  } else {
	    // assume relativa path
	    char abs_path[MAX_PATH];
	    path_lookup(abs_path, arg_list[0]);
	    execv(abs_path, arg_list);
	  }
}

void do_redir(const char* out_path, char** arg_list, const char* mode) {
	if(out_path == NULL)
	  panic("do_redir: no path");
	int pid = fork();
	if(pid > 0) {
	  int wpid = wait(NULL);
	  if(wpid < 0) panic("do_redir: wait");
	} else if(pid == 0) {
	// begin child code
	  FILE* out = fopen(out_path, mode);
	  if(out == NULL) {
	    perror(out_path);
	    exit(EXIT_FAILURE);
	  }
	  dup2(fileno(out), 1);
	  exec_rel2abs(arg_list);
	  perror(arg_list[0]);
	  exit(EXIT_FAILURE);
	  // end child code
	} else {
	  panic("do_redir: fork");
	}
}

void do_pipe(size_t pipe_pos, char** arg_list){
	int pipefd[2];
	int pid;
	if(pipe(pipefd) < 0) panic("do_pipe: pipe");
	// left side of pipe
	pid = fork();
	if(pid > 0) {
	  int wpid = wait(NULL);
	  if(wpid < 0) panic("do_pipe: wait");
	} else if(pid == 0) {
	  // begin child code
	  close(pipefd[0]);
	  dup2(pipefd[1], 1);
	  close(pipefd[1]);
	  exec_rel2abs(arg_list);
	  perror(arg_list[0]);
	  exit(EXIT_FAILURE);
	  // exit child code
	} else {
	  panic("do_pipe: fork");
	}
	// right side of the pipe
	pid = fork();
	if(pid > 0) {
	  close(pipefd[0]);
	  close(pipefd[1]);
	  int wpid = wait(NULL);
	  if(wpid < 0) panic("do_pipe: wait");
	} else if(pid == 0) {
	  // begin child code
	  close(pipefd[1]);
	  dup2(pipefd[0], 0);
	  close(pipefd[0]);
	  exec_rel2abs(arg_list + pipe_pos +1);
	  perror(arg_list[pipe_pos + 1]);
	  exit(EXIT_FAILURE);
	  // end child code
	} else {
	  panic("do_pipe: fork");
	}
}

void do_exec(char** arg_list) {
	int pid = fork();
	if(pid > 0) {
	  int wpid = wait(NULL);
	  if(wpid < 0) panic("do_exec: wait");
	} else if(pid == 0) {
	  // begin child code
	  exec_rel2abs(arg_list);
	  perror(arg_list[0]);
	  exit(EXIT_FAILURE);
	  // end child code
	} else {
	  panic("do_exec: fork");
	}
}

int main(void) {
	char input_buffer[MAX_LINE];
	size_t arg_count;
	char* arg_list[MAX_ARGS];
	char prompt_string[MAX_PROMPT] = "\0";
	if(isatty(0)) {
	// we are in an interactive session
	  strcpy(prompt_string, "dsh$ \0");
	}
	while(prompt(input_buffer, MAX_LINE, prompt_string) >= 0) {
	   // tokenize input
	   arg_count = 0;
	   arg_list[arg_count] = strtok(input_buffer, " ");
	   if(arg_list[arg_count] == NULL) {
	     // nothing specified at command prompt
	     continue;     
	   }else {
	     do {
	       arg_count++;
	       if(arg_count > MAX_ARGS) break;
	       arg_list[arg_count] = strtok(NULL, " ");
	     }while(arg_list[arg_count] != NULL);
	   }
#if USE_DEBUG_PRINTF
	   // DEBUG: print tokens
	   printf("DEBUG: tokens:");
	   for(size_t i = 0; i < arg_count; i++){
	     printf(" %s", arg_list[i]);   
	   } 
	   puts("");  
#endif
	   // builtins
	   if(strcmp(arg_list[0], "exit") == 0){
	     //delete all allocated memory
	     for(int i = 0; i < VAR_SIZE; i++){
	       if(variables[i] != NULL) free(variables[i]);
	     }
	     break;
	   }
	   if(strcmp(arg_list[0], "set") == 0){ //NEW CODE
	     if(arg_list[1] == NULL || arg_list[2] == NULL) { //check format
	       panic("Var doesn't have a name or a content");
	     }else { //if it's okay: set it
	       set_var(arg_list[1], arg_list[2], variables);
	       continue;
	     }
	   }
	   if(strcmp(arg_list[0], "delete") == 0){ //NEW CODE
	     if(arg_list[1] == NULL) { //check format
	       panic("Not specified Var"); 
	     }else {
	       delete_var(arg_list[1], variables);
	       continue;
	     }
	   }
	   if(strcmp(arg_list[0], "showvar") == 0){ //NEW CODE
	     show_var(variables);
	     continue;
	   }
	   if(strcmp(arg_list[0], "echo") == 0 && arg_list[1] == NULL){ //NEW CODE
	     panic("Not valid format for command echo");
	     continue;
	   }
	   if(strcmp(arg_list[0], "echo") == 0 && arg_list[1][0] == '$'){ //NEW CODE
	     var_value(arg_list[1] + 1, variables);
	     continue;
	   }
	   {
	   // check for special characters
	     size_t redir_pos = 0;
	     size_t append_pos = 0;
	     size_t pipe_pos = 0;
	     for(size_t i = 0; i < arg_count; i++) {
	       if(strcmp(arg_list[i], ">") == 0) {
	         redir_pos = i;
	         break;
	       }
	       if(strcmp(arg_list[i], ">>") == 0) {
	         append_pos = i;
	         break;
	       }
	       if(strcmp(arg_list[i], "|") == 0) {
	         pipe_pos = i;
	         break;
	       }
	     }
	     // do shell ops
	     if(redir_pos != 0) {
	       // redirect
	       arg_list[redir_pos] = NULL;
	       do_redir(arg_list[redir_pos+1], arg_list, "w+");
	     } else if(append_pos != 0){
	         arg_list[append_pos] = NULL;
	         do_redir(arg_list[append_pos+1], arg_list, "a+");
	     } else if(pipe_pos != 0) {
	         arg_list[pipe_pos] = NULL;
	         do_pipe(pipe_pos, arg_list);
	     } else {
	     	 // exec
	         do_exec(arg_list);
	     }
	   }
	}
	
	exit(EXIT_SUCCESS);
}
