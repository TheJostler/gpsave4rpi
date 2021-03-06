
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

#ifndef BUF_SIZE
#define BUF_SIZE 4096
#endif

static int filewrite (const char *fname, const char *data) {
	int fw;

	fw = open(fname, O_WRONLY | O_NONBLOCK);
	if (fw < 0) {
		printf("Could not open %s...%d\n",fname, fw);
		return 1;
	}
	write(fw, data, strlen(data));
	close(fw);

	return 0;
}

int GPIO (char *num, char *command) {
	if (command == "export") {
		return filewrite("/sys/class/gpio/export", num);
	}
	else if (command == "unexport") {
		return filewrite("/sys/class/gpio/unexport", num);
	}
	else {
		return 1;
	}
}

static int cp (char *file_src, char *file_des) {

    int inputFd, outputFd, openFlags;
    mode_t filePerms;
    ssize_t numRead;
    char buf[BUF_SIZE];

    printf("cp %s to %s", file_src, file_des);

    /* Open input and output files */

    inputFd = open(file_src, O_RDONLY);
    if (inputFd == -1) {
        printf(" -- ERROR opening file %s\n", file_src);
    	return 1;
    }

    openFlags = O_CREAT | O_WRONLY | O_TRUNC;
    filePerms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |
                S_IROTH | S_IWOTH;      /* rw-rw-rw- */
    outputFd = open(file_des, openFlags, filePerms);
    if (outputFd == -1) {
        printf(" -- ERROR opening file %s\n", file_des);
    	return 1;
    }

    /* Transfer data until we encounter end of input or an error */

    while ((numRead = read(inputFd, buf, BUF_SIZE)) > 0)
        if (write(outputFd, buf, numRead) != numRead)
            printf(" -- write() returned error or partial write occurred\n");
    if (numRead == -1) {
        printf(" -- ERROR read\n");
    	return 1;
    }

    if (close(inputFd) == -1) {
        printf(" -- ERROR closing input\n");
        return 1;
    }
    if (close(outputFd) == -1) {
        printf(" -- ERROR closing output\n");
        return 1;
    }
    printf(" -- FINISHED\n");
    return 0;
}

static int ls_cp(const char *src, const char *dest) {

	struct stat src_st, des_st;
	DIR *h_src = opendir(src);
	if (h_src == NULL) {
		printf("Could not open directory: %s\n", src);
		return 1;
	}
	DIR *h_dest = opendir(dest);
	if (h_dest == NULL) {
		printf("Could not open directory: %s\n", dest);
		return 1;
	}
	closedir(h_dest);

	struct dirent *de;
    char dest_[100];
    char src_[100];
	while ((de = readdir(h_src)) != NULL) {
		if (de->d_type != 4) {
			/*** SRC SIZE ***/
			stat(de->d_name, &src_st);
			int src_size = src_st.st_size;

			/*** DEFINE SRC PATH ***/
			strcpy(src_, src);
			strcat(src_, "/");
			strcat(src_, de->d_name);				

			/*** DEFINE DEST PATH ***/
			strcpy(dest_, dest);
			strcat(dest_, "/");
			strcat(dest_, de->d_name);
			
			/*** DEST SIZE ***/
			stat(dest_, &des_st);
			int des_size = des_st.st_size;

			if (src_size != des_size)
		    	if (cp(src_, dest_) > 0)
					printf("Failed Backing up %s\n", de->d_name);
				chmod(dest_, src_st.st_mode);
		}
		/*** RECURSION ***/
		else if (strcmp(de->d_name, ".")){
			if (strcmp(de->d_name, ".."))
				printf("Found dir: %s\n", de->d_name);

				/*** SRC SIZE ***/
				stat(de->d_name, &src_st);
				int src_size = src_st.st_size;

				/*** DEFINE SRC PATH ***/
				strcpy(src_, src);
				strcat(src_, "/");
				strcat(src_, de->d_name);				

				/*** DEFINE DEST PATH ***/
				strcpy(dest_, dest);
				strcat(dest_, "/");
				strcat(dest_, de->d_name);

				/*** DEST SIZE ***/
				stat(dest_, &des_st);
				int des_size = des_st.st_size;

				mkdir(dest_, S_IRWXU);
				if (src_size != des_size)
					ls_cp(src_, dest_);
		}
	}
	closedir(h_src);
	return 0;
}

