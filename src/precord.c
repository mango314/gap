/****************************************************************************
**
*W  precord.c                   GAP source                   Martin Schönert
**
**
*Y  Copyright (C)  1996,  Lehrstuhl D für Mathematik,  RWTH Aachen,  Germany
*Y  (C) 1998 School Math and Comp. Sci., University of St Andrews, Scotland
*Y  Copyright (C) 2002 The GAP Group
**
**  This file contains the functions for plain records.
**
**  A plain record  with <n>  components is stored  as  a bag  with 2*<n>+2
**  entries.  The odd entries are the record  names of the components and the
**  even entries are the corresponding values.
**  The first pointer in the bag is reserved for the pointer to the type
**  object if the record becomes a component object. The second slot
**  in the bag is the number <n> of components as a C int.
**  Normally, the rnams are sorted such that we can do a binary lookup.
**  However, we allow for lazy sorting. This is done as follows: New
**  components are always appended (record bags grow by a factor of 5/4
**  if needed as plists do), already sorted rnams are stored by storing
**  their negative value to indicate sortedness. The new entries will have
**  positive rnams and can thus be distinguished. Every read access will
**  clean up the mess by sorting the new part and then merging the two
**  sorted areas. After that, all rnams are negative indicating sortedness.
**
*/
#include        <stdlib.h>              /* for qsort */
#include        <sys/time.h>            /* for gettimeofday() */
#include        "system.h"              /* system dependent part           */


#include        "gasman.h"              /* garbage collector               */
#include        "objects.h"             /* objects                         */
#include        "scanner.h"             /* scanner                         */

#include        "gap.h"                 /* error handling, initialisation  */

#include        "gvars.h"               /* global variables                */
#include        "calls.h"               /* generic call mechanism          */
#include        "opers.h"               /* generic operations              */

#include        "ariths.h"              /* basic arithmetic                */
#include        "records.h"             /* generic records                 */
#include        "lists.h"               /* generic lists                   */
#include        "integer.h"             /* arbitrary size integers         */

#include        "bool.h"                /* booleans                        */

#include        "precord.h"             /* plain records                   */

#include        "plist.h"               /* plain lists                     */
#include        "string.h"              /* strings                         */

#include        "saveload.h"            /* saving and loading              */

#include	"code.h"		/* coder                           */
#include	"thread.h"		/* threads			   */
#include	"tls.h"			/* thread-local storage		   */
#include	"aobjects.h"		/* thread-local storage		   */


/****************************************************************************
**

*F * * * * * * * * * standard functions for plain records * * * * * * * * * *
*/


/****************************************************************************
**

*F  TypePRec( <rec> ) . . . . . . . . . . . . . . . .  kind of a plain record
**
**  'TypePRec' returns the kind of the plain record <rec>.
**
**  'TypePRec' is the function in 'TypeObjFuncs' for plain records.
*/
Obj TYPE_PREC_MUTABLE;

Obj TypePRecMut (
    Obj                 prec )
{
    return TYPE_PREC_MUTABLE;
}


Obj TYPE_PREC_IMMUTABLE;

Obj TypePRecImm (
    Obj                 prec )
{
    return TYPE_PREC_IMMUTABLE;
}

/****************************************************************************
**
*F  SetTypePRecToComObj( <rec>, <kind> )  convert record to component object
**
*/

void SetTypePRecToComObj( Obj rec, Obj kind )
{
  TYPE_COMOBJ(rec) = kind;
  RetypeBag(rec, T_COMOBJ);
  CHANGED_BAG(rec);
}


/****************************************************************************
**
*F  IsMutablePRecYes( <rec> ) . . . . . . mutability test for mutable records
*F  IsMutablePRecNo( <rec> )  . . . . . mutability test for immutable records
**
**  'IsMutablePRecYes' simply returns 1.  'IsMutablePRecNo' simply returns 0.
**  Note that we can decide from the type number whether  a record is mutable
**  or immutable.
**
**  'IsMutablePRecYes'  is the function   in 'IsMutableObjFuncs'  for mutable
**  records.   'IsMutablePRecNo' is  the function  in 'IsMutableObjFuncs' for
**  immutable records.
*/
Int IsMutablePRecYes (
    Obj                 rec )
{
    return 1;
}

Int IsMutablePRecNo (
    Obj                 rec )
{
    return 0;
}


/****************************************************************************
**
*F  IsCopyablePRecYes( <rec> )  . . . . . . . .  copyability test for records
**
**  'IsCopyablePRec' simply returns 1.  Note that all records are copyable.
**
**  'IsCopyablePRec' is the function in 'IsCopyableObjFuncs' for records.
*/
Int IsCopyablePRecYes (
    Obj                 rec )
{
    return 1;
}

/****************************************************************************
**
*F  NEW_PREC( <len> ) . . . . . . . . . . . . . . . . make a new plain record
**
**  'NEW_PREC' returns a new plain record with room for <len> components,
**  already set to length len.
*/
Obj NEW_PREC(UInt len)
{
    Obj o = NewBag( T_PREC, (len) * 2*sizeof(Obj) + 2*sizeof(Obj) );
    SET_LEN_PREC(o,0);
    return o;
}

/****************************************************************************
**
*F  GrowPRec( <rec>, <len> ) . . . .  make sure <rec> can hold <len> components
**
**  Returns 0 if nothing changed and 1 if enlarged.
*/
Int             GrowPRec (
    Obj                 rec,
    UInt                need )
{
    UInt                newsize, want, good;

    /* check if big enough */
    want = (2*need+2)*sizeof(Obj);
    if (SIZE_OBJ(rec) >= want) return 0L;


    /* find out how large the bag should become at least                   */
    good = ((5 * LEN_PREC(rec) + 3)/4 + 1) * 2 * sizeof(Obj) + 2;

    /* but maybe we need more                                              */
    newsize = (want < good) ? good : want;

    /* resize the plain list                                               */
    ResizeBag( rec, newsize );

    return 1L;
}

