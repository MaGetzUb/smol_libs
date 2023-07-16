

#define swap(a, b) {\
    if(sizeof(a) == sizeof(b)) { \
        switch(sizeof(a)) {  \
            case 1: { \
                unsigned char c; \
                *((unsigned char*)&c) = *((unsigned char*)&a); \
                *((unsigned char*)&a) = *((unsigned char*)&b); \
                *((unsigned char*)&b) = *((unsigned char*)&c); \
            } break; \
            case 2: { \
                unsigned short c; \
                *((unsigned short*)&c) = *((unsigned short*)&a); \
                *((unsigned short*)&a) = *((unsigned short*)&b); \
                *((unsigned short*)&b) = *((unsigned short*)&c); \
            } break; \
            case 4: { \
                unsigned int c; \
                *((unsigned int*)&c) = *((unsigned int*)&a); \
                *((unsigned int*)&a) = *((unsigned int*)&b); \
                *((unsigned int*)&b) = *((unsigned int*)&c); \
            } break; \
            case 8: { \
                unsigned long long c; \
                *((unsigned long long*)&c) = *((unsigned long long*)&a); \
                *((unsigned long long*)&a) = *((unsigned long long*)&b); \
                *((unsigned long long*)&b) = *((unsigned long long*)&c); \
            } break; \
            default: { \
                char tmp[sizeof(a)]; \
                memcpy(tmp, &a, sizeof(a)); \
                memcpy(&a, &b, sizeof(a)); \
                memcpy(&b, tmp, sizeof(a)); \
            } break; \
        } \
    } \
}

typedef struct ASD { 
    int x[5];
} ASD;

int main() {

    int x = 5;
    int y = 7;

    printf("before: x: %d y: %d\n", x, y);

    swap(x, y);

    printf("before: x: %d y: %d\n", x, y);

    return 0;
}