#include <sys/kstring.h>





unsigned int kstrlen(const char *s){
	if(!s){
		return -1;
	}
	unsigned int d = 0;
	for(d=0;s[d]!='\0';d++);
	return d;
}


int kstrcmp(const char *s1, const char *s2){
	while(*s1 && *s2 && *s1 == *s2) {
		s1++;
		s2++;
	}
	return *s1 - *s2;
}

int kstrncmp(const char *s1, const char *s2, unsigned int n){
	unsigned int _s1_len;
	unsigned int _s2_len;
	_s1_len = kstrlen(s1);
	_s2_len = kstrlen(s2);
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


long kstoi(const char *s){
    long result = 0;
	while(*s !='\0' && *s>='0' && *s <='9'){
        result = result*10 + (*s-'0');
        s++;
	}
    return result;
}

char *kstrcpy(char *dest, const char *src){
	unsigned int s = 0;
	for(s=0;src[s]!='\0';s++){
		dest[s]=src[s];
	}
	dest[s]='\0';
	return dest;
}

void* kmemcpy( void* dest, const void* src, unsigned long count) {
	char *d = dest;
	const char *s = src;
	for (int i = 0; i < count; i++) {
		d[i] = s[i];
	}
	return dest;
}



char *kstrncpy(char *dest, const char *src, unsigned int n){
    unsigned int i =0;
    for(i=0;i<n;i++){
        dest[i]=src[i];
    }
    dest[i]='\0';
    return dest;

}

char* kstrcat(char *dest,const char *src){
	unsigned int d;
	unsigned int s=0;
	for(d=0;dest[d]!='\0';d++);
	for(s=0;src[s]!='\0';s++){
		dest[d+s] = src[s];
	}

	dest[d+s]='\0';
	return dest;
}

int ktostring(char buf[], int num){
	int i, rem, len = 0, n;

	n = num;
	while (n != 0) {
		len++;
		n /= 10;
	}
	for (i = 0; i < len; i++) {
		rem = num % 10;
		num = num / 10;
		buf[len - (i + 1)] = rem + '0';
	}
	buf[len] = '\0';
	return len;
}
