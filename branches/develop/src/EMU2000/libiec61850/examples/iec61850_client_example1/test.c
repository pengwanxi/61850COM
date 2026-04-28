void recursive(MmsValue *self)
{
	switch(self->type){
	case MMS_ARRAY...MMS_STRUCTURE:
		//MmsValue_getElement(),递归!包括对ARRAY的遍历!
		//MmsValue_getArraySize()
		;
		break;
	case MMS_BOOLEAN:
		//MmsValue_getBoolean()
		;
		break;
	case MMS_BIT_STRING:
		//MmsValue_getBitStringBit(也许是该函数!)
		;
		break;
	case MMS_INTEGER:
		;
		break;			//和MMS_UNSIGNED是不是同样的处理方式有待确认!
	case MMS_UNSIGNED:
		;
		break;
	case MMS_FLOAT:
		//MmsValue_toFloat();
		;
		break;
	case MMS_OCTET_STRING:
		//MmsValue_getOctetStingBuffer
		;
		break;
	case MMS_VISIBLE_STRING:
		//
		;
		break;
	case MMS_GENERALIZED_TIME:
		//MmsValue_getUtcTimeInMs()
		;
		break;
	case MMS_BINARY_TIME:
		//MmsValue_getBinaryTimeAsUtcMs
		;
		break;
	case MMS_BCD:
		;
		break;
	case MMS_OBJ_ID:
		;
		break;
	case MMS_STRING:
		;
		break;
	case MMS_UTC_TIME:
		;
		break;
	case MMS_DATA_ACCESS_ERROR:
		;
		break;
	}
}
