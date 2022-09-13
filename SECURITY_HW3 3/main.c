#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFFER_SIZE           1024
#define MAIL_FROM_PREFIX_SIZE 10
#define RCPT_TO_PREFIX_SIZE   8

/*
1. checking if a username is a valid username
2. DATA handling
3. End of DATA (.) 
*/

int num_valid_usernames = 0;
char **valid_usernames = NULL;
int receipt_to_handled = 0;

char *tmpfilename = "tmpmail";
//char *tmpfilename = "../tmp/prefix";

int tmpfilename_count = 0;
FILE *currentFile = NULL;


void handle_mailfrom(char *line, char *from_user);
void handle_receipt_to(char *line, int *num_recipients, char *recipients[]);
int handle_line(char *line);
void open_tmp_file() ;
void close_tmp_file() ;
//void handle_end_of_mail() ;




static int my_id = 0;


/*
void handle_end_of_mail() {
    pid_t cpid = fork();

    if (cpid==0) {
        printf("child\n");

        char filename[1024] = {0};
        sprintf( filename, "%s%d", tmpfilename, tmpfilename_count );

        my_id++;
        
        exit(0);

    }
    
    printf("parent");
}
*/


int main(int argc, char *argv[]) {


    int mailfrom_read_in = 0;
    char from_user[1024] = {0};
    
    int num_recipients = 0;
    char *recipients[BUFFER_SIZE] = {0};

    char line[BUFFER_SIZE] = {0};
    // read first line
    char *fgets_result = fgets(line, BUFFER_SIZE, stdin);
    // continue reading lines until we have no more
    while (fgets_result != NULL) {
        if ( strcmp( line, "\n" ) != 0 ) {
            if ( ! mailfrom_read_in ) {
                open_tmp_file();
                handle_mailfrom(line, from_user);   
                fprintf(currentFile, "From: %s\n", from_user);
                mailfrom_read_in = 1;
            }
            else if (receipt_to_handled == 0) {
                handle_receipt_to(line, &num_recipients, recipients);
            }
            else {
                // just print the line because that's what happens here
                // we will have to come back and scrub the lines for periods
                // as well as handle end-of-mail and possible other mails in this file
                int r = handle_line(line);

                // is end-of-mail?
                if (r==1) {
                    putchar('\n');
                    mailfrom_read_in = 0;
                    receipt_to_handled = 0;
                    close_tmp_file();
         

                        int cpid = fork();
                        // child
                        if (cpid == 0) {
                            
                            my_id++;

                            for (int i = 0; i < num_recipients; i++) {
                            

                                printf("recipient[%d]: %s\n", i, recipients[i]);

                                int cpid2 = fork();
                                if (cpid2 == 0) {

                                    char filename[1024] = {0};
                                    sprintf( filename, "%s%d", tmpfilename, tmpfilename_count );
                                    int tmpmail_fd = open(filename, O_RDONLY);
                                    if (tmpmail_fd == -1) {
                                        perror ("Failed to open file");
                                        exit(-1);
                                    }

                                    // redirect stdin to our file
                                    int dup2_result = dup2( tmpmail_fd, STDIN_FILENO );
                                    if (dup2_result == -1) {
                                        perror("dup2 error");
                                        exit(-1);
                                    }
                                    close(tmpmail_fd);

                                    char *mailout_argv[] = {
                                        "./mail-out",
                                        
                                        //from_user,
                                        recipients[i],
                                        
                                        NULL
                                    };
                                    execvp("./mail-out", mailout_argv);
                                    exit(0);
                                }
                            }
                            exit(0);

                        }
                        else if (cpid > 0) {
                            // parent
                            bzero( from_user, 1024 );

                            num_recipients = 0;
                            for (int i = 0; i < num_recipients; i++) {
                                bzero( recipients[i], BUFFER_SIZE );
                            }

                        }
                        else {
                            perror("Error forking");
                            exit(-1);
                        }    

                    //}



                    

                    tmpfilename_count++;
                    //printf("moving on to next mail...%d\n", tmpfilename_count);
                }
                // more lines to read in for this mail
            }
        }
        
        bzero( line, BUFFER_SIZE );

        fgets_result = fgets(line, BUFFER_SIZE, stdin);

    }
    return 0;
}


void handle_mailfrom(char *line, char *from_user){
    // check to see if the first 10 chars are "MAIL FROM:"
    //printf("handle_mailfrom: [%s]\n", line);
    int r = strncmp(line, "MAIL FROM:", MAIL_FROM_PREFIX_SIZE);
    if (r==0) {
        // read in "MAIL FROM", handle appropriately
        // copy just the part of the line after "MAIL FROM:"
        //printf(":::is mail from\n");

        strcpy(from_user, line + MAIL_FROM_PREFIX_SIZE );
        from_user[ strlen(from_user)-1 ] = 0;
    }
    else {
        perror("Error, expecting MAIL FROM, exiting...\n");
        exit(-1);
    }
}


void handle_receipt_to(char *line, int *num_recipients, char *recipients[]) {
    int r = strncmp(line, "RCPT TO:", RCPT_TO_PREFIX_SIZE);
    int r0 = strncmp(line, "DATA", 4);
    int r1 = strncmp(line, "\n", 1);
    if (r==0) {
        int index = *num_recipients;
        recipients[index] = (char *)calloc(sizeof(char), BUFFER_SIZE);
        if (recipients[index]==NULL) {
            perror("Error callocing recipients[index], exiting...\n");
            exit(-1);
        }
        strcpy(recipients[index], line + RCPT_TO_PREFIX_SIZE); 
        recipients[index][ strlen( recipients[index] ) - 1 ] = 0;
        fprintf(currentFile, "To: %s\n", recipients[index]);
        (*num_recipients)++;
    }
    else if (r0 == 0) {
        receipt_to_handled = 1;
    }
    else if (r1 == 0) {
        fprintf(stderr, "Error processing handle_receipt_to, line contained blank line\n" );
        exit(-1);
    }
    else {
        fprintf(stderr, "Error processing handle_receipt_to, line contained: [%s]\n", line);
        exit(-1);
    }
}



int handle_line(char *line) {
    
    //printf("handle_line %d: %s\n", my_id, line);

    int i = 0;
    int len = strlen(line);
    // is the line a single period?
    int result = strcmp(line, ".\n");
    if (result==0) {
        return 1;     
    }

    while (i < len-1) {
        char c = line[i];
        if (c != '.') {
            //putchar(c);
            fputc(c, currentFile);
        }
        else {
            char d = line[i+1];
            if (d == '.') {
                //putchar(c);
                fputc(c, currentFile);
                i++;
            }
        }
        i++;
    }

    //putchar('\n');
    fputc('\n', currentFile);

    return 0;
}


void open_tmp_file() {
    //printf("open_tmp_file\n");

    char filename[1024] = {0};
    sprintf( filename, "%s%d", tmpfilename, tmpfilename_count );

    //printf("filename: %s\n", filename);

    currentFile = fopen(filename, "w+");
    if (currentFile==NULL) {
        perror("Error opening file");
        exit(-1);
    }
}

void close_tmp_file() {
    fclose(currentFile);
}
