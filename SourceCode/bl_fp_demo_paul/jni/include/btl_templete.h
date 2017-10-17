#ifndef _BL_TEMPLETE_H_
#define	_BL_TEMPLETE_H_

#include "btl_algo.h"


#define FINGER_DEBUG            1
#define MAX_FINGER_NUMS    5



char mEnrolledIndex[MAX_FINGER_NUMS];

typedef struct tag_templete_db{
//header begin
unsigned int magicnum; //0xfdfcfbfa
unsigned int fingernum; 
unsigned int fingerindex ;//
unsigned int MaxtemplateSize ;//
unsigned int fingerTemplateOffset ;//10*1024
unsigned int fingerHeaderOffset; //
unsigned int fingerTypeOffset ;//20
unsigned int fingerSizeOffset ;//10
unsigned int fingerDataOffset ;//10
unsigned int fingerUnitTmoduleOffset; //

//header end
}Templete_db_t;




typedef struct tag_templete_manager{

Templete_db_t tdb ;//used for check temptlete database
int nbr_of_templetes;//enrolled  templetes
PBL_TEMPLATE pCurrent_templete;
PBL_TEMPLATE *pArry_templete;
PBL_TEMPLATE *PMatch_templete;

int need_init;
int (*get_templete_by_id)(int,PBL_TEMPLATE);
int (*save_templete_by_id)(int,PBL_TEMPLATE);
int (*get_all_templete)(PBL_TEMPLATE *);
int (*init_templete_manager)(PBL_TEMPLATE *);
int (*get_enrolled_fingerNum)(int *);
int (*get_enroll_fingerid)();
int (*create_databasefile)();
int (*delete_fingerprint_by_id)(int);

}Templete_manager_t,*pTemplete_manager_t;

int btl_tmanager_init_templete_manager();
pTemplete_manager_t btl_get_templete_manager();

#include "btlfp.h"
#endif

