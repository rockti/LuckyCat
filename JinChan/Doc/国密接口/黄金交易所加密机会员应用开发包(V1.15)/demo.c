#include "standardapi.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IP 				"192.168.2.18"
#define PORT 			8889
#define TIMEOUT			3000

#define MAXPTHREADNUM 	30
void *threadfun(void *arg)
{
	int ret = 0;
	void *stat = malloc(sizeof(int));
	encryptor_ctx_t enctor = {0};
	socket_ctx_t * sockfd = NULL;
	memset(&enctor,0,sizeof(enctor));
	memcpy(enctor.encryptor_ip,IP,strlen(IP));
	enctor.port = PORT;
	enctor.timeout = TIMEOUT;
		
	sockfd = cb_connect_(&enctor);
	if(!sockfd)
	{
		printf("cb_connect_ error\n");
		*(int*)stat = -1;
		goto EXIT;
	}

	ret = cb_reconnect_(sockfd);
	if(ret)
	{
		printf("cb_reconnect_ error\n");		
		*(int*)stat = -1;
		goto EXIT;
	}

	ret = cb_reset_connect_(sockfd);
	if(ret)
	{
		printf("cb_resetconnect_ error\n");		
		*(int*)stat = -1;
		goto EXIT;
	}

	ret = cb_is_connected_(sockfd);
	if(ret!=1)
	{
		printf("cb_connectstatus_ error\n");		
		*(int*)stat = -1;
		goto EXIT;
	}

	unsigned char random[200] = {0};
	ret = cb_gen_random_(sockfd,55,random);
	if(ret)
	{
		printf("cb_random_ error\n");		
		*(int*)stat = -1;
		goto EXIT;
	}
	
	keypair_ctx_t *enc_keypair;
	const char idx[9]="00000000";
	ret = cb_get_enc_keypair_(sockfd,idx,&enc_keypair);
	if(ret)
	{
		printf("cb_get_enc_keypair_ error\n");		
		*(int*)stat = -1;
		goto EXIT;
	}

	unsigned char pCipherKey[256] = {0};
	int nCipherKeyLen = 256;
	unsigned char pKCV[32] = {0};
	unsigned char pSessionKey[256];
	int pSessionKeyLen=256;
	ret = SYD_SM2_GenSessionKey(
  			sockfd->sockfd,
  			enc_keypair->pPubKey,
  			enc_keypair->pPubKeyLen,
  			pCipherKey,
			&nCipherKeyLen,
			pSessionKey,
			&pSessionKeyLen,
			pKCV
  			);
	if(ret)
	{
		printf("SYD_SM2_GenSessionKey error\n");		
		*(int*)stat = -1;
		goto EXIT;
	}

	ret = cb_free_enc_keypair_(sockfd,enc_keypair);
	if(ret)
	{
		printf("cb_free_enc_ error\n");		
		*(int*)stat = -1;
		goto EXIT;
	}

	ret = cb_get_skey_(sockfd,idx,pCipherKey,nCipherKeyLen,pSessionKey,&pSessionKeyLen);
	if(ret)
	{
		printf("cb_get_skey_ error\n");		
		*(int*)stat = -1;
		goto EXIT;
	}

	unsigned char src[1024] = {0};
	int srclen=300;
	unsigned char dst[2048] = {0};
	int cap=2048;
	memset(src,1,1024);
	ret = cb_encrypt_(sockfd,pSessionKey,pSessionKeyLen,src,srclen,dst,&cap);
	if(ret)
	{
		printf("cb_encrypt_ error\n");		
		*(int*)stat = -1;
		goto EXIT;
	}
	srclen=1024;
	ret = cb_decrypt_(sockfd,pSessionKey,pSessionKeyLen,dst,cap,src,&srclen);
	if(ret)
	{
		printf("cb_decrypt_ error\n");		
		*(int*)stat = -1;
		goto EXIT;
	}

	memset(dst,0,2048);
	cap=2048;
	ret = cb_mac_(sockfd,pSessionKey,pSessionKeyLen,src,srclen,dst,&cap);
	if(ret)
	{
		printf("cb_decrypt_ error\n");		
		*(int*)stat = -1;
		goto EXIT;
	}

	unsigned char hash[32] = {0};
	int hash_cap=32;
	ret = cb_hash_(sockfd,src,srclen,hash,&hash_cap);
	if(ret)
	{
		printf("cb_hash_ error\n");		
		*(int*)stat = -1;
		goto EXIT;
	}

	keypair_ctx_t *sign_keypair;
	ret = cb_get_sign_keypair_(sockfd,idx,&sign_keypair);
	if(ret)
	{
		printf("cb_get_sign_keypair_ error\n");		
		*(int*)stat = -1;
		goto EXIT;
	}

	memset(dst,0,2048);
	cap=2048;
	ret = cb_sign_(sockfd,sign_keypair,src,100,dst,&cap);
	if(ret)
	{
		printf("cb_sign_ error\n");		
		*(int*)stat = -1;
		goto EXIT;
	}

	ret = cb_verify_sign_(sockfd,src,100,dst,cap,sign_keypair->pPubKey+8,sign_keypair->pPubKeyLen-8);
	if(ret)
	{
		printf("cb_verify_ error\n");		
		*(int*)stat = -1;
		goto EXIT;
	}

	ret = cb_free_sign_keypair_(sockfd,sign_keypair);
	if(ret)
	{
		printf("cb_free_sign_ error\n");		
		*(int*)stat = -1;
		goto EXIT;
	}
	
	ret = cb_disconnect_(sockfd);
	if(ret)
	{
		printf("cb_disconnect_ error\n");		
		*(int*)stat = -1;
		goto EXIT;
	}
EXIT:

	pthread_exit(stat);
}
int main(int argc, char *argv[])
{
	int i=0, ret=0;
	int threadnum = 0;
	int cyclenum = 0;
	void *state = NULL;
	pthread_t pthid[MAXPTHREADNUM];

	if(argc != 3)
	{
		printf("argc is not three\n");
		exit(-1);
	}
	
	threadnum = atoi(argv[1]);
	cyclenum = atoi(argv[2]);

	for(i=0;i<threadnum;i++)
	{
		ret = pthread_create(&pthid[i],NULL,threadfun,(void*)&cyclenum);
		if(ret < 0)
		{
			printf("pthread_create err\n");
			exit(-1);
		}
	}

	
	for(i=0;i<threadnum;i++)
	{
			ret = pthread_join(pthid[i],&state);
			if(ret != 0 && *(int *)state != 0)
			{
					perror("pthread_join");
					exit(-3);
			}
			else
					printf("thread state:%d\n",*(int *)state);

			free(state);
			state = NULL;
	}

	return 0;
}
