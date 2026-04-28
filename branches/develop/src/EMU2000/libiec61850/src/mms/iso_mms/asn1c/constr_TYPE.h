/*-
 * Copyright (c) 2003, 2004, 2005, 2006 Lev Walkin <vlm@lionet.info>.
 * All rights reserved.
 * Redistribution and modifications are permitted subject to BSD license.
 */
/*
 * This file contains the declaration structure called "ASN.1 Type Definition",
 * which holds all information necessary for encoding and decoding routines.
 * This structure even contains pointer to these encoding and decoding routines
 * for each defined ASN.1 type.
 */
#ifndef	_CONSTR_TYPE_H_
#define	_CONSTR_TYPE_H_

#include <ber_tlv_length.h>
#include <ber_tlv_tag.h>

#ifdef __cplusplus
extern "C" {
#endif

struct asn_TYPE_descriptor_s;	/* Forward declaration */
struct asn_TYPE_member_s;	/* Forward declaration */

/*
 * This type provides the context information for various ASN.1 routines,
 * primarily ones doing decoding. A member _asn_ctx of this type must be
 * included into certain target language's structures, such as compound types.
 */
/*
 * ASN.1简介:
 * ASN.1规范位于ISO/OSI七层开放互连模型的第六层表示层，主要分为语法规则和编码规则两部分，
 * 语法规则用于描述信息对象的具体构成，包括数据类型，内容顺序或结构；编码规则定义了信息的具体编解码语法。
 * ASN.1定义的数据类型既有简单的基本数据类型，也有复杂的结构类型，如下所示:
 * ---------+-----------------------------------+--------------------------------------
 * 分类		|	类型							|				含义
 * ---------+-----------------------------------+--------------------------------------
 * 基本类型 |				NULL				|只包含一个值NULL
 * 			|				INTEGER				|全部整数(包含正数和负数)
 * 			|				REAL				|实数
 * 			|				ENUMERATED			|标识符的枚举
 * 			|				BITSTRING			|比特串
 * 			|				OCTETSTRING			|字节串
 * 			|	OBJECT IDENTIFIER RELATIVE-OID	|一个实体的标识符，它在一个全世界范围数状结构中注册
 * 			|		EXTERNAL,EMBEDDED PDV		|表示层上下文交换类型
 * 			|...String(除BITSTRING,OCTETSTRING)	|各种字符串
 * 			|			CHARACTERSTRING			|允许为字符串协商一个明确的字符表!
 * 			|		UTCTime,GeneralizedTime		|日期!
 * ---------+-----------------------------------+--------------------------------------
 * 结构类型	|				CHOICE				|在类型中选择(相当于c中的共用体)
 * 			|				SEQUENCE			|由不同类型的值组成一个有序的结构(相当于c中的结构体)
 * 			|				SET					|由不同类型的值组成一个无序的结构
 * 			|				SEQUENCEOF			|由相同类型的值组成一个有序的结构(相当于c中的数组)
 * 			|				SETOF				|由相同类型的值组成一个无序的结构
 * ---------+-----------------------------------+--------------------------------------
 *  tag用来表示数据类型!
 */
typedef struct asn_struct_ctx_s {
	short phase;		/* Decoding phase */			//phase:阶段，相位!
	short step;		/* Elementary step of a phase */	//Elementary:初等的，基本的，元素的!
	int context;		/* Other context information */
	void *ptr;		/* Decoder-specific stuff (stack elements) */		//stuff:材料，原料!
	ber_tlv_len_t left;	/* Number of bytes left, -1 for indefinite */	//indefinite:不明确的，模糊的!
} asn_struct_ctx_t;

#include <ber_decoder.h>	/* Basic Encoding Rules decoder */				//BER:基本编码规则
#include <der_encoder.h>	/* Distinguished Encoding Rules encoder */		//DER:唯一编码规则
#include <xer_decoder.h>	/* Decoder of XER (XML, text) */				//XER:XML编码规则
#include <xer_encoder.h>	/* Encoder into XER (XML, text) */
#include <per_decoder.h>	/* Packet Encoding Rules decoder */
#include <per_encoder.h>	/* Packet Encoding Rules encoder */				//PER:压缩编码规则!
#include <constraints.h>	/* Subtype constraints support */				//CER:规范编码规则!

/*
 * Free the structure according to its specification.
 * If (free_contents_only) is set, the wrapper structure itself (struct_ptr)
 * will not be freed. (It may be useful in case the structure is allocated
 * statically or arranged on the stack, yet its elements are allocated
 * dynamically.)
 */
typedef void (asn_struct_free_f)(
		struct asn_TYPE_descriptor_s *type_descriptor,
		void *struct_ptr, int free_contents_only);
#define	ASN_STRUCT_FREE(asn_DEF, ptr)	(asn_DEF).free_struct(&(asn_DEF),ptr,0)
#define	ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF, ptr)	\
					(asn_DEF).free_struct(&(asn_DEF),ptr,1)

/*
 * Print the structure according to its specification.
 */
typedef int (asn_struct_print_f)(
		struct asn_TYPE_descriptor_s *type_descriptor,
		const void *struct_ptr,
		int level,	/* Indentation level */
		asn_app_consume_bytes_f *callback, void *app_key);

/*
 * Return the outmost tag of the type.
 * If the type is untagged CHOICE, the dynamic operation is performed.
 * NOTE: This function pointer type is only useful internally.
 * Do not use it in your application.
 */
typedef ber_tlv_tag_t (asn_outmost_tag_f)(
		struct asn_TYPE_descriptor_s *type_descriptor,
		const void *struct_ptr, int tag_mode, ber_tlv_tag_t tag);
/* The instance of the above function type; used internally. */
asn_outmost_tag_f asn_TYPE_outmost_tag;


/*
 * The definitive description of the destination language's structure.
 */
typedef struct asn_TYPE_descriptor_s {
	char *name;	/* A name of the ASN.1 type. "" in some cases. */
	char *xml_tag;	/* Name used in XML tag */

	/*
	 * Generalized functions for dealing with the specific type.
	 * May be directly invoked by applications.
	 */
	asn_struct_free_f  *free_struct;	/* Free the structure */
	asn_struct_print_f *print_struct;	/* Human readable output */
	asn_constr_check_f *check_constraints;	/* Constraints validator */
	ber_type_decoder_f *ber_decoder;	/* Generic BER decoder */		//ber_decoder是一枚函数指针!
	der_type_encoder_f *der_encoder;	/* Canonical DER encoder */
	xer_type_decoder_f *xer_decoder;	/* Generic XER decoder */
	xer_type_encoder_f *xer_encoder;	/* [Canonical] XER encoder */
	per_type_decoder_f *uper_decoder;	/* Unaligned PER decoder */
	per_type_encoder_f *uper_encoder;	/* Unaligned PER encoder */

	/***********************************************************************
	 * Internally useful members. Not to be used by applications directly. *
	 **********************************************************************/

	/*
	 * Tags that are expected to occur.
	 */
	asn_outmost_tag_f  *outmost_tag;	/* <optional, internal> */
	ber_tlv_tag_t *tags;	/* Effective tags sequence for this type */
	int tags_count;		/* Number of tags which are expected */
	ber_tlv_tag_t *all_tags;/* Every tag for BER/containment */
	int all_tags_count;	/* Number of tags */

	asn_per_constraints_t *per_constraints;	/* PER compiled constraints */

	/*
	 * An ASN.1 production type members (members of SEQUENCE, SET, CHOICE).
	 */
	struct asn_TYPE_member_s *elements;
	int elements_count;

	/*
	 * Additional information describing the type, used by appropriate
	 * functions above.
	 */
	void *specifics;
} asn_TYPE_descriptor_t;

/*
 * This type describes an element of the constructed type,
 * i.e. SEQUENCE, SET, CHOICE, etc.
 */
  enum asn_TYPE_flags_e {
	ATF_NOFLAGS,
	ATF_POINTER	= 0x01,	/* Represented by the pointer */
	ATF_OPEN_TYPE	= 0x02	/* ANY type, without meaningful tag */
  };
typedef struct asn_TYPE_member_s {
	enum asn_TYPE_flags_e flags;	/* Element's presentation flags */
	int optional;	/* Following optional members, including current */
	int memb_offset;		/* Offset of the element */
	ber_tlv_tag_t tag;		/* Outmost (most immediate) tag */
	int tag_mode;		/* IMPLICIT/no/EXPLICIT tag at current level */
	asn_TYPE_descriptor_t *type;	/* Member type descriptor */
	asn_constr_check_f *memb_constraints;	/* Constraints validator */
	asn_per_constraints_t *per_constraints;	/* PER compiled constraints */
	int (*default_value)(int setval, void **sptr);	/* DEFAULT <value> */
	char *name;			/* ASN.1 identifier of the element */
} asn_TYPE_member_t;

/*
 * BER tag to element number mapping.
 */
typedef struct asn_TYPE_tag2member_s {
	ber_tlv_tag_t el_tag;	/* Outmost tag of the member */
	int el_no;		/* Index of the associated member, base 0 */
	int toff_first;		/* First occurence of the el_tag, relative */
	int toff_last;		/* Last occurence of the el_tag, relatvie */
} asn_TYPE_tag2member_t;

/*
 * This function is a wrapper around (td)->print_struct, which prints out
 * the contents of the target language's structure (struct_ptr) into the
 * file pointer (stream) in human readable form.
 * RETURN VALUES:
 * 	 0: The structure is printed.
 * 	-1: Problem dumping the structure.
 * (See also xer_fprint() in xer_encoder.h)
 */
int asn_fprint(FILE *stream,		/* Destination stream descriptor */
	asn_TYPE_descriptor_t *td,	/* ASN.1 type descriptor */
	const void *struct_ptr);	/* Structure to be printed */

#ifdef __cplusplus
}
#endif

#endif	/* _CONSTR_TYPE_H_ */