/****************************************************************************
**
*F  CopyPRec( <rec> ) . . . . . . . . . . . . . . . . . . copy a plain record
*F  CleanPRec( <rec> )  . . . . . . . . . . . . . . . clean up a plain record
**
**  'CopyPRec' returns a structural (deep) copy  of the record <rec>, i.e., a
**  recursive copy that preserves the structure.
**
**  If <rec>  has not yet  been  copied, it makes a   copy, leaves a  forward
**  pointer to the copy in  the first entry   of the record, where the  first
**  record name usually resides,  and copies all the  entries.  If the record
**  has alread been copied, it returns the value of the forwarding pointer.
**
**  'CopyPRec' is the function in 'TabCopyObj' for records.
**
**  'CleanPRec' removes the  mark and the forwarding  pointer from the record
**  <rec>.
**
**  'CleanPRec' is the function in 'TabCleanObj' for records.
*/
Obj CopyPRec (
    Obj                 rec,
    Int                 mut )
{
    Obj                 copy;           /* copy, result                    */
    Obj                 tmp;            /* temporary variable              */
    UInt                i;              /* loop variable                   */

    /* don't change immutable objects                                      */
    if ( ! IS_MUTABLE_OBJ(rec) ) {
        return rec;
    }

    /* if an empty record has not yet been copied                          */
    if ( LEN_PREC(rec) == 0 ) {

        /* make a copy                                                     */
        if ( mut ) {
            copy = NewBag( TNUM_OBJ(rec), SIZE_OBJ(rec) );
        }
        else {
            copy = NewBag( IMMUTABLE_TNUM(TNUM_OBJ(rec)), SIZE_OBJ(rec) );
        }

        /* leave a forwarding pointer                                      */
        ResizeBag( rec, SIZE_OBJ(rec) + sizeof(Obj) );
        SET_RNAM_PREC( rec, 1, (UInt)copy );
        CHANGED_BAG( rec );

        /* now it is copied                                                */
        RetypeBag( rec, TNUM_OBJ(rec) + COPYING );
    }

    /* if the record has not yet been copied                               */
    else {

        /* make a copy                                                     */
        if ( mut ) {
            copy = NewBag( TNUM_OBJ(rec), SIZE_OBJ(rec) );
        }
        else {
            copy = NewBag( IMMUTABLE_TNUM(TNUM_OBJ(rec)), SIZE_OBJ(rec) );
        }
        SET_RNAM_PREC( copy, 1, GET_RNAM_PREC( rec, 1 ) );

        /* leave a forwarding pointer                                      */
        SET_RNAM_PREC( rec, 1, (UInt)copy );
        CHANGED_BAG( rec );

        /* now it is copied                                                */
        RetypeBag( rec, TNUM_OBJ(rec) + COPYING );

        /* copy the subvalues                                              */
        tmp = COPY_OBJ( GET_ELM_PREC( rec, 1 ), mut );
        SET_LEN_PREC( copy, LEN_PREC(rec) );
        SET_ELM_PREC( copy, 1, tmp );
        CHANGED_BAG( copy );
        for ( i = 2; i <= LEN_PREC(copy); i++ ) {
            SET_RNAM_PREC( copy, i, GET_RNAM_PREC( rec, i ) );
            tmp = COPY_OBJ( GET_ELM_PREC( rec, i ), mut );
            SET_ELM_PREC( copy, i, tmp );
            CHANGED_BAG( copy );
        }

    }

    /* return the copy                                                     */
    return copy;
}

Obj CopyPRecCopy (
    Obj                 rec,
    Int                 mut )
{
    return (Obj)GET_RNAM_PREC( rec, 1 );
}

void CleanPRec (
    Obj                 rec )
{
}

void CleanPRecCopy (
    Obj                 rec )
{
    UInt                i;              /* loop variable                   */

    /* empty record                                                        */
    if ( LEN_PREC(rec) == 0 ) {

        /* remove the forwarding pointer                                   */
        ResizeBag( rec, SIZE_OBJ(rec) - sizeof(Obj) );

        /* now it is cleaned                                               */
        RetypeBag( rec, TNUM_OBJ(rec) - COPYING );
    }

    /* nonempty record                                                     */
    else {

        /* remove the forwarding pointer                                   */
        SET_RNAM_PREC( rec, 1, GET_RNAM_PREC( GET_RNAM_PREC( rec, 1 ), 1 ) );

        /* now it is cleaned                                               */
        RetypeBag( rec, TNUM_OBJ(rec) - COPYING );

        /* clean the subvalues                                             */
        CLEAN_OBJ( GET_ELM_PREC( rec, 1 ) );
        for ( i = 2; i <= LEN_PREC(rec); i++ ) {
            CLEAN_OBJ( GET_ELM_PREC( rec, i ) );
        }
    }
}

/****************************************************************************
**
*F  MakeImmutablePRec( <rec> )
*/

void MakeImmutablePRec( Obj rec)
{
  UInt len;
  UInt i;
  len = LEN_PREC( rec );
  for ( i = 1; i <= len; i++ )
    MakeImmutable(GET_ELM_PREC(rec,i));
  RetypeBag(rec, IMMUTABLE_TNUM(TNUM_OBJ(rec)));
}


/****************************************************************************
 * FindPRec( <rec>, <rnam>, <pos>, <cleanup> )
 *   . . . . . . . . . . . . . . . . . find a component name by binary search
 *
 * Searches rnam in rec, sets pos to the position where it is found (return
 * value 1) or where it should be inserted if it is not found (return val 0).
 * If cleanup is nonzero, a dirty record is automatically cleaned up.
 * If cleanup is 0, this does not happen.
 */

UInt FindPRec( Obj rec, UInt rnam, UInt *pos, int cleanup )
{
    /* This only assumes that the rnam values in the record are sorted! */
    UInt i;
    Int rnam2;
    UInt low = 1;
    UInt high;

    high = LEN_PREC(rec);
    if (high > 0 && (Int) (GET_RNAM_PREC(rec,high)) > 0) {
        /* DIRTY! Not everything sorted! */
        if (cleanup) {
            SortPRecRNam(rec,0);
            /* Note that this does not change the length and it cannot
             * trigger a garbage collection if cleanup is 1!
             * We do not want record accesses to trigger garbage
             * collections! */
        } else {
            /* We are not allowed to cleanup, so we live with it, we
             * first try to find rnam in the mess at the end, then
             * fall back to binary search: */
            i = high;
            while (i >= 1) {
                rnam2 = (Int) (GET_RNAM_PREC(rec,i));
                if (rnam == rnam2) {
                    *pos = i;
                    return 1;
                }
                if (rnam2 < 0) { /* reached the sorted area! */
                    high = i;  /* will be incremented by 1 */
                    break;
                }
                i--;
            }
            if (i == 0) return 0;
            /* Here, high is the number of the sorted entries. */
        }
    }
    high++;
    while (low < high) {
        i = (low + high) >> 1;   /* we always have low <= i < high */
        rnam2 = -(Int)(GET_RNAM_PREC( rec, i ));
        if (rnam2 < rnam) low = i+1;
        else if (rnam2 > rnam) high = i;
        else {
            /* found! */
            *pos = i;
            return 1;
        }
    }
    /* Now low == high and we did not find it. */
    *pos = low;
    return 0;
}

/****************************************************************************
**
*F  IsbPRec( <rec>, <rnam> )  . . . . test for an element from a plain record
**
**  'IsbPRec' returns 1 if the record <rec> has a component with  the  record
**  name <rnam>, and 0 otherwise.
*/
Int IsbPRec (
    Obj                 rec,
    UInt                rnam )
{
    UInt                i;              /* loop variable                   */

    return FindPRec(rec,rnam,&i,1);
}


