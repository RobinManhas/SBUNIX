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
