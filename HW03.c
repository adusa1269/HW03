#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>

// Function to print file information
void printFileInfo(const char* filePath, struct stat* fileStat, int options) {
    if (options & 1) {
        // -S option
        printf("%s (%ld bytes, permissions: %o, last access time: %s)\n",
               filePath, (long)fileStat->st_size, fileStat->st_mode & 0777, ctime(&fileStat->st_atime));
    } else {
        printf("%s\n", filePath);
    }
}

// Function to execute a command for a given file
void executeCommand(const char* command, const char* filePath) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "%s \"%s\"", command, filePath);

    if (system(cmd) == -1) {
        perror("Command execution failed");
    }
}

// Function to search files and directories recursively
void searchDirectory(const char* path, int depth, int options, const char* execCommand) {
    DIR *dir;
    struct dirent *entry;
    struct stat fileStat;

    if (!(dir = opendir(path))) {
        return;
    }

    while ((entry = readdir(dir))) {
        char filePath[1024];
        snprintf(filePath, sizeof(filePath), "%s/%s", path, entry->d_name);

        if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            if (options & 2) {
                // -f option
                if (strstr(entry->d_name, (const char*)(options + 3)) == NULL || depth > options - 3) {
                    continue;
                }
            }

            printf("%*s[%s]\n", depth, "", entry->d_name);
            searchDirectory(filePath, depth + 1, options, execCommand);
        } else if (entry->d_type == DT_REG) {
            if (options & 2) {
                // -f option
                if (strstr(entry->d_name, (const char*)(options + 3)) == NULL || depth > options - 3) {
                    continue;
                }
            }

            if (options & 1) {
                // -S option
                if (stat(filePath, &fileStat) == 0) {
                    printFileInfo(filePath, &fileStat, options);
                }
            } else {
                printf("%s\n", entry->d_name);
            }

            if (execCommand != NULL) {
                executeCommand(execCommand, filePath);
            }
        } else if (entry->d_type == DT_LNK) {
            if (options & 2) {
                // -f option
                if (strstr(entry->d_name, (const char*)(options + 3)) == NULL || depth > options - 3) {
                    continue;
                }
            }

            if (options & 1) {
                // -S option
                if (stat(filePath, &fileStat) == 0) {
                    printFileInfo(filePath, &fileStat, options);
                }
            } else {
                printf("%s (%s)\n", entry->d_name, filePath);
            }

            if (execCommand != NULL) {
                executeCommand(execCommand, filePath);
            }
        }
    }
    closedir(dir);
}

int main(int argc, char *argv[]) {
    char *path;
    int options = 0; // Bitmask for options
    const char* execCommand = NULL;

    // Parse command-line arguments
    if (argc >= 2) {
        path = argv[1];
    } else {
        path = ".";
    }

    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-S") == 0) {
            options |= 1;
        } else if (strcmp(argv[i], "-s") == 0) {
            if (i + 1 < argc) {
                options |= 1;
                int size = atoi(argv[i + 1]);
                options |= size << 1;
                i++;
            } else {
                printf("Invalid -s option. Missing file size argument.\n");
                return 1;
            }
        } else if (strcmp(argv[i], "-f") == 0) {
            if (i + 2 < argc) {
                options |= 2;
                options |= i;
                i += 2;
            } else {
                printf("Invalid -f option. Missing string pattern or depth argument.\n");
                return 1;
            }
        } else if (strcmp(argv[i], "-e") == 0) {
            if (i + 1 < argc) {
                execCommand = argv[i + 1];
                i++;
            } else {
                printf("Invalid -e option. Missing command argument.\n");
                return 1;
            }
        } else if (strcmp(argv[i], "-E") == 0) {
            if (i + 1 < argc) {
                execCommand = argv[i + 1];
                i++;
            } else {
                printf("Invalid -E option. Missing command argument.\n");
                return 1;
            }
        }
            else if (strcmp(argv[i], "-t") == 0) {
            if (i + 1 < argc) {
                execCommand = argv[i + 1];
                i++;
            } else {
                printf("Invalid -T option. Missing command argument.\n");
                return 1;
            }
        }
    }

    // Start the search
    searchDirectory(path, 0, options, execCommand);

    return 0;
}
