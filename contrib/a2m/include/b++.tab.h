typedef union {
	double value;
	char carat;
	unsigned int unreal;
	char* stringa;
	char angle_spec;
} YYSTYPE;
#define	NUM	257
#define	NUM_DEG	258
#define	SEP	259
#define	TERM	260
#define	NL	261
#define	EQ	262
#define	EXCL	263
#define	SLASH	264
#define	COLON	265
#define	INVSLASH	266
#define	ID	267
#define	ANG_TYPE	268
#define	D	269
#define	TRUE	270
#define	FALSE	271
#define	BEAM	272
#define	PART	273
#define	I	274
#define	J	275
#define	LENGTH	276
#define	AREA	277
#define	IXX	278
#define	IYY	279
#define	IZZ	280
#define	ASY	281
#define	ASZ	282
#define	EMODULUS	283
#define	GMODULUS	284
#define	CMATRIX	285
#define	CRATIO	286
#define	GROUND	287
#define	MASS	288
#define	CM	289
#define	IM	290
#define	IP	291
#define	QG	292
#define	REULER	293
#define	ZG	294
#define	XG	295
#define	VX	296
#define	VY	297
#define	VZ	298
#define	WX	299
#define	WY	300
#define	WZ	301
#define	VM	302
#define	WM	303
#define	EXACT	304
#define	PSI	305
#define	THETA	306
#define	PHI	307
#define	XCOORD	308
#define	YCOORD	309
#define	ZCOORD	310
#define	MARKER	311
#define	USEXP	312
#define	POINT_MASS	313
#define	FLEX_BODY	314
#define	NODE_ID	315
#define	FLOATING	316
#define	QP	317
#define	XP	318
#define	ZP	319
#define	ACCGRAV	320
#define	IGRAV	321
#define	JGRAV	322
#define	KGRAV	323
#define	END	324
#define	EQUILIBRIUM	325
#define	ALIMIT	326
#define	ERROR	327
#define	IMBALANCE	328
#define	MAXIT	329
#define	PATTERN	330
#define	STABILITY	331
#define	TLIMIT	332
#define	GFORCE	333
#define	JFLOAT	334
#define	RM	335
#define	FX	336
#define	FY	337
#define	FZ	338
#define	TX	339
#define	TY	340
#define	TZ	341
#define	FUNCTION	342
#define	USER	343
#define	IC	344
#define	AERROR	345
#define	AMAXIT	346
#define	APATTERN	347
#define	VERROR	348
#define	MATERIAL	349
#define	DENSITY	350
#define	NAME	351
#define	YOUNGS_MODULUS	352
#define	POISSONS_RATIO	353
#define	JOINT	354
#define	CONVEL	355
#define	CYLINDRICAL	356
#define	FIXED	357
#define	HOOKE	358
#define	PLANAR	359
#define	RACKPIN	360
#define	REVOLUTE	361
#define	SCREW	362
#define	PITCH	363
#define	SPHERICAL	364
#define	TRANSLATIONAL	365
#define	UNIVERSAL	366
#define	ICTRAN	367
#define	ICROT	368
#define	DELTA_V	369
#define	INNER_RADIUS	370
#define	FRICTION	371
#define	MAXIMUM_DEFORMATION	372
#define	MU_DYN_ROT	373
#define	MU_STAT_ROT	374
#define	OUTER_RADIUS	375
#define	PRELOAD_RADIAL	376
#define	PRELOAD_AXIAL	377
#define	HEIGHT	378
#define	MU_DYN_TRANS	379
#define	MU_STAT_TRANS	380
#define	PRELOAD_X	381
#define	PRELOAD_Y	382
#define	WIDTH	383
#define	PRELOAD_ONLY	384
#define	RACKPID	385
#define	ON	386
#define	OFF	387
#define	PD	388
#define	MAX_FRIC_ROT	389
#define	JPRIM	390
#define	ATPOINT	391
#define	INLINE	392
#define	INPLANE	393
#define	ORIENTATION	394
#define	PARALLEL_AXES	395
#define	PERPENDICULAR	396
#define	SPRINGDAMPER	397
#define	TRANSLATION	398
#define	ROTATION	399
#define	CT	400
#define	KT	401
#define	TORQUE	402
#define	ANGLE	403
#define	FORCE	404
#define	C0	405
#define	K0	406
#define	LSE	407
#define	LSE_C	408
#define	LSE_X	409
#define	LSE_A	410
#define	STATIC_HOLD	411
#define	LSE_U	412
#define	LSE_B	413
#define	LSE_Y	414
#define	RESULTS	415
#define	FORMATTED	416
#define	COMMENT	417
#define	NOACCELERATIONS	418
#define	NOAPPLIEDFORCES	419
#define	NODATASTRUCTURES	420
#define	NODISPLACEMENTS	421
#define	NOFLOATINGMARKERS	422
#define	NOLINEAR	423
#define	NOREACTIONFORCES	424
#define	NOSYSTEMELEMENTS	425
#define	NOTIRES	426
#define	NOVELOCITIES	427
#define	REQUEST	428
#define	DISPLACEMENT	429
#define	VELOCITY	430
#define	ACCELERATION	431
#define	COMMENTS	432
#define	TITLE	433
#define	F1	434
#define	F2	435
#define	F3	436
#define	F4	437
#define	F5	438
#define	F6	439
#define	F7	440
#define	F8	441
#define	UNITS	442
#define	DYNE	443
#define	KILOGRAM_FORCE	444
#define	KNEWTON	445
#define	KPOUND_FORCE	446
#define	NEWTON	447
#define	OUNCE_FORCE	448
#define	POUND_FORCE	449
#define	GRAM	450
#define	KILOGRAM	451
#define	KPOUND_MASS	452
#define	OUNCE_MASS	453
#define	POUND_MASS	454
#define	SLUG	455
#define	CENTIMETER	456
#define	FOOT	457
#define	KILOMETER	458
#define	INCH	459
#define	METER	460
#define	MILLIMETER	461
#define	MILE	462
#define	TIME	463
#define	HOUR	464
#define	MILLISECOND	465
#define	MINUTE	466
#define	SECOND	467
#define	SYSTEM	468
#define	CGS	469
#define	FPS	470
#define	IPS	471
#define	MKS	472
#define	NONE	473
#define	UCF	474
#define	SFORCE	475
#define	ACTIONONLY	476
#define	VTORQUE	477
#define	VFORCE	478
#define	ADAMS	479
#define	VIEW	480
#define	MODEL	481
#define	GRAPHICS	482
#define	OUTPUT	483
#define	_SET_	484
#define	NOT_CLASSIFIED	485


extern YYSTYPE yylval;
