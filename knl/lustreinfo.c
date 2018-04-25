#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <lustre/lustreapi.h>
#include <lustre/lustre_user.h>
#include "lustreinfo.h"

#define MAX_OSTS 256
#define LOV_EA_SIZE(lum, num) (sizeof(*lum) + num * sizeof(*lum->lmm_objects))
#define LOV_EA_MAX(lum) LOV_EA_SIZE(lum, MAX_OSTS)
 
struct OST2LNET {

  char ostidx[16];
  char ostib[16];
  char lnets[64];

} *ost2lnet;

/* Print out some LOV attributes. List our objects */
int get_file_info(char *path)
{
 
	struct lov_user_md *lump;
	int rc;
	int i;

/*  
  ost2lnet = malloc (MAX_OSTS * sizeof (struct OST2LNET*));

  FILE *fp;
  fp = fopen("theta_ost2lnet", "r");
 
  int line=0;

  while(!feof(fp))
  {
   fscanf (fp, "%s %s %s", ost2lnet[line].ostidx, ost2lnet[line].ostib, ost2lnet[line].lnets);
   //printf ("%s %s %s\n", ost2lnet[line].ostidx, ost2lnet[line].ostib, ost2lnet[line].lnets);
   line++;
  }

  fclose(fp);
*/
   
	lump = malloc(LOV_EA_MAX(lump));
	if (lump == NULL) {
		return -1;
  }
 
  rc = llapi_file_get_stripe(path, lump);
        
  if (rc != 0) {
		fprintf(stderr, "get_stripe failed: %d (%s)\n",errno, strerror(errno));
		return -1;
  }

	printf("Lov magic %u\n", lump->lmm_magic);
	printf("Lov pattern %u\n", lump->lmm_pattern);
	//printf("Lov object id %llu\n", lump->lmm_object_id);
	//printf("Lov object group %llu\n", lump->lmm_object_gr);
	printf("Lov stripe size %u\n", lump->lmm_stripe_size);
	printf("Lov stripe count %hu\n", lump->lmm_stripe_count);
	printf("Lov stripe offset %d\n", lump->lmm_stripe_offset);

  printf("\n");
	for (i = 0; i < lump->lmm_stripe_count; i++) {
		//printf("Object index %d OST%04x %Lu %Lu Generation of OST index %d\n", lump->lmm_objects[i].l_ost_idx, lump->lmm_objects[i].l_ost_idx, lump->lmm_objects[i].l_ost_oi.oi.oi_id, lump->lmm_objects[i].l_ost_oi.oi.oi_seq, lump->lmm_objects[i].l_ost_gen);
		printf("OST%04x ", lump->lmm_objects[i].l_ost_idx); 
    
		//printf("Object index %d Generation of OST index %d Objid %llu\n", lump->lmm_objects[i].l_ost_idx, lump->lmm_objects[i].l_ost_gen, lump->lmm_objects[i].l_object_id);
   }
  printf("\n");
 
	free(lump);
	return rc;
   
}