/****************************************************************************
**
*F  ElmPRec( <rec>, <rnam> )  . . . . . select an element from a plain record
**
**  'ElmPRec' returns the element, i.e., the value of the component, with the
**  record name <rnam> in  the plain record <rec>.   An error is signalled if
**  <rec> has no component with record name <rnam>.
*/
Obj ElmPRec (
    Obj                 rec,
    UInt                rnam )
{
    UInt                i;              /* loop variable                   */

    if (FindPRec(rec,rnam,&i,1))
        return GET_ELM_PREC( rec, i );
    else {
        ErrorReturnVoid(
            "Record: '<rec>.%s' must have an assigned value",
            (Int)NAME_RNAM(rnam), 0L,
            "you can 'return;' after assigning a value" );
        return ELM_REC( rec, rnam );
    }
}


/****************************************************************************
**
*F  UnbPRec( <rec>, <rnam> )  . unbind a record component from a plain record
**
**  'UnbPRec'  removes the record component  with the record name <rnam> from
**  the record <rec>.
*/
void UnbPRec (
    Obj                 rec,
    UInt                rnam )
{
    UInt                len;            /* length of <rec>                 */
    UInt                i;              /* loop variable                   */

    if (FindPRec( rec, rnam, &i, 1 )) {
        /* otherwise move everything forward                               */
        len = LEN_PREC( rec );
        for ( ; i < len; i++ ) {
            SET_RNAM_PREC( rec, i, GET_RNAM_PREC( rec, i+1 ) );
            SET_ELM_PREC(  rec, i, GET_ELM_PREC(  rec, i+1 ) );
        }
        SET_RNAM_PREC( rec, len, 0 );
        SET_ELM_PREC( rec, len, 0L );


        /* resize the record                                               */
        SET_LEN_PREC(rec,LEN_PREC(rec)-1);

    } else
        /* do nothing if no such component exists                          */
        return;
}

void            UnbPRecImm (
    Obj                 rec,
    UInt                rnam )
{
    ErrorReturnVoid(
        "Record Unbind: <rec> must be a mutable record",
        0L, 0L,
        "you can 'return;' and ignore the unbind" );
}


/****************************************************************************
**
*F  AssPRec( <rec>, <rnam>, <val> ) . . . . . . . .  assign to a plain record
**
**  'AssPRec' assigns the value <val> to the record component with the record
**  name <rnam> in the plain record <rec>.
*/
void AssPRec (
    Obj                 rec,
    UInt                rnam,
    Obj                 val )
{
    UInt                len;            /* length of <rec>                 */
    UInt                i;              /* loop variable                   */

    /* get the length of the record                                        */
    len = LEN_PREC( rec );

    if (len % 1000 == 0) {   /* A hack to occasionally do some cleanup! */
        SortPRecRNam(rec,0);
    }

    if (!FindPRec( rec, rnam, &i, 0 )) {
        /* No cleanup allowed here to allow for multiple assignments! */
        /* extend the record if no such component exists                   */
        len++;
        GrowPRec(rec, len);
        i = len;
        SET_RNAM_PREC( rec, i, rnam );
        SET_LEN_PREC(rec,len);
    }
    /* assign the value to the component                                   */
    SET_ELM_PREC( rec, i, val );
    CHANGED_BAG( rec );
}

void            AssPRecImm (
    Obj                 rec,
    UInt                rnam,
    Obj                 val )
{
    ErrorReturnVoid(
        "Records Assignment: <rec> must be a mutable record",
        0L, 0L,
        "you can 'return;' and ignore the assignment" );
}

/****************************************************************************
**
*F  PrintPRec( <rec> )  . . . . . . . . . . . . . . . . . . .  print a record
**
**  'PrintRec' prints the plain record <rec>.
*/
extern Obj PrintObjOper;

void PrintPRec (
    Obj                 rec )
{
    DoOperation1Args( PrintObjOper, rec );
}



/****************************************************************************
**
*F  SortPRecRNam(<rec>, <inplace>) . . . . . . . sort the Rnams of the record
**
**  This is needed after the components of a record have been assigned
**  in not necessarily sorted order in the kernel. It is automatically
**  called on the first read access if necessary. See the top of "precord.c"
**  for a comment on lazy sorting.
**  If inplace is 1 then a slightly slower algorithm is used of
**  which we know that it does not produce garbage collections.
**  If inplace is 0 a garbage collection may be triggered.
**
*/
static int PrecComparer(const void *a, const void *b)
{
    const UInt *aa = (const UInt *) a;
    const UInt *bb = (const UInt *) b;
    if (*aa < *bb) return -1;
    else if (*aa == *bb) return 0;
    else return 1;
}

