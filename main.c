#include "streaming.h"
#include <stdio.h>
#include <string.h>	//for strtok()

int main(){
	int ret = 0;
	char *temp1 = NULL;
	char *temp2 = NULL;
	char *temp3 = NULL;
	char lineBuf[500];
	memset(lineBuf, '\0', 500);

	FILE *fp;
	if((fp = fopen("./playlist.txt", "r")) == NULL){
		fprintf(stderr, "Cannot open the target file.\n");
		return 1;
	}else{
		while(fgets(lineBuf, sizeof(lineBuf), fp)){
			//printf("%s", lineBuf);
			temp1 = strtok(lineBuf, "#");
			temp2 = strtok(NULL, "#");
			temp3 = strtok(NULL, "#");
			if(temp1==NULL || temp2==NULL || temp3==NULL){
				fprintf(stderr, "Grammer error in pgmlist.\n");
				return 1;
			}
			ret = streaming(temp1, temp2, atoi(temp3));
			if(ret != 0){
				fprintf(stderr, "Something error while streaming.\n");
				return 1;
			}
			temp1 = temp2 = temp3 = NULL;
		}
	}
	return 0;
}
