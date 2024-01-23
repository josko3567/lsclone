#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <stdbool.h>

#define MAXLEN 1024
 
char filetype_to_char(mode_t mode) {

    if     (S_ISREG(mode))  return '-';
    else if(S_ISFIFO(mode)) return 'f';
    else if(S_ISCHR(mode))  return 'c';
    else if(S_ISLNK(mode))  return 'l';
    else if(S_ISBLK(mode))  return 'b';
    else if(S_ISDIR(mode))  return 'd';
    return '?';

}

char * owner_privlages_to_string(char str[static 4], mode_t mode) {

    memcpy(str, "---\0", 4);
    if(S_IRUSR & mode) str[0] = 'r';
    if(S_IWUSR & mode) str[1] = 'w';
    if(S_IXUSR & mode) str[2] = 'x';
    return str;

}

char * group_privlages_to_string(char str[static 4], mode_t mode) {

    memcpy(str, "---\0", 4);
    if(S_IRGRP & mode) str[0] = 'r';
    if(S_IWGRP & mode) str[1] = 'w';
    if(S_IXGRP & mode) str[2] = 'x';
    return str;

}

char * other_privlages_to_string(char str[static 4], mode_t mode) {

    memcpy(str, "---\0", 4);
    if(S_IROTH & mode) str[0] = 'r';
    if(S_IWOTH & mode) str[1] = 'w';
    if(S_IXOTH & mode) str[2] = 'x';
    return str;

}

char * get_owner_name(uid_t id) {

    struct passwd * pwd = getpwuid(id);
    return pwd->pw_name;

}

char * get_group_name(gid_t id) {

    struct group * grp = getgrgid(id);
    return grp->gr_name;

}

struct named_stat {

    struct stat st;
    char name[MAXLEN];
    char path[MAXLEN];

};

struct named_stat * get_directory_contents(
    const char        *__restrict directory, 
    size_t            *__restrict ret_size,
    struct named_stat *__restrict ret_biggest)
{
    struct named_stat *stats  = NULL; // Svojstva stavke.
    struct dirent     *entry  = NULL; // Stavka direktorija
    DIR                 *dir  = NULL; // Direktorija.
    size_t              size  = 0; // Broj nađenih stavki 
                                   // direktorija, upisuje 
                                   // se u size poslije
    struct named_stat biggest = {0}; // Sadrži najveće vrijednosti
                                     // od sviju svojstva datoteka.
    
    if((dir = opendir(directory)) == NULL) {
        perror("opendir");
        goto exit_error;
    }

    // Izračunaj broj stavki datoteke.
    while((entry = readdir(dir)) != NULL) {size++;};

    if(size == 0) {
        errno = ENODATA;
        goto exit_error;
    }

    rewinddir(dir);
    
    if((stats = malloc(sizeof(struct named_stat) * size)) == NULL) {
        perror("malloc");
        goto exit_error;
    }

    for(int i = 0; (entry = readdir(dir)) != NULL && i < size; i++) {
        
        strcpy(stats[i].name, entry->d_name);
        // Puni relativni put do datoteka, dodaje se "/" između putanje
        // direktorija i ime jednih od stavki direktorija ako
        // nije nađen "/".
        snprintf(stats[i].path, MAXLEN, "%s%s%s", 
            directory, 
            directory[strlen(directory)-1] == '/' ? "" : "/", 
            entry->d_name
        );
        
        lstat(stats[i].path, &(stats[i].st));

        // Provjere za nove kandidate najvećih vrijednosti.
        biggest.st.st_size = stats[i].st.st_size > biggest.st.st_size ? 
            stats[i].st.st_size : biggest.st.st_size;

        biggest.st.st_nlink = stats[i].st.st_nlink > biggest.st.st_nlink ? 
            stats[i].st.st_nlink : biggest.st.st_nlink;

        int size_grpb = strnlen( get_group_name(biggest.st.st_gid), MAXLEN);
        int size_grpn = strnlen(get_group_name(stats[i].st.st_gid), MAXLEN);
        
        int size_usrb = strnlen( get_owner_name(biggest.st.st_uid), MAXLEN);
        int size_usrn = strnlen(get_owner_name(stats[i].st.st_uid), MAXLEN);

        biggest.st.st_gid = size_grpb < size_grpn ? stats[i].st.st_gid : biggest.st.st_gid;
        biggest.st.st_uid = size_usrb < size_usrn ? stats[i].st.st_uid : biggest.st.st_uid;

    };

    if(ret_size) *ret_size = size;
    if(ret_biggest) memcpy(ret_biggest, &biggest, sizeof(struct named_stat));
    return stats;    

exit_error:

    if(ret_size) *ret_size = 0;
    if(ret_biggest) memset(ret_biggest, 0, sizeof(struct named_stat));
    return NULL;  

}

size_t digits(size_t n) 
{
    char buf[MAXLEN/10];
    return(sprintf(buf,"%ld",n));
}

