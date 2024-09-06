#define _XOPEN_SOURCE
#define _GNU_SOURCE

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <crypt.h>
#include <stdbool.h>

#define LIMIT 16

int found = 0;
char stop = 'z';
char start = 'a';
int threadCount;

pthread_mutex_t lock;

struct Param {
	int id;
	int len;
	char *salt;
	char *pswd;
	struct crypt_data *data;
}; 

void init(char *word, int size){
	int i = 0;
	for(i; i<size; i++) word[i] = start;
	word[size] = '\0';
}

void getSalt(char *pswd, char *salt){
	salt[0] = pswd[0];
	salt[1] = pswd[1];
	salt[2] = '\0';

	int size = strlen(pswd);
	int i,j = 0;
	for (i=2; i < size; i++) pswd[j++] = pswd[i];
	pswd[j] = '\0';
}

bool done(char *word, int index, int max, int offset){
        if (index < max) return true;

	char c = word[index];

        if (c == stop) {
                word[index] = start;
                return done(word, index-1, max, offset);
        }

	 word[index] += offset;
	 return false;
}


int crack(char *salt, char *word, char *pswd, int index, int max, struct crypt_data *data, int id){
	char *hash = crypt_r(word, salt, data);
	
	getSalt(hash, salt);
	
	if (hash != NULL && strcmp(hash, pswd) == 0) return 1;

	if (word[index] == stop){
		word[index] = start;
	 	return 0;
	}

	word[index] += 1;

	return crack(salt, word, pswd, index, max, data, id);
}

void *run(void *args){
        struct Param *p = (struct Param *) args;
        char word[LIMIT];
	int j = p->len;
	int count = 1;

	struct crypt_data data;
	data.initialized = 0;

        while (j >= 1){
               	init(word, j);
		int h;
		char c;
		h = j - 1;

                while(h >= 0){

			bool end = false;

			if (h != j-1) word[h] += 1;

			while(!end){
					if (found > 0) pthread_exit(NULL);

					if (count == p->id){

						//if (p->id == 1) printf("Now Word for %d : %s\n", p->id, word);
			
	               		      		int res = crack(p->salt, word, p->pswd, j-1, h, &data, p->id);			
		                        
						if (res == 1){
							__sync_add_and_fetch(&found, 1);

							printf("Password is: %s\n", word);
        			        	        pthread_exit(NULL);
	        	        	        }
					}

				if (count == threadCount) count = 1;
				else count++;

			        end = done(word, j-2, h, 1);

				 //if ((p->id == 8 || p->id == 7) && j == 4) printf("Next Word for %d : %s\n", p->id, word);
			}
			h--;
                }

		j--;
        }
}

int main(char argc, char *argv[]){

	if (argc < 4){
		printf("Invalid number of arguments\n");
		printf("Usage: ./crack <threads> <keysize> <target> <uppercase> \n");
		return 0;
	}

	int keysize = atoi(argv[2]);
	threadCount = atoi(argv[1]);

	if (keysize > 8 || keysize < 1){
		printf("Keysize must from 1 to 8 (inclusive)\n");
		return 0;
	}


	if (threadCount < 1){
		printf("Number of threads must be atleast 1\n");
		return 0;
	}

	if (argc > 4 && argv[4] != NULL){
		start = '!';
		stop = '~';
	}

	char salt[3];
	getSalt(argv[3], salt);

	int k = 0;
	pthread_t thread[threadCount];
	struct Param p[threadCount];
	int res[threadCount];

	pthread_mutex_init(&lock, NULL);
	 
	for (k; k < threadCount; k++){
	        p[k].salt = salt;
	        p[k].pswd = argv[3];
		p[k].id = k+1;

		p[k].len = keysize;

		res[k] = pthread_create(&thread[k], NULL, run, &p[k]);
	}

	for (k = 0; k < threadCount; k++){
		if (res[k] == 0){ 
			pthread_join(thread[k], NULL);
		}
	}

	return 0;
}
