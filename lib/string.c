#include "string.h"
#include "mkrtos/mem.h"

static const unsigned char charmap[] = 
{
'\000', '\001', '\002', '\003', '\004', '\005', '\006', '\007',
'\010', '\011', '\012', '\013', '\014', '\015', '\016', '\017',
'\020', '\021', '\022', '\023', '\024', '\025', '\026', '\027',
'\030', '\031', '\032', '\033', '\034', '\035', '\036', '\037',
'\040', '\041', '\042', '\043', '\044', '\045', '\046', '\047',
'\050', '\051', '\052', '\053', '\054', '\055', '\056', '\057',
'\060', '\061', '\062', '\063', '\064', '\065', '\066', '\067',
'\070', '\071', '\072', '\073', '\074', '\075', '\076', '\077',
'\100', '\141', '\142', '\143', '\144', '\145', '\146', '\147',
'\150', '\151', '\152', '\153', '\154', '\155', '\156', '\157',
'\160', '\161', '\162', '\163', '\164', '\165', '\166', '\167',
'\170', '\171', '\172', '\133', '\134', '\135', '\136', '\137',
'\140', '\141', '\142', '\143', '\144', '\145', '\146', '\147',
'\150', '\151', '\152', '\153', '\154', '\155', '\156', '\157',
'\160', '\161', '\162', '\163', '\164', '\165', '\166', '\167',
'\170', '\171', '\172', '\173', '\174', '\175', '\176', '\177',
'\200', '\201', '\202', '\203', '\204', '\205', '\206', '\207',
'\210', '\211', '\212', '\213', '\214', '\215', '\216', '\217',
'\220', '\221', '\222', '\223', '\224', '\225', '\226', '\227',
'\230', '\231', '\232', '\233', '\234', '\235', '\236', '\237',
'\240', '\241', '\242', '\243', '\244', '\245', '\246', '\247',
'\250', '\251', '\252', '\253', '\254', '\255', '\256', '\257',
'\260', '\261', '\262', '\263', '\264', '\265', '\266', '\267',
'\270', '\271', '\272', '\273', '\274', '\275', '\276', '\277',
'\300', '\341', '\342', '\343', '\344', '\345', '\346', '\347',
'\350', '\351', '\352', '\353', '\354', '\355', '\356', '\357',
'\360', '\361', '\362', '\363', '\364', '\365', '\366', '\367',
'\370', '\371', '\372', '\333', '\334', '\335', '\336', '\337',
'\340', '\341', '\342', '\343', '\344', '\345', '\346', '\347',
'\350', '\351', '\352', '\353', '\354', '\355', '\356', '\357',
'\360', '\361', '\362', '\363', '\364', '\365', '\366', '\367',
'\370', '\371', '\372', '\373', '\374', '\375', '\376', '\377',
};
int strncasecmp(const char *s1, const char *s2, register unsigned int n)
{
	register unsigned char u1, u2;
	for (; n != 0; --n) 
	{
		u1 = (unsigned char) *s1++;
		u2 = (unsigned char) *s2++;
		if (charmap[u1] != charmap[u2]) 
		{
			return charmap[u1] - charmap[u2];
		}
		if (u1 == '\0') 
		{
			return 0;
		}
	}
	return 0;
}

int strcmp(register const char *s1, register const char *s2)
{
	int r;

	while (((r = ((int)(*((char *)s1))) - *((char *)s2++))
			== 0) && *s1++);

	return r;
}

void *memcpy(void * __restrict s1, const void * __restrict s2, unsigned int n)
{
	register char *r1 = s1;
	register const char *r2 = s2;

	while (n) {
		*r1++ = *r2++;
		--n;
	}

	return s1;
}
void *memmove(void *s1, const void *s2, int n)
{
	register char *s = (char *) s1;
	register const char *p = (const char *) s2;

	if (p >= s) {
		while (n) {
			*s++ = *p++;
			--n;
		}
	} else {
		while (n) {
			--n;
			s[n] = p[n];
		}
	}

	return s1;
}

int strlen(const char *s)
{
	register const char *p;

	for (p=s ; *p ; p++);

	return p - s;
}
char * strcpy(char * __restrict s1, const char * __restrict s2)
{
	register char *s = s1;

	while ( (*s++ = *s2++) != 0 );

	return s1;
}
char* strcat(char * __restrict s1, register const char * __restrict s2)
{
	register char *s = s1;

	while (*s++);
	--s;
	while ((*s++ = *s2++) != 0);

	return s1;
}
char *strstr(const char *s1, const char *s2)
{
	register const char *s = s1;
	register const char *p = s2;

	do {
		if (!*p) {
			return (char *) s1;;
		}
		if (*p == *s) {
			++p;
			++s;
		} else {
			p = s2;
			if (!*s) {
				return NULL;
			}
			s = ++s1;
		}
	} while (1);
}

void *memset(void *s, int c, int n)
{
	register uint8_t *p = (uint8_t *) s;

	while (n) {
		*p++ = (uint8_t) c;
		--n;
	}

	return s;
}
int memcmp(const void *s1, const void *s2, int n)
{
	register const uint8_t *r1 = (const uint8_t *) s1;
	register const uint8_t *r2 = (const uint8_t *) s2;
		int r = 0;

	while (n-- && ((r = ((int)(*r1++)) - *r2++) == 0));

	return r;

}
int strncmp(register const char *s1, register const char *s2, int n){
	int r = 0;

	while (n--
		   && ((r = ((int)(*((unsigned char *)s1))) - *((unsigned char *)s2++))
			== 0)
		   && *s1++);

	return r;
}

char* strncpy(char * __restrict s1, register const char * __restrict s2,
				int n)
{
	register char *s = s1;

	while (n) {
		if ((*s = *s2) != 0) s2++; /* Need to fill tail with 0s. */
		++s;
		--n;
	}

	return s1;
}

char *strchr(register const char *s, int c)
{
	do {
		if (*s == ((char)c)) {
			return (char *) s;	/* silence the warning */
		}
	} while (*s++);

	return NULL;
}

char * strdup(char *str)  
{
	char * strNew;
	// assert(str != NULL);
	if(str==NULL){
	return NULL;
	}
	strNew = (char *)OSMalloc(strlen(str)+1);
	strcpy(strNew,str);
	return strNew;
}    
int index(register char*str ,register char c){
	register int i;
	for(i=0;str[i];i++){
		if(str[i]==c){
			break;
		}
	}
	return i;
}
