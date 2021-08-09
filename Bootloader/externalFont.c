/**************************************************************************/
/*!  
		External Font Rom setup
		This will not phisically change the register but should be called before setFont(EXT)!
		You should use this values accordly Font ROM datasheet!
		Parameters:
		ert:ROM Type          (GT21L16T1W, GT21H16T1W, GT23L16U2W, GT30H24T3Y, GT23L24T3Y, GT23L24M1Z, GT23L32S4W, GT30H32S4W)
		erc:ROM Font Encoding (GB2312, GB12345, BIG5, UNICODE, ASCII, UNIJIS, JIS0208, LATIN)
		erf:ROM Font Family   (STANDARD, ARIAL, ROMAN, BOLD)
*/
/**************************************************************************/
void RA8875::setExternalFontRom(enum RA8875extRomType ert, enum RA8875extRomCoding erc, enum RA8875extRomFamily erf){
	uint8_t temp = _SFRSETReg;//just to preserve the reg in case something wrong
	switch(ert){ //type of rom
		case GT21L16T1W:
		case GT21H16T1W:
			temp &= 0x1F;
		break;
		case GT23L16U2W:
			temp &= 0x1F; temp |= 0x20;
		break;
		case GT23L24T3Y:
		case GT30H24T3Y:
		case ER3303_1://encoding GB12345
			temp &= 0x1F; temp |= 0x40;
			//erc = GB12345;//forced
		break;
		case GT23L24M1Z:
			temp &= 0x1F; temp |= 0x60;
		break;
		case GT23L32S4W:
		case GT30H32S4W:
			temp &= 0x1F; temp |= 0x80;
		break;
		default:
			_extFontRom = false;//wrong type, better avoid for future
			return;//cannot continue, exit
		}
		_fontRomType = ert;
	switch(erc){	//check rom font coding
		case GB2312:
			temp &= 0xE3;
		break;
		case GB12345:
			temp &= 0xE3; temp |= 0x04;
		break;
		case BIG5:
			temp &= 0xE3; temp |= 0x08;
		break;
		case UNICODE:
			temp &= 0xE3; temp |= 0x0C;
		break;
		case ASCII:
			temp &= 0xE3; temp |= 0x10;
		break;
		case UNIJIS:
			temp &= 0xE3; temp |= 0x14;
		break;
		case JIS0208:
			temp &= 0xE3; temp |= 0x18;
		break;
		case LATIN:
			temp &= 0xE3; temp |= 0x1C;
		break;
		default:
			_extFontRom = false;//wrong coding, better avoid for future
			return;//cannot continue, exit
		}
		_fontRomCoding = erc;
		_SFRSETReg = temp;
		setExtFontFamily(erf,false);	
		_extFontRom = true;
		//writeReg(RA8875_SFRSET,_SFRSETReg);//0x2F
		//delay(4);
}

/**************************************************************************/
/*!  
		select the font family for the external Font Rom Chip
		Parameters:
		erf: STANDARD, ARIAL, ROMAN, BOLD
		setReg: 
		true(send phisically the register, useful when you change
		family after set setExternalFontRom)
		false:(change only the register container, useful during config)
*/
/**************************************************************************/
void RA8875::setExtFontFamily(enum RA8875extRomFamily erf,boolean setReg) {
	_fontFamily = erf;
	switch(erf){	//check rom font family
		case STANDARD:
			_SFRSETReg &= 0xFC;
		break;
		case ARIAL:
			_SFRSETReg &= 0xFC; _SFRSETReg |= 0x01;
		break;
		case ROMAN:
			_SFRSETReg &= 0xFC; _SFRSETReg |= 0x02;
		break;
		case BOLD:
			_SFRSETReg |= ((1<<1) | (1<<0)); // set bits 1 and 0
		break;
		default:
			_fontFamily = STANDARD; _SFRSETReg &= 0xFC;
			return;
	}
	if (setReg) writeReg(RA8875_SFRSET,_SFRSETReg);
}

/**************************************************************************/
/*!  
		choose from internal/external (if exist) Font Rom
		Parameters:
		s: Font source (INT,EXT)
*/
/**************************************************************************/
void RA8875::setFont(enum RA8875fontSource s) {
	//enum RA8875fontCoding c
	if (s == INT){
		//check the font coding
		if (_extFontRom) {
			setFontSize(X16,false);
			writeReg(RA8875_SFRSET,0b00000000);//_SFRSETReg
		}
		_FNCR0Reg &= ~((1<<7) | (1<<5));// Clear bits 7 and 5
		writeReg(RA8875_FNCR0,_FNCR0Reg);
		_fontSource = s;
		delay(1);
	} else {
		if (_extFontRom){
			_fontSource = s;
			//now switch
			_FNCR0Reg |= (1 << 5);
			writeReg(RA8875_FNCR0,_FNCR0Reg);//0x21
			delay(1);
			writeReg(RA8875_SFCLR,0x02);//Serial Flash/ROM CLK frequency/2
			setFontSize(X24,false);////X24 size
			writeReg(RA8875_SFRSET,_SFRSETReg);//at this point should be already set
			delay(4);
			writeReg(RA8875_SROC,0x28);// 0x28 rom 0,24bit adrs,wave 3,1 byte dummy,font mode, single mode
			delay(4);
		} else {
			setFont(INT);
		}
	}
}
/**************************************************************************/
/*!		
		Choose between 16x16(8x16) - 24x24(12x24) - 32x32(16x32)
		for External Font ROM
		Parameters:
		ts:X16,X24,X32
		halfSize:true/false (16x16 -> 8x16 and so on...)
*/
/**************************************************************************/
void RA8875::setFontSize(enum RA8875tsize ts,boolean halfSize){
	switch(ts){
		case X16:
			_FWTSETReg &= 0x3F;
		break;
		case X24:
			_FWTSETReg &= 0x3F; _FWTSETReg |= 0x40; 
		break;
		case X32:
			_FWTSETReg &= 0x3F; _FWTSETReg |= 0x80; 
		break;
		default:
		return;
	}
	_textSize = ts;
	writeReg(RA8875_FWTSET,_FWTSETReg);
}