void SortPRecRNam (
    Obj                 rec, int inplace )
{
    UInt len = LEN_PREC(rec);
    UInt i,j,k,save;
    int issorted = 1;
    Obj space;
    Obj tmp;

    /* Nothing has to be done if it is already sorted: */
    if ( len == 0 || (Int) (GET_RNAM_PREC(rec,len)) < 0) return;

    /* First find the "unsorted part" and check whether it is sorted! */
    for (i = len-1;i >= 1 && (Int)(GET_RNAM_PREC(rec,i)) > 0;i--) {
        if (issorted && GET_RNAM_PREC(rec,i) > GET_RNAM_PREC(rec,i+1)) {
            issorted = 0;
        }
    }
    i++;
    /* Now i points to the last positive (and thus still unsorted)
     * component and issorted is 1 iff positions i to len are sorted! */
    if (!issorted && len - i + 1 > 1) {
        qsort( ADDR_OBJ(rec)+2*i, len-i+1, 2*sizeof(Obj), &PrecComparer );
    }
    /* Now i points to the first positive component and the rest is
     * sorted. */
    save = i;
    if (save == 1 ||
        -(Int)(GET_RNAM_PREC(rec,save-1)) < GET_RNAM_PREC(rec,save)) {
        /* Otherwise, nothing has to be done since it is already
         * sorted, we only have to negate the RNams! */
        for (j = save;j <= len;j++)
            SET_RNAM_PREC(rec,j,-(Int)(GET_RNAM_PREC(rec,j)));
        return;
    }
    /* Next we perform a merge sort on the two presorted areas. */
    /* For optimal performance, we need some space to mess around: */
    if (!inplace) {
        space = NEW_PREC(len);
        j = 1;
        k = 1;
        while (j < save && i <= len) {
            if (-(Int)(GET_RNAM_PREC(rec,j)) < GET_RNAM_PREC(rec,i)) {
                SET_RNAM_PREC(space,k,GET_RNAM_PREC(rec,j));
                SET_ELM_PREC(space,k,GET_ELM_PREC(rec,j));
                j++; k++;
            } else {
                SET_RNAM_PREC(space,k,-(Int)(GET_RNAM_PREC(rec,i)));
                SET_ELM_PREC(space,k,GET_ELM_PREC(rec,i));
                i++; k++;
            }
        }
        /* Copy the rest of the part still missing: */
        while (j < save) {
            SET_RNAM_PREC(space,k,GET_RNAM_PREC(rec,j));
            SET_ELM_PREC(space,k,GET_ELM_PREC(rec,j));
            j++; k++;
        }
        while (i <= len) {
            SET_RNAM_PREC(space,k,-(Int)(GET_RNAM_PREC(rec,i)));
            SET_ELM_PREC(space,k,GET_ELM_PREC(rec,i));
            i++; k++;
        }
        /* Finally, copy everything back to where it came from: */
        memcpy(ADDR_OBJ(rec)+2,ADDR_OBJ(space)+2,sizeof(Obj)*2*len);
    } else {   /* We have to work in place to avoid a garbage collection. */
        /* i == save is the cut point */
        j = 1;
        for (j = 1; j < save; j++) {
            if (-(Int)(GET_RNAM_PREC(rec,j)) > GET_RNAM_PREC(rec,i)) {
                /* we have to move something to position j! */
                tmp = (Obj) (-(Int)(GET_RNAM_PREC(rec,j)));
                SET_RNAM_PREC(rec,j,-(Int)(GET_RNAM_PREC(rec,i)));
                SET_RNAM_PREC(rec,i,(UInt) tmp);
                tmp = GET_ELM_PREC(rec,j);
                SET_ELM_PREC(rec,j,GET_ELM_PREC(rec,i));
                SET_ELM_PREC(rec,i,tmp);
                /* Now we have to "bubble pos i up" until it is in the
                 * right position: */
                for (k = i;k < len;k++) {
                    if (GET_RNAM_PREC(rec,k) > GET_RNAM_PREC(rec,k+1)) {
                        tmp = (Obj) GET_RNAM_PREC(rec,k);
                        SET_RNAM_PREC(rec,k,GET_RNAM_PREC(rec,k+1));
                        SET_RNAM_PREC(rec,k+1,(UInt) tmp);
                        tmp = GET_ELM_PREC(rec,k);
                        SET_ELM_PREC(rec,k,GET_ELM_PREC(rec,k+1));
                        SET_ELM_PREC(rec,k+1,tmp);
                    } else break;
                }
            }
        }
        /* Finally, we have to negate everything in the end: */
        for (j = save;j <= len;j++)
            SET_RNAM_PREC(rec,j,-(Int)(GET_RNAM_PREC(rec,j)));
    }
}

#if 0
void SortPRec ( Obj rec )
{
    UInt                rnam;           /* name of component               */
    Obj                 val;            /* value of component              */
    UInt                h;              /* gap width in shellsort          */
    UInt                i,  k;          /* loop variables                  */

    /* sort the right record with a shellsort                              */
    h = 1;  while ( 9*h + 4 < LEN_PREC(rec) )  h = 3*h + 1;
    while ( 0 < h ) {
        for ( i = h+1; i <= LEN_PREC(rec); i++ ) {
            rnam = GET_RNAM_PREC( rec, i );
            val  = GET_ELM_PREC(  rec, i );
            k = i;
            while ( h < k
                 && strcmp( NAME_RNAM(rnam),
                              NAME_RNAM( GET_RNAM_PREC(rec,k-h) ) ) < 0 ) {
                SET_RNAM_PREC( rec, k, GET_RNAM_PREC( rec, k-h ) );
                SET_ELM_PREC(  rec, k, GET_ELM_PREC(  rec, k-h ) );
                k -= h;
            }
            SET_RNAM_PREC( rec, k, rnam );
            SET_ELM_PREC(  rec, k, val  );
        }
        h = h / 3;
    }

}
#endif

/****************************************************************************
**

*F * * * * * * * * * * * default functions for records  * * * * * * * * * * *
*/


/****************************************************************************
**

*F  MethodPRec( <rec>, <rnam> ) . . . . . . . . . . get a method for a record
**
**  'MethodPRec' returns  the function in the  component with the record name
**  <rnam> in the  record in the component  with the name 'operations' (which
**  really should be  called 'methods') of  the record <rec>  if this exists.
**  Otherwise it return 0.
*/
UInt OperationsRNam;                    /* 'operations' record name        */
UInt COMPONENTSRNam;                    /* COMPONENTS record name          */

Obj MethodPRec (
    Obj                 rec,
    UInt                rnam )
{
    Obj                 method;         /* method, result                  */
    Obj                 opers;          /* operations record               */
    UInt                i;              /* loop variable                   */

    /* is <rec> a record?                                                  */
    if ( ! IS_PREC_REP(rec) )
        return 0;

    /* try to get the operations record                                    */
    if (!FindPRec(rec,OperationsRNam,&i,1)) {
        return 0;
    }
    opers = GET_ELM_PREC( rec, i );

    /* check for an Operations Record object */
    if ( TNUM_OBJ(opers) == T_COMOBJ)
      {
        /* Make use of the fact the Component objects look like Precs */
        if ( !FindPRec(opers,COMPONENTSRNam,&i,1) ) {
          return 0;
        }
        opers = GET_ELM_PREC( opers, i );
      }


    if ( ! IS_PREC_REP(opers) ) {
        return 0;
    }

    /* try to get the method                                               */
    if ( !FindPRec(opers,rnam,&i,1) ) {
        return 0;
    }
    method = GET_ELM_PREC( opers, i );

    /* return the method                                                   */
    return method;
}

void PrintPathPRec (
    Obj                 rec,
    Int                 indx )
{
    Pr( ".%I", (Int)NAME_RNAM( labs((Int)(GET_RNAM_PREC(rec,indx))) ), 0L );
}

/****************************************************************************
**
*F  FuncREC_NAMES( <self>, <rec> )  . . . . . . . .  record names of a record
**
**  'FuncREC_NAMES' implements a method for the operations 'RecNames'.
**
**  'RecNames( <rec> )'
**
**  'RecNames'  returns a list containing the  names of the components of the
**  record <rec> as strings.
*/
Obj InnerRecNames( Obj rec )
{
    Obj                 list;           /* list of record names, result    */
    UInt                rnam;           /* one name of record              */
    Obj                 string;         /* one name as string              */
    UInt                i, len;

    SortPRecRNam(rec,0);   /* Make sure rnams are sorted and thus negative */

    /* allocate the list                                                   */
    list = NEW_PLIST( T_PLIST, LEN_PREC(rec) );
    SET_LEN_PLIST( list, LEN_PREC(rec) );

    /* loop over the components                                            */
    for ( i = 1; i <= LEN_PREC(rec); i++ ) {
        rnam = -(Int)(GET_RNAM_PREC( rec, i ));
        len = strlen(NAME_RNAM(rnam));
        /* could have been moved by garbage collection */
        C_NEW_STRING( string, len, NAME_RNAM(rnam) );
        SET_ELM_PLIST( list, i, string );
        CHANGED_BAG( list );
    }

    /* return the list                                                     */
    return list;
}

