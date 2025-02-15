#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_INPUT 255
#define MAX_ARGS 10

/* 함수 선언 */
int getUserInput(char* input);
void executeCommand(char* input);

/* 메인 함수 */
int main() {
    char input[MAX_INPUT];

    while (1) {
        char cwd[1024];  // 현재 디렉토리 저장할 변수
        getcwd(cwd, sizeof(cwd));  // 현재 디렉토리 가져오기
        printf("[3150 Shell:%s]=> ", cwd);  // 올바른 프롬프트 출력

        if (getUserInput(input)) break;  // Ctrl+D 입력 시 종료

        executeCommand(input);  // 명령어 실행
    }
    return 0;
}

/* 사용자 입력 받기 */
int getUserInput(char* input) {
    if (fgets(input, MAX_INPUT, stdin) == NULL) {
        putchar('\n');
        return 1;  // Ctrl+D 입력 시 종료
    }
    input[strcspn(input, "\n")] = '\0';  // 개행 문자 제거
    return 0;
}

/* 명령어 실행 */
void executeCommand(char* input) {
    char* args[MAX_ARGS];
    int i = 0;

    // 입력을 공백 기준으로 분할
    char* token = strtok(input, " ");
    while (token != NULL && i < MAX_ARGS - 1) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;  // execvp를 위해 마지막 인자는 NULL

    if (args[0] == NULL) return;  // 빈 입력이면 실행 안 함

    pid_t pid = fork();
    if (pid == 0) {  // 자식 프로세스
        execvp(args[0], args);
        printf("%s: command not found\n", args[0]);  // 실행 실패 시 메시지 출력
        exit(127);
    } else if (pid > 0) {  // 부모 프로세스
        int status;
        waitpid(pid, &status, 0);
    } else {
        perror("fork failed");
    }
}


