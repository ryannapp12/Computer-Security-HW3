#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>

void print_usage(char *arg);
int validate_username(char *username);
void read_from_stdin( char *username ) ;
int count_mails_for_username( char *username ) ;



char *mailPathRoot = "mail";

// mail/username/0
// mail/username/1
// mail/username/2


int main(int argc, char *argv[]) {
    if (argc != 2) {
        print_usage(argv[0]);
        exit(-1);
    }
    // username is argv[1]
    char *username = argv[1];
    // validate username
    int r = validate_username(username);
    // handle 
    if (r!=0) {
        printf("Error: invalid username: %s\n", username);
        exit(-1);
    }

    read_from_stdin(username);

    return EXIT_SUCCESS;
}

void print_usage(char *arg) {
    printf("Usage: %s <username>\n", arg);
}

int validate_username(char *username) {
    // open the mail directory
    DIR* mydir = opendir("mail");
    if (mydir==NULL) {
        return -1;
    }
    // read individual directory entries
    struct dirent *mydirent = NULL;
    mydirent = readdir(mydir);
    
    int success = -1;
    while (mydirent != NULL) {
        //printf("dirname: %s\n", mydirent->d_name);
        if (strcmp( username, mydirent->d_name ) == 0 ) {
            success = 0;
            break;
        }
        mydirent = readdir(mydir);
    }

    closedir(mydir);
    return success;
}


int count_mails_for_username( char *username ) {
    char myfilepath[1024] = {0};
    sprintf(myfilepath, "%s/%s", 
        mailPathRoot, 
        username
    );

    DIR *mydir = opendir(myfilepath);
    struct dirent *mydirent=NULL;
    mydirent = readdir(mydir);
    int count = 0;
    while (mydirent != NULL) {
        count++;   
        mydirent = readdir(mydir);
    }

    return count;
}



void read_from_stdin( char *username ) {
    char myfilepath[1024] = {0};
    
    int count = count_mails_for_username( username );

    sprintf(myfilepath, "%s/%s/%05d", 
        mailPathRoot, 
        username,
        count
    );


    FILE *mymailfile = fopen(myfilepath, "w+");
    if (mymailfile==NULL) {
        perror("Failed to open mymailfile");
        exit(-1);
    }


    while (! feof(stdin) ) { 
        char buf[1024] = {0};
        fgets(buf, 1024, stdin);
        //printf("%s", buf);
        fprintf(mymailfile, "%s", buf);
    }

    fclose(mymailfile);
}