Obj FuncREC_NAMES (
    Obj                 self,
    Obj                 rec )
{
    /* check the argument                                                  */
    UInt tnum;
    for (;;) {
      tnum = TNUM_OBJ(rec);
      switch (tnum) {
        case T_PREC:
	case T_PREC+IMMUTABLE:
	  return InnerRecNames(rec);
	case T_AREC:
	  return InnerRecNames(FromAtomicRecord(rec));
      }
      rec = ErrorReturnObj(
	  "RecNames: <rec> must be a record (not a %s)",
	  (Int)TNAM_OBJ(rec), 0L,
	  "you can replace <rec> via 'return <rec>;'" );
    }
}


/****************************************************************************
**
*F  FuncREC_NAMES_COMOBJ( <self>, <rec> ) . . . record names of a record object
*/
/* same as FuncREC_NAMES except for different argument check  */
Obj FuncREC_NAMES_COMOBJ (
    Obj                 self,
    Obj                 rec )
{
    /* check the argument                                                  */
    UInt tnum;
    for (;;) {
      tnum = TNUM_OBJ(rec);
      if (tnum == T_COMOBJ)
        return InnerRecNames(rec);
      if (tnum == T_ACOMOBJ) {
        rec = FromAtomicRecord(rec);
	return InnerRecNames(rec);
      }
      rec = ErrorReturnObj(
	  "RecNames: <rec> must be a component object (not a %s)",
	  (Int)TNAM_OBJ(rec), 0L,
	  "you can replace <rec> via 'return <rec>;'" );
    }
}


/****************************************************************************
**
*F  FuncSUM_PREC( <self>, <left>, <right> ) . . . . . . .  sum of two records
**
**  'SumRec' returns the  sum  of the two   operands <left> and <right>.   At
**  least one of the operands must be a plain record.
**
**  If  at least one of the  operands is a  record and has a '.operations.\+'
**  method, than this  is called and its  result  is returned.
*/
UInt SumRNam;                           /* '+' record name                 */

Obj FuncSUM_PREC (
    Obj                 self,
    Obj                 left,
    Obj                 right )
{
    Obj                 method;         /* method                          */

    /* try to find an applicable method                                    */
    if ( ! (method = MethodPRec( right, SumRNam ))
      && ! (method = MethodPRec( left,  SumRNam )) )
    {
        return TRY_NEXT_METHOD;
    }

    /* call that function                                                  */
    return CALL_2ARGS( method, left, right );
}


/****************************************************************************
**
*F  FuncDIFF_PREC( <self>, <left>, <right> )  . . . difference of two records
**
**  'DiffRec' returns the difference of the two  operands <left> and <right>.
**  At least one of the operands must be a plain record.
**
**  If  at least one  of the operands is  a record and has a '.operations.\-'
**  method, then this  is called  and  its result is returned.
*/
UInt DiffRNam;                          /* '-' record name                 */

Obj FuncDIFF_PREC (
    Obj                 self,
    Obj                 left,
    Obj                 right )
{
    Obj                 method;           /* operation                       */

    /* try to find an applicable method                                    */
    if ( ! (method = MethodPRec( right, DiffRNam ))
      && ! (method = MethodPRec( left,  DiffRNam )) )
    {
        return TRY_NEXT_METHOD;
    }

    /* call that function                                                  */
    return CALL_2ARGS( method, left, right );
}


/****************************************************************************
**
*F  FuncPROD_PREC( <self>, <left>, <right> )  . . . .  product of two records
**
**  'ProdRec' returns the product of the two operands <left> and <right>.  At
**  least one of the operands must be a plain record.
**
**  If  at least one  of the operands is  a record and has a '.operations.\*'
**  method, then this  is called  and  its result is returned.
*/
UInt ProdRNam;                          /* '*' record name                 */

Obj FuncPROD_PREC (
    Obj                 self,
    Obj                 left,
    Obj                 right )
{
    Obj                 method;         /* method                          */

    /* try to find an applicable method                                    */
    if ( ! (method = MethodPRec( right, ProdRNam ))
      && ! (method = MethodPRec( left,  ProdRNam )) )
    {
        return TRY_NEXT_METHOD;
    }

    /* call that function                                                  */
    return CALL_2ARGS( method, left, right );
}


/****************************************************************************
**
*F  FuncQUO_PREC( <self>, <left>, <right> ) . . . . . quotient of two records
**
**  'QuoRec' returns the quotient of the two operands <left> and <right>.  At
**  least one of the operands must be a plain record.
**
**  If  at least one  of the operands is  a record and has a '.operations.\/'
**  method, then this  is called  and  its result is returned.
*/
UInt QuoRNam;                           /* '/' record name                 */

Obj FuncQUO_PREC (
    Obj                 self,
    Obj                 left,
    Obj                 right )
{
    Obj                 method;         /* method                          */

    /* try to find an applicable method                                    */
    if ( ! (method = MethodPRec( right, QuoRNam ))
      && ! (method = MethodPRec( left,  QuoRNam )) )
    {
        return TRY_NEXT_METHOD;
    }

    /* call that function                                                  */
    return CALL_2ARGS( method, left, right );
}


/****************************************************************************
**
*F  FuncLQUO_PREC( <self>, <left>, <right> )  .  left quotient of two records
**
**  'LQuoPRec' returns the   left quotient  of  the two   operands <left> and
**  <right>.  At least one of the operands must be a plain record.
**
**  If  at   least   one   of  the  operands     is a  record   and    has  a
**  '.operations.LeftQuotient' method, then this is  called and its result is
**  returned.
*/
UInt LQuoRNam;                          /* 'LeftQuotient' record name      */

Obj FuncLQUO_PREC (
    Obj                 self,
    Obj                 left,
    Obj                 right )
{
    Obj                 method;         /* method                          */

    /* try to find an applicable method                                    */
    if ( ! (method = MethodPRec( right, LQuoRNam ))
      && ! (method = MethodPRec( left,  LQuoRNam )) )
    {
        return TRY_NEXT_METHOD;
    }

    /* call that function                                                  */
    return CALL_2ARGS( method, left, right );
}


/****************************************************************************
**
*F  FuncPOW_PREC( <self>, <left>, <right> ) . . . . . .  power of two records
**
**  'PowPRec' returns the power of  the two operands  <left> and <right>.  At
**  least one of the operands must be a plain record.
**
**  If  at least one  of the operands is  a record and has a '.operations.\^'
**  method, then this  is called  and  its result is returned.
*/
UInt PowRNam;                           /* '^' record name                 */

