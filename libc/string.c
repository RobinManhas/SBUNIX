#include <string.h>



char* strcat(char *dest,const char *src){
	unsigned int d;
	unsigned int s=0;
	for(d=0;dest[d]!='\0';d++);
	for(s=0;src[s]!='\0';s++){
		dest[d+s] = src[s];
	}

	dest[d+s]='\0';
	return dest;
}

unsigned int strlen(const char *s){
	if(!s){
		return -1;
	}
	unsigned int d = 0;
	for(d=0;s[d]!='\0';d++);
	return d;
}

char *strcpy(char *dest, const char *src){
	unsigned int s = 0;
	for(s=0;src[s]!='\0';s++){
		dest[s]=src[s];
	}
	dest[s]='\0';
	return dest;
}

char *strncpy(char *dest, const char *src, unsigned int n){
	unsigned int i =0;
	for(i=0;i<n;i++){
		dest[i]=src[i];
	}
	dest[i]='\0';
	return dest;

}
int strncmp(const char *s1, const char *s2, unsigned int n){
	unsigned int _s1_len;
	unsigned int _s2_len;
	_s1_len = strlen(s1);
	_s2_len = strlen(s2);
	if(_s1_len >= n && _s2_len >= n){
		for(unsigned int i=0;i<n;i++){
			if(s1[i]<s2[i]){
				return -1;
			}else if(s1[i]>s2[i]){
				return 1;
			}
		}
	}else if(_s1_len < _s2_len){
		return -1;
	}else if(_s1_len > _s2_len){
		return 1;
	}
	return 0;
}
char *strchr(const char *s, int c){
	char _c = (char)c;
	while(*s!='\0'){
		if(*s==_c){
			return (char *)s;
		}
		s++;
	}
	return 0;
}
char *strtok(char *str, const char *delim){
	static char *perm;
	return strtok_r(str,delim ,&perm);
	/*char *_token;
	int len;
	int str_len;
	if(str!=NULL){
		perm = str;
	}else if(perm==NULL){
		return 0;
	}
	printf("%s\n", perm);
	_token = perm;
	while(_token!=NULL && strchr(delim,*_token))
        len++;_token++;
	str = perm + strspn(perm,delim);
	_token = str;
	len =0;
    while(*_token!=NULL){
        if(strchr(delim,*_token)){
            break;
        }
        else{
            _token++,len++;
        }
    }
    perm = str + len;
    if(perm==str)
        return perm=0;
    if(*perm!=NULL){
    	*perm = '\0';
    	perm++;
    }else{
    	perm = NULL;
    }
    return str;*/

}

char *strtok_r(char *str, const char *delim , char **saveptr){
	char *perm;
	char *_token;
	int len=0;
	perm = *saveptr;
	if(str!=NULL){
		perm = str;
	}else if(perm==NULL){
		return 0;
	}
	_token = perm;
	while(_token!=NULL && strchr(delim,*_token))
        len++;_token++;
	str = perm + len;
	_token = str;
	len =0;
    while(*_token!='\0'){
        if(strchr(delim,*_token)){
            break;
        }
        else{
            _token++,len++;
        }
    }
    perm = str + len;
    if(perm==str)
        return perm=0;
    if(*perm!='\0'){
    	*perm = '\0';
    	perm++;
    }else{
    	perm = NULL;
    }
    *saveptr = perm;
    return str;
}

char* trimString(char* str){
	if (!str)
		return '\0';
	if(strlen(str)<1) return '\0';
	while(*str == ' ')
		str++;

	if(*str == '\0')
		return str;

	char* end = str+strlen(str) - 1;
	while(*end == ' ' && end > str)
		end --;

	*(end+1) = '\0';

	return str;
}

void* memset(void* ptr, int val, unsigned int len){
	unsigned char *p = ptr;
	while(len > 0)
	{
		*p = val;
		p++;
		len--;
	}
	return(p);
}

long stoi(const char *s){
	long result = 0;
	while(*s !='\0' && *s>='0' && *s <='9'){
		result = result*10 + (*s-'0');
		s++;
	}
	return result;
}