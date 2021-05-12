#ifndef ENCRYPTION_H
#define ENCRYPTION_H
/* Structs */
struct keyArray {
	uint8_t		*array;
	uint16_t	length;
};
struct messageArray {
	uint8_t		*array;
	size_t		length;
};

/* Function declarations */
void messagePadding(struct messageArray **, uint16_t);
void reverseMessagePadding(struct messageArray **, uint16_t);
void blockEnc(uint8_t *, struct keyArray *);
void blockDec(uint8_t *, struct keyArray *);
void cbcEnc(struct messageArray **, struct keyArray *);
void cbcDec(struct messageArray **, struct keyArray *);
#endif