Obj FuncPOW_PREC (
    Obj                 self,
    Obj                 left,
    Obj                 right )
{
    Obj                 method;         /* method                          */

    /* try to find an applicable method                                    */
    if ( ! (method = MethodPRec( right, PowRNam ))
      && ! (method = MethodPRec( left,  PowRNam )) )
    {
        return TRY_NEXT_METHOD;
    }

    /* call that function                                                  */
    return CALL_2ARGS( method, left, right );
}


/****************************************************************************
**
*F  FuncCOMM_PREC( <self>, <left>, <right> )  . .   commutator of two records
**
**  'CommPRec' returns the commutator of the two operands <left> and <right>.
**  At least one of the operands must be a plain record.
**
**  If at least one of the operands is  a record and has a '.operations.Comm'
**  method, then this  is called and its  result  is returned.
*/
UInt CommRNam;                          /* 'Comm' record name              */

Obj FuncCOMM_PREC (
    Obj                 self,
    Obj                 left,
    Obj                 right )
{
    Obj                 method;         /* method                          */

    /* try to find an applicable method                                    */
    if ( ! (method = MethodPRec( right, CommRNam ))
      && ! (method = MethodPRec( left,  CommRNam )) )
    {
        return TRY_NEXT_METHOD;
    }

    /* call that function                                                  */
    return CALL_2ARGS( method, left, right );
}


/****************************************************************************
**
*F  FuncMOD_PREC( <self>, <left>, <right> ) . . . .  remainder of two records
**
**  'ModPRec' returns the   remainder  the operands  <left>   by  the operand
**  <right>.  At least one of the operands must be a plain record.
**
**  If at least one of the operands is a  record and has a '.operations.\mod'
**  method, then this  is  called and its  result is  returned.
*/
UInt ModRNam;                /* 'mod' record name               */

Obj FuncMOD_PREC (
    Obj                 self,
    Obj                 left,
    Obj                 right )
{
    Obj                 method;         /* method                          */

    /* try to find an applicable method                                    */
    if ( ! (method = MethodPRec( right, ModRNam ))
      && ! (method = MethodPRec( left,  ModRNam )) )
    {
        return TRY_NEXT_METHOD;
    }

    /* call that function                                                  */
    return CALL_2ARGS( method, left, right );
}


/****************************************************************************
**
*F  FuncIN_PREC( <self>, <left>, <right> )  . . . membership test for records
**
**  'InRec' returns 'True' if the operand <left>  is an element of the record
**  <right>.  <right> must be a plain record.
**
**  If <right> has  a '.operations.\in' method, than this  is called  and its
**  result is returned.
*/
UInt InRNam;                            /* 'in' record name                */

Obj FuncIN_PREC (
    Obj                 self,
    Obj                 left,
    Obj                 right )
{
    Obj                 method;         /* method                          */

    /* try to find an applicable method                                    */
    if ( ! (method = MethodPRec( right, InRNam )) ) {
        return TRY_NEXT_METHOD;
    }

    /* call that function                                                  */
    return CALL_2ARGS( method, left, right );
}


/****************************************************************************
**
*F  FuncEQ_PREC_DEFAULT( <self>, <left>, <right> )  comparison of two records
**
**  'EqRec' returns '1L'  if the two  operands <left> and <right> are equal
**  and '0L' otherwise.  At least one operand must be a plain record.
*/
Obj FuncEQ_PREC_DEFAULT (
    Obj                 self,
    Obj                 left,
    Obj                 right )
{
    UInt                i;              /* loop variable                   */

    /* quick first checks                                                  */
    if ( ! IS_PREC_REP(left) )
        return False;
    if ( ! IS_PREC_REP(right) )
        return False;
    if ( LEN_PREC(left) != LEN_PREC(right) )
        return False;

    /* Records are now always sorted by their RNam */

    /* compare componentwise                                               */
    for ( i = 1; i <= LEN_PREC(right); i++ ) {

        /* compare the names                                               */
        if ( GET_RNAM_PREC(left,i) != GET_RNAM_PREC(right,i) ) {
            return False;
        }

        /* compare the values                                              */
        if ( ! EQ(GET_ELM_PREC(left,i),GET_ELM_PREC(right,i)) ) {
            return False;
        }
    }

    /* the records are equal                                               */
    return True;
}


/****************************************************************************
**
*F  FuncEQ_PREC( <self>, <left>, <right> )  . . . . comparison of two records
**
**  'EqRec' returns '1L'  if the two  operands <left> and <right> are equal
**  and '0L' otherwise.  At least one operand must be a plain record.
**
**  If at  least one of the  operands is a  record and has a '.operations.\='
**  method,  than this is called  and its result  is returned.  Otherwise the
**  records are compared componentwise.
*/
UInt EqRNam;                            /* '=' record name                 */

Obj FuncEQ_PREC (
    Obj                 self,
    Obj                 left,
    Obj                 right )
{
    Obj                 method;         /* method                          */

    /* try to find an applicable method                                    */
    if ( ! (method = MethodPRec( right, EqRNam ))
      && ! (method = MethodPRec( left,  EqRNam )) )
    {
        return FuncEQ_PREC_DEFAULT( self, left, right );
    }

    /* call that function                                                  */
    return CALL_2ARGS( method, left, right );
}


/****************************************************************************
**
*F  FuncLT_PREC_DEFAULT( <self>, <left>, <right> )  comparison of two records
**
**  'LtRec' returns '1L'  if the operand  <left> is  less than the  operand
**  <right>, and '0L'  otherwise.  At least  one operand  must be a  plain
**  record.
*/
Obj FuncLT_PREC_DEFAULT (
    Obj                 self,
    Obj                 left,
    Obj                 right )
{
    UInt                i;              /* loop variable                   */

    /* quick first checks                                                  */
    if ( ! IS_PREC_REP(left) || ! IS_PREC_REP(right) ) {
        if ( TNUM_OBJ(left ) < TNUM_OBJ(right) )  return True;
        if ( TNUM_OBJ(left ) > TNUM_OBJ(right) )  return False;
    }

    /* Records are now always sorted by their RNam */

    /* compare componentwise                                               */
    for ( i = 1; i <= LEN_PREC(right); i++ ) {

        /* if the left is a proper prefix of the right one                 */
        if ( LEN_PREC(left) < i )  return True;

        /* compare the names                                               */
        /* The sense of this comparison is determined by the rule that
           unbound entries compare less than bound ones                    */
        if ( GET_RNAM_PREC(left,i) != GET_RNAM_PREC(right,i) ) {
            if ( strcmp( NAME_RNAM( labs((Int)(GET_RNAM_PREC(left,i))) ),
                   NAME_RNAM( labs((Int)(GET_RNAM_PREC(right,i))) ) ) > 0 ) {
                return True;
            }
            else {
                return False;
            }
        }

        /* compare the values                                              */
        if ( ! EQ(GET_ELM_PREC(left,i),GET_ELM_PREC(right,i)) ) {
            return LT( GET_ELM_PREC(left,i), GET_ELM_PREC(right,i) ) ?
                   True : False;
        }

    }

    /* the records are equal or the right is a prefix of the left          */
    return False;
}


