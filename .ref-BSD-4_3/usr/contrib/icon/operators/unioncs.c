#include "../h/rt.h"

/*
 * x ++ y - union of csets x and y or of sets x and y.
 */

unioncs(nargs, arg2, arg1, arg0)
int nargs;
struct descrip arg2, arg1, arg0;
   {
   DclSave
   register int i;
   union block *bp;
   int *cs1, *cs2, csbuf1[CSETSIZE], csbuf2[CSETSIZE];
   extern struct b_cset *alccset();
#ifdef SETS
   int res;
   struct b_set *srcp, *tstp, *dstp;
   struct b_selem *ep;
   struct descrip *dp, *hook;
   extern struct b_set *alcset();
   extern struct b_selem *alcselem();
   extern struct descrip *memb();
#endif SETS

   SetBound;
#ifdef SETS
   DeRef(arg1)
   DeRef(arg2)
   if (QUAL(arg1) || QUAL(arg2))
      goto skipsets;
   if (TYPE(arg1) == T_SET && TYPE(arg2) != T_SET)
      runerr(119,&arg2);
   if (TYPE(arg2) == T_SET && TYPE(arg1) != T_SET)
      runerr(119,&arg1);
   if (TYPE(arg1) == T_SET && TYPE(arg2) == T_SET) {
      /*
       *  Both x and y are sets - do set union
       *  get enough space for a set as big as x + y.
       */
         hneed(sizeof(struct b_set) + (BLKLOC(arg1)->set.setsize +
            BLKLOC(arg2)->set.setsize) * sizeof(struct b_selem));
         /*
          *  Select the larger of the two sets as the source
          *  copy each element to a new set for the result
          *  then insert each member of the second set into the
          *  result set if it is not already there.
          */
         if (BLKLOC(arg1)->set.setsize >= BLKLOC(arg2)->set.setsize) {
            srcp = (struct b_set *) BLKLOC(arg1);
            tstp = (struct b_set *) BLKLOC(arg2);
            }
         else {
            srcp = (struct b_set *) BLKLOC(arg2);
            tstp = (struct b_set *) BLKLOC(arg1);
            }
         arg0.type = D_SET;
         dstp = alcset();
         BLKLOC(arg0) = (union block *) dstp;
         for (i = 0; i < NBUCKETS; i++) {
            ep = (struct b_selem *) BLKLOC(srcp->sbucks[i]);
            dp = &dstp->sbucks[i];
            while (ep != NULL) {
               dp->type = D_SELEM;
               BLKLOC(*dp) = (union block *) alcselem(&ep->setmem, ep->hnum);
               dp = &BLKLOC(*dp)->selem.sblink;
               dstp->setsize++;
               ep = (struct b_selem *) BLKLOC(ep->sblink);
               }
            }
         for (i = 0; i < NBUCKETS; i++) {
            ep = (struct b_selem *) BLKLOC(tstp->sbucks[i]);
            while (ep != NULL) {
               hook = memb(dstp, &ep->setmem, ep->hnum, &res);
               if (res == 0)
                  addmem(dstp, alcselem(&ep->setmem,  ep->hnum), hook);
               ep = (struct b_selem *) BLKLOC(ep->sblink);
               }
            }
         }
      else {
      skipsets:
#endif SETS

   hneed(sizeof(struct b_cset));

   /*
    * x and y must be csets.
    */
   if (cvcset(&arg1, &cs1, csbuf1) == NULL)
      runerr(104, &arg1);
   if (cvcset(&arg2, &cs2, csbuf2) == NULL)
      runerr(104, &arg2);

   /*
    * Allocate a new cset and in each word of it, compute the value
    *  of the bitwise union of the corresponding words in the
    *  x and y csets.
    */
   bp = (union block *) alccset();
   for (i = 0; i < CSETSIZE; i++)
       bp->cset.bits[i] = cs1[i] | cs2[i];

   arg0.type = D_CSET;
   BLKLOC(arg0) = bp;
#ifdef SETS
   }
#endif SETS
   ClearBound;
   }

Opblock(unioncs,2,"++")