char * create_format_string(
    struct named_stat *biggest) 
{   
    static bool  start  = true;
    static char *format = NULL;

    if(__builtin_expect((start == true), 0)) {
        format = malloc(sizeof(char)*MAXLEN*10);
        start  = false;
    } else {
        return format;
    }

    char 
        *buffer_nlinks = (char[MAXLEN/10]){0},
        *buffer_owner  = (char[MAXLEN/10]){0},
        *buffer_group  = (char[MAXLEN/10]){0},
        *buffer_size   = (char[MAXLEN/10]){0}
    ;

    // Broj karaktera nekog broja.
    unsigned long nlink_digit = biggest ? digits(biggest->st.st_nlink) : 0;
    unsigned long size_digit  = biggest ? digits(biggest->st.st_size)  : 0;

    // Format našeg glavnog ispisa, ovdje zapisujemo koliko neki pojedini brojevi ili stringovi mogu biti dugi.
    snprintf(format, MAXLEN*10, "%%c%%s%%s%%s. %%%sld %%%ss %%%ss %%%sld %%12s %%s%%s%%s\n", 
                               (snprintf(buffer_nlinks, MAXLEN/10, "%ld", nlink_digit),                                buffer_nlinks), 
        biggest == NULL ? "" : (snprintf(buffer_owner , MAXLEN/10, "%ld", strlen(get_owner_name(biggest->st.st_uid))), buffer_owner ),
        biggest == NULL ? "" : (snprintf(buffer_group , MAXLEN/10, "%ld", strlen(get_group_name(biggest->st.st_gid))), buffer_group ),
                               (snprintf(buffer_size  , MAXLEN/10, "%ld", size_digit),                                 buffer_size  )
    );

    return format;
}



void print_named_stat(
    struct named_stat *__restrict nm_st,
    struct named_stat *__restrict biggest) 
{   
    // Izračunamo broj karaktera najveće datotečne veličine i ubacujemo je u
    // string za formatiranje glavnog ispisa.
    char 
        *format      = create_format_string(biggest),
        *buffer_date = (char[32]){0},
        *buffer_link = (char[MAXLEN]){0}
    ;

    struct tm 
        st_time  = {0},
        cur_time = {0}
    ;

    memcpy(&st_time,  localtime(&nm_st->st.st_mtim.tv_sec), sizeof(struct tm));
    memcpy(&cur_time, localtime(&(time_t){time(NULL)}),     sizeof(struct tm));

    // Glavni ispis.
    printf((const char*)format, 
        filetype_to_char(nm_st->st.st_mode),                        // File type,
        owner_privlages_to_string((char[4]){0}, nm_st->st.st_mode), // Owner rights
        group_privlages_to_string((char[4]){0}, nm_st->st.st_mode), // Group rights
        other_privlages_to_string((char[4]){0}, nm_st->st.st_mode), // Other rights
        nm_st->st.st_nlink,                                         // Number of links
        get_owner_name(nm_st->st.st_uid),                           // Owner name
        get_group_name(nm_st->st.st_gid),                           // Group name
        nm_st->st.st_size,                                          // File size
        cur_time.tm_year == st_time.tm_year ?                       // Modify date
              (strftime(buffer_date, MAXLEN, "%b %e %H:%M", &st_time), buffer_date)
            : (strftime(buffer_date, MAXLEN, "%b %e  %Y",   &st_time), buffer_date),
        nm_st->name,                                                // Filename
        // Link arrow and link name.
        S_ISLNK(nm_st->st.st_mode) ? " -> " : "",                   
        S_ISLNK(nm_st->st.st_mode) ? (readlink(nm_st->path, buffer_link, MAXLEN), buffer_link) : ""
    );
}


int main(
    int argc, 
    char **argv) 
{

    char *stavka = argv[1];

    if(argc < 2) {
        if((stavka = getenv("PWD")) == NULL) {
            perror("getenv");
            exit(EXIT_FAILURE);
        }
    }

    struct stat st = {0};
    if(lstat(stavka, &st) != 0) {
        perror("stat");
        exit(EXIT_FAILURE);
    }

    if( S_ISDIR(st.st_mode) ) {

        size_t size = 0;
        struct named_stat biggest = {0};
        struct named_stat *stats = get_directory_contents(stavka, &size, &biggest);
        if(stats == NULL) {
            perror("get_directory_contents");
            exit(EXIT_FAILURE);
        }
        printf("Found %ld entries in \"%s\", biggest entry is %ld bytes.\n", size, stavka, biggest.st.st_size);
        for(int i = 0; i < size; i++) {
            print_named_stat(&(stats[i]), &biggest);
        }
        free(stats);

    } else {

        struct named_stat * f_st = &(struct named_stat){
            .st = st,
            .name = {0},
            .path = {0},
        };

        strcpy(f_st->name, stavka);
        strcpy(f_st->path, stavka);
        print_named_stat(f_st, NULL);

    }

    exit(EXIT_SUCCESS);

}