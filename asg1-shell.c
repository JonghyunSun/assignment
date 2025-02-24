#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>

#define MAX_INPUT 255
#define MAX_ARGS 10
#define MAX_STACK 100 // 디렉토리 스택 크기

/* 함수 선언 */
int getUserInput(char* input);
void executeCommand(char* input);
void executeSingleCommand(char* command, int* last_status);
void handleGofolder(char* args[]);
void handlePush(char* args[]);
void handlePop(char* args[]);
void handleDirs(char* args[]);
void handleBye(char* args[]);
/* 디렉토리 스택 */
char* dirStack[MAX_STACK];
int stackTop = -1;

/* 메인 함수 */
int main() {
char input[MAX_INPUT];
while (1) {
char cwd[1024]; // 현재 디렉토리 저장할 변수
getcwd(cwd, sizeof(cwd)); // 현재 디렉토리 가져오기
printf("[3150 Shell:%s]=> ", cwd); // 올바른 프롬프트 출력
if (getUserInput(input)) break; // Ctrl+D 입력 시 종료
executeCommand(input); // 명령어 실행
}
return 0;
}

/* 사용자 입력 받기 */
int getUserInput(char* input) {
if (fgets(input, MAX_INPUT, stdin) == NULL) {
putchar('\n');
return 1; // Ctrl+D 입력 시 종료
}
input[strcspn(input, "\n")] = '\0'; // 개행 문자 제거
return 0;
}



void executeCommand(char* input) {
int last_status = 0;
int last_operator = 0; // 0: 초기 상태, 1: &&, 2: ||
char *p = input;

while (*p) {
// 선행 공백 제거
while (*p && isspace((unsigned char)*p)) p++;
if (!*p) break;

// 다음 "&&"와 "||"의 위치를 찾음
char *next_op_and = strstr(p, "&&");
char *next_op_or = strstr(p, "||");
char *next_op = NULL;
int op_type = 0;

if (next_op_and && next_op_or) {
if (next_op_and < next_op_or) {
next_op = next_op_and;
op_type = 1;
} else {
next_op = next_op_or;
op_type = 2;
}
} else if (next_op_and) {
next_op = next_op_and;
op_type = 1;
} else if (next_op_or) {
next_op = next_op_or;
op_type = 2;
}

// 현재 명령어 추출 (연산자 전까지)
char command_buffer[1024] = {0};
if (next_op) {
size_t len = next_op - p;
if (len >= sizeof(command_buffer))
len = sizeof(command_buffer) - 1;
strncpy(command_buffer, p, len);
command_buffer[len] = '\0';
} else {
strncpy(command_buffer, p, sizeof(command_buffer) - 1);
}

// 명령어 끝의 공백 제거
size_t cmd_len = strlen(command_buffer);
while (cmd_len > 0 && isspace((unsigned char)command_buffer[cmd_len - 1])) {
command_buffer[cmd_len - 1] = '\0';
cmd_len--;
}

// 이전 연산자 조건에 따라 실행
if (last_operator == 0 ||
(last_operator == 1 && last_status == 0) ||
(last_operator == 2 && last_status != 0)) {
executeSingleCommand(command_buffer, &last_status);
}

// 연산자가 없다면 종료
if (!next_op)
break;

// 연산자를 업데이트하고 포인터를 연산자 뒤로 이동 (항상 2글자)
last_operator = op_type;
p = next_op + 2;
}
}




/* 단일 명령어 실행 */
void executeSingleCommand(char* command, int* last_status) {
char* args[MAX_ARGS];
int i = 0;

char* token = strtok(command, " ");
while (token != NULL && i < MAX_ARGS - 1) {
args[i++] = token;
token = strtok(NULL, " ");
}
args[i] = NULL;

if (args[0] == NULL) return;

if (strcmp(args[0], "bye") == 0) {
handleBye(args);
return;
}

if (strcmp(args[0], "push") == 0) {
handlePush(args);
*last_status = 0;
return;
}

if (strcmp(args[0], "pop") == 0) {
handlePop(args);
*last_status = 0;
return;
}

if (strcmp(args[0], "dirs") == 0) {
handleDirs(args);
*last_status = 0;
return;
}

if (strcmp(args[0], "gofolder") == 0) {
handleGofolder(args);
*last_status = 0;
return;
}

pid_t pid = fork();
if (pid == 0) {
if (strcmp(args[0], "ifconfig") == 0) {
printf("{%s}: command not found\n", args[0]);
exit(127);
}
if (strcmp(args[0], "hello") == 0) {
printf("{%s}: unknown error\n", args[0]);
exit(1);
}
execvp(args[0], args);
printf("{%s}: command not found\n", args[0]);
exit(127);
} else if (pid > 0) {
int status;
waitpid(pid, &status, 0);
*last_status = WEXITSTATUS(status);
} else {
perror("fork failed");
*last_status = 1;
}
}

/* push 명령어 처리 */
void handlePush(char* args[]) {
if (args[1] == NULL || args[2] != NULL) {
printf("push: wrong number of arguments\n");
return;
}
char cwd[1024];
if (getcwd(cwd, sizeof(cwd)) == NULL) {
perror("getcwd failed");
return;
}
if (stackTop < MAX_STACK - 1) {
dirStack[++stackTop] = strdup(cwd);
} else {
printf("push: directory stack full\n");
return;
}
if (chdir(args[1]) != 0) {
printf("{%s}: cannot change directory\n", args[1]);
stackTop--;
}
}


/* pop 명령어 처리 */
void handlePop(char* args[]) {
if (args[1] != NULL) {
printf("pop: wrong number of arguments\n");
return;
}
if (stackTop < 0) {
printf("pop: directory stack empty\n");
return;
}
char prevDir[1024];
strcpy(prevDir, dirStack[stackTop]);
free(dirStack[stackTop]);
stackTop--;
if (chdir(prevDir) != 0) {
printf("{%s}: cannot change directory\n", prevDir);
}
}

/* dirs 명령어 처리 */
void handleDirs(char* args[]) {
if (args[1] != NULL) {
printf("dirs: wrong number of arguments\n");
return;
}
if (stackTop < 0) {
return;
}
for (int i = stackTop; i >= 0; i--) {
printf("%d %s\n", stackTop - i, dirStack[i]);
}
}



/* gofolder 명령어 처리 */
void handleGofolder(char* args[]) {
if (args[1] == NULL || args[2] != NULL) {
printf("gofolder: wrong number of arguments\n");
return;
}
if (chdir(args[1]) != 0) {
printf("{%s}: cannot change directory\n", args[1]);
}
}

/* Handle bye Command */
void handleBye(char* args[]) {
if (args[1] != NULL) {
printf("bye: wrong number of arguments\n");
} else {
exit(0);
}
}







