/*	SCCS Id: @(#)permonst.h 3.4	1999/07/02	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef PERMONST_H
#define PERMONST_H

/*	This structure covers all attack forms.
 *	aatyp is the gross attack type (eg. claw, bite, breath, ...)
 *	adtyp is the damage type (eg. physical, fire, cold, spell, ...)
 *	damn is the number of hit dice of damage from the attack.
 *	damd is the number of sides on each die.
 *
 *	Some attacks can do no points of damage.  Additionally, some can
 *	have special effects *and* do damage as well.  If damn and damd
 *	are set, they may have a special meaning.  For example, if set
 *	for a blinding attack, they determine the amount of time blinded.
 */

struct attack {
	uchar		aatyp;
	uchar		adtyp, damn, damd;
};

/*	Max # of attacks for any given monster.
 */

#define NATTK		6

/*	Weight of a human body
 */

#define WT_HUMAN	1450

#ifndef ALIGN_H
#include "align.h"
#endif
#include "monattk.h"
#include "monflag.h"

struct permonst {
	const char	*mname;			/* full name */
	char		mlet;			/* symbol */
	schar		mlevel,			/* base monster level */
			mmove,			/* move speed */
			ac,			/* (base) armor class */
			mr;			/* (base) magic resistance */
	aligntyp	maligntyp;		/* basic monster alignment */
	unsigned short	geno;			/* creation/geno mask value */
	struct	attack	mattk[NATTK];		/* attacks matrix */
	unsigned short	cwt,			/* weight of corpse */
			cnutrit;		/* its nutritional value */
	short		pxlth;			/* length of extension */
	uchar		msound;			/* noise it makes (6 bits) */
	uchar		msize;			/* physical size (3 bits) */
	uchar		mresists;		/* resistances */
	uchar		mconveys;		/* conveyed by eating */
	unsigned long	mflags1,		/* boolean bitflags */
			mflags2;		/* more boolean bitflags */
	unsigned short	mflags3;		/* yet more boolean bitflags */
# ifdef TEXTCOLOR
	uchar		mcolor;			/* color to use */
# endif
};

extern NEARDATA struct permonst
		mons[];		/* the master list of monster types */

#define VERY_SLOW 3
#define SLOW_SPEED 9
#define NORMAL_SPEED 12 /* movement rates */
#define FAST_SPEED 15
#define VERY_FAST 24

#define NON_PM		PM_PLAYERMON		/* "not a monster" */
#define LOW_PM		(NON_PM+1)		/* first monster in mons[] */
#define SPECIAL_PM	PM_LONG_WORM_TAIL	/* [normal] < ~ < [special] */
	/* mons[SPECIAL_PM] through mons[NUMMONS-1], inclusive, are
	   never generated randomly and cannot be polymorphed into */

#define race_lev_mod(racemask)  ((racemask & M2_KOBOLD)          ? -3 : \
                                 (racemask & (M2_GNOME|M2_ORC))  ? -2 : \
                                 (racemask & (M2_DWARF|M2_ELF))  ? -1 : \
				 (racemask & M2_HUMAN)           ?  0 : \
				 (racemask & M2_OGRE)            ?  1 : \
                                 (racemask & M2_ETTIN)           ?  2 : \
                                 (racemask & M2_GIANT)           ?  3 : 0)

#define race_lev_mod2(racemask) ((racemask & M2_GIANT)          ?  3 : \
                                 (racemask & M2_ETTIN)          ?  2 : \
				 (racemask & M2_OGRE)           ?  1 : \
				 (racemask & M2_HUMAN)          ?  0 : \
                                 (racemask & (M2_DWARF|M2_ELF)) ? -1 : \
                                 (racemask & (M2_GNOME|M2_ORC)) ? -2 : \
                                 (racemask & M2_KOBOLD)         ? -3 : 0)

#endif /* PERMONST_H */
