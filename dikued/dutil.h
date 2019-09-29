/****************************************************************************
 *                                                                          *
 *             dutil.h (c) by Georg Essl in 1992                            *
 *                                                                          *
 ****************************************************************************/

/*** DEFINES ***/

#define MAX_STRING_LENGTH   4096
#define CREATE(result, type, number)  do {\
	if (!((result) = (type *) calloc ((number), sizeof(type))))\
		{ perror("malloc failure"); abort(); } } while(0)

#define DU_ERR_NOERROR      0
#define DU_ERR_LINEINVALID  1
#define DU_ERR_INVALIDSTART 2
#define DU_ERR_OOMFOROBJ    3
#define DU_ERR_STRINGTOOLONG 4
#define DU_ERR_CANTOPEN     5

/*** STRUCTURES ***/

struct index_data
{
	int virtual;    /* virtual number of this mob/obj           */
	long pos;       /* file position of this field              */
	long cpos;      /* char position */
};


#define BITSHIFT(c) (c - 'A')

#define FLAG(cr)  (BITSHIFT(cr) <= 25 ?  (1 << BITSHIFT(cr)) : \
        ((BITSHIFT(cr)-6) < 31 ? (1 << (BITSHIFT(cr)-6)): 0))


/*** PROTOTYPES ***/

int strnccmp(char *, char *,int);
int AskYesNo(char *);
char *fread_string(FILE *);
struct index_data *generate_indices(FILE *, int *);
void fwrite_string(FILE *,char *);
void write_string(char *, int);
char *read_multi_string(void);
long flag_convert(char *argument);

int GetADef(FILE *,char *,char *,char ***,int *);
void FreeADef(char **,int);