/*This is only the begining*/
int main () {



/***/// GET CONFIG ///***/

	char *gpio_in, *gpio_out, *src, *dest, *cp_recursive, *input_flip, *buff_size, *restore_perms;

	//temps
	char *set, *line = NULL;
	size_t len;
	ssize_t read;
	FILE *conf;

	conf = fopen("/etc/gpsave/gpsave.config", "r");
	if (conf == NULL) {
		printf("Couldn't open config file\n");
		return 1;
	}

	//find configs, strip '\n' char from the ends and declare them as variables. Will ignore all lines with '#'
	while ((read = getline(&line, &len, conf)) != -1) {
		if(strstr(line, "gpio_in") && ! strstr(line, "#"))
			while((set = strsep(&line, "=")) != NULL) {
				if (strcmp(set, "gpio_in")){
					int len = strlen(set);
					set[len-1] = '\0';
					gpio_in = set;
				}
			}
		else if(strstr(line, "gpio_out") && ! strstr(line, "#"))
			while((set = strsep(&line, "=")) != NULL) {
				if (strcmp(set, "gpio_out")){
					int len = strlen(set);
					set[len-1] = '\0';
					gpio_out = set;
				}
			}
		else if(strstr(line, "src") && ! strstr(line, "#"))
			while((set = strsep(&line, "=")) != NULL) {
				if (strcmp(set, "src")){
					int len = strlen(set);
					set[len-1] = '\0';
					src = set;
				}
			}
		else if(strstr(line, "dest") && ! strstr(line, "#"))
			while((set = strsep(&line, "=")) != NULL) {
				if (strcmp(set, "dest")){
					int len = strlen(set);
					set[len-1] = '\0';
					dest = set;
				}
			}
		else if(strstr(line, "cp_recursive") && ! strstr(line, "#"))
			while((set = strsep(&line, "=")) != NULL) {
				if (strcmp(set, "cp_recursive")){
					int len = strlen(set);
					set[len-1] = '\0';
					cp_recursive = set;
				}
			}
		else if(strstr(line, "input_flip") && ! strstr(line, "#"))
			while((set = strsep(&line, "=")) != NULL) {
				if (strcmp(set, "gpio_in_flip")){
					int len = strlen(set);
					set[len-1] = '\0';
					input_flip = set;
				}
			}
		else if(strstr(line, "buff_size") && ! strstr(line, "#"))
			while((set = strsep(&line, "=")) != NULL) {
				if (strcmp(set, "buff_size")){
					int len = strlen(set);
					set[len-1] = '\0';
					buff_size = set;
				}
			}
		else if(strstr(line, "restore_perms") && ! strstr(line, "#"))
			while((set = strsep(&line, "=")) != NULL) {
				if (strcmp(set, "restore_perms")){
					int len = strlen(set);
					set[len-1] = '\0';
					restore_perms = set;
				}
			}
	}
	fclose(conf);

	/*** DEBUG ***/

	printf("\n\ngpio_in: %s\n gpio_out: %s\n src: %s\n dest: %s\n cp_recursive: %s\n gpio_in_flip: %s\n buff_size: %s\n restore_perms: %s\n",gpio_in, gpio_out, src, dest, cp_recursive, input_flip, buff_size, restore_perms);


	while(1 == 1){
		/*** READ GPIO ***/
		if (GPIO(gpio_in, "export") > 0) {
			printf("ERROR could not export gpio%s\n", gpio_in);
			exit(1);
		}

	        char *buff;
	        char *gpiovalue = "/sys/class/gpio/gpio23/value";
	       	FILE *fr;

	/*** WAIT FOR BUTTON PUSH ***/
		while (1 == 1) {
	        usleep(50000);
			fr = fopen(gpiovalue,"r");
			if (fr == NULL) {
				printf("ERROR could not open %s\n", gpiovalue);
				exit(1);
			}
			fscanf(fr, "%d", &buff);
			if (buff) {
				printf("\nBacking up\n");
				break;
			}
			printf("\rWaiting for button push");
			fflush(stdout);
			fclose(fr);
		}

	/*** LS DIR + COPY ***/
		if (ls_cp(src, dest) > 0)
			return 1;

	/*** WAIT FOR BUTTON RELEASE ***/
		while (1 == 1) {
	        usleep(50000);
			fr = fopen(gpiovalue,"r");
			if (fr == NULL) {
				printf("ERROR could not open %s\n", gpiovalue);
				exit(1);
			}
			fscanf(fr, "%d", &buff);
			if (!buff) {
				printf("\n");
				usleep(150000);
				break;
			}
			printf("\rfinished");
			fflush(stdout);
			fclose(fr);
		}
	}
}
