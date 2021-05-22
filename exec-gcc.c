#include <stdio.h>
#include <string.h>

#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

size_t BUFSIZE = 8192;
char* file_name = "tmp.c";

void create_cfile(char* expression)
{
    int fd = open(file_name, O_WRONLY | O_CREAT, 0644);
    if (fd == -1) {
        perror("open");
        return;
    }
    char c_program[BUFSIZE];
    snprintf(
        c_program,
        BUFSIZE,
        "#include <stdio.h>\n"
        "int main() { printf(\"%%d\", (%s));"
        "return 0; }",
        expression);
    if (write(fd, c_program, strlen(c_program)) == -1) {
        perror("write");
        close(fd);
        return;
    }
    close(fd);
}

int execute(const char* file_name, char* const argv[])
{
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return 1;
    }
    if (pid == 0) {
        execvp(file_name, argv);
        perror("exec");
        return 2;
    } else if (pid > 0) {
        int status;
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid");
            return 3;
        }
        if (WEXITSTATUS(status) != 0) {
            return WEXITSTATUS(status);
        }
    }
    return 0;
}

int main(int argc, char** argv)
{
    char expression[BUFSIZE];
    fgets(expression, BUFSIZE, stdin);
    create_cfile(expression);

    int status;
    char* const compiler_args[] = {"gcc", file_name, NULL};
    char* const run_args[] = {"./a.out", NULL};
    if ((status = execute("gcc", compiler_args)) != 0) {
        return status;
    }
    if ((status = execute("./a.out", run_args)) != 0) {
        return status;
    }

    remove(file_name);
    remove("a.out");
    return 0;
}