/****************************************************************************
**
*F  FuncLT_PREC( <self>, <left>, <right> )   . . .  comparison of two records
**
**  'LtRec' returns '1L'  if the operand  <left> is  less than the  operand
**  <right>, and '0L'  otherwise.  At least  one operand  must be a  plain
**  record.
**
**  If at least  one of the operands is  a record and  has a '.operations.\<'
**  method, than this  is called and its  result is  returned.  Otherwise the
**  records are compared componentwise.
*/
UInt LtRNam;                            /* '<' record name                 */

Obj FuncLT_PREC (
    Obj                 self,
    Obj                 left,
    Obj                 right )
{
    Obj                 method;         /* method                          */

    /* try to find an applicable method                                    */
    if ( ! (method = MethodPRec( right, LtRNam ))
      && ! (method = MethodPRec( left,  LtRNam )) )
    {
        return FuncLT_PREC_DEFAULT( self, left, right );
    }

    /* call that function                                                  */
    return CALL_2ARGS( method, left, right );
}


/****************************************************************************
**
*F  SavePRec( <prec> )
**
*/

void SavePRec( Obj prec )
{
  UInt len,i;
  len = LEN_PREC(prec);
  SaveUInt(len);
  for (i = 1; i <= len; i++)
    {
      SaveUInt(GET_RNAM_PREC(prec, i));
      SaveSubObj(GET_ELM_PREC(prec, i));
    }
  return;
}

/****************************************************************************
**
*F  LoadPRec( <prec> )
**
*/

void LoadPRec( Obj prec )
{
  UInt len,i;
  len = LoadUInt();
  SET_LEN_PREC(prec,len);
  for (i = 1; i <= len; i++)
    {
      SET_RNAM_PREC(prec, i, LoadUInt());
      SET_ELM_PREC(prec, i, LoadSubObj());
    }
  return;
}

/* Temporarily borrowed by AK from the IO package for timing
   (see also an entry for this in GVarFuncs below) */

#ifndef USE_GMP
static Obj MyObjInt_Int(Int i)
{
    Obj n;
    Int bound = 1L << NR_SMALL_INT_BITS;
    if (i >= bound) {
        /* We have to make a big integer */
        n = NewBag(T_INTPOS,4*sizeof(TypDigit));
        ADDR_INT(n)[0] = (TypDigit) (i & ((Int) INTBASE - 1L));
        ADDR_INT(n)[1] = (TypDigit) (i >> NR_DIGIT_BITS);
        ADDR_INT(n)[2] = 0;
        ADDR_INT(n)[3] = 0;
        return n;
    } else if (-i > bound) {
        n = NewBag(T_INTNEG,4*sizeof(TypDigit));
        ADDR_INT(n)[0] = (TypDigit) ((-i) & ((Int) INTBASE - 1L));
        ADDR_INT(n)[1] = (TypDigit) ((-i) >> NR_DIGIT_BITS);
        ADDR_INT(n)[2] = 0;
        ADDR_INT(n)[3] = 0;
        return n;
    } else {
        return INTOBJ_INT(i);
    }
}
#else
#define MyObjInt_Int(i) ObjInt_Int(i)
#endif

Obj Func_gettimeofday( Obj self )
{
   Obj tmp;
   struct timeval tv;
   gettimeofday(&tv, NULL);
   tmp = NEW_PREC(0);
   AssPRec(tmp, RNamName("tv_sec"), MyObjInt_Int( tv.tv_sec ));
   AssPRec(tmp, RNamName("tv_usec"), MyObjInt_Int( tv.tv_usec ));
   return tmp;
}

/****************************************************************************
**

*F * * * * * * * * * * * * * initialize package * * * * * * * * * * * * * * *
*/

/****************************************************************************
**

*V  BagNames  . . . . . . . . . . . . . . . . . . . . . . . list of bag names
*/
static StructBagNames BagNames[] = {
  { T_PREC,                     "record (plain)"            },
  { T_PREC +IMMUTABLE,          "record (plain,imm)"        },
  { T_PREC            +COPYING, "record (plain,copied)"     },
  { T_PREC +IMMUTABLE +COPYING, "record (plain,imm,copied)" },
  { -1,                         ""                          }
};


/****************************************************************************
**
*V  GVarFuncs . . . . . . . . . . . . . . . . . . list of functions to export
*/
static StructGVarFunc GVarFuncs [] = {

    { "REC_NAMES", 1, "rec",
      FuncREC_NAMES, "src/precord.c:REC_NAMES" },

    { "REC_NAMES_COMOBJ", 1, "rec obj",
      FuncREC_NAMES_COMOBJ, "src/precord.c:REC_NAMES_COMOBJ" },

    { "SUM_PREC", 2, "left, right",
      FuncSUM_PREC, "src/precord.c:SUM_PREC" },

    { "DIFF_PREC", 2, "left, right",
      FuncDIFF_PREC, "src/precord.c:DIFF_PREC" },

    { "PROD_PREC", 2, "left, right",
      FuncPROD_PREC, "src/precord.c:PROD_PREC" },

    { "QUO_PREC", 2, "left, right",
      FuncQUO_PREC, "src/precord.c:QUO_PREC" },

    { "LQUO_PREC", 2, "left, right",
      FuncLQUO_PREC, "src/precord.c:LQUO_PREC" },

    { "POW_PREC", 2, "left, right",
      FuncPOW_PREC, "src/precord.c:POW_PREC" },

    { "MOD_PREC", 2, "left, right",
      FuncMOD_PREC, "src/precord.c:MOD_PREC" },

    { "COMM_PREC", 2, "left, right",
      FuncCOMM_PREC, "src/precord.c:COMM_PREC" },

    { "IN_PREC", 2, "left, right",
      FuncIN_PREC, "src/precord.c:IN_PREC" },

    { "EQ_PREC", 2, "left, right",
      FuncEQ_PREC, "src/precord.c:EQ_PREC" },

    { "EQ_PREC_DEFAULT",2,"left, right",
      FuncEQ_PREC_DEFAULT, "src/precord.c:EQ_PREC_DEFAULT" },

    { "LT_PREC", 2, "left, right",
      FuncLT_PREC, "src/precord.c:LT_PREC" },

    { "LT_PREC_DEFAULT",2,"left, right",
      FuncLT_PREC_DEFAULT, "src/precord.c:LT_PREC_DEFAULT" },

    { "CurrentTime", 0, "",
      Func_gettimeofday, "src/precord.c:CurrentTime" },

    { 0 }

};


