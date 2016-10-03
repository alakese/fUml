#ifndef GLOBALS_H
#define GLOBALS_H

/* Enums */
enum { GENERALPAGE = 0, MEMBERSPAGE, FUNCTIONSPAGE, DISPLAYPAGE, FONTPAGE };
/* States of the checks in a 8 bit member
 *  Bit 1 : show members 
 *  Bit 2 : show membernames only
 *  Bit 3 : show methods
 *  Bit 4 : show methodnames only
 *  Bit 5 : show public only */
enum { BIT1 = 1, BIT2 = 2, BIT3 = 4, BIT4 = 8, BIT5 = 16 };

/* Wird verwendet in AddLiteralCommand */
enum literalType { MUL_BEGIN = 0, MUL_END, ROLE_BEGIN, ROLE_END, DESC };

/* Wird verwendet in AssociationItem */
enum ITEM_TYPE { ASSOCIATION = 0, AGGREGATION, COMPOSITION };

/* Macros */
#define SET_BIT(in, x) (in |= x)
#define GET_BIT(in, x) ((in & x) > 0 ? 1 : 0)

#endif // GLOBALS_H