/****************************************************************************
**

*F  InitKernel( <module> )  . . . . . . . . initialise kernel data structures
*/
static Int InitKernel (
    StructInitInfo *    module )
{
    /* check dependencies                                                  */
    RequireModule( module, "records", 503600000UL );

    /* GASMAN marking functions and GASMAN names                           */
    InitBagNamesFromTable( BagNames );

    InitMarkFuncBags( T_PREC                     , MarkAllSubBags );
    InitMarkFuncBags( T_PREC +IMMUTABLE          , MarkAllSubBags );
    InitMarkFuncBags( T_PREC            +COPYING , MarkAllSubBags );
    InitMarkFuncBags( T_PREC +IMMUTABLE +COPYING , MarkAllSubBags );

    /* Immutable records are public					   */
    MakeBagTypePublic( T_PREC + IMMUTABLE );

    /* init filters and functions                                          */
    InitHdlrFuncsFromTable( GVarFuncs );

    /* Install saving functions                                            */
    SaveObjFuncs[ T_PREC            ] = SavePRec;
    SaveObjFuncs[ T_PREC +IMMUTABLE ] = SavePRec;
    LoadObjFuncs[ T_PREC            ] = LoadPRec;
    LoadObjFuncs[ T_PREC +IMMUTABLE ] = LoadPRec;

    /* install into record function tables                                 */
    ElmRecFuncs[ T_PREC            ] = ElmPRec;
    ElmRecFuncs[ T_PREC +IMMUTABLE ] = ElmPRec;
    IsbRecFuncs[ T_PREC            ] = IsbPRec;
    IsbRecFuncs[ T_PREC +IMMUTABLE ] = IsbPRec;
    AssRecFuncs[ T_PREC            ] = AssPRec;
    AssRecFuncs[ T_PREC +IMMUTABLE ] = AssPRecImm;
    UnbRecFuncs[ T_PREC            ] = UnbPRec;
    UnbRecFuncs[ T_PREC +IMMUTABLE ] = UnbPRecImm;

    /* install mutability test                                             */
    IsMutableObjFuncs[  T_PREC            ] = IsMutablePRecYes;
    IsMutableObjFuncs[  T_PREC +IMMUTABLE ] = IsMutablePRecNo;
    IsCopyableObjFuncs[ T_PREC            ] = IsCopyablePRecYes;
    IsCopyableObjFuncs[ T_PREC +IMMUTABLE ] = IsCopyablePRecYes;

    /* install into copy function tables                                  */
    CopyObjFuncs [ T_PREC                     ] = CopyPRec;
    CopyObjFuncs [ T_PREC +IMMUTABLE          ] = CopyPRec;
    CopyObjFuncs [ T_PREC            +COPYING ] = CopyPRecCopy;
    CopyObjFuncs [ T_PREC +IMMUTABLE +COPYING ] = CopyPRecCopy;
    CleanObjFuncs[ T_PREC                     ] = CleanPRec;
    CleanObjFuncs[ T_PREC +IMMUTABLE          ] = CleanPRec;
    CleanObjFuncs[ T_PREC            +COPYING ] = CleanPRecCopy;
    CleanObjFuncs[ T_PREC +IMMUTABLE +COPYING ] = CleanPRecCopy;

    /* install printer                                                     */
    PrintObjFuncs[  T_PREC            ] = PrintPRec;
    PrintObjFuncs[  T_PREC +IMMUTABLE ] = PrintPRec;
    PrintPathFuncs[ T_PREC            ] = PrintPathPRec;
    PrintPathFuncs[ T_PREC +IMMUTABLE ] = PrintPathPRec;

    /* install the kind function                                           */
    ImportGVarFromLibrary( "TYPE_PREC_MUTABLE",   &TYPE_PREC_MUTABLE   );
    ImportGVarFromLibrary( "TYPE_PREC_IMMUTABLE", &TYPE_PREC_IMMUTABLE );

    TypeObjFuncs[ T_PREC            ] = TypePRecMut;
    TypeObjFuncs[ T_PREC +IMMUTABLE ] = TypePRecImm;

    SetTypeObjFuncs [ T_PREC ] = SetTypePRecToComObj;

    MakeImmutableObjFuncs[ T_PREC   ] = MakeImmutablePRec;

    /* return success                                                      */
    return 0;
}


/****************************************************************************
<**
*F  PostRestore( <module> ) . . . . . . . . . . . . . after restore workspace
*/
static Int PostRestore (
    StructInitInfo *    module )
{
    /* get the appropriate record record name                              */
    OperationsRNam = RNamName( "operations"   );
    COMPONENTSRNam = RNamName( "COMPONENTS"   );
    EqRNam         = RNamName( "="            );
    LtRNam         = RNamName( "<"            );
    InRNam         = RNamName( "in"           );
    SumRNam        = RNamName( "+"            );
    DiffRNam       = RNamName( "-"            );
    ProdRNam       = RNamName( "*"            );
    QuoRNam        = RNamName( "/"            );
    LQuoRNam       = RNamName( "LeftQuotient" );
    PowRNam        = RNamName( "^"            );
    CommRNam       = RNamName( "Comm"         );
    ModRNam        = RNamName( "mod"          );

    /* return success                                                      */
    return 0;
}


/****************************************************************************
**
*F  InitLibrary( <module> ) . . . . . . .  initialise library data structures
*/
static Int InitLibrary (
    StructInitInfo *    module )
{
    /* init filters and functions                                          */
    InitGVarFuncsFromTable( GVarFuncs );

    /* return success                                                      */
    return PostRestore( module );
}


/****************************************************************************
**
*F  InitInfoPRecord() . . . . . . . . . . . . . . . . table of init functions
*/
static StructInitInfo module = {
    MODULE_BUILTIN,                     /* type                           */
    "precord",                          /* name                           */
    0,                                  /* revision entry of c file       */
    0,                                  /* revision entry of h file       */
    0,                                  /* version                        */
    0,                                  /* crc                            */
    InitKernel,                         /* initKernel                     */
    InitLibrary,                        /* initLibrary                    */
    0,                                  /* checkInit                      */
    0,                                  /* preSave                        */
    0,                                  /* postSave                       */
    PostRestore                         /* postRestore                    */
};

StructInitInfo * InitInfoPRecord ( void )
{
    FillInVersion( &module );
    return &module;
}


/****************************************************************************
**

*E  precord.c . . . . . . . . . . . . . . . . . . . . . . . . . . . ends here
*/



